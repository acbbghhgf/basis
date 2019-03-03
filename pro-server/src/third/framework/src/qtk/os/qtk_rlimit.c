#define _GNU_SOURCE
#include <unistd.h>
#include <sys/resource.h>
#include "qtk_rlimit.h"
#include "wtk/core/wtk_type.h"


int qtk_rlimit_set_nofile(int max_files) {
    struct rlimit l;
    if (max_files > 0) {
        l.rlim_cur = max_files;
        l.rlim_max = max_files;
        int ret = setrlimit(RLIMIT_NOFILE, &l);
        if (ret) {
            perror(__FUNCTION__);
            wtk_debug("set max = %d failed, ", (int)l.rlim_max);
            getrlimit(RLIMIT_NOFILE, &l);
            wtk_debug("cur max is %d. perhaps you should use sudo\n", (int)l.rlim_max);
            return -1;
        }
    }
    return 0;
}
