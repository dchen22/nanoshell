#include "fslib/tfs.h"
#include "clilib/cli.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    // initialize the filesystem
    if (init_tfs() != 0) {
        printf("Failed to initialize filesystem\n");
        return -1;
    }

    fflush(stdout);

    while (1) {
        
        printf("$ ");

        char *userinput = NULL;     // buffer to store user input
        size_t len = 0;             // specify initial size of the buffer
        ssize_t nread = getline(&userinput, &len, stdin);   // read user input, store bytes read

        // error handling for reading user input
        if (nread == -1) {
            fprintf(stderr, "Error reading input\n");
            free(userinput);
            return -1;
        }

        // parse user input into tokens
        parse_command(split_line(userinput));

    }

    cleanup_tfs();
    return 0;
}