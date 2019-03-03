#ifndef QTK_ZEUS_QTK_ZEUS_SERVER_H
#define QTK_ZEUS_QTK_ZEUS_SERVER_H
#include <stdint.h>
#include "wtk/os/wtk_thread.h"
#include "wtk/core/wtk_stack.h"
#include "qtk/zeus/qtk_zeus_cfg.h"
#include "qtk/os/qtk_log.h"


#ifdef __cplusplus
extern "C" {
#endif

struct qtk_zeus_mq;
struct qtk_zeus_timer;
struct qtk_zeus_modules;
struct qtk_zhandle;
struct qtk_zcontext;
struct qtk_zmesg_queue;
struct qtk_zmesg;

struct qtk_zcontext;
struct qtk_zsocket_server;
struct qtk_deque;


#define qtk_zserver_log(fmt, ...) qtk_log_log(qtk_zserver_get_log(), fmt, __VA_ARGS__)
#define qtk_zserver_log0(fmt) qtk_log_log0(qtk_zserver_get_log(), fmt)

typedef struct qtk_zserver qtk_zserver_t;


struct qtk_zserver {
    struct qtk_zeus_mq *mq;
    struct qtk_zeus_timer *timer;
    struct qtk_zeus_modules *modules;
    struct qtk_zhandle *handles;
    struct qtk_zsocket_server *socket_server;
    qtk_zeus_cfg_t *cfg;
    qtk_log_t *log;
    int nctx;
    wtk_thread_t *threads;
    wtk_thread_t timer_thread;
    wtk_thread_t socket_thread;
    int nthread;
    int nsleep;
    pthread_mutex_t msg_mutex;
    pthread_cond_t msg_cond;
    uint32_t monitor_exit;
    unsigned quit:1;
};


qtk_zserver_t* qtk_zserver_new(qtk_zeus_cfg_t *cfg, qtk_log_t *log);
int qtk_zserver_start(qtk_zserver_t *z);
int qtk_zserver_stop(qtk_zserver_t *z);
int qtk_zserver_join(qtk_zserver_t *z);
int qtk_zserver_delete(qtk_zserver_t *z);
qtk_log_t* qtk_zserver_get_log();
struct qtk_zmesg_queue* qtk_zserver_dispatch(qtk_zserver_t *z, struct qtk_zmesg_queue* q);
struct qtk_zcontext* qtk_zcontext_new(qtk_zserver_t *z, const char *name, const char *param);
uint32_t qtk_zcontext_handle(struct qtk_zcontext*);
uint32_t* qtk_zcontext_handlep(struct qtk_zcontext*);
const char* qtk_zcontext_name(struct qtk_zcontext *);
void qtk_zcontext_set_name(struct qtk_zcontext *ctx, const char *name);
void qtk_zcontext_grab(struct qtk_zcontext *ctx);
struct qtk_zcontext* qtk_zcontext_release(struct qtk_zcontext *ctx);
void qtk_zcontext_dispatchall(struct qtk_zcontext *ctx);
int qtk_zcontext_push(uint32_t handle, struct qtk_zmesg *msg);
int qtk_zcontext_newsession(struct qtk_zcontext *ctx);
int qtk_zsocket_open(uint32_t src, const char *addr, size_t sz, int mode);
int qtk_zsocket_listen(uint32_t src, const char *addr, size_t sz, int backlog);
void qtk_zsocket_close(uint32_t src, int id);
void qtk_zsocket_send(uint32_t src, int id, const char *msg, size_t sz);
void qtk_zsocket_send_buffer(uint32_t src, int id, struct qtk_deque *dq);

qtk_zserver_t* qtk_zserver_g();

#ifdef __cplusplus
}
#endif

#endif
