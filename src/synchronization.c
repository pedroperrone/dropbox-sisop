#include "../include/synchronization.h"

#define MAX_EVENTS 64
#define FILENAME_LEN 64
#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN MAX_EVENTS * (EVENT_SIZE + FILENAME_LEN)

void* handleLocalChanges(void *sockfd) {
    int socket = *(int *) sockfd;
    int inotifyfd, watch_dir;
    char buffer[BUF_LEN];
    char path[FILENAME_LEN];
    int i, length;

    if ((inotifyfd = inotify_init()) < 0 ) {
        perror("Couldn't initialize inotify");
    }

    watch_dir = inotify_add_watch(inotifyfd, "sync_dir", IN_CREATE | IN_MODIFY | IN_DELETE);

    if (watch_dir == -1)
        perror("Couldn't add watch to sync_dir");

    while(1) {
        i = 0;
        length = read(inotifyfd, buffer, BUF_LEN);

        while ( i < length ) {
            struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
            if ( event->len ) {
                strcpy(path, "sync_dir/");
                strcat(path, event->name);
                if ( event->mask & IN_CREATE && !(event->mask & IN_ISDIR) ) {
                    // File created
                    upload(socket, path);
                }
                if ( event->mask & IN_DELETE && !(event->mask & IN_ISDIR) ) {
                    // File deleted
                    delete(socket, event->name);
                }
                if ( event->mask & IN_MODIFY && !(event->mask & !IN_ISDIR) ) {
                    // File modified
                    upload(socket, path);
                }
            }
            i += EVENT_SIZE + event->len;
        }
    }

    inotify_rm_watch(inotifyfd, watch_dir);
    close(inotifyfd);
    return NULL;
}

void* handleRemoteChanges(void *sockfd) {
    printf("TODO: handle remote changes\n");
    return NULL;
}

