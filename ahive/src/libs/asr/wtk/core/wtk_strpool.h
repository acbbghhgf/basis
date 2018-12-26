#ifndef WTK_CORE_WTK_STRPOOL_H_
#define WTK_CORE_WTK_STRPOOL_H_
#include "wtk/core/wtk_str_hash.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_strpool wtk_strpool_t;

typedef struct
{
	wtk_string_t v;
	/*
	union{
		int i;
		float f;
		void *p;
	}hook;
	*/
	void *hook;
}wtk_strpool_item_t;

typedef struct
{
	wtk_string_t v;
	union{
		int i;
		float f;
		void *p;
	}hook;
}wtk_strpool_xitem_t;

struct wtk_strpool
{
	wtk_str_hash_t *hash;
};

wtk_strpool_t* wtk_strpool_new(int nhint);
void wtk_strpool_delete(wtk_strpool_t *p);
void wtk_strpool_reset(wtk_strpool_t *p);
int wtk_strpool_bytes(wtk_strpool_t *p);

wtk_strpool_item_t* wtk_strpool_find_item(wtk_strpool_t *p,char *v,int v_len,int insert);
wtk_strpool_item_t* wtk_strpool_find_item2(wtk_strpool_t *p,char *v,int v_len,int insert,int *is_new);
wtk_string_t* wtk_strpool_find(wtk_strpool_t *p,char *v,int v_len,int insert);
#ifdef __cplusplus
};
#endif
#endif
