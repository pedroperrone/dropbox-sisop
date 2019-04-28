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
    return list;
}

void add(void* data, LIST *list) {
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
}

void removeFromList(void *data, LIST *list) {
    NODE *current = list->head;
    NODE *previous = current;
    while (current != NULL) {
        if (current->data == data) {
            previous->next = current->next;
            if (current == list->head) {
                list->head = current->next;
            }
            free(current);
            return;
        }
        previous = current;
        current = current->next;
    }
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

void* findUser(char username[], LIST *list) {
    NODE *current = list->head;
    USER *userPointer;
    if(current == NULL) {
        return NULL;
    }
    while(current != NULL) {
        userPointer = (USER*) current->data;
        if(strncmp(userPointer->username, username, USERNAME_LENGTH) == 0) {
            return userPointer;
        }
        current = current->next;
    }
    return NULL;
}
