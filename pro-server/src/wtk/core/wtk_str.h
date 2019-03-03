#ifndef WTK_STRING_WTK_STR_H_
#define WTK_STRING_WTK_STR_H_
#include "wtk/core/wtk_type.h"
#include <stdlib.h>
#include <string.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_string wtk_string_t;
#define is_hex(c) (((c)>='0' && (c)<='9')||((c)>='A' && (c)<='F') || ((c)>='a' && (c)<='f'))
#define wtk_string(s) {s,sizeof(s)-1}
#define wtk_string_dup(s) wtk_string_dup_data(s,sizeof(s))
#define wtk_string_end(s) ((s)->data+(s)->len)
#define wtk_string_set(str,d,bytes) (str)->data=d;(str)->len=bytes;
#define wtk_string_set_s(str,data) wtk_string_set(str,data,sizeof(data)-1)
#define wtk_string_cmp_s(str,s) wtk_string_cmp(str,s,sizeof(s)-1)
#define wtk_string_cmp_s_withstart(str,s) wtk_string_cmp_withstart(str,s,sizeof(s)-1)
#define wtk_string_equal_s(str,s) (((str)->len == (sizeof(s)-1)) && (strncmp((str)->data,s,sizeof(s)-1)==0))
#define wtk_str_equal(s1,ns1,s2,ns2) ( (ns1==ns2) && (strncmp(s1,s2,ns1)==0))
#define wtk_str_equal_s(s1,ns1,s2) wtk_str_equal(s1,ns1,s2,sizeof(s2)-1)
#define wtk_string_equal(str1,str2) (((str1)->len==(str2)->len) && (strncmp((str1)->data,(str2)->data,(str1)->len)==0))
#define ngx_str3_cmp(m,c0,c1,c2) \
	(m[0] == c0 && m[1]==c1 && m[2] == c2)

#define ngx_str4_cmp(m,c0,c1,c2,c3) \
	(m[0] == c0 && m[1]==c1 && m[2] == c2 && m[3]==c3)

#define wtk_string_print(s) printf("%.*s\n",(s)->len,(s)->data)
#define wtk_string_str_s(str,s) wtk_string_str(str,s,sizeof(s)-1)
#define wtk_str_str_s(str,len,s) wtk_str_str(str,len,s,sizeof(s)-1)
#define wtk_str_end_with_s(data,len,suf) wtk_str_end_with(data,len,suf,sizeof(suf)-1)
#define wtk_str_start_with_s(data,len,suf) wtk_str_start_with(data,len,suf,sizeof(suf)-1)
#define wtk_string_cmp_withstart_s(s,data) wtk_string_cmp_withstart(s,data,sizeof(data)-1)


struct wtk_string
{
	char* data;
	int len;
};
wtk_string_t* wtk_string_new(int len);
int wtk_string_delete(wtk_string_t *s);
wtk_string_t* wtk_string_dup_data(char* data,int len);
wtk_string_t* wtk_string_dup_data_pad0(char* data,int len);
char* wtk_string_to_str(wtk_string_t *s);
int wtk_string_cmp_withstart(wtk_string_t* str, char*s, int bytes);
int wtk_str_end_with(char *data,int len,char *suf,int suf_bytes);
int wtk_str_start_with(char *data,int len,char *suf,int suf_bytes);
int wtk_string_cmp(wtk_string_t *str,char* s,int bytes);
int wtk_data_cmp(char *str,int str_bytes,char* s,int bytes);
int wtk_string_cmp2(wtk_string_t *str1,wtk_string_t *str2);
int wtk_string_is_char_in(wtk_string_t *s,char c);
int wtk_string_array_has(wtk_string_t **strs,int n,wtk_string_t *s);
unsigned int wtk_string_to_ord(wtk_string_t *str);
char* wtk_str_dup(const char* str);
int wtk_string_char_count(wtk_string_t *str,char c);
int wtk_str_char_count(const char *s,char c);

/**
 *	@brief like stdlib:atoi
 */
long long wtk_str_atoi(char* s,int len);

/**
 * @brief like stdlib:atof
 */
double wtk_str_atof(char *s,int len);

