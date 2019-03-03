#ifndef QTK_ENTRY_QTK_SOCKET_H
#define QTK_ENTRY_QTK_SOCKET_H
#include <netdb.h>
#include "wtk/core/wtk_stack.h"
#include "qtk/event/qtk_event.h"
#include "qtk/entry/qtk_parser.h"

#ifdef __cplusplus
extern "C" {
#endif


struct qtk_entry;
struct qtk_socket_server;
typedef struct qtk_socket qtk_socket_t;


enum qtk_socket_state {
    SOCKET_INVALID = 0,
    SOCKET_RESERVE,
    SOCKET_LISTEN,
    SOCKET_CONNECTING,
    SOCKET_CONNECTED,
    SOCKET_CLOSED,
    SOCKET_WAIT_REC,
};


struct qtk_socket {
    int fd;
    int id;
    int pid;
    int state;
    qtk_event_t event;
    wtk_stack_t *send_stk;
    union {
        qtk_parser_t *parser;
        struct lws *wsi;
    } proto;
    wtk_string_t *remote_addr;
    wtk_string_t *local_addr;
    struct qtk_socket_server *server;
    int reconnect;
    unsigned stale:1;
    unsigned websocket:1;
};


int qtk_socket_init(qtk_socket_t *s, int fd,
                    struct qtk_socket_server *ss,
                    const char *proto);
int qtk_socket_init_ws(qtk_socket_t *s, struct qtk_entry *entry, struct lws *wsi);
void qtk_socket_init_event(qtk_socket_t *s);
int qtk_socket_reset(qtk_socket_t *s);
int qtk_socket_close(qtk_socket_t *s);
int qtk_socket_clean(qtk_socket_t *s);
int qtk_socket_connect(qtk_socket_t *s, struct sockaddr* saddr, int sz);
int qtk_socket_listen(qtk_socket_t *s, struct sockaddr *saddr, int sz, int backlog);
int qtk_socket_writable_bytes(qtk_socket_t *s);
int qtk_socket_send_stack(qtk_socket_t *s, wtk_stack_t *stk);
int qtk_socket_send(qtk_socket_t *s, const char *data, int len);
wtk_string_t *qtk_socket_ntop(struct sockaddr_in* addr);
int qtk_socket_aton(const char *addr, struct sockaddr_in* saddr);


#ifdef __cplusplus
    }
#endif


#endif
