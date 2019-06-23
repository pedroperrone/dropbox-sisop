#include "../include/connection.h"
#include "../include/user.h"

int main(int argc, char *argv[]) {
    int server_fd;
    int port = atoi(argv[1]);
    
    if(argc < 2) {
        printf("Missing parameter: port\n");
        return 1;
    }
    if(initializeUsersList() == 0) {
        perror("Error initializing users list\n");
    }

    setReadFromSocketFunction(readSocketServer);
    setWriteInSocketFunction(writeSocketServer);

    server_fd = initializeMainSocket(port, 5);
    while(1) {
        handleNewRequest(server_fd);
    }
    close(server_fd);
    return 0;
}
