#ifndef WTK_CORE_WTK_ROBIN_H_
#define WTK_CORE_WTK_ROBIN_H_
#include "wtk/core/wtk_type.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_robin wtk_robin_t;
#define wtk_robin_at(rb,i) ((rb)->r[((rb)->pop+(i))%((rb)->nslot)])
#define wtk_robin_is_full(rb) ((rb)->nslot==(rb)->used)

/**
 * @brief robin is used for save fixed size array;
 */

struct wtk_robin
{
	int nslot;	//slots of robin
	int pop;	//the first valid data slot
	int used;	//length of valid data
	void **r;
};

wtk_robin_t* wtk_robin_new(int n);
int wtk_robin_delete(wtk_robin_t* r);
void wtk_robin_push(wtk_robin_t* r,void *d);
void* wtk_robin_push2(wtk_robin_t *r,void *d);
void* wtk_robin_pop(wtk_robin_t *r);
void wtk_robin_pop2(wtk_robin_t *r,int n);
void wtk_robin_reset(wtk_robin_t *r);
void* wtk_robin_next(wtk_robin_t* r);

/**
 * @brief used for window shift
 */
void wtk_robin_peek_win_array(wtk_robin_t *robin,void **array,int pad_end);
#ifdef __cplusplus
};
#endif
#endif
