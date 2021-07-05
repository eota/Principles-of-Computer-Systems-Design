#include <stdlib.h>
#include <string.h>
#include "List.h"
#include <stdbool.h>

// Source: https://github.com/dxtr/list

/* Creates a list (node) and returns it
 * Arguments: The data the list will contain or NULL to create an empty
 * list/node
 */
list_node* list_create(thread_t *thread)
{
	list_node *l = malloc(sizeof(list_node));
	if (l != NULL) {
		l->next = NULL;
		l->thread = thread;
	}

	return l;
}

/* Completely destroys a list
 * Arguments: A pointer to a pointer to a list
 */
void list_destroy(list_node **list)
{
	if (list == NULL) return;
	while (*list != NULL) {
		list_remove(list, *list);
	}
}

/* Creates a list node and inserts it after the specified node
 * Arguments: A node to insert after and the data the new node will contain
 */
list_node* list_insert_after(list_node *node, thread_t *thread)
{
	list_node *new_node = list_create(thread);
	if (new_node) {
		new_node->next = node->next;
		node->next = new_node;
	}
	return new_node;
}

/* Creates a new list node and inserts it in the beginning of the list
 * Arguments: The list the node will be inserted to and the data the node will
 * contain
 */
list_node* list_insert_beginning(list_node *list, thread_t *thread)
{
	list_node *new_node = list_create(thread);
	if (new_node != NULL) { new_node->next = list; }
	return new_node;
}

/* Creates a new list node and inserts it at the end of the list
 * Arguments: The list the node will be inserted to and the data the node will
 * contain
 */
list_node* list_insert_end(list_node *list, thread_t *thread)
{
	list_node *new_node = list_create(thread);
	if (new_node != NULL) {
		for(list_node *it = list; it != NULL; it = it->next) {
			if (it->next == NULL) {
				it->next = new_node;
				break;
			}
		}
	}
	return new_node;
}

/* Removes a node from the list
 * Arguments: The list and the node that will be removed
 */
void list_remove(list_node **list, list_node *node)
{
	list_node *tmp = NULL;
	if (list == NULL || *list == NULL || node == NULL) return;

	if (*list == node) {
		*list = (*list)->next;
		free(node);
		node = NULL;
	} else {
		tmp = *list;
		while (tmp->next && tmp->next != node) tmp = tmp->next;
		if (tmp->next) {
			tmp->next = node->next;
			// free(node); // Commented out to fix strange bug
			node = NULL;
		}
	}
}

/* Removes an element from a list by comparing the data pointers
 * Arguments: A pointer to a pointer to a list and the pointer to the data
 */
void list_remove_by_data(list_node **list, thread_t *thread)
{
	if (list == NULL || *list == NULL || thread == NULL) return;
	list_remove(list, list_find_by_data(*list, thread));
}

/* Find an element in a list by the pointer to the element
 * Arguments: A pointer to a list and a pointer to the node/element
 */
list_node* list_find_node(list_node *list, list_node *node)
{
	while (list) {
		if (list == node) break;
		list = list->next;
	}
	return list;
}

/* Finds an elemt in a list by the data pointer
 * Arguments: A pointer to a list and a pointer to the data
 */
list_node* list_find_by_data(list_node *list, thread_t *thread)
{
	while (list) {
		if (list->thread == thread) break;
		list = list->next;
	}
	return list;
}

/* Finds an element in the list by using the comparison function
 * Arguments: A pointer to a list, the comparison function and a pointer to the
 * data
 */
list_node* list_find(list_node *list, int(*func)(list_node*,void*), thread_t *thread)
{
	if (!func) return NULL;
	while(list) {
		if (func(list, thread)) break;
		list = list->next;
	}
	return list;
}

/*
 * Returns true if the list is empty, false otherwise
 */
bool is_empty(list_node *list) {
	if (!list) {
		return true;
	}
	return false;
}

/*
 * get front node of list
 */
list_node* list_front(list_node *list) {
	while (list) {
		if (list->next == NULL) {
			break;
		}
		list = list->next;
	}
	return list;
}

/*
 * List length/size of function
 * Source: https://stackoverflow.com/questions/2389904/total-size-of-a-linked-list-in-c
 */
 int list_size(list_node *list) {
	 list_node* cur = list;
	 int size = 0;

	 while (cur != NULL) {
		cur = cur->next;
		size++;
	 }
	 return size;
 }