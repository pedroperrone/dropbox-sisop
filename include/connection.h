#ifndef __connection__
#define __connection__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <math.h>
#include <sys/stat.h>
#include <dirent.h>
#include "../include/user.h"
#include "../include/command.h"
#include "../include/dropbox.h"
#include "../include/linked_list.h"

#define PACKAGE_SIZE 4096
#define FAILURE_BYTE_MESSAGE 'F'
#define SUCCESS_BYTE_MESSAGE 'S'
#define FILENAME_LENGTH 64
#define DATE_STRING_LENTH 30
#define IP_LENGTH 30

typedef struct package {
    int index;
    int dataSize;
    char data[PACKAGE_SIZE];
} PACKAGE;

typedef struct command_package {
    COMMAND command;
    int dataPackagesAmount;
    char filename[FILENAME_LENGTH];
    char username[USERNAME_LENGTH];
} COMMAND_PACKAGE;

typedef struct file_info {
    char filename[FILENAME_LENGTH];
    struct stat details;
} FILE_INFO;

typedef struct network_address {
    char ip[IP_LENGTH];
    int port;
} NETWORK_ADDRESS;

void setPort(int portValue);
int createSocket(char *hostname, int port);
int connectSocket(SOCKET_TYPE type, char *username, struct sockaddr_in serv_addr, int sockfd, int mainLocalPort);
void* processConnection_REQUEST(void *clientSocket);
void* processConnection_NOTIFY_CLIENT(void *clientSocket);
void* processConnection_NOTIFY_SERVER(void *clientSocket);
int initializeMainSocket(int port, int list_queue_size);
USER* handleNewRequest(int mainSocket);
int getHostname(int new_socket, char hostname[]);
int getPort(int new_socket, int *port);
int sendFile(FILE *fileDescriptor, int socketDescriptor, char filename[], char username[]);
int sendExit(int socketDescriptor);
int sendRemove(int socketDescriptor, char filename[], char username[]);
void receiveUserInfo(int socket);
int receiveFile(int socketDescriptor, COMMAND_PACKAGE command, LOCATION location);
int deleteFile(int _socketDescriptor, COMMAND_PACKAGE commandPackage, LOCATION location);
int receiveCommandPackage(COMMAND_PACKAGE *commandPackage, int socketDescriptor);
int receivePackage(PACKAGE *package, int socketDescriptor);
int writePackage(PACKAGE package, FILE *file);
int calculateFileSize(FILE *fileDescriptor);
int readAmountOfBytes(void *buffer, int socketDescriptor, int amountOfBytes);
int getUsernameFromNewConnection(int newSocket, char username[]);
int getSocketType(int socket);
int getUserPort(int socket);
int getAddressFromSocket(int socket, char address[]);
void destroyConnection(int socketDescriptor);
int listServer(int socketDescriptor);
void sendListServer(int socketDescriptor);
void receiveServerNotification(int socket, LIST *ignore_list);
void enqueueSyncFile(int sockfd, COMMAND_PACKAGE command, int action, USER *user);
LIST *getListServer(int socketDescriptor);
int countNumberOfFiles(DIR *dirDescriptor);
LIST *getListOfFilesInfo(DIR *dirDescriptor, char username[]);
int requestDownload(int socketDescriptor, char filename[]);
int requestSyncDir(int socketDescriptor);
int sendDownload(int socketDescriptor, COMMAND_PACKAGE commandPackage);
int sendSyncDir(int socketDescriptor);
int sendCreateSession(USER *user, int socket);
void setReadFromSocketFunction(int (*function)(int, void *, int));
void setWriteInSocketFunction(int (*function)(int, void *, int));
int readSocketServer(int sockfd, void *destiny, int bytesToRead);
int writeSocketServer(int sockfd, void *source, int bytesToWrite);
void setRmInfos(int *sockets, int *valid, int num_replica_managers, int port);
void replicateFile(char filename[], char username[]);
void replicateDeletedFile(char filename[], char username[]);
void notifyClients();
void notifyClient(USER *user);

#endif
