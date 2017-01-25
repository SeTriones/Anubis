#include <tr1/unordered_map>
#include <pthread.h>
#include "double_linked_list.h"

struct HashItem{
	__node *node;
	int* pos;
};

class LRUCache{
public:
	LRUCache(int capacity);
	~LRUCache();

	int* get(uint64_t key);

	int* set(uint64_t key, int* pos, int overwrite=0);

private:

	int size;
	int capacity;
	double_linked_list *list;
	std::tr1::unordered_map<uint64_t, struct HashItem> hash;
};
