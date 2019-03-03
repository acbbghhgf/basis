#include "qtk_entry_cfg.h"
#include "wtk/os/wtk_cpu.h"


int qtk_entry_cfg_init(qtk_entry_cfg_t *cfg)
{
    qtk_socket_server_cfg_init(&cfg->socket_server);
    cfg->cpus = wtk_get_cpus();
    cfg->worker = 5;   /* TODO update from cfg file */
    cfg->port = 8080;
    cfg->statis_time = 1000;
    cfg->slot_size = 8192;
    cfg->request_pool_size = 100;

    return 0;
}

int qtk_entry_cfg_clean(qtk_entry_cfg_t *cfg)
{
    qtk_socket_server_cfg_clean(&cfg->socket_server);

    return 0;
}

int qtk_entry_cfg_update(qtk_entry_cfg_t *cfg)
{
    qtk_socket_server_cfg_update(&cfg->socket_server);

    return 0;
}

int qtk_entry_cfg_update_local(qtk_entry_cfg_t *cfg,
                              wtk_local_cfg_t *main)
{
    wtk_string_t *v = NULL;
    wtk_local_cfg_t *lc = NULL;

    wtk_local_cfg_update_cfg_i(main, cfg, worker, v);
    wtk_local_cfg_update_cfg_i(main, cfg, port, v);
    wtk_local_cfg_update_cfg_i(main, cfg, statis_time, v);
    wtk_local_cfg_update_cfg_i(main, cfg, slot_size, v);
    wtk_local_cfg_update_cfg_i(main, cfg, request_pool_size, v);

    lc = wtk_local_cfg_find_lc_s(main, "socket_server");
    if (lc) {
        qtk_socket_server_cfg_update_local(&cfg->socket_server, lc);
    }

    return 0;
}


void qtk_entry_cfg_print(qtk_entry_cfg_t *cfg) {
    printf("--------------ENTRY------------\r\n");
    print_cfg_i(cfg, worker);
    print_cfg_i(cfg, port);
    print_cfg_i(cfg, statis_time);
}
