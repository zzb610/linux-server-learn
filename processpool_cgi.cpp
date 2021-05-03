#include <netinet/in.h>
#include <cstdlib>
#include <arpa/inet.h>
#include "processpool.h"

class cgi_conn
{
public:
    cgi_conn(){}
    ~cgi_conn(){}
    void init(int epoll_fd, int sockfd, const sockaddr_in& client_addr)
    {
        m_epoll_fd = epoll_fd;
        m_sockfd = sockfd;
        m_address = client_addr;
        memset(m_buf,'\0', BUFFER_SIZE);
        m_read_idx = 0;
    }
    void process()
    {
        int idx = 0;
        int ret = -1;
        while(true)
        {
            idx = m_read_idx;
            ret = recv(m_sockfd, m_buf+idx, BUFFER_SIZE-1-idx, 0);
            if(ret < 0)
            {
                if(errno != EAGAIN)
                {
                    removefd(m_epoll_fd, m_sockfd);
                }
                break;
            }
            else if(ret == 0)
            {
                removefd(m_epoll_fd, m_sockfd);
                break;
            }
            else
            {
                m_read_idx += ret;
                printf("user content is %s\n", m_buf);
                for(;idx < m_read_idx;++idx)
                {
                    if((idx >= 1) && (m_buf[idx-1] == '\r') && (m_buf[idx] == '\n'))
                    {
                        break;
                    }
                }
                if(idx == m_read_idx)
                {
                    continue;
                }
                m_buf[idx-1] = '\0';
                char* file_name = m_buf;
                if(access(file_name, F_OK) == -1)
                {
                    removefd(m_epoll_fd, m_sockfd);
                    break;
                }
                ret = fork();
                if(ret == -1)
                {
                    removefd(m_epoll_fd, m_sockfd);
                    break;
                }
                else if(ret > 0)
                {
                    removefd(m_epoll_fd, m_sockfd);
                    break;
                }
                else
                {
                    close(STDIN_FILENO);
                    dup(m_sockfd);
                    execl(m_buf, m_buf, 0);
                    exit(0);
                }
            }
        }
    }
private:
    static const int BUFFER_SIZE = 1024;
    static int m_epoll_fd;
    int m_sockfd;
    sockaddr_in m_address;
    char m_buf[BUFFER_SIZE];
    int m_read_idx; 
};

int cgi_conn::m_epoll_fd = -1;


int main(int argc, char const *argv[])
{
    if(argc <= 2)
    {
        printf("usage: %s ip_address port_number\n", basename(argv[0]));
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);

    sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    inet_pton(AF_INET, ip, &address.sin_addr);

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listen_fd >= 0);

    int ret = 0;

    ret = bind(listen_fd, (sockaddr*) &address, sizeof(address));
    assert(ret != -1);

    ret = listen(listen_fd, 5);
    assert(ret != -1);

    processpool<cgi_conn>* pool = processpool<cgi_conn>::create(listen_fd);
    if(pool)
    {
        pool->run();
        delete pool;
    }
    close(listen_fd);
    return 0;
}
