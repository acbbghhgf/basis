#include "qtk_epoll_cfg.h"


int qtk_epoll_cfg_init(qtk_epoll_cfg_t *cfg) {
    cfg->event_num = 0;
    cfg->et = 0;
    return 0;
}


int qtk_epoll_cfg_clean(qtk_epoll_cfg_t *cfg) {
    return 0;
}


int qtk_epoll_cfg_update(qtk_epoll_cfg_t *cfg) {
    return 0;
}


int qtk_epoll_cfg_update_local(qtk_epoll_cfg_t *cfg, wtk_local_cfg_t *main) {
    wtk_local_cfg_t *lc = main;
    wtk_string_t *v;

    wtk_local_cfg_update_cfg_i(lc, cfg, event_num, v);
    wtk_local_cfg_update_cfg_b(lc, cfg, et, v);

    return 0;
}


void qtk_epoll_cfg_print(qtk_epoll_cfg_t *cfg) {
    printf("--------------EPOLL------------\r\n");
    print_cfg_i(cfg, event_num);
    print_cfg_i(cfg, et);
}
