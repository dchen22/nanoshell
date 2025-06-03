#include <stdio.h>
#include "process.h"
#include "basic_functions.h"
#include <assert.h>

void print_hello_world() {
    printf("hello world\n");
}

void print_identity(void *args) {
    (void)args; // unused
    
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


void generate_numbers(void *args) {
    assert(args != NULL);
    generate_numbers_args_t *gen_args = (generate_numbers_args_t*)args;

    for (unsigned int i = 0; i < gen_args->num_integers; i++) {
        // if (i % 1000 == 0) {
            printf("%d ", i);
        // }
        
    }

    free(gen_args);
}
