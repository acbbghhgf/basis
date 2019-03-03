#ifndef WTK_FISH_LEARN_WTK_MODEL
#define WTK_FISH_LEARN_WTK_MODEL
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_heap.h"
#include "wtk/core/wtk_str_hash.h"
#include "wtk/core/wtk_queue2.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_model wtk_model_t;
#define wtk_model_add_listener_s(m,k,ths,n) wtk_model_add_listener(m,k,sizeof(k)-1,ths,n)
#define wtk_model_set_i_s(m,k,i) wtk_model_set_i(m,k,sizeof(k)-1,i)
#define wtk_model_set_f_s(m,k,f) wtk_model_set_f(m,k,sizeof(k)-1,f);
#define wtk_model_set_p_s(m,k,p) wtk_model_set_p(m,k,sizeof(k)-1,p);
#define wtk_model_get_item_s(m,k) wtk_model_get_item(m,k,sizeof(k)-1)
#define wtk_model_get_data_s(m,k) wtk_model_get_data(m,k,sizeof(k)-1)

typedef enum
{
	WTK_MODEL_V_NIL,
	WTK_MODEL_V_P,
	WTK_MODEL_V_I,
	WTK_MODEL_V_F,
}wtk_model_item_type_t;

typedef struct
{
	wtk_model_item_type_t type;
	union
	{
		void *p;
		int i;
		float f;
	}v;
}wtk_model_item_v_t;

typedef void(*wtk_model_notify_f)(void *ths,wtk_model_item_v_t *old_value,wtk_model_item_v_t *new_value);

typedef void*(*wtk_model_get_data_f)(void *ths,char *k,int k_bytes);
typedef	void (*wtk_model_touch_f)(void *ths);


typedef struct
{
	wtk_queue_node_t q_n;
	void *ths;
	wtk_model_notify_f notify;
}wtk_model_listener_item_t;

typedef struct
{
//	wtk_queue_node_t q_n;
	wtk_string_t *k;
	wtk_model_item_v_t v;
	wtk_queue2_t listener_q;
}wtk_model_item_t;

struct wtk_model
{
	wtk_str_hash_t *hash;
	void *data_ths;
	wtk_model_get_data_f get_data_f;
	void *touch_ths;
	wtk_model_touch_f touch_f;
};

wtk_model_t* wtk_model_new(int nslot);
void wtk_model_delete(wtk_model_t *m);
void wtk_model_add_listener(wtk_model_t *m,char *k,int k_bytes,void *ths,wtk_model_notify_f notify);
void wtk_model_add_listener2(wtk_model_t *m,wtk_model_item_t *item,void *ths,wtk_model_notify_f notify);
wtk_model_item_t* wtk_model_set_i(wtk_model_t *m,char *k,int k_bytes,int i);
wtk_model_item_t* wtk_model_set_f(wtk_model_t *m,char *k,int k_bytes,float f);
wtk_model_item_t* wtk_model_set_p(wtk_model_t *m,char *k,int k_bytes,void *p);
wtk_model_item_t* wtk_model_get_item(wtk_model_t *m,char *k,int k_bytes);
void wtk_model_item_set(wtk_model_item_t *item,wtk_model_item_v_t *new_value);
void wtk_model_item_set_i(wtk_model_item_t *item,int i);
void wtk_model_item_set_f(wtk_model_item_t *item,float f);
void wtk_model_item_set_p(wtk_model_item_t *item,void *p);
void wtk_model_set_data_get(wtk_model_t *m,void *ths,wtk_model_get_data_f get_data);
void* wtk_model_get_data(wtk_model_t *m,char *k,int k_bytes);
void wtk_model_set_touch(wtk_model_t *m,void *ths,wtk_model_touch_f touch);
#ifdef __cplusplus
};
#endif
#endif
