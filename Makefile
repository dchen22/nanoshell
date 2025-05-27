# Variables
CC      := gcc
CFLAGS  := -Wall -Wextra -Wno-deprecated-declarations -g -D_XOPEN_SOURCE=700 

# List of object files
OBJS    := process.o queue.o basic_functions.o 

# Default target
all: process

# Link the executable
process: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Compile process.c (depends on queue.h)
process.o: processlib/process.c queuelib/queue.h
	$(CC) $(CFLAGS) -c $<

# Compile queue.c (depends on queue.h)
queue.o: queuelib/queue.c queuelib/queue.h
	$(CC) $(CFLAGS) -c $<

basic_functions.o: processlib/basic_functions.c processlib/basic_functions.h
	$(CC) $(CFLAGS) -c $<

# Clean up build artifacts
.PHONY: clean
clean:
	rm -f $(OBJS) process
