#ifndef _QTK_IRIS_QTK_STORAGE_H
#define _QTK_IRIS_QTK_STORAGE_H
#include "wtk/os/wtk_blockqueue.h"
#include "wtk/os/wtk_thread.h"
#include "qtk/core/qtk_abstruct_hashtable.h"
#include "wtk/core/wtk_strbuf.h"
#include "qtk/os/qtk_log.h"
#include "qtk_storage_cfg.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct qtk_storage qtk_storage_t;

struct qtk_storage {
    qtk_storage_cfg_t *cfg;
    qtk_log_t *log;
    qtk_abstruct_hashtable_t *wtopics;
    wtk_blockqueue_t delete_q;
    wtk_thread_t delete_thread;
    wtk_queue_t wtopic_q;
    wtk_queue_t act_topics;
    wtk_string_t *delete_dir;
    wtk_string_t *trash_dir;
    wtk_string_t *old_dir;
    wtk_string_t *act_dir;
    wtk_string_t *data_dir;
    wtk_string_t *data_fn;
    int data_fd;
    int data_rfd;
    int data_flen;            /* write position of data_fd */
    int act_max;
};


qtk_storage_t* qtk_storage_new(qtk_storage_cfg_t *cfg, qtk_log_t *log);
int qtk_storage_delete(qtk_storage_t *cache);
int qtk_storage_start(qtk_storage_t *cache);
int qtk_storage_stop(qtk_storage_t *cache);
int qtk_storage_save(qtk_storage_t *cache, wtk_string_t *topic, wtk_strbuf_t *body);
int qtk_storage_topic_delete(qtk_storage_t *cache, wtk_string_t *topic);

/*
  return: 1 read success but not eof
          0 stream eof
          -1 error
 */
int qtk_storage_read(qtk_storage_t *cache, wtk_string_t *topic,
                     int offset, wtk_strbuf_t *buf);
void* qtk_storage_read_idx(qtk_storage_t *cache, wtk_string_t *topic, int idx);
/*
  get idx line in idx_fn, read chunk posited by the idx line
 */
int qtk_storage_read_chunk(qtk_storage_t *cache, const char *idx_fn, int idx,
                           wtk_strbuf_t *buf);
/*
  return: 1 read success
          0 eof
          -1 error
 */
int qtk_storage_read_pack(qtk_storage_t *cache, void *idx, wtk_strbuf_t *buf);
int qtk_storage_release_idx(qtk_storage_t *cache, void *idx);
/*
  return: id(a random integer) if write register info success
          -1 if error
 */
int qtk_storage_reg(qtk_storage_t *cache, wtk_strbuf_t *data);
int qtk_storage_unreg(qtk_storage_t *cache, int id, const char *host);


#ifdef __cplusplus
}
#endif

#endif
