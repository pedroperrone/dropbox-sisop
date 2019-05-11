#include "../include/cli.h"

void read_line(char *command, char *argument) {
    char input[100];

    command[0] = '\0';
    argument[0] = '\0';

    printf(">> ");
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%s %s", command, argument);
}

void upload(int socketDescriptor, char *file_path) {
    FILE *file = fopen(file_path, "r");
    const char s[2] = "/";
    char *token, *filename;

    token = strtok(file_path, s);
   
    while( token != NULL ) {
        filename = token;
        token = strtok(NULL, s);
    }

    if(file == NULL)
        printf("Error opening file\n");
    else {
        sendFile(file, socketDescriptor, filename);
        fclose(file);
    }
}

void download(int socketDescriptor, char *file_name) {
    printf("TODO\n");
}

void delete(int socketDescriptor, char *file_name) {
    sendRemove(socketDescriptor, file_name);
}

void list_server(int socketDescriptor) {
    LIST *filesInfo = getListServer(socketDescriptor);
    printListOfFileInfo(filesInfo);
    destroy(filesInfo);
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

    sleep(0.1);
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
        } else if(strcmp(command, "") == 0){
            // pass
        } else if(strcmp(command, "?") == 0){
            printf("Available commands:\n");
            printf("  upload <path/filename.ext>\n");
            printf("  download <filename.ext>\n");
            printf("  delete <filename.ext>\n");
            printf("  list_server\n");
            printf("  list_client\n");
            printf("  get_sync_dir\n");
            printf("  exit\n");
        } else {
            printf("Invalid command. Type '?' for help.\n");
        }
    }
}

void printListOfFileInfo(LIST *fileInfos) {
    struct stat fileStat;
    NODE *current = fileInfos->head;
    FILE_INFO *fileInfo;
    char dateString[DATE_STRING_LENTH];
    printf("Created at\t\t\tModified at\t\t\tAccesses at\t\t\tFile Name\n");
    while (current != NULL) {
        fileInfo = (FILE_INFO *)current->data;
        memcpy(&fileStat, &(fileInfo->details), sizeof(struct stat));
        strncpy(dateString, ctime(&(fileStat.st_ctime)), DATE_STRING_LENTH);
        dateString[strlen(dateString) - 1] = '\0';
        printf("%s\t", dateString);
        strncpy(dateString, ctime(&(fileStat.st_mtime)), DATE_STRING_LENTH);
        dateString[strlen(dateString) - 1] = '\0';
        printf("%s\t", dateString);
        strncpy(dateString, ctime(&(fileStat.st_atime)), DATE_STRING_LENTH);
        dateString[strlen(dateString) - 1] = '\0';
        printf("%s\t", dateString);
        printf("%s\n", fileInfo->filename);
        current = current->next;
    }
}
