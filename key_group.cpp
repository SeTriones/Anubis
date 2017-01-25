#include "key_group.hpp"
#include "service_log.hpp"
#include <string.h>

KeyGroup::KeyGroup(int _capacity) {
	capacity = _capacity;
	cache = new LRUCache(capacity);
	store = (int*)malloc(sizeof(int) * capacity);
	memset(store, 0, sizeof(int) * capacity);
	for (int i = 0; i < capacity; i++) {
		free_poses.push_back(i);
	}
	pthread_mutex_init(&group_mutex, NULL);
}

KeyGroup::~KeyGroup() {
	delete cache;
	free(store);
	pthread_mutex_destroy(&group_mutex);
}

int KeyGroup::Inc(uint64_t key) {
	int ret;
	pthread_mutex_lock(&group_mutex);
	int* pos = cache->get(key);
	if (pos == NULL) {
		if (free_poses.empty()) {
			_INFO("no free pos");
			pos = NULL;
		} else {
			int idx = free_poses.front();
			free_poses.pop_front();
			pos = store + idx;
		}
		_INFO("cache set");
		pos = cache->set(key, pos, 0);
		*pos = 1;
		pthread_mutex_unlock(&group_mutex);
		return 1;
	} else {
		pthread_mutex_unlock(&group_mutex);
		ret = __sync_add_and_fetch(pos, 1);
		return ret;
	}

}

void KeyGroup::dump() {
	for (int i = 0; i < capacity; i++) {
		_INFO("pos[%d]=%d", i, store[i]);
	}
}
