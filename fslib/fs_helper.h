#include "mkfs.h"
#include "helpers.h"

enum {
    ERROR_FILE_NOT_FOUND = -1,
    ERROR_FILE_TYPE_MISMATCH = -2,
    ERROR_FILE_ALREADY_EXISTS = -3,
    ERROR_INVALID_PATH = -4,
    ERROR_MAX_FILES_REACHED = -5,
    ERROR_CORRUPTION_DETECTED = -6,
    ERROR_BUFFER = -7,
    ERROR_FS = -8,
    ERROR_STORAGE_FULL = -9,
};

typedef struct fs_result {
    uint32_t bytes; // number of bytes read or written
    int code;       // error code, 0 on success
} fs_result_t;

/**
 * Get an inode by name
 * 
 * @param parent Parent directory of the file
 * @param filename Name of the file to get
 * @param inode_index Pointer to store index of this inode. Leave NULL if not needed
 * 
 * @return Pointer to inode if found, NULL if not found
 */
inode_t* get_subfile_by_name(inode_t *parent, const char *filename, uint32_t* inode_index);

/**
 * Get an inode by its index
 * 
 * @param inode_index Index of the inode
 * 
 * @return Pointer to inode if found, NULL if not found
 */
inode_t* get_inode_by_index(uint32_t inode_index);


/**
 * Write from a buffer to a data block
 * 
 * Updates the inode and bytes_written accordingly.
 * 
 * @param inode Inode of the file to write to
 * @param block_index Index of the block to write to. Assumes this is a newly allocated data block
 * @param buffer Buffer to write from
 * @param buffer_size Size of the buffer
 * @param bytes_written Number of bytes already written to the buffer. Updated by this function
 * 
 */
fs_result_t write_to_datablock(inode_t* inode, uint32_t block_index, const char* buffer, uint32_t buffer_size, uint32_t* bytes_written);


/**
 * Get all direct subfiles in a directory as an array of inode pointers.
 * 
 * @param directory Pointer to directory inode
 * @param num_files Pointer to store number of subfiles, leave NULL if unneeded
 * @return NULL-terminated array of inode pointers, or NULL on error
 */
inode_t** get_files_in_dir(inode_t* directory, uint32_t* num_files);

/**
 * Free the array of inode pointers returned by get_files_in_dir.
 * 
 * @param files Array of inode pointers to free
 */
void free_get_files_in_dir(inode_t** files);

/**
 * Check whether a file exists in a directory.
 * 
 * @param directory Pointer to directory inode
 * @param filename Name of file
 * 
 * @return true if file exists, false if file does not exist. Return false on fs error
 */
bool _inode_exists(inode_t* directory, const char* filename);


/**
 * Free the array of inode pointers returned by split_path_inodes.
 * 
 * @param inodes Array of inode pointers to free
 */
void free_split_path_inodes(inode_t** inodes);

/**
 * Split a path into a NULL-terminated array of inode pointers.
 * @param filepath Path to split
 * @param out_len Pointer to store the length of the path
 * @param out_is_dir Pointer to store whether the last component of the path is a directory
 * @return NULL-terminated array of inode pointers, or NULL on error (i.e. anywhere the path is invalid)
 */
inode_t** split_path_inodes(const char* filepath, uint32_t* out_len, bool* out_is_dir);

/**
 * Return a NULL-terminated array of inode copies of the subfiles in a given directory
 * 
 * MALLOC: Returned pointer needs to be freed.
 * 
 * @param filepath Path to the directory
 * @return NULL-terminated array of inode copies of the subfiles in a given directory. Returns NULL if the filepath does not exist or is not a directory
 */
inode_t* get_subfiles(const char *filepath);