char* wtk_data_dup(const char* data,size_t data_len,size_t alloc_len);
char* wtk_data_dup2(char *data,int bytes);
char* wtk_data_to_str(char *data,int len);
void print_hex(char *data,int len);
void print_char(unsigned char *data,int len);

/**
 * @brief print data in readable format;
 */
void print_data(char* data, int len);
void print_short(short *f,int len);
void print_short2(short *f,int len);
void print_int(int *f,int len);
void print_char2(signed char *f,int len);
void print_uchar(unsigned char *f,int len);
void print_data_f(FILE* f,char* data, int len);
void print_data_f2(FILE* f,char* data, int len,int cn);
int zero_dispose(void *data);
char* str_merge(char* arg1,...);
char *wtk_str_found(char *src,char *key,int key_bytes);

/**
 * find the first occurrence of the sub string in src.
 */
int wtk_string_str(wtk_string_t *str,char *sub,int bytes);
char* wtk_str_chr(char* s,int slen,char c);
char* wtk_str_rchr(char *s,int len,char c);
int wtk_str_str(char *src,int src_len,char *sub,int bytes);
int wtk_char_to_hex(char c);
uint32_t hash_string_value(char* name);
uint32_t hash_string(char* name, uint32_t hash_size);
//uint32_t hash_string_value_len(char* data,int len,int hash_size);

uint32_t hash_string_value_len_seed(unsigned char* p,int len,int hash_size);

#define hash_string_value_len(p,len,slot) hash_string_value_len_seed((unsigned char*)p,len,slot)

char* wtk_itoa(int n);


//---------------------- string util section --------------------
typedef void (*wtk_str_split_f)(void *ths,char *item,int len,int index);
typedef int (*wtk_str_split_is_sep_f)(void *ths,char c);

int wtk_str_split(char *data,int len,char sep,void *split_ths,wtk_str_split_f split);
int wtk_str_split2(char *data,int len,void *split_ths,wtk_str_split_f split,wtk_str_split_is_sep_f is_sep);

void print_float(float *f,int len);
void print_float2(float *f,int len);
void print_double(double *f,int len);
void print_double2(double *f,int len);
void print_double3(double *f,int len);
int wtk_chnstr_atoi(char *data,int len,int *left);
int wtk_chnstr_atoi2(char *data,int len);
int wtk_chnstr_atoi3(char *s,int len);
int wtk_chnstr_atoi4(char *s,int len);
int wtk_str_atoi2(char *data,int len,int *left);
int wtk_is_all_digit(char *data,int len);
int wtk_num_get_unit(int num);
wtk_string_t* wtk_chnstr_itoa(int v);

//-------------------- string format parser --------------------------
typedef void(*wtk_str_attr_f)(void *ths,wtk_string_t *k,wtk_string_t *v);
/**
 * [a=1,b,c=d]
 */
int wtk_str_attr_parse(char *data,int bytes,void *ths,wtk_str_attr_f attr_notify);
int wtk_str_attr_parse2(char *data,int bytes,void *ths,wtk_str_attr_f attr_notify,int *consume);
void print_hex2(char *data,int len);

typedef void(*wtk_str_chnwrd_f)(void *ths,char *data,int len);
void wtk_str_parse_chnwrd(char *data,int len,void *ths,wtk_str_chnwrd_f wrd);

typedef struct
{
	char *s;
	char *e;
}wtk_str_chnwrd_iter_t;

void wtk_str_chnwrd_iter_init(wtk_str_chnwrd_iter_t *iter,char *data,int len);
wtk_string_t wtk_str_chnwrd_iter_next(wtk_str_chnwrd_iter_t *iter);

typedef struct
{
	char *s;
	char *e;
}wtk_str_spwrd_iter_t;
void wtk_str_spwrd_iter_init(wtk_str_spwrd_iter_t *iter,char *data,int len);
wtk_string_t wtk_str_spwrd_iter_next(wtk_str_spwrd_iter_t *iter);
void float_nan_check(float *f,int len);

void wtk_float_print2(float **f,int n1,int n2);
#ifdef __cplusplus
};
#endif
#endif
