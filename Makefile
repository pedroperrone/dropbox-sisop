CC=gcc
LIB_DIR=./lib
INC_DIR=./include
BIN_DIR=./bin
SRC_DIR=./src
FLAGS=-Wall

all: server client

server: server.o connection.o
	$(CC) -o dropboxServer $(BIN_DIR)/connection.o $(BIN_DIR)/server.o $(BIN_DIR)/user.o $(BIN_DIR)/linked_list.o $(FLAGS)

server.o: $(SRC_DIR)/server.c
	$(CC) -c -o $(BIN_DIR)/server.o $(SRC_DIR)/server.c $(FLAGS)

connection.o: $(SRC_DIR)/connection.c user.o
	$(CC) -c -o $(BIN_DIR)/connection.o $(SRC_DIR)/connection.c $(FLAGS)

client: client.o connection.o
	$(CC) -o dropboxClient $(BIN_DIR)/client.o $(BIN_DIR)/connection.o  $(BIN_DIR)/user.o $(BIN_DIR)/linked_list.o $(FLAGS)

client.o: $(SRC_DIR)/client.c
	$(CC) -c -o $(BIN_DIR)/client.o $(SRC_DIR)/client.c $(FLAGS)

user.o: $(SRC_DIR)/user.c linked_list.o
	$(CC) -c -o $(BIN_DIR)/user.o $(SRC_DIR)/user.c $(FLAGS)

linked_list.o: $(SRC_DIR)/linked_list.c
	$(CC) -c -o $(BIN_DIR)/linked_list.o $(SRC_DIR)/linked_list.c $(FLAGS)

clean:
	rm $(BIN_DIR)/*.o dropboxServer dropboxClient


