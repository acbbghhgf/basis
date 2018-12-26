#ifndef WTK_CORE_WTK_QUEUE3_H_
#define WTK_CORE_WTK_QUEUE3_H_
#include "wtk/core/wtk_queue.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_queue3 wtk_queue3_t;
#define wtk_queue3_init(q) {(q)->pop=(q)->push=0;(q)->len=0;}

struct wtk_queue3
{
	wtk_queue_node_t *pop;
	wtk_queue_node_t *push;
	int len;
};

//void wtk_queue3_init(wtk_queue3_t *q);

int wtk_queue3_len(wtk_queue3_t *q);

/**
 * @brief push node to the tail of the queue;
 */
void wtk_queue3_push(wtk_queue3_t *q,wtk_queue_node_t *n);

/**
 * @brief push node to the head of the queue;
 */
void wtk_queue3_push_front(wtk_queue3_t *q,wtk_queue_node_t *n);

/**
 * @brief remove node from queue;
 * @warning: make sure node n is in queue;
 */
void wtk_queue3_remove(wtk_queue3_t *q,wtk_queue_node_t *n);

/**
 *	@brief pop the head node from queue;
 */
wtk_queue_node_t* wtk_queue3_pop(wtk_queue3_t *q);

wtk_queue_node_t* wtk_queue3_pop_back(wtk_queue3_t *q);
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
void wtk_queue3_insert_to(wtk_queue3_t *q,wtk_queue_node_t *n,wtk_queue_node_t* n2);


/**
 *	n0->n2->n3
 *				=>	n0->n1->n2->n3
 *	   n1
 */
void wtk_queue3_insert_before(wtk_queue3_t *q,wtk_queue_node_t *n2,wtk_queue_node_t *n1);

void wtk_queue3_check(wtk_queue3_t *q);

void wtk_queue_link2(wtk_queue_t *dst,wtk_queue3_t *src);

wtk_queue_node_t* wtk_queue3_peek(wtk_queue3_t *q,int index);
#ifdef __cplusplus
};
#endif
#endif
