CC=gcc

SERVER_CFLAGS=-Include -O0 -Wall -Wextra -std=gnu11 -pthread -g
SERVER_SRCS=src/server/main.c src/server/log_queue.c src/server/worker.c src/server/helpers.c
SERVER_OBJS=$(SERVER_SRCS:.c=.o)
SERVER_OUT=socklogd.out

CLIENT_CFLAGS=-Include -O2 -Wall -Wextra -std=gnu11
CLIENT_SRCS=src/client/main.c
CLIENT_OBJS=$(CLIENT_SRCS:.c=.o)
CLIENT_OUT=client.out

all: $(SERVER_OUT) $(CLIENT_OUT)

# Link objects
$(SERVER_OUT): $(SERVER_OBJS)
	$(CC) $(SERVER_CFLAGS) -o $@ $(SERVER_OBJS)

# Link objects
$(CLIENT_OUT): $(CLIENT_OBJS)
	$(CC) $(CLIENT_CFLAGS) -o $@ $(CLIENT_OBJS)

# Compile sources
src/server/%.o: src/server/%.c
	$(CC) $(SERVER_CFLAGS) -c $< -o $@

src/client/%.o: src/client/%.c
	$(CC) $(CLIENT_CFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(SERVER_OBJS) $(SERVER_OUT) $(CLIENT_OBJS) $(CLIENT_OUT)

.PHONY: all clean
