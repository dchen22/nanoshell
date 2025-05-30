#include "../fslib/tfs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

int main() {
    // initialize the filesystem
    if (init_tfs() != 0) {
        fprintf(stderr, "Failed to initialize filesystem\n");
        return -1;
    }

    assert(MAX_NUM_FILES == 100); // ensure MAX_NUM_FILES is 100, otherwise buffers might be too small for filenames

    // temp buffers for filename and data
    char temp_filename[10];     
    char temp_data[15];   
    char temp_compname[10];
    char temp_compdata[15];   

    // create 100 files
    for (int i = 0; i < 100; i++) {
        // clear buffers
        memset(temp_filename, 0, 10);   
        memset(temp_data, 0, 15);
        // write filename and data to buffers
        snprintf(temp_filename, 7, "file%d", i);
        snprintf(temp_data, 14, "%dfiledata%d", i, i);
        // try to create file
        create_file(temp_filename);
        // try to write to file
        if (write_file(temp_filename, temp_data, 14) != 0) {
            printf("Failed to write to %s\n", temp_filename);
            cleanup_tfs();
            return -1;
        }
    }
    
    // read all files
    for (int i = 0; i < 100; i++) {
        // clear buffers
        memset(temp_compname, 0, 10);   
        memset(temp_compdata, 0, 15);
        memset(temp_filename, 0, 10);   
        memset(temp_data, 0, 15);
        // write filename and data to buffers
        snprintf(temp_filename, 7, "file%d", i);
        snprintf(temp_data, 14, "%dfiledata%d", i, i);
        // read filename and data into compbuffers
        read_file(temp_filename, temp_compdata, 14);
        memcpy(temp_compname, temp_filename, 7);

        // check if read data matches written data
        if (strcmp(temp_compdata, temp_data) != 0) {
            printf("Data mismatch for file%d\n", i);
            cleanup_tfs();
            return -1;
        }
        if (strcmp(temp_compname, temp_filename) != 0) {
            printf("Filename mismatch for file%d\n", i);
            cleanup_tfs();
            return -1;
        }
    }

    // try to create one more file
    if (create_file("file101") == 0) {
        printf("Created 101th file, which exceeds max number of files\n");
        cleanup_tfs();
        return -1;
    }

    cleanup_tfs();

    printf("All tests passed\n");
    return 0;
}