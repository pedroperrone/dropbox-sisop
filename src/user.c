#include "../include/user.h"

LIST *usersList;

int initializeUsersList() {
    usersList = createList();
    return usersList != NULL;
}

int createSession(char username[], int socketDescriptor) {
    USER *userPointer;
    userPointer = (USER*) findUser(username);
    if(userPointer == NULL) {
        userPointer = (USER*) malloc(sizeof(USER));
        if(userPointer == NULL) {
            return -1;
        }
        memcpy(&(userPointer->username), username, USERNAME_LENGTH);
        userPointer->sessionTwo = 0;
        userPointer->sessionTwo = 0;
        add(userPointer, usersList);
    }
    if(userHasFreeSession(*userPointer)) {
        setSession(userPointer, socketDescriptor);
        fflush(stdout);
        return 1;
    }
    return 0;
}

int userHasFreeSession(USER user) {
    return user.sessionOne == 0 || user.sessionTwo == 0;
}

void setSession(USER* user, int socketDescriptor) {
    if(user->sessionOne == 0) {
        user->sessionOne = socketDescriptor;
        return;
    }
    if(user->sessionTwo == 0) {
        user->sessionTwo = socketDescriptor;
    }
}

void printUsers() {
    NODE *current = usersList->head;
    USER* userPtr;
    while(current != NULL) {
        userPtr = (USER*) current->data;
        printf("Username: %s\n", userPtr->username);
        printf("Socket 1: %d\n", userPtr->sessionOne);
        printf("Socket 2: %d\n\n", userPtr->sessionTwo);
        current = current->next;
    }
    printf("FIM DA LISTA DE USERS\n\n");
}

USER *findUserFromSocket(int socketDescriptor) {
    NODE *current = usersList->head;
    USER *userPointer;
    if (current == NULL) {
        return NULL;
    }
    while (current != NULL) {
        userPointer = (USER *)current->data;
        if (socketBelongsToUser(*userPointer, socketDescriptor)) {
            return userPointer;
        }
        current = current->next;
    }
    return NULL;
}

int socketBelongsToUser(USER user, int socketDescriptor) {
    return user.sessionOne == socketDescriptor || user.sessionTwo == socketDescriptor;
}

void *findUser(char username[]) {
    NODE *current = usersList->head;
    USER *userPointer;
    if (current == NULL) {
        return NULL;
    }
    while (current != NULL) {
        userPointer = (USER *)current->data;
        if (strncmp(userPointer->username, username, USERNAME_LENGTH) == 0) {
            return userPointer;
        }
        current = current->next;
    }
    return NULL;
}

void removeUserSession(int socketDescriptor) {
    USER *user = findUserFromSocket(socketDescriptor);
    if(user == NULL) {
        return;
    }
    removeFromList(user, usersList);
    free(user);
}
