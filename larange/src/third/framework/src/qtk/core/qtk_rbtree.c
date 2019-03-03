#include    "qtk/core/qtk_rbtree.h"

static qtk_rbtree_t *qtk_rbtree_makeblack(qtk_rbtree_t *t);
static qtk_rbtree_t *qtk_rbtree_makeblack1(qtk_rbtree_t *t);

static qtk_rbtree_node_t    *qtk_rbtree_min(qtk_rbtree_t *t);

static void *qtk_rbtree_fold(qtk_rbtree_t *t, void *init, qtk_Rbtree_f f);
static void *qtk_rbtree_foldl(qtk_rbtree_t *t, void *init, qtk_Rbtree_f f);
static void *qtk_rbtree_foldr(qtk_rbtree_t *t, void *init, qtk_Rbtree_f f);

static qtk_rbtree_t *qtk_rbtree_fixdb(Color color, qtk_rbtree_t *l, qtk_rbtree_node_t *a, qtk_rbtree_t *r);
static qtk_rbtree_t *qtk_rbtree_balance(Color color, qtk_rbtree_t *l, qtk_rbtree_node_t *a, qtk_rbtree_t *r);

static qtk_rbtree_t *qtk_rbtree_elem(qtk_rbtree_t *t, qtk_rbtree_t *elem, qtk_rbtree_node_cmp_f cmp);
static qtk_rbtree_t *qtk_rbtree_ins(qtk_rbtree_node_t *a, qtk_rbtree_t *t, qtk_rbtree_node_cmp_f cmp);
static qtk_rbtree_t *qtk_rbtree_insert(qtk_rbtree_node_t *a, qtk_rbtree_t *t, qtk_rbtree_node_cmp_f cmp);
static qtk_rbtree_t *qtk_rbtree_del(qtk_rbtree_node_t *a, qtk_rbtree_t *t, qtk_rbtree_node_cmp_f cmp, qtk_rbtree_node_t **rp);
static qtk_rbtree_t *qtk_rbtree_delete(qtk_rbtree_node_t *a, qtk_rbtree_t *t, qtk_rbtree_node_cmp_f cmp, qtk_rbtree_node_t **rp);

qtk_Rbtree_t    *
qtk_Rbtree_new(qtk_rbtree_node_cmp_f cmp)
{
    qtk_Rbtree_t *t;

    if(!cmp || !(t = calloc(1, sizeof(*t))))
        return NULL;
    t->cmp = cmp;
    t->tree = qtk_rbtree_empty;
    return t;
}

int
qtk_Rbtree_delete(qtk_Rbtree_t *t)
{
    free(t);
    return 0;
}

qtk_Rbtree_node_t   *
qtk_Rbtree_remove(qtk_Rbtree_t *t, qtk_Rbtree_node_t *node)
{
    qtk_Rbtree_node_t   *r;
    
    r = NULL;
    t->tree = qtk_rbtree_delete(node, t->tree, t->cmp, &r);
    return r;
}

int
qtk_Rbtree_insert(qtk_Rbtree_t *t, qtk_Rbtree_node_t *node)
{
    t->tree = qtk_rbtree_insert(node, t->tree, t->cmp);
    return 0;
}

qtk_Rbtree_node_t    *
qtk_Rbtree_elem(qtk_Rbtree_t *t, qtk_Rbtree_node_t *elem)
{
    return qtk_rbtree_elem(t->tree, elem, t->cmp);
}

void    *
qtk_Rbtree_fold(qtk_Rbtree_t *t, void *init, qtk_Rbtree_f f)
{
    return qtk_rbtree_fold(t->tree, init, f);
}

void    *
qtk_Rbtree_foldl(qtk_Rbtree_t *t, void *init, qtk_Rbtree_f f)
{
    return qtk_rbtree_foldl(t->tree, init, f);
}

void    *
qtk_Rbtree_foldr(qtk_Rbtree_t *t, void *init, qtk_Rbtree_f f)
{
    return qtk_rbtree_foldr(t->tree, init, f);
}

static qtk_rbtree_node_t    *
qtk_rbtree_min(qtk_rbtree_t *t)
{
    if(qtk_rbtree_isempty(t) || qtk_rbtree_isempty(t->left))
        return t;
    return qtk_rbtree_min(t->left);
}

