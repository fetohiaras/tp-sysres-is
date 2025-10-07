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

void *malloc_3is(size_t size) {
    HEADER *current = free_list;
    HEADER *previous = NULL;

    // check for free blocks of same size to reuse
    while (current != NULL) {
        if (current->bloc_size >= size) {
            if (previous != NULL) {
                previous->ptr_next = current->ptr_next;
            } else {
                free_list = current->ptr_next;
            }

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
    if (mem == (void *)-1) {
        return NULL; 
    }

    current = (HEADER *)mem;
    current->ptr_next = NULL;
    current->bloc_size = size;
    current->magic_number = MAGIC_NUMBER;

    long *end_magic = (long *)((char *)(current + 1) + size);
    *end_magic = MAGIC_NUMBER;

    return (void *)(current + 1);
}

void free_3is(void *ptr) {  
    if (ptr == NULL) {
        return;
    }

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
}

int main() {

    printf("Test : Allocations multiples\n");

    char *bloc1 = (char *)malloc_3is(20);
    char *bloc2 = (char *)malloc_3is(30);
    char *bloc3 = (char *)malloc_3is(40);

    if (bloc1 == NULL || bloc2 == NULL || bloc3 == NULL) {
        perror("Erreur : échec d’allocation mémoire !\n");
        return 1;
    }

    snprintf(bloc1, 20, "Bloc 1: Hello");
    snprintf(bloc2, 30, "Bloc 2: Allocateur 3IS");
    snprintf(bloc3, 40, "Bloc 3: Test d'allocation OK");

    printf("%s\n", bloc1);
    printf("%s\n", bloc2);
    printf("%s\n", bloc3);

    printf("\nAdresses allouées :\n");
    printf("bloc1 = %p\n", (void *)bloc1);
    printf("bloc2 = %p\n", (void *)bloc2);
    printf("bloc3 = %p\n", (void *)bloc3);

    HEADER *hdr1 = ((HEADER *)bloc1) - 1;
    HEADER *hdr2 = ((HEADER *)bloc2) - 1;
    HEADER *hdr3 = ((HEADER *)bloc3) - 1;

    printf("\nMAGIC NUMBERS:\n");
    printf("bloc1: end = 0x%lx\n", hdr1->magic_number);
    printf("bloc2: end = 0x%lx\n", hdr2->magic_number);
    printf("bloc3: end = 0x%lx\n", hdr3->magic_number);

    printf("Test de réutilisation de blocs: \n");

    free_3is(bloc3);
    printf("Vérification - nombre d'erreurs de corruption de mémoire: %i\n", MEM_ERROR_CORRUPT_START_FLAG);
    printf("Vérification - nombre d'erreurs d'overflow: %i\n", MEM_ERROR_CORRUPT_END_FLAG);

    char* newbloc = (char*)malloc_3is(35);

    if (newbloc == NULL) {
        perror("Erreur : échec d’allocation mémoire !\n");
        return 1;
    }

    HEADER *hdrnew = ((HEADER *)newbloc) - 1;

    snprintf(newbloc, 35, "new block!");
    printf("%s\n", newbloc);
    printf("\nAdresse alloué à newbloc :\n");
    printf("newbloc = %p\n", (void *)newbloc);

    printf("\nMAGIC NUMBER:\n");
    printf("newbloc: end = 0x%lx\n", hdrnew->magic_number);

    free_3is(bloc1);
    free_3is(bloc2);
    free_3is(newbloc);

    printf("Vérification - nombre d'erreurs de corruption de mémoire: %i\n", MEM_ERROR_CORRUPT_START_FLAG);
    printf("Vérification - nombre d'erreurs d'overflow: %i\n", MEM_ERROR_CORRUPT_END_FLAG);

    return 0;
}
