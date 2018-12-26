#ifndef WTK_CORE_WTK_RBTREE_H_
#define WTK_CORE_WTK_RBTREE_H_
#include "wtk/core/wtk_type.h"
#include "wtk_alloc.h"
#include "wtk_treenode.h"
#ifdef __cplusplus
extern "C" {
#endif
#define RED 1
#define BLACK 0

typedef struct wtk_rbnode wtk_rbnode_t;
typedef double wtk_rbnode_key_t;

struct wtk_rbnode
{
	wtk_rbnode_t *left;
	wtk_rbnode_t *right;
	wtk_rbnode_t *parent;
	wtk_rbnode_key_t key;
	unsigned	color:1;
};

#define wtk_rbnode_red(n) ((n) ? ((n)->color=RED):0)
#define wtk_rbnode_black(n) ( (n) ? ((n)->color=BLACK):0)
#define wtk_rbnode_is_red(n) ( (n) ? (n)->color : 0 )
#define wtk_rbnode_is_black(n) (!wtk_rbnode_is_red(n))

typedef struct wtk_rbtree wtk_rbtree_t;
struct wtk_rbtree
{
	wtk_rbnode_t	*root;
	unsigned long len;
};

/**
 *	@brief create red-black tree;
 */
wtk_rbtree_t* wtk_rbtree_new();

/**
 * @brief initialize reb-black tree;
 */
void wtk_rbtree_init(wtk_rbtree_t *t);

/**
 * @brief delete red-black tree;
 */
int wtk_rbtree_delete(wtk_rbtree_t* t);

/**
 * @brief insert new node and sort;
 */
void wtk_rbtree_insert(wtk_rbtree_t *t,wtk_rbnode_t *n);

/**
 * @brief remove node;
 */
void wtk_rbtree_remove(wtk_rbtree_t *t,wtk_rbnode_t *n);

/**
 * @brief find node by key;
 */
wtk_rbnode_t* wtk_rbtree_find(wtk_rbtree_t *t,wtk_rbnode_key_t key);

/**
 *	@brief check red-black tree is valid or not;
 */
int wtk_rbtree_is_valid(wtk_rbtree_t *t);

/**
 *	@brief print;
 */
void wtk_rbtree_print(wtk_rbtree_t *t);

/**
 * @brief depth of the tree;
 */
int wtk_rbtree_depth(wtk_rbtree_t *t);

wtk_rbnode_t* wtk_rbtree_find_like(wtk_rbtree_t *t,wtk_rbnode_key_t key);
#ifdef __cplusplus
};
#endif
#endif
