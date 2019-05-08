#ifndef __cli__
#define __cli__

#include <stdio.h>
#include <string.h>
#include <linux/inotify.h>
#include "../include/connection.h"

#define MAX_COMMAND_SIZE 100
#define FALSE 0
#define TRUE 1

void read_line(char *command, char *argument);
void upload(int socketDescriptor, char *file_name);
void download(int socketDescriptor, char *file_name);
void delete(int socketDescriptor, char *file_name);
void list_server(int socketDescriptor);
void list_client(int socketDescriptor);
void get_sync_dir(int socketDescriptor);
void cli(int socketDescriptor);

#endif
