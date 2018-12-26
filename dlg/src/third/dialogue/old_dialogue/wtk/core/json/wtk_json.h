#ifndef WTK_CORE_WTK_JSON_H_
#define WTK_CORE_WTK_JSON_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_queue.h"
#include "wtk/core/wtk_array.h"
#include "wtk/core/wtk_heap.h"
#include "wtk/core/wtk_strbuf.h"
#include "wtk/core/parse/wtk_str_parse.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_json wtk_json_t;
typedef struct wtk_json_item wtk_json_item_t;
//typedef struct wtk_json_parser wtk_json_parser_t;
#define wtk_json_obj_add_ref_str_s(json,item,k,v) wtk_json_obj_add_ref_str(json,item,k,sizeof(k)-1,v)
#define wtk_json_obj_add_str_s(json,item,k,v) wtk_json_obj_add_str(json,item,k,sizeof(k)-1,v)
#define wtk_json_obj_add_ref_number_s(json,item,k,number) wtk_json_obj_add_ref_number(json,item,k,sizeof(k)-1,number)
#define wtk_json_obj_add_item2_s(json,obj,k,item) wtk_json_obj_add_item2(json,obj,k,sizeof(k)-1,item)
#define wtk_json_obj_get_s(obj,k) wtk_json_obj_get(obj,k,sizeof(k)-1)
#define wtk_json_obj_add_str2_s(json,obj,k,v,v_len) wtk_json_obj_add_str2(json,obj,k,sizeof(k)-1,v,v_len)
#define wtk_json_obj_add_str2_ss(json,obj,k,v) wtk_json_obj_add_str2(json,obj,k,sizeof(k)-1,v,sizeof(v)-1)
#define wtk_json_array_add_str_s(json,a,s) wtk_json_array_add_str(json,a,s,sizeof(s)-1)
#define wtk_json_obj_remove_s(i,k) wtk_json_obj_remove(i,k,sizeof(k)-1)
#define wtk_json_item_set_path_str_s(j,s,v,v_len) wtk_json_item_set_path_str(j,s,sizeof(s)-1,v,v_len)
#define wtk_json_item_get_path_string_s(item,k) wtk_json_item_get_path_string(item,k,sizeof(k)-1)

typedef enum
{
	WTK_JSON_FALSE,
	WTK_JSON_TRUE,
	WTK_JSON_NULL,
	WTK_JSON_STRING,
	WTK_JSON_NUMBER,
	WTK_JSON_ARRAY,
	WTK_JSON_OBJECT,
}wtk_json_item_type_t;

typedef struct
{
	wtk_queue_node_t q_n;
	wtk_string_t k;
	wtk_json_item_t *item;
}wtk_json_obj_item_t;

typedef struct
{
	wtk_queue_node_t q_n;
	wtk_json_item_t *item;
}wtk_json_array_item_t;

struct wtk_json_item
{
	wtk_json_item_type_t type;
	union{
		double number;
		wtk_string_t *str;
		wtk_queue_t *array;		//wtk_json_array_item_t
		wtk_queue_t *object;	//wtk_json_obj_item_t
	}v;
};

struct wtk_json
{
	wtk_heap_t *heap;
	wtk_json_item_t  *main;
};


wtk_json_t* wtk_json_new();
void wtk_json_delete(wtk_json_t *json);
void wtk_json_reset(wtk_json_t *json);

wtk_json_item_t* wtk_json_new_item2(wtk_heap_t *heap,wtk_json_item_type_t type);
wtk_json_item_t* wtk_json_new_number2(wtk_heap_t *heap,double v);
wtk_json_item_t* wtk_json_new_string2(wtk_heap_t *heap,char *data,int len);
wtk_json_item_t* wtk_json_new_array2(wtk_heap_t *heap);
wtk_json_item_t* wtk_json_new_object2(wtk_heap_t *heap);


wtk_json_item_t* wtk_json_new_item(wtk_json_t *json,wtk_json_item_type_t type);
wtk_json_item_t* wtk_json_new_number(wtk_json_t *json,double v);
wtk_json_item_t* wtk_json_new_string(wtk_json_t *json,char *data,int len);
wtk_json_item_t* wtk_json_new_array(wtk_json_t *json);
wtk_json_item_t* wtk_json_new_object(wtk_json_t *json);

