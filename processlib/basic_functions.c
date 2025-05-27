#include <stdio.h>
#include "process.h"

void print_hello_world() {
    printf("hello world\n");
}

void print_identity() {
    process_t *curr_thr = get_current_process();
    if (curr_thr == NULL) {
        printf("Error: current process is NULL\n");
        return;
    }

    if (curr_thr->id % 2 == 0) {
        process_yield();
    }

    printf("I am process %u\n", curr_thr->id);
}

void interrupted_print_to_maxprocesss() {
    unsigned int i = 0;
    while (i < MAX_PROCESSES) {
        printf("%d ", i);
        if (i == get_current_process()->id) {
            process_yield();
        }
        i++;
    }
}

void print_1mil_nums() {
    for (unsigned int i = 0; i < 1e4; i++) {
        printf("%d ", i);
    }
    printf("\n\n");
}