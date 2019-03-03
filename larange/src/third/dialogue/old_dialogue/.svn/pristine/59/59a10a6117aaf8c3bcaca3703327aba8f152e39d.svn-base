#ifndef WTK_CORE_ZIP_WTK_ZIP
#define WTK_CORE_ZIP_WTK_ZIP
#include "wtk/core/wtk_type.h" 
#include "wtk_zip_cfg.h"
#include "wtk/core/wtk_queue2.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_zip wtk_zip_t;
typedef struct wtk_zip_blk wtk_zip_blk_t;
typedef struct wtk_zip_item wtk_zip_item_t;

struct wtk_zip_item
{
//	wtk_zip_blk_t *prev_blk;
//	unsigned int id;'
	wtk_queue_node_t q_n;
	wtk_queue2_t next_q;
	unsigned short prev;
	unsigned char c;
	unsigned char used:1;
};

struct wtk_zip_blk
{
	wtk_queue_node_t q_n;
	wtk_zip_item_t *item;
};


struct wtk_zip
{
	wtk_zip_cfg_t *cfg;
	wtk_strbuf_t *buf;
	unsigned int cache_v;
	int cache_bit;
	int len;
	wtk_zip_item_t *item;
	unsigned short nxt_idx;
	wtk_zip_item_t *prev_item;
	unsigned int cnt;
	//unsigned short prev_idx;
	unsigned use:1;
};

extern unsigned int zip_bit_map[];

wtk_zip_t* wtk_zip_new(wtk_zip_cfg_t *cfg);
void wtk_zip_delete(wtk_zip_t *z);
void wtk_zip_reset(wtk_zip_t *z);

int wtk_zip_file(wtk_zip_t *z,char *ifn,char *ofn);
#ifdef __cplusplus
};
#endif
#endif
