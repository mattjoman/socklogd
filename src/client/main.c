#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>



void child_loop(int child_no) {
    int fd;
    char *msg;
    struct sockaddr_un addr = {0};

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, "/tmp/socklogd.sock");
    if (connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) < 0) {
        perror("connect");
        return;
    }
    msg = "Hello";
    for (;;) {
        sleep(0.1);
        printf("%d\n", child_no);
        write(fd, "Hello", strlen(msg));
    }
}

void parent_loop() {
    for (;;) {
        sleep(10000);
    }
}



int main(int argc, char *argv[]) {
    int n;
    pid_t pid;

    while ((opt = getopt(argc, argv, "n:r:")) != -1) {
    switch (opt) {
        case 'n': n = atoi(optarg); break;
        case 'r': rate = atoi(optarg); break;
        default:
            fprintf(stderr, "Usage: %s [-n children] [-r ms] socket\n", argv[0]);
            return 1;
        }
    }
    
    n = 7;

    for (int i = 0; i < n; i++) {
        if ((pid = fork()) < 0) {
            perror("fork");
            return 1;
        } else if (pid == 0) {
            child_loop(i);
            break;
        }
    }

    if (pid > 0)
        parent_loop();

    return 0;
}
