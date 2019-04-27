#include "../include/connection.h"

int main(int argc, char *argv[]) {
    int server_fd;
    struct sockaddr_in address;
    initializeMainSocket(&server_fd, &address);
    while(1) {
        handleNewRequest(server_fd, address);
    }
    close(server_fd);
    return 0;
}
