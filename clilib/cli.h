/**
 * Library for nanoshell command line interface handling
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TOK_BUFSIZE 64
#define TOK_DELIM    " \t\r\n"
#define MAX_READ_SIZE 268435456 // 256 MiB

/**
 * Run the nanoshell CLI
 */
void run_CLI(void *args);

/**
 * Splits a string into tokens based on delimiters.
 * 
 * @param line The string to split.
 * 
 * @return A dynamically allocated array of pointers to each token in <line>
 * 
 * Only the returned array <tokens> needs to be freed, DO NOT free individual tokens <tokens[i]>
 */
char **split_line(char *line);

/**
 * Read an array of arguments and handle the command
 * 
 * @param args Pointer to struct containing the array of arguments
 * 
 * The array <argv> must be terminated with NULL. The first element of <argv> must be the commandname
 * 
 * @return (args.retval): 0 on success, -1 on failure, 1 on exit
 */
void parse_command(void *args);
typedef struct parse_command_args {
    char **argv; // array of arguments
    int retval; // return value of the command
} parse_command_args_t;


/**
 * Get the number of arguments in an array of arguments
 * 
 * @param argv Array of arguments
 * 
 * The array <argv> must be terminated with NULL.
 * 
 * @return Number of arguments in <argv>
 */
unsigned int get_argc(char **argv);