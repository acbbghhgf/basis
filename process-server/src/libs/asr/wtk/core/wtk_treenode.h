#ifndef WTK_CORE_WTK_TREENODE_H_
#define WTK_CORE_WTK_TREENODE_H_
#include "wtk/core/wtk_type.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_treenode wtk_treenode_t;

struct wtk_treenode
{
	wtk_treenode_t *left;
	wtk_treenode_t *right;
	wtk_treenode_t *parent;
};
#define wtk_treenode_left(t) ((t)->left)
#define wtk_treenode_right(t) ((t)->right)
#define wtk_treenode_parent(t) ((t)->parent)

typedef void (*WtkTreeNodePrintFunc)(wtk_treenode_t *n);
typedef int (*WtkTreeNodeTraverseFunc)(void *data,wtk_treenode_t *n);

/**
 * @brief return left node of t.
 */
wtk_treenode_t* wtk_treenode_min(wtk_treenode_t *t);

/**
 * @brief return right node of t.
 */
wtk_treenode_t* wtk_treenode_max(wtk_treenode_t *t);

/**
 * @brief traverse tree node.
 */
int wtk_treenode_traverse(wtk_treenode_t *t,WtkTreeNodeTraverseFunc traverse,void *data);

/**
 * @brief print node.
 */
void wtk_treenode_print(wtk_treenode_t *n,int depth,WtkTreeNodePrintFunc print);

/**
 * @brief get max depth of the tree.
 */
int wtk_treenode_depth(wtk_treenode_t* n,int depth);

int wtk_treenode_len(wtk_treenode_t *n);
#ifdef __cplusplus
};
#endif
#endif
