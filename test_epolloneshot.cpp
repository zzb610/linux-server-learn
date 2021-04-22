#include <fcntl.h>
#include <sys/epoll.h>
#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <cstdlib>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cassert>
#include <pthread.h>

struct fds
{
    int epollfd;
    int sockfd;
};

int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

// add event in fd to kernal epollfd
void addfd(int epollfd, int fd, bool oneshot)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    if (oneshot)
    {
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

void reset_oneshot(int epollfd, int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

#define BUFSIZE 10

// working thread
void *worker(void *arg)
{
    int sockfd = ((fds *)arg)->sockfd;
    int epollfd = ((fds *)arg)->epollfd;
    printf("start new thread to receive data on fd: %d\n", sockfd);
    char buf[BUFSIZ];
    memset(buf, '\0', BUFSIZE);
    while (true)
    {
        int ret = recv(sockfd, buf, BUFSIZE - 1, 0);
        if (ret == 0)
        {
            close(sockfd);
            printf("foreiner closed the connection\n");
            break;
        }
        else if (ret < 0)
        {
            if (errno == EAGAIN)
            {
                reset_oneshot(epollfd, sockfd);
                printf("read later\n");
                break;
            }
        }
        else
        {
            printf("get content: %s\n", buf);
            sleep(1);
        }
    }
    printf("end thread receiving data on fd: %d\n", sockfd);
}



#define MAX_EVENT_NUMBER 1024
int main(int argc, char const *argv[])
{
    if (argc <= 2)
    {
        printf("usage: %s ip_address port number\n", basename(argv[0]));
    }
    const char *ip = argv[1];
    int port = atoi(argv[2]);

    sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    inet_pton(AF_INET, ip, &address.sin_addr);

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    int ret = bind(listenfd, (sockaddr *)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(listenfd, 5);
    assert(ret != -1);

    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    assert(epollfd != -1);

    // add listen fd
    addfd(epollfd, listenfd, false);

    while (true)
    {
        int ret = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if (ret < 0)
        {
            printf("epoll failure.\n");
            break;
        }
        for (int i = 0; i < ret; ++i)
        {
            int sockfd = events[i].data.fd;
            if (sockfd == listenfd)
            {
                sockaddr_in client_addr;
                socklen_t client_addrlen = sizeof(client_addr);
                int connfd = accept(sockfd, (sockaddr *)&client_addr, &client_addrlen);
                addfd(epollfd, connfd, true);
            }
            else if (events[i].events & EPOLLIN)
            {
                pthread_t thread;
                fds fds_for_new_worker;
                fds_for_new_worker.epollfd = epollfd;
                fds_for_new_worker.sockfd = sockfd;
                pthread_create(&thread, NULL, worker, (void *)&fds_for_new_worker);
            }
            else
            {
                printf("something else happend \n");
            }
        }
    }
    close(listenfd);
    return 0;
}
