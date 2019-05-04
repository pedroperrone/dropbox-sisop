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

//colocar as constantes num util.h mais além
#define PORT 4000
#define FALSE 0
#define TRUE 1

#define MAXNAME 25
#define MAXFILES 50
#define MAXPATH MAXFILES*MAXNAME

int main(int argc, char *argv[])
{
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    // char buffer[256];
    if (argc < 4)
    {
        fprintf(stderr, "usage %s hostname username filename\n", argv[0]);
        exit(0);
    }

    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        printf("ERROR opening socket\n");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        printf("ERROR connecting\n");

    // printf("Enter the message: ");
    // bzero(buffer, 256);
    // fgets(buffer, 256, stdin);

    // /* write in the socket */
    // n = write(sockfd, buffer, strlen(buffer));
    // if (n < 0)
    //     printf("ERROR writing to socket\n");

    // bzero(buffer, 256);

    // /* read from the socket */
    // n = read(sockfd, buffer, 256);
    // if (n < 0)
    //     printf("ERROR reading from socket\n");

    // printf("%s\n", buffer);

    n = write(sockfd, argv[2], USERNAME_LENGTH);

    FILE *file = fopen(argv[3], "r");
    if(file == NULL) {
        printf("Error opening file");
        return 0;
    }
    sendFile(file, sockfd, argv[3]);

    sendRemove(sockfd, "received_file.txt");

    sendExit(sockfd);
    shutdown(sockfd, 2);
    // close(sockfd);

    return 0;
}

void cli(){
    char *command;
    char *commandLine[MAXPATH];
    char *pathOrFilename;
    int done = FALSE;

    while(done != TRUE){

        printf("\n Waiting commands\n");

        if(fgets(commandLine,sizeof(commandLine),stdin)!= NULL){
          commandLine[strcspn(commandLine, "\r\n")] = 0;
          command = strtok(commandLine, " ");
          pathOrFilename = strtok(NULL, " ");

          if(strcmp(command, "upload") == 0){
            //sendFile(fileDescriptor, socketDescriptor, pathOrFilename);
            printf("upload: %s\n", pathOrFilename); //debug
          }
          else if(strcmp(command, "download") == 0){
            //getfile
            printf("download: %s\n", pathOrFilename); //debug
          }
          else if(strcmp(command, "list_server") == 0){
            //listServer();
            printf("list server");// debug
          }
          else if(strcmp(command, "list_client") == 0){
            //listClient();
            printf("list client");// debug
          }
          else if(strcmp(command, "get_sync_dir") == 0){
            //getSyncDir();
            printf("get sync dir");// debug
          }
          else if(strcmp(command, "delete") == 0){
            //deleteFile();
            printf("delete %s\n", pathOrFilename);
          }
          else if(strcmp(command, "exit") == 0){
            //done = TRUE;
            printf("adeus");
          }

        }else
          printf("\n ERROR");
    }

}
