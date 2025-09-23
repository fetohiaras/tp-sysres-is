#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h> 
#include <sys/types.h>
#include <sys/wait.h>

int bss_var;
int data_var = 7;
char *rodata_var = "READ ONLY";

/*void print_pmap() {
    char buf[64];
    snprintf(buf, sizeof(buf), "pmap -X %d", getpid());
    system(buf); //change
}*/

void execv_pmap() {
    pid_t pid = fork();

    if (pid == -1) {
        perror("Error: fork failed!");
        return;
    }

    if (pid == 0) {
        char pid_buf[16];
        snprintf(pid_buf, sizeof(pid_buf), "%d", getpid());

        char *args[] = {"/usr/bin/pmap", "-X", pid_buf, NULL};
        execv("/usr/bin/pmap", args);

        perror("Error: execv failed!");
        exit(EXIT_FAILURE);
    } else {
        waitpid(pid, NULL, 0);
    }
}


void print_mem(int *data_ptr, int *bss_ptr, const char *rodata_ptr, int *heap_ptr, char *stack_ptr, void (*main_fn)(), void (*printf_fn)(const char *, ...), void *mmap_ptr) {
    printf("Showing current process memory addresses :\n");
    printf("------------------------------------------\n");
    printf(".data   : %p\n", (void *)data_ptr);
    printf(".bss    : %p\n", (void *)bss_ptr);
    printf(".rodata : %p\n", (void *)rodata_ptr);
    printf("heap    : %p\n", (void *)heap_ptr);
    printf("stack   : %p\n", (void *)stack_ptr);
    printf(".text   : %p\n", (void *)main_fn);
    printf("libc    : %p\n", (void *)printf_fn);
    printf("mmap    : %p\n", mmap_ptr);
    printf("------------------------------------------\n");
}

int main (){
    char stack_var = 'b';
    int* heap_int_var = malloc((1<<20)*sizeof(int));
    *heap_int_var = 42;

    printf("Allocating very long memory page...\n");
    void *mmap_page = mmap(NULL, (1<<20)*sizeof(long), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    print_mem(&data_var, &bss_var, rodata_var, heap_int_var, &stack_var, (void *)main, (void *)printf, mmap_page);
    /*
    printf("Showing current process memory addresses :\n");
    printf("------------------------------------------\n");
    printf(".data   : %p\n", (void *)&data_var);
    printf(".bss    : %p\n", (void *)&bss_var);
    printf(".rodata : %p\n", (void *)rodata_var);
    printf("heap    : %p\n", (void *)heap_int_var);
    printf("stack   : %p\n", (void *)&stack_var);
    printf(".text   : %p\n", (void *)&main);
    printf("libc    : %p\n", (void *)&printf);
    printf("mmap    : %p\n", mmap_page);
    printf("------------------------------------------\n"); */

    printf("Printing process memory regions with pmap: \n");
    printf("------------------------------------------\n");
    execv_pmap();
    //sleep(10);

    printf("Deallocating memory... \n");
    free(heap_int_var);
    munmap(mmap_page, 4096*sizeof(long));

    return 0;
}