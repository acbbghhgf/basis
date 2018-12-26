#ifndef WTK_HTTP_WTK_HTTP_UTIL_H_
#define WTK_HTTP_WTK_HTTP_UTIL_H_
#include "wtk/core/wtk_strbuf.h"
#include "wtk/core/wtk_str.h"
#ifdef __cplusplus
extern "C" {
#endif
#define wtk_http_url_encode_s(buf,s) wtk_http_url_encode(buf,s,sizeof(s)-1)
#define wtk_http_url_encode_kv_s(buf,k,v,vl) wtk_http_url_encode_kv(buf,k,sizeof(k)-1,v,vl)
#define wtk_http_url_encode_kv_ss(buf,k,v) wtk_http_url_encode_kv(buf,k,sizeof(k)-1,v,sizeof(v)-1)

int wtk_http_url_decode(wtk_strbuf_t *buf,wtk_string_t *p);
void wtk_http_url_encode(wtk_strbuf_t *buf,char *s,int s_bytes);
void wtk_http_url_encode2(wtk_strbuf_t *buf,char *s,int s_bytes);
void wtk_http_url_encode_kv(wtk_strbuf_t *buf,char *key,int key_bytes,char *v,int v_bytes);
wtk_string_t* wtk_http_file2content(char *fn,int len);

typedef void (*wtk_http_param_cb_f)(void *app_data,wtk_string_t *k,wtk_string_t *v);

/**
 *	@brief: url: 	http://10.12.7.100:8083/test?data=help&simple=1
 *			param:	data=help&simple=1
 */
void wtk_http_param_parse(wtk_strbuf_t *buf,char *p,int bytes,wtk_http_param_cb_f cb,void *cb_data);


typedef struct
{
	wtk_string_t ip;
	wtk_string_t port;
	wtk_string_t uri;
}wtk_http_url_t;

//http://58.210.177.12:9005/res/music/Schnappi - Schnappi.mp3
int wtk_http_url_decode_http(wtk_http_url_t *url,char *data,int len);
void wtk_http_url_print(wtk_http_url_t *url);
#ifdef __cplusplus
};
#endif
#endif
