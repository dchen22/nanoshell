#include <stdlib.h>

/**
 * Prints "hello world" to the console.
 */
void print_hello_world();

/**
 * Generate a specified number of processs.
 */
void generate_processs(unsigned int num_processs);

/**
 * Prints identity of current process.
 */
void print_identity();

/**
 * Prints numbers from 0 up to MAX_PROCESSES.asm
 * 
 * Should be interrupted by process_yield() in the middle of printing, after printing its own ID.
 */
void interrupted_print_to_maxprocesss();

/**
 * Generate as many processs as possible
 */
void fill_process_list();

/**
 * Prints 1 million numbers to the console.
 */
void print_1mil_nums();
