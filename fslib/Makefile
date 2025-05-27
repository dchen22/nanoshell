CC      := gcc
CFLAGS  := -Wall -Wextra -Wno-deprecated-declarations -g -D_XOPEN_SOURCE=700 

OBJS	:= fs.o bitmap.o mkfs.o 

all: fs

fs: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

fs.o : fslib/fs.c fslib/fs.h
	$(CC) $(CFLAGS) -c $<

bitmap.o : fslib/bitmap.c fslib/bitmap.h
	$(CC) $(CFLAGS) -c $<

mkfs.o : fslib/mkfs.c fslib/mkfs.h
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm -f $(OBJS) fs