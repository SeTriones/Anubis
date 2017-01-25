#ifndef DOUBLE_LINKED_LIST
#define DOUBLE_LINKED_LIST
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef struct __node node;

struct __node {
	uint64_t key;
	node* prev;
	node* next;
};

typedef struct {
	node* header;
	node* tail;
	int node_cnt;
} double_linked_list;

#ifdef __cplusplus
extern "C"
{
#endif

double_linked_list* init_list();
void destroy_list(double_linked_list* lst);
node* new_node();
void mv_node_to_header(double_linked_list* lst, node* n, int is_new_node);
int rm_node_from_tail(double_linked_list* lst, int cnt, uint64_t* keys);
int push_from_header(double_linked_list* lst, node* n);
int push_from_tail(double_linked_list* lst, node* n);
node* pop_from_header(double_linked_list* lst);
node* pop_from_tail(double_linked_list* lst);

#ifdef __cplusplus
}
#endif

#endif
