#ifndef _QTK_ACCESS_ENTRY_QTK_ENTRY_CFG_H
#define _QTK_ACCESS_ENTRY_QTK_ENTRY_CFG_H
#include "wtk/core/cfg/wtk_local_cfg.h"
#include "qtk_socket_server_cfg.h"
#include "libwebsockets.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct qtk_entry_cfg qtk_entry_cfg_t;
struct qtk_entry_cfg {
    qtk_socket_server_cfg_t socket_server;
    int slot_size;
    int request_pool_size;
    int worker;
    int cpus;
    uint16_t port;
    int statis_time;
    char *net_log_fn;
    struct lws_context_creation_info websocket;
    unsigned use_websocket : 1;
};

int qtk_entry_cfg_init(qtk_entry_cfg_t *cfg);
int qtk_entry_cfg_clean(qtk_entry_cfg_t *cfg);
int qtk_entry_cfg_update(qtk_entry_cfg_t *cfg);
int qtk_entry_cfg_update_local(qtk_entry_cfg_t *cfg,
                              wtk_local_cfg_t *lc);
void qtk_entry_cfg_print(qtk_entry_cfg_t *cfg);

#ifdef __cplusplus
};
#endif
#endif
