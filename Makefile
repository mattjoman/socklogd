CC=gcc
CFLAGS=-Include -O2 -Wall -Wextra -std=gnu11 -pthread

SRCS=src/main.c src/log_queue.c
OBJS=$(SRCS:.c=.o)

OUT=out

all: $(OUT)

# Link objects
$(OUT): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Compile sources
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(OBJS) $(OUT)

.PHONY: all clean
