#include "../include/user.h"

LIST *usersList;

int initializeUsersList() {
    usersList = createList();
    return usersList != NULL;
}

int createSession(char username[], int socketDescriptor,
                  SOCKET_TYPE socket_type)
{
    USER *userPointer;
    userPointer = (USER*) findUser(username);
    if(userPointer == NULL) {
        userPointer = (USER*) malloc(sizeof(USER));
        if(userPointer == NULL) {
            return -1;
        }
        memcpy(&(userPointer->username), username, USERNAME_LENGTH);
        for (int i = 0; i < NUM_SESSIONS; i++) {
            for (int j = 0; j < SOCKETS_PER_SESSION; j++) {
                userPointer->sockets[i][j] = 0;
            }
        }
        add(userPointer, usersList);
    }
    if(userHasFreeSession(userPointer, socket_type)) {
        setSession(userPointer, socketDescriptor, socket_type);
        fflush(stdout);
        return 1;
    }
    return 0;
}

int userHasFreeSession(USER *user, SOCKET_TYPE socket_type) {
    for (int i = 0; i < NUM_SESSIONS; i++) {
        if (user->sockets[i][socket_type] == 0) return 1;
    }

    return 0;
}

int allSocketsAreFree(USER *user) {
    for (int i = 0; i < NUM_SESSIONS; i++) {
        for (int j = 0; j < SOCKETS_PER_SESSION; j++) {
            if (user->sockets[i][j] != 0) return 0;
        }
    }

    return 1;
}

void setSession(USER* user, int socketDescriptor, SOCKET_TYPE socket_type) {
    for (int i = 0; i < NUM_SESSIONS; i++) {
        if (user->sockets[i][socket_type] == 0) {
            user->sockets[i][socket_type] = socketDescriptor;
        }
    }
}

void printUsers() {
    NODE *current = usersList->head;
    USER* userPtr;
    while(current != NULL) {
        userPtr = (USER*) current->data;
        printf("Username: %s\n", userPtr->username);

        for (int i = 0; i < NUM_SESSIONS; i++) {
            printf("Session %d:\n", i);
            for (int j = 0; j < SOCKETS_PER_SESSION; j++) {
                printf("  Socket %d: %d\n", j, userPtr->sockets[i][j]);
            }
        }

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
        if (socketBelongsToUser(userPointer, socketDescriptor)) {
            return userPointer;
        }
        current = current->next;
    }
    return NULL;
}

int socketBelongsToUser(USER *user, int socketDescriptor) {
    for (int i = 0; i < NUM_SESSIONS; i++) {
        for (int j = 0; j < SOCKETS_PER_SESSION; j++) {
            if (user->sockets[i][j] == socketDescriptor) return 1;
        }
    }

    return 0;
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

void removeUserSocket(int socketDescriptor) {
    USER *user = findUserFromSocket(socketDescriptor);
    if(user == NULL) {
        return;
    }
    removeSocketFromUser(user, socketDescriptor);
    if(allSocketsAreFree(user)) {
        removeFromList(user, usersList);
        free(user);
    }
}

void removeSocketFromUser(USER *user, int socketDescriptor) {
    for (int i = 0; i < NUM_SESSIONS; i++) {
        for (int j = 0; j < SOCKETS_PER_SESSION; j++) {
            if (user->sockets[i][j] == socketDescriptor) {
                user->sockets[i][j] = 0;
            }
        }
    }
}
