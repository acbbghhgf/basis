#ifndef QTK_SERVICE_QTK_AISRV_CFG_H
#define QTK_SERVICE_QTK_AISRV_CFG_H
#include "wtk/core/wtk_heap.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct qtk_aisrv_cfg qtk_aisrv_cfg_t;


struct qtk_aisrv_cfg {
    wtk_heap_t *heap;
    wtk_cfg_file_t *cfg_file;
    wtk_local_cfg_t *lc;
    int max_instance;
    int max_loop;
};


#ifdef __cplusplus
}
#endif
#endif
