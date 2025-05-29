#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include "fs.h"

/**
 * @brief Initializes the filesystem disk.
 * 
 * @param fs Pointer to the filesystem context 
 * @param size Size of the disk in bytes
 * @param num_inodes Number of inodes to be created in the filesystem
 * 
 * @return Return 0 on success, negative value on failure.
 */
int init_disk(fs_ctx_t *fs, unsigned size, unsigned num_inodes);
