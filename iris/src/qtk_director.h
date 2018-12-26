#ifndef _QTK_CACHED_QTK_DIRECTOR_H
#define _QTK_CACHED_QTK_DIRECTOR_H
#include "qtk_director_cfg.h"
#include "wtk/os/wtk_thread.h"
#include "wtk/os/wtk_pipequeue.h"
#include "qtk/os/qtk_log.h"
#include "qtk_storage.h"
#include "qtk_fetch.h"


#ifdef __cplusplus
extern "C" {
#endif

struct qtk_sframe_method;
struct qtk_iris;

typedef struct qtk_director qtk_director_t;

struct qtk_director {
    qtk_director_cfg_t *cfg;
    wtk_thread_t thread;
    qtk_log_t *log;
    struct qtk_sframe_method *method;
    struct qtk_iris *iris;
    qtk_fetch_t *fetch;
    qtk_storage_t *cache;
    qtk_abstruct_hashtable_t *watchers;
    qtk_abstruct_hashtable_t *conn_info;
    unsigned run :1;
};

qtk_director_t* qtk_director_new(qtk_director_cfg_t *cfg,
                                 qtk_log_t *log);
int qtk_director_init(qtk_director_t *director);
int qtk_director_delete(qtk_director_t *director);
int qtk_director_start(qtk_director_t *director);
int qtk_director_stop(qtk_director_t *director);
int qtk_director_join(qtk_director_t *director);
void qtk_director_set_method(qtk_director_t *director, void *sframe);
void topic_parse_to_cache(const char *src, char *dst, int len);
void topic_parse_from_cache(const char *src, char *dst, int len);

#ifdef __cplusplus
}
#endif

#endif
