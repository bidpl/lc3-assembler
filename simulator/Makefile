CC=gcc
CFLAGS=-g -Wall -Wextra -std=c99
BIN=bm3sim.exe
TESTFILE=factorial.obj

all: simulator.c
	$(CC) $(CFLAGS) simulator.c -o $(BIN)

test: $(BIN) $(TESTFILE)
	.\$(BIN) $(TESTFILE)

clean:
	rm -f bm3sim.exe