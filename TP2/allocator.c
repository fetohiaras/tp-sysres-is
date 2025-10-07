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
            size_t excess_size = current->bloc_size - size;
            size_t min_split = sizeof(HEADER) + sizeof(long) + 1;

            // check if can be dividided
            if (excess_size >= min_split) {
                // make new header on rest of block
                HEADER *new_block = (HEADER *)((char *)(current + 1) + size + sizeof(long));
                new_block->bloc_size = excess_size - sizeof(HEADER) - sizeof(long);
                new_block->magic_number = MAGIC_NUMBER;
                long *new_end_magic = (long *)((char *)(new_block + 1) + new_block->bloc_size);
                *new_end_magic = MAGIC_NUMBER;

                // connect new block
                new_block->ptr_next = current->ptr_next;
                if (previous)
                    previous->ptr_next = new_block;
                else
                    free_list = new_block;

                // trim block to requested size
                current->bloc_size = size;
                current->ptr_next = NULL;
                current->magic_number = MAGIC_NUMBER;
                long *end_magic = (long *)((char *)(current + 1) + size);
                *end_magic = MAGIC_NUMBER;
                return (void *)(current + 1);
            }

            // if cannot be divided, use whole block
            if (previous)
                previous->ptr_next = current->ptr_next;
            else
                free_list = current->ptr_next;

            current->ptr_next = NULL;
            current->magic_number = MAGIC_NUMBER;
            long *end_magic = (long *)((char *)(current + 1) + size);
            *end_magic = MAGIC_NUMBER;
            return (void *)(current + 1);
        }

        previous = current;
        current = current->ptr_next;
    }

    // if no blocks are adequate, make another
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
    printf("Test : Erreurs malloc/free\n");

    char *overflow = (char *)malloc_3is(10);
    strcpy(overflow, "0123456789ABCD");
    HEADER *h_overflow = ((HEADER *)overflow) - 1;
    printf("Magic début overflow = 0x%lx\n", h_overflow->magic_number);
    long *end_magic = (long *)((char *)(h_overflow + 1) + h_overflow->bloc_size);
    printf("Magic fin overflow = 0x%lx\n", *end_magic);
    free_3is(overflow);

    printf("Erreurs détectées : début = %d, fin = %d\n",
           MEM_ERROR_CORRUPT_START_FLAG, MEM_ERROR_CORRUPT_END_FLAG);

    printf("\nTest : Allocations multiples\n");
    char *a = malloc_3is(32);
    char *b = malloc_3is(40);
    char *c = malloc_3is(48);
    snprintf(a, 32, "bloc a");
    snprintf(b, 40, "bloc b");
    snprintf(c, 48, "bloc c");
    printf("a: %s @ %p\n", a, (void *)a);
    printf("b: %s @ %p\n", b, (void *)b);
    printf("c: %s @ %p\n", c, (void *)c);

    HEADER *ha = ((HEADER *)a) - 1;
    HEADER *hb = ((HEADER *)b) - 1;
    HEADER *hc = ((HEADER *)c) - 1;
    printf("MAGIC a: début = 0x%lx, fin = 0x%lx\n", ha->magic_number,
           *(long *)((char *)(ha + 1) + ha->bloc_size));
    printf("MAGIC b: début = 0x%lx, fin = 0x%lx\n", hb->magic_number,
           *(long *)((char *)(hb + 1) + hb->bloc_size));
    printf("MAGIC c: début = 0x%lx, fin = 0x%lx\n", hc->magic_number,
           *(long *)((char *)(hc + 1) + hc->bloc_size));

    free_3is(a);
    free_3is(b);
    free_3is(c);

    printf("\nTest : Réutilisation et découpage\n");
    char *d = malloc_3is(20);
    char *e = malloc_3is(20);
    char *f = malloc_3is(20);
    snprintf(d, 20, "bloc d");
    snprintf(e, 20, "bloc e");
    snprintf(f, 20, "bloc f");
    printf("d: %s @ %p\n", d, (void *)d);
    printf("e: %s @ %p\n", e, (void *)e);
    printf("f: %s @ %p\n", f, (void *)f);

    free_3is(d);
    free_3is(e);
    free_3is(f);

    printf("\nTest : Fusion et nouvelle allocation\n");
    char *g = malloc_3is(60);
    snprintf(g, 60, "bloc g fusionné");
    printf("g: %s @ %p\n", g, (void *)g);
    HEADER *hg = ((HEADER *)g) - 1;
    printf("MAGIC g: début = 0x%lx, fin = 0x%lx\n", hg->magic_number,
           *(long *)((char *)(hg + 1) + hg->bloc_size));
    free_3is(g);

    return 0;
}
