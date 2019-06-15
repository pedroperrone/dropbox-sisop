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
#include "../include/frontend.h"

int main(int argc, char *argv[]) {
    char *hostname;
    char *username;
    int port;
    pthread_t handleLocalChangesThread;
    pthread_t handleRemoteChangesThread;
    int *notifyServerSocket = (int *)malloc(sizeof(int)), *notifyClientSocket = (int *)malloc(sizeof(int));

    if (argc != 4) {
        fprintf(stderr, "Usage: %s username hostname port\n", argv[0]);
        exit(0);
    }

    username = argv[1];
    hostname = argv[2];
    port = atoi(argv[3]);

    // sockfd[REQUEST] = createSocket(REQUEST, username, hostname, port);
    // sockfd[NOTIFY_CLIENT] = createSocket(NOTIFY_CLIENT, username, hostname, port);
    // sockfd[NOTIFY_SERVER] = createSocket(NOTIFY_SERVER, username, hostname, port);

    initializeFrontend(hostname, port, username);

    get_sync_dir(getSocketByType(REQUEST));

    *notifyClientSocket = getSocketByType(NOTIFY_CLIENT);
    *notifyServerSocket = getSocketByType(NOTIFY_SERVER);

    pthread_create(&handleLocalChangesThread, NULL, handleLocalChanges, (void *)notifyServerSocket);
    pthread_create(&handleRemoteChangesThread, NULL, handleRemoteChanges, (void *)notifyClientSocket);
    cli();

    sendExit(getSocketByType(REQUEST));
    shutdown(getSocketByType(REQUEST), 2);

    sendExit(getSocketByType(REQUEST));
    shutdown(getSocketByType(REQUEST), 2);

    sendExit(getSocketByType(REQUEST));
    shutdown(getSocketByType(REQUEST), 2);

    return 0;
}
