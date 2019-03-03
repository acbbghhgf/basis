#ifndef WTK_NK_WTK_CONNECTION_H_
#define WTK_NK_WTK_CONNECTION_H_
#ifdef WIN32
#include "wtk/os/wtk_socket.h"
#else
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif
#include <errno.h>
#include "wtk/core/wtk_type.h"
#include "wtk_event.h"
#include "wtk/core/wtk_stack.h"
#include "wtk/core/wtk_queue.h"
#include "wtk/http/nk/listen/wtk_listen.h"
#include "wtk/os/wtk_fd.h"
#include "wtk/os/wtk_socket.h"
#ifdef __cplusplus
extern "C" {
#endif
#define wtk_connection_write_s(c,d) wtk_connection_write(c,d,sizeof(d)-1)

struct wtk_listen;
struct wtk_nk;
typedef struct wtk_connection wtk_connection_t;
typedef enum
{
	CONNECTION_EVENT_READ=0x00000001,
	CONNECTION_EVENT_WRITE=0x0000002,
	CONNECTION_EVENT_ACCEPT=0x0000004
}connection_type_t;

typedef void(*wtk_connection_close_notify_f)(void *ths);

typedef struct
{
	void *ths;
	wtk_connection_close_notify_f close;
}wtk_connection_listener_t;

struct wtk_connection
{
    wtk_queue_node_t net_node_q;
	struct sockaddr_in addr;
	wtk_event_t *event;
	int active_count;
	int fd;
#ifdef SAVE_SOCK_PORT
	char sock_port[sizeof("80808080")];
	wtk_string_t sock_port_text;
#endif
	char name[sizeof("127.127.127.127:80800000000000")];
	wtk_string_t remote_ip;
	int remote_port;
	wtk_string_t addr_text;
	struct wtk_listen *listen;
	struct wtk_nk *net;
	wtk_stack_t *wstack;
	wtk_parser_t *parser;
	void *app_data;
	//----------- listener 0000000000000
	wtk_connection_listener_t *evt_listener;
	unsigned want_close:1;
	unsigned busy:1;
    unsigned recv_delay:1;
    unsigned keep_alive:1;
    unsigned ready:1; //ready for non-blocking connect socket.
    unsigned valid:1;
};

wtk_connection_t* wtk_connection_new(struct wtk_nk *n);
int wtk_connection_bytes(wtk_connection_t *c);
int wtk_connection_delete(wtk_connection_t *c);
int wtk_connection_init(wtk_connection_t *c,int fd,struct wtk_listen *l,int t);
int wtk_connection_attach_fd(wtk_connection_t *c,int fd);
int wtk_connection_clean(wtk_connection_t *c);
int wtk_connection_reset(wtk_connection_t *c);
int wtk_connection_process(wtk_connection_t *t,wtk_event_t* event);
int wtk_connection_write(wtk_connection_t *c,char* buf,int len);
int wtk_connection_flush(wtk_connection_t *c);
int wtk_connection_write_stack(wtk_connection_t *c,wtk_stack_t *s);
int wtk_connection_try_close(wtk_connection_t *c);
int wtk_connection_connect(wtk_connection_t *c,struct sockaddr *ai_addr,socklen_t ai_addrlen);
int wtk_connection_connect2(wtk_connection_t *c,struct sockaddr *ai_addr,socklen_t ai_addrlen,int timeout);
void wtk_connection_print(wtk_connection_t *c);
void wtk_connection_set_delay(wtk_connection_t* c,int delay);
void wtk_connection_shutdown(wtk_connection_t *c);
int wtk_connection_get_sock_peer_name(wtk_connection_t *c,char *buf);
void wtk_connection_log_read_failed(wtk_connection_t *c);
int wtk_connection_process(wtk_connection_t *c, wtk_event_t* event);
#ifdef __cplusplus
};
#endif
#endif
