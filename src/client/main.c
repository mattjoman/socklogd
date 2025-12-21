#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>


int connect_socket() {
    int fd;
    struct sockaddr_un addr = {0};

    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        _exit(1);
    }
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, "/tmp/socklogd.sock");
    if (connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) < 0) {
        perror("connect");
        _exit(1);
    }
    return fd;
}



void child_loop(int child_no, int rate) {
    int fd;
    float sleep_time;
    char *msg;

    fd = connect_socket();

    msg = "Hello";
    sleep_time = 1000000.0 / rate;
    printf("%d\n", child_no);
    for (;;) {
        printf("yes\n");
        usleep(sleep_time);
        write(fd, msg, strlen(msg));
    }
    close(fd);
}

void parent_loop() {
    for (;;) {
        sleep(10000);
    }
}



int main(int argc, char *argv[]) {
    int n, rate, opt;
    pid_t pid;
    
    n = 1;
    rate = 1;

    while ((opt = getopt(argc, argv, "n:r:")) != -1) {
    switch (opt) {
        case 'n':
            n = atoi(optarg);
            break;
        case 'r':
            rate = atoi(optarg);
            break;
        default:
            fprintf(stderr, "Usage: %s [-n children] [-r ms] socket\n", argv[0]);
            return 1;
        }
    }

    for (int i = 0; i < n; i++) {
        if ((pid = fork()) < 0) {
            perror("fork");
            return 1;
        } else if (pid == 0) {
            child_loop(i, rate);
            break;
        }
    }

    if (pid > 0)
        parent_loop();

    return 0;
}
