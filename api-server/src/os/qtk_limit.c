#define _GNU_SOURCE
#include "qtk_limit.h"
#include "wtk/core/wtk_type.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>

int qtk_limit_set_maxfiles(int max_files)
{
    struct rlimit l;
    if (max_files > 0) {
        l.rlim_cur = max_files;
        l.rlim_max = max_files;
        int ret = setrlimit(RLIMIT_NOFILE, &l);
        if (ret) {
            perror(__FUNCTION__);
            wtk_debug("set max = %d failed, ", (int)l.rlim_max);
            getrlimit(RLIMIT_NOFILE, &l);
            printf("cur max is %d. perhaps you should use sudo\n", (int)l.rlim_max);
            return -1;
        }
    }
    return 0;
}

int qtk_limit_set_pipe_sz(int sz, int fd)
{
    int ret = fcntl(fd, F_SETPIPE_SZ, sz);
    if (-1 == ret) { perror(__FUNCTION__); }
    return ret;
}
