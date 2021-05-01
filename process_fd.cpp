#include <sys/socket.h>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <cstring>
#include <cassert>


static const int CONTROL_LEN = CMSG_LEN(sizeof(int));
void send_fd(int fd, int fd_to_send)
{
    iovec iov[1];
    msghdr msg;
    char buf[10];

    iov[0].iov_base = buf;
    iov[0].iov_len = 1;
    msg.msg_name = nullptr;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    cmsghdr cm;
    cm.cmsg_len = CONTROL_LEN;
    cm.cmsg_level = SOL_SOCKET;
    cm.cmsg_type = SCM_RIGHTS;
    *(int *) CMSG_DATA(&cm) = fd_to_send;
    msg.msg_control = &cm;
    msg.msg_controllen = CONTROL_LEN;

    sendmsg(fd, &msg, 0);
}

int recv_fd(int fd)
{
    iovec iov[1];
    msghdr msg;
    char buf[0];

    iov[0].iov_base = buf;
    iov[0].iov_len = 1;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    cmsghdr cm;
    msg.msg_control = &cm;
    msg.msg_controllen = CONTROL_LEN;

    recvmsg(fd, &msg, 0);
    int fd_to_send = *(int*) CMSG_DATA(&cm);
    return fd_to_send;
}

int main(int argc, char const *argv[])
{
    int pipefd[2];
    int fd_to_pass = 0;

    int ret = socketpair(PF_UNIX, SOCK_DGRAM, 0, pipefd);
    assert(ret != -1);

    pid_t pid = fork();
    assert(pid >= 0);

    if(pid == 0)
    {
        close(pipefd[0]);
        fd_to_pass = open("README.md", O_RDWR, 0666);
        send_fd(pipefd[1], (fd_to_pass > 0)?fd_to_pass:0);
        close(fd_to_pass);
        exit(0);
    }

    close(pipefd[1]);
    fd_to_pass = recv_fd(pipefd[0]);
    char buf[BUFSIZ];
    memset(buf, '\0', BUFSIZ);
    read(fd_to_pass, buf, BUFSIZ);
    printf("I got fd %d and data %s\n", fd_to_pass, buf);
    close(fd_to_pass);
    return 0;
}
