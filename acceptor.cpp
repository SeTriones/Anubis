#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>
#include <string.h>
#include <service_log.hpp>
#include <unistd.h>
#include <errno.h>
#include "acceptor.hpp"

#define SOCKET_SND_BUF_SIZE (1024*1024)
#define SOCKET_RCV_BUF_SIZE (1024*1024)

acceptor::acceptor(int _epoll_cnt, int _max_conn) {
	epoll_cnt = _epoll_cnt;
	max_conn = _max_conn;
	_INFO("epoll_cnt=%d,max_conn=%d", epoll_cnt, max_conn);
	listen_port = -1;
	stop_task = 0;
	conn_num = 0;
	wid = 0;
	epoll_fds = new int[epoll_cnt];
	epoll_ready_event_num = new int[epoll_cnt];
	pipe_read_fds = new int[epoll_cnt];
	pipe_write_fds = new int[epoll_cnt];
	add_sum = 2;
	for (int i = 0; i < epoll_cnt; i++) {
		epoll_ready_event_num[i] = 0;
	}
	epoll_mutexes = new pthread_mutex_t[epoll_cnt];
	for (int i = 0; i < epoll_cnt; i++) {
		pthread_mutex_init(&epoll_mutexes[i], NULL);
	}
	epoll_ready_events = new epoll_event*[epoll_cnt];
	for (int i = 0; i < epoll_cnt; i++) {
		epoll_ready_events[i] = new epoll_event[max_conn];
	}
	pthread_mutex_init(&epoll_idx_mutex, NULL);
	incoming_fd_cnt = 0;
	epoll_thread_cnt = 0;
	pthread_mutex_init(&conn_mutex, NULL);
}

acceptor::~acceptor() {
	delete[] epoll_fds;
	delete[] epoll_ready_event_num;
	delete[] pipe_read_fds;
	delete[] pipe_write_fds;
	for (int i = 0; i < epoll_cnt; i++) {
		pthread_mutex_destroy(&epoll_mutexes[i]);
	}
	delete[] epoll_mutexes;
	for (int i = 0; i < epoll_cnt; i++) {
		delete[] epoll_ready_events[i];
	}
	delete[] epoll_ready_events;
	pthread_mutex_destroy(&epoll_idx_mutex);
	pthread_mutex_destroy(&conn_mutex);
}
/*
int acceptor::open(configuration config) {
	listen_port = config.port;
	max_conn = config.max_conn;
	epoll_ready_event = (epoll_event*)malloc(sizeof(epoll_event) * max_conn);
	if ((epoll_fd = epoll_create(max_conn)) == -1) {
		_ERROR("create epoll fd err");
		return -1;
	}
	if (create_listen(listen_fd, listen_port)) {
		_ERROR("create listen fail on port=%d", listen_port);
		return -1;
	}
	add_input_fd(listen_fd);
	return task_base::open(1, config.thread_stack_size);
}

int acceptor::stop() {
	::close(listen_fd);
	stop_task = 1;
	::close(epoll_fd);
	join();
	_INFO("acceptor stopped");
	return 0;
}

int acceptor::svc() {
	int event_num = -1;
	int i;
	int fd;
	while (!stop_task) {
		event_num = epoll_wait(epoll_fd, epoll_ready_event, max_conn, -1);
		_INFO("acceptor event_num=%d", event_num);
		if (event_num < 0) {
			if (errno == EINTR) {
				_INFO("acceptor EINTR");
				continue;
			}
			else {
				_ERROR("epoll_wait err, errno=%d", errno);
				break;
			}
		}
		for (i = 0; i < event_num; i++) {
			fd = epoll_ready_event[i].data.fd;
			if (fd == listen_fd) {
				del_input_fd(fd);
				int new_fd;
				sockaddr_in sa;
				socklen_t len;
				int tmp;
				while ((new_fd = accept(fd, NULL, NULL)) >= 0) {
					pthread_mutex_lock(&conn_mutex);
					conn_num++;
					tmp = conn_num;
					pthread_mutex_unlock(&conn_mutex);
					len = sizeof(sa);
					getpeername(new_fd, (struct sockaddr *)&sa, &len);
					_INFO("new conn on %s:%d, conn_num=%d", inet_ntoa(sa.sin_addr), ntohs(sa.sin_port), tmp);
					set_socket(new_fd, O_NONBLOCK);
					add_input_fd(new_fd);
				}
				add_input_fd(fd);
			}
			else {
				// TODO submit to handler
				del_input_fd(fd);
				Worker* worker = new Worker(fd);
				worker->id = genWid();
				_INFO("[%08X] recv req from fd=%d", worker->id, fd);
				handler->submitWorker(worker);
			}
		}
	}
	return 0;
}
*/

int acceptor::recycle_fd(int fd, int epoll_idx) {
	if (epoll_idx < 0 || epoll_idx >= epoll_cnt) {
		_ERROR("err epollidx=%d", epoll_idx);
		return -1;
	}
	return add_input_fd(fd, epoll_fds[epoll_idx]);
}

int acceptor::close_fd(int fd) {
	int tmp;
	pthread_mutex_lock(&conn_mutex);
	conn_num--;
	tmp = conn_num;
	pthread_mutex_unlock(&conn_mutex);
	_INFO("close fd=%d, conn_num=%d", fd, tmp);
	return close(fd);
}

int acceptor::create_listen(int &socket_fd, unsigned short port) {
	sockaddr_in addr;
	memset(&addr, 0, sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		return -1;
	if (set_socket(socket_fd, O_NONBLOCK))
		return -1;
	if (::bind(socket_fd, (const sockaddr*)&addr, sizeof addr))
		return -1;
	if (listen(socket_fd, max_conn))
		return -1;

	return 0;
}

int acceptor::set_socket(int fd, int flag) {
	int options;
	options = SOCKET_SND_BUF_SIZE;
	setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &options, sizeof(int));
	options = SOCKET_RCV_BUF_SIZE;
	setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &options, sizeof(int));
	options = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &options, sizeof(int));
	options = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, options | flag);
	int on = 1;
	int ret = -1;
	ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(on));
	return ret;
}

int acceptor::add_input_fd(int fd, int efd) {
	epoll_event event;
	event.events = EPOLLIN | EPOLLET;
	event.data.fd = fd;
	int ret = epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event);
	if (ret < 0) {
		_ERROR("add_input_fd err,fd=%d,errno=%d", fd, errno);
	}
	return ret;
}

int acceptor::del_input_fd(int fd, int efd) {
	epoll_event event;
	event.events = EPOLLIN | EPOLLET;
	event.data.fd = fd;
	int ret = epoll_ctl(efd, EPOLL_CTL_DEL, fd, &event);
	if (ret < 0) {
		_ERROR("del_input_fd err,fd=%d,errno=%d", fd, errno);
	}
	return ret;
}


uint32_t acceptor::genWid() {
	return __sync_fetch_and_add(&wid, add_sum);
}

int acceptor::create_pipe(int idx) {
	int options;
	int pipe_fd[2];
	
	if (pipe(pipe_fd)) {
		return -1;
	}

	pipe_read_fds[idx] = pipe_fd[0];
	pipe_write_fds[idx] = pipe_fd[1];

	for (int i = 0;i < 2;i++) {
		if ((options = fcntl(pipe_fd[i], F_GETFL)) == -1) {
			return -1;
		}
		if (fcntl(pipe_fd[i], F_SETFL, options | O_NONBLOCK) == -1) {
			return -1;
		}
	}
	return 0;
}
