#include <netinet/in.h>
#include <cstdlib>
#include <arpa/inet.h>
#include <cstring>
#include <cassert>
#include <cerrno>
#include <cstdio>
int timeout_connect(const char* ip, int port, int time)
{
    int ret = 0;
    sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);

    timeval timeout;
    timeout.tv_sec = time;
    timeout.tv_usec = 0;
    socklen_t time_len = sizeof(timeout);

    ret = setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, time_len);
    assert(ret != -1);

    ret = connect(sockfd, (sockaddr*) &addr, sizeof(addr));
    if(ret == -1)
    {
        if(errno == EINPROGRESS)
        {
            printf("connecting timeout, process timeout logic\n");
            return -1;
        }
        printf("error occur when connectuing to server\n");
        return -1;
    }
    return sockfd;

}

int main(int argc, char const *argv[])
{
    if(argc <= 2)
    {
        printf("usage: %s ip port", basename(argv[0]));
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);
    
    int sockfd = timeout_connect(ip, port, 10);
    if(sockfd < 0)
    {
        return 1;
    }
    return 0;
}
