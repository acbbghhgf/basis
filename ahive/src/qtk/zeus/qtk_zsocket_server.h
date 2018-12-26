#ifndef QTK_ZEUS_QTK_ZSOCKET_SERVER_H
#define QTK_ZEUS_QTK_ZSOCKET_SERVER_H
#include "wtk/os/wtk_pipequeue.h"
#include "qtk/event/qtk_epoll.h"

#ifdef __cplusplus
#extern "C" {
#endif


typedef struct qtk_zsocket_server qtk_zsocket_server_t;


struct qtk_zsocket;
struct qtk_deque;

struct qtk_zsocket_server {
    wtk_pipequeue_t cmd_q;
    qtk_event_t cmd_ev;
    qtk_event_module_t *em;
    int max_fd;
    struct qtk_zsocket *slot;
    int alloc_id;
};

qtk_zsocket_server_t* qtk_zsocket_server_new(int max_fd);
int qtk_zsocket_server_init(qtk_zsocket_server_t* ss);
int qtk_zsocket_server_delete(qtk_zsocket_server_t* ss);
int qtk_zsocket_server_process(qtk_zsocket_server_t* ss);

int qtk_ss_cmd_open(qtk_zsocket_server_t *ss, uint32_t src,
                    const char *addr, size_t sz, int mode);
int qtk_ss_cmd_listen(qtk_zsocket_server_t *ss, uint32_t src,
                      const char *addr, size_t sz, int backlog);
void qtk_ss_cmd_close(qtk_zsocket_server_t *ss, uint32_t src, int id);
void qtk_ss_cmd_send(qtk_zsocket_server_t *ss, uint32_t src, int id, const char *msg, size_t sz);
void qtk_ss_cmd_send_buffer(qtk_zsocket_server_t *ss, uint32_t src, int id, struct qtk_deque *dq);

#ifdef __cplusplus
}
#endif

#endif
