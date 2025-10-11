#include "fs_helper.h"

inode_t* get_subfile_by_name(inode_t* parent, const char *filename, uint32_t* inode_index) {
    if (parent == NULL) {
        printf("GET_INODE_BY_NAME FAILURE: Parent must be a valid inode\n");
        return NULL;
    }
    if (!parent->is_directory) {
        printf("GET_INODE_BY_NAME FAILURE: Parent must be a directory\n");
        return NULL;
    }
    for (uint32_t i = 0; i < 12; i++) {
        if (parent->direct_blocknums[i] != 0) {
            uint32_t* direct_block = (uint32_t*)(disk_start + parent->direct_blocknums[i] * BLOCK_SIZE);
            for (uint32_t d = 0; d < BLOCK_SIZE / sizeof(uint32_t); d++) {
                if (direct_block[d] != 0) {
                    inode_t* inode = get_inode_by_index(direct_block[d]);
                    if (inode == NULL) {
                        printf("GET_INODE_BY_NAME FAILURE: Inode is null\n");
                        return NULL;
                    }
                    if (strcmp(inode->name, filename) == 0) {
                        if (inode_index) *inode_index = direct_block[d];
                        return inode;
                    }
                }
            }
        }
    }
    if (parent->indirect_blocknum != 0) {
        uint32_t* indirect_block = (uint32_t*)(disk_start + parent->indirect_blocknum * BLOCK_SIZE);
        for (uint32_t i = 0; i < BLOCK_SIZE / sizeof(uint32_t); i++) {
            if (indirect_block[i] != 0) {
                uint32_t* directory_block = (uint32_t*)(disk_start + indirect_block[i] * BLOCK_SIZE);
                for (uint32_t d = 0; d < BLOCK_SIZE / sizeof(uint32_t); d++) {
                    if (directory_block[d] != 0) {
                        inode_t* inode = get_inode_by_index(directory_block[d]);
                        if (inode == NULL) {
                            printf("GET_INODE_BY_NAME FAILURE: Inode is null\n");
                            return NULL;
                        }
                        if (strcmp(inode->name, filename) == 0) {
                            if (inode_index) *inode_index = directory_block[d];
                            return inode;
                        }
                    }
                }
            }
        }
    }

    return NULL;
}

inode_t* get_inode_by_index(uint32_t inode_index) {

    if (bitmapget(inode_bitmap, sb->num_max_inodes, inode_index) == 1) {
        inode_t* inode = (inode_t*)(inode_table + inode_index * sizeof(inode_t));
        if (!inode->is_allocated) { // sanity check
            printf("GET_INODE_BY_INDEX FAILURE: Inode allocated in bitmap but is_allocated=false\n");
            return NULL;
        }
        return inode;
    }
    return NULL;
}

uint32_t write_to_datablock(inode_t* inode, uint32_t block_index, const char* buffer, uint32_t buffer_size, uint32_t* bytes_written) {
    if (buffer_size < *bytes_written) {
        printf("WRITE_TO_DATABLOCK FAILURE: bytes_read is greater than buffer_size\n");
        return 0;
    }

    if (buffer_size == *bytes_written) {
        return 0;
    }

    if (buffer_size - *bytes_written >= BLOCK_SIZE) {
        memcpy(disk_start + block_index * BLOCK_SIZE, buffer + *bytes_written, BLOCK_SIZE);
        inode->size += BLOCK_SIZE;
        *bytes_written += BLOCK_SIZE;
        return BLOCK_SIZE;
    } else {
        uint32_t bytes_to_write = buffer_size - *bytes_written;
        memcpy(disk_start + block_index * BLOCK_SIZE, buffer + *bytes_written, bytes_to_write);
        inode->size += bytes_to_write;
        *bytes_written += bytes_to_write;
        return bytes_to_write;  // Fixed: return the actual bytes written
    }

    printf("WRITE_TO_DATABLOCK FAILURE: Failed to write to data block\n");
    return 0;
}

