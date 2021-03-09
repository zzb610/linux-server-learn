#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <cassert>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cerrno>
#include <unistd.h>

const int kBufSize = 1024;

// server
int main(int argc, char const *argv[])
{
    if (argc <=2 )
    {
        printf("%s\n", basename(argv[0]));
        return 1;
    }

    // creat socket
    const char* ip = argv[1];
    int port = atoi(argv[2]);
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);
    // net ip
    sockaddr_in address;
    address.sin_family = AF_INET;
    int ret = inet_pton(AF_INET, ip, &address.sin_addr);
    assert(ret == 1);
    address.sin_port = htons(port);
    // bind socket
    ret = bind(sock, (sockaddr*) &address, sizeof(address));
    assert(ret != -1);
    // listen socket
    ret = listen(sock, 5);
    assert(ret != -1);
    // accept client
    sockaddr_in clinet;
    socklen_t client_addrlength = sizeof(clinet);
    int connfd = accept(sock, (sockaddr*)&clinet, &client_addrlength);
    if(connfd < 0)
    {
        printf("errno is: %d", errno);
    }
    else
    {
        char buffer[kBufSize];

        memset(buffer, '\0', kBufSize);
        ret = recv(connfd, buffer, kBufSize-1, 0);
        printf("got %d bytes of normal data: %s\n", ret, buffer);

        memset(buffer,'\0', kBufSize);
        ret = recv(connfd, buffer, kBufSize-1, MSG_OOB);
        printf("got %d bytes of oob data: %s\n", ret, buffer);


        memset(buffer, '\0', kBufSize);
        ret = recv(connfd, buffer, kBufSize-1, 0);
        printf("got %d bytes of normal data: %s\n", ret, buffer);

        close(connfd);

    }
    close(sock);
    return 0;
}
