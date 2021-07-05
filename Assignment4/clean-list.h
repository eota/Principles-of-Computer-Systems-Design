#ifndef CLEAN_LIST_H
#define CLEAN_LIST_H

// Source: Lecture 14
typedef struct node_t {
    void *data;
    struct node_t *next;
} Node;

typedef struct {
    Node *head;
    Node *tail;
} List;

/* linked list */
void list_enqueue(List *list, void *data);
void *list_dequeue(List *list);

#endif