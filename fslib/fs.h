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

    // superblock_t *superblock; // pointer to superblock

} fs_ctx_t;

// typedef struct superblock_t {	
// 	uint64_t   sb_magic;       /* Must match VSFS_MAGIC. */
// 	uint64_t   sb_size;        /* File system size in bytes. */
// 	uint32_t   sb_num_inodes;  /* Total number of inodes (set by mkfs) */
// 	uint32_t   sb_free_inodes; /* Number of available inodes */ 
// 	blk_num_t sb_num_blocks;  /* File system size in blocks */
// 	blk_num_t sb_free_blocks; /* Number of available blocks in file sys */
// 	blk_num_t sb_data_region; /* First block after inode table */ 
// } superblock_t;

// /* Superblock must fit into a single disk sector */
// static_assert(sizeof(superblock_t) <= BLOCK_SIZE,
//               "superblock is too large");