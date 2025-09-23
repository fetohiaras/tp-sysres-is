#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

int global_init = 42;
int global_zero;
char *str = "Hello World";

void dummy_function() {
    printf("Inside dummy_function\n");
}

int main() {
    int local_var = 123;
    int *heap_var = malloc((4<<20)*sizeof(int));
    *heap_var = 999;

    void *mmap_area = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    printf("Data (.data)         : %p\n", (void*)&global_init);
    printf("BSS (.bss)           : %p\n", (void*)&global_zero);
    printf("String (.rodata)     : %p\n", (void*)str);
    printf("Heap (malloc)        : %p\n", (void*)heap_var);
    printf("Stack (local var)    : %p\n", (void*)&local_var);
    printf("Text (.text, main)   : %p\n", (void*)&main);
    printf("LibC function (printf): %p\n", (void*)&printf);
    printf("Mmap area            : %p\n", mmap_area);

    printf("\nPID: %d\n", getpid());

    getchar();

    free(heap_var);
    munmap(mmap_area, 4096);
    return 0;
}