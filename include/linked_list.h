#ifndef LINKEDLIST_HEADER
#define LINKEDLIST_HEADER

#include <stdlib.h>
#include <string.h>
#include "../include/user.h"

typedef struct node {
  void* data;
  struct node *next;
} NODE;

typedef struct list {
  NODE *head;
} LIST;

NODE *createNode(void *data);
LIST *createList();
void add(void* data, LIST *list);
void removeFromList(void* data, LIST *list);
void destroy(LIST *list);

#endif
