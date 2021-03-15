#include <cstdio>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <sys/socket.h>
#include <unistd.h>
#include <cassert>
#include <cerrno>
#include <netdb.h>
int main(int argc, char const *argv[])
{
    if (argc <= 2)
    {
        printf("usage: %s ip_address, port_number", basename(argv[1]));
    }
    // ipv4 socket address
    sockaddr_in address;
    address.sin_family = AF_INET;
    const char* ip = argv[1];
    inet_pton(AF_INET, ip, &address.sin_addr);
    int port = atoi(argv[2]);
    address.sin_port = htons(port);
    // create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);
    // bind socket
    int ret = bind(sockfd, (sockaddr*)&address, sizeof(address));
    assert(ret != -1);
    // listen socket
    ret = listen(sockfd, 5);
    assert(ret != -1);
    // accept
    sockaddr_in client;
    socklen_t client_addrlength = sizeof(client);
    int connfd = accept(sockfd, (sockaddr*)&client, &client_addrlength);
    if(connfd < 0)
    {
        printf("errno: %d , %s\n", errno, gai_strerror(errno));
    }
    else
    {
        close(STDOUT_FILENO);
        dup(connfd);
        printf("abcd\n");
    }
    close(connfd);
    close(sockfd);
    return 0;
}
