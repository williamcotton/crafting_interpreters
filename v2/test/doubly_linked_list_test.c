#include <Block.h>
#include <doubly_linked_list.h>
#include <tape/tape.h>

int main() {
  tape_t *test = tape();

  int testStatus = test->test("doubly linked list", ^(tape_t *t) {
    t->clearState();

    doubly_linked_list_t *list = doubly_linked_list_new();

    t->ok("list is not null", list != NULL);

    t->ok("list is empty", list->size == 0);

    t->ok("list is empty", list->head == NULL);

    t->ok("list is empty", list->tail == NULL);

    doubly_linked_node_t *first_new_node = malloc(sizeof(doubly_linked_node_t));

    first_new_node->data = "hello";

    doubly_linked_list_insert_beginning(list, first_new_node);

    t->ok("list has 1 element", list->size == 1);
    t->ok("list head is not null", list->head != NULL);
    t->ok("list tail is not null", list->tail != NULL);
    t->ok("list tail is the list head", list->head == list->tail);
    t->strEqual("list head has the correct data", string(list->head->data),
                "hello");
    t->ok("list head next is null", list->head->next == NULL);
    t->ok("list head prev is null", list->head->prev == NULL);

    doubly_linked_node_t *second_new_node =
        malloc(sizeof(doubly_linked_node_t));

    second_new_node->data = "world";

    doubly_linked_list_insert_beginning(list, second_new_node);

    t->ok("list has 2 elements", list->size == 2);

    t->strEqual("list head has the correct data", string(list->head->data),
                "world");
    t->ok("list head next is not null", list->head->next != NULL);
    t->ok("list head prev is null", list->head->prev == NULL);
    t->ok("list head next is the list tail", list->head->next == list->tail);
    t->ok("list head next prev is the list head",
          list->head->next->prev == list->head);
    t->ok("list tail next is null", list->tail->next == NULL);
    t->ok("list tail prev is the list head", list->tail->prev == list->head);
    t->strEqual("list tail has the correct data", string(list->tail->data),
                "hello");

    doubly_linked_node_t *third_new_node = malloc(sizeof(doubly_linked_node_t));

    third_new_node->data = "again";

    doubly_linked_list_insert_end(list, third_new_node);

    t->ok("list has 3 elements", list->size == 3);
    t->strEqual("list tail has the correct data", string(list->tail->data),
                "again");

    doubly_linked_list_remove(list, first_new_node);

    t->ok("list has 2 elements", list->size == 2);
    t->strEqual("list head has the correct data", string(list->head->data),
                "world");
    t->ok("list head next is not null", list->head->next != NULL);
    t->ok("list head prev is null", list->head->prev == NULL);
    t->ok("list head next is the list tail", list->head->next == list->tail);
    t->ok("list head next prev is the list head",
          list->head->next->prev == list->head);
    t->ok("list tail next is null", list->tail->next == NULL);
    t->ok("list tail prev is the list head", list->tail->prev == list->head);
    t->strEqual("list tail has the correct data", string(list->tail->data),
                "again");

    free(list);
  });

  exit(testStatus);
}