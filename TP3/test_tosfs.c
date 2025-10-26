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
    printf("[DEBUG] getattr called for inode: %lu\n", ino);
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

static void tosfs_readdir(fuse_req_t req, fuse_ino_t ino, size_t size,
                          off_t off, struct fuse_file_info *fi)
{
    printf("[DEBUG] readdir called for inode: %lu\n", ino);
    (void) fi;

    if (ino != TOSFS_ROOT_INODE) {
        fuse_reply_err(req, ENOTDIR);
        return;
    }

    char buf[1024];
    size_t pos = 0;
    struct stat st;


    memset(&st, 0, sizeof(st));
    st.st_ino = ino;
    st.st_mode = S_IFDIR | 0755;
    printf("[DEBUG] Adding entry: .\n");
    pos += fuse_add_direntry(req, buf + pos, sizeof(buf) - pos, ".", &st, pos);
    printf("[DEBUG] Adding entry: ..\n");
    pos += fuse_add_direntry(req, buf + pos, sizeof(buf) - pos, "..", &st, pos);

    struct tosfs_inode *root_inode = get_inode(ino);
    int nb_entries = 0;
    if (root_inode)
        nb_entries = root_inode->size / sizeof(struct tosfs_dentry);

    if (nb_entries == 0) {

        memset(&st, 0, sizeof(st));
        st.st_ino = 2;
        st.st_mode = S_IFREG | 0644;
        pos += fuse_add_direntry(req, buf + pos, sizeof(buf) - pos, "one_file.txt", &st, pos);
    } else {

        struct tosfs_dentry *entries = (struct tosfs_dentry *)(fs_memory + TOSFS_ROOT_BLOCK * TOSFS_BLOCK_SIZE);
        for (int i = 0; i < nb_entries; i++) {
            if (entries[i].inode == 0)
                continue;
            memset(&st, 0, sizeof(st));
            st.st_ino = entries[i].inode;
            st.st_mode = S_IFREG | 0644;
            printf("[DEBUG] Adding entry: %s (inode %d)\n", entries[i].name, entries[i].inode);
            pos += fuse_add_direntry(req, buf + pos, sizeof(buf) - pos, entries[i].name, &st, pos);
        }
    }

    fuse_reply_buf(req, buf, pos);
}

static void tosfs_lookup(fuse_req_t req, fuse_ino_t parent, const char *name)
{
    printf("[DEBUG] lookup called: parent=%lu, name=%s\n", parent, name);

    struct fuse_entry_param e;
    memset(&e, 0, sizeof(e));

    if (parent != TOSFS_ROOT_INODE) {
        fuse_reply_err(req, ENOENT);
        return;
    }

    struct tosfs_dentry *entries = (struct tosfs_dentry *)(fs_memory + TOSFS_ROOT_BLOCK * TOSFS_BLOCK_SIZE);
    int nb_entries = sb->blocks; 

    for (int i = 0; i < nb_entries; i++) {
        if (entries[i].inode != 0 && strcmp(entries[i].name, name) == 0) {
            printf("[DEBUG] Found entry '%s' with inode %d\n", name, entries[i].inode);
            e.ino = entries[i].inode;
            e.attr_timeout = 1.0;
            e.entry_timeout = 1.0;

            struct stat stbuf;
            memset(&stbuf, 0, sizeof(stbuf));
            stbuf.st_ino = entries[i].inode;
            stbuf.st_mode = S_IFREG | 0644;
            stbuf.st_nlink = 1;
            stbuf.st_size = TOSFS_BLOCK_SIZE;
            e.attr = stbuf;

            fuse_reply_entry(req, &e);
            return;
        }
    }

    printf("[DEBUG] Entry '%s' not found in directory\n", name);
    fuse_reply_err(req, ENOENT);
}

static void tosfs_read(fuse_req_t req, fuse_ino_t ino, size_t size,
                       off_t off, struct fuse_file_info *fi)
{
    printf("[DEBUG] read called: ino=%lu, size=%lu, off=%ld\n", ino, size, off);

    struct tosfs_inode *inode = get_inode(ino);
    if (!inode || inode->inode == 0) {
        fuse_reply_err(req, ENOENT);
        return;
    }

    if (off >= inode->size) {
        fuse_reply_buf(req, NULL, 0); 
        return;
    }

    if (off + size > inode->size)
        size = inode->size - off;

    void *block_addr = fs_memory + inode->block_no * TOSFS_BLOCK_SIZE;
    fuse_reply_buf(req, (const char *)block_addr + off, size);
}

