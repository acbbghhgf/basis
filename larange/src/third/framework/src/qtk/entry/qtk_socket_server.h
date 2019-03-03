#ifndef QTK_ENTRY_QTK_SOCKET_SERVER_H
#define QTK_ENTRY_QTK_SOCKET_SERVER_H
#include "wtk/os/wtk_thread.h"
#include "qtk/event/qtk_event.h"
#include "qtk/event/qtk_epoll.h"
#include "wtk/os/wtk_pipequeue.h"
#include "qtk_socket_server_cfg.h"


#ifdef __cplusplus
extern "C" {
#endif


struct qtk_socket;
struct qtk_entry;
struct qtk_sframe_msg;


typedef struct qtk_socket_server qtk_socket_server_t;
typedef struct qtk_socket_listen qtk_socket_listen_t;


struct qtk_socket_listen {
    int id;
    qtk_event_t event;
    qtk_socket_server_t *server;
};


struct qtk_socket_server {
    wtk_pipequeue_t msg_q;
    qtk_socket_server_cfg_t *cfg;
    qtk_event_t msg_ev;
    qtk_event_module_t *em;
    int slot_cap;
    struct qtk_socket* slot;
    int alloc_id;
    struct qtk_entry *entry;
    qtk_socket_listen_t **listens;
    int nlis;
    int lis_cap;
    wtk_thread_t thread;
    unsigned run:1;
};

qtk_socket_listen_t* qtk_ss_get_listen(qtk_socket_server_t *ss, int id);
int qtk_ss_add_listen(qtk_socket_server_t *ss, int id);
int qtk_ss_remove_listen(qtk_socket_server_t *ss, int id);


qtk_socket_server_t *qtk_socket_server_new(qtk_socket_server_cfg_t *cfg, struct qtk_entry *entry);
int qtk_socket_server_init(qtk_socket_server_t *ss);
int qtk_socket_server_delete(qtk_socket_server_t *ss);
int qtk_socket_server_start(qtk_socket_server_t *ss);
int qtk_socket_server_stop(qtk_socket_server_t *ss);
void qtk_ss_add_socket(qtk_socket_server_t *ss, struct qtk_socket *socket);
int qtk_ss_push_msg(qtk_socket_server_t *ss, struct qtk_sframe_msg *msg);


#ifdef __cplusplus
}
#endif

#endif
