#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

int main() {
    const char *filename = "test.txt";
    int fd = open(filename, O_RDWR);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        exit(EXIT_FAILURE);
    }

    size_t size = st.st_size;
    if (size == 0) {
        printf("Fichier vide\n");
        close(fd);
        return 0;
    }

    char *addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        close(fd);
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < size / 2; i++) {
        char tmp = addr[i];
        addr[i] = addr[size - i - 1];
        addr[size - i - 1] = tmp;
    }

    if (munmap(addr, size) == -1) {
        perror("munmap");
    }

    close(fd);
    return 0;
}
