#ifndef WTK_CORE_WTK_QUEUE2_H_
#define WTK_CORE_WTK_QUEUE2_H_
#include "wtk/core/wtk_queue.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_queue2 wtk_queue2_t;
#define wtk_queue2_init(q) {(q)->pop=(q)->push=0;}

struct wtk_queue2
{
	wtk_queue_node_t *pop;
	wtk_queue_node_t *push;
};

//void wtk_queue2_init(wtk_queue2_t *q);

int wtk_queue2_len(wtk_queue2_t *q);

/**
 * @brief push node to the tail of the queue;
 */
void wtk_queue2_push(wtk_queue2_t *q,wtk_queue_node_t *n);

/**
 * @brief push node to the head of the queue;
 */
void wtk_queue2_push_front(wtk_queue2_t *q,wtk_queue_node_t *n);

/**
 * @brief remove node from queue;
 * @warning: make sure node n is in queue;
 */
void wtk_queue2_remove(wtk_queue2_t *q,wtk_queue_node_t *n);

/**
 *	@brief pop the head node from queue;
 */
wtk_queue_node_t* wtk_queue2_pop(wtk_queue2_t *q);

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
void wtk_queue2_insert_to(wtk_queue2_t *q,wtk_queue_node_t *n,wtk_queue_node_t* n2);


/**
 * insert n1 before n2
 *	n0->n2->n3
 *				=>	n0->n1->n2->n3
 *	   n1
 */
void wtk_queue2_insert_before(wtk_queue2_t *q,wtk_queue_node_t *n2,wtk_queue_node_t *n1);

/**
 * @brief move node the end of queue(push).
 */
void wtk_queue2_touch_node(wtk_queue2_t *q,wtk_queue_node_t *n);
#ifdef __cplusplus
};
#endif
#endif
