#ifndef WTK_CORE_WTK_STRLIST
#define WTK_CORE_WTK_STRLIST
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_queue.h"
#include "wtk/core/wtk_strbuf.h"
#include "wtk/core/cfg/wtk_source.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_strlist wtk_strlist_t;

typedef struct
{
	wtk_queue_node_t q_n;
	wtk_string_t *k;
	union
	{
		int i;
		float f;
		void *p;
	}v;
}wtk_strlist_item_t;

struct wtk_strlist
{
	wtk_queue_t q;
};

wtk_strlist_t* wtk_strlist_new();
wtk_strlist_t* wtk_strlist_new2(char *fn);
void wtk_strlist_delete(wtk_strlist_t *l);

#ifdef __cplusplus
};
#endif
#endif
