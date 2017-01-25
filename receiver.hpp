#ifndef _RECEIVER_HPP_
#define _RECEIVER_HPP_

#include "task_base.hpp"
#include "acceptor.hpp"
#include "base_nio_handler.hpp"

class receiver : public task_base, public base_nio_handler, public acceptor {
public:
	receiver(int epoll_cnt = 4, int max_conn = 2048);
	~receiver();
	virtual int open();
	virtual int stop();
	virtual int svc();
	int start_listen();
};

#endif
