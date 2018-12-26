#ifndef WTK_CORE_WTK_STRV_H_
#define WTK_CORE_WTK_STRV_H_
#include "wtk/core/wtk_type.h"
#include "wtk_str.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_strv wtk_strv_t;
#define wtk_strv_s(s,v) {{s,sizeof(s)-1},{(void*)v}}
#define wtk_strv_array_find_s(v,v_len,s) wtk_strv_array_find(v,v_len,s,sizeof(s)-1)

struct wtk_strv
{
	wtk_string_t key;
	union
	{
		void *v;
		char *s;
		int i;
	}v; //don't use type for when use this,you must known the value is;
};

/**
 *	@brief sort array by key, 0<1<2<3
 */
void wtk_strv_array_sort(wtk_strv_t* v,int v_len);

/**
 * @brief search by key use binary search, v must be ordered;
 */
wtk_strv_t* wtk_strv_array_find(wtk_strv_t *v,int v_len,char *key,int key_bytes);

/**
 * @brief search by key, use increase search element 0,1,2,3 ...
 */
wtk_strv_t* wtk_strv_array_find2(wtk_strv_t *v,int v_len,char *key,int key_bytes);

/**
 * @brief print strv;
 */
void wtk_strv_print(wtk_strv_t *v);

/**
 * @brief print strv array;
 */
void wtk_strv_array_print(wtk_strv_t *v,int v_len);

/**
 * @brief example;
 */
void wtk_strv_array_test();
#ifdef __cplusplus
};
#endif
#endif
