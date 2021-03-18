#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cassert>
#include <unistd.h>
#include <cerrno>
#include <netdb.h>
#include <fcntl.h>
int main(int argc, char const *argv[])
{
    if(argc <= 2)
    {
        printf("%usage: %s ip_address port_number\n", basename(argv[0]));
        return 1;
    }        
    // socket
    const char* ip = argv[1];
    int port = atoi(argv[2]);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);
    int ret = bind(sockfd, (sockaddr*) &addr, sizeof(addr));
    assert(ret != -1);
    ret = listen(sockfd, 5);
    assert(ret != -1);

    // accept
    sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int connfd = accept(sockfd, (sockaddr*)&client_addr, &client_addr_len);
    if(connfd < 0)
    {
        printf("error: %s\n", gai_strerror(errno));
    }
    else
    {
        int piped[2];
        // 创建管道
        ret = pipe(piped);
        // connfd -> piped
        ret = splice(connfd, NULL, piped[1], NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
        assert(ret != -1);
        // piped -> connfd                                 
        ret = splice(piped[0], NULL, connfd, NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
        assert(ret != -1);
        close(connfd); 
    }
    close(sockfd);
    return 0;
}
