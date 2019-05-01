#ifndef __connection__
#define __connection__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <math.h>
#include <sys/stat.h>
#include "../include/user.h"
#include "../include/command.h"

#define PACKAGE_SIZE 4096
#define FAILURE_BYTE_MESSAGE 'F'
#define SUCCESS_BYTE_MESSAGE 'S'
#define FILENAME_LENGTH 64

typedef struct package {
    int index;
    int dataSize;
    char data[PACKAGE_SIZE];
} PACKAGE;

typedef struct command_package {
    int command;
    int dataPackagesAmount;
    char filename[FILENAME_LENGTH];
} COMMAND_PACKAGE;

void setPort(int portValue);
void* processConnection(void* clientSocket);
void initializeMainSocket(int *serverfd, struct sockaddr_in *address);
void handleNewRequest(int mainSocket, struct sockaddr_in address);
int sendFile(FILE *fileDescriptor, int socketDescriptor, char filename[]);
int sendExit(int socketDescriptor);
int receiveFile(int socketDescriptor, COMMAND_PACKAGE command);
int receiveCommandPackage(COMMAND_PACKAGE *commandPackage, int socketDescriptor);
int receivePackage(PACKAGE *package, int socketDescriptor);
int writePackage(PACKAGE package, FILE *file);
int calculateFileSize(FILE *fileDescriptor);
int readAmountOfBytes(void *buffer, int socketDescriptor, int amountOfBytes);
int getUsernameFromNewConnection(int newSocket, char username[]);

#endif
