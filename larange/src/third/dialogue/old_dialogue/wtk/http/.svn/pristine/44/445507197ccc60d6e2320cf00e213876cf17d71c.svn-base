#ifndef WTK_HTTP_WTK_HTTP_PARAM_H_
#define WTK_HTTP_WTK_HTTP_PARAM_H_
#include "wtk/core/wtk_str_hash.h"
#include "wtk_http_util.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_http_param wtk_http_param_t;
#define wtk_http_param_get_s(p,s) wtk_http_param_get(p,s,sizeof(s)-1)

struct wtk_http_param
{
	wtk_str_hash_t* hash;
	wtk_strbuf_t *buf;
};

wtk_http_param_t *wtk_http_param_new(int nslot);
int wtk_http_param_delete(wtk_http_param_t *p);
int wtk_http_param_bytes(wtk_http_param_t *p);
int wtk_http_param_reset(wtk_http_param_t *p);
int wtk_http_param_feed(wtk_http_param_t *param,char *p,int bytes);
wtk_string_t* wtk_http_param_get(wtk_http_param_t *p,char* key,int bytes);
//void wtk_http_param_print(wtk_http_param_t *param);
#ifdef __cplusplus
};
#endif
#endif
