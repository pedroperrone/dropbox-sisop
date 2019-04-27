#include "../include/user.h"

LIST *usersList;

int initializeUsersList() {
    usersList = createList();
    return usersList != NULL;
}

int createSession(char username[], int socketDescriptor) {
    USER *userPointer;
    userPointer = (USER*) findUser(username, usersList);
    if(userPointer == NULL) {
        userPointer = (USER*) malloc(sizeof(USER));
        if(userPointer == NULL) {
            return -1;
        }
        memcpy(&(userPointer->username), username, USERNAME_LENGTH);
        userPointer->sessionOne = socketDescriptor;
        userPointer->sessionTwo = 0;
        return 1;
    }
    if(userHasFreeSession(*userPointer)) {
        setSession(userPointer, socketDescriptor);
        add(userPointer, usersList);
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

// USER *findUserFromSocket(int socketDescriptor) {
//     NODE *current = usersList->head;
//     USER *userPointer;
//     if (current == NULL) {
//         return NULL;
//     }
//     while (current != NULL) {
//         userPointer = (USER *)current->data;
//         if (socketBelongsToUser(*userPointer, socketDescriptor)) {
//             return userPointer;
//         }
//         current = current->next;
//     }
//     return NULL;
// }

// int socketBelongsToUser(USER user, int socketDescriptor) {
//     return user.sessionOne == socketDescriptor || user.sessionTwo == socketDescriptor;
// }