wtk_json_item_t* wtk_json_obj_get(wtk_json_item_t *obj,char *key,int key_bytes);
wtk_json_item_t* wtk_json_obj_get_first(wtk_json_item_t *obj);
wtk_json_obj_item_t* wtk_json_new_obj_item(wtk_json_t *json,char *key,int key_bytes,wtk_json_item_t *item);
void wtk_json_obj_add_item(wtk_json_t *json,wtk_json_item_t *obj,wtk_json_obj_item_t *item);
void wtk_json_obj_add_item2(wtk_json_t *json,wtk_json_item_t *obj,char *key,int key_bytes,wtk_json_item_t *item);
void wtk_json_obj_set_last_item_value(wtk_json_t *json,wtk_json_item_t *obj,wtk_json_item_t *item);
wtk_json_item_t* wtk_json_obj_add_ref_str(wtk_json_t *json,wtk_json_item_t *obj,char *key,int key_len,wtk_string_t *v);
wtk_json_item_t* wtk_json_obj_add_str(wtk_json_t *json,wtk_json_item_t *obj,char *key,int key_len,wtk_string_t *v);
wtk_json_item_t* wtk_json_obj_add_str2(wtk_json_t *json,wtk_json_item_t *obj,char *key,int key_len,char *v,int v_len);
wtk_json_item_t* wtk_json_obj_add_ref_number(wtk_json_t *json,wtk_json_item_t *obj,char *key,int key_len,double number);
wtk_json_item_t* wtk_json_obj_remove(wtk_json_item_t *item,char *k,int k_len);
wtk_json_obj_item_t* wtk_json_obj_get_valid_item(wtk_json_item_t *item);

void wtk_json_array_add_item(wtk_json_t *json,wtk_json_item_t *array,wtk_json_item_t *item);
void wtk_json_array_add_item2(wtk_json_t *json,wtk_json_item_t *array,wtk_json_item_t *item,int push_front);
wtk_json_item_t* wtk_json_array_add_ref_str(wtk_json_t *json,wtk_json_item_t *array,wtk_string_t *v);
wtk_json_item_t* wtk_json_array_add_str(wtk_json_t *json,wtk_json_item_t *array,char *data,int bytes);
void wtk_json_array_add_ref_number(wtk_json_t *json,wtk_json_item_t *array,double v);
wtk_json_item_t* wtk_json_array_get(wtk_json_item_t* item,int idx);
int wtk_json_array_has_string_value(wtk_json_item_t *item,char *v,int v_bytes);
void wtk_json_array_add_unq_str(wtk_json_t* json,wtk_json_item_t *item,char *v,int v_bytes);
void wtk_json_array_remove_string_value(wtk_json_item_t *item,char *v,int v_bytes);

//---------------------------------------------
void wtk_json_print(wtk_json_t *json,wtk_strbuf_t *buf);
void wtk_json_item_print(wtk_json_item_t *item,wtk_strbuf_t *buf);
void wtk_json_item_print2(wtk_json_item_t *item,wtk_strbuf_t *buf);
void wtk_json_item_print3(wtk_json_item_t *item);
void wtk_json_item_print4(wtk_json_item_t *item);

wtk_json_item_t* wtk_json_item_dup(wtk_json_item_t *item,wtk_heap_t *heap);
int wtk_json_copy_obj_dict(wtk_json_t *json,wtk_json_item_t *dst,wtk_json_item_t *src);
int wtk_json_item_cmp(wtk_json_item_t *src,wtk_json_item_t *dst);

/**
 * return string is ref;
 */
wtk_string_t* wtk_json_item_get_str_value(wtk_json_item_t *item);

/**
 * returned string must deleted
 */
wtk_string_t* wtk_json_item_get_str_value2(wtk_json_item_t *item);
/**
 *[pvt.喜欢吃.菜系]=[湘菜]
 */
int wtk_json_item_set_path_str(wtk_json_t *json,char *k,int k_len,char *v,int v_len);

wtk_json_item_t* wtk_json_item_get_path_item(wtk_json_item_t *item,char *k,int k_len,wtk_string_t *last_k);

wtk_json_item_t* wtk_json_item_add_path_item(wtk_json_t *json,wtk_json_item_t *item,char *k,int k_len,wtk_json_item_type_t type);
wtk_string_t* wtk_json_item_get_path_string(wtk_json_item_t *item,char *k,int k_len);

int wtk_json_item_len(wtk_json_item_t *item);
wtk_json_item_t* wtk_json_array_get_string_value(wtk_json_item_t *item,char *v,int v_bytes);
#ifdef __cplusplus
};
#endif
#endif
