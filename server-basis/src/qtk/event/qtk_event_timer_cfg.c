#include "qtk_event_timer_cfg.h"


int qtk_event_timer_cfg_init(qtk_event_timer_cfg_t *cfg) {
    cfg->timer_size = 100;
    return 0;
}


int qtk_event_timer_cfg_clean(qtk_event_timer_cfg_t *cfg) {
    return 0;
}


int qtk_event_timer_cfg_update(qtk_event_timer_cfg_t *cfg) {
    return 0;
}


int qtk_event_timer_cfg_update_local(qtk_event_timer_cfg_t *cfg, wtk_local_cfg_t *main) {
    wtk_local_cfg_t *lc = main;
    wtk_string_t *v;

    wtk_local_cfg_update_cfg_i(lc, cfg, timer_size, v);

    return 0;
}


void qtk_event_timer_cfg_print(qtk_event_timer_cfg_t *cfg) {
    printf("--------------EVENT_TIMER------------\r\n");
    print_cfg_i(cfg, timer_size);
}
