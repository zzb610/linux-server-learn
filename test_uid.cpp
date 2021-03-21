#include <cstdio>
#include <unistd.h>
int main(int argc, char const *argv[])
{
    uid_t uid = getuid();
    uid_t euid = geteuid();
    printf("userid: %d effective userid %d \n", uid, euid);
    return 0;
}
