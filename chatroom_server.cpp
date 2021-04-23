#define _GNU_SOURCE 1
#include <netinet/in.h>
#include <fcntl.h>

#define BUFFER_SIZE 64

struct client_data
{
    sockaddr_in address; // client socket addr
    char* write_buf; // write to client
    char buf[BUFFER_SIZE]; // read from client
};

int setnonblock(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cassert>
#include <poll.h>
#include <fcntl.h>
#include <cerrno>
#include <unistd.h>
#define USER_LIMIT 5
int main(int argc, char const *argv[])
{
    if(argc <= 2)
    {
        printf("usage: %s ip port", basename(argv[0]));
        return 1;
    }

    const char* ip = argv[1];
    int port = atoi(argv[2]);

    int ret = 0;
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    inet_pton(AF_INET, ip, &address.sin_addr);

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    ret = bind(listenfd, (sockaddr*)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(listenfd, 5);
    assert(ret != -1);

    client_data* users = new client_data[USER_LIMIT];
    pollfd fds[USER_LIMIT+1];

    int user_counter = 0;
    for(int i=0; i <= USER_LIMIT; ++i)
    {
        fds[i].fd = -1;
        fds[i].events = 0;
    } 
    fds[0].fd = listenfd;
    fds[0].events = POLLIN | POLLERR;
    fds[0].revents = 0;

    while(true)
    {
        ret = poll(fds, user_counter, -1);
        if(ret < 0 )
        {
            printf("poll failure.\n");
            break;
        }
        for(int i=0;i < user_counter;++i)
        {
            if((fds[i].fd == listenfd) && (fds[i].revents & POLLIN)) // add new connection
            {
                sockaddr_in client_addr;
                socklen_t client_addrlen = sizeof(client_addr);
                int connfd = accept(listenfd, (sockaddr*)&client_addr, &client_addrlen);
                if(connfd < 0)
                {
                    printf("errorn is %d\n", errno);
                    continue;
                }
                if (user_counter >= USER_LIMIT)
                {
                    const char* info = "too many user!\n";
                    printf("%s", info);
                    send(connfd, info, strlen(info), 0);
                    close(connfd);
                    continue;
                }
                ++user_counter;
                users[connfd].address = client_addr;
                setnonblock(connfd);
                fds[user_counter].fd = connfd;
                fds[user_counter].events = POLLIN | POLLRDHUP | POLLERR;
                fds[user_counter].revents = 0;
                printf("comes a new user, now have %d users\n", user_counter);
            }
            else if(fds[i].revents & POLLERR)
            {
                printf("get an error from  %d\n", fds[i].fd);
                char errors[100];
                memset(errors, '\0', 100);
                socklen_t length = sizeof(errors);
                if(getsockopt(fds[i].fd, SOL_SOCKET, SO_ERROR, &errors, &length) < 0)
                {
                    printf("get socket option failed\n");
                }
                continue;
            }
            else if(fds[i].revents & POLLRDHUP) // client close connection
            {
                users[fds[i].fd] = users[fds[user_counter].fd];
                close(fds[i].fd);
                fds[i] = fds[user_counter];
                --i;
                --user_counter;
            }
            else if(fds[i].revents & POLLIN) // read from user
            {
                int connfd = fds[i].fd;
                memset(users[connfd].buf, '\0', BUFFER_SIZE);
                ret = recv(connfd, users[connfd].buf, BUFFER_SIZE-1, 0);
                printf("get %d bytes of client data %s from %d\n", ret, users[connfd].buf, connfd);
                if (ret < 0)
                {
                    if(errno != EAGAIN)
                    {
                        close(connfd);
                        users[fds[i].fd] = users[fds[user_counter].fd];
                        fds[i] = fds[user_counter];
                        --i;
                        --user_counter;
                    }
                }
                else if(ret == 0)
                {

                }
                else
                {   
                    // received data. write to other sockets
                    for(int j=1;i <= user_counter;++j)
                    {
                        if(fds[j].events == connfd)
                        {
                            continue;
                        }
                        fds[j].events |= ~POLLIN;
                        fds[j].events |= POLLOUT;
                    }
                }
            }
            else if(fds[i].revents & POLLOUT)
            {
                int connfd = fds[i].fd;
                if(! users[connfd].write_buf)
                {
                    continue;
                }
                ret = send(connfd, users[connfd].write_buf, strlen(users[connfd].write_buf), 0);
                users[connfd].write_buf = NULL;
                fds[i].events |= ~POLLOUT;
                fds[i].events |= POLLIN;
            }
        }
    }
    delete[] users;
    close(listenfd);
    return 0;
}


