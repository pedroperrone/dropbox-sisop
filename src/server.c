#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "../include/connection.h"

#define PORT 4000

int main(int argc, char *argv[]) {
    int server_fd, new_socket;
    int *newSocketPointer;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    pthread_t deamonThread;

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = INADDR_ANY;

    // Bind socket to address
    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    while(1) {
        if (listen(server_fd, 5) < 0){
            perror("listen failure");
            exit(EXIT_FAILURE);
        }
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *) &addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        newSocketPointer = (int*) malloc(sizeof(int));
        memcpy(newSocketPointer, &new_socket, sizeof(int));
        pthread_create(&deamonThread, NULL, processConnection, (void*) newSocketPointer);
    }
    close(server_fd);
    return 0;
}
