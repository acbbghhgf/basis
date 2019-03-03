#ifndef QTK_IRIS_QTK_FETCH_H
#define QTK_IRIS_QTK_FETCH_H
#include "wtk/os/wtk_thread.h"
#include "qtk/sframe/qtk_sframe.h"
#include "wtk/os/wtk_blockqueue.h"

#ifdef __cplusplus
extern "C" {
#endif

struct qtk_director;

typedef struct qtk_fetch qtk_fetch_t;

struct qtk_fetch {
    const char *root_dir;
    wtk_thread_t thread;
    wtk_blockqueue_t req_q;
    struct qtk_director* director;
    unsigned run: 1;
};


qtk_fetch_t* qtk_fetch_new(struct qtk_director *director);
int qtk_fetch_delete(qtk_fetch_t *fetch);
int qtk_fetch_start(qtk_fetch_t *fetch);
int qtk_fetch_stop(qtk_fetch_t *fetch);
int qtk_fetch_push(qtk_fetch_t *fetch, qtk_sframe_msg_t *msg);

#ifdef __cplusplus
}
#endif

#endif
