#include "vim.h"


/* escape sequences to clear screen and reposition cursor to (1,1) */
#define CLEAR_SCREEN()   write(STDOUT_FILENO, "\x1b[2J", 4)
#define MOVE_CURSOR_1_1 write(STDOUT_FILENO, "\x1b[H", 3)

static struct termios orig_termios;

void enable_raw_mode() {
    // save current termios settings
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        perror("tcgetattr");
        exit(-1);
    }

    // make a copy of the original settings
    struct termios raw = orig_termios;

    // disable ctrl-c and ctrl-z (do not disable for now tho)
    // raw.c_lflag &= ~ISIG;

    // disable echo and canonical mode
    raw.c_lflag &= ~(ICANON | ECHO);

    // disable ctrl-s and ctrl-q
    raw.c_iflag &= ~IXON;

    // disable cr -> nl translation
    raw.c_iflag &= ~ICRNL;

    // set minimum data for read()
    raw.c_cc[VMIN] = 0; // no minimum data
    raw.c_cc[VTIME] = 1; // 100ms timeout

    // apply new settings
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr");
        exit(-1);
    }
}

void disable_raw_mode() {
    // restore original termios settings
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
        perror("tcsetattr");
        exit(-1);
    }
}

/* A fixed‐size buffer for simplicity.  You can increase MAX_BUFFER
 * or make this dynamically grow if needed.
 */
#define MAX_BUFFER  4096

static char buffer[MAX_BUFFER];
static size_t buf_len = 0;   /* how many valid bytes are in `buffer[]` */

/*
 * refresh_screen()
 *
 *  1. Clears the entire screen.
 *  2. Moves the cursor to row 1, col 1.
 *  3. Writes the contents of `buffer[0..buf_len-1]`.
 */
static void refresh_screen(void) {
    CLEAR_SCREEN();
    MOVE_CURSOR_1_1;

    if (buf_len > 0) {
        write(STDOUT_FILENO, buffer, buf_len);
    }
}



/*
 * read_key()
 *
 * Attempts to read exactly one byte from STDIN.  Because raw mode is set
 * with VMIN=0, VTIME=1, this will:
 *   - return the byte read (0–255) if available
 *   - return -1 if no byte arrives within 0.1s
 */
static int read_key(void) {
    char c;
    ssize_t n = read(STDIN_FILENO, &c, 1);
    if (n == 1) return (unsigned char)c;
    return -1;
}





char *run_editor(void) {
    enable_raw_mode();
    /* Ensure raw‐mode is undone on exit */
    atexit(disable_raw_mode);

    /* Initial draw (empty buffer) */
    refresh_screen();

    while (1) {
        int c = read_key();
        if (c == -1) {
            /* no key within timeout; loop again */
            continue;
        }

        if ((unsigned char)c == 27) {
            /* ESC pressed → exit editor */
            break;
        }
        else if (c >= 32 && c <= 126) {
            /* Printable ASCII: append if we have room */
            if (buf_len < MAX_BUFFER - 1) {
                buffer[buf_len++] = (char)c;
                buffer[buf_len]  = '\0';
            }
        }
        else if (c == 127 || c == '\b') {
            /* Backspace: remove last character, if any */
            if (buf_len > 0) {
                buf_len--;
                buffer[buf_len] = '\0';
            }
        }
        else if (c == '\r' || c == '\n') {
            // Enter: insert a newline into the buffer
            if (buf_len < MAX_BUFFER - 1) {
                buffer[buf_len++] = '\n';
                buffer[buf_len]   = '\0';
            }
        }
        /* (You can add more key handling here: Enter, arrows, etc.) */

        /* Redraw after every keystroke */
        refresh_screen();
    }

    disable_raw_mode();

    return buffer; // return the buffer containing the edited text
}

