#include "mkfs.h"
#include "helpers.h"

/**
 * Get an inode by name
 * 
 * @param parent Parent directory of the file
 * @param filename Name of the file to get
 * @param inode_index Pointer to store index of this inode. Leave NULL if not needed
 * 
 * @return Pointer to inode if found, NULL if not found
 */
inode_t* get_inode_by_name(inode_t* parent, const char *filename, uint32_t* inode_index);

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
uint32_t write_to_datablock(inode_t* inode, uint32_t block_index, const char* buffer, uint32_t buffer_size, uint32_t* bytes_written);


/**
 * Get all direct subfiles in a directory as an array of inode pointers.
 * 
 * @param directory Pointer to directory inode
 * @return NULL-terminated array of inode pointers, or NULL on error
 */
inode_t** get_files_in_dir(inode_t* directory);

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
 * @return 0 if file exists, -1 if file does not exist. Return -2 on fs error
 */
int file_exists(inode_t* directory, char* filename);