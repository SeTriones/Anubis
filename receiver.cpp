#include "receiver.hpp"

receiver::receiver(int _epoll_cnt, int _max_conn) : acceptor(_epoll_cnt, _max_conn) {
}

receiver::~receiver() {
}

int receiver::open() {
	return 0;
}

int receiver::stop() {
	return 0;
}

int receiver::svc() {
	return 0;
}

int receiver::start_listen() {
	return 0;
}
