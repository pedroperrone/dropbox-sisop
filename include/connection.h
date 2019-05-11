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
#include <dirent.h>
#include "../include/user.h"
#include "../include/command.h"
#include "../include/dropbox.h"

#define PACKAGE_SIZE 4096
#define FAILURE_BYTE_MESSAGE 'F'
#define SUCCESS_BYTE_MESSAGE 'S'
#define FILENAME_LENGTH 64
#define DATE_STRING_LENTH 30

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

typedef struct file_info {
    char filename[FILENAME_LENGTH];
    struct stat details;
} FILE_INFO;

void setPort(int portValue);
int createSocket(SOCKET_TYPE type, char *hostname, char *username, int port);
void* processConnection_REQUEST(void *clientSocket);
void* processConnection_NOTIFY_CLIENT(void *clientSocket);
void* processConnection_NOTIFY_SERVER(void *clientSocket);
void initializeMainSocket(int *serverfd, struct sockaddr_in *address);
void handleNewRequest(int mainSocket);
int sendFile(FILE *fileDescriptor, int socketDescriptor, char filename[]);
int sendExit(int socketDescriptor);
int sendRemove(int socketDescriptor, char filename[]);
int receiveFile(int socketDescriptor, COMMAND_PACKAGE command);
int deleteFile(int _socketDescriptor, COMMAND_PACKAGE commandPackage);
int receiveCommandPackage(COMMAND_PACKAGE *commandPackage, int socketDescriptor);
int receivePackage(PACKAGE *package, int socketDescriptor);
int writePackage(PACKAGE package, FILE *file);
int calculateFileSize(FILE *fileDescriptor);
int readAmountOfBytes(void *buffer, int socketDescriptor, int amountOfBytes);
int getUsernameFromNewConnection(int newSocket, char username[]);
int getSocketType(int socket);
void destroyConnection(int socketDescriptor);
int listServer(int socketDescriptor);
void sendListServer(int socketDescriptor);
void receiveServerNotification(int socket);
void enqueueSyncFile(int sockfd, COMMAND_PACKAGE command, int action);

#endif
