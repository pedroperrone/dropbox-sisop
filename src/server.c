#include "../include/connection.h"

int main(int argc, char *argv[]) {
    int server_fd;
    struct sockaddr_in address;
    if(argc < 2) {
        printf("Missing parameter: port");
        return 1;
    }
    setPort(atoi(argv[1]));
    initializeMainSocket(&server_fd, &address);
    while(1) {
        handleNewRequest(server_fd, address);
    }
    close(server_fd);
    return 0;
}
