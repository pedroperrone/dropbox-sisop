#include "../include/linked_list.h"

NODE *createNode(void* data) {
    NODE *newNode = malloc(sizeof(NODE));
    if (!newNode) {
        return NULL;
    }
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

LIST *createList() {
    LIST *list = malloc(sizeof(LIST));
    if (!list) {
        return NULL;
    }
    list->head = NULL;
    pthread_mutex_init(&list->mutex, NULL);
    return list;
}

void add(void* data, LIST *list) {
    pthread_mutex_lock(&list->mutex);
    NODE *current = NULL;
    if (list->head == NULL) {
        list->head = createNode(data);
    }
    else {
        current = list->head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = createNode(data);
    }
    pthread_mutex_unlock(&list->mutex);
}

void removeFromList(void *data, LIST *list) {
    pthread_mutex_lock(&list->mutex);
    NODE *current = list->head;
    NODE *previous = current;
    while (current != NULL) {
        if (current->data == data) {
            previous->next = current->next;
            if (current == list->head) {
                list->head = current->next;
            }
            free(current);
            pthread_mutex_unlock(&list->mutex);
            return;
        }
        previous = current;
        current = current->next;
    }
    pthread_mutex_unlock(&list->mutex);
}

void destroy(LIST *list) {
    NODE *current = list->head;
    NODE *next = current;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    free(list);
}

int hasStringElement(char *string, LIST *list) {
    pthread_mutex_lock(&list->mutex);
    NODE *current = list->head;
    while (current != NULL) {
        if (strcmp(current->data,  string) == 0) {
            pthread_mutex_unlock(&list->mutex);
            return 1;
        }
        current = current->next;
    }
    pthread_mutex_unlock(&list->mutex);
    return 0;
}
