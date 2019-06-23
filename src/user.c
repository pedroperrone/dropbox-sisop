#include "../include/user.h"

LIST *usersList;
pthread_mutex_t removeUserSocket_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t createSession_lock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t createSession_lock2 = PTHREAD_MUTEX_INITIALIZER;

int initializeUsersList() {
    usersList = createList();
    return usersList != NULL;
}

int createSession(char username[], int socketDescriptor,
                  SOCKET_TYPE socket_type, char ipaddress[], int port, int id)
{
    USER *userPointer;

    pthread_mutex_lock(&createSession_lock1);
    userPointer = (USER*) findUser(username);
    if(userPointer == NULL) {
        userPointer = (USER*) malloc(sizeof(USER));
        if(userPointer == NULL) {
            pthread_mutex_unlock(&createSession_lock1);
            return -1;
        }
        memcpy(&(userPointer->username), username, USERNAME_LENGTH);
        userPointer->sync_queue = createList();
        for (int i = 0; i < NUM_SESSIONS; i++) {
            userPointer->exit[i] = 0;
            for (int j = 0; j < SOCKETS_PER_SESSION; j++) {
                userPointer->sockets[i][j] = 0;
            }
        }
        add(userPointer, usersList);
    }
    pthread_mutex_unlock(&createSession_lock1);

    pthread_mutex_lock(&createSession_lock2);
    if(userHasFreeSession(userPointer, socket_type)) {
        id = setSession(userPointer, socketDescriptor, socket_type, ipaddress, port, id);
        pthread_mutex_unlock(&createSession_lock2);
        return id;
    }
    pthread_mutex_unlock(&createSession_lock2);
    return -1;
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

int setSession(USER* user, int socketDescriptor, SOCKET_TYPE socket_type, char ipaddress[], int port, int id) {
    if (socket_type == REQUEST) {
        for (int i = 0; i < NUM_SESSIONS; i++) {
            if (user->sockets[i][socket_type] == 0) {
                user->sockets[i][socket_type] = socketDescriptor;
                memcpy(&(user->ipaddresses[i]), ipaddress, IP_LENGTH);
                user->ports[i] = port;
                return i;
            }
        }
    } else {
        if (user->sockets[id][socket_type] == 0) {
            user->sockets[id][socket_type] = socketDescriptor;
            memcpy(&(user->ipaddresses[id]), ipaddress, IP_LENGTH);
            user->ports[id] = port;
            return id;
        }
    }
    return -1;
}

void printUsers() {
    NODE *currentUser = usersList->head;
    NODE *currentSyncFile;
    USER* userPtr;
    SYNC_FILE* syncFilePtr;
    while(currentUser != NULL) {
        userPtr = (USER*) currentUser->data;
        printf("Username: %s\n", userPtr->username);

        for (int i = 0; i < NUM_SESSIONS; i++) {
            printf("Session %d:\n", i);
            printf("  Ip: %s\n", userPtr->ipaddresses[i]);
            printf("  Port: %d\n", userPtr->ports[i]);
            for (int j = 0; j < SOCKETS_PER_SESSION; j++) {
                printf("  Socket %d: %d\n", j, userPtr->sockets[i][j]);
            }
        }

        printf("Files to sync:\n");
        currentSyncFile = userPtr->sync_queue->head;
        while(currentSyncFile != NULL) {
            syncFilePtr = (SYNC_FILE*) currentSyncFile->data;
            printf("  %s / %d / %d\n", syncFilePtr->filename, syncFilePtr->sockfd, syncFilePtr->action);
            currentSyncFile = currentSyncFile->next;
        }
        currentUser = currentUser->next;
    }
    printf("FIM DA LISTA DE USERS\n\n");
}

USER *findUserFromSocket(int socketDescriptor) {
    pthread_mutex_lock(&usersList->lock);
    NODE *current = usersList->head;
    USER *userPointer;
    if (current == NULL) {
        pthread_mutex_unlock(&usersList->lock);
        return NULL;
    }
    while (current != NULL) {
        userPointer = (USER *)current->data;
        if (socketBelongsToUser(userPointer, socketDescriptor)) {
            pthread_mutex_unlock(&usersList->lock);
            return userPointer;
        }
        current = current->next;
    }
    pthread_mutex_unlock(&usersList->lock);
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
    pthread_mutex_lock(&usersList->lock);
    NODE *current = usersList->head;
    USER *userPointer;
    if (current == NULL) {
        pthread_mutex_unlock(&usersList->lock);
        return NULL;
    }
    while (current != NULL) {
        userPointer = (USER *)current->data;
        if (strncmp(userPointer->username, username, USERNAME_LENGTH) == 0) {
            pthread_mutex_unlock(&usersList->lock);
            return userPointer;
        }
        current = current->next;
    }
    pthread_mutex_unlock(&usersList->lock);
    return NULL;
}

void removeUserSocket(int socketDescriptor) {
    USER *user = findUserFromSocket(socketDescriptor);
    if(user == NULL) {
        return;
    }
    pthread_mutex_lock(&removeUserSocket_lock);
    removeSocketFromUser(user, socketDescriptor);
    if(allSocketsAreFree(user)) {
        removeFromList(user, usersList);
        free(user);
    }
    pthread_mutex_unlock(&removeUserSocket_lock);
}

void removeSocketFromUser(USER *user, int socketDescriptor) {
    for (int i = 0; i < NUM_SESSIONS; i++) {
        for (int j = 0; j < SOCKETS_PER_SESSION; j++) {
            if (user->sockets[i][j] == socketDescriptor) {
                user->sockets[i][j] = 0;
                user->exit[i] = 1;
            }
        }
    }
}

int getSession(USER *user, int socketDescriptor) {
    for (int i = 0; i < NUM_SESSIONS; i++) {
        for (int j = 0; j < SOCKETS_PER_SESSION; j++) {
            if (user->sockets[i][j] == socketDescriptor) {
                return i;
            }
        }
    }
    return -1;
}

void addUser(USER *user) {
    add(user, usersList);
}

LIST *getUsersList() {
    return usersList;
}

void removeFromUsersList(USER *user) {
    removeFromList(user, usersList);
}
