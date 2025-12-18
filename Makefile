CC=gcc
CFLAGS=-Include -O2 -Wall -Wextra -std=gnu11 -pthread

SERVER_SRCS=src/server/main.c src/server/log_queue.c src/server/worker.c src/server/listener.c src/server/helpers.c
SERVER_OBJS=$(SERVER_SRCS:.c=.o)
SERVER_OUT=socklogd

CLIENT_SRCS=src/client/main.c
CLIENT_OBJS=$(CLIENT_SRCS:.c=.o)
CLIENT_OUT=client

all: $(SERVER_OUT) $(CLIENT_OUT)

# Link objects
$(SERVER_OUT): $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $@ $(SERVER_OBJS)

# Link objects
$(CLIENT_OUT): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $@ $(CLIENT_OBJS)

# Compile sources
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(SERVER_OBJS) $(SERVER_OUT) $(CLIENT_OBJS) $(CLIENT_OUT)

.PHONY: all clean
