#include "fs.h"
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define VSFS_MAGIC 0x56534653 // "VSFS" in hex

// Global variables to track the loaded filesystem
static int disk_fd = -1;
static size_t disk_size = 0;

int load_fs(const char *disk_name) {
    // Check if filesystem is already loaded
    if (disk_start != NULL) {
        // If it's the same disk that's already loaded (e.g. from format_disk), that's ok
        if (sb != NULL && strncmp(sb->disk_name, disk_name, MAX_FILENAME_LEN) == 0) {
            // Already loaded with the same disk, just return success
            return 0;
        }
        fprintf(stderr, "Filesystem already loaded\n");
        return -1;
    }
    
    // Open the disk file
    disk_fd = open(disk_name, O_RDWR);
    if (disk_fd < 0) {
        fprintf(stderr, "Failed to open disk file '%s': %s\n", disk_name, strerror(errno));
        return -1;
    }
    
    // Get file size
    struct stat st;
    if (fstat(disk_fd, &st) < 0) {
        fprintf(stderr, "Failed to get file size: %s\n", strerror(errno));
        close(disk_fd);
        disk_fd = -1;
        return -1;
    }
    
    disk_size = st.st_size;
    
    // Memory map the file
    disk_start = mmap(NULL, disk_size, PROT_READ | PROT_WRITE, MAP_SHARED, disk_fd, 0);
    if (disk_start == MAP_FAILED) {
        fprintf(stderr, "Failed to memory map disk file: %s\n", strerror(errno));
        close(disk_fd);
        disk_fd = -1;
        return -1;
    }
    
    // Set up global pointers
    sb = (superblock_t *)disk_start;
    
    // Validate that this is a VSFS filesystem
    if (sb->magic != VSFS_MAGIC) {
        fprintf(stderr, "Invalid magic number: 0x%08x (expected 0x%08x)\n", 
                sb->magic, VSFS_MAGIC);
        unload_fs();
        return -1;
    }
    
    if (sb->block_size != BLOCK_SIZE) {
        fprintf(stderr, "Invalid block size: %u (expected %d)\n", 
                sb->block_size, BLOCK_SIZE);
        unload_fs();
        return -1;
    }
    
    // Calculate layout and set up pointers based on superblock information
    size_t offset = BLOCK_SIZE;  // Start after superblock
    
    // Set inode bitmap pointer
    inode_bitmap = disk_start + offset;
    offset += sb->num_inode_bitmap_blocks * BLOCK_SIZE;
    
    // Set data bitmap pointer
    data_bitmap = disk_start + offset;
    offset += sb->num_data_bitmap_blocks * BLOCK_SIZE;
    
    // Set inode table pointer
    inode_table = disk_start + offset;
    offset += sb->num_inode_table_blocks * BLOCK_SIZE;
    
    // Set data section pointer
    data_section = disk_start + offset;
    
   
    
    return 0;
}

