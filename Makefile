CC=gcc
LIB_DIR=./lib
INC_DIR=./include
BIN_DIR=./bin
SRC_DIR=./src
FLAGS=-Wall

all: main.o example.o
	$(CC) -o dropbox $(BIN_DIR)/*.o

main.o: $(SRC_DIR)/main.c
	$(CC) -c -o $(BIN_DIR)/main.o $(SRC_DIR)/main.c $(FLAGS)

example.o: $(SRC_DIR)/example.c
	$(CC) -c -o $(BIN_DIR)/example.o $(SRC_DIR)/example.c $(FLAGS)

clean:
	rm $(BIN_DIR)/*.o dropbox


