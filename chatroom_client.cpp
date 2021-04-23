#define _GNU_SOURCE 1
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cassert>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 64
int main(int argc, char const *argv[])
{
    if(argc <= 2)
    {
        printf("usage: %s ip port\n", basename(argv[0]));
        return 1;
    }

    const char* ip = argv[1];
    int port = atoi(argv[2]);

    sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    inet_pton(AF_INET, ip, &server_addr.sin_addr);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sockfd>=0);

    if(connect(sockfd, (sockaddr*) &server_addr, sizeof(server_addr)) < 0)
    {
        printf("connection failed\n");
        close(sockfd);
        return 1;
    }

    pollfd fds[2];
    fds[0].fd = 0; // stdin input data
    fds[0].events = POLLIN;

    fds[1].fd = sockfd; // read data from sockfd
    fds[1].events = POLLIN | POLLRDHUP;
    fds[1].revents = 0;
    
    char read_buf[BUFFER_SIZE];
    int pipefd[2];
    //  data <-- |0|1| <-- data
    int ret = pipe(pipefd);
    assert(ret != -1);

    while(true)
    {
        ret = poll(fds, 2, -1);
        if(ret < 0)
        {
            printf("poll failure.\n");
            break;
        }

        // read from serser
        if(fds[1].revents & POLLRDHUP) // server close
        {
            printf("server close the connection.\n");
            break;
        }
        else if(fds[1].revents & POLLIN)
        {
            memset(read_buf,'\0', BUFFER_SIZE);
            recv(fds[1].fd, read_buf, BUFFER_SIZE-1, 0);
            printf("%s\n", read_buf);
        }
        // write to server
        if(fds[0].revents & POLLIN)
        {   // stdin -> pipefd[1]
            ret = splice(0, NULL, pipefd[1], NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
            // pipefd[0] -> sockfd
            ret = splice(pipefd[0], NULL, sockfd, NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
        }
    }
    close(sockfd);
    return 0;
}
