#include <pthread.h>
#include <signal.h>
#include <cerrno>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>

#define handle_error_en(en, msg) \
    do                           \
    {                            \
        errno = en;              \
        perror(msg);             \
        exit(EXIT_FAILURE);      \
    } while (0)

static void* sig_thread(void* arg)
{
    sigset_t* set = (sigset_t*) arg;
    int s, sig;
    for(;;)
    {
        s = sigwait(set, &sig);
        if(s != 0)
        {
            handle_error_en(s, "sigwait");
        }
        printf("Signal handling thread got signal %d\n", sig);
    }
}

int main(int argc, char const *argv[])
{
    pthread_t thread;
    sigset_t set;

    int s;
    sigemptyset(&set);
    sigaddset(&set, SIGQUIT);
    sigaddset(&set, SIGUSR2);

    s = pthread_sigmask(SIG_BLOCK, &set, NULL);
    if (s != 0)
    {
        handle_error_en(s, "pthread_sig_mask");
    }
    s = pthread_create(&thread, NULL, &sig_thread, (void*) &set);
    if(s != 0)
    {
        handle_error_en(s, "pthread_create");
    }
    
    pause();
    return 0;
}