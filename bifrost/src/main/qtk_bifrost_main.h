#ifndef MAIN_QTK_BIFROST_MAIN_H_
#define MAIN_QTK_BIFROST_MAIN_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "os/qtk_thread.h"
#include "qtk/sframe/qtk_sframe.h"
#include "third/lua5.3.4/src/lua.h"
#include "base/qtk_strbuf.h"
#include "third/framework/src/wtk/os/wtk_blockqueue.h"

#define _get_file_processer(L)  lua_getglobal(L, "loadfilprocesser")

typedef struct qtk_upload_mode{  
    wtk_blockqueue_t *upload_q_read;
    wtk_blockqueue_t *upload_q_write;
    wtk_blockqueue_t *upload_q_send;
}qtk_upload_mode_t;

typedef struct qtk_bifrost qtk_bifrost_t;
struct qtk_bifrost {
    qtk_thread_t *workers;
    qtk_thread_t timer_thread;
    qtk_sframe_method_t *gate;
    qtk_upload_mode_t *up;
    unsigned run : 1;

};
typedef struct qtk_bifrost_upload_queue qtk_bifrost_upload_queue_t;
struct qtk_bifrost_upload_queue {
    int file;
    wtk_queue_node_t q_n;
    char filename[30];
    qtk_strbuf_t *msg_strbuf;
};

qtk_bifrost_t *qtk_bifrost_new(void *gate);
int qtk_bifrost_start(qtk_bifrost_t *b);
int qtk_bifrost_delete(qtk_bifrost_t *b);
int qtk_bifrost_stop(qtk_bifrost_t *b);
qtk_bifrost_t *qtk_bifrost_self();

int qtk_context_total();
void qtk_context_inc();
void qtk_context_dec();

#ifdef __cplusplus
};
#endif
#endif
