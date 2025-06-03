#include "process.h"
#include "basic_functions.h"
#include "process.c"

int main() {
    // initialize process list
    init_processlib();
    
    // create scheduler queue. Exit on failure
    if (scheduler_queue_create() < 0) {
        printf("Failed to create scheduler queue\n");
        return cleanup_processlib(-1);
    }

    // create process0
    if (process_create(fill_process_list, NULL) < 0) {
        printf("Failed to create root process\n");
        return cleanup_processlib(-1);
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
        return cleanup_processlib(-1);
    }

    // run processs
    while (!queue_is_empty(scheduler_queue)) {
        current_process = queue_pop(scheduler_queue);    // get next process to run
        procid_t current_process_id = current_process->id;   // get its ID
        if (current_process == NULL) {                   // ensure its not NULL (if it is, nothing to run)
            printf("No more processs to run\n");
            return cleanup_processlib(-1);
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
    return cleanup_processlib(0);
}