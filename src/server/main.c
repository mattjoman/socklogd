#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdatomic.h>
#include <poll.h>
#include <sys/socket.h>
#include <stdbool.h>

#include "./helpers.h"
#include "./log_queue.h"
#include "./worker.h"

#define MAX_CLIENTS 128




struct log_entry new_log_entry() {
    struct log_entry ntry;
    clock_gettime(CLOCK_REALTIME, &(ntry.ts));
    return ntry;
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



atomic_bool graceful_shutdown = false;
void sigint_handler() {
    graceful_shutdown = true;
}




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
        if (graceful_shutdown)
            return;

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




int main(int argc, char *argv[]) {
    int is_daemon, opt, listen_fd;
    pthread_t tid;
    struct log_queue *q;
    struct sigaction sa;

    sa.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa, NULL);

    is_daemon = 0;
    while ((opt = getopt(argc, argv, "d")) != -1) {
        switch (opt) {
            case 'd':
                is_daemon = 1;
                break;
            default:
                exit(1);
        }
    }

    if (is_daemon) {
        printf("Daemon\n");
        daemonise();
    } else {
        printf("Not daemon\n");
    }


    q = log_queue_init();

    pthread_create(&tid, NULL, worker_thread, q);

    listen_fd = socket_listen();
    event_loop(listen_fd, q);

    log_queue_shutdown(q);
    pthread_join(tid, NULL);
    free(q);

    return 0;
}

