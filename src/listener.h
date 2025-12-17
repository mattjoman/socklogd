#ifndef LISTENER_H
#define LISTENER_H

#include "./log_queue.h"

#define MAX_CLIENTS 128

int socket_listen();

void event_loop(int listen_fd, struct log_queue *q);

#endif

