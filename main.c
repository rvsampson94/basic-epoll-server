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
#define BUFSIZE     1024

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
    eevent.events = EPOLLIN
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, serverfd, &eevent) < 0)
        Exit("Failed to add server file descriptor to epoll");

    // // Accept a single client connection
    // int clientfd;
    // struct sockaddr_in client;
    // socklen_t client_size = (size_t)sizeof(client);
    // memset(&client, 0, sizeof(client));
    // if ((clientfd = accept(serverfd, (struct sockaddr*)&clientfd, &client_size)) < 0)
    //     Exit("Client connection failed");

    // // Receive and print output from client
    // int bytes = 0;
    // char buf[BUFSIZE];
    // while(1) {
    //     memset(&buf, 0, sizeof(buf));
    //     bytes = recv(clientfd, &buf, BUFSIZE, 0);
    //     printf("Received Message: %s", &buf);
    //     if (strcmp(buf, "quit\n") == 0)
    //         break;
    // }

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