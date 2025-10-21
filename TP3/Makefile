CC = gcc
CFLAGS = -Wall -Wextra -fPIC -g

LIBRARY = libmymalloc.so
ALLOCSRC = allocator.c

TESTSRC = malloctest.c
TESTBIN = malloctest

all: $(LIBRARY) $(TESTBIN)

$(LIBRARY): $(ALLOCSRC)
	$(CC) $(CFLAGS) -shared -o $@ $^

$(TESTBIN): $(TESTSRC)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f *.o *.so $(TESTBIN)

.PHONY: all clean
