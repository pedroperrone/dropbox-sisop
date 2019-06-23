#include "../include/replica_manager.h"
#include "../include/connection.h"
#include "../include/user.h"
#include <stdlib.h>
#include <sys/select.h>
#include <pthread.h>
#include <time.h>

int id, port, primary_id, num_replica_managers;
ROLE role;
REPLICA_MANAGER *replica_managers; // Array of replica_managers indexed by id.

int main(int argc, char *argv[]) {
  pthread_t receive_election_thread;

  if(argc < 3 || (argc - 3) % 2 != 0) {
    printf("Usage: %s <port> <id> <hostname1> <port1> ...\n"
           "where:\n"
           "<port>: network port where this RM accepts connections.\n"
           "<id>: id of this RM. The RM with greatest id is the primary.\n"
           "<hostnameN> <portN> ...: hostname and port of the other RMs, ordered by id.\n", argv[0]);
    return 1;
  }

  setReadFromSocketFunction(readSocketServer);
  setWriteInSocketFunction(writeSocketServer);

  port = atoi(argv[1]);
  id = atoi(argv[2]);
  num_replica_managers = (argc - 3) / 2 + 1;
  primary_id = num_replica_managers - 1;
  role = (id == primary_id)? PRIMARY : BACKUP;

  printf("id: %d\n"
         "Num. replica managers: %d\n"
         "Role: %s\n", id, num_replica_managers, role == PRIMARY? "primary" : "backup");

  replica_managers = calloc(num_replica_managers, sizeof(REPLICA_MANAGER));

  // Read list of hostnames and ports
  for (int list_index = 0, rm_id = 0; rm_id < num_replica_managers; list_index++, rm_id++) {
    if (rm_id == id) {
      replica_managers[rm_id].valid = 0;
      list_index--;
      continue;
    }

    replica_managers[rm_id].valid = 1;
    replica_managers[rm_id].hostname = strdup(argv[3 + 2 * list_index]);
    replica_managers[rm_id].port = atoi(argv[3 + 2 * list_index + 1]);
  }

  printf("\n");

  connectToOtherReplicaManagers(port);

  pthread_create(&receive_election_thread, NULL, receiveElectionAndCoordinator, NULL);

  if(initializeUsersList() == 0) {
    perror("Error initializing users list\n");
  }

  printf("Ready!\n\n");
  if (role == PRIMARY) {
    primary();
  }
  else {
    backup();
  }
}

void connectToOtherReplicaManagers(int port) {
  int server_socket, addrlen;
  struct sockaddr_in cliendAddress;
  int rm_id;
  int socket;
  RM_SOCKET_TYPE socket_type;
  
  // Wait connections from RMs with smaller id.
  if (id > 0) {
    int num_sockets = NUM_SOCKETS_PER_RM * id;

    server_socket = initializeMainSocket(port, num_sockets);

    addrlen = sizeof (struct sockaddr_in);

    printf("Waiting for connections\n");
    for (int i = 0; i < num_sockets; i++) {

      if ((socket = accept(server_socket, (struct sockaddr *) &cliendAddress, (socklen_t *) &addrlen)) < 0) {
          perror("accept");
          exit(EXIT_FAILURE);
      }
      
      readAmountOfBytes(&rm_id, socket, sizeof(int));
      readAmountOfBytes(&socket_type, socket, sizeof(RM_SOCKET_TYPE));

      // The SEND_ELECTION on one side of the channel is the RECEIVE_ELECTION on the other side.
      if (socket_type == RECEIVE_ELECTION) {
        socket_type = SEND_ELECTION;
      }
      else if (socket_type == SEND_ELECTION) {
        socket_type = RECEIVE_ELECTION;
      }

      printf("Received connection from RM %d with socket type %d\n", rm_id, socket_type);

      replica_managers[rm_id].sockets[socket_type] = socket;
    }

    close(server_socket);

    printf("\n");
  }

  // Connect to RMs with greater id.
  for (int rm_id = id + 1; rm_id < num_replica_managers; rm_id++) {
    for (int socket_type = 0; socket_type < NUM_SOCKETS_PER_RM; socket_type++) {
      printf("Trying to connect to RM %d with socket type %d...", rm_id, socket_type);
      fflush(stdout);

      socket = createSocket(replica_managers[rm_id].hostname, replica_managers[rm_id].port);

      write(socket, &id, sizeof(int));
      write(socket, &socket_type, sizeof(RM_SOCKET_TYPE));

      printf(" Connected\n");

      replica_managers[rm_id].sockets[socket_type] = socket;
    }
    printf("\n");
  }

  printf("Connected to all RMs!\n");
}