int _create_inode(inode_t* parent, const char *filename, bool is_directory) {
    // ensure parent is valid
    if (parent == NULL || parent->is_allocated == false) {
        return ERROR_INVALID_PATH;
    }
    // ensure parent is a directory
    if (!parent->is_directory) {
        return ERROR_FILE_TYPE_MISMATCH;
    }

    if (_inode_exists(parent, filename)) {
        return ERROR_FILE_ALREADY_EXISTS;
    }

    // ensure we haven't run out of inodes
    if (sb->num_used_inodes >= sb->num_max_inodes) {
        return ERROR_MAX_FILES_REACHED;
    }
    // try to allocate new inode
    uint32_t new_inode_index = bitmapalloc(inode_bitmap, sb->num_max_inodes);
    if (new_inode_index == 0) {
        return ERROR_FS;
    }

    inode_t* new_inode = (inode_t*)(inode_table + new_inode_index * sizeof(inode_t));
    strncpy(new_inode->name, filename, MAX_FILENAME_LEN);
    new_inode->name[MAX_FILENAME_LEN-1] = '\0';   // ensure null termination

    new_inode->size = 0;
    new_inode->is_directory = is_directory;
    new_inode->is_allocated = true;
    new_inode->nlinks = 1; // parent directory


    // update new file's block pointers
    for (unsigned int i = 0; i < 12; i++) {
        new_inode->direct_blocknums[i] = 0;    // points at 0th disk block (superblock), not 0th data block
    }
    new_inode->indirect_blocknum = 0;
    sb->num_used_inodes++;


    // update parent
    // increment parent directory's nlinks
    parent->nlinks++;
    // add file to parent's data blocks
    for (uint32_t i = 0; i < 12; i++) { // iterate through direct blocks
        // TODO: this is extremely space inefficient
        if (parent->direct_blocknums[i] == 0) {
            uint32_t available_dir_block_index = bitmapalloc(data_bitmap, sb->num_total_blocks);
            if (available_dir_block_index == 0) {   
                return ERROR_FS;
            }
            parent->direct_blocknums[i] = available_dir_block_index;
            uint32_t* directory_block = (uint32_t*)(disk_start + available_dir_block_index * BLOCK_SIZE);
            directory_block[0] = new_inode_index;
            return 0;
        } else {
            uint32_t* direct_block = (uint32_t*)(disk_start + parent->direct_blocknums[i] * BLOCK_SIZE);
            for (uint32_t d = 0; d < BLOCK_SIZE / sizeof(uint32_t); d++) {
                if (direct_block[d] == 0) {
                    direct_block[d] = new_inode_index;  // place newly created inode's index as a directory entry
                    return 0;
                }
            }
        }
    }
    // no space in direct blocks, add to indirect block
    if (parent->indirect_blocknum == 0) {   // no indirect block allocated yet
        // allocate indirect block
        uint32_t available_indirect_block_index = bitmapalloc(data_bitmap, sb->num_total_blocks);
        if (available_indirect_block_index == 0) {
            return ERROR_FS;
        }
        parent->indirect_blocknum = available_indirect_block_index;
        uint32_t* indirect_block = (uint32_t*)(disk_start + available_indirect_block_index * BLOCK_SIZE);
        // initialize indirect block to all zeroes
        memset(indirect_block, 0, BLOCK_SIZE);  

        // allocate direct block
        uint32_t available_dir_block_index = bitmapalloc(data_bitmap, sb->num_total_blocks);
        if (available_dir_block_index == 0) {
            return ERROR_FS;
        }
        uint32_t* dir_block = (uint32_t*)(disk_start + available_dir_block_index * BLOCK_SIZE);
        // initialize dir block to all zeroes
        memset(dir_block, 0, BLOCK_SIZE);
        indirect_block[0] = available_dir_block_index;  // place dir block in indirect block
        dir_block[0] = new_inode_index;    // place new inode index as a directory entry of parent
        return 0; 
    } else {    // indirect block allocated already
        uint32_t* indirect_block = (uint32_t*)(disk_start + parent->indirect_blocknum * BLOCK_SIZE);
        for (uint32_t i = 0; i < BLOCK_SIZE / sizeof(uint32_t); i++) {  // iterate through directory blocks
            if (indirect_block[i] == 0) {
                uint32_t available_dir_block_index = bitmapalloc(data_bitmap, sb->num_total_blocks);
                if (available_dir_block_index == 0) {
                    return ERROR_FS;
                }
                indirect_block[i] = available_dir_block_index;
                uint32_t* directory_block = (uint32_t*)(disk_start + available_dir_block_index * BLOCK_SIZE);
                directory_block[0] = new_inode_index;
                return 0;
            } else {
                uint32_t* directory_entry_block = (uint32_t*)(disk_start + indirect_block[i] * BLOCK_SIZE);
                for (uint32_t d = 0; d < BLOCK_SIZE / sizeof(uint32_t); d++) {
                    if (directory_entry_block[d] == 0) {    // unallocated slot in directory entry block
                        directory_entry_block[d] = new_inode_index;
                        return 0;
                    }
                }
            }
            
        }
    }


    return ERROR_STORAGE_FULL;
}


