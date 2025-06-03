#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#define MAX_BUFFER  4096

/**
 * Enable raw mode for terminal. 
 * 
 * Raw mode disables certain terminal features like enter to input or ctrl-c to interrupt
 * 
 * All keypresses are taken literally
 */
void enable_raw_mode();

/**
 * Turn off raw mode for terminal
 */
void disable_raw_mode();

/**
 * Switches the terminal into raw mode, lets the user type into a simple
 * buffer (PRINTABLE ASCII only, plus Backspace), and exits on ESC.
 * Restores the terminal before returning.
 * 
 * @param initial_content Pointer to buffer of initial content to display in editor.
 * 
 * @return Pointer to the buffer containing the final (potentially modified) content of the editor.
 */
char *run_editor(char *initial_content);
