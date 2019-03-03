#ifndef QTK_ENTRY_QTK_REQUEST_POOL_H
#define QTK_ENTRY_QTK_REQUEST_POOL_H
#include "wtk/os/wtk_pipequeue.h"
#include "qtk/event/qtk_event.h"
#include "qtk/sframe/qtk_sframe.h"

#ifdef __cplusplus
extern "C" {
#endif

struct qtk_request;
struct qtk_socket_server;

typedef struct qtk_socket_slot qtk_socket_slot_t;
typedef struct qtk_request_pool qtk_request_pool_t;


struct qtk_socket_slot {
    int sock_id;
    int state;
    struct qtk_request *req;
};


struct qtk_request_pool {
    wtk_pipequeue_t req_q;
    qtk_event_t req_ev;
    wtk_pipequeue_t cmd_q;
    qtk_event_t cmd_ev;
    qtk_socket_slot_t *slots;
    int id;
    int slot_cap;
    int nslot;
    int state;
    struct qtk_socket_server* server;
    qtk_sframe_connect_param_t *con_param;
    int req_cache;
    wtk_string_t *remote_addr;
};

int qtk_request_pool_init(qtk_request_pool_t *pool, int nslot, struct qtk_socket_server *ss);
int qtk_request_pool_clean(qtk_request_pool_t *pool);
int qtk_request_pool_response(qtk_request_pool_t *pool, qtk_sframe_msg_t *msg);
int qtk_request_pool_push(qtk_request_pool_t *pool, qtk_sframe_msg_t *msg);

#ifdef __cplusplus
}
#endif

#endif
