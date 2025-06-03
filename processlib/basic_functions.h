#include <stdlib.h>

/**
 * Prints "hello world" to the console.
 */
void print_hello_world();

/**
 * Print a specified number of integers.
 */
void generate_numbers(void *args);
typedef struct generate_numbers_args {
    unsigned int num_integers;
} generate_numbers_args_t;

/**
 * Prints identity of current process.
 */
void print_identity(void *args);


/**
 * Generate as many processs as possible
 */
void fill_process_list(void *args);
