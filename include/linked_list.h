#ifndef LINKEDLIST_HEADER
#define LINKEDLIST_HEADER

#include <stdlib.h>
#include <string.h>
#include <pthread.h>

typedef struct node {
  void* data;
  struct node *next;
} NODE;

typedef struct list {
  NODE *head;
  pthread_mutex_t lock;
} LIST;

NODE *createNode(void *data);
LIST *createList();
void add(void* data, LIST *list);
void removeFromList(void* data, LIST *list);
void destroy(LIST *list);
int hasStringElement(char *string, LIST *list);

#endif
