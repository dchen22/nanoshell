# Compiler & flags
CC      := gcc
CFLAGS  := -Wall -Wextra -Wno-deprecated-declarations -g -D_XOPEN_SOURCE=700 

# Source files
SRCS    := main.c fslib/tfs.c clilib/cli.c clilib/vim.c

# Object files (same paths, but .o)
OBJS    := $(SRCS:.c=.o)

# The final executable
TARGET  := nanoshell

.PHONY: all clean

all: $(TARGET)

# Link step
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile any .c -> .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

