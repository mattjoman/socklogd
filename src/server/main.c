#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "./listener.h"
#include "./log_queue.h"
#include "./worker.h"




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

