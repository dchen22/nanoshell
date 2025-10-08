#include "interface.h"

int create_file(const char *filepath, bool is_directory) {
    if (filepath == NULL) {
        return ERROR_INVALID_PATH;
    }

    // We need to remove the last component of the filepath
    // Otherwise the split_path_inodes parser will think the filepath does not exist
    
    // Find the last '/' in filepath
    const char *last_slash = strrchr(filepath, '/');
    if (last_slash == NULL) {
        // No '/' found, invalid path format
        return ERROR_INVALID_PATH;
    }
    
    // Create a copy of filepath without the filename (up to and including last '/')
    size_t parent_len = last_slash - filepath + 1; // +1 to include the '/'
    char *parent_path = malloc(parent_len + 1); // +1 for null terminator
    if (parent_path == NULL) {
        return ERROR_INVALID_PATH;
    }
    strncpy(parent_path, filepath, parent_len);
    parent_path[parent_len] = '\0';
    
    // Extract the filename (everything after last '/')
    const char *filename = last_slash + 1;

    printf("Parent path: %s, Filename: %s\n", parent_path, filename);
    
    unsigned int path_len = 0; bool out_is_dir = false;
    inode_t** inodes = split_path_inodes(parent_path, &path_len, &out_is_dir);
    free(parent_path);
    
    if (inodes == NULL) {
        return ERROR_INVALID_PATH;
    }
    
    printf("path_len: %d\n", path_len);

    // Get parent directory (last inode in the array)
    inode_t *parent = inodes[path_len - 1];
    
    // create file in parent directory
    if (_create_inode(parent, filename, is_directory) < 0) {
        free_split_path_inodes(inodes);
        return ERROR_FAILED_TO_CREATE_FILE;
    }
    
    free_split_path_inodes(inodes);
    return 0;
}

int delete_file(const char *filepath) {
    if (filepath == NULL) {
        return ERROR_INVALID_PATH;
    }
    unsigned int path_len = 0; bool out_is_dir = false;
    inode_t** inodes = split_path_inodes(filepath, &path_len, &out_is_dir);
    if (inodes == NULL) {
        return ERROR_INVALID_PATH;
    }
    if (_delete_inode(inodes[path_len - 2], inodes[path_len - 1]->name) < 0) {
        return ERROR_FAILED_TO_DELETE_FILE;
    }

    free_split_path_inodes(inodes);

    return 0;
}

uint32_t read_file(const char *filepath, char *buffer, uint32_t buffer_size) {
    if (filepath == NULL) {
        return ERROR_INVALID_PATH;
    }
    unsigned int path_len = 0; bool out_is_dir = false;
    inode_t** inodes = split_path_inodes(filepath, &path_len, &out_is_dir);
    if (inodes == NULL) {
        return ERROR_INVALID_PATH;
    }
    free_split_path_inodes(inodes);

    return _read_inode(inodes[path_len - 2], inodes[path_len - 1]->name, buffer, buffer_size);
}

uint32_t write_file(const char *filepath, const char *buffer, uint32_t buffer_size) {
    if (filepath == NULL) {
        return ERROR_INVALID_PATH;
    }
    unsigned int path_len = 0; bool out_is_dir = false;
    inode_t** inodes = split_path_inodes(filepath, &path_len, &out_is_dir);
    if (inodes == NULL) {
        return ERROR_INVALID_PATH;
    }
    free_split_path_inodes(inodes);

    return _write_inode(inodes[path_len - 2], inodes[path_len - 1]->name, buffer, buffer_size);
}