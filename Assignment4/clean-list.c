#include <stdlib.h>
#include "clean-list.h"

void list_enqueue(List *list, void *data){
    Node *node = malloc(sizeof(Node));
    node->data = data;
    if (list->head == NULL) {
        list->head = node;
    } else {
        list->tail->next = node;
    }
    list->tail = node;
}

void *list_dequeue(List *list) {
    if (list->head == NULL) {
        return NULL;
    }
    Node *node = list->head;
    void *data = node->data;
    list->head = list->head->next;
    free(node);
    return data;
}