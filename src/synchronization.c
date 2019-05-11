#include "../include/synchronization.h"

#define MAX_EVENTS 64
#define FILENAME_LEN 64
#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN MAX_EVENTS * (EVENT_SIZE + FILENAME_LEN)

LIST *ignore_list = NULL;
pthread_mutex_t ignore_list_lock;
int j = 0;

void* handleLocalChanges(void *sockfd) {
    int socket = *(int *) sockfd;
    int inotifyfd, watch_dir;
    char buffer[BUF_LEN];
    char path[FILENAME_LEN];
    int i, length;

    if(!ignore_list)
        if ((ignore_list = createList()) == NULL)
            perror("Could not create ignore list");

    pthread_mutex_lock(&ignore_list_lock);
    for (j = 0; j < 10000000; j++);
    pthread_mutex_unlock(&ignore_list_lock);
    // printf("%d\n", j);

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
                if ( event->mask & IN_CLOSE_WRITE && !(event->mask & !IN_ISDIR) ) {
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
    char *filename = NULL;
    int socket = *(int *) sockfd;

    if(!ignore_list)
        if ((ignore_list = createList()) == NULL)
            perror("Could not create ignore list");

    pthread_mutex_lock(&ignore_list_lock);
    // printf("%d\n", j);
    pthread_mutex_unlock(&ignore_list_lock);

    filename = (char*) malloc(sizeof("teste.txt"));
    strcpy(filename, "teste.txt");
    add(filename, ignore_list);

    filename = (char*) malloc(sizeof("teste2.txt"));
    strcpy(filename, "teste2.txt");
    add(filename, ignore_list);

    filename = (char*) malloc(sizeof("teste3.txt"));
    strcpy(filename, "teste3.txt");
    add(filename, ignore_list);

    print_ignore_list();
    receiveServerNotification(socket);

    return NULL;
}

void print_ignore_list() {
    NODE *current = ignore_list->head;
    char* filename;
    while(current != NULL) {
        filename = (char*) current->data;
        printf("Filename: %s\n", filename);
        current = current->next;
    }
    printf("FIM DA LISTA DE FILENAMES\n");
}
