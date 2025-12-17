#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <errno.h>
#include <poll.h>

#include "./listener.h"
#include "./helpers.h"




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