inode_t** get_files_in_dir(inode_t* directory, uint32_t *num_files) {
    if (!directory->is_directory) {
        printf("GET_FILES_IN_DIR FAILURE: Not a directory\n");
        return NULL;
    }
    
    // First pass: count the number of files
    uint32_t file_count = 0;
    
    // Count files in direct blocks
    for (uint32_t i = 0; i < 12; i++) {
        if (directory->direct_blocknums[i] != 0) {
            uint32_t* direct_block = (uint32_t*)(disk_start + directory->direct_blocknums[i] * BLOCK_SIZE);
            for (uint32_t d = 0; d < BLOCK_SIZE / sizeof(uint32_t); d++) {
                if (direct_block[d] != 0) {
                    file_count++;
                }
            }
        }
    }
    
    // Count files in indirect blocks
    if (directory->indirect_blocknum != 0) {
        uint32_t* indirect_block = (uint32_t*)(disk_start + directory->indirect_blocknum * BLOCK_SIZE);
        for (uint32_t i = 0; i < BLOCK_SIZE / sizeof(uint32_t); i++) {
            if (indirect_block[i] != 0) {
                uint32_t* directory_block = (uint32_t*)(disk_start + indirect_block[i] * BLOCK_SIZE);
                for (uint32_t d = 0; d < BLOCK_SIZE / sizeof(uint32_t); d++) {
                    if (directory_block[d] != 0) {
                        file_count++;
                    }
                }
            }
        }
    }
    
    // Allocate array for inode pointers (+1 for NULL terminator)
    inode_t** files = (inode_t**)malloc((file_count + 1) * sizeof(inode_t*));
    if (files == NULL) {
        printf("GET_FILES_IN_DIR FAILURE: Memory allocation failed\n");
        return NULL;
    }
    
    // Initialize the array
    for (uint32_t i = 0; i <= file_count; i++) {
        files[i] = NULL;
    }
    
    // Second pass: populate the array
    uint32_t file_index = 0;
    
    // Add files from direct blocks
    for (uint32_t i = 0; i < 12; i++) {
        if (directory->direct_blocknums[i] != 0) {
            uint32_t* direct_block = (uint32_t*)(disk_start + directory->direct_blocknums[i] * BLOCK_SIZE);
            for (uint32_t d = 0; d < BLOCK_SIZE / sizeof(uint32_t); d++) {
                if (direct_block[d] != 0) {
                    inode_t* subfile = get_inode_by_index(direct_block[d]);
                    if (subfile == NULL) {
                        printf("GET_FILES_IN_DIR FAILURE: Found subfile is null\n");
                        free(files);
                        return NULL;
                    }
                    files[file_index++] = subfile;
                }
            }
            
        }
    }
    
    // Add files from indirect blocks
    if (directory->indirect_blocknum != 0) {
        uint32_t* indirect_block = (uint32_t*)(disk_start + directory->indirect_blocknum * BLOCK_SIZE);
        for (uint32_t i = 0; i < BLOCK_SIZE / sizeof(uint32_t); i++) {
            if (indirect_block[i] != 0) {
                uint32_t* directory_block = (uint32_t*)(disk_start + indirect_block[i] * BLOCK_SIZE);
                for (uint32_t d = 0; d < BLOCK_SIZE / sizeof(uint32_t); d++) {
                    if (directory_block[d] != 0) {
                        inode_t* subfile = get_inode_by_index(directory_block[d]);
                        if (subfile == NULL) {
                            printf("GET_FILES_IN_DIR FAILURE: Found subfile is null\n");
                            free(files);
                            return NULL;
                        }
                        files[file_index++] = subfile;
                    }
                }
                
            }
        }
    }
    
    if (num_files) *num_files = file_count;
    return files;
}

void free_get_files_in_dir(inode_t** files) {
    if (files != NULL) {
        free(files);
    }
}


