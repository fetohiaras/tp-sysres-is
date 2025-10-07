#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAGIC_NUMBER 0x0123456789ABCDEFL

typedef struct HEADER_TAG {
    struct HEADER_TAG *ptr_next;
    size_t bloc_size;
    long magic_number;
} HEADER;

static HEADER *free_list = NULL;
volatile int MEM_ERROR_CORRUPT_START_FLAG = 0;
volatile int MEM_ERROR_CORRUPT_END_FLAG = 0;

void merge_free_blocks() {
    if (!free_list) return;
    HEADER *current = free_list;
    while (current != NULL) {
        HEADER *next = current->ptr_next;
        while (next != NULL) {
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

void *malloc_3is(size_t size) {
    HEADER *current = free_list;
    HEADER *previous = NULL;
    while (current != NULL) {
        if (current->bloc_size >= size) {
            if (previous != NULL) previous->ptr_next = current->ptr_next;
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
    void *mem = sbrk(sizeof(HEADER) + size + sizeof(long));
    if (mem == (void *)-1) return NULL;
    current = (HEADER *)mem;
    current->ptr_next = NULL;
    current->bloc_size = size;
    current->magic_number = MAGIC_NUMBER;
    long *end_magic = (long *)((char *)(current + 1) + size);
    *end_magic = MAGIC_NUMBER;
    return (void *)(current + 1);
}

void free_3is(void *ptr) {
    if (ptr == NULL) return;
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

int main() {
    char *bloc1 = (char *)malloc_3is(20);
    char *bloc2 = (char *)malloc_3is(30);
    char *bloc3 = (char *)malloc_3is(40);
    snprintf(bloc1, 20, "Bloc 1: Hello");
    snprintf(bloc2, 30, "Bloc 2: Allocateur 3IS");
    snprintf(bloc3, 40, "Bloc 3: Test allocation");
    printf("%s\n%s\n%s\n", bloc1, bloc2, bloc3);
    printf("\nAdresses allouées:\nbloc1 = %p\nbloc2 = %p\nbloc3 = %p\n", (void *)bloc1, (void *)bloc2, (void *)bloc3);

    free_3is(bloc1);
    free_3is(bloc2);
    free_3is(bloc3);

    char *newbloc = (char *)malloc_3is(45);
    snprintf(newbloc, 45, "Nouveau bloc après fusion!");
    printf("%s\nAdresse newbloc = %p\n", newbloc, (void *)newbloc);

    char *overflow = (char *)malloc_3is(10);
    strcpy(overflow, "0123456789ABC");
    free_3is(overflow);
    printf("Erreurs début = %d, Erreurs fin = %d\n", MEM_ERROR_CORRUPT_START_FLAG, MEM_ERROR_CORRUPT_END_FLAG);

    free_3is(newbloc);
    return 0;
}
