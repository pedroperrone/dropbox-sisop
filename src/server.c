#include "../include/connection.h"
#include "../include/user.h"

int main(int argc, char *argv[]) {
    int server_fd;
    
    if(argc < 2) {
        printf("Missing parameter: port\n");
        return 1;
    }
    if(initializeUsersList() == 0) {
        perror("Error initializing users list\n");
    }
    int port = atoi(argv[1]);
    server_fd = initializeServerSocket(port, 5);
    while(1) {
        handleNewRequest(server_fd);
    }
    close(server_fd);
    return 0;
}
