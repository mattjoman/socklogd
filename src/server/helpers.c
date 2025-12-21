#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <errno.h>

#include "./helpers.h"




void handle_error(char *message) {
    perror(message);
    exit(1);
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

