#include <unistd.h>
#include <cstdio>
static bool switch_to_user(uid_t user_id, gid_t gp_id)
{
    // make sure target user is not root
    if ((user_id == 0) || (gp_id == 0))
    {
        return false;
    }

    // legal user: root or target user
    uid_t uid = getuid();
    gid_t gid = getgid();
    printf("before switch gid: %d uid: %d\n", gid, uid);

    if (!((gid == 0) &&(uid == 0)) && !((gid == gp_id) && (uid == user_id)))
    {
        return false;
    }

    // if not root
    if(uid != 0)
    {
        return true;
    }

    // switch to user

    if ( (setgid(gp_id) < 0) || (setuid(user_id) < 0))
    {
     
        return false;
    }
    uid = getuid();
    gid = getgid();
    printf("after switch gid: %d uid: %d\n", gid, uid);
    return true;
}


#include <cassert>
int main(int argc, char const *argv[])
{
    uid_t uid = getuid();
    gid_t gid = getgid();
    
    bool ret =  switch_to_user(1000, 1000);
    assert(ret);
  
    return 0;
}
