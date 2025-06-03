#include <stdlib.h>
#include "process.h"
#include <stdio.h>
#include <assert.h>
#include "../queuelib/queue.h"
#include <ucontext.h>
#include "basic_functions.h"
#include <sys/ucontext.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdarg.h>



// array of PCBs pointers
process_t *process_list[MAX_PROCESSES];
// scheduler queue
fifo_queue_t *scheduler_queue = NULL;

ucontext_t main_context;

process_t *current_process = NULL;

unsigned int process_count = 0;

process_t *get_process(tid_t tid) {
    assert(tid < MAX_PROCESSES);
    assert(tid >= 0);
    return process_list[tid];
}

process_t *get_current_process() {
    return current_process;
}

void init_process_list() {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_list[i] = NULL;
    }
}

int process_create(void (*stub_function)(void *), void *args) {
    // check if process list is full
    if (process_count >= MAX_PROCESSES) {
        printf("Process list is full, cannot create more processs\n");
        return -TOO_MANY_PROCESSES;
    }
    // loop through process list and check for available slot
    for (int i = 0; i < MAX_PROCESSES; i++) {
        // if slot is available
        if (process_list[i] == NULL) {
            // create a new process
            process_t *new_process = malloc(sizeof(process_t));
            if (new_process == NULL) {
                return -ENOMEM;
            }
            // place in process list
            process_list[i] = new_process;

            // initialize the process
            process_init(i, stub_function, args);

            // increment process count
            process_count++;

            // schedule
            if (queue_push(scheduler_queue, new_process) != 0) {
                free(new_process);
                return -ENOMEM;
            }

            printf("Process %d created and initialized\n", new_process->id);

            // return process ID
            return i;
        }
    }
    return -TOO_MANY_PROCESSES;
}

int process_init(tid_t tid, func_t stub_function, void *args) {
    assert(get_process(tid) != NULL);
    process_t *process_ptr = get_process(tid);
    
    // initialize the process
    process_ptr->id = tid;
    process_ptr->stub_func = stub_function; 
    process_ptr->args = args; 
    process_ptr->state = PROCESS_ALIVE; // set process state to alive
    process_ptr->inQueue = false;
    process_ptr->ahead = NULL;
    process_ptr->behind = NULL;
    process_ptr->exit_code = NULL;

    // initialize process context
    if (getcontext(&process_ptr->context) < 0) {
        return -CONTEXT_ERROR;
    }
    // allocate stack for process
    void *stack = malloc(DEFAULT_STACK_SIZE);
    if (stack == NULL) {
        return -ENOMEM;
    }
    
    // set up the process context
    process_ptr->context.uc_stack.ss_sp = stack;
    process_ptr->context.uc_stack.ss_size = DEFAULT_STACK_SIZE;
    process_ptr->context.uc_link = &main_context; 

    // process will start executing at process_execute_stub when it is next run
    makecontext(&process_ptr->context, process_execute_stub, 0);

    return 0;
}

void process_yield() {
    assert(current_process != NULL);
    swapcontext(&current_process->context, &main_context); // save current process context and return to scheduler
}

void process_execute_stub() {
    // execute the process's function
    if (current_process->stub_func != NULL) {
        // printf("Process %d executing stub\n", current_process->id);
        current_process->stub_func(current_process->args);
    }
    // declare self as exited
    current_process->state = PROCESS_EXITED;
    // TODO: process should return a value
}

void process_exit(tid_t tid) {
    assert(tid < MAX_PROCESSES);
    assert(tid >= 0);
    assert(get_process(tid) != NULL);

    // free the process
    free(current_process);

    // set the slot to NULL
    process_list[tid] = NULL;

    // decrement process count
    process_count--;
}

void free_all_processs() {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_list[i] != NULL) {
            process_exit(i);
        }
    }
}

int scheduler_queue_create() {
    // create queue
    scheduler_queue = queue_create(MAX_PROCESSES);
    if (scheduler_queue == NULL) {
        return -ENOMEM;
    }
    return 0;
}

int clean_exit(int exit_code) {
    printf("Cleaning up...\n");
    // free all processs
    free_all_processs();
    // destroy queue
    while (scheduler_queue->usage > 0) {
        queue_pop(scheduler_queue);
    }
    queue_destroy(scheduler_queue);

    printf("Successfully cleaned up processes system.\n");

    return exit_code;
}




void fill_process_list(void *args) {
    (void)args; // unused
    for (unsigned int i = 1; i < MAX_PROCESSES; i++) {
        generate_numbers_args_t *gen_nums_args = malloc(sizeof(generate_numbers_args_t));
        gen_nums_args->num_integers = i;
        process_create(generate_numbers, gen_nums_args);
    }
}



void timer_jumpto_scheduler(int signum, siginfo_t *info, void *vuc) {
    (void)signum; // unused
    (void)info; // unused

    ucontext_t *process_context = (ucontext_t *)vuc;
    if (current_process == NULL) return; // don't yield if there is no current process, e.g. we are in the scheduler context
    current_process->context = *process_context; // save the current process context
    setcontext(&main_context); // switch to the scheduler
}

void timer_interrupt() {
    struct sigaction sa ;
    sa.sa_sigaction = timer_jumpto_scheduler; // run timer handler on timer interrupt
    sa.sa_flags = 0; // initialize flags
    sa.sa_flags |= SA_RESTART; // restart system calls
    sa.sa_flags |= SA_SIGINFO; // use siginfo_t
    sigemptyset(&sa.sa_mask);
    
    sigaction(SIGVTALRM, &sa, NULL); // set up signal handler

    struct itimerval timer;
    timer.it_value.tv_usec = 1000; // 1 ms initial delay
    timer.it_value.tv_sec = 0; // 0 seconds
    timer.it_interval.tv_usec = 1000; // 1 ms interval
    timer.it_interval.tv_sec = 0; // 0 seconds

    setitimer(ITIMER_REAL, &timer, NULL); // set up timer
}

void schedule() {

}


int main() {
    // initialize process list
    init_process_list();
    
    // create scheduler queue. Exit on failure
    if (scheduler_queue_create() < 0) {
        printf("Failed to create scheduler queue\n");
        return clean_exit(-1);
    }

    // create process0
    if (process_create(fill_process_list, NULL) < 0) {
        printf("Failed to create root process\n");
        return clean_exit(-1);
    }

    // initialize scheduler context
    getcontext(&main_context);

    printf("\nSCHEDULER RUNNING\n");

    // set up timer interrupt
    // timer_interrupt();

    // process0 will create other processs
    process_t *process0 = get_process(0);
    if (process0 == NULL) {
        printf("Process 0 not found\n");
        return clean_exit(-1);
    }

    // run processs
    while (!queue_is_empty(scheduler_queue)) {
        current_process = queue_pop(scheduler_queue);    // get next process to run
        tid_t current_process_id = current_process->id;   // get its ID
        if (current_process == NULL) {                   // ensure its not NULL (if it is, nothing to run)
            printf("No more processs to run\n");
            return clean_exit(-1);
        }
        printf("Running process %d\n", current_process->id);
        swapcontext(&main_context, &current_process->context);

        printf("\nSCHEDULER RUNNING\n");

        // check if process has exited
        if (current_process->state == PROCESS_EXITED) {
            process_exit(current_process_id);
        } else {
            // reschedule process that just ran
            queue_push(scheduler_queue, current_process);
        }

        
    }
   
    // Free all processs
    return clean_exit(0);
}