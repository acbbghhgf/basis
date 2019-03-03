#ifndef WTK_UTIL_WTK_FLIST_H_
#define WTK_UTIL_WTK_FLIST_H_
#include "wtk/core/wtk_str.h"
#include "wtk/core/wtk_heap.h"
#include "wtk/core/wtk_queue.h"
#include "wtk/core/wtk_strbuf.h"
#include "wtk/core/wtk_os.h"
#include "wtk/core/cfg/wtk_source.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_flist wtk_flist_t;

typedef enum
{
	WTK_FITEM_START,
	WTK_FITEM_APPEND,
}wtk_fitem_state_t;

typedef struct
{
	wtk_queue_node_t q_n;
	wtk_string_t *str;
}wtk_fitem_t;

struct wtk_flist
{
	wtk_queue_t queue;
	wtk_heap_t *heap;
	wtk_strbuf_t *buf;
	wtk_fitem_state_t state;
};

wtk_flist_t* wtk_flist_new(char *fn);
int wtk_flist_delete(wtk_flist_t *fl);

typedef void(*wtk_flist_notify_f)(void *ths,char *fn);
void wtk_flist_process(char *fn,void *ths,wtk_flist_notify_f notify);

typedef void(*wtk_flist_notify_f2)(void *ths,char *fn,int index,int cnt);
void wtk_flist_process2(char *fn,void *ths,wtk_flist_notify_f2 notify);

void wtk_flist_process4(char *fn,void *ths,wtk_flist_notify_f notify);

typedef struct
{
	wtk_strbuf_t *buf;
	wtk_source_t src;
}wtk_flist_it_t;

wtk_flist_it_t* wtk_flist_it_new(char *fn);
char* wtk_flist_it_next(wtk_flist_it_t *fl);
void wtk_flist_it_delete(wtk_flist_it_t *fl);
#ifdef __cplusplus
};
#endif
#endif
