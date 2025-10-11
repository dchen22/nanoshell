#include "vim.h"
#include <string.h>
#include <wchar.h> // For wide character functions like wprintf
#include <locale.h> // For setting the locale
#include <sys/ioctl.h>

/* escape sequences to clear screen and reposition cursor to (1,1) */
#define CLEAR_SCREEN() write(STDOUT_FILENO, "\x1b[2J", 4)
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

// Enable alternate screen buffer (disable scrolling)
void DISABLE_SCROLLING() {
    write(STDOUT_FILENO, "\x1b[?1049h", 8);
}
// Disable alternate screen buffer (re-enable scrolling, restore screen)
void ENABLE_SCROLLING() {
    write(STDOUT_FILENO, "\x1b[?1049l", 8);
}

/* A fixed‐size buffer for simplicity.  You can increase MAX_BUFFER
 * or make this dynamically grow if needed.
 */
#define MAX_BUFFER  4096

static char buffer[MAX_BUFFER];
static size_t buf_len = 0;   /* how many valid bytes are in `buffer[]` */
static size_t cursor_pos = 0; /* cursor position within buffer */
static int row_offset = 0;    /* first line visible on screen (for scrolling) */


static int get_window_size(int *rows, int *cols) {
    struct winsize ws;
    
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        return -1;  // Failed to get window size
    }
    
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
}

/*
 * refresh_screen()
 *
 *  1. Clears the entire screen.
 *  2. Renders visible lines (accounting for row_offset)
 *  3. Draws status line at bottom
 *  4. Positions cursor at cursor_pos (relative to viewport)
 */