bool _inode_exists(inode_t* directory, const char* filename) {
    if (directory == NULL) {
        return false;
    }
    if (!directory->is_directory) {
        return false;
    }

    // Add files from direct blocks
    for (uint32_t i = 0; i < 12; i++) {
        if (directory->direct_blocknums[i] != 0) {
            uint32_t* direct_block = (uint32_t*)(disk_start + directory->direct_blocknums[i] * BLOCK_SIZE);
            for (uint32_t d = 0; d < BLOCK_SIZE / sizeof(uint32_t); d++) {
                if (direct_block[d] != 0) {
                    inode_t* subfile = get_inode_by_index(direct_block[d]);
                    if (subfile == NULL) {
                        printf("_inode_exists: corruption detected");
                        return false;
                    }
                    if (strcmp(subfile->name, filename) == 0) {
                        return true;
                    }
                }
            }
            
        }
    }
    
    // Add files from indirect blocks
    if (directory->indirect_blocknum != 0) {
        uint32_t* indirect_block = (uint32_t*)(disk_start + directory->indirect_blocknum * BLOCK_SIZE);
        for (uint32_t i = 0; i < BLOCK_SIZE / sizeof(uint32_t); i++) {
            if (indirect_block[i] != 0) {
                uint32_t* directory_block = (uint32_t*)(disk_start + indirect_block[i] * BLOCK_SIZE);
                for (uint32_t d = 0; d < BLOCK_SIZE / sizeof(uint32_t); d++) {
                    if (directory_block[d] != 0) {
                        inode_t* subfile = get_inode_by_index(directory_block[d]);
                        if (subfile == NULL) {
                            printf("_inode_exists: corruption detected");
                            return false;
                        }
                        if (strcmp(subfile->name, filename) == 0) {
                            return true;
                        }
                    }
                }
                
            }
        }
    }
    return false;
}

void free_split_path_inodes(inode_t** inodes) {
    free(inodes);
}

inode_t** split_path_inodes(const char *filepath, uint32_t* out_len, bool* out_is_dir) {
    unsigned int path_len = 0; bool end_is_dir = false;
    char** path_components = split_path(filepath, &path_len, &end_is_dir);
    if (path_len == 0) {
        return NULL;
    }
    if (strcmp(path_components[0], "root") != 0) {
        return NULL;
    }


    inode_t** inodes = (inode_t**)malloc((path_len + 1) * sizeof(inode_t*));
    if (inodes == NULL) {
        return NULL;
    }
    inode_t* curr_dir = get_inode_by_index(0);
    inodes[0] = curr_dir;
    for (unsigned int c = 1; c < path_len; c++) {
        curr_dir = get_subfile_by_name(curr_dir, path_components[c], NULL);
        if (curr_dir == NULL) {
            free_split_path_inodes(inodes);
            free_split_path(path_components);
            return NULL;
        }
        inodes[c] = curr_dir;
    }
    free_split_path(path_components);
    if (out_len) *out_len = path_len;
    if (out_is_dir) *out_is_dir = end_is_dir;
    return inodes;
}

inode_t* get_subfiles(const char *filepath) {
    if (filepath == NULL) {
        return NULL;
    }
    unsigned int path_len = 0; bool out_is_dir = false;
    inode_t** inodes = split_path_inodes(filepath, &path_len, &out_is_dir);
    if (inodes == NULL || path_len == 0) {
        free_split_path_inodes(inodes);
        return NULL;
    }

    inode_t* target = inodes[path_len-1];
    if (!(target->is_directory)) {
        free_split_path_inodes(inodes);
        return NULL;
    }

    uint32_t num_subfiles = 0;
    inode_t** subfile_ptrs = get_files_in_dir(target, &num_subfiles);

    inode_t* subfiles = malloc(sizeof(inode_t) * (num_subfiles+1));
    for (uint32_t i = 0; i < num_subfiles + 1; i++) {
        memset(subfiles + i, 0, sizeof(inode_t));
    }
    for (uint32_t i = 0; i < num_subfiles; i++) {
        subfiles[i] = *(subfile_ptrs[i]);
    }
    subfiles[num_subfiles].is_allocated = false;

    return subfiles;

}