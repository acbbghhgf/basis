#ifndef WTK_NK_WTK_CONNECTION
#define WTK_NK_WTK_CONNECTION
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_queue.h"
#include "wtk/nk/poll/wtk_poll_evt.h"
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "wtk/os/wtk_fd.h"
#include "wtk/core/wtk_buf.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_connection wtk_connection_t;
struct wtk_nk;

struct wtk_connection
{
	wtk_queue_node_t hoard_n;
	struct sockaddr_in addr;
	char peer_name[sizeof("127.127.127.127")];
	unsigned short peer_port;
	int fd;
	wtk_poll_evt_t evt;
	struct wtk_nk *nk;
	void *parser;
	wtk_buf_t *wbuf;
};

wtk_connection_t* wtk_connection_new(	struct wtk_nk *nk);
int wtk_connection_delete(wtk_connection_t *c);
void wtk_connection_init(wtk_connection_t *c,int fd,int evt_type);
int wtk_connection_process(wtk_connection_t *c);
int wtk_connection_write(wtk_connection_t *c, char* buf, int len);
void wtk_connection_update_addr(wtk_connection_t *c);
void wtk_connection_print(wtk_connection_t *c);
void wtk_sockaddr_print(struct sockaddr_in *addr);
#ifdef __cplusplus
};
#endif
#endif
