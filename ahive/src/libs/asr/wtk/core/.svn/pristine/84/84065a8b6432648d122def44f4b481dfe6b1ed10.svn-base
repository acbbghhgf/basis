#ifndef WTK_CORE_WTK_ARG_H_
#define WTK_CORE_WTK_ARG_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_str_hash.h"
#include "wtk/core/wtk_array.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_arg wtk_arg_t;
#define wtk_arg_get_int_s(arg,k,n) wtk_arg_get_int(arg,k,sizeof(k)-1,n)
#define wtk_arg_get_float_s(arg,k,n) wtk_arg_get_float(arg,k,sizeof(k)-1,n)
#define wtk_arg_get_number_s(arg,k,n) wtk_arg_get_number(arg,k,sizeof(k)-1,n)
#define wtk_arg_exist_s(arg,k) wtk_arg_exist(arg,(char*)k,sizeof(k)-1)
#define wtk_arg_get_str_s(arg,k,pv) wtk_arg_get_str(arg,(char*)k,sizeof(k)-1,pv)

typedef struct wtk_arg_item
{
	wtk_queue_node_t q_n;
	wtk_string_t k;
	wtk_string_t v;
}wtk_arg_item_t;

struct wtk_arg
{
	wtk_queue_t queue;
	wtk_array_t *args;
    wtk_heap_t *heap;
};

wtk_arg_t* wtk_arg_new(int argc,char** argv);
int wtk_arg_delete(wtk_arg_t *arg);
int wtk_arg_get_int(wtk_arg_t *arg,const char *key,int bytes,int* number);
int wtk_arg_get_float(wtk_arg_t *arg,const char *key,int bytes,float* number);
int wtk_arg_get_number(wtk_arg_t *arg,const char *key,int bytes,double *n);
int wtk_arg_exist(wtk_arg_t *arg,const char* key,int bytes);
int wtk_arg_get_str(wtk_arg_t *arg,const char *key,int bytes,char** pv);
void wtk_arg_print(wtk_arg_t *arg);
#ifdef __cplusplus
};
#endif
#endif