int _delete_inode(inode_t* parent, const char *filename) {
    // ensure parent is valid
    if (parent == NULL || parent->is_allocated == false) {
        return ERROR_INVALID_PATH;
    }
    // ensure parent is a directory
    if (!parent->is_directory) {
        return ERROR_FILE_TYPE_MISMATCH;
    }
    
    
    // look for file in parent
    // iterate through direct blocks
    uint32_t inode_index;
    for (uint32_t i = 0; i < 12; i++) {
        if (parent->direct_blocknums[i] != 0) {
            uint32_t* direct_block = (uint32_t*)(disk_start + parent->direct_blocknums[i] * BLOCK_SIZE);
            for (uint32_t d = 0; d < BLOCK_SIZE / sizeof(uint32_t); d++) {
                if (direct_block[d] != 0) {
                    inode_t* inode = get_inode_by_index(direct_block[d]);
                    if (inode == NULL) {
                        return ERROR_CORRUPTION_DETECTED;
                    }
                    if (strcmp(inode->name, filename) == 0) {
                        inode_index = direct_block[d];
                        direct_block[d] = 0;    // clear this entry
                        // this direct block may become empty, perhaps deallocate using cleanup daemon
                        // TODO: if deleting directory, need to delete subfiles too
        
                        // update superblock
                        sb->num_used_inodes--;
                        // decrement parent directory's nlinks
                        parent->nlinks--;
                        // set inode is_allocated to false
                        inode->is_allocated = false; 
                        // free inode
                        bitmapset(inode_bitmap, sb->num_max_inodes, inode_index, 0);
                        
                        return 0;
        
                    }
                }
            }
            
        }
    }
    // look for file through indirect block
    if (parent->indirect_blocknum != 0) {
        uint32_t* indirect_block = (uint32_t*)(disk_start + parent->indirect_blocknum * BLOCK_SIZE);
        for (uint32_t i = 0; i < BLOCK_SIZE / sizeof(uint32_t); i++) {
            if (indirect_block[i] != 0) {
                uint32_t* directory_entry_block = (uint32_t*)(disk_start + indirect_block[i] * BLOCK_SIZE);
                for (uint32_t d = 0; d < BLOCK_SIZE / sizeof(uint32_t); d++) {
                    if (directory_entry_block[d] != 0) {
                        inode_index = directory_entry_block[d];
                        inode_t* inode = get_inode_by_index(inode_index);
                        if (inode == NULL) {
                            return ERROR_CORRUPTION_DETECTED;
                        }
                        if (strcmp(inode->name, filename) == 0) {
                            directory_entry_block[d] = 0;    // clear this entry
                            // this directory entry block may become empty, perhaps deallocate using cleanup daemon
                            // TODO: if deleting directory, need to delete subfiles too

                            // update superblock
                            sb->num_used_inodes--;
                            // decrement parent directory's nlinks
                            parent->nlinks--;
                            // set inode is_allocated to false
                            inode->is_allocated = false; 
                            // free inode
                            bitmapset(inode_bitmap, sb->num_max_inodes, inode_index, 0);
                            return 0;
                        }
                    }
           
                }
            }
        }
    }

    return ERROR_FILE_NOT_FOUND;
  
}

fs_result_t _read_inode(inode_t* parent, const char *filename, char *buffer, uint32_t buffer_size) {
    fs_result_t result = {0};
    if (parent == NULL) {
        result.code = ERROR_INVALID_PATH;
        return result;
    }
    if (!parent->is_directory) {
        result.code = ERROR_FILE_TYPE_MISMATCH;
        return result;
    }
    // linear search through files
    inode_t* inode = get_subfile_by_name(parent, filename, NULL);
    if (inode == NULL) {
        result.code = ERROR_FILE_NOT_FOUND;
        return result;
    }
    
    if (inode->is_directory) {
        result.code = ERROR_FILE_TYPE_MISMATCH;
        return result;
    }

    unsigned long bytes_read = 0; 

    // iterate through direct block pointers
    for (unsigned int i = 0; i < 12; i++) {
        // no more direct blocks
        if (inode->direct_blocknums[i] == 0) {
            break;
        }

        // read block 
        char* block = (char*)(disk_start + inode->direct_blocknums[i] * BLOCK_SIZE);

        // copy block to buffer
        if (bytes_read + BLOCK_SIZE <= buffer_size) {   // buffer can still fit at least one block
            // fully copy block
            memcpy(buffer + bytes_read, block, BLOCK_SIZE);
            bytes_read += BLOCK_SIZE;
        } else {    // buffer cannot fit another block
            // partially copy block
            memcpy(buffer + bytes_read, block, buffer_size - bytes_read);
            bytes_read += buffer_size - bytes_read;
            // debug
            if (bytes_read != buffer_size) {
                result.code = ERROR_CORRUPTION_DETECTED;
                return result;
            }
            result.bytes = bytes_read;
            return result;
        }
        
    }

    // read indirect block
    if (inode->indirect_blocknum != 0) {
        uint32_t* indirect_blocknum = (uint32_t*)(disk_start + inode->indirect_blocknum * BLOCK_SIZE);
        for (uint32_t i = 0; i < BLOCK_SIZE / sizeof(uint32_t); i++ ) {
            if (indirect_blocknum[i] != 0) {
                char* block = (char*)(disk_start + indirect_blocknum[i] * BLOCK_SIZE);
                if (bytes_read + BLOCK_SIZE <= buffer_size) {   // buffer can still fit at least one block
                    // copy full block
                    memcpy(buffer + bytes_read, block, BLOCK_SIZE);
                    bytes_read += BLOCK_SIZE;
                } else {    // buffer cannot fit another block
                    // partially copy block
                    memcpy(buffer + bytes_read, block, buffer_size - bytes_read);
                    bytes_read += buffer_size - bytes_read;
                    // debug
                    if (bytes_read != buffer_size) {
                        result.code = ERROR_CORRUPTION_DETECTED;
                        return result;
                    }
                    result.bytes = bytes_read;
                    return result;
                }
            }
            
        }
    }

    
    result.bytes = bytes_read;
    return result;

}

