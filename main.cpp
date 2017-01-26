//
//////                       _oo0oo_
//////                      o8888888o
//////                      88" . "88
//////                      (| -_- |)
//////                      0\  =  /0
//////                    ___/`---'\___
//////                  .' \\|     |// '.
//////                 / \\|||  :  |||// \
//////                / _||||| -:- |||||- \
//////               |   | \\\  -  /// |   |
//////               | \_|  ''\---/''  |_/ |
//////               \  .-\__  '-'  ___/-. /
//////             ___'. .'  /--.--\  `. .'___
//////          ."" '<  `.___\_<|>_/___.' >' "".
//////         | | :  `- \`.;`\ _ /`;.`/ - ` : | |
//////         \  \ `_.   \_ __\ /__ _/   .-` /  /
//////     =====`-.____`.___ \_____/___.-`___.-'=====
//////                       `=---='
//////
//////
//////     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//////
//////               佛祖保佑         永无BUG
//////
//////
//////
////
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>
#include "service_log.hpp"
#include "key_group.hpp"
#include "receiver.hpp"

void sigterm_handler(int signo){}
void sigint_handler(int signo){}

void testKeyGroup() {
	KeyGroup* kg = new KeyGroup(10);
	kg->dump();
	int cnt;
	for (int i = 0; i < 10; i++) {
		for (int inc_cnt = 0; inc_cnt <= i; inc_cnt++) {
			cnt = kg->Inc((uint64_t)i);
			_INFO("inc %d,get %d", i, cnt);
		}
	}
	for (int i = 0; i < 100; i++) {
		kg->Inc(1);
		kg->Inc(2);
		kg->Inc(3);
		kg->Inc(4);
	}
	kg->dump();
	for (int i = 0; i < 5; i++) {
		for (int inc_cnt = 0; inc_cnt < i + 10; inc_cnt++) {
			int tmp = i + 20;
			cnt = kg->Inc((uint64_t)tmp);
			_INFO("inc %d,get %d", tmp, cnt);
		}
	}
	kg->dump();
	return;
}

int main(int argc, char* argv[]) {
	_INFO("Anubis starting");

	close(STDIN_FILENO);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGTERM, &sigterm_handler);
	signal(SIGINT, &sigint_handler);

	receiver* r = new receiver(4, 20480, 2, 100000);
	r->open(8);
	r->start_listen(4444);
	r->activate();
	_INFO("Anubis started");
	pause();
	_INFO("Anubis stopping");
	r->stop();
	delete r;
	_INFO("Anubis stoppped");
	return 0;
}

