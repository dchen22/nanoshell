#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ucontext.h>

#define MAX_PROCESSES 100
#define DEFAULT_STACK_SIZE 8192

#define GENERIC_ERROR 1
#define ENOMEM 2
#define TOO_MANY_PROCESSES 3
#define CONTEXT_ERROR 4

typedef enum {
    PROCESS_ALIVE = 0,
    PROCESS_EXITED = 1,
} process_state_t;

typedef void (*func_t)(void); // used for process stub function

typedef unsigned int tid_t;

struct PCB {
    tid_t id;
    func_t stub_func; // function to run
    ucontext_t context; // context for the process

    struct PCB *ahead;
    struct PCB *behind;
    bool inQueue;

    process_state_t state; // process state
    void *exit_code; // exit code for the process, if any
};

typedef struct PCB process_t;

/**
 * Get a pointer to PCB for a given process ID.
 * 
 * This function retrieves the PCB from the process list using the provided process ID.
 * Return NULL if the process does not exist.
 */
process_t *get_process(tid_t tid);

/**
 * Initializes the process list to NULL.
 * 
 * This function allocates memory for an array of PCB pointers and initializes
 * each pointer to NULL. The size of the array is defined by MAX_PROCESSES.
 */
void _os_init_process_list();

/**
 * Create and initialize PCB for a new process.
 * 
 * Pass in the process's stub function, the number of arguments, and the arguments themselves.
 * 
 * @return Returns the process ID of the newly created process, or -ERRNO on failure.
 */
int _os_process_create(func_t stub_function);

/**
 * Return pointer to current process.
 */
process_t *get_current_process();

/**
 * Initialize the state data of process with the given process ID.
 * 
 * Returns 0 on success, or -ERRNO on failure.
 */
int _os_process_init(tid_t tid, func_t stub_function);

/**
 * Exit the process with the given process ID.
 * 
 * @param tid The process ID of the process to exit.
 * @return Returns 0 on success, or -ERRNO on failure.
 */
void _os_process_exit(tid_t tid);

/**
 * Free all processs in the process list.
 * 
 * This function iterates through the process list and frees each PCB that is not NULL.
 * It is used to clean up resources when the program is terminating.
 */
void free_all_processs();

/**
 * Create a process scheduler queue.
 * 
 * @return Returns 0 on success, or -ERRNO on failure.
 */
int scheduler_queue_create();

/**
 * Clean exit function.
 * 
 * Return exit code
 */
int _os_clean_exit(int exit_code);

/**
 * Yield the current process.
 * 
 * Next process in the queue will be run
 */
void process_yield();

/**
 * Execute process stub function
 */
void process_execute_stub();

