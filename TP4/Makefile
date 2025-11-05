CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude
LDFLAGS = -Linclude -lseg

SRC = ipc_client.c
BIN = build/ipc_client

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(BIN)

.PHONY: all clean
