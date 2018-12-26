#ifndef _WTK_CORE_WTK_QUEUE_H_
#define _WTK_CORE_WTK_QUEUE_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_str.h"
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
struct wtk_heap;
typedef struct wtk_queue_node wtk_queue_node_t;
typedef struct wtk_queue wtk_queue_t;
//#define wtk_queue_node_data(q,type,link) ( (q) ? (void*)((char*)q-(unsigned long)&(((type*)0)->link)) : 0)
#define wtk_queue_node_data(q,type,link) (type*)((q)>0 ? (void*)((char*)q-offsetof(type,link))  : 0)
#define wtk_queue_node_data_byof(q,off)  ( (q)> 0 ? ((void*)((char*)q-off)) : 0)
#define wtk_queue_node_init(n) (n)->next=(n)->prev=0
#define wtk_queue_init(q) memset(q,0,sizeof(*q))

typedef void (*queue_push_listener)(void* data);

struct wtk_queue_node
{
	wtk_queue_node_t* next;
	wtk_queue_node_t* prev;
};

typedef struct
{
	wtk_queue_node_t q_n;
	void *hook;
}wtk_queue_item_t;

typedef struct
{
	wtk_queue_node_t q_n;
	union
	{
		float f;
		void *p;
		wtk_string_t str;
	}v;
}wtk_queue_node_v_t;

#define WTK_QUEUE \
	wtk_queue_node_t *pop; \
	wtk_queue_node_t *push; \
	int length;\
	queue_push_listener listener; \
	void *data; \

struct wtk_queue
{
	WTK_QUEUE
};

/**
 * @brief create queue from heap;
 */
wtk_queue_t* wtk_queue_new_h(struct wtk_heap *h);

/**
 * @brief new queue, just malloc wtk_queue_t memory and init;
 */
wtk_queue_t* wtk_queue_new();

/**
 * @brief free memory of wtk_queue_t;
 */
int wtk_queue_delete(wtk_queue_t *q);

/**
 * @brief init queue struct;
 */
int wtk_queue_init2(wtk_queue_t *q);

/**
 * @brief push node to the tail of the queue;
 */
int wtk_queue_push(wtk_queue_t *q,wtk_queue_node_t *n);

/**
 *   add src to dst
 */
void wtk_queue_link(wtk_queue_t *dst,wtk_queue_t *src);

/**
 * @brief push node to the head of the queue;
 */
int wtk_queue_push_front(wtk_queue_t *q,wtk_queue_node_t *n);

/**
 * @brief remove node from queue;
 * @warning: make sure node n is in queue;
 */
int wtk_queue_remove(wtk_queue_t *q,wtk_queue_node_t *n);

/**
 *	@brief pop the head node from queue;
 */
wtk_queue_node_t* wtk_queue_pop(wtk_queue_t *q);

wtk_queue_node_t* wtk_queue_pop_back(wtk_queue_t *q);

/**
 * @brief insert n2 node after n, like:
 * 	 n->next=n2; n2->prev=n;
 * 	 dono't care about n->next queue list;
 * 	 n->n00->n01->n02
 * 	                        =>     n->n2->n20->n21->n22;
 * 	 n2->n20->n21->n22
 *
 * 	 this usually used the next node of n is not important,and attach n2;
 */
void wtk_queue_insert_to(wtk_queue_t *q,wtk_queue_node_t *n,wtk_queue_node_t* n2);

/*
 * @brief insert n2 befor n;
 */
void wtk_queue_insert_before(wtk_queue_t *q,wtk_queue_node_t *n,wtk_queue_node_t *n2);

/**
 *	@brief find value by cmp handler, value is node of queue minus of;
 */
void* wtk_queue_find(wtk_queue_t *q,int of,wtk_cmp_handler_t cmp,void *user_data);

/**
 *	@brief swap n1 and n2 in the queue;
 */
void wtk_queue_swap(wtk_queue_t *q,wtk_queue_node_t *n1,wtk_queue_node_t *n2);

/**
 * @brief move node the end of queue(push).
 */
void wtk_queue_touch_node(wtk_queue_t *q,wtk_queue_node_t *n);

/**
 * @brief move node the head of queue(push).
 */
void wtk_queue_touch_front(wtk_queue_t *q,wtk_queue_node_t *n);

/**
 * @brief set push listener, when push element to the queue, listener is called
 * notify there is some element is pushed, used for multithread blockqueue;
 */
void wtk_queue_set_push_listener(wtk_queue_t *q,queue_push_listener l,void *data);

/**
 * @brief traversal node of the queue;
 * 	for walk: typedef int (*wtk_walk_handler_t)(void *user_data,void* data);
 * 		data: node - of;
 */
int wtk_queue_walk(wtk_queue_t *q,int of,wtk_walk_handler_t walk,void *user_data);

/**
 * @brief peek node at given index,index is [0,length)
 */
wtk_queue_node_t* wtk_queue_peek(wtk_queue_t *q,int index);

/**
 *	@brief print the queue;
 *	for print:typedef void (*wtk_print_handler_t)(void *data);
 *		data: node -of;
 */
void wtk_queue_print(wtk_queue_t *q,int of,wtk_print_handler_t print);

/**
 *  wtk_cmp_handler_t(wtk_queue_node_t *src,wtk_queue_node_t *dst);
 */
void wtk_queue_insert(wtk_queue_t *q,wtk_queue_node_t *n,wtk_cmp_handler_t cmp);

/**
 * @brief check queue is valid or not;
 */
int wtk_queue_check(wtk_queue_t *q);

int wtk_queue_node_len(wtk_queue_node_t *qn);

wtk_queue_node_t* wtk_queue_find_node(wtk_queue_t *q,wtk_queue_node_t *qn);


typedef int (*wtk_queue_node_cmp_f)(wtk_queue_node_t *qn1,wtk_queue_node_t *qn2);

void wtk_queue_sort(wtk_queue_t *q,wtk_queue_node_cmp_f cmp);
//------------------------ test/example section ------------
void wtk_queue_test_g();
#ifdef __cplusplus
};
#endif
#endif
