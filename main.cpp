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
#include "service_log.hpp"
#include "key_group.hpp"

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
	_INFO("Anubis starts");
	return 0;
}

