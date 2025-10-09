# Compiler & flags
CC      := gcc
CFLAGS  := -Wall -Wextra -Wno-deprecated-declarations -g -D_XOPEN_SOURCE=700 

# Source files
SRCS    := main.c fslib/fs.c fslib/mkfs.c fslib/helpers.c fslib/fs_helper.c fslib/interface.c clilib/cli.c clilib/vim.c processlib/process.c processlib/basic_functions.c queuelib/queue.c

# Object files (same paths, but .o)
OBJS    := $(SRCS:.c=.o)

# The final executable
TARGET  := nanoshell

.PHONY: all clean fslib-tests

all: $(TARGET)

# Build fslib tests
fslib-tests:
	$(MAKE) -C fslib

# Link step
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile any .c -> .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
	$(MAKE) -C fslib clean

