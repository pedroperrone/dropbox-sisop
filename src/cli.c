#include "../include/cli.h"

void read_line(char *command, char *argument) {
    char input[100];

    command[0] = '\0';
    argument[0] = '\0';

    printf(">> ");
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%s %s", command, argument);
}

void upload(int socketDescriptor, char *file_name) {
    FILE *file = fopen(file_name, "r");
    if(file == NULL) {
        printf("Error opening file");
    }
    sendFile(file, socketDescriptor, file_name);
}

void download(int socketDescriptor, char *file_name) {
    printf("TODO\n");
}

void delete(int socketDescriptor, char *file_name) {
    sendRemove(socketDescriptor, file_name);
}

void list_server(int socketDescriptor) {
    printf("TODO\n");
}

void list_client(int socketDescriptor) {
    printf("TODO\n");
}

void get_sync_dir(int socketDescriptor) {
    printf("TODO\n");
}

void cli(int socketDescriptor) {
    char command[MAX_COMMAND_SIZE];
    char argument[MAX_COMMAND_SIZE];

    while(TRUE) {
        read_line(command, argument);

        if(strcmp(command, "upload") == 0) {
            upload(socketDescriptor, argument);
        } else if(strcmp(command, "download") == 0){
            download(socketDescriptor, argument);
        } else if(strcmp(command, "delete") == 0){
            delete(socketDescriptor, argument);
        } else if(strcmp(command, "list_server") == 0){
            list_server(socketDescriptor);
        } else if(strcmp(command, "list_client") == 0){
            list_client(socketDescriptor);
        } else if(strcmp(command, "get_sync_dir") == 0){
            get_sync_dir(socketDescriptor);
        } else if(strcmp(command, "exit") == 0){
            break;
        } else {
            printf("Available commands:\n");
            printf("  upload <path/filename.ext>\n");
            printf("  download <filename.ext>\n");
            printf("  delete <filename.ext>\n");
            printf("  list_server\n");
            printf("  list_client\n");
            printf("  get_sync_dir\n");
            printf("  exit\n");
        }
    }
}

