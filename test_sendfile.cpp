#include <cstdio>
#include <cstring>
#include <cassert>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <cstdlib>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cerrno>
#include <netdb.h>
#include <sys/sendfile.h>
int main(int argc, char const *argv[])
{
    if (argc <= 3)
    {
        printf("usage: %s ip_address port_number filename", basename(argv[0]));
        return 1;
    }

    // 读取文件
    const char* file_name = argv[3];
    int filefd = open(file_name, O_RDONLY);
    assert(filefd > 0);
    struct stat stat_buf;
    fstat(filefd, &stat_buf);
    // socket
    const char* ip = argv[1];
    int port = atoi(argv[2]);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);
    int ret = bind(sockfd, (sockaddr *)&addr, sizeof(addr));
    assert(ret != -1);
    ret = listen(sockfd, 5);
    assert(ret != -1);

    // accept
    sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int connfd = accept(sockfd, (sockaddr*)&client_addr, &client_addr_len);
    if(connfd < 0)
    {
        printf("error: %s", gai_strerror(errno));
    }
    else
    {
        sendfile(filefd, connfd, NULL, stat_buf.st_size);
        close(connfd);
    }
    close(sockfd);
    return 0;
}