fs_result_t _write_inode(inode_t* parent, const char *filename, const char *buffer, uint32_t buffer_size) {
    fs_result_t result = {0};
    if (parent == NULL) {
        result.code = ERROR_INVALID_PATH;
        return result;
    }
    if (!parent->is_directory) {
        result.code = ERROR_FILE_TYPE_MISMATCH;
        return result;
    }
    inode_t* inode = get_subfile_by_name(parent, filename, NULL);
    if (inode == NULL) {
        result.code = ERROR_FILE_NOT_FOUND;
        return result;
    }

    if (inode->is_directory) {
        result.code = ERROR_FILE_TYPE_MISMATCH;
        return result;
    }

    uint32_t bytes_written = 0;
    uint32_t available_data_block_index = 0;

    // First, deallocate all existing data blocks
    for (unsigned int i = 0; i < 12; i++) {
        if (inode->direct_blocknums[i] != 0) {
            bitmapset(data_bitmap, sb->num_total_blocks, inode->direct_blocknums[i], false);
            inode->direct_blocknums[i] = 0;
        }
    }
    
    // Deallocate indirect block and all blocks it points to
    if (inode->indirect_blocknum != 0) {
        uint32_t* indirect_block = (uint32_t*)(disk_start + inode->indirect_blocknum * BLOCK_SIZE);
        for (uint32_t i = 0; i < BLOCK_SIZE / sizeof(uint32_t); i++) {
            if (indirect_block[i] != 0) {
                bitmapset(data_bitmap, sb->num_total_blocks, indirect_block[i], false);
            }
        }
        bitmapset(data_bitmap, sb->num_total_blocks, inode->indirect_blocknum, false);
        inode->indirect_blocknum = 0;
    }

    inode->size = 0; // reset file size, it is being overwritten

    // iterate through direct block pointers
    for (unsigned int i = 0; i < 12; i++) {
        available_data_block_index = bitmapalloc(data_bitmap, sb->num_total_blocks);
        if (available_data_block_index == 0) {
            result.code = ERROR_STORAGE_FULL;
            return result;
        }
        inode->direct_blocknums[i] = available_data_block_index;   // update direct block pointer

        // if less than one block was written, all data was written (or error occurred)
        fs_result_t wtd_result = write_to_datablock(inode, available_data_block_index, buffer, buffer_size, &bytes_written);
        result.bytes += wtd_result.bytes;
        if (wtd_result.code != 0) { // failed write
            result.code = wtd_result.code;
            return result;
        } else if (wtd_result.bytes < BLOCK_SIZE) {    // successful write of all remaining data
            return result;
        }
    }

    uint32_t remaining_blocks = ceildiv(buffer_size - bytes_written, BLOCK_SIZE);

    // if more blocks to write, allocate an indirect block
    if (remaining_blocks > 0) {
        available_data_block_index = bitmapalloc(data_bitmap, sb->num_total_blocks);
        if (available_data_block_index == 0) {
            result.code = ERROR_STORAGE_FULL;
            return result;
        }
        inode->indirect_blocknum = available_data_block_index;
    } else {
        inode->indirect_blocknum = 0;
    }

    // allocate data blocks, write to them and track them in indirect block
    for (uint32_t i = 0; i < remaining_blocks; i++) {
        // allocate a data block and write to it
        available_data_block_index = bitmapalloc(data_bitmap, sb->num_total_blocks);
        if (available_data_block_index == 0) {
            result.code = ERROR_STORAGE_FULL;
            return result;
        }

        // track data block in indirect block
        uint32_t* indirect_block = (uint32_t*)(disk_start + inode->indirect_blocknum * BLOCK_SIZE);
        indirect_block[i] = available_data_block_index;

        // if less than one block was written, all data was written (or error occurred)
        fs_result_t wtd_result = write_to_datablock(inode, available_data_block_index, buffer, buffer_size, &bytes_written);
        result.bytes += wtd_result.bytes;
        if (wtd_result.code != 0) { // failed write
            result.code = wtd_result.code;
            return result;
        } else if (wtd_result.bytes < BLOCK_SIZE) {    // successful write of all remaining data
            return result;
        }
        
    }

    if (bytes_written != buffer_size) {
        result.code = ERROR_STORAGE_FULL;
    } else {
        result.code = ERROR_FS;
    }
    result.bytes = bytes_written;
    return result;
}


