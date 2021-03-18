#include <cstring>
#include <cstdio>
#include <cassert>
#include <sys/socket.h>
#include <cstdlib>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cerrno>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/uio.h>

#define BUFFER_SIZE 1024
const char* status_line[2] = {"200 OK", "500 Internal server error"};
int main(int argc, char const *argv[])
{
    if (argc <= 3)
    {
        printf("usage: %s ip_address port_number file_name", basename(argv[0]));
        return 1;
    }
    // create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);
    // socket addr
    const char* ip = argv[1];
    int port = atoi(argv[2]);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);
    // bind socket
    int ret = bind(sockfd, (sockaddr*) &addr, sizeof(addr));
    assert(ret != -1);
    // listen socket
    ret = listen(sockfd, 5);
    assert(ret != -1);
    // accept
    sockaddr_in client;
    socklen_t client_addr_len = sizeof(client);
    int connfd = accept(sockfd, (sockaddr*) &client, &client_addr_len);

    if(connfd < 0)
    {
        printf("error is %s", gai_strerror(errno));
    }
    else
    {
        printf("connected!!");
        char header_buf[BUFFER_SIZE];
        memset(header_buf, '\0', BUFFER_SIZE);
        char* file_buf;
        struct stat file_stat;
        // 记录目标文件是否为有效文件
        bool valid = true;
        // 缓冲区header_buf 目前已经使用了多少字节的空间
        int len = 0;
        const char* file_name = argv[3];
        if(stat(file_name, &file_stat) < 0) // 目标文件不存在
        {
            valid = false; 
        }
        else
        {
            if(S_ISDIR(file_stat.st_mode)) // 请求的是一个目录
            {
                valid = false;
            }
            else if(file_stat.st_mode & S_IROTH) // 有读权限
            {
                int fd = open(file_name, O_RDONLY);
                file_buf = new char(file_stat.st_size + 1);
                memset(file_buf, '\0', file_stat.st_size + 1);
                if(read(fd, file_buf, file_stat.st_size) < 0)
                {
                    valid = false;
                }
                else
                {
                    valid = true;
                }
            }
            // 文件有效进行应答
            if (valid)
            {
                ret = snprintf(header_buf, BUFFER_SIZE-1, "%s %s\r\n", "HTTP/1.1", status_line[0]);
                len += ret;
                ret = snprintf(header_buf + len, BUFFER_SIZE - 1 - len, "Content-Length: %d\r\n", file_stat.st_size);
                len += ret;
                ret = snprintf(header_buf + len, BUFFER_SIZE -1 -len, "%s", "\r\n");

                iovec iv[2];
                iv[0].iov_base = header_buf;
                iv[0].iov_len = strlen(header_buf);
                iv[1].iov_base = file_buf;
                iv[1].iov_len = file_stat.st_size;
                ret = writev(connfd, iv, 2);
            }
            else
            {
                ret = snprintf(header_buf, BUFFER_SIZE-1, "%s %s\r\n", "HTTP/1.1", status_line[1]);
                len += ret;
                ret = snprintf(header_buf+len, BUFFER_SIZE-1-len, "%s", "\r\n");
                send(connfd, header_buf, strlen(header_buf), 0);
            }
            close(connfd);
            delete[] file_buf;
        }


    }
    close(sockfd);
    return 0;
}
