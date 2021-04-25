#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <cstdio>

#define BUFFER_SIZE 1024

static int connfd;

void sig_urg(int sig)
{
    int save_errno = errno;
    char buffer[BUFFER_SIZE];
    memset(buffer, '\0', BUFFER_SIZE);
    int ret = recv(connfd, buffer, BUFFER_SIZE-1, MSG_OOB);
    printf("got %d bytes of oob data %s \n", ret, buffer);
    errno = save_errno;    
}

#include <signal.h>
#include <cassert>
void addsig(int sig, void (*sig_handler)(int))
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = sig_handler;

    sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

#include <cstdlib>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <cstdlib>
#include <unistd.h>
int main(int argc, char const *argv[])
{
    if(argc <= 2)
    {
        printf("usage: %s ip, port \n", basename(argv[0]));
        return 1;
    }

    const char* ip =argv[1];
    int port = atoi(argv[2]);

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    inet_pton(AF_INET, ip, &address.sin_addr);

    int ret = 0;
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    ret = bind(listenfd, (sockaddr*) &address, sizeof(address));
    assert(ret != -1);
    ret = listen(listenfd, 5);
    assert(ret != -1);

    sockaddr_in client_addr;
    socklen_t client_addrlen = sizeof(client_addr);
    
    connfd = accept(listenfd, (sockaddr*) &client_addr, &client_addrlen);
    if(connfd < 0)
    {
        printf("errno is : %d\n", errno);
    }
    else
    {
        addsig(SIGURG, sig_urg);
        // set siocket process owner or group
        fcntl(connfd, F_SETOWN, getpid());
        char buffer[BUFFER_SIZE];
        // get normal data
        while(true)
        {
            memset(buffer, '\0', BUFFER_SIZE);
            ret = recv(connfd, buffer, BUFFER_SIZE-1, 0);
            if(ret <=0)
            {
                break;
            }
            printf("got %d bytes of normal data: %s\n", ret, buffer);

        }
        close(connfd);
    }
    close(listenfd);
    return 0;
}
