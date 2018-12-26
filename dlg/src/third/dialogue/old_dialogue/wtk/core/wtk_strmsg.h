#ifndef WTK_CORE_WTK_STRMSG_H_
#define WTK_CORE_WTK_STRMSG_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_strbuf.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_strmsg wtk_strmsg_t;

typedef enum
{
	WTK_STRMSG_INIT,
	WTK_STRMSG_MAGIC0,
	WTK_STRMSG_MAGIC1,
	WTK_STRMSG_MAGIC2,
	WTK_STRMSG_MAGIC3,
	WTK_STRMSG_LEN0,
	WTK_STRMSG_LEN1,
	WTK_STRMSG_LEN2,
	//WTK_STRMSG_LEN3,
	WTK_STRMSG_VALUE,
	WTK_STRMSG_END,
}wtk_strmsg_state_t;

struct wtk_strmsg
{
	wtk_strmsg_state_t state;
	wtk_strbuf_t *buf;
	int len;
	unsigned seek:1;
};

void wtk_strmsg_init(wtk_strmsg_t *msg,wtk_strbuf_t *buf,int seek);
int wtk_strmsg_feed(wtk_strmsg_t *msg,char *data,int bytes,int *left);
int wtk_strmsg_is_filled(wtk_strmsg_t *msg);
#ifdef __cplusplus
};
#endif
#endif
