#ifndef __connection__
#define __connection__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

void* processConnection(void* clientSocket);
void initializeMainSocket(int *serverfd, struct sockaddr_in *address);
void handleNewRequest(int mainSocket, struct sockaddr_in address);

#endif
