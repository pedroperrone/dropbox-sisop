#ifndef DROPBOX_H
#define DROPBOX_H

typedef enum socket_type {
    REQUEST,
    NOTIFY_CLIENT,
    NOTIFY_SERVER,
    NUMBER_OF_SOCKET_TYPES
} SOCKET_TYPE;

typedef enum location {
    SERVER,
    CLIENT
} LOCATION;

#endif // DROPBOX_H