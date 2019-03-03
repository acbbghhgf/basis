#include "qtk_entry_cfg.h"
#include "wtk/os/wtk_cpu.h"
#include "qtk/websocket/qtk_ws_protobuf.h"
#include "third/libwebsockets/test-apps/test-server.h"


static struct lws_protocols protocols[] = {
    {
        "http-only",		/* name */
        callback_http,		/* callback */
        sizeof (struct per_session_data__http),	/* per_session_data_size */
        0,			/* max frame size / rx buffer */
    },
    {
        "default",		/* name */
        ws_callback_protobuf,		/* callback */
        sizeof (struct per_session_data__protobuf),	/* per_session_data_size */
        0,			/* max frame size / rx buffer */
    },
    { NULL, NULL, 0, 0 } /* terminator */
};



int qtk_entry_cfg_init(qtk_entry_cfg_t *cfg)
{
    qtk_socket_server_cfg_init(&cfg->socket_server);
    cfg->cpus = wtk_get_cpus();
    cfg->worker = 5;   /* TODO update from cfg file */
    cfg->port = 8080;
    cfg->statis_time = 1000;
    cfg->slot_size = 8192;
    cfg->request_pool_size = 100;
    cfg->net_log_fn = NULL;
    cfg->websocket.port = 17681;
    cfg->websocket.iface = NULL;
    cfg->websocket.protocols = protocols;
    cfg->websocket.extensions = NULL;
    cfg->websocket.gid = -1;
    cfg->websocket.uid = -1;
    cfg->websocket.max_http_header_pool = 100;
    cfg->websocket.options = 0;
    cfg->use_websocket = 0;

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
    if (cfg->use_websocket) {
        cfg->websocket.count_threads = cfg->worker;
    }

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
    wtk_local_cfg_update_cfg_str(main, cfg, net_log_fn, v);
    wtk_local_cfg_update_cfg_b(main, cfg, use_websocket, v);

    lc = wtk_local_cfg_find_lc_s(main, "socket_server");
    if (lc) {
        qtk_socket_server_cfg_update_local(&cfg->socket_server, lc);
    }

    if (cfg->use_websocket) {
        lc = wtk_local_cfg_find_lc_s(main, "websocket");
        if (lc) {
            struct lws_context_creation_info *ws = &cfg->websocket;
            wtk_local_cfg_update_cfg_i(lc, ws, port, v);
            wtk_local_cfg_update_cfg_i(lc, ws, gid, v);
            wtk_local_cfg_update_cfg_i(lc, ws, uid, v);
            wtk_local_cfg_update_cfg_i(lc, ws, max_http_header_pool, v);
        }
    }
    return 0;
}


void qtk_entry_cfg_print(qtk_entry_cfg_t *cfg) {
    printf("--------------ENTRY------------\r\n");
    print_cfg_i(cfg, worker);
    print_cfg_i(cfg, port);
    print_cfg_i(cfg, statis_time);
}
