#include "../include/replica_manager.h"
#include "../include/connection.h"
#include <stdlib.h>

int id, primary_id, num_replica_managers;
ROLE role;
REPLICA_MANAGER *replica_managers; // Array of replica_managers indexed by id.

int main(int argc, char *argv[]) {
  if(argc < 3 || (argc - 3) % 2 != 0) {
    printf("Usage: %s <port> <id> <hostname1> <port1> ...\n"
           "where:\n"
           "<port>: network port where this RM accepts connections.\n"
           "<id>: id of this RM. The RM with greatest id is the primary.\n"
           "<hostnameN> <portN> ...: hostname and port of the other RMs, ordered by id.\n", argv[0]);
    return 1;
  }

  int port = atoi(argv[1]);
  id = atoi(argv[2]);
  num_replica_managers = (argc - 3) / 2 + 1;
  role = (id == num_replica_managers - 1)? PRIMARY : BACKUP;

  printf("id: %d\n"
         "Num. replica managers: %d\n"
         "Role: %s\n", id, num_replica_managers, role == PRIMARY? "primary" : "backup");

  replica_managers = calloc(num_replica_managers, sizeof(REPLICA_MANAGER));

  // Read list of hostnames and ports
  for (int list_index = 0, rm_id = 0; rm_id < num_replica_managers; list_index++, rm_id++) {
    if (rm_id == id) {
      list_index--;
      continue;
    }

    replica_managers[rm_id].hostname = strdup(argv[3 + 2 * list_index]);
    replica_managers[rm_id].port = atoi(argv[3 + 2 * list_index + 1]);
  }

  printf("\n");

  connectToOtherReplicaManagers(port);


}

void connectToOtherReplicaManagers(int port) {
  
  // Wait connections from RMs with smaller id.
  if (id > 0) {
    int server_socket, addrlen;
    struct sockaddr_in cliendAddress;
    int num_sockets = NUM_SOCKETS_PER_RM * id;

    server_socket = initializeServerSocket(port, num_sockets);

    addrlen = sizeof (struct sockaddr_in);

    printf("Waiting for connections\n");
    for (int i = 0; i < num_sockets; i++) {
      int rm_id;
      int new_socket;
      RM_SOCKET_TYPE socket_type;

      if ((new_socket = accept(server_socket, (struct sockaddr *) &cliendAddress, (socklen_t *) &addrlen)) < 0) {
          perror("accept");
          exit(EXIT_FAILURE);
      }

      readAmountOfBytes(&rm_id, new_socket, sizeof(int));
      readAmountOfBytes(&socket_type, new_socket, sizeof(RM_SOCKET_TYPE));

      printf("Received connection from RM %d with socket type %d\n", rm_id, socket_type);

      replica_managers[rm_id].sockets[socket_type] = new_socket;
    }

    close(server_socket);

    printf("\n");
  }

  // Connect to RMs with greater id.
  for (int rm_id = id + 1; rm_id < num_replica_managers; rm_id++) {
    for (int socket_type = 0; socket_type < NUM_SOCKETS_PER_RM; socket_type++) {
      printf("Trying to connect to RM %d with socket type %d...", rm_id, socket_type);
      fflush(stdout);

      int socket = createSocket(replica_managers[rm_id].hostname, replica_managers[rm_id].port);

      write(socket, &id, sizeof(int));
      write(socket, &socket_type, sizeof(RM_SOCKET_TYPE));

      printf(" Connected\n");

      replica_managers[rm_id].sockets[socket_type] = socket;
    }
    printf("\n");
  }

  printf("\nConnected to all RMs!\n");
}