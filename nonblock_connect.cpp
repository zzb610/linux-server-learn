#include <fcntl.h>

int setnonblock(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstdio>
#include <cerrno>
#include <unistd.h>
int unblock_connect(const char*ip, int port, int time)
{
    sockaddr_in address;
    address.sin_port = htons(port);
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);

    int ret = 0;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int fdopt = setnonblock(sockfd);
    ret = connect(sockfd, (sockaddr*)&address, sizeof(address));
    if(ret == 0)
    {
        printf("connect with server immediatly\n");
        // reset 
        fcntl(sockfd, F_SETFL, fdopt);
        return sockfd;
    }
    else if(ret != EINPROGRESS)
    {
        printf("unblock connect not support.\n");
        return -1;
    }
    fd_set readfds;
    fd_set writefds;
    timeval timeout;

    FD_ZERO(&readfds);
    FD_SET(sockfd, &writefds);

    timeout.tv_sec = time;
    timeout.tv_usec = 0;

    ret = select(sockfd+1, &readfds, &writefds, NULL, &timeout);
    if(ret <= 0)
    {
        printf("connection time out \n");
        close(sockfd);
        return -1;
    }
    if(! FD_ISSET(sockfd, &writefds))
    {
        printf("no events on sockfd found\n");
        close(sockfd);
        return -1;
    }
    int error = 0;
    socklen_t length = sizeof(error);
    if(getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &length) < 0)
    {
        printf("get socket option failed\n");
        close(sockfd);
        return -1;
    }
    if(errno != 0)
    {
        printf("Connection failed after select with the error: %d\n", error);
        close(sockfd);
        return -1;
    }
    printf("connection ready after select with the socket: %d \n", sockfd);
    // reset 
    fcntl(sockfd, F_SETFL, fdopt);
    return sockfd;
}

#include <cstring>
#include <cstdlib>
int main(int argc, char const *argv[])
{
    if(argc <= 2)
    {
        printf("usage: %s ip port\n", basename(argv[0]));
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);

    int sockfd = unblock_connect(ip, port, 10);
    if(sockfd < 0)
    {
        return 1;
    }
    close(sockfd);
    return 0;
}
