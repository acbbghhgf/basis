#ifndef WTK_CORE_WTK_SHASH_H_
#define WTK_CORE_WTK_SHASH_H_
#include "wtk/core/wtk_type.h"
#include "wtk_heap.h"
#include "wtk_slist.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_shash wtk_shash_t;

struct wtk_shash
{
	wtk_slist_node_t *slot;
	unsigned int nslot;
	unsigned int used;
};

wtk_shash_t* wtk_shash_new(int nslot);
void wtk_shash_delete(wtk_shash_t *h);
void wtk_shash_reset(wtk_shash_t *h);
int wtk_shash_bytes(wtk_shash_t *h);
void wtk_shash_add(wtk_shash_t *h,unsigned int id,wtk_slist_node_t *q_n);
void wtk_shash_dump(wtk_shash_t *h,char *fn);
#ifdef __cplusplus
};
#endif
#endif
