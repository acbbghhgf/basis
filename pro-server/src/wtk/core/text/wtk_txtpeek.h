#ifndef WTK_CORE_TEXT_WTK_TXTPEEK
#define WTK_CORE_TEXT_WTK_TXTPEEK
#include "wtk/core/wtk_type.h" 
#include "wtk_txtpeek_cfg.h"
#include "wtk/core/wtk_strbuf.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_txtpeek wtk_txtpeek_t;

typedef enum
{
	WTK_TXTPEEK_EOF,
	WTK_TXTPEEK_CHN,
	WTK_TXTPEEK_ENG,
	WTK_TXTPEEK_NUM,
}wtk_txtpeek_item_type_t;

typedef struct
{
	wtk_txtpeek_item_type_t type;
	wtk_string_t v;
}wtk_txtpeek_item_t;

struct wtk_txtpeek
{
	wtk_txtpeek_cfg_t *cfg;
	char *s;
	char *e;
};

wtk_txtpeek_t* wtk_txtpeek_new(wtk_txtpeek_cfg_t *cfg);
void wtk_txtpeek_delete(wtk_txtpeek_t *p);
void wtk_txtpeek_set(wtk_txtpeek_t *p,char *txt,int bytes);
wtk_txtpeek_item_t wtk_txtpeek_next(wtk_txtpeek_t *p);
void wtk_txtpeek_process(wtk_txtpeek_t *p,char *txt,int bytes,wtk_strbuf_t *buf);
#ifdef __cplusplus
};
#endif
#endif
