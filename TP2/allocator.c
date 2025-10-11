#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAGIC_NUMBER 0x0123456789ABCDEFL
#define PREALLOC_SIZE 1024

typedef struct HEADER_TAG {
    struct HEADER_TAG *ptr_next;
    size_t bloc_size;
    long magic_number;
} HEADER;

static HEADER *free_list = NULL;
volatile int MEM_ERROR_CORRUPT_START_FLAG = 0;
volatile int MEM_ERROR_CORRUPT_END_FLAG = 0;

void merge_free_blocks() {
    HEADER *current = free_list;
    while (current) {
        HEADER *next = current->ptr_next;
        while (next) {
            char *current_end = (char *)(current + 1) + current->bloc_size + sizeof(long);
            if (current_end == (char *)next) {
                current->bloc_size += sizeof(HEADER) + next->bloc_size + sizeof(long);
                current->ptr_next = next->ptr_next;
                next = current->ptr_next;
            } else {
                next = next->ptr_next;
            }
        }
        current = current->ptr_next;
    }
}

void preallocate_block() {
    void *mem = sbrk(PREALLOC_SIZE);
    if (mem == (void *)-1) return;
    HEADER *block = (HEADER *)mem;
    block->bloc_size = PREALLOC_SIZE - sizeof(HEADER) - sizeof(long);
    block->ptr_next = free_list;
    block->magic_number = MAGIC_NUMBER;
    long *end_magic = (long *)((char *)(block + 1) + block->bloc_size);
    *end_magic = MAGIC_NUMBER;
    free_list = block;
}

void *malloc_3is(size_t size) {
    HEADER *current = free_list;
    HEADER *previous = NULL;

    while (current) {
        if (current->bloc_size >= size) {
            size_t excess = current->bloc_size - size;
            size_t min_split = sizeof(HEADER) + sizeof(long) + 1;

            if (excess >= min_split) {
                HEADER *new_block = (HEADER *)((char *)(current + 1) + size + sizeof(long));
                new_block->bloc_size = excess - sizeof(HEADER) - sizeof(long);
                new_block->magic_number = MAGIC_NUMBER;
                long *new_end_magic = (long *)((char *)(new_block + 1) + new_block->bloc_size);
                *new_end_magic = MAGIC_NUMBER;
                new_block->ptr_next = current->ptr_next;

                if (previous) previous->ptr_next = new_block;
                else free_list = new_block;

                current->bloc_size = size;
                current->ptr_next = NULL;
                current->magic_number = MAGIC_NUMBER;
                long *end_magic = (long *)((char *)(current + 1) + size);
                *end_magic = MAGIC_NUMBER;
                return (void *)(current + 1);
            }

            if (previous) previous->ptr_next = current->ptr_next;
            else free_list = current->ptr_next;

            current->ptr_next = NULL;
            current->magic_number = MAGIC_NUMBER;
            long *end_magic = (long *)((char *)(current + 1) + size);
            *end_magic = MAGIC_NUMBER;
            return (void *)(current + 1);
        }

        previous = current;
        current = current->ptr_next;
    }

    preallocate_block();
    return malloc_3is(size);
}

void free_3is(void *ptr) {
    if (!ptr) return;
    HEADER *block = ((HEADER *)ptr) - 1;

    if (block->magic_number != MAGIC_NUMBER) {
        MEM_ERROR_CORRUPT_START_FLAG++;
        return;
    }

    long *end_magic = (long *)((char *)(block + 1) + block->bloc_size);
    if (*end_magic != MAGIC_NUMBER) {
        MEM_ERROR_CORRUPT_END_FLAG++;
        return;
    }

    block->ptr_next = free_list;
    free_list = block;
    merge_free_blocks();
}

void run_allocator_tests() {
    printf("[Test] Memory corruption detection (overflow)\n");
    char *overflow = (char *)malloc_3is(10);
    strcpy(overflow, "0123456789ABCD");
    free_3is(overflow);
    printf("Errors: header = %d, footer = %d\n", MEM_ERROR_CORRUPT_START_FLAG, MEM_ERROR_CORRUPT_END_FLAG);

    printf("\n[Test] Multiple allocations and release\n");
    char *a = malloc_3is(32);
    char *b = malloc_3is(40);
    char *c = malloc_3is(48);
    snprintf(a, 32, "block a");
    snprintf(b, 40, "block b");
    snprintf(c, 48, "block c");
    printf("%s @ %p\n%s @ %p\n%s @ %p\n", a, (void *)a, b, (void *)b, c, (void *)c);
    free_3is(a);
    free_3is(b);
    free_3is(c);
    printf("Errors: header = %d, footer = %d\n", MEM_ERROR_CORRUPT_START_FLAG, MEM_ERROR_CORRUPT_END_FLAG);


    printf("\n[Test] Reuse and splitting\n");
    char *d = malloc_3is(20);
    char *e = malloc_3is(20);
    char *f = malloc_3is(20);
    snprintf(d, 20, "block d");
    snprintf(e, 20, "block e");
    snprintf(f, 20, "block f");
    printf("%s @ %p\n%s @ %p\n%s @ %p\n", d, (void *)d, e, (void *)e, f, (void *)f);
    free_3is(d);
    free_3is(e);
    free_3is(f);
    printf("Errors: header = %d, footer = %d\n", MEM_ERROR_CORRUPT_START_FLAG, MEM_ERROR_CORRUPT_END_FLAG);


    printf("\n[Test] Merge and allocate large block\n");
    char *g = malloc_3is(60);
    snprintf(g, 60, "merged block g");
    printf("%s @ %p\n", g, (void *)g);
    free_3is(g);
    printf("Errors: header = %d, footer = %d\n", MEM_ERROR_CORRUPT_START_FLAG, MEM_ERROR_CORRUPT_END_FLAG);

    printf("\n[Test] Memory preallocation\n");
    for (int i = 0; i < 10; i++) {
        char *blk = malloc_3is(50);
        if (blk) {
            snprintf(blk, 50, "Prealloc block %d", i);
            printf("%s @ %p\n", blk, (void *)blk);
            free_3is(blk);
        }
    }
    printf("Errors: header = %d, footer = %d\n", MEM_ERROR_CORRUPT_START_FLAG, MEM_ERROR_CORRUPT_END_FLAG);

}

int main() {
    run_allocator_tests();
    return 0;
}
