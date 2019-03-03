#ifndef _QTK_EVENT_QTK_EVENT_CFG_H
#define _QTK_EVENT_QTK_EVENT_CFG_H


#include "wtk/core/cfg/wtk_local_cfg.h"
#include "qtk/event/qtk_epoll_cfg.h"
#include "qtk/event/qtk_event_timer_cfg.h"

typedef struct qtk_event_cfg qtk_event_cfg_t;

struct qtk_event_cfg {
    union {
        qtk_epoll_cfg_t epoll;
    } poll;
    qtk_event_timer_cfg_t timer;
    unsigned use_epoll:1;
    unsigned use_timer:1;
};


int qtk_event_cfg_init(qtk_event_cfg_t *cfg);
int qtk_event_cfg_clean(qtk_event_cfg_t *cfg);
int qtk_event_cfg_update(qtk_event_cfg_t *cfg);
int qtk_event_cfg_update_local(qtk_event_cfg_t *cfg, wtk_local_cfg_t *lc);
void qtk_event_cfg_print(qtk_event_cfg_t *cfg);


#endif
