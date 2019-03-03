#ifdef USE_CRF
#ifndef WTK_SEMDLG_OWLKG_WTK_CRFACT_PARSER
#define WTK_SEMDLG_OWLKG_WTK_CRFACT_PARSER
#include "wtk/core/wtk_type.h" 
#include "wtk/core/segmenter/wtk_poseg.h"
#include "wtk_crfact_parser_cfg.h"
#include "wtk/core/wtk_str_parser.h"
#include "wtk_crf.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_crfact_parser wtk_crfact_parser_t;


typedef struct
{
	wtk_queue_node_t q_n;
	wtk_string_t *wrd;
	wtk_string_t *pos;
	//wtk_string_t *value;
	wtk_string_t *cls;	//process by lex
	wtk_string_t *inst;
}wtk_crfact_value_t;


/*
 * * question: .ask|.check|.for|.how|.is_or_not|.select|.where|.who|.why
 * * ap|apn|at|n|p|pa|pn|pv|pv1|pvd|t|v|vt|vt1|vtd
*start: [.ask|.check|.for|.how|.is_or_not|.select|.where|.who|.why|p]
* ap* p+ (vt1|vtd)* vt at* t+
* ap* p+ apn pn   地球 上 最大 的 海洋
* ap* p+ pa  今天 天气 很 好
* ap* p+ pv 那 你 就 好好 休息 吧
* ap* p+ t (vt1|vtd)* vt 我 觉得 兔子 胖嘟嘟 的 还是 蛮 喜欢 的 啊
* 你是帅哥还是美女呀 => select(type="还是/pos=c/",p="你/pos=r/",vt="是/pos=v/",t="帅哥/pos=n/",st="美女/pos=n/")
* 爸爸是妈妈的老公 => p t rt
 */
typedef struct
{
	wtk_queue_node_t q_n;
	wtk_string_t *type;
	wtk_crfact_value_t *type_value;

	wtk_queue_t *ap;		//wtk_crfact_item_value_t
	wtk_crfact_value_t *p;

	wtk_queue_t *vtd;
	wtk_queue_t *vt1;
	wtk_crfact_value_t *vt;

	wtk_queue_t *at;
	wtk_crfact_value_t *t;
	wtk_queue_t *st;

	wtk_crfact_value_t *apn;
	wtk_crfact_value_t *pn;

	wtk_crfact_value_t *pa;

	wtk_queue_t *pvd;
	wtk_queue_t *pv1;
	wtk_crfact_value_t *pv;

	wtk_crfact_value_t *rt;
	wtk_crfact_value_t *is;
}wtk_crfact_item_t;

typedef struct
{
	wtk_queue_t act_item_q;
	wtk_crfact_item_t *item;
}wtk_crfact_t;


struct wtk_crfact_parser
{
	wtk_crfact_parser_cfg_t *cfg;
	wtk_poseg_t *seg;
	wtk_crf_t *crf;
	wtk_strbuf_t *buf;
	wtk_heap_t *heap;
};

wtk_crfact_parser_t* wtk_crfact_parser_new(wtk_crfact_parser_cfg_t *cfg);
void wtk_crfact_parser_delete(wtk_crfact_parser_t *p);
void wtk_crfact_parser_reset(wtk_crfact_parser_t *p);
wtk_crfact_t* wtk_crfact_parser_process(wtk_crfact_parser_t *p,char *s,int len);

void wtk_crfact_print(wtk_crfact_t *act);
wtk_string_t* wtk_crfact_get_value(wtk_crfact_t *act,char *k,int k_bytes);
void wtk_crfact_item_get_full_value(wtk_crfact_item_t *item,char *k,int k_bytes,wtk_strbuf_t *buf);
int wtk_crfact_item_has_key(wtk_crfact_item_t *item,char *k,int k_bytes);
int wtk_crfact_item_nvalue(wtk_crfact_item_t *item);
void wtk_crfact_item_print2(wtk_crfact_item_t *v,wtk_strbuf_t *buf);
wtk_crfact_t* wtk_crfact_new2(wtk_heap_t *heap,char *act,int act_bytes);
wtk_crfact_t* wtk_crfact_new(wtk_heap_t *heap);
void wtk_crfact_set(wtk_crfact_t *act,wtk_heap_t *heap,char *k,int k_bytes,char *v,int v_bytes);
wtk_crfact_value_t* wtk_crfact_value_new(wtk_heap_t *heap,wtk_string_t *wrd,wtk_string_t *pos);

void wtk_crfact_item_merge_vt(wtk_crfact_item_t *item,wtk_strbuf_t *buf);
#ifdef __cplusplus
};
#endif
#endif
#endif
