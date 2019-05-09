#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/inotify.h>
#include <netinet/in.h>
#include <netdb.h>
#include "../include/connection.h"
#include "../include/user.h"
#include "../include/cli.h"

int main(int argc, char *argv[]) {
    int sockfd;
    char *hostname;
    char *username;
    int port;

    if (argc != 4) {
        fprintf(stderr, "usage %s hostname username port\n", argv[0]);
        exit(0);
    }

    hostname = argv[1];
    username = argv[2];
    port = atoi(argv[3]);

    sockfd = createSocket(REQUEST, hostname, username, port);

    if (sockfd < 0) {
        return 1;
    }

    cli(sockfd);

    sendExit(sockfd);
    shutdown(sockfd, 2);

    return 0;
}
