#ifndef QTK_HTTPS_H
#define QTK_HTTPS_H
/*
 *使用https的方法
 * */
#include"wtk/core/wtk_type.h"
#include"curl/curl.h"
#include"wtk/core/wtk_str.h"
#include"wtk/core/wtk_strbuf.h"
#ifdef __cpluscplus
extern "C"{
#endif

struct qtk_https {
	CURL *curl;
	unsigned timeout;
	wtk_string_t url;
	wtk_string_t host;
	wtk_string_t port;
	wtk_strbuf_t *rbuf; //返回的内容
};

typedef struct qtk_https qtk_https_t;

qtk_https_t* qtk_https_new(void);

int qtk_https_get(qtk_https_t *https);
int qtk_https_post(qtk_https_t *https, wtk_string_t *postthis);

void qtk_https_delete(qtk_https_t *https);
#ifdef __cpluscplus
};
#endif
#endif
