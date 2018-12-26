#ifndef WTK_CORE_WTK_SPLAYTREE_H_
#define WTK_CORE_WTK_SPLAYTREE_H_
#include "wtk/core/wtk_type.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_spnode wtk_spnode_t;
typedef struct wtk_splaytree wtk_splaytree_t;

struct wtk_spnode
{
	wtk_spnode_t *left;
	wtk_spnode_t *right;
	void *value;
};

struct wtk_splaytree
{
	wtk_spnode_t *root;
};

/**
 * @brief create splay tree;
 */
wtk_splaytree_t* wtk_splaytree_new();

/**
 * @brief init splay tree;
 */
int wtk_splaytree_init(wtk_splaytree_t* t);//,dispose_handler dispose,void *data);

/**
 *	@brief resplay the tree by cmp;
 */
wtk_spnode_t* wtk_splaytree_splay(wtk_splaytree_t *t,wtk_cmp_handler_t cmp,void *value,int *cmp_ret);

/**
 *	@brief find node by cmp;
 */
wtk_spnode_t* wtk_splaytree_find_node(wtk_splaytree_t *t,wtk_cmp_handler_t cmp,void *value);

/**
 *	@brief find node value by cmp;
 */
void* wtk_splaytree_find(wtk_splaytree_t *t,wtk_cmp_handler_t cmp,void *value);

/**
 *	@brief insert node and keep the tree order;
 */
int wtk_splaytree_insert(wtk_splaytree_t *t, wtk_cmp_handler_t cmp,void *cmp_param,wtk_spnode_t *node);

/**
 *	@brief remove node by cmp;
 */
wtk_spnode_t* wtk_splaytree_remove(wtk_splaytree_t* tree,wtk_cmp_handler_t cmp,void *value);

/**
 *	@walk the tree;
 */
int wtk_splaytree_walk(wtk_splaytree_t* t,wtk_walk_handler_t walk,void *user_data);
#ifdef __cplusplus
};
#endif
#endif
