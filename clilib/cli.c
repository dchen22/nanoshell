#include "cli.h"


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



int parse_command(char **args) {
    if (args[0] == NULL) {
        return 0;
    }
    char *command = args[0];    // command must be first argument

    // lookup commands 
    if (strcmp(command, "hello") == 0) {
        printf("Hello, world!\n");
        return 0;
    }
    else {
        printf("tinyshell: %s: command not found\n", command);
        return -1;
    }

    unsigned int i = 1;
    while (args[i] != NULL) {
        
        i++;
    }
}