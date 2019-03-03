#ifndef QTK_OS_QTK_FD_H
#define QTK_OS_QTK_FD_H

#ifdef __cplusplus
extern "C" {
}
#endif

int qtk_fd_read(int fd, char *data, int len, int *readed);
int qtk_fd_write(int fd, char *data, int len, int *writed);

#ifdef __cplusplus
#endif

#endif
