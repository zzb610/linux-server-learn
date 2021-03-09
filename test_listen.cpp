#include <sys/socket.h>
#include <netinet/in.h> // stockaddr_in
#include <cstdlib> // atoi
#include <cassert>
#include <cstring> // bzore
#include <arpa/inet.h>
#include <csignal>
#include <unistd.h> // sleep
#include <cstdio>
static bool stop = false;
static void handle_term(int sig)
{
    stop = true;
}
int main(int argc, char* argv[])
{
    signal(SIGTERM, handle_term);
    if(argc <= 3 )
    {
        printf("%s\n", basename(argv[0]));
        return 1;
    }
    // 创建socket
    char* ip = argv[1];
    int port = atoi(argv[2]);
    int backlog = atoi(argv[3]);
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);
    // 创建ipv4 socket 地址
    sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);
    // 命名socket
    int ret = bind(sock, (sockaddr*) &address, sizeof(address));
    assert(ret != -1);
    // 监听socket
    ret = listen(sock, backlog);
    assert(ret != -1);
    // 循环等待连接
    while(!stop)
    {
        sleep(1);
    }
    close(sock);
    return 0;
}
