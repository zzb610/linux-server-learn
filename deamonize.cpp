#include <unistd.h>
#include <cstdlib>
#include <sys/stat.h>
#include <fcntl.h>
bool deamonize()
{
    // create child process and shutdown parent process
    pid_t pid = fork();
    if(pid < 0)
    {
        return false;
    }
    else if(pid > 0)
    {
        exit(0);
    }

    // set file permission mask
    umask(0);

    // create new session
    pid_t sid = setsid();
    if(sid < 0)
    {
        return false;
    }
    // chage working path
    if(chdir("/") < 0)
    {
        return false;
    }

    // close std in & out & error
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // redirect output to /dev/null
    open("/dev/null", O_RDONLY);
    open("/dev/null", O_RDWR);
    open("/dev/null", O_RDWR);
    return true;

}

#include <unistd.h>

// daemon();