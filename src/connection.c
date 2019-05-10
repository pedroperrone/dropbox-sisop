#include "../include/connection.h"
#include <stdio.h>

int port;

char failureByteMessage[1] = {FAILURE_BYTE_MESSAGE};
char successByteMessage[1] = {SUCCESS_BYTE_MESSAGE};

void setPort(int portValue) {
    port = portValue;
}

int createSocket(SOCKET_TYPE type, char *username, char *hostname, int port) {
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char byte_message;
    SOCKET_TYPE socket_type;

    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        printf("ERROR opening socket\n");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "ERROR connecting to socket\n");
        exit(1);
    }

    if (write(sockfd, username, USERNAME_LENGTH) != USERNAME_LENGTH) {
        fprintf(stderr, "ERROR writing username to socket\n");
        exit(1);
    }

    socket_type = REQUEST;
    if (write(sockfd, &socket_type, sizeof (SOCKET_TYPE))
        != sizeof (SOCKET_TYPE))
    {
        fprintf(stderr, "ERROR writing socket type to socket\n");
        exit(1);
    }

    if (read(sockfd, &byte_message, 1) != 1) {
        fprintf(stderr, "ERROR reading from socket\n");
        exit(1);
    }

    if (byte_message != SUCCESS_BYTE_MESSAGE) {
        fprintf(stderr, "ERROR establishing a new session\n");
        exit(1);
    }

    return sockfd;
}

