#ifndef WTK_CORE_WTK_HOARD_H_
#define WTK_CORE_WTK_HOARD_H_
#include "wtk/core/wtk_type.h"
#include "wtk_queue.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_hoard  wtk_hoard_t;
typedef int (*wtk_hoard_bytes_f)(void *data);

#define WTK_HOARD \
		wtk_queue_node_t	*free;		\
		wtk_queue_node_t	*use;		\
		wtk_new_handler_t 	newer; 		\
		wtk_delete_handler_t deleter;	\
		wtk_delete_handler2_t deleter2;	\
		void 		*user_data;			\
		int			offset;				\
	    int			max_free;			\
	    int			cur_free;			\
	    int         use_length;

struct wtk_hoard
{
	WTK_HOARD
	/*
	wtk_queue_node_t	*free;		//is the last node in the queue.
	wtk_queue_node_t	*use;		//the last node in the queue.
	wtk_new_handler_t 	newer;
	wtk_delete_handler_t deleter;
	void 		*user_data;
	int			offset;
    int			max_free;
    int			cur_free;
    int         use_length;
    */
};

/**
 * @param offet, offsetof wtk_queuenode_t in the user struct;
 * @param max_free, cache size;
 * @param newer, object create function;
 * @param deleter, object delete function;
 * @param data, newer function app_data;
 */
int wtk_hoard_init(wtk_hoard_t *h,int offset,int max_free,wtk_new_handler_t newer,
		wtk_delete_handler_t deleter,void *data);

int wtk_hoard_init2(wtk_hoard_t *h,int offset,int max_free,wtk_new_handler_t newer,
		wtk_delete_handler2_t deleter,void *data);

void wtk_hoard_reset(wtk_hoard_t *h);

/**
 *	@brief delete all memory;
 */
int wtk_hoard_clean(wtk_hoard_t *h);

/**
 * @brief pop element;
 */
void* wtk_hoard_pop(wtk_hoard_t *h);

/**
 * @brief push element;
 */
int wtk_hoard_push(wtk_hoard_t *h,void* data);
int wtk_hoard_bytes(wtk_hoard_t *h,wtk_hoard_bytes_f bf);

/*
 * @brief reuse use queue;
 */
void wtk_hoard_reuse(wtk_hoard_t *h);
void wtk_hoard_pack(wtk_hoard_t *h);
//=============== hoard test section =================
void wtk_hoard_test_g();
#ifdef __cplusplus
};
#endif
#endif
