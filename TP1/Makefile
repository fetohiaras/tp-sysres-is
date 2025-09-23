SRC = $(wildcard *.c)
BIN = $(SRC:.c=)
CFLAGS += -Wall -Wextra -g

all: $(BIN)

indent:
	indent -linux -i4 -nut -ts2 *.c

clean:
	rm -f $(BIN)
	rm -f *.o

.PHONY: all clean indent
