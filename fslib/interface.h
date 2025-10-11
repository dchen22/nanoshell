#include "fs.h"

/**
 * Create a file 
 * 
 * @param filepath Path to the file
 * @return 0 on success, negative error code on failure
 */
int create_file(const char *filepath);

/**
 * Delete a file
 * @param filepath Path to the file
 * @return 0 on success, negative error code on failure
 */
int delete_file(const char *filepath);

/**
 * Read a file
 * @param filepath Path to the file
 * @param buffer Buffer to read the file into
 * @param buffer_size Size of the buffer
 * @return Number of bytes read, 0 indicates empty file or error
 */
fs_result_t read_file(const char *filepath, char *buffer, uint32_t buffer_size);

/**
 * Write to a file
 * @param filepath Path to the file
 * @param buffer Buffer to write to the file
 * @param buffer_size Size of the buffer
 * @return Number of bytes written, 0 indicates error
 */
fs_result_t write_file(const char *filepath, const char *buffer, uint32_t buffer_size);

/**
 * Get the metadata of a file
 * @param filepath Path to the file
 * @return Metadata of the file
 */
inode_t get_file_metadata(const char *filepath);

/**
 * Return whether a file exists
 * 
 * @param filepath Path to the file
 * @return true if file exists, false if file does not exist
 */
bool file_exists(const char *filepath);

/**
 * List all files in a directory
 * 
 * @param filepath Path to the directory
 * @return Number of files listed, 0 indicates error
 */
int list_files(const char *filepath);

