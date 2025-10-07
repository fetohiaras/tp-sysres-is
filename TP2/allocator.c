#include <unistd.h>
#include <stddef.h>
#include <stdio.h>

typedef struct HEADER {
    size_t bloc_size;
} HEADER;

void *malloc_3is(size_t size) {
    size_t total_size = sizeof(HEADER) + size;

    HEADER *block = sbrk(total_size);
    if (block == (void *)-1) {
        return NULL;
    }

    block->bloc_size = size;

    return (void *)(block + 1);  
}

void free_3is(void *ptr) {
    (void)ptr;
}


int main() {
    char *data = malloc_3is(20);
    if (data == NULL) {
        perror("malloc_3is failed");
        return 1;
    }

    snprintf(data, 20, "Hello world");
    printf("Memory content: %s\n", data);

    free_3is(data);  

    return 0;
}
