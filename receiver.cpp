#include "receiver.hpp"
#include "service_log.hpp"
#include <sys/epoll.h>
#include <unistd.h>
#include <assert.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "util.hpp"

const static int max_recv_buf_len = (1024*1024);

receiver::receiver(int _epoll_cnt, int _max_conn, int _key_group_cnt, int _group_capacity) : acceptor(_epoll_cnt, _max_conn) {
	key_group_cnt = _key_group_cnt;
	group_capacity = _group_capacity;
	key_groups = (KeyGroup**)malloc(key_group_cnt * sizeof(KeyGroup*));
	for (int i = 0; i < key_group_cnt; i++) {
		key_groups[i] = new KeyGroup(group_capacity);
	}
}

receiver::~receiver() {
	for (int i = 0; i < key_group_cnt; i++) {
		delete key_groups[i];
	}
	free(key_groups);
}

int receiver::open(int thread_cnt) {
	for (int i = 0; i < epoll_cnt; i++) {
		epoll_fds[i] = epoll_create(max_conn);
		if (epoll_fds[i] == -1) {
			_ERROR("create epoll fd %d err", i);
			return -1;
		}
	}
	for (int i = 0; i < epoll_cnt; i++) {
		if (create_pipe(i) < 0) {
			_ERROR("create pipe err on idx=%d", i);
			return -1;
		}
		add_input_fd(pipe_read_fds[i], epoll_fds[i]);
	}
	return task_base::open(thread_cnt * epoll_cnt, (1<<10));
}

int receiver::stop() {
	int write_cnt = 0;
	for (int i = 0; i < epoll_cnt; i++) {
		int stop = 1;
		write_cnt = write(pipe_write_fds[i], &stop, 1);
		assert(write_cnt == 1);
	}
	stop_task = 1;
	join();
	_INFO("receiver stops");
	return 0;
}

int receiver::svc() {
	int local_epoll_idx = 0;
	pthread_mutex_lock(&epoll_idx_mutex);
	local_epoll_idx = epoll_thread_cnt;
	epoll_thread_cnt++;
	pthread_mutex_unlock(&epoll_idx_mutex);
	local_epoll_idx = local_epoll_idx % epoll_cnt;

	int epoll_fd = epoll_fds[local_epoll_idx];
	epoll_event* es = epoll_ready_events[local_epoll_idx];
	pthread_mutex_t* epoll_mutex = &epoll_mutexes[local_epoll_idx];
	int* event_num = &epoll_ready_event_num[local_epoll_idx];

	char* recv_buf = (char*)malloc(max_recv_buf_len);
	int fd, new_fd;
	int tmp_conn;
	sockaddr_in sa;
	socklen_t len;

	while (!stop_task) {
		pthread_mutex_lock(epoll_mutex);
		if (stop_task) {
			pthread_mutex_unlock(epoll_mutex);
			break;
		}

		if ((*event_num) <= 0) {
			*event_num = epoll_wait(epoll_fd, es, max_conn, -1);
		}
		if ((*event_num)-- < 0) {
			pthread_mutex_unlock(epoll_mutex);
			if (errno == EINTR) {
				continue;
			}
			else {
				break;
			}
		}

		fd = es[(*event_num)].data.fd;

		if (fd == listen_fd) {
			del_input_fd(fd, epoll_fd);
			pthread_mutex_unlock(epoll_mutex);
			while ((new_fd = accept(fd, NULL, NULL)) >= 0) {
				pthread_mutex_lock(&conn_mutex);
				conn_num++;
				tmp_conn = conn_num;
				pthread_mutex_unlock(&conn_mutex);
				len = sizeof(sa);
				getpeername(new_fd, (struct sockaddr *)&sa, &len);
				set_socket(new_fd, O_NONBLOCK);
				add_input_fd(new_fd, epoll_fds[incoming_fd_cnt % epoll_cnt]);
				_INFO("new conn on %s:%d,fd=%d,conn_num=%d,epollno=%d", inet_ntoa(sa.sin_addr), ntohs(sa.sin_port), new_fd, tmp_conn, incoming_fd_cnt % epoll_cnt);
				incoming_fd_cnt++;
			}
			add_input_fd(fd, epoll_fd);
		} else if (fd == pipe_read_fds[local_epoll_idx]) {
			_INFO("get a stop signal");
			pthread_mutex_unlock(epoll_mutex);
			continue;
		} else {
			del_input_fd(fd, epoll_fd);
			pthread_mutex_unlock(epoll_mutex);
			handle_request(fd, epoll_fd, recv_buf);
		}
	}

	free(recv_buf);
	return 0;
}

int receiver::start_listen() {
	if (create_listen(listen_fd, listen_port)) {
		_ERROR("create listen fail on port=%d", listen_port);
		return -1;
	}
	add_input_fd(listen_fd, epoll_fds[0]);
	return 0;
}

int receiver::handle_request(int fd, int epoll_fd, char* buf) {
	int need_to_read = sizeof(request);
	timeval timeout = {0, 50*1000};
	int ret;
	ret = readn_timeout(fd, buf, need_to_read, &timeout);
	if (ret != need_to_read) {
		close_fd(fd);
		return -1;
	}
	request* req = (request*)buf;
	need_to_read = req->key_len + req->body_len;
	ret = readn_timeout(fd, buf + sizeof(uint32_t) * 2, need_to_read, &timeout);
	if (ret != need_to_read) {
		close_fd(fd);
		return -1;
	}
	uint64_t key;
	uint8_t section_id;
	key = get_raw_key(buf, &section_id);
	_INFO("get key=%ld,seciond_id=%u", key, section_id);
	KeyGroup* key_group = key_groups[section_id % key_group_cnt];
	int cnt = key_group->Inc(key);
	writen_timeout(fd, &cnt, sizeof(int), 10);
	add_input_fd(fd, epoll_fd);
	return 0;
}
