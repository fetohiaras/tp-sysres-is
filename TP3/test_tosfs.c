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
int fs_fd = -1;
size_t fs_size = 0;

int mount_tosfs_img(const char *filename) {
    printf("Mounting TOSFS image...\n");

    fs_fd = open(filename, O_RDONLY);
    if (fs_fd < 0) {
        perror("Failed to open filesystem image");
        return 1;
    }

    struct stat st;
    if (fstat(fs_fd, &st) < 0) {
        perror("Failed to stat file");
        close(fs_fd);
        return 1;
    }
    fs_size = st.st_size;

    fs_memory = mmap(NULL, fs_size, PROT_READ, MAP_SHARED, fs_fd, 0);
    if (fs_memory == MAP_FAILED) {
        perror("Failed to mmap file");
        close(fs_fd);
        return 1;
    }

    sb = (struct tosfs_superblock *)fs_memory;
    if (sb->magic != TOSFS_MAGIC) {
        fprintf(stderr, "Invalid magic number: expected 0x%x, got 0x%x\n", TOSFS_MAGIC, sb->magic);
        munmap(fs_memory, fs_size);
        close(fs_fd);
        return 1;
    }

    printf("TOSFS image mounted successfully.\n");
    return 0;
}

void show_tosfs_img_info(const struct tosfs_superblock *sb) {
    printf("TOSFS image info:\n");
    printf("------------------------------------------------\n");

    printf("Magic number  : 0x%x\n", sb->magic);
    printf("Block size    : %u\n", sb->block_size);
    printf("Total blocks  : %u\n", sb->blocks);
    printf("Total inodes  : %u\n", sb->inodes);
    printf("Root inode    : %u\n", sb->root_inode);
    printf("Block bitmap  : "PRINTF_BINARY_PATTERN_INT32"\n", PRINTF_BYTE_TO_BINARY_INT32(sb->block_bitmap));
    printf("Inode bitmap  : "PRINTF_BINARY_PATTERN_INT32"\n", PRINTF_BYTE_TO_BINARY_INT32(sb->inode_bitmap));
    printf("------------------------------------------------\n");
}

void cleanup_tosfs() {
    if (fs_memory && fs_memory != MAP_FAILED)
        munmap(fs_memory, fs_size);
    if (fs_fd >= 0)
        close(fs_fd);
}

int main(int argc, char *argv[]) {
    const char *filename = (argc > 1) ? argv[1] : TOSFS_FILENAME;

    if (mount_tosfs_img(filename) != 0) {
        fprintf(stderr, "Could not mount filesystem image.\n");
        return 1;
    }

    show_tosfs_img_info(sb);
    cleanup_tosfs();

    return 0;
}
