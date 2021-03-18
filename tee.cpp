#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <cassert>
int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        printf("usage %s <file>\n", argv[0]);
        return 1;
    }
    int filefd = open(argv[1], O_CREAT | O_WRONLY | O_TRUNC, 0666);
    assert(filefd > 0);
    // 0 out 1 in
    int pipefd_stdout[2];
    int ret = pipe(pipefd_stdout);
    assert(ret != -1);

    int pipedfd_file[2];
    ret = pipe(pipedfd_file);
    assert(ret != -1);
    // stdin -> stdpipe
    ret = splice(STDIN_FILENO, NULL, pipefd_stdout[1], NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);    
    assert(ret != -1);

    // stdpipe -> file_pipe
    ret = splice(pipefd_stdout[0], NULL, pipedfd_file[1], NULL, 32768, SPLICE_F_NONBLOCK);
    assert(ret != -1);
    // file_pipe -> filefd
    ret = splice(pipedfd_file[0], NULL, filefd, NULL, 32768,SPLICE_F_MOVE | SPLICE_F_MORE);
    assert(ret != -1);
    // stdpipe -> stdout
    ret = splice(pipefd_stdout[0], NULL, STDOUT_FILENO, NULL, 32768, SPLICE_F_MORE |SPLICE_F_MOVE);
    assert(ret != -1);

    close(filefd);
    close(pipefd_stdout[0]);
    close(pipefd_stdout[1]);

    close(pipedfd_file[0]);
    close(pipedfd_file[1]);

    return 0;
}
