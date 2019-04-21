#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "../include/connection.h"
#include <stdio.h>

void* processConnection(void *clientSocket) {
    int socket = *(int *) clientSocket;
    int valread;
    char buffer[1024] = {0};
    char* receivedMessage = "Message received";
    valread = read(socket, buffer, 1024);
    printf("Read message: %s\n", buffer);
    send(socket, receivedMessage, strlen(receivedMessage), 0);
    printf("Confirmation message sent\n");
    return NULL;
}
