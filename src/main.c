#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <poll.h>
#include <pthread.h>

void handle_error(char *message) {
    perror(message);
    exit(1);
}

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

    printf("%s\n", q->tail->ntry.message);

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












void *worker_thread(void *arg) {
    struct log_queue *q;
    struct log_entry ntry;
    FILE *fp;

    q = (struct log_queue *)arg;
    for (;;) {
        ntry = log_queue_deq(q);
        //printf("Worker: %s\n", ntry.message);
        if (!(fp = fopen("/tmp/socklogd.log", "a")))
            handle_error("open");
        fprintf(fp, "%s\n", ntry.message);
        fclose(fp);
    }
}












struct log_entry new_log_entry() {
    struct log_entry ntry;
    clock_gettime(CLOCK_REALTIME, &(ntry.ts));
    //printf("%lu\n", ntry.ts.tv_nsec);
    return ntry;
}


int socket_listen() {
    int fd;
    struct sockaddr_un addr;

    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
        handle_error("socket");

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, "/tmp/socklogd.sock", sizeof(addr.sun_path) -1);

    if (unlink(addr.sun_path) < 0 && errno != ENOENT)
        handle_error("unlink");
    
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        handle_error("bind");

    if (listen(fd, 128) < 0)
        handle_error("listen");

    return fd;
}

#define MAX_CLIENTS 128

void event_loop(int listen_fd, struct log_queue *q) {
    struct log_entry ntry;
    struct pollfd fds[MAX_CLIENTS + 1];
    int nfds, n, cfd;
    ssize_t r, truncated_len;
    char buf[1024];

    nfds = 1;
    fds[0].fd = listen_fd;
    fds[0].events = POLLIN;

    for (;;) {
        if ((n = poll(fds, nfds, -1)) <= 0)
            continue;

        /* New Client */
        if (fds[0].revents & POLLIN) {
            cfd = accept(listen_fd, NULL, NULL);
            fds[nfds].fd = cfd;
            fds[nfds].events = POLLIN;
            nfds++;
        }

        for (int i = 1; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                r = read(fds[i].fd, buf, sizeof(buf));

                if (r <= 0) {
                    close(fds[i].fd);
                    fds[i] = fds[--nfds];
                    i--;
                } else {
                    truncated_len = (r < LOG_MESSAGE_MAX_LEN) ? r : LOG_MESSAGE_MAX_LEN;
                    memcpy(ntry.message, buf, truncated_len);
                    ntry.message_len = truncated_len;
                    log_queue_enq(q, ntry); // thread-safe queue
                }
            }
        }
    }
}



void daemonise() {
    pid_t pid;
    int fd;

    if ((pid = fork()) < 0)
        exit(1);
    if (pid > 0)
        _exit(0);

    if (setsid() < 0)
        exit(1);

    if ((pid = fork()) < 0)
        exit(1);
    if (pid > 0)
        _exit(0);

    umask(027);
    chdir("/");
    
    if ((fd = open("/dev/null", O_RDWR)) < 0)
        exit(1);

    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);

    if (fd > 2)
        close(fd);

}

int main(int argc, char *argv[]) {
    pthread_t tid;
    struct log_queue *q;
    int foreground, opt, listen_fd;

    foreground = 0;
    while ((opt = getopt(argc, argv, "f")) != -1) {
        switch (opt) {
            case 'f':
                foreground = 1;
                break;
            default:
                exit(1);
        }
    }

    if (!foreground) {
        printf("Daemon\n");
        daemonise();
    } else {
        printf("Not daemon\n");
    }


    q = log_queue_init();

    pthread_create(&tid, NULL, worker_thread, q);

    listen_fd = socket_listen();
    event_loop(listen_fd, q);


    pthread_join(tid, NULL);

    return 0;
}

