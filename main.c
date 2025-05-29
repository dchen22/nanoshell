#include "fslib/tfs.h"

int main() {
    if (init_tfs() != 0) {
        fprintf(stderr, "Failed to initialize filesystem\n");
        return -1;
    }

    create_file("file1");
    write_file("file1", "hello", 5);
    char buffer[6];

    read_file("file1", buffer, sizeof(buffer) - 1);
    buffer[5] = '\0'; // null-terminate the string
    printf("Read from file1: %s\n", buffer);
    cleanup_tfs();
    return 0;
}