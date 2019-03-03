#ifndef WTK_LIB_OBJECT_PARAM_H_
#define WTK_LIB_OBJECT_PARAM_H_
#include "wtk/core/wtk_type.h"
//#include "wtk/core/wtk_heap.h"
#ifdef __cplusplus
extern "C" {
#endif
#define wtk_param_set_str_s(p,s) {(p)->value.str.data=s;(p)->value.str.len=sizeof(s)-1;(p)->value.str.is_ref=1;}
typedef struct wtk_param wtk_param_t;
typedef void (*wtk_param_free_handler_t)(void *data);
#define wtk_param_is_string(p) ((p)->type==WTK_STRING)
#define wtk_param_new_str_s(s) wtk_param_new_str(s,sizeof(s)-1)
#define wtk_param_set_ref_str_s(p,msg) wtk_param_set_ref_str(p,msg,sizeof(msg)-1)
typedef wtk_param_t* (*wtk_param_query_f)(void *usr_data,char *key,int key_bytes);


typedef enum
{
	WTK_NIL=0,
	WTK_BIN=1,
	WTK_NUMBER=2,
	WTK_STRING=3,
	WTK_ARRAY=4,
	WTK_OCT=5,
}wtk_paramtype_t;

struct wtk_param
{
	wtk_paramtype_t type;
	wtk_param_free_handler_t free;
	union {
		struct {
			char* data;
			int len;
			//if data is reference, data need not be freed.
			unsigned char is_ref:1;
		} bin;
		struct{
			char* data;
			int len;
			unsigned char is_ref:1;
		}str;
		struct{
			wtk_param_t **params;
			int len;
		}array;
		double number;
	} value;
	unsigned char is_ref:1;
};

wtk_param_t* wtk_param_dup(wtk_param_t *param);
wtk_param_t* wtk_param_new(wtk_paramtype_t type);
//wtk_param_t* wtk_param_dup(wtk_param_t *src);
wtk_param_t* wtk_param_new_bin(char* data,int len);
wtk_param_t* wtk_param_new_bin2(char* data,int len);
wtk_param_t* wtk_param_new_oct(char* data,int len);
wtk_param_t* wtk_param_new_str(char* data,int len);
wtk_param_t* wtk_param_new_number(double number);
wtk_param_t* wtk_param_new_array(int len);
void wtk_param_set_ref_number(wtk_param_t *param,int v);
void wtk_param_set_ref_str(wtk_param_t *p,char *msg,int msg_bytes);
int wtk_param_delete(wtk_param_t *p);
int wtk_param_bytes(wtk_param_t *p);
int wtk_param_clean(wtk_param_t* p);
void wtk_param_print(wtk_param_t* p);
#ifdef __cplusplus
};
#endif
#endif
