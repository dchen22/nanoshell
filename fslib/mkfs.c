#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "mkfs.h"
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>


int init_disk(fs_ctx_t *fs, unsigned size, unsigned num_inodes) {
    char *disk_path = "disk"; // path to the disk file
    int fs_fd = open(disk_path, O_RDWR);   // file descriptor of filesystem disk

    // error handling for opening disk file
    if (fs_fd < 0) {
        perror("Failed to open disk file");
        return -1;
    }

    // truncate the disk file to the specified size
    if (ftruncate(fs_fd, size) < 0) {
        perror("ftruncate");
        close(fs_fd);
        return -1;
    }

    // need at least one block for superblock, block bitmap, inode bitmap, one block 
    if (size < 4 * BLOCK_SIZE) {
        fprintf(stderr, "Disk size too small. Minimum size is %d bytes.\n", 4 * BLOCK_SIZE);
        return -1;
    }

    // map file system context to the disk
    fs->disk = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fs_fd, 0);

    // ensure mapping was successful
    if (fs->disk == MAP_FAILED) {
        perror("mmap");
        close(fs_fd);
        return -1;
    }

    // assign bitmap pointers
    fs->block_bitmap = (bitmap_t)(fs->disk + BLOCK_SIZE); // block bitmap starts after superblock
    fs->inode_bitmap = (bitmap_t)(fs->disk + 2 * BLOCK_SIZE); // inode bitmap starts after block bitmap

    // fs->num_inodes = num_inodes; // total number of inodes
    // fs->free_inodes = num_inodes; // initially all inodes are free
    // fs->num_blocks = size / BLOCK_SIZE; // total number of blocks in the filesystem
    // fs->free_blocks = fs->num_blocks - 3; // 3 blocks are reserved for superblock, block bitmap, and inode bitmap

    // initialize bitmaps
    bitmap_init(fs->block_bitmap, fs->num_blocks); 
    bitmap_init(fs->inode_bitmap, fs->num_inodes); 


}