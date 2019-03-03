#ifndef QTK_MAIN_PROXY_H_
#define QTK_MAIN_PROXY_H_

#include "qtk_proxy_cfg.h"
#include "qtk_proxy_worker.h"

typedef struct qtk_proxy qtk_proxy_t;

struct qtk_proxy {
    qtk_proxy_cfg_t *cfg;
    void *gate;
    qtk_proxy_worker_t *worker;
    unsigned run : 1;
};

#endif
