#include <fcntl.h>
int setnonblock(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

#include <sys/epoll.h>
void addfd(int epollfd, int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblock(fd);
}

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cassert>
#include <cerrno>
#include <unistd.h>
#define MAX_EVENT_NUMBER 1024
#define UDP_BUFFER_SIZE 1024
#define TCP_BUFFER_SIZE 512
int main(int argc, char const *argv[])
{

    if (argc <= 2)
    {
        printf("usage: %s ip port\n", basename(argv[0]));
        return 1;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);

    int ret = 0;

    // tcp
    sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    inet_pton(AF_INET, ip, &address.sin_addr);

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    ret = bind(listenfd, (sockaddr*)&address, sizeof(address));
    assert(ret != -1);
    ret = listen(listenfd, 5);
    assert(ret != -1);

    // udp
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    inet_pton(AF_INET, ip, &address.sin_addr);

    int udpfd = socket(AF_INET, SOCK_DGRAM, 0);
    assert(udpfd >= 0);

    ret = bind(udpfd, (sockaddr*) &address, sizeof(address));
    assert(ret != -1);
    
    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    assert(epollfd != -1);

    addfd(epollfd, listenfd);
    addfd(epollfd, udpfd);

    while(true)
    {
        int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if(number < 0)
        {
            printf("epoll failure.\n");
            break;
        }
        for(int i=0;i < number ;++i)
        {
            int sockfd = events[i].data.fd;
            if(sockfd == listenfd)
            {
                sockaddr_in client_addr;
                socklen_t client_addrlen = sizeof(client_addr);
                int connfd = accept(sockfd, (sockaddr*) &client_addr, &client_addrlen);
                addfd(epollfd, connfd);
            }
            else if(sockfd == udpfd)
            {
                char buf[UDP_BUFFER_SIZE];
                memset(buf, '\0', UDP_BUFFER_SIZE);
                sockaddr_in client_addr;
                socklen_t client_addrlen = sizeof(client_addr);
                ret = recvfrom(sockfd, buf, UDP_BUFFER_SIZE-1, 0, (sockaddr*) &client_addr, &client_addrlen);
                // reflect
                if(ret > 0)
                {
                    sendto(udpfd, buf, UDP_BUFFER_SIZE-1, 0, (sockaddr*) &client_addr, client_addrlen);
                }
            }
            else if(events[i].events & EPOLLIN)
            {
                char buf[TCP_BUFFER_SIZE];
                // et
                while(true)
                {
                    memset(buf, '\0', TCP_BUFFER_SIZE);
                    ret = recv(sockfd, buf, TCP_BUFFER_SIZE-1, 0);
                    if(ret < 0)
                    {
                        if((errno == EAGAIN) | (errno == EWOULDBLOCK))
                        {
                            break;
                        }
                        close(sockfd);
                        break;
                    }
                    else if(ret == 0)
                    {
                        close(sockfd);
                    }
                    else
                    {
                        // reflect
                        send(sockfd, buf, ret, 0);
                    }
                }
            }
            else
            {
                printf("something else happend.\n");
            }
        }
    }
    close(listenfd);
    return 0;
}
