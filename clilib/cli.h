/**
 * Library for tinyshell command line interface handling
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 
 #define TOK_BUFSIZE 64
 #define TOK_DELIM    " \t\r\n"

 /**
  * Splits a string into tokens based on delimiters.
  * 
  * @param line The string to split.
  * 
  * @return A dynamically allocated array of pointers to each token in <line>
  */
 char **split_line(char *line);

 /**
  * Read an array of arguments and handle the command
  * 
  * @param args Array of arguments
  * 
  * The array <args> must be terminated with NULL. The first element of <args> must be the commandname
  * 
  * @return 0 on success, -1 on failure
  */
 int parse_command(char **args);