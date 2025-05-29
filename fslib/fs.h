#include <stdio.h>
#include <stdlib.h>
#include "bitmap.h"

#define BLOCK_SIZE 4096 // size of a block in bytes

/** Block number (block pointer) type. */
typedef unsigned blk_num_t;

/** Inode number type. */
typedef unsigned ino_num_t;

typedef struct fs_ctx {
    void *disk; // pointer to start of disk
    unsigned num_blocks; // number of blocks in the filesystem
    bitmap_t block_bitmap; // bitmap for used blocks
    bitmap_t inode_bitmap; // bitmap for used inodes

    ino_num_t num_inodes; // total number of inodes
    ino_num_t free_inodes; // number of free inodes
    blk_num_t free_blocks; // number of free blocks in the filesystem

} fs_ctx_t;

