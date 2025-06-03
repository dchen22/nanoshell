#include "fslib/tfs.h"
#include "clilib/cli.h"
#include "processlib/process.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    // initialize the filesystem
    if (init_tfs() != 0) {
        printf("Failed to initialize filesystem\n");
        return -1;
    }

    // initialize the process library
    if (init_processlib() != 0) {
        printf("Failed to initialize process library\n");
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
            cleanup_tfs();
            cleanup_processlib(0);
            return -1;
        }
        // create a new process to handle the user input
        parse_command_args_t *parse_command_args = malloc(sizeof(parse_command_args_t));
        parse_command_args->argv = split_line(userinput);  // split the user input into tokens
        // process_create(parse_command, args);
        // TODO: process stub can be parse_command

        // parse user input into tokens
        parse_command(parse_command_args);
        free(userinput);  // free the user input buffer
    }

    cleanup_tfs();
    cleanup_processlib(0);
    return 0;
}