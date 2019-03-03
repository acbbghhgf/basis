#ifndef WTK_CORE_WTK_STRBUF_H_
#define WTK_CORE_WTK_STRBUF_H_
#include "wtk/core/wtk_type.h"
#include "wtk_str.h"
#include "wtk_str_encode.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_strbuf wtk_strbuf_t;
#define wtk_strbuf_push_s(b,s) wtk_strbuf_push(b,s,sizeof(s)-1)
#define wtk_strbuf_push_s0(b,s) wtk_strbuf_push(b,s,sizeof(s))
#define wtk_strbuf_push_string(b,s) wtk_strbuf_push(b,s,strlen(s))
#define wtk_strbuf_reset(b) ((b)->pos=0)
#define wtk_strbuf_push_front_s(b,s) wtk_strbuf_push_front(b,s,sizeof(s)-1)
//#define wtk_strbuf_replace_s(buf,src,src_len,needle) wtk_strbuf_replace(buf,src,src_len,needle,sizeof(needle)-1)
#define BUF_USE_MACRO
#ifndef BUF_USE_MACRO
void wtk_strbuf_push_c(wtk_strbuf_t *s,char b);
#else
#define wtk_strbuf_push_c(buf,b) \
{\
	    if(buf->length<=buf->pos) \
	    {\
	        wtk_strbuf_expand(buf,1); \
	    }\
	    buf->data[buf->pos++]=b; \
}
#endif

struct wtk_strbuf
{
    char *data;					//raw data;
    int pos;					//valid data size;
    int length;					//memory size of raw data;
    float rate;					//memory increase rate;
};

/**
 * @brief create string buffer;
 */
wtk_strbuf_t* wtk_strbuf_new(int init_len,float rate);

/**
 * @brief delete string buffer;
 */
int wtk_strbuf_delete(wtk_strbuf_t* b);

/**
 * @brief push data to the end of buffer;
 */
void wtk_strbuf_push(wtk_strbuf_t *s,const char *buf,int bytes);

/*
 * @brief push
 */
void wtk_strbuf_push_word(wtk_strbuf_t *buf,char *data,int bytes);

/**
 * @brief push data by fmt like printf,this is simple util function.
 */
void wtk_strbuf_push_f(wtk_strbuf_t *b,const char *fmt,...);

/**
 * @brief push data to the font of buffer;
 */
void wtk_strbuf_push_front(wtk_strbuf_t *s,const char *buf,int bytes);

/**
 * @brief pop data from the front of buffer;
 */
int wtk_strbuf_pop(wtk_strbuf_t *s,char* data,int bytes);

void wtk_strbuf_replace(wtk_strbuf_t *buf,char *data,int data_len,char *src,int src_len,char *dst,int dst_len);
//void wtk_strbuf_reset(wtk_strbuf_t *s);
void wtk_strbuf_print(wtk_strbuf_t *s);
void wtk_strbuf_merge(wtk_strbuf_t *buf,wtk_string_t *p1,...);
void wtk_strbuf_merge2(wtk_strbuf_t *buf,char *p1,...);
int wtk_strbuf_read(wtk_strbuf_t *buf,char *fn);
int wtk_strbuf_read2(wtk_strbuf_t *buf,char *fn);
void wtk_strbuf_expand(wtk_strbuf_t *s,int bytes);
char* wtk_strbuf_to_str(wtk_strbuf_t *s);
void wtk_strbuf_strip(wtk_strbuf_t *buf);
void wtk_strbuf_pad0(wtk_strbuf_t *buf,int bytes);
int wtk_strbuf_bytes(wtk_strbuf_t *b);

/**
 * @brief skip white space \t
 */
void wtk_strbuf_push_skip_ws(wtk_strbuf_t *b,char *data,int len);

/**
 * @brief skip ws between chinese;  中文 see you 你好=> 中文see you你好
 */
void wtk_strbuf_push_skip_utf8_ws(wtk_strbuf_t *buf,char *data,int len);

/**
 * @brief push string and pad 0 to data;
 */
void wtk_strbuf_string_to_str(wtk_strbuf_t *buf,char *data,int bytes);

void wtk_strbuf_push_add_escape_str(wtk_strbuf_t *buf,char *data,int bytes);

int wtk_strbuf_itoa(wtk_strbuf_t *buf,int v);
void wtk_strbuf_atochn(wtk_strbuf_t *buf,char *data,int len);
int wtk_strbuf_atochn2(wtk_strbuf_t *buf,char *data,int len);


/**
 *  parse; "hello\n\rbcd"
 */
void wtk_strbuf_parse_quote(wtk_strbuf_t *buf,char *data,int bytes);

void wtk_itochn(wtk_strbuf_t *buf,int v);
void wtk_stochn(wtk_strbuf_t *buf,char *data,int len);
void wtk_stotel(wtk_strbuf_t *buf,char *data,int len);
void wtk_strbuf_real_path(wtk_strbuf_t *buf,char *name,int len);
#ifdef __cplusplus
};
#endif
#endif
