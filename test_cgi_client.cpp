#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <cassert>
#include <cstdlib>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cerrno>

const int kBufSize = 1024;
int main(int argc, char const *argv[])
{
    if(argc <= 2)
    {
        printf("usage: %s server_ip port_number", basename(argv[0]));
        return 1;
    }

    // create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);
    // net socket addr
    const char* ip = argv[1];
    int port = atoi(argv[2]);
    sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET,  ip, &server_addr.sin_addr);
    // connect
    if (connect(sockfd, (sockaddr*)&server_addr, sizeof(server_addr)))
    {
        char buffer[kBufSize];
        recv(sockfd, buffer, kBufSize, 0);
        printf("%s\n", buffer);
    }
    else
    {
        printf("fail to connect: errno: %d \n", errno);
    }
    return 0;
}
