#include "../include/connection.h"
#include <stdio.h>

int port;

char failureByteMessage[1] = {FAILURE_BYTE_MESSAGE};
char successByteMessage[1] = {SUCCESS_BYTE_MESSAGE};

pthread_mutex_t sync_queue_lock;

void setPort(int portValue) {
    port = portValue;
}

int createSocket(SOCKET_TYPE type, char *username, char *hostname, int port) {
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char byte_message;

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

    if (write(sockfd, &type, sizeof(SOCKET_TYPE)) != sizeof(SOCKET_TYPE)) {
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

    // Array of pointers to functions that receive (void *) and return (void *).
    void *(*processConnection[])(void *) = {
        processConnection_REQUEST,
        processConnection_NOTIFY_CLIENT,
        processConnection_NOTIFY_SERVER
    };

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
    write(new_socket, successByteMessage, 1);
    memcpy(newSocketPointer, &new_socket, sizeof(int));
    pthread_create(&deamonThread, NULL, processConnection[socket_type],
                   (void *)newSocketPointer);
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

void* processConnection_REQUEST(void *clientSocket) {
    // Código provisório para manter a funcionalidade implementada até agora.
    // O REQUEST atende requisições de download, list_server e get_sync_dir.
    int socket = *(int *) clientSocket;
    USER *user = findUserFromSocket(socket);
    COMMAND_PACKAGE commandPackage;
    do {
        receiveCommandPackage(&commandPackage, socket);
        switch (commandPackage.command) {
        case UPLOAD:
            // printf("Received upload request for file '%s' from REQUEST\n", commandPackage.filename);
            receiveFile(socket, commandPackage, SERVER);
            enqueueSyncFile(-1, commandPackage, UPLOAD, user);
            break;
        case DELETE:
            // printf("Received delete request for file '%s' from REQUEST\n", commandPackage.filename);
            deleteFile(socket, commandPackage, SERVER);
            enqueueSyncFile(-1, commandPackage, DELETE, user);
            break;
        case LIST_SERVER:
            listServer(socket);
            break;
        default:
            break;
        }
    } while (commandPackage.command != EXIT);
    destroyConnection(socket);
    return NULL;
}

void* processConnection_NOTIFY_CLIENT(void *clientSocket) {
    // O NOTIFY_CLIENT notifica o cliente sobre criação, atualização e exclusão
    // de arquivos.
    int socket = *(int *) clientSocket;
    int i;
    char *file_path = (char*) malloc(FILENAME_LENGTH);
    NODE *current;
    FILE *file;
    USER *user = findUserFromSocket(socket);
    SYNC_FILE *sync_file;


    while(1) {
        sleep(2);
        //printUsers();
        pthread_mutex_lock(&sync_queue_lock);
        current = user->sync_queue->head;
        if(current) {
            sync_file = current->data;
            strcpy(file_path, user->username);
            strcat(file_path, "/");
            strcat(file_path, sync_file->filename);
            for(i = 0; i < NUM_SESSIONS; i++){
                if(user->sockets[i][NOTIFY_SERVER] != sync_file->sockfd &&
                   user->sockets[i][REQUEST] != sync_file->sockfd) {
                    if(sync_file->action == UPLOAD) {
                        if((file = fopen(file_path, "r")) == NULL) {
                            printf("Error openning file '%s'", file_path);
                        } else {
                            sendFile(file, user->sockets[i][NOTIFY_CLIENT], sync_file->filename);
                            fclose(file);
                        }
                    }
                    else {
                        sendRemove(user->sockets[i][NOTIFY_CLIENT], sync_file->filename);
                    }
                }
            }
            removeFromList(sync_file, user->sync_queue);
        }
        pthread_mutex_unlock(&sync_queue_lock);
    }

    destroyConnection(socket);
    return NULL;
}

void* processConnection_NOTIFY_SERVER(void *clientSocket) {
    // O NOTIFY_SERVER recebe notificações do cliente sobre criação, 
    // atualização e exclusão de arquivos.
    int socket = *(int *) clientSocket;
    USER *user = findUserFromSocket(socket);
    COMMAND_PACKAGE commandPackage;
    do {
        receiveCommandPackage(&commandPackage, socket);
        switch (commandPackage.command) {
        case UPLOAD:
            // printf("Received upload request for file '%s' from INOTIFY\n", commandPackage.filename);
            receiveFile(socket, commandPackage, SERVER);
            enqueueSyncFile(socket, commandPackage, UPLOAD, user);
            break;
        case DELETE:
            // printf("Received delete request for file '%s' from INOTIFY\n", commandPackage.filename);
            deleteFile(socket, commandPackage, SERVER);
            enqueueSyncFile(socket, commandPackage, DELETE, user);
            break;
        default:
            break;
        }
        // printUsers();
    } while (commandPackage.command != EXIT);
    destroyConnection(socket);
    return NULL;
}

void receiveServerNotification(int socket) {
    COMMAND_PACKAGE commandPackage;
    do {
        receiveCommandPackage(&commandPackage, socket);
        switch (commandPackage.command) {
        case UPLOAD:
            // printf("received upload from server\n");
            receiveFile(socket, commandPackage, CLIENT);
            break;
        case DELETE:
            // printf("received delete from server\n");
            deleteFile(socket, commandPackage, CLIENT);
        default:
            break;
        }
    } while (commandPackage.command != EXIT);
    destroyConnection(socket);
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
    if (write(socketDescriptor, &commandPackage, sizeof(COMMAND_PACKAGE)) < sizeof(COMMAND_PACKAGE)) {
        perror("Error on sending data");
        return 0;
    }
    return 1;
}

int sendRemove(int socketDescriptor, char filename[]) {
    COMMAND_PACKAGE commandPackage;
    commandPackage.command = DELETE;
    strncpy((char*) &(commandPackage.filename), filename, FILENAME_LENGTH);
    if (write(socketDescriptor, &commandPackage, sizeof(COMMAND_PACKAGE)) < sizeof(COMMAND_PACKAGE)) {
        perror("Error on sending data");
        return 0;
    }
    return 1;
}

int receiveFile(int socketDescriptor, COMMAND_PACKAGE command, LOCATION location) {
    PACKAGE package;
    FILE *receivedFile;
    USER *user;
    char filename[256];

    if(location == SERVER) {
        user = findUserFromSocket(socketDescriptor);
        if(user == NULL) {
            perror("No user with the current socket");
        }
        mkdir(user->username, 0777);
        strcpy(filename, user->username);
    } else {
        strcpy(filename, "sync_dir");
    }
    strcat(filename, "/");
    strncat(filename, (char*) &(command.filename), FILENAME_LENGTH);
    if ((receivedFile = fopen(filename, "w")) == NULL) {
        return 0;
    }

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
    } while (package.index < command.dataPackagesAmount);
    fclose(receivedFile);
    return 1;
}

int deleteFile(int socketDescriptor, COMMAND_PACKAGE commandPackage, LOCATION location) {
    USER *user;
    char filename[FILENAME_LENGTH];

    if(location == SERVER) {
        user = findUserFromSocket(socketDescriptor);
        if (user == NULL) {
            perror("No user with the current socket");
        }
        strcpy(filename, user->username);
    } else
        strcpy(filename, "sync_dir");
    strcat(filename, "/");
    strncat(filename, (char*) &(commandPackage.filename), FILENAME_LENGTH);
    return remove(filename) == 0;
}

int listServer(int socketDescriptor) {
    USER *user;
    DIR *mydir;
    struct dirent *myfile;
    struct stat mystat;
    char buf[512];
    PACKAGE package;
    COMMAND_PACKAGE command;
    FILE_INFO fileInfo;

    user = findUserFromSocket(socketDescriptor);
    if (user == NULL){
        perror("No user with the current socket");
    }

    package.index = 1;
    package.dataSize = sizeof(FILE_INFO);
    command.command = LIST_SERVER;
    command.dataPackagesAmount = -2; // Not count . and ..

    mydir = opendir((char *) &(user->username));
    if(mydir == NULL) {
        command.dataPackagesAmount = 0;
        if (write(socketDescriptor, &command, sizeof(COMMAND_PACKAGE)) < sizeof(COMMAND_PACKAGE)) {
            perror("Error on sending command for list server");
            return 0;
        }
        return 1;
    }
    while ((myfile = readdir(mydir)) != NULL) {
        command.dataPackagesAmount++;
    }
    rewinddir(mydir);
    if(write(socketDescriptor, &command, sizeof(COMMAND_PACKAGE)) < sizeof(COMMAND_PACKAGE)) {
        perror("Error on sending command for list server");
        return 0;
    }
    while ((myfile = readdir(mydir)) != NULL) {
        if ((strcmp((char *)&(myfile->d_name), ".") != 0) && (strcmp((char *) &(myfile->d_name), "..") != 0)) {
            strncpy((char*) &(fileInfo.filename), myfile->d_name, FILENAME_LENGTH);
            sprintf(buf, "%s/%s", user->username, myfile->d_name);
            stat(buf, &mystat);
            memcpy(&(fileInfo.details), &mystat, sizeof(struct stat));
            memcpy(&(package.data), &fileInfo, sizeof(FILE_INFO));
            fflush(stdout);
            if(write(socketDescriptor, &(package), sizeof(PACKAGE)) < sizeof(PACKAGE)) {
                perror("Error on sending data for list server");
            }
            package.index++;
        }
    }
    closedir(mydir);
    return 1;
}

void sendListServer(int socketDescriptor) {
    COMMAND_PACKAGE commandPackage;
    PACKAGE package;
    FILE_INFO fileInfo;
    commandPackage.command = LIST_SERVER;
    struct stat fileStat;
    int i = 0;
    char dateString[DATE_STRING_LENTH];
    if (write(socketDescriptor, &commandPackage, sizeof(COMMAND_PACKAGE)) < sizeof(COMMAND_PACKAGE)) {
        perror("Error on sending command for request list server");
    }
    receiveCommandPackage(&commandPackage, socketDescriptor);
    printf("Created at\t\t\tModified at\t\t\tAccesses at\t\t\tFile Name\n");
    while(i < commandPackage.dataPackagesAmount) {
        receivePackage(&package, socketDescriptor);
        memcpy(&fileInfo, &(package.data), package.dataSize);
        memcpy(&fileStat, &(fileInfo.details), sizeof(struct stat));
        strncpy(dateString, ctime(&(fileStat.st_ctime)), DATE_STRING_LENTH);
        dateString[strlen(dateString) - 1] = '\0';
        printf("%s\t", dateString);
        strncpy(dateString, ctime(&(fileStat.st_mtime)), DATE_STRING_LENTH);
        dateString[strlen(dateString) - 1] = '\0';
        printf("%s\t", dateString);
        strncpy(dateString, ctime(&(fileStat.st_atime)), DATE_STRING_LENTH);
        dateString[strlen(dateString) - 1] = '\0';
        printf("%s\t", dateString);
        printf("%s\n", fileInfo.filename);
        i++;
    }
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

void enqueueSyncFile(int sockfd, COMMAND_PACKAGE command, int action, USER *user) {
    pthread_mutex_lock(&sync_queue_lock);
    SYNC_FILE *sync =(SYNC_FILE*) malloc(sizeof(SYNC_FILE));

    sync->sockfd = sockfd;
    sync->filename = (char*) malloc(sizeof(command.filename));
    strcpy(sync->filename, command.filename);
    sync->action = action;
    add(sync, user->sync_queue);
    pthread_mutex_unlock(&sync_queue_lock);
}