static void tosfs_write(fuse_req_t req, fuse_ino_t ino, const char *buf,
                        size_t size, off_t off, struct fuse_file_info *fi)
{
    printf("[DEBUG] write called: ino=%lu, size=%lu, off=%ld\n", ino, size, off);
    (void) fi;

    struct tosfs_inode *inode = get_inode(ino);
    if (!inode || inode->inode == 0) {
        fuse_reply_err(req, ENOENT);
        return;
    }


    if (inode->block_no == 0) {
        fuse_reply_err(req, EIO);
        return;
    }


    if (off + size > TOSFS_BLOCK_SIZE) {
        size = TOSFS_BLOCK_SIZE - off;
    }

    void *block_addr = fs_memory + inode->block_no * TOSFS_BLOCK_SIZE;
    memcpy((char *)block_addr + off, buf, size);


    if (off + size > inode->size)
        inode->size = off + size;


    msync(block_addr, TOSFS_BLOCK_SIZE, MS_SYNC);

    printf("[DEBUG] Wrote %lu bytes to inode %u (new size = %u)\n", size, inode->inode, inode->size);

    fuse_reply_write(req, size);
}




static int dir_add(struct tosfs_inode *dir, const char *name, tosfs_ino_t ino)
{
    struct tosfs_dentry *entries = (struct tosfs_dentry *)(fs_memory + dir->block_no * TOSFS_BLOCK_SIZE);
    int nb_entries = dir->size / sizeof(struct tosfs_dentry);


    for (int i = 0; i < nb_entries; i++) {
        if (entries[i].inode != 0 && strcmp(entries[i].name, name) == 0)
            return -EEXIST;
    }


    struct tosfs_dentry *new_entry = &entries[nb_entries];
    strncpy(new_entry->name, name, sizeof(new_entry->name) - 1);
    new_entry->inode = ino;

    dir->size += sizeof(struct tosfs_dentry);
    return 0;
}


static void tosfs_create(fuse_req_t req, fuse_ino_t parent, const char *name,
                         mode_t mode, struct fuse_file_info *fi)
{
    printf("[DEBUG] create called: parent=%lu, name=%s, mode=%o\n", parent, name, mode);

    if (parent != TOSFS_ROOT_INODE) {
        fuse_reply_err(req, ENOENT);
        return;
    }

    struct tosfs_inode *root = get_inode(parent);
    if (!root) {
        fuse_reply_err(req, EIO);
        return;
    }


    struct tosfs_inode *inodes = (struct tosfs_inode *)(fs_memory + TOSFS_BLOCK_SIZE);
    tosfs_ino_t new_ino = 0;
    for (tosfs_ino_t i = 1; i <= sb->inodes; i++) {
        if (inodes[i - 1].inode == 0) {
            new_ino = i;
            inodes[i - 1].inode = i;
            inodes[i - 1].mode = mode;
            inodes[i - 1].perm = mode & 0777;
            inodes[i - 1].nlink = 1;
            inodes[i - 1].size = 0;
            inodes[i - 1].uid = getuid();
            inodes[i - 1].gid = getgid();
            inodes[i - 1].block_no = TOSFS_ROOT_BLOCK + 1 + i;
            break;
        }
    }

    if (new_ino == 0) {
        fuse_reply_err(req, ENOSPC);
        return;
    }


    int res = dir_add(root, name, new_ino);
    if (res < 0) {
        fuse_reply_err(req, -res);
        return;
    }

    struct fuse_entry_param e;
    memset(&e, 0, sizeof(e));
    e.ino = new_ino;
    e.attr_timeout = 1.0;
    e.entry_timeout = 1.0;

    struct stat stbuf;
    memset(&stbuf, 0, sizeof(stbuf));
    stbuf.st_ino = new_ino;
    stbuf.st_mode = S_IFREG | (mode & 0777);
    stbuf.st_nlink = 1;
    stbuf.st_uid = getuid();
    stbuf.st_gid = getgid();
    stbuf.st_size = 0;
    e.attr = stbuf;

    printf("[DEBUG] Created file '%s' with inode %d\n", name, new_ino);

    fuse_reply_entry(req, &e);
}


static struct fuse_lowlevel_ops tosfs_oper = {
    .getattr = tosfs_getattr,
    .lookup  = tosfs_lookup,
    .readdir = tosfs_readdir,
    .create  = tosfs_create,
    .open    = NULL,
    .read    = tosfs_read,
    .write   = tosfs_write,
};


int mount_tosfs_img(const char *filename) {
    fs_fd = open(filename, O_RDWR);
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

    fs_memory = mmap(NULL, fs_size, PROT_READ | PROT_WRITE, MAP_SHARED, fs_fd, 0);
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


    struct tosfs_inode *root = get_inode(TOSFS_ROOT_INODE);
    if (root) {
        printf("Root inode size: %u\n", root->size);
    } else {
        printf("Impossible to recover root inode\n");
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
