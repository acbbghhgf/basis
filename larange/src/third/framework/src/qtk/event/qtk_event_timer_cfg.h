#ifndef _QTK_EVENT_QTK_EVENT_TIMER_CFG_H
#define _QTK_EVENT_QTK_EVENT_TIMER_CFG_H
#include "wtk/core/cfg/wtk_local_cfg.h"


typedef struct qtk_event_timer_cfg qtk_event_timer_cfg_t;


struct qtk_event_timer_cfg {
    int timer_size;
};


int qtk_event_timer_cfg_init(qtk_event_timer_cfg_t *cfg);
int qtk_event_timer_cfg_clean(qtk_event_timer_cfg_t *cfg);
int qtk_event_timer_cfg_update(qtk_event_timer_cfg_t *cfg);
int qtk_event_timer_cfg_update_local(qtk_event_timer_cfg_t *cfg, wtk_local_cfg_t *lc);
void qtk_event_timer_cfg_print(qtk_event_timer_cfg_t *cfg);


#endif
