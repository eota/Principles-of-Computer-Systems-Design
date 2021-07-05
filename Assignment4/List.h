#ifndef _LIST_H
#define _LIST_H

#include "simulator.h"
#include <stdbool.h>

// Source: https://github.com/dxtr/list
typedef struct list_node {
	void *data;
	thread_t *thread;
	struct list_node *next;
	// time variables
	int arival;
  	int finish;
  	int first_cpu_burst_start;
  	int second_cpu_burst_start;
  	int first_cpu_burst_end;
  	int io_burst_start;
  	int io_burst_end;

} list_node;

/* linked list */
list_node* list_create(thread_t *thread);
void list_destroy(list_node **list);
list_node* list_insert_after(list_node *node, thread_t *thread);
list_node* list_insert_beginning(list_node *list, thread_t *thread);
list_node* list_insert_end(list_node *list, thread_t *thread);
void list_remove(list_node **list, list_node *node);
void list_remove_by_data(list_node **list, thread_t *thread);
list_node* list_find_node(list_node *list, list_node *node);
list_node* list_find_by_data(list_node *list, thread_t *thread);
list_node* list_find(list_node *list, int(*func)(list_node*,void*), thread_t *thread);

// add list_empty
// add list_front
// fetch thread via tid
bool is_empty(list_node *list);
list_node* list_front(list_node *list);
int list_size(list_node *list);

#endif