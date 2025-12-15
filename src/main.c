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

int main() {
    struct log_entry ntry;
    clock_gettime(CLOCK_REALTIME, &(ntry.ts));
    printf("%lu\n", ntry.ts.tv_nsec);
    return 0;
}

