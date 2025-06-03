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

typedef void (*func_t)(void*); // used for process stub function

typedef unsigned int procid_t;

struct PCB {
    procid_t id;
    func_t stub_func; // function to run
    void *args; // arguments for the stub function
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
process_t *get_process(procid_t pid);

/**
 * Initializes the process library. Does not run any processes. 
 * 
 * @param main_stub_function The function to be executed by the main process. Should be a function that runs the main loop of the entire OS
 * @param args Pointer to a struct with arguments to be passed to the main process's stub function.
 * 
 * @return 0 on success, -1 on failure
 */
int init_processlib(void (*main_stub_function)(void *), void *args);

/**
 * Create, initialize and schedule PCB for a new process.
 * 
 * Pass in the process's stub function, the number of arguments, and the arguments themselves.
 * 
 * @param stub_function The function to be executed by the new process.
 * @param args Pointer to a struct with arguments to be passed to the stub function.
 * 
 * @return Returns the process ID of the newly created process, or -ERRNO on failure.
 */
int process_create(void (*stub_function)(void *),void *args);

/**
 * Return pointer to current process.
 */
process_t *get_current_process();

/**
 * Initialize the state data of process with the given process ID.
 * 
 * @param pid The process ID of the process to initialize.
 * @param stub_function The function to be executed by the process.
 * @param args Pointer to struct of args to be passed to the stub function.
 * 
 * Returns 0 on success, or -ERRNO on failure.
 */
int process_init(procid_t pid, func_t stub_function, void *args);

/**
 * Exit the process with the given process ID.
 * 
 * @param pid The process ID of the process to exit.
 * @return Returns 0 on success, or -ERRNO on failure.
 */
void process_exit(procid_t pid);

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
int cleanup_processlib(int exit_code);

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

/**
 * Schedule next process to run.
 */
void process_schedule(procid_t tid);

/**
 * Check if the scheduler queue is empty.
 * 
 * @return Returns true if the queue is empty, false otherwise.
 */
bool process_scheduler_is_empty();


/**
 * Should be run by scheduler.asm
 * 
 * Run the next process in the scheduler queue.
 */
void scheduler_run_next_process();


