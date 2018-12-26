#ifndef WTK_OS_WTK_FD_H_
#define WTK_OS_WTK_FD_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_stack.h"
#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#include <io.h>
#include <ws2tcpip.h>
#else
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif
#ifdef __cplusplus
extern "C" {
#endif
typedef enum
{
	WTK_OK=0,
	WTK_AGAIN,
	WTK_EOF,
	WTK_ERR
}wtk_fd_state_t;

int fd_read(int fd, char* data, int len, int* readed);
int fd_write(int fd, const char* data, int len, int* writed);
int wtk_fd_read(int fd,char* buf,int len);
int wtk_fd_write(int fd,char* buf,int len);
int wtk_fd_write_string(int fd,char* buf,int len);
wtk_string_t* wtk_fd_read_string(int fd);
int wtk_fd_write_stack(int fd,wtk_stack_t *s);
wtk_fd_state_t wtk_fd_recv(int fd, char* buf, int len, int* readed);
wtk_fd_state_t wtk_fd_recv2(int fd, char* buf, int len);
wtk_fd_state_t wtk_fd_recv_blk(int fd, char* buf, int len, int* readed);
wtk_fd_state_t wtk_fd_send(int fd, char* buf, int len, int* writed);
wtk_fd_state_t wtk_fd_flush_send_stack(int fd,wtk_stack_t *s);
int wtk_fd_set_nonblock(int fd);
#ifdef WIN32
#else
int wtk_fd_set_block(int fd);
int wtk_fd_is_block(int fd);
#endif
int wtk_fd_set_tcp_client_opt(int fd);
wtk_fd_state_t wtk_fd_write_nonblock(int fd,const char *data,int bytes,int *writed);
wtk_fd_state_t wtk_fd_read_nonblock(int fd,char *data,int bytes,int *readed);
#ifdef __cplusplus
};
#endif
#endif
