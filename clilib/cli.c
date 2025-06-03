#include "cli.h"
#include "../fslib/tfs.h"
#include "vim.h"
#include <termios.h>

char **split_line(char *line) {
    int bufsize = TOK_BUFSIZE, pos = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token;

    if (!tokens) {
        fprintf(stderr, "allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, TOK_DELIM);
    while (token) {
        tokens[pos++] = token;
        if (pos >= bufsize) {
            bufsize *= 2;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens) {
                fprintf(stderr, "allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, TOK_DELIM);
    }
    tokens[pos] = NULL;
    return tokens;
}



void parse_command(void *args) {
    parse_command_args_t *params = (parse_command_args_t *)args;
    char **argv = params->argv; // array of arguments
    if (argv[0] == NULL) {
        free(args); // free the args struct
        params->retval = 0;
        return;
    }
    char *command = argv[0];    // command must be first argument

    // command table 
    if (strcmp(command, "hello") == 0) {        // hello command
        printf("Hello, world!\n");
        free(args); // free the args struct
        params->retval = 0; // set return value to 0
        return;
    }
    else if (strcmp(command, "ls") == 0) {      // list files
        // process_create(list_files, NULL);
        char **filelist = list_files();

        size_t index = 0;
        while (filelist[index] != NULL) {
            printf("%s\n", filelist[index]);

            // free the filename pointer
            free(filelist[index]);
            filelist[index] = NULL; // good practice to avoid dangling the pointer

            index++;
        }
        free(filelist);
        free(args); // free the args struct
        params->retval = 0;
        return;
    }
    else if (strcmp(command, "touch") == 0) {   // create a file
        if (get_argc(argv) < 2) {
            printf("nanoshell: touch: missing file operand\n");
            free(args); // free the args struct
            params->retval = -1;
            return;
        }
        create_file(argv[1]);
        free(args); // free the args struct
        params->retval = 0;
        return;
    }
    else if (strcmp(command, "write") == 0) {   // write to a file
        if (get_argc(argv) < 3) {
            printf("nanoshell: write: Usage: write [filename] [contents]\n");
            free(args); // free the args struct
            params->retval = -1;
            return;
        }
        if (write_file(argv[1], argv[2], strlen(argv[2])) < 0) {
            printf("nanoshell: write: %s: No such file\n", argv[1]);
            free(args); // free the args struct
            params->retval = -1;
            return;
        }
        free(args); // free the args struct
        params->retval = 0;
        return;
    }
    else if (strcmp(command, "cat") == 0) {     // read a file
        if (get_argc(argv) < 2) {
            printf("nanoshell: cat: Usage: read [filename]\n");
            free(args); // free the args struct
            params->retval = -1;
            return;
        }
        // check if file exists
        if (!file_exists(argv[1])) {
            printf("nanoshell: cat: %s: No such file\n", argv[1]);
            free(args); // free the args struct
            params->retval = -1;
            return;
        }

        tfs_size_t filesize = get_size(argv[1]);
    
        if (filesize == 0)  {
            free(args); // free the args struct 
            params->retval = 0;    // empty file, do nothing
            return;
        }

        char *readbuffer = malloc(filesize + 1); // +1 for null terminator
        read_file(argv[1], readbuffer, filesize);
        readbuffer[filesize] = '\0'; // null terminate the buffer
        printf("%s\n", readbuffer); // we will add a newline for qol
        free(readbuffer);
        free(args); // free the args struct
        params->retval = 0;
        return;
    }
    else if (strcmp(command, "rm") == 0) {
        if (get_argc(argv) < 2) {
            printf("nanoshell: rm: Usage: rm [filename]\n");
            free(args); // free the args struct
            params->retval = -1;
            return;
        }
        if (delete_file(argv[1]) < 0) {
            printf("nanoshell: rm: %s: No such file\n", argv[1]);
            free(args); // free the args struct
            params->retval = -1;
            return;
        }
        free(args); // free the args struct
        params->retval = 0;
        return;
    }
    else if (strcmp(command, "vim") == 0) { // open vim editor on a file
        if (get_argc(argv) < 2) {
            printf("nanoshell: vim: Usage: vim [filename]\n");
            free(args); // free the args struct
            params->retval = -1;
            return;
        }
        // check if file exists, if not, create it
        // checking if file exists could be redundant, as create_file would fail if it exists
        if (!file_exists(argv[1])) {    
            if (create_file(argv[1]) < 0) {
                printf("nanoshell: vim: %s: Could not create file\n", argv[1]);
                free(args); // free the args struct
                params->retval = -1;
                return;
            }
        }
        // read content of file into buffer
        char *file_content = malloc(get_size(argv[1]) + 1); // +1 for null terminator
        if (read_file(argv[1], file_content, get_size(argv[1])) < 0) {
            printf("nanoshell: vim: %s: Could not read file\n", argv[1]);
            free(file_content);
            free(args); // free the args struct
            params->retval = -1;
            return;
        }
        file_content[get_size(argv[1])] = '\0'; // null terminate the buffer
        // start editor and write file contents to it
        char *vim_contents = run_editor(file_content);
        write_file(argv[1], vim_contents, strlen(vim_contents));
        printf("\n"); // newline after exiting vim for better formatting
        free(args); // free the args struct
        params->retval = 0;
        return;
    }
    else if (strcmp(command, "exit") == 0) {
        params->retval = 1;
        return;
    }
    else {
        printf("nanoshell: %s: command not found\n", command);
        free(args); // free the args struct
        params->retval = -1;
        return;
    }

}

unsigned int get_argc(char **argv) {
    unsigned int argc = 0;
    while (argv[argc] != NULL) {
        argc++;
    }
    return argc;
}