#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cassert>
#include <cerrno>
#include <unistd.h>
const int kBufferSize = 1024;
int main(int argc, char const *argv[])
{
    if(argc <= 2)
    {
        printf("usage: %s ipaddress port_number recv_buffer_size\n",basename(argv[0]));
        return 1;
    }
    // create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);
    int recv_buf = atoi(argv[3]);
    int len = sizeof(recv_buf);
    setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &recv_buf, sizeof(recv_buf));
    getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &recv_buf, (socklen_t*)&len);
    printf("the tcp reveive buffer size after setting is %d\n", recv_buf);
    // ipv4 socket address
    const char* ip = argv[1];
    int port = atoi(argv[2]);
    sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);
    // bind socket
    int ret = bind(sock, (sockaddr *)&address, sizeof(address));
    assert(ret != -1);
    // listen socket
    ret = listen(sock, 5);
    assert(ret != -1);
    // accept connection
    sockaddr_in client;
    socklen_t client_addrlength = sizeof(client);
    int connfd = accept(sock, (sockaddr *)&client, &client_addrlength);
    if (connfd < 0)
    {
        printf("errno is: %d\n", errno);
    }
    else
    {
        char buffer[kBufferSize];
        memset(buffer, '\0', kBufferSize);
        while(recv(connfd, buffer, kBufferSize-1, 0) > 0)
        {
            
        }
        close(connfd);
    }
    close(sock);
    return 0;
}
