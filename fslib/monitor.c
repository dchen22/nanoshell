#include "fs.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    const char *disk_name = "disk";
    
    // Allow specifying disk name as argument
    if (argc > 1) {
        disk_name = argv[1];
    }
    
    printf("===== File System Monitor Tool =====\n");
    printf("Loading filesystem from '%s'...\n", disk_name);
    
    // Load the filesystem
    if (load_fs(disk_name) < 0) {
        printf("ERROR: Failed to load filesystem from '%s'\n", disk_name);
        printf("Make sure the disk file exists and is properly formatted.\n");
        return -1;
    }
    
    printf("✓ Filesystem loaded successfully\n");
    
    
    return 0;
}

