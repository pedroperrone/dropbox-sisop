#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "../include/connection.h"
#include "../include/user.h"
#include "../include/cli.h"
#include "../include/synchronization.h"

int main(int argc, char *argv[]) {
    int sockfd[NUMBER_OF_SOCKET_TYPES];
    char *hostname;
    char *username;
    int port;
    pthread_t handleLocalChangesThread;
    pthread_t handleRemoteChangesThread;

    if (argc != 4) {
        fprintf(stderr, "Usage: %s username hostname port\n", argv[0]);
        exit(0);
    }

    username = argv[1];
    hostname = argv[2];
    port = atoi(argv[3]);

    sockfd[REQUEST] = createSocket(REQUEST, username, hostname, port);
    sockfd[NOTIFY_CLIENT] = createSocket(NOTIFY_CLIENT, username, hostname, port);
    sockfd[NOTIFY_SERVER] = createSocket(NOTIFY_SERVER, username, hostname, port);

    get_sync_dir(sockfd[REQUEST]);

    pthread_create(&handleLocalChangesThread, NULL, handleLocalChanges, (void *)&sockfd[NOTIFY_SERVER]);
    pthread_create(&handleRemoteChangesThread, NULL, handleRemoteChanges, (void *)&sockfd[NOTIFY_CLIENT]);
    cli(sockfd[REQUEST]);

    sendExit(sockfd[REQUEST]);
    shutdown(sockfd[REQUEST], 2);

    sendExit(sockfd[NOTIFY_CLIENT]);
    shutdown(sockfd[NOTIFY_CLIENT], 2);

    sendExit(sockfd[NOTIFY_SERVER]);
    shutdown(sockfd[NOTIFY_SERVER], 2);

    return 0;
}
