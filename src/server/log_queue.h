#ifndef LOG_QUEUE_H
#define LOG_QUEUE_H

#include <pthread.h>
#include <sys/types.h>

#define LOG_MESSAGE_MAX_LEN 64

struct log_entry {
    struct timespec ts;
    pid_t client_pid;
    uid_t client_uid;
    size_t message_len;
    char message[LOG_MESSAGE_MAX_LEN];
};

struct log_queue_element {
    struct log_entry ntry;
    struct log_queue_element *next;
};

struct log_queue {
    struct log_queue_element *head;
    struct log_queue_element *tail;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int shutdown;
    int queue_len;
};

struct log_queue *log_queue_init();

void log_queue_shutdown(struct log_queue *q);

void log_queue_enq(struct log_queue *q, struct log_entry ntry);

int log_queue_deq(struct log_queue *q, struct log_entry *ntry);

#endif

