CC      = gcc
CFLAGS  = -Wall -Wextra -g -Iinclude
BUILD   = build

LIBNAME     = myseg
LIB_STATIC  = $(BUILD)/lib$(LIBNAME).a
LIBSRC      = sem_functions.c
LIBOBJ      = $(BUILD)/sem_functions.o

IPC_SRC = ipc_client.c
IPC_BIN = $(BUILD)/ipc_client

all: $(BUILD) $(LIB_STATIC) $(IPC_BIN)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/sem_functions.o: $(LIBSRC) | $(BUILD)
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

$(LIB_STATIC): $(LIBOBJ)
	ar rcs $@ $^

$(IPC_BIN): $(IPC_SRC) $(LIB_STATIC)
	$(CC) $(CFLAGS) $< -L$(BUILD) -l$(LIBNAME) -o $@

clean:
	rm -rf $(BUILD)

.PHONY: all clean
