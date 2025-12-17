#include <stdlib.h>

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

    pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->mutex);
}




struct log_entry log_queue_deq(struct log_queue *q) {
    struct log_queue_element *e;
    struct log_entry ntry;

    pthread_mutex_lock(&q->mutex);

    while (!q->head) {
        pthread_cond_wait(&q->cond, &q->mutex);
    }

    e = q->head;
    ntry = e->ntry;

    q->head = e->next;
    if (!q->head)
        q->tail = NULL;

    free(e);

    pthread_mutex_unlock(&q->mutex);

    return ntry;
};


