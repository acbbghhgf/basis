#ifndef QTK_IRIS_CFG_H
#define QTK_IRIS_CFG_H
#include "wtk/core/cfg/wtk_local_cfg.h"
#include "qtk_director_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct qtk_iris_cfg qtk_iris_cfg_t;

struct qtk_iris_cfg {
    qtk_director_cfg_t director;
    char* listen;
    int port;
    int backlog;
    char* log_fn;
};


int qtk_iris_cfg_init(qtk_iris_cfg_t *cfg);
int qtk_iris_cfg_clean(qtk_iris_cfg_t *cfg);
int qtk_iris_cfg_update(qtk_iris_cfg_t *cfg);
int qtk_iris_cfg_update_local(qtk_iris_cfg_t *cfg, wtk_local_cfg_t *lc);


#ifdef __cplusplus
}
#endif


#endif
