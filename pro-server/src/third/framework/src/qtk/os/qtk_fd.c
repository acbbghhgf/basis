#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "qtk_fd.h"
#include "wtk/core/wtk_type.h"

int qtk_fd_read(int fd, char *data, int len, int *readed) {
    int ret = 0, index = 0, nLeft = len, count;
    int step;

    if (!data) {
        return -1;
    }
    while (nLeft > 0) {
        step = min(4096, nLeft);
        count = read(fd, &data[index], step);
        if (count == 0) break;
        if (count < step) {
            if ((count < 0) && (errno != EINTR )) {
                ret = -1;
                break;
            }
        }
        index += count;
        nLeft -= count;
    }
    if (readed) {
        *readed = index;
    }
    return ret;
}


int qtk_fd_write(int fd, char *data, int len, int *writed) {
    int ret = 0, index = 0, nLeft = len;
    int step;

    if (!data) {
        return -1;
    }
    while (nLeft > 0) {
        step = min(4096, nLeft);
        ret = write(fd, &data[index], step);
        if (ret == 0) break;
        if (ret < step) {
            if ((ret < 0) && (errno != EINTR)) {
                ret = -1;
                break;
            }
        }
        index += ret;
        nLeft -= ret;
    }
    if (writed) {
        *writed = index;
    }
    return ret;
}
