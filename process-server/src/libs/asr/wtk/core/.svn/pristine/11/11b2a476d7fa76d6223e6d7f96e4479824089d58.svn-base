#ifndef WTK_CORE_WTK_STRIDX_H_
#define WTK_CORE_WTK_STRIDX_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_strpool.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_stridx wtk_stridx_t;

typedef struct
{
	wtk_string_t *str;
	int idx;
}wtk_stridx_item_t;

struct wtk_stridx
{
	wtk_str_hash_t *hash;
	wtk_stridx_item_t **items;
	int alloc;
	int used;
};

wtk_stridx_t* wtk_stridx_new(int n);
void wtk_stridx_delete(wtk_stridx_t *idx);
int wtk_stridx_get_id(wtk_stridx_t *idx,char *data,int bytes,int insert);
wtk_string_t*  wtk_stridx_get_str(wtk_stridx_t *idx,int id);
#ifdef __cplusplus
};
#endif
#endif
