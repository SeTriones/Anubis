#include "lru_cache.hpp"
#include "service_log.hpp"

LRUCache::LRUCache(int capacity):capacity(capacity){
	size = 0;
	list = init_list();
	init_list();
}

LRUCache::~LRUCache() {
	destroy_list(list);
}

int* LRUCache::get(uint64_t key){
	std::tr1::unordered_map<uint64_t, struct HashItem>::iterator it;
	it = hash.find(key);
	if (it == hash.end()) {
		return NULL;
	}
	mv_node_to_header(list, it->second.node, 0);
	return it->second.pos;
}

int* LRUCache::set(uint64_t key, int* pos, int overwrite){
	std::tr1::unordered_map<uint64_t, struct HashItem>::iterator it;
	it = hash.find(key);
	if (it != hash.end()) {
		if(overwrite) {
			it->second.pos = pos;
			mv_node_to_header(list, it->second.node, 0);
		}
		else {
			mv_node_to_header(list, it->second.node, 0);
		}
		return pos;
	}
	int* free_pos = pos;
	if (size == capacity) {
		node *n = pop_from_tail(list);
		it = hash.find(n->key);
		free_pos = it->second.pos;
		hash.erase(it);
		free(n);
		size--;
	}
	node *tmp = new_node();
	tmp->key = key;
	mv_node_to_header(list, tmp, 1);
	HashItem item;
	item.node = tmp;
	item.pos = free_pos;
	hash.insert(std::pair<uint64_t, struct HashItem>(key, item));
	size++;
	return free_pos;
}
