#include "../include/connection.h"
#include <stdio.h>

int port;
char failureByteMessage[1] = {FAILURE_BYTE_MESSAGE};
char successByteMessage[1] = {SUCCESS_BYTE_MESSAGE};

void setPort(int portValue) {
    port = portValue;
}

void initializeMainSocket(int *serverfd, struct sockaddr_in *address) {
    struct sockaddr_in add;
    // Creating socket file descriptor
    if ((*serverfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    add.sin_family = AF_INET;
    add.sin_port = htons(port);
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
    char username[USERNAME_LENGTH];
    if ((new_socket = accept(mainSocket, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    newSocketPointer = (int *)malloc(sizeof(int));
    if(getUsernameFromNewConnection(new_socket, username) == 0) {
        perror("Error receiving username");
    }
    fflush(stdout);
    if (createSession(username, new_socket) != 1) {
        write(new_socket, failureByteMessage, 1);
        close(new_socket);
        return;
    }
    write(new_socket, successByteMessage, 1);
    memcpy(newSocketPointer, &new_socket, sizeof(int));
    pthread_create(&deamonThread, NULL, processConnection, (void *)newSocketPointer);
}

int getUsernameFromNewConnection(int newSocket, char username[]) {
    return readAmountOfBytes(username, newSocket, USERNAME_LENGTH);
}

void* processConnection(void *clientSocket) {
    int socket = *(int *) clientSocket;
    // int valread;
    // char buffer[1024] = {0};
    // char* receivedMessage = "Message received";
    // valread = read(socket, buffer, 1024);
    // printf("Read message: %s\n", buffer);
    // send(socket, receivedMessage, strlen(receivedMessage), 0);
    // printf("Confirmation message sent\n");
    receiveFile(socket);
    return NULL;
}

int sendFile(FILE *fileDescriptor, int socketDescriptor) {
    PACKAGE package;
    int wroteBytes;
    package.totalSize = calculateFileSize(fileDescriptor);
    package.index = 1;
    while ((package.dataSize = fread(&(package.data), 1, PACKAGE_SIZE, fileDescriptor)) == PACKAGE_SIZE) {
        wroteBytes = write(socketDescriptor, &package, sizeof(PACKAGE));
        package.index++;
    }
    wroteBytes = write(socketDescriptor, &package, sizeof(PACKAGE));
    package.index++;
    return 1;
}

int receiveFile(int socketDescriptor) {
    PACKAGE package;
    FILE *receivedFile;
    USER *user;
    char filename[256];
    // printUsers();
    user = findUserFromSocket(socketDescriptor);
    if(user == NULL) {
        perror("No user with the current socket");
    }
    mkdir(user->username, 0777);
    strcpy(filename, user->username);
    strcat(filename, "/received_file.txt");
    if ((receivedFile = fopen(filename, "w")) == NULL) {
        return 0;
    }
    do {
        bzero(&(package), sizeof(PACKAGE));
        if(receivePackage(&package, socketDescriptor) == 0) {
            fclose(receivedFile);
            return 0;
        }
        if(writePackage(package, receivedFile) == 0) {
            fclose(receivedFile);
            return 0;
        }
    } while(package.index != package.totalSize);
    fclose(receivedFile);
    return 1;
}

int receivePackage(PACKAGE *package, int socketDescriptor) {
    return readAmountOfBytes(package, socketDescriptor, sizeof(PACKAGE));
}

int readAmountOfBytes(void *buffer, int socketDescriptor, int amountOfBytes) {
    int totalReadBytes = 0, bytesToRead, partialReadBytes;
    while ((bytesToRead = amountOfBytes - totalReadBytes) != 0) {
        partialReadBytes = read(socketDescriptor, (buffer + totalReadBytes), bytesToRead);
        if (partialReadBytes <= 0) {
            return 0;
        }
        totalReadBytes += partialReadBytes;
    }
    return 1;
}

int writePackage(PACKAGE package, FILE *file) {
    int wroteBytes;
    wroteBytes = fwrite(&(package.data), 1, package.dataSize, file);
    if (wroteBytes != package.dataSize) {
        return 0;
    }
    return 1;
}

int calculateFileSize(FILE *fileDescriptor) {
    int fileSize;
    fseek(fileDescriptor, 0, SEEK_END);
    fileSize = ftell(fileDescriptor);
    fseek(fileDescriptor, 0, SEEK_SET);
    return ceil((float) fileSize / (float) PACKAGE_SIZE);
}