static void refresh_screen(void) {
    int rows, cols;
    if (get_window_size(&rows, &cols) == -1) {
        rows = 24;
        cols = 80;
    }
    
    int visible_rows = rows - 1;  // Reserve last row for status line
    
    CLEAR_SCREEN();
    MOVE_CURSOR_1_1;
    
    // Calculate cursor's absolute row/col in buffer
    int cursor_row = 0, cursor_col = 0;
    for (size_t i = 0; i < cursor_pos && i < buf_len; i++) {
        if (buffer[i] == '\n') {
            cursor_row++;
            cursor_col = 0;
        } else {
            cursor_col++;
        }
    }
    
    // Adjust row_offset to keep cursor visible
    if (cursor_row < row_offset) {
        row_offset = cursor_row;
    }
    if (cursor_row >= row_offset + visible_rows) {
        row_offset = cursor_row - visible_rows + 1;
    }
    
    // Render visible lines only
    int current_row = 0;
    int screen_row = 0;
    size_t i = 0;
    
    // Skip lines before row_offset
    while (i < buf_len && current_row < row_offset) {
        if (buffer[i] == '\n') {
            current_row++;
        }
        i++;
    }
    
    // Render visible lines
    while (i < buf_len && screen_row < visible_rows) {
        write(STDOUT_FILENO, &buffer[i], 1);
        if (buffer[i] == '\n') {    
            screen_row++;
            current_row++;
        } 
        i++;
    }
    
    // Draw status line at bottom
    char pos_cmd[32];
    int len = snprintf(pos_cmd, sizeof(pos_cmd), "\x1b[%d;1H", rows);
    write(STDOUT_FILENO, pos_cmd, len);
    
    // Count total lines in buffer
    int total_lines = 1;  // At least one line
    for (size_t j = 0; j < buf_len; j++) {
        if (buffer[j] == '\n') {
            total_lines++;
        }
    }
    
    char status[128];
    len = snprintf(status, sizeof(status), 
                   "\x1b[7m Press ESC to save | Line %d/%d | Chars: %zu \x1b[0m\x1b[K",
                   cursor_row + 1, total_lines, buf_len);
    write(STDOUT_FILENO, status, len);
    
    // Position cursor at correct screen position
    int screen_cursor_row = cursor_row - row_offset + 1;  // +1 for 1-based indexing
    int screen_cursor_col = cursor_col + 1;  // +1 for 1-based indexing
    
    len = snprintf(pos_cmd, sizeof(pos_cmd), "\x1b[%d;%dH", screen_cursor_row, screen_cursor_col);
    write(STDOUT_FILENO, pos_cmd, len);
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





char *run_editor(char *initial_content) {
    enable_raw_mode();
    /* Ensure raw‐mode is undone on exit */
    atexit(disable_raw_mode);
    atexit(ENABLE_SCROLLING);

    DISABLE_SCROLLING();

    if (initial_content != NULL) {
        /* Copy initial content into the buffer */
        buf_len = strnlen(initial_content, MAX_BUFFER - 1);
        if (buf_len >= MAX_BUFFER) {
            buf_len = MAX_BUFFER - 1; // ensure we don't overflow
        }
        memcpy(buffer, initial_content, buf_len);
        buffer[buf_len] = '\0'; // null-terminate the buffer
        // cursor_pos = buf_len;  // Start cursor at end of content
    } else {
        buf_len = 0; // start with an empty buffer
        
    }
    cursor_pos = 0;  // Start cursor at beginning
    row_offset = 0;  // Start at top of file
    MOVE_CURSOR_1_1;

    /* Initial draw (empty buffer) */
    refresh_screen();

    while (1) {
        int c = read_key();
        if (c == -1) {
            /* no key within timeout; loop again */
            continue;
        }

        if ((unsigned char)c == 27) {
            char seq[2];
            if (read(STDIN_FILENO, seq, 2) != 2) {
                /* ESC pressed → exit editor */
                MOVE_CURSOR_1_1;
                CLEAR_SCREEN();
                break;
            } else {
                if (seq[0] == '[') {    // arrow keys
                    if (seq[1] == 'A') {    // up arrow
                        // Move cursor up one line
                        // Find the previous newline from cursor_pos
                        if (cursor_pos > 0) {
                            // Go back to find current line start
                            size_t line_start = cursor_pos;
                            while (line_start > 0 && buffer[line_start - 1] != '\n') {
                                line_start--;
                            }
                            
                            if (line_start > 0) {
                                // We're not on the first line, move up
                                // Find the previous line start
                                size_t prev_line_start = line_start - 1;  // Skip the \n
                                while (prev_line_start > 0 && buffer[prev_line_start - 1] != '\n') {
                                    prev_line_start--;
                                }
                                
                                // Calculate column offset
                                size_t col_offset = cursor_pos - line_start;
                                
                                // Move to same column on previous line (or end of that line)
                                size_t prev_line_len = (line_start - 1) - prev_line_start;
                                cursor_pos = prev_line_start + (col_offset < prev_line_len ? col_offset : prev_line_len);
                            }
                        }
                    } else if (seq[1] == 'B') {    // down arrow
                        // Move cursor down one line
                        if (cursor_pos < buf_len) {
                            // Find current line start
                            size_t line_start = cursor_pos;
                            while (line_start > 0 && buffer[line_start - 1] != '\n') {
                                line_start--;
                            }
                            size_t col_offset = cursor_pos - line_start;
                            
                            // Find next newline
                            size_t next_line_start = cursor_pos;
                            while (next_line_start < buf_len && buffer[next_line_start] != '\n') {
                                next_line_start++;
                            }
                            
                            if (next_line_start < buf_len) {
                                next_line_start++;  // Skip the \n
                                
                                // Find end of next line
                                size_t next_line_end = next_line_start;
                                while (next_line_end < buf_len && buffer[next_line_end] != '\n') {
                                    next_line_end++;
                                }
                                
                                size_t next_line_len = next_line_end - next_line_start;
                                cursor_pos = next_line_start + (col_offset < next_line_len ? col_offset : next_line_len);
                            }
                        }
                    } else if (seq[1] == 'C') {    // right arrow
                        // Move cursor right one character
                        if (cursor_pos < buf_len) {
                            cursor_pos++;
                        }
                    } else if (seq[1] == 'D') {    // left arrow
                        // Move cursor left one character
                        if (cursor_pos > 0) {
                            cursor_pos--;
                        }
                    }
                }
            }
        }
        else if (c >= 32 && c <= 126) {
            /* Printable ASCII: insert at cursor position */
            if (buf_len < MAX_BUFFER - 1) {
                // Shift everything after cursor_pos to the right
                memmove(buffer + cursor_pos + 1, buffer + cursor_pos, buf_len - cursor_pos);
                buffer[cursor_pos] = (char)c;
                buf_len++;
                cursor_pos++;  // Move cursor forward
                buffer[buf_len] = '\0';
            }
        }
        else if (c == 127 || c == '\b') {
            /* Backspace: remove character before cursor */
            if (cursor_pos > 0) {
                // Shift everything after cursor_pos to the left
                memmove(buffer + cursor_pos - 1, buffer + cursor_pos, buf_len - cursor_pos);
                buf_len--;
                cursor_pos--;  // Move cursor back
                buffer[buf_len] = '\0';
            }
        }
        else if (c == '\r' || c == '\n') {
            // Enter: insert a newline at cursor position
            if (buf_len < MAX_BUFFER - 1) {
                // Shift everything after cursor_pos to the right
                memmove(buffer + cursor_pos + 1, buffer + cursor_pos, buf_len - cursor_pos);
                buffer[cursor_pos] = '\n';
                buf_len++;
                cursor_pos++;  // Move cursor forward
                buffer[buf_len] = '\0';
            }
        }
        /* (You can add more key handling here: Enter, arrows, etc.) */

        /* Redraw after every keystroke */
        refresh_screen();
    }

    disable_raw_mode();
    ENABLE_SCROLLING();

    return buffer; // return the buffer containing the edited text
}

