// Generic doubley-linked link-list implementation
//
// Copyright:
//   Matthew Brewer (mbrewer@smalladventures.net)
//   2009-09-22
//
// Usage:
//   The user should
//   1) include this header
//   2) declare a "node" type, with a "dlist_node_t" as a member
//   3) call "DEFINE_DLIST" with their node-type, and the member name
//   4) "DEFINE_DLIST" will define a set of dlist functions over the users
//     node-type
//   5) The user must allocate a "dlist_t", to store the list, and call
//      dlist_init() on it.
//   6) The user must allocate all nodes before passing them in
//   7) When done with the list user must call "dlist_destroy" on the list head
// 
//   See dlist_unittest.cc for example usage.
//
// Threadsafety:
//   Not threadhostile, not threadsafe
//   This datastructure includes no mutexing at all, and should be mutexed
//   externally if locking is desired.
//   This datastructure also never calls malloc - so it never calls any mutex.
//
// Usage Notes:
//   This datastructure never calls malloc. Making it realtime-safe.
//
// Design Decisions:
//   * Everything is in the header - so it can be inlined by the compiler
//   * Where possible functions are shared by all types, this saves icache
//     if functions are not inlined
//   * Macros will write a typesafe interface for a given type, these functions
//     being trivial will always get inlined - creating no overhead
//   * Lists are of only one type, this is considered a feature, not a bug. If
//     dynamic types are desired, as long as the "dlist_node_t" is at the same
//     offset in the struct, it will work (I suggest the first element).
//   * All arithmatic is done in macros - so offsets are computed statically
//     this saves us double-variable adds.
//   * If a standard "allocating" linklist is desired, simply wrap this with a 
//     trivial interface layer, you'll incure no overhead
//   * Note that "foldr" and "foldl" incure a jump indirect on every iteration
//   * It was decided that having foldr and foldl be cleanly abstract was more
//     important than the icache - another advantage is fast offset calculation
//     (since an offset must be computed every iteration)

#include <assert.h>
#include "offset.h"
#include "panic.h"

#ifndef DLIST_H
#define DLIST_H

// ******************* typedefs ****************

// User should include this as a field in their node struct
typedef struct dlist_node_struct {
  struct dlist_node_struct *next;
  struct dlist_node_struct *prev;
} dlist_node_t;

// User should use this type to store the list
typedef struct {
  dlist_node_t *head;
  dlist_node_t *tail;
} dlist_t;

// We define a *new* struct that's identical to the original
// Struct types are generative, so this gives us typechecking on the listtype.
// We can then simply perform a cast to call our backend functions, since
// we know the two types are actually the same.
#define DEFINE_DLIST(type, metaname)  \
  typedef struct {  \
    dlist_node_t *head;  \
    dlist_node_t *tail;  \
  } dlist_##type;  \
  void dlist_##type##_init(dlist_##type *root) {  \
    dlist_init((dlist_t*) root);  \
  }  \
  void dlist_##type##_destroy(dlist_##type *root) {  \
    dlist_destroy((dlist_t*) root);  \
  }  \
  void dlist_##type##_check(const dlist_##type *root) { \
    dlist_check((const dlist_t *) root); \
  }  \
  void dlist_##type##_enqueue(dlist_##type *root, type *data) {  \
    dlist_enqueue((dlist_t*) root, &(data->metaname));  \
  }  \
  void dlist_##type##_pushback(dlist_##type *root, type *data) {  \
    dlist_pushback((dlist_t*) root, &(data->metaname)); \
  }  \
  void dlist_##type##_push(dlist_##type *root, type *data) {  \
    dlist_push((dlist_t*) root, &(data->metaname));  \
  }  \
  type * dlist_##type##_dequeue(dlist_##type *root) {  \
    return GET_CONTAINER(dlist_dequeue((dlist_t*) root), type, metaname);  \
  }  \
  type * dlist_##type##_pop(dlist_##type *root) {  \
    return GET_CONTAINER(dlist_pop((dlist_t*) root), type, metaname);  \
  }  \
  dlist_##type##_remove(dlist_##type *root, type *data) {  \
    dlist_remove((dlist_t*) root, &(data->metaname));  \
  }  \
  type * dlist_##type##_head(const dlist_##type *root){  \
    return GET_CONTAINER(dlist_head((dlist_t*) root), type, metaname);  \
  }  \
  type * dlist_##type##_tail(const dlist_##type *root){  \
    return GET_CONTAINER(dlist_tail((dlist_t*) root), type, metaname);  \
  }  \
  void * dlist_##type##_foldr(  \
      const dlist_##type *root,  \
      void *(*func)(type*, void*, char*),  \
      void *init) {  \
    dlist_node_t *ptr;  \
    void* result = init;  \
    for (ptr = root->head; ptr; ptr = ptr->next) {  \
      char terminate = 0;  \
      result = (*func)(GET_CONTAINER(ptr, type, metaname), result, &terminate);  \
      if (terminate)  \
        break;  \
    }  \
    return result;  \
  }  \
  void * dlist_##type##_foldl(  \
      const dlist_##type *root,  \
      void *(*func)(type*, void*, char*),  \
      void *init) {  \
    dlist_node_t *ptr;  \
    void* result = init;  \
    for (ptr = root->tail; ptr; ptr = ptr->prev) {  \
      char terminate = 0;  \
      result = (*func)(GET_CONTAINER(ptr, type, metaname), result, &terminate);  \
      if (terminate)  \
        break;  \
    }  \
    return result;  \
  } \


