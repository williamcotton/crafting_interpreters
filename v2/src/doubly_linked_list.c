#include "doubly_linked_list.h"

doubly_linked_list_t *doubly_linked_list_new() {
  doubly_linked_list_t *list = malloc(sizeof(doubly_linked_list_t));
  list->head = NULL;
  list->tail = NULL;
  list->size = 0;
  return list;
}

void doubly_linked_list_insert_before(doubly_linked_list_t *list,
                                      doubly_linked_node_t *node,
                                      doubly_linked_node_t *new_node) {
  if (list->size == 0) {
    list->head = new_node;
    list->tail = new_node;
    new_node->next = NULL;
    new_node->prev = NULL;
  } else {
    new_node->next = node;
    new_node->prev = node->prev;
    if (node->prev != NULL) {
      node->prev->next = new_node;
    } else {
      list->head = new_node;
    }
    node->prev = new_node;
  }
  list->size++;
}

void doubly_linked_list_insert_after(doubly_linked_list_t *list,
                                     doubly_linked_node_t *node,
                                     doubly_linked_node_t *new_node) {
  if (list->size == 0) {
    list->head = new_node;
    list->tail = new_node;
    new_node->next = NULL;
    new_node->prev = NULL;
  } else {
    new_node->prev = node;
    new_node->next = node->next;
    if (node->next != NULL) {
      node->next->prev = new_node;
    } else {
      list->tail = new_node;
    }
    node->next = new_node;
  }
  list->size++;
}

void doubly_linked_list_insert_beginning(doubly_linked_list_t *list,
                                         doubly_linked_node_t *new_node) {
  if (list->size == 0) {
    list->head = new_node;
    list->tail = new_node;
    list->size++;
  } else {
    doubly_linked_list_insert_before(list, list->head, new_node);
  }
}

void doubly_linked_list_insert_end(doubly_linked_list_t *list,
                                   doubly_linked_node_t *new_node) {
  if (list->size == 0) {
    doubly_linked_list_insert_beginning(list, new_node);
  } else {
    doubly_linked_list_insert_after(list, list->tail, new_node);
  }
}

void doubly_linked_list_remove(doubly_linked_list_t *list,
                               doubly_linked_node_t *node) {
  if (list->size == 0) {
    return;
  } else if (list->size == 1) {
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
  } else if (node == list->head) {
    list->head = node->next;
    list->head->prev = NULL;
    list->size--;
  } else if (node == list->tail) {
    list->tail = node->prev;
    list->tail->next = NULL;
    list->size--;
  } else {
    node->prev->next = node->next;
    node->next->prev = node->prev;
    list->size--;
  }
}
