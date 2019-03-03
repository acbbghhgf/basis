/*
 * traditional
 * ================================================
 * Color = Red | Black
 * rbtree a = {
 *          Color
 *          rbtree a
 *          a
 *          rbtree a
 *       } | LEAF 
 * }
 */
#ifndef QTK_CORE_QTK_RBTREE_H
#define QTK_CORE_QTK_RBTREE_H

#include    "qtk_usr_data.h"

#ifdef __cplusplus
extern "C" {
#endif

#define qtk_rbtree_leaf     (NULL)
#define qtk_rbtree_empty    (NULL)
#define qtk_rbtree_BBempty  ((void *)1)
#define qtk_rbtree_node_empty    (NULL)

#define qtk_rbtree_isempty(t)   (!(t))
#define qtk_rbtree_isbbempty(t) ((t) == (void *)1)
#define qtk_rbtree_node_isempty(b)    (!(b))

#define qtk_rbtree_left_branch(t)   ((t)->left)
#define qtk_rbtree_right_branch(t)  ((t)->right) 

typedef struct qtk_rbtree qtk_rbtree_t;
typedef struct qtk_Rbtree qtk_Rbtree_t;
typedef struct qtk_rbtree qtk_Rbtree_node_t;
typedef struct qtk_rbtree qtk_rbtree_node_t;

typedef int (*qtk_Rbtree_f)(qtk_rbtree_t *, qtk_rbtree_t *);
typedef int (*qtk_rbtree_node_cmp_f)(qtk_rbtree_node_t *, void *);

typedef enum{
    R,
    B,
    BB 
}Color;

struct qtk_rbtree{
    Color   color;
    qtk_rbtree_t    *left;
    qtk_rbtree_t    *right;
};

struct qtk_Rbtree{
    qtk_rbtree_t    *tree;
    qtk_rbtree_node_cmp_f   cmp;
};

static inline qtk_rbtree_t  *
qtk_rbtree_T(Color color, qtk_rbtree_t *left, qtk_rbtree_node_t *node, qtk_rbtree_t *right)
{
    node->color = color;
    node->left = left;
    node->right = right;
    return node;
}

int    qtk_Rbtree_delete(qtk_Rbtree_t *t);
qtk_Rbtree_t    *qtk_Rbtree_new(qtk_rbtree_node_cmp_f cmp);

int qtk_Rbtree_insert(qtk_Rbtree_t *t, qtk_Rbtree_node_t *node);
qtk_Rbtree_node_t   *qtk_Rbtree_elem(qtk_Rbtree_t *t, qtk_Rbtree_node_t *elem);
qtk_Rbtree_node_t   *qtk_Rbtree_remove(qtk_Rbtree_t *t, qtk_Rbtree_node_t *node);
void    *qtk_Rbtree_fold(qtk_Rbtree_t *t, void *init, qtk_Rbtree_f f);
void    *qtk_Rbtree_foldl(qtk_Rbtree_t *t, void *init, qtk_Rbtree_f f);
void    *qtk_Rbtree_foldr(qtk_Rbtree_t *t, void *init, qtk_Rbtree_f f);

#ifdef __cplusplus
};
#endif

#endif
