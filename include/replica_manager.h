#ifndef REPLICA_MANAGER_H
#define REPLICA_MANAGER_H

#include <sys/select.h>
#include "../include/user.h"

#define NUM_SOCKETS_PER_RM 3

#define ANSWER_TIMEOUT_SECONDS 1
#define ANSWER_TIMEOUT_MICROSECONDS 0

#define COORDINATOR_TIMEOUT_SECONDS 2
#define COORDINATOR_TIMEOUT_NANOSECONDS 0

typedef enum ROLE {
  PRIMARY,
  BACKUP
} ROLE;

typedef struct REPLICA_MANAGER {
  int valid;
  char *hostname;
  int port;
  int sockets[NUM_SOCKETS_PER_RM];
} REPLICA_MANAGER;

typedef enum RM_SOCKET_TYPE {
  REPLICATION,
  RECEIVE_ELECTION,
  SEND_ELECTION
} RM_SOCKET_TYPE;

typedef enum ELECTION_MESSAGE {
  ELECTION,
  ANSWER,
  COORDINATOR
} ELECTION_MESSAGE;

typedef enum RECEIVED_ANSWER_RETVAL {
  RECEIVED_ANSWER,
  TIMEOUT,
  TRY_AGAIN
} RECEIVED_ANSWER_RETVAL;

void connectToOtherReplicaManagers();
void* receiveElectionAndCoordinator(void *);
RECEIVED_ANSWER_RETVAL receivedAnswer(fd_set *readfds);

void primary();
void backup();

void replicateSession(USER *user);
void startElection();

#endif // REPLICA_MANAGER_H