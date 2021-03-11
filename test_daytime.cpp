#include <cassert>
#include <netdb.h>
#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <cerrno>
int main(int argc, char const *argv[])
{
    // addrinfo
    addrinfo hints;
    addrinfo* res;
    bzero(&hints, sizeof(hints));
    int ret = getaddrinfo("192.168.60.133", "daytime", &hints, &res);

    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    assert(sock >= 0);
    // // connect
    if (connect(sock, res->ai_addr, INET_ADDRSTRLEN) == -1)
    {
        printf("errno: %d\n， %s", errno, gai_strerror(errno));
    }
    else
    {
        char buf[128];
        ret = read(sock, buf, sizeof(buf));
        assert(ret > 0);
        buf[ret] = '\0';
        printf("the daytime is: %s\n", buf);
    }
    
    freeaddrinfo(res);
    close(sock);
    
    // // host info
    // const char* host = "ubuntu_client";
    // hostent* hostinfo = gethostbyname(host);
    // // const char* host = "192.168.31.39";
    // // hostent* hostinfo = gethostbyaddr(host, INET_ADDRSTRLEN, AF_INET);
    // assert(hostinfo);

    // // servive info
    // servent* servinfo = getservbyname("daytime","tcp");
    // assert(servinfo);
    // printf("daytime port is %d\n", ntohs(servinfo->s_port));

    // // socket address
    // sockaddr_in address;
    // address.sin_family = AF_INET;
    // address.sin_addr = *(in_addr*)*(hostinfo->h_addr_list);
    // address.sin_port = servinfo->s_port;
    // // connect
    // int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // int ret = connect(sockfd, (sockaddr*)&address, sizeof(address));
    // assert(ret != -1);
    // // read time
    // char buffer[128];
    // ret = read(sockfd, buffer, sizeof(buffer));
    // assert(ret > 0);
    // buffer[ret] = '\0';
    // printf("the day time is %s", buffer);
    // close(sockfd);

    return 0;
}
