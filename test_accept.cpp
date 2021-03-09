#include <sys/socket.h>
#include <netinet/in.h> // stockaddr_in
#include <cstdlib> // atoi
#include <cassert>
#include <cstring> // bzore
#include <arpa/inet.h>
#include <csignal>
#include <unistd.h> // sleep
#include <cstdio>
#include <cerrno>
int main(int argc, char *argv[])
{
    if(argc <= 2)
    {
        printf("%s\n", basename(argv[0]));
        return 1;
    }
    // create a socket
    const char* ip = argv[1];
    int port = atoi(argv[2]);
   
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);
    // create a ipv4 address
    sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    // bind socket
    int ret = bind(sock, (sockaddr*) &address, sizeof(address));
    assert(ret != -1);
    
    // listen socket
    ret = listen(sock, 5);
    assert(ret != -1);
    // sleep for client (drop)
    sleep(20);
    // accept client
    sockaddr_in clinet;
    socklen_t client_addrlength = sizeof(clinet);
    // create new socket
    int connfd = accept(sock, (sockaddr *) &clinet, &client_addrlength);
    if (connfd < 0)
    {
        printf("error is : %d\n", errno);
    }
    else
    {
        char remote[INET_ADDRSTRLEN];
        const char* ip = inet_ntop(AF_INET, &clinet.sin_addr, remote, INET_ADDRSTRLEN);
        int port = ntohs(clinet.sin_port);
        printf("connected with ip %s and port %d \n", ip, port);
        close(connfd);
    }
    close(sock);
    return 0;
}