void initializeMainSocket(int *serverfd, struct sockaddr_in *address) {
    struct sockaddr_in add;
    // Creating socket file descriptor
    if ((*serverfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    add.sin_family = AF_INET;
    add.sin_port = htons(port);
    add.sin_addr.s_addr = INADDR_ANY;
    bzero(&(add.sin_zero), 8);

    // Bind socket to address
    *address = add;
    if (bind(*serverfd, (struct sockaddr *)address, sizeof(add)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(*serverfd, 5) < 0){
        perror("listen failure");
        exit(EXIT_FAILURE);
    }
}

void handleNewRequest(int mainSocket) {
    int new_socket, addrlen, *newSocketPointer;
    pthread_t deamonThread;
    struct sockaddr_in cliendAddress;
    char username[USERNAME_LENGTH];
    SOCKET_TYPE socket_type;

    addrlen = sizeof (struct sockaddr_in);
    
    if ((new_socket = accept(mainSocket, (struct sockaddr *)&cliendAddress, (socklen_t *)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    newSocketPointer = (int *)malloc(sizeof(int));
    if(getUsernameFromNewConnection(new_socket, username) != 0) {
        perror("Error receiving username");
        exit(EXIT_FAILURE);
    }

    socket_type = getSocketType(new_socket);

    if (socket_type < 0) {
        perror("Error receiving socket type");
        exit(EXIT_FAILURE);
    }

    if (createSession(username, new_socket, socket_type) != 1) {
        write(new_socket, failureByteMessage, 1);
        close(new_socket);
        return;
    }
    // printUsers();
    write(new_socket, successByteMessage, 1);
    memcpy(newSocketPointer, &new_socket, sizeof(int));
    pthread_create(&deamonThread, NULL, processConnection, (void *)newSocketPointer);
}

int getUsernameFromNewConnection(int newSocket, char username[]) {
    return readAmountOfBytes(username, newSocket, USERNAME_LENGTH);
}

int getSocketType(int socket) {
    SOCKET_TYPE socket_type;

    if (readAmountOfBytes(&socket_type, socket, sizeof(SOCKET_TYPE)) != 0) {
        return -1;
    }

    return socket_type;
}

void* processConnection(void *clientSocket) {
    int socket = *(int *) clientSocket;
    COMMAND_PACKAGE commandPackage;
    do {
        receiveCommandPackage(&commandPackage, socket);
        switch (commandPackage.command) {
        case UPLOAD:
            receiveFile(socket, commandPackage);
            break;
        case DELETE:
            deleteFile(socket, commandPackage);

        default:
            break;
        }
    } while (commandPackage.command != EXIT);
    destroyConnection(socket);
    return NULL;
}

void destroyConnection(int socketDescriptor) {
    removeUserSocket(socketDescriptor);
    close(socketDescriptor);
}

int sendFile(FILE *fileDescriptor, int socketDescriptor, char filename[]) {
    PACKAGE package;
    COMMAND_PACKAGE commandPackage;

    commandPackage.command = UPLOAD;
    commandPackage.dataPackagesAmount = calculateFileSize(fileDescriptor);
    strncpy((char*) &(commandPackage.filename), filename, FILENAME_LENGTH);
    write(socketDescriptor, &commandPackage, sizeof(COMMAND_PACKAGE));

    package.index = 1;
    while ((package.dataSize = fread(&(package.data), 1, PACKAGE_SIZE, fileDescriptor)) == PACKAGE_SIZE) {
        if(write(socketDescriptor, &package, sizeof(PACKAGE)) < sizeof(PACKAGE)) {
            perror("Error on sending data");
            return 0;
        }
        package.index++;
    }
    if (write(socketDescriptor, &package, sizeof(PACKAGE)) < sizeof(PACKAGE)) {
        perror("Error on sending data");
        return 0;
    }
    return 1;
}

int sendExit(int socketDescriptor) {
    COMMAND_PACKAGE commandPackage;
    commandPackage.command = EXIT;
    if (write(socketDescriptor, &commandPackage, sizeof(PACKAGE)) < sizeof(PACKAGE)) {
        perror("Error on sending data");
        return 0;
    }
    return 1;
}

int sendRemove(int socketDescriptor, char filename[]) {
    COMMAND_PACKAGE commandPackage;
    commandPackage.command = DELETE;
    strncpy((char*) &(commandPackage.filename), filename, FILENAME_LENGTH);
    if (write(socketDescriptor, &commandPackage, sizeof(PACKAGE)) < sizeof(PACKAGE)) {
        perror("Error on sending data");
        return 0;
    }
    return 1;
}

int receiveFile(int socketDescriptor, COMMAND_PACKAGE command) {
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
    strcat(filename, "/");
    strncat(filename, (char*) &(command.filename), FILENAME_LENGTH);
    if ((receivedFile = fopen(filename, "w")) == NULL) {
        return 0;
    }
    fflush(stdout);

    do {
        bzero(&(package), sizeof(PACKAGE));
        if (receivePackage(&package, socketDescriptor) != 0) {
            fclose(receivedFile);
            return 0;
        }
        if (writePackage(package, receivedFile) == 0) {
            fclose(receivedFile);
            return 0;
        }
    } while (package.index != command.dataPackagesAmount);
    fclose(receivedFile);
    return 1;
}

int deleteFile(int socketDescriptor, COMMAND_PACKAGE commandPackage) {
    USER *user;
    char filename[FILENAME_LENGTH];
    user = findUserFromSocket(socketDescriptor);
    if (user == NULL) {
        perror("No user with the current socket");
    }
    strncpy(filename, (char*)&(user->username), USERNAME_LENGTH);
    strcat(filename, "/");
    strncat(filename, (char*) &(commandPackage.filename), FILENAME_LENGTH);
    return remove(filename) == 0;
}

int receiveCommandPackage(COMMAND_PACKAGE *commandPackage, int socketDescriptor) {
    return readAmountOfBytes(commandPackage, socketDescriptor, sizeof(COMMAND_PACKAGE));
}

int receivePackage(PACKAGE *package, int socketDescriptor) {
    return readAmountOfBytes(package, socketDescriptor, sizeof(PACKAGE));
}

int readAmountOfBytes(void *buffer, int socketDescriptor, int amountOfBytes) {
    int totalReadBytes = 0, bytesToRead, partialReadBytes;
    while ((bytesToRead = amountOfBytes - totalReadBytes) != 0) {
        partialReadBytes = read(socketDescriptor, (buffer + totalReadBytes), bytesToRead);
        if (partialReadBytes < 0) {
            perror("Error reading socket");
            return 1;
        }
        totalReadBytes += partialReadBytes;
    }
    return 0;
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
