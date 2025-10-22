#define FUSE_USE_VERSION 26

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fuse_lowlevel.h>
#include <errno.h>
#include <linux/types.h>
#include "tosfs.h"
#define TOSFS_FILENAME "test_tosfs_files"
typedef __u32 tosfs_ino_t;

void *fs_memory = NULL;
struct tosfs_superblock *sb = NULL;
int fs_fd = -1;
size_t fs_size = 0;

struct tosfs_inode *get_inode(tosfs_ino_t ino) {
    if (!sb || ino == 0 || ino > sb->inodes)
        return NULL;

    struct tosfs_inode *inodes = (struct tosfs_inode *)(fs_memory + TOSFS_BLOCK_SIZE);
    return &inodes[ino - 1];
}


static void tosfs_getattr(fuse_req_t req, fuse_ino_t ino,
                          struct fuse_file_info *fi)
{
    struct stat stbuf;
    memset(&stbuf, 0, sizeof(stbuf));
    (void) fi;


    if (ino == TOSFS_ROOT_INODE || ino == FUSE_ROOT_ID) {
        stbuf.st_ino = TOSFS_ROOT_INODE;
        stbuf.st_mode = S_IFDIR | 0755;
        stbuf.st_nlink = 2;
        stbuf.st_size = TOSFS_BLOCK_SIZE;
        fuse_reply_attr(req, &stbuf, 1.0);
        return;
    }

    struct tosfs_inode *inode = &((struct tosfs_inode *)(fs_memory + TOSFS_BLOCK_SIZE))[ino - 1];
    if (!inode || inode->inode == 0) {
        fuse_reply_err(req, ENOENT);
        return;
    }


    stbuf.st_ino = inode->inode;
    if (S_ISDIR(inode->mode))
        stbuf.st_mode = S_IFDIR | inode->perm;
    else
        stbuf.st_mode = S_IFREG | inode->perm;

    stbuf.st_nlink = inode->nlink;
    stbuf.st_size  = inode->size;
    stbuf.st_uid   = inode->uid;
    stbuf.st_gid   = inode->gid;

    fuse_reply_attr(req, &stbuf, 1.0);
}

static struct fuse_lowlevel_ops tosfs_oper = {
    .getattr = tosfs_getattr,
    .lookup  = NULL,
    .readdir = NULL,
    .open    = NULL,
    .read    = NULL
};

int mount_tosfs_img(const char *filename) {
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

    return 0;
}

void cleanup_tosfs() {
    if (fs_memory && fs_memory != MAP_FAILED)
        munmap(fs_memory, fs_size);
    if (fs_fd >= 0)
        close(fs_fd);
}

int main(int argc, char *argv[]) {
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    struct fuse_chan *ch;
    char *mountpoint = NULL;
    int err = -1;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <mountpoint> [fs_image]\n", argv[0]);
        return 1;
    }

    const char *fs_image = (argc >= 3) ? argv[2] : TOSFS_FILENAME;

    if (mount_tosfs_img(fs_image) != 0) {
        return 1;
    }

    if (fuse_parse_cmdline(&args, &mountpoint, NULL, NULL) != -1 &&
        (ch = fuse_mount(mountpoint, &args)) != NULL) {

        struct fuse_session *se = fuse_lowlevel_new(&args, &tosfs_oper, sizeof(tosfs_oper), NULL);
        if (se != NULL) {
            if (fuse_set_signal_handlers(se) != -1) {
                fuse_session_add_chan(se, ch);
                err = fuse_session_loop(se);
                fuse_remove_signal_handlers(se);
                fuse_session_remove_chan(ch);
            }
            fuse_session_destroy(se);
        }
        fuse_unmount(mountpoint, ch);
    } else {
        fprintf(stderr, "Failed to mount or parse FUSE mountpoint.\n");
    }

    fuse_opt_free_args(&args);
    cleanup_tosfs();

    return err ? 1 : 0;
}
