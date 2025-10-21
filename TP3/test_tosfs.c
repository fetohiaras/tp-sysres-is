#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "tosfs.h"

#define TOSFS_FILENAME "test_tosfs_files"

void *fs_memory = NULL;
struct tosfs_superblock *sb = NULL;

int main(int argc, char *argv[]) {
    const char *filename = (argc > 1) ? argv[1] : TOSFS_FILENAME;
    
    printf("Opening tosfs image...\n");
    
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open filesystem image");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("Failed to stat file");
        close(fd);
        return 1;
    }

    fs_memory = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (fs_memory == MAP_FAILED) {
        perror("Failed to mmap file");
        close(fd);
        return 1;
    }

    printf("Mounting superblock...\n");
    sb = (struct tosfs_superblock *)fs_memory;

    if (sb->magic != TOSFS_MAGIC) {
        fprintf(stderr, "Invalid magic number: expected 0x%x, got 0x%x\n", TOSFS_MAGIC, sb->magic);
        munmap(fs_memory, st.st_size);
        close(fd);
        return 1;
    }

    printf("TOSFS filesystem loaded successfully.\n");
    printf("Magic number  : 0x%x\n", sb->magic);
    printf("Block size    : %u\n", sb->block_size);
    printf("Total blocks  : %u\n", sb->blocks);
    printf("Total inodes  : %u\n", sb->inodes);
    printf("Root inode    : %u\n", sb->root_inode);
    printf("Block bitmap  : "PRINTF_BINARY_PATTERN_INT32"\n", PRINTF_BYTE_TO_BINARY_INT32(sb->block_bitmap));
    printf("Inode bitmap  : "PRINTF_BINARY_PATTERN_INT32"\n", PRINTF_BYTE_TO_BINARY_INT32(sb->inode_bitmap));

    munmap(fs_memory, st.st_size);
    close(fd);
    return 0;
}
