#ifndef __synchronization__
#define __synchronization__

#include <stdio.h>
#include <string.h>
#include <sys/inotify.h>
#include "../include/connection.h"
#include "../include/cli.h"

#define MAX_FILENAME_SIZE 100
#define FALSE 0
#define TRUE 1

void* handleLocalChanges(void *sockfd);
void* handleRemoteChanges(void *sockfd);

#endif