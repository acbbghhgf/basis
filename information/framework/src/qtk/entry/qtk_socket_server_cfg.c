#include "qtk_socket_server_cfg.h"
#include "qtk/event/qtk_event_cfg.h"


int qtk_socket_server_cfg_init(qtk_socket_server_cfg_t* cfg) {
    cfg->slot_size = 10000;
    cfg->send_buf = 5 * 1024 * 1024;
    qtk_event_cfg_init(&cfg->event);
    return 0;
}


int qtk_socket_server_cfg_clean(qtk_socket_server_cfg_t* cfg) {
    qtk_event_cfg_clean(&cfg->event);
    return 0;
}


int qtk_socket_server_cfg_update(qtk_socket_server_cfg_t* cfg) {
    qtk_event_cfg_update(&cfg->event);
    return 0;
}


int qtk_socket_server_cfg_update_local(qtk_socket_server_cfg_t* cfg, wtk_local_cfg_t *main) {
    wtk_local_cfg_t *lc = main;
    wtk_string_t *v;

    wtk_local_cfg_update_cfg_i(lc, cfg, slot_size, v);
    wtk_local_cfg_update_cfg_i(lc, cfg, send_buf, v);

    lc = wtk_local_cfg_find_lc_s(main, "event");
    if (lc) {
        qtk_event_cfg_update_local(&cfg->event, lc);
    }

    return 0;
}
