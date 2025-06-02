#include "tfs.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


static tfs_file_t *files[MAX_NUM_FILES];    // file pointers
static size_t num_files = 0;        


int init_tfs() {
    num_files = 0;
    for (size_t i = 0; i < MAX_NUM_FILES; i++) {
        files[i] = NULL;
    }
    return 0;
}

void cleanup_tfs() {
    for (size_t i = 0; i < MAX_NUM_FILES; i++) {
        if (files[i] != NULL) {
            if (cleanup_file(files[i]) < 0) fprintf(stderr, "Failed to cleanup file %lu\n", i);
            files[i] = NULL;
        }
    }
}




int cleanup_file(tfs_file_t *fp) {
    if (fp == NULL) return -1;

    // free any allocated memory in file properties
    free(fp->name); // free file name (allocated by strndup)
    free(fp->data); // free file data
    fp->data = NULL;

    return 0;
}



/**
 * Get pointer to a file
 * 
 * @param filename Name of the file to get
 * 
 * @return Pointer to file if found, NULL otherwise
 */
static tfs_file_t *get_file(char *filename) {
    if (filename == NULL) return NULL;

    // iterate through filelist and compare filenames
    for (size_t i = 0; i < MAX_NUM_FILES; i++) {
        if (files[i] != NULL && strcmp(files[i]->name, filename) == 0) {    
            return files[i];
        }
    }
    return NULL;
}




int create_file(char* filename) {
    // check if we reached max number of files
    if (num_files >= MAX_NUM_FILES) {
        return -1; 
    }

    // check if filename already exists
    if (get_file(filename) != NULL) {
        return -1;
    }

    // malloc for new file
    tfs_file_t *new_fp = malloc(sizeof(tfs_file_t));
    if (new_fp == NULL) return -1; // malloc fail

    // place in file list
    for (size_t i = 0; i < MAX_NUM_FILES; i++) {
        if (files[i] == NULL) {
            files[i] = new_fp;
            num_files++;
            break;
        }
    }

    // initialize file properties
    new_fp->name = strndup(filename, strlen(filename)); 
    new_fp->size = 0;
    new_fp->data = NULL; // no data yet

    return 0;
}





int read_file(char *filename, char *buffer, tfs_size_t buffer_size) {
    tfs_file_t *fp = get_file(filename);
    if (fp == NULL || buffer == NULL) return -1;    

    // empty file, do nothing
    if (fp->data == NULL) {
        return 0;
    }

    // file fits in buffer
    if (fp->size <= buffer_size) {
        memcpy(buffer, fp->data, fp->size); // copy file data
        return fp->size;
    } else {     // file does not fit in buffer
        memcpy(buffer, fp->data, buffer_size);  // copy only as much as fits
        return buffer_size;
    }
}




int write_file(char *filename, char* data, tfs_size_t data_size) {
    tfs_file_t *fp = get_file(filename);
    if (fp == NULL || data == NULL) return -1;

    // free old data if it exists
    if (fp->data != NULL) {
        free(fp->data);
    }

    // allocate new data
    fp->data = malloc(data_size);
    if (fp->data == NULL) return -1; // malloc fail

    memcpy(fp->data, data, data_size); // copy data to file
    fp->size = data_size;

    return 0;
}




int delete_file(char *filename) {
    if (filename == NULL) return -1;

    // find file in list
    for (size_t i = 0; i < MAX_NUM_FILES; i++) {
        if (files[i] != NULL && strcmp(files[i]->name, filename) == 0) {
            cleanup_file(files[i]); // cleanup file
            files[i] = NULL;
            num_files--; 
            return 0;
        }
    }

    return -1; // file not found
}




char **list_files() {
    char **filelist = malloc((num_files + 1) * sizeof(char*)); // +1 for NULL at the end

    size_t index = 0;
    for (size_t i = 0; i < MAX_NUM_FILES; i++) {
        if (files[i] != NULL) {
            filelist[index] = strndup(files[i]->name, strlen(files[i]->name)); // copy filename
        } 
        index++;
    }

    return filelist;
}

tfs_size_t get_size(char *filename) {
    tfs_file_t *fp = get_file(filename);
    if (fp == NULL) return -1; // file not found

    return fp->size; // return file size
}




bool file_exists(char *filename) {
    tfs_file_t *fp = get_file(filename);
    if (fp == NULL) {
        return false; // file does not exist
    }
    return true; // file exists
}