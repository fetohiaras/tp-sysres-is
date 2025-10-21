CC = gcc
CFLAGS = -Wall -Wextra -g `pkg-config fuse --cflags`
LDFLAGS = `pkg-config fuse --libs`

SRC_MAIN := $(wildcard *.c)
SRC_EXAMPLES := $(wildcard fuse_examples/*.c)

BUILD_DIR := build

MAIN_BIN := $(BUILD_DIR)/test_tosfs
EXAMPLE_BINS := $(patsubst fuse_examples/%.c, $(BUILD_DIR)/%, $(SRC_EXAMPLES))

ALL_BINS := $(MAIN_BIN) $(EXAMPLE_BINS)

all: $(BUILD_DIR) $(ALL_BINS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(MAIN_BIN): $(SRC_MAIN)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%: fuse_examples/%.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
