#ifndef WTK_OS_WTK_SOCKET_H_
#define WTK_OS_WTK_SOCKET_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_strbuf.h"
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
#include "wtk_sem.h"
#endif
#ifdef __cplusplus
extern "C" {
#endif
#ifdef WIN32
int wtk_socketpair(int socks[2], int make_overlapped);
#endif
typedef struct
{
	int addrlen;
	struct sockaddr * addr;
}wtk_addrinfo_t;

wtk_addrinfo_t* wtk_addrinfo_get(char* server,char *port);
int wtk_addrinfo_delete(wtk_addrinfo_t *i);
struct addrinfo* wtk_socket_get_addrinfo(char* server,char* port);
int wtk_socket_free_addr(struct addrinfo* info);
/*
 * @brief returned string must be freed when unused.
 */
char* wtk_get_host_ip(char *iface);
int  wtk_get_host_addr(char *iface,struct in_addr *addr);
int wtk_socket_get_addr(char *ip,int port,struct sockaddr_in *addr);
int wtk_socket_connect(struct sockaddr *ai_addr,socklen_t ai_addrlen,int *pfd);
int wtk_socket_connect2(char *ip,int port,int *pfd);
int wtk_socket_connect4(char *ip,char* port,int *pfd);

/**
 * @timeout: ms;
 */
void wtk_socket_set_timeout(int fd,int timeout);
/**
 * @timeout: ms;
 */
int wtk_socket_connect3(struct sockaddr *ai_addr,socklen_t ai_addrlen,int *pfd,int timeout);
int wtk_socket_close_fd(int fd);
int wtk_socket_get_port(int fd,int *port);
void wtk_socket_print(int fd);
int wtk_socket_set_reuse(int fd);

/**
 * @brief crate udp socket and bind, used for send and recv msg;
 */
int wtk_socket_create_udp_fd(int port);
int wtk_socket_create_tcp_listen_fd(int port);
int wtk_socket_readable(int fd);
#ifdef WIN32
#else
int wtk_socket_sendto(int fd,const char *data,int bytes,const struct sockaddr* addr,socklen_t len);
int wtk_socket_sendto_host(int fd,const char *data,int bytes,char *host,int port);
int wtk_socket_recvfrom(int fd,char *buf,int len,struct sockaddr *addr,socklen_t *addrlen);
wtk_addrinfo_t* wtk_addrinfo_get2(char *server,char *port);
wtk_addrinfo_t* wtk_addrinfo_get3(char *server,char *port,int timeout);

int wtk_socket_send_strmsg(int fd,char *data,int bytes);
int wtk_socket_read_strmsg(int fd,wtk_strbuf_t *buf);
#endif
#ifdef __cplusplus
};
#endif
#endif
