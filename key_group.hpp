#ifndef _KEY_GROUP_
#define _KEY_GROUP_

#include <pthread.h>
#include <list>
#include <stdint.h>
#include "lru_cache.hpp"

class KeyGroup {
public:
	KeyGroup(int capacity);
	~KeyGroup();
	int Inc(uint64_t key);
	void dump();

private:
	int capacity;
	int* store;
	LRUCache* cache;
	std::list<int> free_poses;
	pthread_mutex_t group_mutex;
};

#endif
