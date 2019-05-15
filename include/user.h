#ifndef __user__
#define __user__

#include <string.h>
#include <stdio.h>
#include "../include/linked_list.h"
#include "../include/dropbox.h"
#include "../include/command.h"

#define USERNAME_LENGTH 64
#define NUM_SESSIONS 2
#define SOCKETS_PER_SESSION 3

typedef struct sync_file {
  char *filename;
  int sockfd;
  COMMAND action;
} SYNC_FILE;

typedef struct user {
  char username[USERNAME_LENGTH];
  int sockets[NUM_SESSIONS][SOCKETS_PER_SESSION];
  int exit[NUM_SESSIONS];
  LIST *sync_queue;
} USER;

int initializeUsersList();
int createSession(char username[], int socketDescriptor,
                  SOCKET_TYPE socket_type);
int userHasFreeSession(USER *user, SOCKET_TYPE socket_type);
int allSocketsAreFree(USER *user);
void setSession(USER *user, int socketDescriptor, SOCKET_TYPE socket_type);
USER *findUserFromSocket(int socketDescriptor);
int socketBelongsToUser(USER *user, int socketDescriptor);
void *findUser(char username[]);
void printUsers();
void removeUserSocket(int socketDescriptor);
void removeSocketFromUser(USER *user, int socketDescriptor);
int getSession(USER *user, int socketDescriptor);

#endif
