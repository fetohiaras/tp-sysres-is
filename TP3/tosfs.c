#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "tosfs.h"

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filesystem_image>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int fd = open(argv[1], O_RDWR);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    size_t fs_size = 32 * TOSFS_BLOCK_SIZE;
    void *fs = mmap(NULL, fs_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (fs == MAP_FAILED) {
        perror("mmap");
        close(fd);
        exit(EXIT_FAILURE);
    }

    struct tosfs_superblock *sb = (struct tosfs_superblock *)(fs + TOSFS_SUPERBLOCK * TOSFS_BLOCK_SIZE);
    struct tosfs_inode *inodes = (struct tosfs_inode *)(fs + TOSFS_INODE_BLOCK * TOSFS_BLOCK_SIZE);
    struct tosfs_dentry *root_dir = (struct tosfs_dentry *)(fs + TOSFS_ROOT_BLOCK * TOSFS_BLOCK_SIZE);

    printf("===== Superblock =====\n");
    printf("Magic number : 0x%x\n", sb->magic);
    printf("Block size   : %u bytes\n", sb->block_size);
    printf("Number of blocks : %u\n", sb->blocks);
    printf("Number of inodes : %u\n", sb->inodes);
    printf("Root inode   : %u\n", sb->root_inode);
    printf("Block bitmap : " PRINTF_BINARY_PATTERN_INT32 "\n", PRINTF_BYTE_TO_BINARY_INT32(sb->block_bitmap));
    printf("Inode bitmap : " PRINTF_BINARY_PATTERN_INT32 "\n", PRINTF_BYTE_TO_BINARY_INT32(sb->inode_bitmap));

    printf("\n===== Inode Table =====\n");
    for (int i = 0; i < sb->inodes; i++) {
        struct tosfs_inode *ino = &inodes[i];
        if (ino->inode != 0) {
            printf("Inode %d : block %u | size %u | links %u | mode 0x%x | perm %o\n",
                   ino->inode, ino->block_no, ino->size, ino->nlink,
                   ino->mode, ino->perm);
        }
    }

    printf("\n===== Root Directory =====\n");
    for (int i = 0; i < 10; i++) {
        struct tosfs_dentry *d = &root_dir[i];
        if (d->inode == 0)
            break;
        printf("Entry: %-20s (inode %u)\n", d->name, d->inode);
    }

    munmap(fs, fs_size);
    close(fd);
    return 0;
}