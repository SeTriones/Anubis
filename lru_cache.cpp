#include "lru_cache.hpp"

LRUCache::LRUCache(int capacity):capacity(capacity){
	size = 0;
	list = (double_linked_list*)malloc(sizeof(double_linked_list));
	init_list();
	pthread_mutex_init(&cache_mutex, NULL);
}

LRUCache::~LRUCache() {
	destroy_list(list);
	pthread_mutex_destroy(&cache_mutex);
}

int* LRUCache::get(uint64_t key){
	pthread_mutex_lock(&cache_mutex);
	it = hash.find(key);
	if (it == hash.end()) {
		pthread_mutex_unlock(&cache_mutex);
		return NULL;
	}
	mv_node_to_header(list, it->second.node, 0);
	pthread_mutex_unlock(&cache_mutex);
	return it->second.pos;
}

void LRUCache::set(uint64_t key, int* pos, int overwrite){
	pthread_mutex_lock(&cache_mutex);
	it = hash.find(key);
	if(it != hash.end()) {
		if(overwrite) {
			it->second.pos = pos;
			mv_node_to_header(list, it->second.node, 0);
		}
		else {
			mv_node_to_header(list, it->second.node, 0);
		}
		pthread_mutex_lock(&cache_mutex);
		return;
	}
	int* free_pos = NULL;
	if(size == capacity) {
		node *n = pop_from_tail(list);
		it = hash.find(n->key);
		free_pos = it->second.pos;
		hash.erase(it);
		free(n);
	}
	node *tmp = new_node();
	tmp->key = key;
	tmp->next = NULL;
	tmp->prev = NULL;
	mv_node_to_header(list, tmp, 1);
	HashItem item;
	item.node = tmp;
	item.pos = free_pos;
	hash.insert(std::pair<int, struct HashItem>(key, item));
	size++;
}
