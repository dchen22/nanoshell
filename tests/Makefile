# Compiler & flags
CC      := gcc
CFLAGS  := -Wall -Wextra -Wno-deprecated-declarations -g -D_XOPEN_SOURCE=700 

# Source files
SRCS    := test_basic.c ../fslib/tfs.c

# Object files (same paths, but .o)
OBJS    := $(SRCS:.c=.o)

# The final executable
TARGET  := test_basic

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

