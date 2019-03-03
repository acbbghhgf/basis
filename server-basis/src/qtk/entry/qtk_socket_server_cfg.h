#ifndef QTK_ENTRY_QTK_SOCKET_SERVER_CFG_H
#define QTK_ENTRY_QTK_SOCKET_SERVER_CFG_H
#include "qtk/event/qtk_event_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct qtk_socket_server_cfg qtk_socket_server_cfg_t;

struct qtk_socket_server_cfg {
    int slot_size;
    int send_buf;
    qtk_event_cfg_t event;
};


int qtk_socket_server_cfg_init(qtk_socket_server_cfg_t* cfg);
int qtk_socket_server_cfg_clean(qtk_socket_server_cfg_t* cfg);
int qtk_socket_server_cfg_update(qtk_socket_server_cfg_t* cfg);
int qtk_socket_server_cfg_update_local(qtk_socket_server_cfg_t* cfg, wtk_local_cfg_t *main);


#ifdef __cplusplus
}
#endif


#endif
