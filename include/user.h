#ifndef __user__
#define __user__

#include <string.h>
#include "../include/linked_list.h"

#define USERNAME_LENGTH 64

typedef struct user {
  char username[USERNAME_LENGTH];
  int sessionOne;
  int sessionTwo;
} USER;

int initializeUsersList();
int createSession(char username[], int socketDescriptor);
int userHasFreeSession(USER user);
void setSession(USER *user, int socketDescriptor);
// char* findUsernameFromSocket(int socketDescriptor);
// USER *findUserFromSocket(int socketDescriptor);
// int socketBelongsToUser(USER user, int socketDescriptor);

#endif
