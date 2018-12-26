#ifndef WTK_CORE_CFG_WTK_LOCAL_CFG_H_
#define WTK_CORE_CFG_WTK_LOCAL_CFG_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_str.h"
#include "wtk/core/wtk_heap.h"
#include "wtk/core/wtk_str_hash.h"
#include "wtk_cfg_queue.h"
#include "wtk/core/wtk_os.h"
#include "wtk/core/wtk_arg.h"
#include "wtk/core/cfg/wtk_source.h"
#include <stdlib.h>
#ifdef __cplusplus
extern "C" {
#endif
/**
 * =============================== Message ====================
 *	* wtk_string_t in item of configure, data of wtk_string_t is 0 end and pos not include 0;
 */

#define wtk_local_cfg_update_cfg_str2(lc,cfg,item,key,v) {v=wtk_local_cfg_find_string_s(lc,STR(key)); if(v){cfg->item=v->data;}}
#define wtk_local_cfg_update_cfg_f2(lc,cfg,item,key,v) {v=wtk_local_cfg_find_string_s(lc,STR(key)); if(v){cfg->item=atof(v->data);}}
#define wtk_local_cfg_update_cfg_i2(lc,cfg,item,key,v) {v=wtk_local_cfg_find_string_s(lc,STR(key)); if(v){cfg->item=atoi(v->data);}}
#define wtk_local_cfg_update_cfg_b2(lc,cfg,item,key,v) {v=wtk_local_cfg_find_string_s(lc,STR(key)); if(v){cfg->item=(atoi(v->data)==1)?1:0;}}
#define wtk_local_cfg_update_cfg_str(lc,cfg,item,v) {v=wtk_local_cfg_find_string_s(lc,STR(item)); if(v){cfg->item=v->data;}}
#define wtk_local_cfg_update_cfg_str_local(lc,cfg,item,v) {v=wtk_local_cfg_find_string2_s(lc,STR(item),0); if(v){cfg->item=v->data;}}
#define wtk_local_cfg_update_cfg_string(lc,cfg,item,v) {v=wtk_local_cfg_find_string_s(lc,STR(item)); if(v){cfg->item=v;}}
#define wtk_local_cfg_update_cfg_string_v(lc,cfg,item,v) {v=wtk_local_cfg_find_string_s(lc,STR(item)); if(v){cfg->item=*v;}}
#define wtk_local_cfg_update_cfg_f(lc,cfg,item,v) {v=wtk_local_cfg_find_string_s(lc,STR(item)); if(v){cfg->item=atof(v->data);}}
#define wtk_local_cfg_update_cfg_i(lc,cfg,item,v) {v=wtk_local_cfg_find_string_s(lc,STR(item)); if(v){cfg->item=atoi(v->data);}}
#define wtk_local_cfg_update_cfg_b(lc,cfg,item,v) {v=wtk_local_cfg_find_string_s(lc,STR(item)); if(v){cfg->item=(atoi(v->data)==1)?1:0;}}
#define wtk_local_cfg_update_cfg_b_local(lc,cfg,item,v) {v=wtk_local_cfg_find_string2_s(lc,STR(item),0); if(v){cfg->item=(atoi(v->data)==1)?1:0;}}
#define wtk_local_cfg_update_cfg_x(lc,cfg,item,v,x) {v=wtk_local_cfg_find_string_s(lc,STR(item)); if(v){cfg->item=x(v->data);}}
#define wtk_local_cfg_find_string_s(cfg,s) wtk_local_cfg_find_string(cfg,s,sizeof(s)-1)
#define wtk_local_cfg_find_string2_s(cfg,s,r) wtk_local_cfg_find_string2(cfg,s,sizeof(s)-1,r)
#define wtk_local_cfg_find_lc_s(cfg,s) wtk_local_cfg_find_lc(cfg,s,sizeof(s)-1)
#define wtk_local_cfg_find_array_s(cfg,s) wtk_local_cfg_find_array(cfg,s,sizeof(s)-1)
#define wtk_local_cfg_find_int_array_s(cfg,s) wtk_local_cfg_find_int_array(cfg,s,sizeof(s)-1)
#define wtk_local_cfg_find_float_array_s(cfg,s) wtk_local_cfg_find_float_array(cfg,s,sizeof(s)-1)
#define wtk_local_cfg_find_s(cfg,n) wtk_local_cfg_find(cfg,n,sizeof(n)-1)
#define print_cfg_s(cfg,s) printf("%s:\t%s\n",STR(s),cfg->s?cfg->s:"NULL")
#define print_cfg_string(cfg,s) printf("%s:\t%*.*s\n",STR(s),cfg->s?cfg->s->len:4,cfg->s?cfg->s->len:4,cfg->s?cfg->s->data:"NULL")
#define print_cfg_i(cfg,s) printf("%s:\t%d\n",STR(s),cfg->s)
#define print_cfg_f(cfg,s) printf("%s:\t%f\n",STR(s),cfg->s)
typedef struct wtk_local_cfg wtk_local_cfg_t;

struct wtk_local_cfg
{
	wtk_queue_node_t q_n;
	wtk_string_t name;
	wtk_cfg_queue_t *cfg;
	wtk_heap_t *heap;
	wtk_local_cfg_t *parent;
};

wtk_local_cfg_t *wtk_local_cfg_new_h(wtk_heap_t *h);
wtk_cfg_item_t* wtk_local_cfg_find(wtk_local_cfg_t *cfg,char *d,int bytes);
wtk_cfg_item_t* wtk_local_cfg_find_local(wtk_local_cfg_t *cfg,char *d,int bytes);
wtk_string_t* wtk_local_cfg_find_string(wtk_local_cfg_t *cfg,char *d,int bytes);
wtk_string_t* wtk_local_cfg_find_string2(wtk_local_cfg_t *cfg,char *d,int bytes,int recursive);
wtk_local_cfg_t* wtk_local_cfg_find_lc(wtk_local_cfg_t *cfg,char *d,int bytes);

/**
 * @param last_field is set when called,if section is "httpd:nk:port",last_field is set to "port";
 * @return NULL if not found;
 */
wtk_local_cfg_t* wtk_local_cfg_find_section_lc(wtk_local_cfg_t* lc,char *section,int section_bytes);

/**
 * @brief wtk_array_t is wtk_string_t* array;
 */
wtk_array_t* wtk_local_cfg_find_array(wtk_local_cfg_t *cfg,char *d,int bytes);

/**
 * @brief wtk_array_t is int array;
 */
wtk_array_t* wtk_local_cfg_find_int_array(wtk_local_cfg_t *cfg,char *d,int bytes);

/**
 * @brief wtk_array_t is float array;
 */
wtk_array_t* wtk_local_cfg_find_float_array(wtk_local_cfg_t *cfg,char *d,int bytes);

void wtk_local_cfg_print(wtk_local_cfg_t *lc);
void wtk_local_cfg_update_arg(wtk_local_cfg_t *lc,wtk_arg_t *arg,int show);
void wtk_local_cfg_update(wtk_local_cfg_t *lc,wtk_local_cfg_t *custom);
void wtk_local_cfg_value_to_string(wtk_local_cfg_t *lc,wtk_strbuf_t *buf);
#ifdef __cplusplus
};
#endif
#endif
