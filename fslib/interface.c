#include "interface.h"

int create_file(const char *filepath) {
    if (filepath == NULL) {
        return ERROR_INVALID_PATH;
    }

    // Infer whether this is a directory based on whether filepath ends with '/'
    size_t filepath_len = strlen(filepath);
    if (filepath_len == 0) {
        return ERROR_INVALID_PATH;
    }
    
    bool is_directory = (filepath[filepath_len - 1] == '/');
    
    // Create a working copy of filepath, stripping trailing '/' if present
    char *working_path = malloc(filepath_len + 1);
    if (working_path == NULL) {
        return ERROR_INVALID_PATH;
    }
    strcpy(working_path, filepath);
    
    if (is_directory && filepath_len > 1) {
        // Remove trailing '/'
        working_path[filepath_len - 1] = '\0';
    }

    // We need to remove the last component of the filepath
    // Otherwise the split_path_inodes parser will think the filepath does not exist
    
    // Find the last '/' in working_path
    const char *last_slash = strrchr(working_path, '/');
    if (last_slash == NULL) {
        // No '/' found, invalid path format
        free(working_path);
        return ERROR_INVALID_PATH;
    }

    // Extract the filename (everything after last '/')
    char *filename = malloc(strlen(last_slash));
    strcpy(filename, last_slash + 1);
    
    // Create a copy of working_path without the filename (up to and including last '/')
    size_t parent_len = last_slash - working_path + 1; // +1 to include the '/'
    char *parent_path = malloc(parent_len + 1); // +1 for null terminator
    if (parent_path == NULL) {
        free(working_path);
        return ERROR_INVALID_PATH;
    }
    strncpy(parent_path, working_path, parent_len);
    parent_path[parent_len] = '\0';
    

    printf("Parent path: %s, Filename: %s\n", parent_path, filename);
    
    unsigned int path_len = 0; bool out_is_dir = false;
    inode_t** inodes = split_path_inodes(parent_path, &path_len, &out_is_dir);
    free(parent_path);
    free(working_path);
    
    if (inodes == NULL) {
        return ERROR_FILE_NOT_FOUND;
    }
    
    printf("path_len: %d\n", path_len);

    // Get parent directory (last inode in the array)
    inode_t *parent = inodes[path_len - 1];
    
    // create file in parent directory
    if (_create_inode(parent, filename, is_directory) < 0) {
        free_split_path_inodes(inodes);
        free(filename);
        return ERROR_FS;
    }
    
    free_split_path_inodes(inodes);
    free(filename);
    return 0;
}

int delete_file(const char *filepath) {
    if (filepath == NULL) {
        return ERROR_INVALID_PATH;
    }
    unsigned int path_len = 0; bool out_is_dir = false;
    inode_t** inodes = split_path_inodes(filepath, &path_len, &out_is_dir);
    if (inodes == NULL || path_len < 2) {
        free_split_path_inodes(inodes);
        return ERROR_FILE_NOT_FOUND;
    }
    if (_delete_inode(inodes[path_len - 2], inodes[path_len - 1]->name) < 0) {
        return ERROR_FS;
    }

    free_split_path_inodes(inodes);

    return 0;
}

fs_result_t read_file(const char *filepath, char *buffer, uint32_t buffer_size) {
    fs_result_t result = {0};
    if (filepath == NULL) {
        result.code = ERROR_INVALID_PATH;
        return result;
    }
    unsigned int path_len = 0; bool out_is_dir = false;
    inode_t** inodes = split_path_inodes(filepath, &path_len, &out_is_dir);
    if (inodes == NULL) {
        free_split_path_inodes(inodes);
        result.code = ERROR_INVALID_PATH;
        return result;
    }

    result = _read_inode(inodes[path_len - 2], inodes[path_len - 1]->name, buffer, buffer_size);
    free_split_path_inodes(inodes);
    return result;
}

fs_result_t write_file(const char *filepath, const char *buffer, uint32_t buffer_size) {
    fs_result_t result = {0};
    if (filepath == NULL) {
        result.code = ERROR_INVALID_PATH;
        return result;
    }
    unsigned int path_len = 0; bool out_is_dir = false;
    inode_t** inodes = split_path_inodes(filepath, &path_len, &out_is_dir);
    if (inodes == NULL) {
        free_split_path_inodes(inodes);
        result.code = ERROR_FILE_NOT_FOUND;
        return result;
    }

    result = _write_inode(inodes[path_len - 2], inodes[path_len - 1]->name, buffer, buffer_size);
    free_split_path_inodes(inodes);
    return result;
}

inode_t get_file_metadata(const char *filepath) {
    inode_t result = {0};
    result.is_allocated = false;
    if (filepath == NULL) {
        return result;
    }
    unsigned int path_len = 0; bool out_is_dir = false;
    inode_t** inodes = split_path_inodes(filepath, &path_len, &out_is_dir);
    if (inodes == NULL) {
        free_split_path_inodes(inodes);
        return result;
    }
    result = *(inodes[path_len - 1]);
    free_split_path_inodes(inodes);
    return result;
}

bool file_exists(const char *filepath) {
    if (filepath == NULL) {
        return false;
    }
    return get_file_metadata(filepath).is_allocated;
}


int list_files(const char *filepath) {
    if (filepath == NULL) {
        return ERROR_INVALID_PATH;
    }
    unsigned int path_len = 0; bool out_is_dir = false;
    inode_t** inodes = split_path_inodes(filepath, &path_len, &out_is_dir);
    if (inodes == NULL) {
        free_split_path_inodes(inodes);
        return ERROR_INVALID_PATH;
    }

    if (!(inodes[path_len - 1]->is_directory)) {
        free_split_path_inodes(inodes);
        return ERROR_FILE_TYPE_MISMATCH;
    }

    inode_t* subfiles = get_subfiles(filepath);
    int count = 0;
    for (inode_t* sf = subfiles; sf->is_allocated; sf = sf + 1) {
        printf("%s\n", sf->name);
        count++;
    }
    free_split_path_inodes(inodes);
    free(subfiles);
    return count;   
}