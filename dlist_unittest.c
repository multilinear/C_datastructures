// Unittest for dlist (doubly linked list)
//
// Copyright:
//   Matthew Brewer (mbrewer@smalladventures.net)
//   2009-09-22


#include <stdio.h>
#include "assert.h"
#include "dlist.h"

typedef struct {
  dlist_node_t list_data;
  int data;
} mynode_t;

DEFINE_DLIST(mynode_t, list_data)
  
dlist_mynode_t list;

void* print_node(mynode_t *n, void *last, char* term) {
  printf("%d ", n->data);
  return 0;
}

void* is_5(mynode_t *n, void *last, char* term) {
  if (n->data == 5) {
    // This short-circuits
    *term = 1;
    return n;
  }
  // doesn't matter what we return - lets return something we'll notice
  // in case short circuit is borked
  return 0;
}

void* is_16(mynode_t *n, void *last, char* term) {
  if (n->data == 16) {
    // This short-circuits
    *term = 1;
    return n;
  } else {
    // doesn't matter what we return - lets return something we'll notice
    // in case short circuit is borked
    return (void*) 0xdeadbeef;
  }
}

void* is_38(mynode_t *n, void *last, char* term) {
  if (n->data == 38) {
    // This short-circuits
    *term = 1;
    return n;
  } else {
    // doesn't matter what we return - lets return something we'll notice
    // in case short circuit is borked
    return (void*) 0xdeadbeef;
  }
}

void print_list(dlist_mynode_t *list) {
  printf("flist = [");
  dlist_mynode_t_foldl(list, print_node, 0);
  printf("]\n");
  printf("blist = [");
  dlist_mynode_t_foldr(list, print_node, 0);
  printf("]\n");
}

int main(unsigned int argc, char **argv) {
  mynode_t *n;

  printf("initializing list\n");
  dlist_mynode_t_init(&list);

  printf("test base cases\n");
  n = malloc(sizeof(mynode_t));
  n->data = 1;
  printf("pushback\n");
  dlist_mynode_t_pushback(&list, n);
  printf("remove\n");
  dlist_mynode_t_remove(&list, n);
  printf("enqueue\n");
  dlist_mynode_t_enqueue(&list, n);
  printf("print\n");
  print_list(&list);
  printf("pop\n");
  n = dlist_mynode_t_pop(&list);
  printf("push\n");
  dlist_mynode_t_push(&list, n);
  printf("dequeue\n");
  n = dlist_mynode_t_dequeue(&list);
  free(n);
  printf("destroy\n");
  dlist_mynode_t_destroy(&list);
  printf("initializing list\n");
  dlist_mynode_t_init(&list);

  // Test basic insertion

  printf("inserting elements\n");
  int x;
  for (x = 0; x<20; x++) {
    n = malloc(sizeof(mynode_t));
    n->data = x;
    dlist_mynode_t_enqueue(&list, n);
  }
  dlist_mynode_t_check(&list);

  print_list(&list);

  // make sure pop/push happen in the right places, and don't break the list 
  n = dlist_mynode_t_head(&list);
  printf("head is %d\n", n->data);
  assert(n->data == 19);
  dlist_mynode_t_check(&list);

  n = dlist_mynode_t_pop(&list);
  printf("head was %d\n", n->data);
  assert(n->data == 19);
  dlist_mynode_t_check(&list);
  free(n);

  n = dlist_mynode_t_tail(&list);
  printf("tail is %d\n", n->data);
  assert(n->data == 0);
  dlist_mynode_t_check(&list);

  n = dlist_mynode_t_dequeue(&list);
  printf("tail was %d\n", n->data);
  assert(n->data == 0);
  dlist_mynode_t_check(&list);
  free(n);
  
  dlist_mynode_t_check(&list);

  for (x = 20; x<40; x++) {
    n = malloc(sizeof(mynode_t));
    n->data = x;
    dlist_mynode_t_pushback(&list, n);
  }

  print_list(&list);

  n = dlist_mynode_t_pop(&list);
  printf("head was %d\n", n->data);
  assert(n->data == 18);
  dlist_mynode_t_check(&list);
  free(n);

  n = dlist_mynode_t_dequeue(&list);
  printf("tail was %d\n", n->data);
  assert(n->data == 39);
  dlist_mynode_t_check(&list);
  free(n);
 
  // find "5" in the list, and remove it
  printf("find and remove 5 from the list\n");
  n = dlist_mynode_t_foldl(&list, is_5, 0);
  dlist_mynode_t_remove(&list, n);
  free(n);

  dlist_mynode_t_check(&list);

  // find "5" in the list, and remove it
  printf("find 5 again - should be gone\n");
  n = dlist_mynode_t_foldl(&list, is_5, 0);
  assert(!n);
  dlist_mynode_t_check(&list);

  // find "16" in the list, and remove it, first element
  printf("find 16 and remove\n");
  n = dlist_mynode_t_foldl(&list, is_16, 0);
  dlist_mynode_t_remove(&list, n);
  free(n);
  
  // find "38" in the list, and remove it, last element
  printf("find 38 and remove\n");
  n = dlist_mynode_t_foldl(&list, is_38, 0);
  dlist_mynode_t_remove(&list, n);
  free(n);

  n = dlist_mynode_t_pop(&list);
  printf("head was %d\n", n->data);
  assert(n->data == 17);
  dlist_mynode_t_check(&list);
  free(n);

  n = dlist_mynode_t_dequeue(&list);
  printf("tail was %d\n", n->data);
  assert(n->data == 37);
  dlist_mynode_t_check(&list);
  free(n);

  print_list(&list);

  printf("PASSED!\n");
}
