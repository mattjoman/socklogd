#include <stdio.h>

#include "./worker.h"
#include "./log_queue.h"




void *worker_thread(void *arg) {
    struct log_queue *q;
    struct log_entry ntry;
    FILE *fp;

    q = (struct log_queue *)arg;
    for (;;) {
        ntry = log_queue_deq(q);
        if (!(fp = fopen("/tmp/socklogd.log", "a")))
            continue;
        fprintf(fp, "%s\n", ntry.message);
        fclose(fp);
    }
}





