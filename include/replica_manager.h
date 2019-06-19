#ifndef REPLICA_MANAGER_H
#define REPLICA_MANAGER_H

#define NUM_SOCKETS_PER_RM 3

typedef enum ROLE {
  PRIMARY,
  BACKUP
} ROLE;

typedef struct REPLICA_MANAGER {
  char *hostname;
  int port;
  int sockets[NUM_SOCKETS_PER_RM];
} REPLICA_MANAGER;

typedef enum RM_SOCKET_TYPE {
  REPLICATION,
  RECEIVE_ELECTION,
  SEND_ELECTION
} RM_SOCKET_TYPE;

void connectToOtherReplicaManagers();

#endif // REPLICA_MANAGER_H