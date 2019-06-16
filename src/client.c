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
    int port, localPort;
    pthread_t handleLocalChangesThread;
    pthread_t handleRemoteChangesThread;

    if (argc != 5) {
        fprintf(stderr, "Usage: %s username hostname port local_port\n", argv[0]);
        exit(0);
    }

    username = argv[1];
    hostname = argv[2];
    port = atoi(argv[3]);
    localPort = atoi(argv[4]);

    initializeFrontend(hostname, port, username, localPort);

    get_sync_dir(getSocketByType(REQUEST));

    pthread_create(&handleLocalChangesThread, NULL, handleLocalChanges, NULL);
    pthread_create(&handleRemoteChangesThread, NULL, handleRemoteChanges, NULL);
    cli();

    sendExit(getSocketByType(REQUEST));
    shutdown(getSocketByType(REQUEST), 2);

    sendExit(getSocketByType(NOTIFY_CLIENT));
    shutdown(getSocketByType(NOTIFY_CLIENT), 2);

    sendExit(getSocketByType(NOTIFY_SERVER));
    shutdown(getSocketByType(NOTIFY_SERVER), 2);

    return 0;
}
