#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#define SERVER_IP   "127.0.0.1"
#define SERVER_PORT 1111
#define BACKLOG     8
#define MAXBUF     1024
#define MAXEVENTS   64

void Exit(char *);

static int serverfd = 0;

int main()
{
    printf("Start server...\n");

    // Create server socket
    if ((serverfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        Exit("Failed to create server socket file descriptor");

    // Bind socket to ip and port
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    server.sin_addr.s_addr = inet_addr(SERVER_IP);
    if (bind(serverfd, (struct sockaddr*)&server, sizeof(server)) < 0)
        Exit("Failed to bind to socket");

    // Start listening on socket
    if (listen(serverfd, BACKLOG) < 0)
        Exit("Failed to start listening");

    // Create epoll instance
    int epfd;
    if ((epfd = epoll_create(1)) < 0)
        Exit("Failed to create epoll file descriptor");
    
    // Add serverfd to epoll instance
    struct epoll_event eevent = {0};
    eevent.events = EPOLLIN;
    eevent.data.fd = serverfd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, serverfd, &eevent) < 0)
        Exit("Failed to add server file descriptor to epoll instance");

    struct epoll_event events[MAXEVENTS];
    int clientfd, nfds, i, bytes_read;
    struct sockaddr_in client;
    socklen_t client_size = sizeof(client);
    char buf[MAXBUF];
    while (i < 2) {
        memset(&events, 0, MAXEVENTS);
        nfds = epoll_wait(epfd, events, MAXEVENTS, 36000);
        for (i = 0; i < nfds; i++) {
            if (events[i].data.fd == serverfd) {
                // If fd is server then accept the incoming connection and add to epoll instance
                clientfd = accept(serverfd, (struct sockaddr*)&client, &client_size);
                memset(&eevent, 0, sizeof(eevent));
                eevent.events = EPOLLIN;
                eevent.data.fd = clientfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &eevent);
            } else {
                // Otherwise this is a client fd and needs to handled
                if (events[i].events & EPOLLHUP) {
                    // The client has closed its connection and should be removed from the epoll instance
                    epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                } else {
                    memset(&buf, 0, sizeof(buf));
                    bytes_read = recv(events[i].data.fd, &buf, MAXBUF, 0);
                    printf("Received: %s\n", buf);
                    if (strcmp(buf, "quit\n") == 0) {
                        goto STOP;
                    }
                }
            }
        }
    }
    STOP:

    printf("Shutting down...\n");
    close(serverfd);
    return 0;
}

void Exit(char *msg) {
    if (serverfd != 0)
        close(serverfd);
    perror(msg);
    exit(errno);
}