// ******************* private functions ****************

void dlist_init(dlist_t *root) {
  root->head = NULL;
  root->tail = NULL;
}

void dlist_destroy(dlist_t *root) {
  if(root->head)
    panic("dlist_destroy: root->head is non-null\n");
  if(root->tail)
    panic("dlist_destroy: root->head is non-null\n");
  // Drop some magic, so we notice if it gets used again without initialization
  root->head = (dlist_node_t*) 0xdeadbeef;
  root->tail = (dlist_node_t*) 0xdeadbeef;
}

void dlist_enqueue(dlist_t *root, dlist_node_t *data) {
  data->prev = NULL;
  dlist_node_t *old_head = root->head;
  data->next = old_head;

  if (!old_head) {
    assert(!root->tail);
    root->tail = data;
  } else {
    assert(!old_head->prev);
    old_head->prev = data;
  }
  root->head = data;
}

void dlist_pushback(dlist_t *root, dlist_node_t *data) {
  data->next = NULL;
  dlist_node_t *old_tail = root->tail;
  data->prev = old_tail;

  if (!old_tail) {
    assert(!root->head);
    root->head = data;
  } else {
    assert(!old_tail->next);
    old_tail->next = data;
  }
  root->tail = data;
}

void dlist_push(dlist_t *root, dlist_node_t *data) {
  dlist_enqueue(root, data);
}

dlist_node_t * dlist_dequeue(dlist_t *root) {
  if (!root->tail)
    return NULL;
  dlist_node_t *retnode = root->tail;
  root->tail = retnode->prev;

  if (root->head == retnode)
    root->head = NULL;
  else 
    root->tail->next = NULL;

  return retnode;
}

dlist_node_t * dlist_pop(dlist_t *root) {
  if (!root->head)
    return NULL;
  dlist_node_t *retnode = root->head;
  root->head = retnode->next;

  if (root->tail == retnode)
    root->tail = NULL;
  else
    root->head->prev = NULL;

  return retnode;
}

dlist_remove(dlist_t *root, dlist_node_t *data) {
  if (data->prev) {
    data->prev->next = data->next;
  } else {
    assert(root->head == data);
    root->head = data->next;
  }
  if (data->next) {
    data->next->prev = data->prev;
  } else {
    assert(root->tail == data);
    root->tail = data->prev;
  }
}

dlist_node_t* dlist_head(const dlist_t *root) {
  return root->head;
}

dlist_node_t* dlist_tail(const dlist_t *root) {
  return root->tail;
}

void dlist_check(const dlist_t *root) {
  dlist_node_t *ptr;
  dlist_node_t *last_ptr = NULL;
  for (ptr = root->head; ptr; ptr = ptr->next) {
    if (last_ptr) {
      assert(last_ptr->next == ptr);
      assert(last_ptr == ptr->prev);
    } else {
      assert(ptr->prev == NULL);
    }
    last_ptr = ptr; 
  }
  assert(last_ptr == root->tail);
}

#endif

