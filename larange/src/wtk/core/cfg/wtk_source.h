#ifndef WTK_MODEL_WTK_SOURCE_H_
#define WTK_MODEL_WTK_SOURCE_H_
#include "wtk/core/wtk_str.h"
#include "wtk/core/wtk_strbuf.h"
#include "wtk/core/wtk_str_encode.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_source wtk_source_t;
typedef struct wtk_source_loader wtk_source_loader_t;
typedef int (*wtk_source_get_handler_t)(void*);
typedef int (*wtk_source_get_str_f)(void*,char *buf,int bytes);
typedef int (*wtk_source_read_str_f)(void*,wtk_strbuf_t *buf);
typedef wtk_string_t* (*wtk_source_get_file_f)(void*);
typedef int (*wtk_soure_get_file_size_f)(void*);
typedef int (*wtk_source_unget_handler_t)(void*,int);
typedef int (*wtk_source_load_handler_t)(void *data_ths,wtk_source_t *s);
typedef int (*wtk_source_loader_v_t)(void* inst,void *data,wtk_source_load_handler_t loader,char *fn);
#define wtk_source_get(s) ((s)->get((s)->data))
//#define wtk_source_get(s) wtk_source_read_char(s)
#define wtk_source_unget(s,c) ((s)->unget((s)->data,c))
#define wtk_source_seek_to_s(s,d) wtk_source_seek_to(s,d,sizeof(d)-1)
#define wtk_source_seek_to2_s(s,d,b) wtk_source_seek_to2(s,d,sizeof(d)-1,b)

typedef struct
{
	FILE *f;
	unsigned char *buf;
	unsigned char *valid;
	unsigned char *cur;
	unsigned eof:1;
	int alloc;
}wtk_source_file_item_t;

struct wtk_source
{
	wtk_source_get_handler_t get;
	wtk_source_unget_handler_t unget;
	wtk_source_get_str_f get_str;
	wtk_source_read_str_f read_str;
	wtk_source_get_file_f get_file;
	//wtk_soure_get_file_size_f get_filesize;
	void *data;
	unsigned char swap:1;
	//wtk_strbuf_t *buf;
};

struct wtk_source_loader
{
	void *hook;
	wtk_source_loader_v_t vf;
};

void wtk_source_init(wtk_source_t *src);
void wtk_swap_short(short *p);
void wtk_swap_int32(int *p);
int wtk_is_little_endian();
int wtk_file_write_float(FILE *file,float *f,int n,int bin,int swap);
int wtk_source_init_file(wtk_source_t *s,char *fn);
int wtk_source_clean_file(wtk_source_t *s);
int wtk_source_init_file2(wtk_source_t *s,char *fn);
int wtk_source_clean_file2(wtk_source_t *s);
int wtk_source_init_str(wtk_source_t *s,const char *data,int bytes);
void wtk_source_set_str(wtk_source_t *s,const char *data,int bytes);
int wtk_source_clean_str(wtk_source_t *s);


int wtk_source_init_fd(wtk_source_t *s,FILE *f,int pos);
int wtk_source_clean_fd(wtk_source_t *s);

/**
 * @brief input buf b will be reset
 */
int wtk_source_read_string(wtk_source_t *s,wtk_strbuf_t *b);

/**
 * @brief input buf b will not be reset
 */
int wtk_source_read_string2(wtk_source_t *s,wtk_strbuf_t *b);
int wtk_source_read_string3(wtk_source_t *s,wtk_strbuf_t *b);

int wtk_source_read_normal_string(wtk_source_t *s,wtk_strbuf_t *b);

int wtk_source_read_wtkstr(wtk_source_t *s,wtk_strbuf_t *b);
int wtk_source_read_wtkstr2(wtk_source_t *s,wtk_strbuf_t *b,int bi);
int wtk_source_read_line(wtk_source_t *s,wtk_strbuf_t *b);
int wtk_source_read_line2(wtk_source_t *s,wtk_strbuf_t *b,int *eof);
int wtk_source_skip_sp(wtk_source_t *s,int *nl);
int wtk_source_skip_sp2(wtk_source_t *s,int *nl,int *eof);
int wtk_source_skip_sp3(wtk_source_t *s,int *nl);
int wtk_source_peek(wtk_source_t *s);
int wtk_source_fill(wtk_source_t* s,char* data,int len);
int wtk_source_read_short(wtk_source_t* s,short* v,int n,int bin);
int wtk_source_read_ushort(wtk_source_t* s,unsigned short* v,int n,int bin);
int wtk_source_read_int(wtk_source_t *s,int* v,int n,int bin);
int wtk_source_atof(wtk_source_t* s,double *v);
int wtk_source_atoi(wtk_source_t* s,int* value);
int wtk_source_read_double(wtk_source_t *s,double *f,int n);
int wtk_source_read_float(wtk_source_t *s,float *f,int n,int bin);
int wtk_source_read_char(wtk_source_t *s);
int wtk_source_read_utf8_char(wtk_source_t *s,wtk_strbuf_t *buf);
int wtk_source_seek_to(wtk_source_t *s,char *data,int len);
int wtk_source_seek_to2(wtk_source_t *src,char *data,int len,wtk_strbuf_t *buf);
int wtk_source_load_file(void *data,wtk_source_load_handler_t loader,char *fn);
int wtk_source_load_file_v(void *hook,void *data,wtk_source_load_handler_t loader,char *fn);
void wtk_source_loader_init_file(wtk_source_loader_t *sl);
int wtk_source_loader_load(wtk_source_loader_t *l,void *data_ths,wtk_source_load_handler_t loader,char *fn);
int wtk_source_loader_file_lines(wtk_source_loader_t *sl,char *fn);
wtk_string_t* wtk_source_read_file(wtk_source_t *src);
void wtk_source_read_file2(wtk_source_t *src,wtk_strbuf_t *buf);

float* wtk_file_read_float(char *fn,int *n);
#ifdef __cplusplus
};
#endif
#endif
