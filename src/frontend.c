#include "../include/frontend.h"

struct sockaddr_in serv_addr;
struct hostent *server;
NETWORK_ADDRESS serverAddress;
int sockfd[NUMBER_OF_SOCKET_TYPES];
char username[USERNAME_LENGTH];
int mainLocalPort;

void initializeFrontend(char* hostname, int port, char* local_username, int localPort) {
    int i;
    pthread_t deamonThread;
    strncpy(username, local_username, USERNAME_LENGTH);
    setNewAddress(hostname, port);
    mainLocalPort = localPort;
    setReadFromSocketFunction(readSocketFrontend);
    setWriteInSocketFunction(writeSocketFrontend);
    for(i = 0; i < NUMBER_OF_SOCKET_TYPES; i++) {
        if ((sockfd[i] = socket(AF_INET, SOCK_STREAM, 0)) == -1)
            fprintf(stderr, "ERROR opening socket\n");
    }
    reconnectSockets();
    pthread_create(&deamonThread, NULL, waitForNewMainServer, NULL);
}

void setNewAddress(char *hostname, int port) {
    strncpy((char *)&(serverAddress.ip), hostname, IP_LENGTH);
    serverAddress.port = port;
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(1);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8);
}

void reconnectSockets() {
    int id;
    int sockfd_REQUEST = socket(AF_INET, SOCK_STREAM, 0);
    int sockfd_NOTIFY_CLIENT = socket(AF_INET, SOCK_STREAM, 0);
    int sockfd_NOTIFY_SERVER = socket(AF_INET, SOCK_STREAM, 0);

    close(sockfd[REQUEST]);
    close(sockfd[NOTIFY_CLIENT]);
    close(sockfd[NOTIFY_SERVER]);

    id = connectSocket(REQUEST, username, serv_addr, sockfd_REQUEST, mainLocalPort, -1);
    connectSocket(NOTIFY_CLIENT, username, serv_addr, sockfd_NOTIFY_CLIENT, mainLocalPort, id);
    connectSocket(NOTIFY_SERVER, username, serv_addr, sockfd_NOTIFY_SERVER, mainLocalPort, id);

    sockfd[REQUEST] = sockfd_REQUEST;
    sockfd[NOTIFY_CLIENT] = sockfd_NOTIFY_CLIENT;
    sockfd[NOTIFY_SERVER] = sockfd_NOTIFY_SERVER;
}

int getSocketByType(SOCKET_TYPE type) {
    return sockfd[type];
}

void* waitForNewMainServer() {
    int newMainServer_fd;
    newMainServer_fd = initializeMainSocket(mainLocalPort, 3);
    while(1) {
        updateSocket(newMainServer_fd);
    }
    return NULL;
}

void updateSocket(int newMainServer_fd) {
    int new_socket, addrlen;
    struct sockaddr_in cliendAddress;
    char hostname[IP_LENGTH];
    int port;
    if ((new_socket = accept(newMainServer_fd, (struct sockaddr *)&cliendAddress, (socklen_t *)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    setReadFromSocketFunction(readSocketServer);
    setWriteInSocketFunction(writeSocketServer);

    getPort(new_socket, &port);
    getAddressFromSocket(new_socket, hostname);
    setNewAddress(hostname, port);

    setReadFromSocketFunction(readSocketFrontend);
    setWriteInSocketFunction(writeSocketFrontend);

    sleep(1);
    reconnectSockets();
    close(new_socket);

}

int readSocketFrontend(int type, void* destiny, int bytesToRead) {
    int readBytes = -1;
    while(readBytes <= 0) {
        readBytes = read(getSocketByType(type), destiny, bytesToRead);
        if(readBytes <= 0) {
            sleep(1);
        }
    }
    return readBytes;
}

int writeSocketFrontend(int type, void* source, int bytesToWrite) {
    int sockfd = getSocketByType(type), readBytes = -1;
    while (readBytes < 0) {
        readBytes = write(sockfd, source, bytesToWrite);
        if (readBytes < 0) {
            sleep(1);
        }
    }
    return readBytes;
}
