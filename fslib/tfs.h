/**
 * Tiny Filesystem
 * 
 * Simple in-memory prototype filesystem
 * 
 * Will be replaced later 
 */

#include <stdlib.h>
#include <stdio.h>


// for now everything done in memory

typedef size_t tfs_size_t;

typedef struct tfs_file {
    char *name; // file name
    size_t size; // file size
    char *data; // file data
} tfs_file_t;

/**
 * @brief Initializes the filesystem.
 * 
 * @return 0 on success, -1 on failure 
 */
int init_tfs();

/**
 * @brief Create a file in the filesystem
 */
int create_file(char* filename);

/**
 * @brief Get pointer to a file
 * 
 * @param filename Name of the file to get
 * @param fp Pointer to a pointer to store the file pointer
 */
tfs_file_t *get_file(char* filename);

/**
 * @brief Overwrite a file in the filesystem.alignas
 * 
 * @param filename Name of the file to write to
 * @param data Pointer to data to write to the file.
 * @param data_size Size of the data to write
 * 
 * @return 0 on success, -1 on failure (e.g. file does not exist)
 */
int write_file(char *filename, char* data, tfs_size_t data_size);

/**
 * @brief Read a file from the filesystem.
 * 
 * @param filename Name of the file to read
 * @param buffer Pointer to a buffer to store the file data.
 * @param size Size of the buffer.
 * 
 * Does not add automatically null terminate the buffer.
 * 
 * @return Number of bytes read
 */
int read_file(char *filename, char* buffer, size_t buffer_size);

/**
 * @brief Deletes a file from the filesystem.
 * 
 * @param filename Name of the file to delete
 * 
 * @return 0 on success, -1 on failure (e.g. file does not exist)
 */
int delete_file(char *filename);

/**
 * @brief Clean up the filesystem and free resources
 */
void cleanup_tfs();

/**
 * @brief Clean up a specific file and free its resources
 * 
 * @param fp Pointer to the file to clean up
 */
int cleanup_file(tfs_file_t *fp);