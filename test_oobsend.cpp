#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <cassert>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    if(argc <= 2)
    {
        printf("%s", basename(argv[0]));
        return 1;
    }
    // create socket
    const char* ip = argv[1];
    int port = atoi(argv[2]);
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);
    // net ip
    sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);
    // bind socket
    if(connect(sockfd, (sockaddr*) &address, sizeof(address)) < 0)
    {
        printf("connection failed\n");
    }
    else
    {
        const char* oob_data = "abc";
        const char* normal_data = "123";
        send(sockfd, normal_data, strlen(normal_data), 0);
        send(sockfd, oob_data, strlen(oob_data), MSG_OOB);
        send(sockfd, normal_data, strlen(normal_data), 0);
    }
    close(sockfd);
    return 0;
}
