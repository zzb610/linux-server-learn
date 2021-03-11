#include <cstdio>
#include <cassert>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <cstdlib>
#include <unistd.h>
const int kBufferSize = 512;
int main(int argc, char const *argv[])
{
    if(argc <=2)
    {
        printf("usage: %d ip_address port_number send_buffer_size", basename(argv[0]));
        return 1;
    } 
    // create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    int sendbuf = atoi(argv[3]);
    int len = sizeof(sendbuf);
    setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &sendbuf, sizeof(sendbuf));
    getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &sendbuf, (socklen_t*)&len);
    printf("the tcp send buffer size after is %d\n", sendbuf);
    assert(sock >= 0);
    // ipv4 socket address
    sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    const char* ip = argv[1];
    int port = atoi(argv[2]);
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);
    // connect
    if(connect(sock, (sockaddr *)&address, sizeof(address)) != -1)
    {
        char buffer[kBufferSize];
        memset(buffer, 'z', kBufferSize);
        send(sock, buffer,kBufferSize, 0);
    }
    close(sock);
    return 0;
}