void* receiveElectionAndCoordinator(void *arg) {
  fd_set readfds;
  ELECTION_MESSAGE message;

  while (1) {
    // Clear the file descriptor set.
    FD_ZERO(&readfds);

    for (int rm_id = 0; rm_id < num_replica_managers; rm_id++) {
      if (!replica_managers[rm_id].valid) continue;

      // Add socket to the file descriptor set.
      FD_SET(replica_managers[rm_id].sockets[RECEIVE_ELECTION], &readfds);
    }
    // Wait until any of the sockets in the set become ready for read.
    if (select(FD_SETSIZE, &readfds, NULL, NULL, NULL) < 0) {
      perror("select");
      exit(-1);
    }
    
    // Read the sockets.
    for (int rm_id = 0; rm_id < num_replica_managers; rm_id++) {
      if (!replica_managers[rm_id].valid) continue;

      int socket = replica_managers[rm_id].sockets[RECEIVE_ELECTION];

      // Test if socket is set as ready for read.
      if (FD_ISSET(socket, &readfds)) {
        if (readAmountOfBytes(&message, socket, sizeof(ELECTION_MESSAGE)) != 0) {
          // The RM is down. Ignore.
          continue;
        }

        if (message == ELECTION) {
          message = ANSWER;
          write(socket, &message, sizeof(ELECTION_MESSAGE));
        }
        else if (message == COORDINATOR) {
          if (rm_id != primary_id) {
            replica_managers[primary_id].valid = 0;
          }

          primary_id = rm_id;
          printf("The primary now is %d\n", primary_id);
        }
        else {
          fprintf(stderr, "Error: receiveElectionAndCoordinator: invalid message\n");
          exit(-1);
        }
      }
    }
  }
}

void primary() {
  int server_fd;
  USER *user;
  int rmSockets[num_replica_managers];

  for (int rm_id = 0; rm_id < num_replica_managers; rm_id++) {
    rmSockets[rm_id] = replica_managers[rm_id].sockets[REPLICATION];
  }

  setRmSockets(rmSockets, num_replica_managers);
  server_fd = initializeMainSocket(port, 10);
  while(1) {
    user = handleNewRequest(server_fd);
    replicateSession(user);
  }
}

void backup() {
  while (1) {
    int socket = replica_managers[primary_id].sockets[REPLICATION];
    //USER *user = NULL;
    COMMAND_PACKAGE commandPackage;

    if (receiveCommandPackage(&commandPackage, socket) != 0) {
      replica_managers[primary_id].valid = 0;
      startElection();
      if (role == PRIMARY) break;
    }
    else {
      switch (commandPackage.command) {
      case UPLOAD:
        printf("UPLOAD\n");
        receiveFile(socket, commandPackage, SERVER);
        break;
      case DELETE:
        printf("DELETE\n");
        deleteFile(socket, commandPackage, SERVER);
        break;
      case CREATE_SESSION:
        printf("CREATE SESSION\n");
        receiveUserInfo(socket);
        //printUsers();
        break;
      default:
        break;
      }
    }
  }

  notifyClients();
  primary();
}