unsigned int print_all_files(void) {
    unsigned int count = 0;
    for (unsigned int i = 0; i < sb->num_max_inodes; i++) {
        if (bitmapget(inode_bitmap, sb->num_max_inodes, i) == 1) {
            inode_t *inode = (inode_t*)(inode_table + i * sizeof(inode_t));
            printf("%s\n", inode->name);
            count++;
        }
    }
    return count;
}

void unload_fs(void) {
    // Unmap memory
    if (disk_start != NULL && disk_start != MAP_FAILED) {
        munmap(disk_start, disk_size);
        disk_start = NULL;
    }
    
    // Close file
    if (disk_fd >= 0) {
        close(disk_fd);
        disk_fd = -1;
    }
    
    // Clear global pointers
    sb = NULL;
    inode_bitmap = NULL;
    data_bitmap = NULL;
    inode_table = NULL;
    data_section = NULL;
    
    disk_size = 0;
}


void print_fs_status(void) {
    printf("Filesystem loaded successfully. File system initial details:\n");
    printf("  Disk name: %s\n", sb->disk_name);
    printf("  Disk size: %u bytes\n", sb->disk_size);
    printf("  Block size: %u\n", sb->block_size);
    printf("  Total blocks: %u blocks\n", sb->num_total_blocks);
    printf("  Inode size: %lu\n", sizeof(inode_t));
    printf("  Max files: %u\n", sb->num_max_inodes);
    printf("  Reserved blocks: %u\n", 1 + sb->num_inode_bitmap_blocks + sb->num_data_bitmap_blocks + sb->num_inode_table_blocks + 0);
    printf("    Breakdown: \n");
    printf("      Superblock: 1\n");
    printf("      Inode bitmap: %u\n", sb->num_inode_bitmap_blocks);
    printf("      Data bitmap: %u\n", sb->num_data_bitmap_blocks);
    printf("      Inode table: %u\n", sb->num_inode_table_blocks);
    printf("      Data section: 0\n");
}

void print_files_in_dir(inode_t* directory) {
    if (!directory->is_directory) {
        printf("Not a directory\n");
        return;
    }
    inode_t** files = get_files_in_dir(directory, NULL);
    if (files == NULL) {
        printf("PRINT_FILES_IN_DIR FAILURE: get_files_in_dir returned NULL\n");
        return;
    }
    
    // Print all files
    for (int i = 0; files[i] != NULL; i++) {
        printf("%s\n", files[i]->name);
    }
    
    // Free the array
    free_get_files_in_dir(files);
}

inode_t get_inode_properties(inode_t* directory, const char *filename) {
    inode_t* file = get_subfile_by_name(directory, filename, NULL);
    if (file == NULL) {
        inode_t empty_inode;
        empty_inode.is_allocated = false;
        return empty_inode;
    }
    return *file;
}