#include "cli.h"
#include "../fslib/interface.h"
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



void parse_command(void *params_struct) {
    // TODO: cannot set retval of struct since we are freeing it
    parse_command_args_t *params = (parse_command_args_t *)params_struct;
    char **argv = params->argv; // array of arguments
    if (argv[0] == NULL) {
        params->retval = 0;
        return;
    }
    char *command = argv[0];    // command must be first argument

    // command table 
    if (strcmp(command, "hello") == 0) {        // hello command
        printf("Hello, world!\n");
        params->retval = 0; // set return value to 0
        return;
    }
    else if (strcmp(command, "ls") == 0) {      // list files
        if (get_argc(argv) < 2) {
            printf("nanoshell: ls: Usage: ls [filepath]\n");
            params->retval = -1;
            return;
        }
        list_files(argv[1]);
        params->retval = 0;
        return;
    }
    else if (strcmp(command, "touch") == 0) {   // create a file
        if (get_argc(argv) < 2) {
            printf("nanoshell: touch: missing file operand\n");
            params->retval = -1;
            return;
        }
        create_file(argv[1]);
        params->retval = 0;
        return;
    }
    else if (strcmp(command, "write") == 0) {   // write to a file
        if (get_argc(argv) < 3) {
            printf("nanoshell: write: Usage: write [filename] [contents]\n");
            params->retval = -1;
            return;
        }
        if (!file_exists(argv[1])) {
            printf("nanoshell: write: %s: No such file\n", argv[1]);
            params->retval = -1;
            return;
        }
        if (!(get_file_metadata(argv[1]).is_directory)) {
            printf("nanoshell: write: %s is a directory\n", argv[1]);
            params->retval = -1;
            return;
        }
        printf("%u bytes written\n", write_file(argv[1], argv[2], strlen(argv[2])));
        params->retval = 0;
        return;
    }
    else if (strcmp(command, "cat") == 0) {     // read a file
        if (get_argc(argv) < 2) {
            printf("nanoshell: cat: Usage: read [filename]\n");
            params->retval = -1;
            return;
        }
        // check if file exists
        if (!file_exists(argv[1])) {
            printf("nanoshell: cat: %s: No such file\n", argv[1]);
            params->retval = -1;
            return;
        }

        inode_t file_metadata = get_file_metadata(argv[1]);
        if (!file_metadata.is_allocated) {
            printf("nanoshell: cat: %s: Corrupted file\n", argv[1]);
            params->retval = -1;
            return;
        }

        uint32_t filesize = file_metadata.size;
    
        if (filesize == 0)  {
            params->retval = 0;    // empty file, do nothing
            return;
        }

        char *readbuffer = malloc(filesize + 1); // +1 for null terminator
        read_file(argv[1], readbuffer, filesize);
        readbuffer[filesize] = '\0'; // null terminate the buffer
        printf("%s\n", readbuffer); // we will add a newline for qol
        free(readbuffer);
        params->retval = 0;
        return;
    }
    else if (strcmp(command, "rm") == 0) {
        if (get_argc(argv) < 2) {
            printf("nanoshell: rm: Usage: rm [filename]\n");
            params->retval = -1;
            return;
        }
        if (delete_file(argv[1]) < 0) {
            printf("nanoshell: rm: %s: No such file\n", argv[1]);
            params->retval = -1;
            return;
        }
        params->retval = 0;
        return;
    }
    else if (strcmp(command, "vim") == 0) { // open vim editor on a file
        if (get_argc(argv) < 2) {
            printf("nanoshell: vim: Usage: vim [filename]\n");
            params->retval = -1;
            return;
        }
        // check if file exists, if not, create it
        // checking if file exists could be redundant, as create_file would fail if it exists
        if (!file_exists(argv[1])) {    
            if (create_file(argv[1]) < 0) {
                printf("nanoshell: vim: %s: Could not create file\n", argv[1]);
                params->retval = -1;
                return;
            }
        }

        inode_t file_metadata = get_file_metadata(argv[1]);
        if (!file_metadata.is_allocated) {
            printf("nanoshell: cat: %s: Corrupted file\n", argv[1]);
            params->retval = -1;
            return;
        }

        uint32_t filesize = file_metadata.size;

        // read content of file into buffer
        char *file_content = malloc(filesize + 1); // +1 for null terminator
        read_file(argv[1], file_content, filesize);    // read file contents into buffer
        file_content[filesize] = '\0'; // null terminate the buffer
        char *vim_contents = run_editor(file_content);  // start editor and write file contents to it
        write_file(argv[1], vim_contents, strlen(vim_contents));    // write new contents back to file
        free(file_content); // free the file content buffer
        
        printf("\n"); // newline after exiting vim for better formatting
        params->retval = 0;
        return;
    }
    else if (strcmp(command, "exit") == 0) {
        params->retval = 1;
        return;
    }
    else {
        printf("nanoshell: %s: command not found\n", command);
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