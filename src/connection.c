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
            receiveFile(socket, commandPackage, SERVER);
            enqueueSyncFile(-1, commandPackage, UPLOAD, user);
            break;
        case DELETE:
            deleteFile(socket, commandPackage, SERVER);
            enqueueSyncFile(-1, commandPackage, DELETE, user);
            break;
        case LIST_SERVER:
            listServer(socket);
            break;
        case DOWNLOAD:
            sendDownload(socket, commandPackage);
            break;
        case GET_SYNC_DIR:
            sendSyncDir(socket);
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
    int i, session;
    char *file_path = (char*) malloc(FILENAME_LENGTH);
    NODE *current;
    FILE *file;
    USER *user = findUserFromSocket(socket);
    SYNC_FILE *sync_file;

    session = getSession(user, socket);

    while(1) {
        if(user->exit[session]) break;
        pthread_mutex_lock(&user->sync_queue->lock);
        current = user->sync_queue->head;
        if(current) {
            sync_file = current->data;
            strcpy(file_path, user->username);
            strcat(file_path, "/");
            strcat(file_path, sync_file->filename);
            for(i = 0; i < NUM_SESSIONS; i++){
                if(user->sockets[i][NOTIFY_SERVER] != sync_file->sockfd &&
                   user->sockets[i][NOTIFY_SERVER] != 0 &&
                   user->sockets[i][REQUEST] != sync_file->sockfd &&
                   user->sockets[i][REQUEST] != 0)
                   {
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
            pthread_mutex_unlock(&user->sync_queue->lock);
            removeFromList(sync_file, user->sync_queue);
        }
        else {
            pthread_mutex_unlock(&user->sync_queue->lock);
        }
    }
    sendExit(socket);
    shutdown(socket, 2);
    removeUserSocket(socket);
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
            receiveFile(socket, commandPackage, SERVER);
            enqueueSyncFile(socket, commandPackage, UPLOAD, user);
            break;
        case DELETE:
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

void receiveServerNotification(int socket, LIST *ignore_list) {
    COMMAND_PACKAGE commandPackage;
    do {
        receiveCommandPackage(&commandPackage, socket);
        switch (commandPackage.command) {
        case UPLOAD:
            add(commandPackage.filename, ignore_list);
            receiveFile(socket, commandPackage, CLIENT);
            break;
        case DELETE:
            add(commandPackage.filename, ignore_list);
            deleteFile(socket, commandPackage, CLIENT);
        default:
            break;
        }
    } while (commandPackage.command != EXIT);
    close(socket);
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
    if(fileDescriptor == NULL) {
        return 1;
    }

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

int sendDownload(int socketDescriptor, COMMAND_PACKAGE commandPackage) {
    USER *user = findUserFromSocket(socketDescriptor);
    char filename[USERNAME_LENGTH + 1 + FILENAME_LENGTH];
    if (user == NULL) {
        perror("No user with the current socket");
    }
    filename[0] = '\0';
    strncpy(filename, (char *) &(user->username), USERNAME_LENGTH);
    strcat(filename, "/");
    strncat(filename, (char*) &(commandPackage.filename), FILENAME_LENGTH);
    return sendFile(fopen(filename, "r"), socketDescriptor, (char*) commandPackage.filename);
}

int sendSyncDir(int socketDescriptor) {
    USER *user;
    DIR *mydir;
    FILE_INFO *fileInfo;
    LIST *listOfFiles;
    NODE *current;
    char filename[USERNAME_LENGTH + 1 + FILENAME_LENGTH];
    COMMAND_PACKAGE commandPackage;

    user = findUserFromSocket(socketDescriptor);
    if (user == NULL){
        perror("No user with the current socket");
    }

    mydir = opendir((char *) &(user->username));

    listOfFiles = getListOfFilesInfo(mydir, (char*) &(user->username));
    current = listOfFiles->head;
    while(current != NULL) {
        fileInfo = current->data;

        filename[0] = '\0';
        strncpy(filename, (char *) &(user->username), USERNAME_LENGTH);
        strcat(filename, "/");
        strncat(filename, fileInfo->filename, FILENAME_LENGTH);

        sendFile(fopen(filename, "r"), socketDescriptor, (char*) fileInfo->filename);

        current = current->next;
    }

    commandPackage.command = END_GET_SYNC_DIR;
    if (write(socketDescriptor, &commandPackage, sizeof(COMMAND_PACKAGE)) < sizeof(COMMAND_PACKAGE)) {
        perror("Error on sending data");
        return 1;
    }

    destroy(listOfFiles);
    closedir(mydir);
    return 0;
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
        strcat(filename, "/");
    } else if(location == CLIENT) {
        strcpy(filename, "sync_dir/");
    } else {
        filename[0] = '\0';
    }
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

int requestDownload(int socketDescriptor, char filename[]) {
    COMMAND_PACKAGE commandPackage;
    commandPackage.command = DOWNLOAD;
    strncpy((char*) &(commandPackage.filename), filename, FILENAME_LENGTH);
    if(write(socketDescriptor, &commandPackage, sizeof(COMMAND_PACKAGE)) < sizeof(commandPackage)) {
        perror("Error sending command to download file");
        return 0;
    }
    return 1;
}

int requestSyncDir(int socketDescriptor) {
    COMMAND_PACKAGE commandPackage;
    commandPackage.command = GET_SYNC_DIR;
    if(write(socketDescriptor, &commandPackage, sizeof(COMMAND_PACKAGE)) < sizeof(commandPackage)) {
        perror("Error sending command to download sync_dir");
        return 1;
    }
    return 0;
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
    PACKAGE package;
    COMMAND_PACKAGE command;
    FILE_INFO *fileInfo;
    LIST *listOfFiles;
    NODE *current;

    user = findUserFromSocket(socketDescriptor);
    if (user == NULL){
        perror("No user with the current socket");
    }
    mydir = opendir((char *) &(user->username));

    package.index = 1;
    package.dataSize = sizeof(FILE_INFO);
    command.command = LIST_SERVER;
    command.dataPackagesAmount = countNumberOfFiles(mydir);
    if(write(socketDescriptor, &command, sizeof(COMMAND_PACKAGE)) < sizeof(COMMAND_PACKAGE)) {
        perror("Error on sending command for list server");
        return 0;
    }
    listOfFiles = getListOfFilesInfo(mydir, (char*) &(user->username));
    current = listOfFiles->head;
    while(current != NULL) {
        fileInfo = current->data;
        memcpy(&(package.data), fileInfo, sizeof(FILE_INFO));
        if (write(socketDescriptor, &(package), sizeof(PACKAGE)) < sizeof(PACKAGE)) {
            perror("Error on sending data for list server");
        }
        package.index++;
        current = current->next;
    }
    destroy(listOfFiles);
    closedir(mydir);
    return 1;
}

int countNumberOfFiles(DIR *dirDescriptor) {
    int i = -2; // Not count . and ..
    struct dirent *myfile;

    if (dirDescriptor == NULL) return 0;

    rewinddir(dirDescriptor);
    while((myfile = readdir(dirDescriptor)) != NULL) {
        i++;
    }
    return i;
}

LIST* getListOfFilesInfo(DIR *dirDescriptor, char username[]) {
    LIST *listOfFiles = createList();
    struct dirent *myfile;
    struct stat mystat;
    char buf[512];
    FILE_INFO *fileInfo;
    if(listOfFiles == NULL) {
        perror("Error allocating memory on getListOfFiles");
        return NULL;
    }

    if (dirDescriptor == NULL) return listOfFiles;

    rewinddir(dirDescriptor);
    while ((myfile = readdir(dirDescriptor)) != NULL) {
        if ((strcmp((char *)&(myfile->d_name), ".") != 0) && (strcmp((char *)&(myfile->d_name), "..") != 0)) {
            fileInfo = (FILE_INFO*) malloc(sizeof(FILE_INFO));
            fflush(stdout);
            strncpy((char *)&(fileInfo->filename), myfile->d_name, FILENAME_LENGTH);
            sprintf(buf, "%s/%s", username, myfile->d_name);
            stat(buf, &mystat);
            fileInfo->details = mystat;
            add(fileInfo, listOfFiles);
        }
    }
    return listOfFiles;
}

LIST* getListServer(int socketDescriptor) {
    COMMAND_PACKAGE commandPackage;
    PACKAGE package;
    FILE_INFO *fileInfo;
    LIST *filesInfo = createList();
    commandPackage.command = LIST_SERVER;
    int i = 0;
    if (write(socketDescriptor, &commandPackage, sizeof(COMMAND_PACKAGE)) < sizeof(COMMAND_PACKAGE)) {
        perror("Error on sending command for request list server");
    }
    receiveCommandPackage(&commandPackage, socketDescriptor);
    while(i < commandPackage.dataPackagesAmount) {
        receivePackage(&package, socketDescriptor);
        fileInfo = (FILE_INFO*) malloc(sizeof(FILE_INFO));
        if(fileInfo == NULL) {
            perror("Error allocating memory on getListServer");
            return NULL;
        }
        memcpy(fileInfo, &(package.data), package.dataSize);
        add(fileInfo, filesInfo);
        i++;
    }
    return filesInfo;
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
    if(fileDescriptor == NULL) return -1;
    fseek(fileDescriptor, 0, SEEK_END);
    fileSize = ftell(fileDescriptor);
    fseek(fileDescriptor, 0, SEEK_SET);
    return ceil((float) fileSize / (float) PACKAGE_SIZE);
}

void enqueueSyncFile(int sockfd, COMMAND_PACKAGE command, int action, USER *user) {
    SYNC_FILE *sync =(SYNC_FILE*) malloc(sizeof(SYNC_FILE));

    sync->sockfd = sockfd;
    sync->filename = (char*) malloc(sizeof(command.filename));
    strcpy(sync->filename, command.filename);
    sync->action = action;
    add(sync, user->sync_queue);
}
