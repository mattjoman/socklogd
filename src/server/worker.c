#include <stdio.h>

#include "./worker.h"
#include "./log_queue.h"




void *worker_thread(void *arg) {
    int safe_to_shutdown;
    struct log_queue *q;
    struct log_entry ntry;
    FILE *fp;

    q = (struct log_queue *)arg;
    for (;;) {
        safe_to_shutdown = log_queue_deq(q, &ntry);
        if (safe_to_shutdown)
            break;
        if (!(fp = fopen("/tmp/socklogd.log", "a")))
            continue;
        fprintf(fp, "%s\n", ntry.message);
        fclose(fp);
    }
    printf("Worker returning\n");
    return NULL;
}





