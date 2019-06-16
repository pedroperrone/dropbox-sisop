#ifndef __frontend__
#define __frontend__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include "../include/connection.h"

void initializeFrontend(char *hostname, int port, char *local_username);
void setNewAddress(char *hostname, int port);
void reconnectSockets();
int getSocketByType(SOCKET_TYPE type);

#endif
