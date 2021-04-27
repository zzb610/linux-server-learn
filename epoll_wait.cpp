#include <ctime>
#include <cstdio>
#include <cerrno>
#include <sys/epoll.h>
#define TIMEOUT 5000

const int epollfd = 0;
#define MAX_EVENT_NUMBER 1024

int main(int argc, char const *argv[])
{
    int timeout = TIMEOUT;
    time_t start = time(nullptr);
    time_t end = time(nullptr);
    epoll_event events[MAX_EVENT_NUMBER];
    while(true)
    {
        printf("the timeout is now %d mil-seconds\n", timeout);
        start = time(nullptr);
        int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, timeout);
        if((number < 0) && (errno != EINTR))
        {
            printf("epoll failure.\n");
            break;
        }

        if(number == 0)
        {
            timeout = TIMEOUT;
            continue;
        }
        end = time(nullptr);
        timeout -= (end - start) * 1000;
        if(timeout <= 0)
        {
            timeout = TIMEOUT;
        }
        // handle connection
    }
    return 0;
}
