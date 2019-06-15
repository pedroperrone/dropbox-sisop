#include "../include/frontend.h"

struct sockaddr_in serv_addr;
struct hostent *server;
NETWORK_ADDRESS serverAddress;
int sockfd[NUMBER_OF_SOCKET_TYPES];
char username[USERNAME_LENGTH];

void initializeFrontend(char* hostname, int port, char* local_username) {
    int i;
    strncpy(username, local_username, USERNAME_LENGTH);
    setNewAddress(hostname, port);
    for(i = 0; i < NUMBER_OF_SOCKET_TYPES; i++) {
        if ((sockfd[i] = socket(AF_INET, SOCK_STREAM, 0)) == -1)
            fprintf(stderr, "ERROR opening socket\n");
    }
    reconnectSockets();
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
    connectSocket(REQUEST, username, serv_addr, sockfd[REQUEST]);
    connectSocket(NOTIFY_CLIENT, username, serv_addr, sockfd[NOTIFY_CLIENT]);
    connectSocket(NOTIFY_SERVER, username, serv_addr, sockfd[NOTIFY_SERVER]);
}

int getSocketByType(SOCKET_TYPE type) {
    return sockfd[type];
}
