CC=gcc
LIB_DIR=./lib
INC_DIR=./include
BIN_DIR=./bin
SRC_DIR=./src
FLAGS=-lpthread -lm -Wall

all: server client replica_manager

server: server.o connection.o frontend.o
	$(CC) -o dropboxServer $(BIN_DIR)/connection.o $(BIN_DIR)/server.o $(BIN_DIR)/user.o $(BIN_DIR)/linked_list.o $(BIN_DIR)/frontend.o $(FLAGS)

server.o: $(SRC_DIR)/server.c
	$(CC) -c -o $(BIN_DIR)/server.o $(SRC_DIR)/server.c $(FLAGS)

connection.o: $(SRC_DIR)/connection.c user.o
	$(CC) -c -o $(BIN_DIR)/connection.o $(SRC_DIR)/connection.c $(FLAGS)

client: client.o connection.o cli.o synchronization.o frontend.o
	$(CC) -o dropboxClient $(BIN_DIR)/client.o $(BIN_DIR)/connection.o  $(BIN_DIR)/user.o $(BIN_DIR)/linked_list.o $(BIN_DIR)/cli.o $(BIN_DIR)/synchronization.o $(BIN_DIR)/frontend.o $(FLAGS)

client.o: $(SRC_DIR)/client.c
	$(CC) -c -o $(BIN_DIR)/client.o $(SRC_DIR)/client.c $(FLAGS)

replica_manager: replica_manager.o connection.o
	$(CC) -o dropboxRM $(BIN_DIR)/connection.o $(BIN_DIR)/replica_manager.o $(BIN_DIR)/user.o $(BIN_DIR)/linked_list.o $(FLAGS)

replica_manager.o: $(SRC_DIR)/replica_manager.c
	$(CC) -c -o $(BIN_DIR)/replica_manager.o $(SRC_DIR)/replica_manager.c $(FLAGS)

cli.o: $(SRC_DIR)/cli.c
	$(CC) -c -o $(BIN_DIR)/cli.o $(SRC_DIR)/cli.c $(FLAGS)

synchronization.o: $(SRC_DIR)/synchronization.c
	$(CC) -c -o $(BIN_DIR)/synchronization.o $(SRC_DIR)/synchronization.c $(FLAGS)

user.o: $(SRC_DIR)/user.c linked_list.o
	$(CC) -c -o $(BIN_DIR)/user.o $(SRC_DIR)/user.c $(FLAGS)

linked_list.o: $(SRC_DIR)/linked_list.c
	$(CC) -c -o $(BIN_DIR)/linked_list.o $(SRC_DIR)/linked_list.c $(FLAGS)

frontend.o: $(SRC_DIR)/frontend.c connection.o
	$(CC) -c -o $(BIN_DIR)/frontend.o $(SRC_DIR)/frontend.c $(FLAGS)

clean:
	rm -f $(BIN_DIR)/*.o dropboxServer dropboxClient dropboxRM


