#include <stdlib.h>
#include <stdio.h>

#include "./log_queue.h"


struct log_queue *log_queue_init() {
    struct log_queue *q;
    q = malloc(sizeof(struct log_queue));
    q->head = q->tail = NULL;
    q->shutdown = 0;
    q->queue_len = 0;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);
    return q;
}




void log_queue_shutdown(struct log_queue *q) {
    pthread_mutex_lock(&q->mutex);

    q->shutdown = 1;

    pthread_cond_broadcast(&q->cond);
    pthread_mutex_unlock(&q->mutex);
}




void log_queue_enq(struct log_queue *q, struct log_entry ntry) {
    struct log_queue_element *e;

    e = malloc(sizeof(struct log_queue_element));
    e->ntry = ntry;
    e->next = NULL;

    pthread_mutex_lock(&q->mutex);

    if (q->tail)
        q->tail->next = e;
    else
        q->head = e;

    q->tail = e;

    q->queue_len++;

    pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->mutex);
}




int log_queue_deq(struct log_queue *q, struct log_entry *ntry) {
    struct log_queue_element *e;

    pthread_mutex_lock(&q->mutex);

    while (!q->head && !q->shutdown) {
        pthread_cond_wait(&q->cond, &q->mutex);
    }

    if (!q->head && q->shutdown) {
        printf("Queue emptied, shutdown\n");
        return 1; // no entries consumed and shutdown
    }

    e = q->head;
    *ntry = e->ntry;

    q->head = e->next;
    if (!q->head)
        q->tail = NULL;

    q->queue_len--;

    free(e);

    pthread_mutex_unlock(&q->mutex);

    return 0; // log entry was consumed
};


