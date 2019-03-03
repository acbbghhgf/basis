#include "qtk_event_cfg.h"


int qtk_event_cfg_init(qtk_event_cfg_t *cfg) {
    cfg->use_epoll = 0;
    cfg->use_timer = 1; // use timer default
    qtk_epoll_cfg_init(&cfg->poll.epoll);
    qtk_event_timer_cfg_init(&cfg->timer);
    return 0;
}


int qtk_event_cfg_clean(qtk_event_cfg_t *cfg) {
    int ret;

    if (cfg->use_epoll) {
        ret = qtk_epoll_cfg_clean(&cfg->poll.epoll);
        if (ret) goto end;
    }
    if (cfg->use_timer) {
        ret = qtk_event_timer_cfg_clean(&cfg->timer);
        if (ret) goto end;
    }
    ret = 0;
end:
    return 0;
}


int qtk_event_cfg_update(qtk_event_cfg_t *cfg) {
    int ret;

    if (cfg->use_epoll) {
        ret = qtk_epoll_cfg_update(&cfg->poll.epoll);
        if (ret) goto end;
    }
    if (cfg->use_timer) {
        ret = qtk_event_timer_cfg_update(&cfg->timer);
        if (ret) goto end;
    }
    ret = 0;
end:
    return 0;
}


int qtk_event_cfg_update_local(qtk_event_cfg_t *cfg, wtk_local_cfg_t *main) {
    wtk_local_cfg_t *lc = main;
    wtk_string_t *v;
    int ret;

    wtk_local_cfg_update_cfg_b(lc, cfg, use_epoll, v);
    wtk_local_cfg_update_cfg_b(lc, cfg, use_timer, v);
    lc = wtk_local_cfg_find_lc_s(main, "epoll");
    if (lc) {
        cfg->use_epoll = 1;
        qtk_epoll_cfg_init(&(cfg->poll.epoll));
        ret = qtk_epoll_cfg_update_local(&(cfg->poll.epoll), lc);
        if (ret) goto end;
    }
    lc = wtk_local_cfg_find_lc_s(main, "timer");
    if (lc) {
        qtk_event_timer_cfg_init(&(cfg->timer));
        ret = qtk_event_timer_cfg_update_local(&(cfg->timer), lc);
        if (ret) goto end;
    }
    ret = 0;

end:
    return 0;
}


void qtk_event_cfg_print(qtk_event_cfg_t *cfg) {
    printf("--------------EVENT------------\r\n");
    print_cfg_i(cfg, use_epoll);
    if (cfg->use_epoll) {
        qtk_epoll_cfg_print(&cfg->poll.epoll);
    }
    print_cfg_i(cfg, use_timer);
    if (cfg->use_timer) {
        qtk_event_timer_cfg_print(&cfg->timer);
    }
}
