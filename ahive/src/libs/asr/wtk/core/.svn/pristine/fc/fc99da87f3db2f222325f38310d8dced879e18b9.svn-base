#ifndef WTK_CORE_WTK_IF
#define WTK_CORE_WTK_IF
#include "wtk/core/wtk_type.h" 
#include "wtk_queue3.h"
#include "wtk_str.h"
#include "wtk_heap.h"
#include <ctype.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_if wtk_if_t;

typedef enum
{
	WTK_IF_LT,
	WTK_IF_LET,
	WTK_IF_EQ,
	WTK_IF_GET,
	WTK_IF_GT,
	WTK_IF_NEQ
}wtk_if_type_t;

typedef wtk_string_t* (*wtk_if_get_var_f)(void *ths,char *k,int k_len);

typedef struct
{
	wtk_queue_node_t q_n;
	wtk_string_t *k;
	union{
		wtk_string_t *str;
		float number;
	}v;
	wtk_if_type_t type;
	unsigned is_str:1;
}wtk_if_item_t;

struct wtk_if
{
	wtk_queue3_t item_q;
};


wtk_if_t* wtk_if_new(wtk_heap_t *heap,char *data,int len);
int wtk_if_check(wtk_if_t *xif,void *ths,wtk_if_get_var_f get_var);
void wtk_if_print(wtk_if_t *xif);
#ifdef __cplusplus
};
#endif
#endif
