#ifndef WTK_CORE_WTK_TIMER_H_
#define WTK_CORE_WTK_TIMER_H_
#include "wtk/core/wtk_rbtree.h"
#include "wtk/core/wtk_queue.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_timer wtk_timer_t;
typedef struct wtk_timer2 wtk_timer2_t;
typedef int (*wtk_timer_handler)(void *data,wtk_timer_t *timer);
typedef int (*wtk_timer2_handler)(void *data,wtk_timer2_t *timer);

struct wtk_timer
{
	//wtk_rbnode_t n;
	wtk_queue_node_t q_n;
	double key;
	wtk_timer_handler handler;
	void *data;
};

struct wtk_timer2
{
	wtk_queue_node_t q_n;
	double key;
	wtk_timer2_handler handler;
	void *data;
};

#ifdef __cplusplus
};
#endif
#endif
