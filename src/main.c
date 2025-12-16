#include <stdio.h>
#include <time.h> // struct timespec
#include <sys/types.h> // uid_t, gid_t, pid_t

#define LOG_MESSAGE_MAX_LEN 64

struct log_entry {
    struct timespec ts;
    pid_t client_pid;
    uid_t client_uid;
    size_t len;
    char message[LOG_MESSAGE_MAX_LEN];
};

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
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

int main() {
    struct log_entry ntry;
    clock_gettime(CLOCK_REALTIME, &(ntry.ts));
    printf("%lu\n", ntry.ts.tv_nsec);
    daemonise();
    return 0;
}

