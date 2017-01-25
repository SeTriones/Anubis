#include "double_linked_list.h"

double_linked_list* init_list() {
	double_linked_list* ret = (double_linked_list*)malloc(sizeof(double_linked_list));
	if (!ret) {
		return NULL;
	}
	ret->header = NULL;
	ret->tail = ret->header;
	ret->node_cnt = 0;
	return ret;
}

void destroy_list(double_linked_list* lst) {
	if (lst == NULL) {
		return;
	}
	node* n = lst->header;
	node* cur;
	while (n) {
		cur = n;
		n = n->next;
		free(cur);
	}
	free(lst);
	return;
}

node* new_node() {
	node* ret = (node*)malloc(sizeof(node));
	ret->prev = NULL;
	ret->next = NULL;
	ret->key = 0;
	return ret;
}

void mv_node_to_header(double_linked_list* lst, node* n, int is_new_node) {
	node* next = NULL;
	node* prev = NULL;
	if (is_new_node == 0) {
		if (n == lst->header) {
			return;
		}
		prev = n->prev;
		next = n->next;
		n->prev->next = next;
		if (n->next) {
			n->next->prev= prev;
		}
		if (n == lst->tail) {
			lst->tail = prev;
		}
	}
	else {
		if (1 == ++lst->node_cnt) {
			lst->tail = n;
		}
	}
	n->prev = NULL;
	n->next = lst->header;
	if (lst->header) {
		lst->header->prev = n;
	}
	lst->header = n;
	return;	
}

int rm_node_from_tail(double_linked_list* lst, int cnt, uint64_t* keys) {
	node* n = lst->tail;
	node* cur;
	int ret = 0;
	keys = (uint64_t*)malloc(sizeof(uint64_t) * cnt);
	while (n && cnt > 0) {
		cur = n;
		n = n->prev;

		keys[ret] = cur->key;
		free(cur);

		cnt--;
		lst->node_cnt--;
		ret++;
	}
	lst->tail = n;
	if (!n) {
		lst->header = NULL;
	}
	return ret; 
}

int push_from_header(double_linked_list* lst, node* n) {
	n->next = lst->header;
	n->prev = NULL;
	if (lst->header) {
		lst->header->prev = n;
	}
	lst->header = n;
	return ++lst->node_cnt;
}

int push_from_tail(double_linked_list* lst, node* n) {
	if (lst->tail) {
		lst->tail->next = n;
		n->prev = lst->tail;
		lst->tail = n;
		return ++lst->node_cnt;
	}
	else {
		return push_from_header(lst, n);
	}
}

node* pop_from_header(double_linked_list* lst) {
	if (!lst->header) {
		return NULL;
	}
	node* n = lst->header;
	lst->header = n->next;
	if (lst->header) {
		lst->header->prev = NULL;
	} else {
		lst->tail = NULL;
	}
	--lst->node_cnt;
	return n;
}

node* pop_from_tail(double_linked_list* lst) {
	if (!lst->tail) {
		return NULL;
	}
	node* n = lst->tail;
	lst->tail = n->prev;
	if (lst->tail) {
		lst->tail->next = NULL;
	} else {
		lst->header = NULL;
	}
	--lst->node_cnt;
	return n;
}
