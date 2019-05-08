#ifndef __user__
#define __user__

#include <string.h>
#include <stdio.h>
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
int allSessionsAreFree(USER user);
void setSession(USER *user, int socketDescriptor);
USER *findUserFromSocket(int socketDescriptor);
int socketBelongsToUser(USER user, int socketDescriptor);
void *findUser(char username[]);
void printUsers();
void removeUserSession(int socketDescriptor);
void removeSessionWithSocket(USER *user, int socketDescriptor);

#endif
