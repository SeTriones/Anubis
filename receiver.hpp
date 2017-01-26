#ifndef _RECEIVER_HPP_
#define _RECEIVER_HPP_

#include "task_base.hpp"
#include "acceptor.hpp"
#include "base_nio_handler.hpp"
#include "key_group.hpp"

class receiver : public task_base, public base_nio_handler, public acceptor {
public:
	receiver(int epoll_cnt = 4, int max_conn = 2048, int key_group_cnt = 16, int group_capacity = 100000);
	~receiver();
	virtual int open(int thread_cnt);
	virtual int stop();
	virtual int svc();
	int start_listen();

private:
	int handle_request(int fd, int epoll_fd, char* buf);
	int key_group_cnt;
	int group_capacity;
	KeyGroup** key_groups;
};

#endif
