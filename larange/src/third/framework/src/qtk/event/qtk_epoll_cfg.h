#ifndef _QTK_EVENT_QTK_EPOLL_CFG_H
#define _QTK_EVENT_QTK_EPOLL_CFG_H


#include "wtk/core/cfg/wtk_local_cfg.h"

typedef struct qtk_epoll_cfg qtk_epoll_cfg_t;

struct qtk_epoll_cfg {
    int event_num;
    unsigned et:1;
};


int qtk_epoll_cfg_init(qtk_epoll_cfg_t *cfg);
int qtk_epoll_cfg_clean(qtk_epoll_cfg_t *cfg);
int qtk_epoll_cfg_update(qtk_epoll_cfg_t *cfg);
int qtk_epoll_cfg_update_local(qtk_epoll_cfg_t *cfg, wtk_local_cfg_t *lc);
void qtk_epoll_cfg_print(qtk_epoll_cfg_t *cfg);


#endif