static qtk_rbtree_node_t    *
qtk_rbtree_elem(qtk_rbtree_t *t, qtk_rbtree_node_t *elem, qtk_rbtree_node_cmp_f cmp)
{
    if(!qtk_rbtree_isempty(t)){
        int r = cmp(elem, t);
        if(r == 0)
            return t;
        if(r < 0)
            return qtk_rbtree_elem(qtk_rbtree_left_branch(t), elem, cmp);
        if(r > 0)
            return qtk_rbtree_elem(qtk_rbtree_right_branch(t), elem, cmp);
    }
    return NULL;
}

static void *
qtk_rbtree_fold(qtk_rbtree_t *t, void *init, qtk_Rbtree_f f)
{
    if(!qtk_rbtree_isempty(t)){
        qtk_rbtree_t    *l, *r;
        l = qtk_rbtree_left_branch(t);
        r = qtk_rbtree_right_branch(t);
        f(t, init);
        qtk_rbtree_fold(l, init, f);
        qtk_rbtree_fold(r, init, f);
    }
    return init;
}

static void *
qtk_rbtree_foldl(qtk_rbtree_t *t, void *init, qtk_Rbtree_f f)
{
    if(!qtk_rbtree_isempty(t)){
        qtk_rbtree_t    *l, *r;
        l = qtk_rbtree_left_branch(t);
        r = qtk_rbtree_right_branch(t);
        qtk_rbtree_foldl(l, init, f);
        f(t, init);
        qtk_rbtree_foldl(r, init, f);
    }
    return init;
}

static void *
qtk_rbtree_foldr(qtk_rbtree_t *t, void *init, qtk_Rbtree_f f)
{
    if(!qtk_rbtree_isempty(t)){
        qtk_rbtree_t    *l, *r;
        l = qtk_rbtree_left_branch(t);
        r = qtk_rbtree_right_branch(t);
        qtk_rbtree_foldr(r, init, f);
        f(t, init);
        qtk_rbtree_foldr(l, init, f);
    }
    return init;
}

static qtk_rbtree_t *
qtk_rbtree_makeblack(qtk_rbtree_t *t)
{
    if(qtk_rbtree_isempty(t) || qtk_rbtree_isbbempty(t))
        return NULL;
    t->color = B;
    return t;
}

static qtk_rbtree_t *
qtk_rbtree_makeblack1(qtk_rbtree_t *t)
{
    if(qtk_rbtree_isempty(t))
        return qtk_rbtree_BBempty;
    t->color = (t->color == B) ? BB : B;
    return t;
}

static qtk_rbtree_t    *
qtk_rbtree_insert(qtk_rbtree_node_t *a, qtk_rbtree_t *t, qtk_rbtree_node_cmp_f cmp)
{
    return qtk_rbtree_makeblack(qtk_rbtree_ins(a, t, cmp));
}

static qtk_rbtree_t *
qtk_rbtree_ins(qtk_rbtree_node_t *a, qtk_rbtree_t *t, qtk_rbtree_node_cmp_f cmp)
{
    int r;

    if(qtk_rbtree_isempty(t))
        return qtk_rbtree_T(R, qtk_rbtree_empty, a, qtk_rbtree_empty);
    r = cmp(a, t);
    if(r < 0)
        return qtk_rbtree_balance(t->color, qtk_rbtree_ins(a, t->left, cmp), t, t->right);
    if(r > 0)
        return qtk_rbtree_balance(t->color, t->left, t, qtk_rbtree_ins(a, t->right, cmp));
    return t;
}

static qtk_rbtree_t *
qtk_rbtree_balance(Color color, qtk_rbtree_t *l, qtk_rbtree_node_t *a, qtk_rbtree_t *r)
{
    qtk_rbtree_t    *t;

    if(color == B){
        if(l && l->color == R && (t = l->left) && t->color == R)
            return qtk_rbtree_T(R, qtk_rbtree_makeblack(l->left), l, qtk_rbtree_T(B, l->right, a, r));
        if(l && l->color == R && (t = l->right) && t->color == R)
            return qtk_rbtree_T(R, qtk_rbtree_T(B, l->left, l, t->left), t, qtk_rbtree_T(B, t->right, a, r));
        if(r && r->color == R && (t = r->right) && t->color == R)
            return qtk_rbtree_T(R, qtk_rbtree_T(B, l, a, r->left), r, qtk_rbtree_makeblack(r->right));
        if(r && r->color == R && (t = r->left) && t->color == R)
            return qtk_rbtree_T(R, qtk_rbtree_T(B, l, a, t->left), t, qtk_rbtree_T(B, t->right, r, r->right));
    }
    return qtk_rbtree_T(color, l, a, r);
}

