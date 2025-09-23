#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>

const char *filename = "test.txt";

int main (){

    int filedesc = open(filename, O_RDWR | O_CREAT, 0644);
    if (filedesc == -1) {
        perror("open error!");
        return 1;
    }

    struct stat st;
    if (fstat(filedesc, &st) == -1) {
        perror("fstat");
        close(filedesc);
        return 1;
    }

    if (st.st_size == 0) {
        const char *txt = "terracotta pie\n";
        ssize_t bytes_written = write(filedesc, txt, strlen(txt));
        if (bytes_written == -1) {
            perror("write error!");
            close(filedesc);
            return 1;
        }

        if (fstat(filedesc, &st) == -1) {
            perror("fstat error");
            close(filedesc);
            return 1;
        }
    }

    size_t filesize = st.st_size;
    printf("Filesize: %ld bytes\n", filesize);

    char *map = mmap(NULL, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, filedesc, 0);
    if (map == MAP_FAILED) {
        perror("mmap");
        close(filedesc);
        return 1;
    }

    for (size_t i = 0; i < filesize / 2; ++i) {
        char tmp = map[i];
        map[i] = map[filesize - 1 - i];
        map[filesize - 1 - i] = tmp;
    }

    printf("File inverted\n");

    if (munmap(map, filesize) == -1) {
        perror("munmap error!");
    }

    close(filedesc);

    printf("Result: \n");
    char *const args[] = {"/usr/bin/cat", (char *)filename, NULL};
    execv("/usr/bin/cat", args);
    printf("\n");
    printf("im invisible bahahahahaha");

    return 0;
}


