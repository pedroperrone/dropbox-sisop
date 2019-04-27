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

#define PACKAGE_SIZE 4096

typedef struct package {
    int totalSize;
    int index;
    int dataSize;
    char data[PACKAGE_SIZE];
} PACKAGE;

void* processConnection(void* clientSocket);
void initializeMainSocket(int *serverfd, struct sockaddr_in *address);
void handleNewRequest(int mainSocket, struct sockaddr_in address);
int sendFile(FILE *fileDescriptor, int socketDescriptor);
int receiveFile(int socketDescriptor);
int receivePackage(PACKAGE *package, int socketDescriptor);
int writePackage(PACKAGE package, FILE *file);
int calculateFileSize(FILE *fileDescriptor);

#endif
