#include "../include/connection.h"
#include <stdio.h>

#define PORT 4000

void initializeMainSocket(int *serverfd, struct sockaddr_in *address) {
    struct sockaddr_in add;
    // Creating socket file descriptor
    if ((*serverfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    add.sin_family = AF_INET;
    add.sin_port = htons(PORT);
    add.sin_addr.s_addr = INADDR_ANY;

    // Bind socket to address
    if (bind(*serverfd, (struct sockaddr *)&add, sizeof(add)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(*serverfd, 5) < 0){
        perror("listen failure");
        exit(EXIT_FAILURE);
    }
    *address = add;
}

void handleNewRequest(int mainSocket, struct sockaddr_in address) {
    int new_socket, addrlen, *newSocketPointer;
    pthread_t deamonThread;
    if ((new_socket = accept(mainSocket, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    newSocketPointer = (int *)malloc(sizeof(int));
    memcpy(newSocketPointer, &new_socket, sizeof(int));
    pthread_create(&deamonThread, NULL, processConnection, (void *)newSocketPointer);
}

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
