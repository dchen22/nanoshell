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
    if (init_processlib(run_CLI, NULL) != 0) {
        printf("Failed to initialize process library\n");
        return -1;
    }

    // main scheduler loop
    while (!process_scheduler_is_empty()) {
        scheduler_run_next_process();
    }

    cleanup_tfs();
    cleanup_processlib(0);
    printf("Exiting shell\n\n");
    return 0;
}

void run_CLI(void *args) {
    (void)args;  // unused parameter

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
            return;
        }
        // create a new process to handle the user input
        parse_command_args_t *parse_command_args = malloc(sizeof(parse_command_args_t));
        parse_command_args->argv = split_line(userinput);  // split the user input into tokens
        process_create(parse_command, parse_command_args);
        process_yield();
        if (parse_command_args->retval == 1) { // if the command was "exit"
            free(userinput);    // free user input buffer
            free(parse_command_args->argv); // free the argument array
            free(parse_command_args);   // free the args struct
            return; 
        }

        free(userinput);  // free the user input buffer
        free(parse_command_args->argv); // free the argument array
        free(parse_command_args);   // free the args struct
        
    }

}