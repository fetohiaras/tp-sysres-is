BUILD_DIR = build

SRC = $(wildcard *.c)
BIN = $(BUILD_DIR)/ipc_client

CFLAGS += -Wall -Wextra -g

all: $(BUILD_DIR) $(BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

indent:
	indent -linux -i4 -nut -ts2 *.c

clean:
	rm -rf $(BUILD_DIR)
	rm -f *.o

.PHONY: all clean indent
