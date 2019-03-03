#ifndef WTK_CFG_WTK_CFG_FILE_H_
#define WTK_CFG_WTK_CFG_FILE_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_heap.h"
#include "wtk/core/wtk_str_hash.h"
#include "wtk/core/wtk_queue.h"
#include "wtk/core/rbin/wtk_rbin2.h"
#include "wtk_source.h"
#include "wtk_local_cfg.h"
#include <stdlib.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_cfg_file wtk_cfg_file_t;
#define wtk_cfg_file_add_var_s(cfg,k,kbytes,v) wtk_cfg_file_add_var(cfg,k,kbytes,v,sizeof(v)-1)
#define wtk_cfg_file_add_var_s_s(cfg,k,v) wtk_cfg_file_add_var_s(cfg,k,sizeof(k)-1,v)
#define wtk_cfg_file_add_var_ks(cfg,k,v,vlen) wtk_cfg_file_add_var(cfg,k,sizeof(k)-1,v,vlen)
#define wtk_cfg_file_feed_s(cfg,data) wtk_cfg_file_feed(cfg,data,sizeof(data)-1)

typedef enum
{
	CF_EXPR_START,
	CF_EXPR_TOK_START,
	CF_EXPR_TOK_WAIT_EQ,
	CF_EXPR_VALUE_START,
	CF_EXPR_VALUE_TOK_START,
	CF_EXPR_VALUE_TOK_END,
	CF_VAR_START,
	CF_VAR_TOK,
	CF_VAR_TOK_START,
	CFG_ARRAY_START,
	CFG_ARRAY_TOK_START,
	CFG_ARRAY_TOK_END,
	CFG_COMMENT,
	CFG_ESCAPE_START,
	CFG_ESCAPE_X1,
	CFG_ESCAPE_X2,
	CFG_ESCAPE_O1,
	CFG_ESCAPE_O2,
	CFG_EXPR_IF,
}wtk_cfg_file_state_t;

typedef struct
{
	wtk_queue_node_t q_n;
	wtk_string_t *k;
	wtk_string_t *v;
}wtk_cfg_if_cond_t;

typedef struct
{
	wtk_queue_node_t q_n;
	wtk_queue_t cond;
	unsigned _or:1;
	unsigned valid:1;
}wtk_cfg_if_item_t;


struct wtk_cfg_file
{
	wtk_queue_t cfg_queue;
	wtk_rbin2_t *rbin;
	wtk_heap_t *heap;
	wtk_local_cfg_t *main;	//main configure;
	wtk_local_cfg_t *cur;
	wtk_cfg_file_state_t state;
	wtk_cfg_file_state_t var_cache_state;
	int sub_state;
	wtk_strbuf_t *tok;
	wtk_strbuf_t *value;
	wtk_strbuf_t *var;
	wtk_queue_t if_q;
	//wtk_strbuf_t *comment;	//comment;
	wtk_array_t *array;
	int scope;
	char quoted_char;
	unsigned char escape_char;
	unsigned escaped:1;
	unsigned quoted:1;
	unsigned included:1;
};

void wtk_cfg_file_set_rbin(wtk_cfg_file_t *cfile,wtk_rbin2_t *rbin);
wtk_cfg_file_t* wtk_cfg_file_new_fn(char *fn);
wtk_cfg_file_t* wtk_cfg_file_new_fn2(char *fn,int add_pwd);
wtk_cfg_file_t* wtk_cfg_file_new_fn3(wtk_string_t *dir,char *data,int len,int add_pwd);
wtk_cfg_file_t *wtk_cfg_file_new();
int wtk_cfg_file_bytes(wtk_cfg_file_t *cfg);

/**
 * @param buf_size bytes of buf used by cfg;
 * @param heap_size block of heap used by cfg;
 */
wtk_cfg_file_t *wtk_cfg_file_new_ex(int buf_size,int heap_size);

int wtk_cfg_file_reset(wtk_cfg_file_t *cfg);
int wtk_cfg_file_delete(wtk_cfg_file_t *c);
int wtk_cfg_file_feed_fn(wtk_cfg_file_t* cfg,char *fn);
int wtk_cfg_file_feed(wtk_cfg_file_t *c,char *d,int bytes);

/**
 *	v is end with 0. vbytes not with include 0
 */
int wtk_cfg_file_add_var(wtk_cfg_file_t *cfg,char *k,int kbytes,char *v,int vlen);
void wtk_cfg_file_print(wtk_cfg_file_t *c);
#ifdef __cplusplus
};
#endif
#endif
