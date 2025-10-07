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

void *malloc_3is(size_t size) {
    HEADER *current = free_list;
    HEADER *previous = NULL;
    
    while (current != NULL && current->bloc_size < size) {
        previous = current;
        current = current->ptr_next;
    }
   
    if (current != NULL) {
        if (previous != NULL) {
            previous->ptr_next = current->ptr_next;
        } else {
            free_list = current->ptr_next;
        }

        current->ptr_next = NULL;  
        current->magic_number = MAGIC_NUMBER;
        return (void *)(current + 1);
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
    if (ptr == NULL) return; 

    HEADER *block = ((HEADER *)ptr) - 1; 

    if (block->magic_number != MAGIC_NUMBER) {
        printf("Erreur: corruption mémoire détectée début du bloc \n");
        return;  
    }

    long *end_magic = (long *)((char *)(block + 1) + block->bloc_size);
    if (*end_magic != MAGIC_NUMBER) {
        printf("Erreur: débordement mémoire détecté  fin du bloc \n");
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

    if (!bloc1 || !bloc2 || !bloc3) {
        printf("Erreur : échec d’allocation mémoire !\n");
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

    free_3is(bloc1);
    free_3is(bloc2);
    free_3is(bloc3);

    return 0;
}
