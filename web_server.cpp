
#include "http_conn.h"
#include "threadpool.h"

#include <signal.h>
#include <cstring>
#include <cassert>
#include <cstdio>
#include <unistd.h>
#include <cstdlib>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <cerrno>
#include <cstdio>

extern int add_fd(int epoll_fd, int fd, bool one_shot);
extern int remove_fd(int epoll_fd, int fd);

#define MAX_FD 65536
#define MAX_EVENT_NUMBER 10000

void addsig(int sig, void(handler)(int), bool restart = true)
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    if (restart)
    {
        sa.sa_flags |= SA_RESTART;
    }
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, nullptr) != -1);
}

void show_error(int conn_fd, const char *info)
{
    printf("%s", info);
    send(conn_fd, info, strlen(info), 0);
    close(conn_fd);
}

int main(int argc, char const *argv[])
{
    if (argc <= 2)
    {
        printf("usage: %s ip_address port_numer\n", basename(argv[0]));
        return 1;
    }
    const char *ip = argv[1];
    int port = atoi(argv[2]);

    addsig(SIGPIPE, SIG_IGN);

    threadpool<http_conn> *pool = nullptr;
    try
    {
        pool = new threadpool<http_conn>;
    }
    catch (...)
    {
        return 1;
    }

    http_conn *users = new http_conn[MAX_FD];
    assert(users);

    int user_count = 0;
    int listen_fd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listen_fd >= 0);
    struct linger tmp = {1, 0};
    setsockopt(listen_fd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));

    int ret = 0;
    sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    inet_pton(AF_INET, ip, &address.sin_addr);

    ret = bind(listen_fd, (sockaddr *)&address, sizeof(address));
    assert(ret >= 0);

    ret = listen(listen_fd, 5);
    assert(ret >= 0);

    epoll_event events[MAX_EVENT_NUMBER];

    int epoll_fd = epoll_create(5);
    assert(epoll_fd != -1);

    add_fd(epoll_fd, listen_fd, false);
    http_conn::m_epoll_fd = epoll_fd;

    while (true)
    {
        int number = epoll_wait(epoll_fd, events, MAX_EVENT_NUMBER, -1);
        if ((number < 0) && (errno != EAGAIN))
        {
            printf("epoll failure\n");
            break;
        }
        for (int i = 0; i < number; ++i)
        {
            int sockfd = events[i].data.fd;
            if (sockfd == listen_fd)
            {
                sockaddr_in client_addr;
                socklen_t client_addrlen = sizeof(client_addr);
                int conn_fd = accept(listen_fd, (sockaddr *)&client_addr, &client_addrlen);
                if (conn_fd < 0)
                {
                    printf("errno is: %d\n", errno);
                    continue;
                }
                if (http_conn::m_user_count >= MAX_FD)
                {
                    show_error(conn_fd, "Interbal server busy");
                    continue;
                }
                users[conn_fd].init(conn_fd, client_addr);
            }
            else if (events[i].events & EPOLLIN)
            {
                if (users[sockfd].read())
                {
                    pool->append(users + sockfd);
                }
                else
                {
                    users[sockfd].close_conn();
                }
            }
            else if (events[i].events & EPOLLOUT)
            {
                if (!users[sockfd].write())
                {
                    users[sockfd].close_conn();
                }
            }
            else if (events[i].events & (EPOLLHUP | EPOLLRDHUP | EPOLLERR))
            {
                users[sockfd].close_conn();
            }
        }
    }
    close(epoll_fd);
    close(listen_fd);
    delete[] users;
    delete pool;
    return 0;
}
