#ifndef DOUBLY_LINKED_LIST_H
#define DOUBLY_LINKED_LIST_H

#include <stdlib.h>

typedef struct doubly_linked_node_t {
  struct doubly_linked_node_t *next;
  struct doubly_linked_node_t *prev;
  void *data;
} doubly_linked_node_t;

typedef struct doubly_linked_list_t {
  doubly_linked_node_t *head;
  doubly_linked_node_t *tail;
  int size;
} doubly_linked_list_t;

doubly_linked_list_t *doubly_linked_list_new();
void doubly_linked_list_insert_beginning(doubly_linked_list_t *list,
                                         doubly_linked_node_t *new_node);
void doubly_linked_list_insert_end(doubly_linked_list_t *list,
                                   doubly_linked_node_t *new_node);

void doubly_linked_list_remove(doubly_linked_list_t *list,
                               doubly_linked_node_t *node);

#endif // DOUBLY_LINKED_LIST_H