void replicateSession(USER *user) {
  int socket;
  if (user == NULL) return;

  printf("Replicando user %s\n", user->username);
  for (int rm_id = 0; rm_id < num_replica_managers; rm_id++) {
    if (!replica_managers[rm_id].valid) continue;
    socket = replica_managers[rm_id].sockets[REPLICATION];
    sendCreateSession(user, socket);
  }
}

void startElection() {
  ELECTION_MESSAGE message;
  fd_set readfds;
  struct timespec coordinator_timeout;

  printf("Starting election\n");

  // Send ELECTION to RMs with greater id
  for (int rm_id = id + 1; rm_id < num_replica_managers; rm_id++) {
    if (!replica_managers[rm_id].valid) continue;

    int socket = replica_managers[rm_id].sockets[SEND_ELECTION];

    message = ELECTION;
    write(socket, &message, sizeof(ELECTION_MESSAGE));
  }

  // Wait for ANSWER
  int try_again;
  do {
    // Clear the file descriptor set.
    FD_ZERO(&readfds);

    for (int rm_id = id + 1; rm_id < num_replica_managers; rm_id++) {
      if (!replica_managers[rm_id].valid) continue;

      int socket = replica_managers[rm_id].sockets[SEND_ELECTION];

      // Add socket to the file descriptor set.
      FD_SET(socket, &readfds);
    }

    switch (receivedAnswer(&readfds)) {
      case TIMEOUT:
        try_again = 0;
        break;
      
      case RECEIVED_ANSWER:
        coordinator_timeout.tv_sec = COORDINATOR_TIMEOUT_SECONDS;
        coordinator_timeout.tv_nsec = COORDINATOR_TIMEOUT_NANOSECONDS;

        // Wait a while for the receive_election_thread to receive the COORDINATOR message.
        nanosleep(&coordinator_timeout, NULL);

        printf("Election finished\n");
        return;
      
      case TRY_AGAIN:
        try_again = 1;
        break;
    }
  } while (try_again);

  // Didn't receive answers.
  role = PRIMARY;
  primary_id = id;

  printf("Now I am the primary\n");

  // Send COORDINATOR to the other RMs
  for (int rm_id = 0; rm_id < num_replica_managers; rm_id++) {
    if (rm_id == id) continue;

    int socket = replica_managers[rm_id].sockets[SEND_ELECTION];

    message = COORDINATOR;
    write(socket, &message, sizeof(ELECTION_MESSAGE));
  }

  printf("Election finished\n");
}

// Waits for an answer, with a timeout specified by answer_timeout.
RECEIVED_ANSWER_RETVAL receivedAnswer(fd_set *readfds) {
  struct timeval answer_timeout;

  answer_timeout.tv_sec = ANSWER_TIMEOUT_SECONDS;
  answer_timeout.tv_usec = ANSWER_TIMEOUT_MICROSECONDS;

  int retval = select(FD_SETSIZE, readfds, NULL, NULL, &answer_timeout);

  if (retval < 0) {
    perror("select");
    exit(-1);
  }
  else if (retval == 0) {
    // Timeout
    return TIMEOUT;
  }
  else {
    int received_answer = 0;
    for (int rm_id = id + 1; rm_id < num_replica_managers; rm_id++) {
      int socket = replica_managers[rm_id].sockets[SEND_ELECTION];

      // Test if socket is set as ready for read.
      if (FD_ISSET(socket, readfds)) {
        ELECTION_MESSAGE message;

        if (readAmountOfBytes(&message, socket, sizeof(ELECTION_MESSAGE)) != 0) {
          // rm_id is down.
          replica_managers[rm_id].valid = 0;
          continue;
        }

        if (message == ANSWER) {
          received_answer = 1;
        }
        else {
          fprintf(stderr, "Error: start_election: wrong message, expected ANSWER\n");
          exit(-1);
        }
      }
    }

    if (received_answer) {
      return RECEIVED_ANSWER;
    }
    else {
      return TRY_AGAIN;
    }
  }
}