static qtk_rbtree_t    *
qtk_rbtree_delete(qtk_rbtree_node_t *a, qtk_rbtree_t *t, qtk_rbtree_node_cmp_f cmp, qtk_rbtree_node_t **rp)
{
    return qtk_rbtree_makeblack(qtk_rbtree_del(a, t, cmp, rp));
}

static qtk_rbtree_t    *
qtk_rbtree_del(qtk_rbtree_node_t *a, qtk_rbtree_t *t, qtk_rbtree_node_cmp_f cmp, qtk_rbtree_node_t **rp)
{
    int r;
    qtk_rbtree_node_t   *min;

    if(qtk_rbtree_isempty(t))
        return t;
    r = cmp(a, t);
    if(r < 0)
        return qtk_rbtree_fixdb(t->color, qtk_rbtree_del(a, t->left, cmp, rp), t, t->right);
    if(r > 0)
        return qtk_rbtree_fixdb(t->color, t->left, t, qtk_rbtree_del(a, t->right, cmp, rp));
    if(rp)
        *rp = t;
    if(qtk_rbtree_isempty(t->left))
        return t->color == B ? qtk_rbtree_makeblack1(t->right) : t->right;
    if(qtk_rbtree_isempty(t->right))
        return t->color == B ? qtk_rbtree_makeblack1(t->left) : t->left;
    min = qtk_rbtree_min(t->right);
    return qtk_rbtree_fixdb(t->color, t->left, min, qtk_rbtree_del(min, t->right, cmp, NULL));
}

static qtk_rbtree_t *
qtk_rbtree_fixdb(Color color, qtk_rbtree_t *l, qtk_rbtree_node_t *a, qtk_rbtree_t *r)
{
    qtk_rbtree_t    *t, *t1;

    if(qtk_rbtree_isbbempty(l) && qtk_rbtree_isempty(r))
        return qtk_rbtree_T(BB, qtk_rbtree_empty, a, qtk_rbtree_empty);
    if(qtk_rbtree_isempty(l) && qtk_rbtree_isbbempty(r))
        return qtk_rbtree_T(BB, qtk_rbtree_empty, a, qtk_rbtree_empty);
    if(qtk_rbtree_isbbempty(l))
        return qtk_rbtree_T(color, qtk_rbtree_empty, a, r);
    if(qtk_rbtree_isbbempty(r))
        return qtk_rbtree_T(color, l, a, qtk_rbtree_empty);
    if(l && l->color == BB && r && r->color == B && (t = r->left) && t->color == R)
        return qtk_rbtree_T(color, qtk_rbtree_T(B, qtk_rbtree_makeblack1(l), a, t->left), t, qtk_rbtree_T(B, t->right, r, r->right));
    if(l && l->color == BB && r && r->color == B && (t = r->right) && t->color == R)
        return qtk_rbtree_T(color, qtk_rbtree_T(B, qtk_rbtree_makeblack1(l), a, r->left), r, qtk_rbtree_T(B, t->left, t, t->right));
    if(r && r->color == BB && l && l->color == B && (t = l->right) && t->color == R)
        return qtk_rbtree_T(color, qtk_rbtree_T(B, l->left, l, t->left), t, qtk_rbtree_T(B, t->right, a, qtk_rbtree_makeblack1(r)));
    if(r && r->color == BB && l && l->color == B && (t = l->left) && t->color == R)
        return qtk_rbtree_T(color, qtk_rbtree_T(B, t->left, t, t->right), l, qtk_rbtree_T(B, l->right, a, qtk_rbtree_makeblack1(r)));
    if(l && l->color == BB && r && r->color == B && (t = r->left) && t->color == B && (t1 = r->right) && t1->color == B)
        return qtk_rbtree_makeblack1(qtk_rbtree_T(color, qtk_rbtree_makeblack1(l), a, qtk_rbtree_T(R, t, r, t1)));
    if(r && r->color == BB && l && l->color == B && (t = l->left) && t->color == B && (t1 = l->right) && t1->color == B)
        return qtk_rbtree_makeblack1(qtk_rbtree_T(color, qtk_rbtree_T(R, t, l, t1), a, qtk_rbtree_makeblack1(r)));
    if(color == B && l && l->color == BB && r && r->color == R)
        return qtk_rbtree_fixdb(B, qtk_rbtree_fixdb(R, l, a, r->left), r, r->right);
    if(color == B && r && r->color == BB && l && l->color == R)
        return qtk_rbtree_fixdb(B, l->left, l, qtk_rbtree_fixdb(R, l->right, a, r));
    return qtk_rbtree_T(color, l, a, r);
}
