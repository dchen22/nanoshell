/**
 * Tiny Filesystem
 * 
 * Simple in-memory prototype filesystem
 * 
 * Will be replaced later 
 */

#include <stdlib.h>
#include <stdio.h>

#define MAX_NUM_FILES 100

typedef size_t tfs_size_t;

typedef struct tfs_file {
    char *name; // file name
    size_t size; // file size
    char *data; // file data
} tfs_file_t;

/**
 * Initializes the filesystem.
 * 
 * @return 0 on success, -1 on failure 
 */
int init_tfs();

/**
 * Create a file in the filesystem
 * 
 * @return 0 on success, -1 on failure
 */
int create_file(char* filename);


/**
 * Overwrite a file in the filesystem
 * 
 * @param filename Name of the file to write to
 * @param data Pointer to data to write to the file.
 * @param data_size Size of the data to write
 * 
 * Does not null terminate the data. This function will attempt to directly write 
 * <data_size> bytes starting at <data>
 * 
 * @return 0 on success, -1 on failure (e.g. file does not exist)
 */
int write_file(char *filename, char* data, tfs_size_t data_size);



/**
 * Read a file from the filesystem.
 * 
 * @param filename Name of the file to read
 * @param buffer Pointer to a buffer to store the file data.
 * @param buffer_size Size of the buffer.
 * 
 * Does not add automatically null terminate the buffer. This function will
 * attempt to read up to <buffer_size> bytes of file.size bytes from the file, whichever
 * is smaller.
 * 
 * @return Number of bytes read. Will be at most file.size
 */
int read_file(char *filename, char* buffer, size_t buffer_size);

/**
 * Deletes a file from the filesystem.
 * 
 * @param filename Name of the file to delete
 * 
 * @return 0 on success, -1 on failure (e.g. file does not exist)
 */
int delete_file(char *filename);

/**
 * Clean up the filesystem and free resources
 */
void cleanup_tfs();

/**
 * Clean up a specific file and free its resources
 * 
 * @param fp Pointer to the file to clean up
 */
int cleanup_file(tfs_file_t *fp);

/**
 * Get all files in the filesystem
 * 
 * @return Pointer to a dynamically allocated array of dynamically allocated filename pointers
 * 
 * The returned array is terminated with a NULL pointer.
 */
char **list_files();

/**
 * Get the size of a file
 * 
 * @param filename Name of the file to get the size of
 * 
 * @return Size of the file in bytes, or -1 on failure
 */
tfs_size_t get_size(char *filename);