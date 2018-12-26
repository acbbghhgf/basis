#include "wtk_lex_net.h"
void wtk_lex_net_create(wtk_lex_net_t *net, wtk_lex_script_t *script);
wtk_lex_node_t* wtk_lext_net_expand_expr_or_q(wtk_lex_net_t *net,
        wtk_lex_node_t *s, wtk_queue_t *q);
wtk_lex_node_t* wtk_lext_net_expand_expr_item(wtk_lex_net_t *net,
        wtk_lex_node_t *s, wtk_lex_expr_item_t *item);

wtk_lex_net_t* wtk_lex_net_new(wtk_lex_script_t *script)
{
    wtk_lex_net_t *net;

    net = (wtk_lex_net_t*) wtk_malloc(sizeof(wtk_lex_net_t));
    net->script = script;
    net->heap = wtk_heap_new(4096);
    net->nroot = 0;
    net->roots = NULL;
    net->use_poseg = 0;
    wtk_lex_net_create(net, script);
    return net;
}

void wtk_lex_net_delete(wtk_lex_net_t *net)
{
    if (net->script->hide) {
        wtk_lex_script_delete(net->script);
    }
    wtk_heap_delete(net->heap);
    wtk_free(net);
}

wtk_lex_node_t* wtk_lex_net_new_node(wtk_lex_net_t *net)
{
    wtk_lex_node_t *node;

    node = (wtk_lex_node_t*) wtk_heap_malloc(net->heap, sizeof(wtk_lex_node_t));
    wtk_queue2_init(&(node->in_q));
    wtk_queue2_init(&(node->out_q));
    node->complex_input_arc = NULL;
    return node;
}

wtk_lex_net_scope_t* wtk_lex_net_new_scope(wtk_lex_net_t *net,
        wtk_lex_expr_scope_t *scope)
{
    wtk_lex_net_scope_t *s;

    s = (wtk_lex_net_scope_t*) wtk_heap_malloc(net->heap,
            sizeof(wtk_lex_net_scope_t));
    s->nyes = scope->yes.length;
    s->nno = scope->no.length;
    s->yes = (wtk_lex_net_root_t**) wtk_heap_malloc(net->heap,
            sizeof(wtk_lex_net_root_t*) * s->nyes);
    s->no = (wtk_lex_net_root_t**) wtk_heap_malloc(net->heap,
            sizeof(wtk_lex_net_root_t*) * s->nno);
    return s;
}

wtk_lex_arc_t* wtk_lex_net_new_arc(wtk_lex_net_t *net, wtk_lex_node_t *from,
        wtk_lex_node_t *to, wtk_lex_expr_item_value_t *value)
{
    wtk_lex_arc_t *arc;

    arc = (wtk_lex_arc_t*) wtk_heap_malloc(net->heap, sizeof(wtk_lex_arc_t));
    arc->from = from;
    arc->to = to;
    arc->value = value;
    wtk_queue2_push(&(from->out_q), &(arc->out_n));
    wtk_queue2_push(&(to->in_q), &(arc->in_n));

    return arc;
}

wtk_lex_node_t* wtk_lext_net_expand_expr_value_q(wtk_lex_net_t *net,
        wtk_lex_node_t *s, wtk_queue_t *q)
{
    wtk_queue_node_t *qn;
    wtk_lex_expr_item_value_t *v;
    wtk_lex_node_t *e;
    wtk_lex_arc_t *arc;

    for (qn = q->pop; qn; qn = qn->next) {
        v = data_offset2(qn, wtk_lex_expr_item_value_t, q_n);
        switch (v->type) {
            case WTK_LEX_VALUE_STRING:
            case WTK_LEX_VALUE_BRACKET:
            case WTK_LEX_VALUE_PERIOD:
            case WTK_LEX_VALUE_RANGE:
            case WTK_LEX_VALUE_ESCAPE:
            case WTK_LEX_VALUE_BUILDIN_VAR:
                e = wtk_lex_net_new_node(net);
                wtk_lex_net_new_arc(net, s, e, v);
                s = e;
                break;
            case WTK_LEX_VALUE_PARENTHESES:
                e = wtk_lex_net_new_node(net);
                arc = wtk_lex_net_new_arc(net, s, e, v);
                s = e;
                if (v->v.parentheses->or_q.length > 0) {
                    s = wtk_lext_net_expand_expr_or_q(net, s,
                            &(v->v.parentheses->or_q));
                } else {
                    s = wtk_lext_net_expand_expr_value_q(net, s,
                            &(v->v.parentheses->value_q));
                    e = wtk_lex_net_new_node(net);
                    wtk_lex_net_new_arc(net, s, e, NULL);
                    s = e;
                }
                s->complex_input_arc = arc;
                break;
            case WTK_LEX_VALUE_EXPR:
                e = wtk_lex_net_new_node(net);
                arc = wtk_lex_net_new_arc(net, s, e, v);
                s = e;
                s = wtk_lext_net_expand_expr_item(net, s, v->v.expr);
                e = wtk_lex_net_new_node(net);
                wtk_lex_net_new_arc(net, s, e, NULL);
                s = e;
                s->complex_input_arc = arc;
                break;
        }
    }
    return s;
}

wtk_lex_node_t* wtk_lext_net_expand_expr_or_q(wtk_lex_net_t *net,
        wtk_lex_node_t *s, wtk_queue_t *q)
{
    wtk_lex_expr_branch_t *b;
    wtk_queue_node_t *qn;
    wtk_lex_node_t *e, *n;

    e = wtk_lex_net_new_node(net);
    for (qn = q->pop; qn; qn = qn->next) {
        b = data_offset2(qn, wtk_lex_expr_branch_t, q_n);
        n = wtk_lext_net_expand_expr_value_q(net, s, &(b->value_q));
        wtk_lex_net_new_arc(net, n, e, NULL);
    }
    return e;
}

wtk_lex_node_t* wtk_lext_net_expand_expr_item(wtk_lex_net_t *net,
        wtk_lex_node_t *s, wtk_lex_expr_item_t *item)
{
    wtk_lex_node_t *node; //,*e;

    if (item->or_q.length > 0) {
        node = wtk_lext_net_expand_expr_or_q(net, s, &(item->or_q));
    } else {
        node = wtk_lext_net_expand_expr_value_q(net, s, &(item->value_q));
    }
//	if(node)
//	{
//		e=wtk_lex_net_new_node(net);
//		wtk_lex_net_new_arc(net,node,e,NULL);
//		node=e;
//	}
    return node;
}

wtk_lex_net_root_t* wtk_lex_net_new_root(wtk_lex_net_t *net,
        wtk_lex_expr_t *expr)
{
    wtk_lex_net_root_t *r;
    wtk_queue_node_t *qn;
    wtk_lex_expr_t *expr2;
    int i;

    r = (wtk_lex_net_root_t*) wtk_heap_malloc(net->heap,
            sizeof(wtk_lex_net_root_t));
    r->expr = expr;
    switch (expr->type) {
        case WTK_LEX_EXPR_ITEM:
            r->v.node = wtk_lex_net_new_node(net);
            if (!net->use_poseg && expr->v.expr->attr.use_seg) {
                net->use_poseg = 1;
            }
            wtk_lext_net_expand_expr_item(net, r->v.node, expr->v.expr);
            break;
        case WTK_LEX_EXPR_SCOPE:
            r->v.scope = wtk_lex_net_new_scope(net, expr->v.scope);
            for (i = 0, qn = expr->v.scope->yes.pop; qn; qn = qn->next, ++i) {
                expr2 = data_offset2(qn, wtk_lex_expr_t, q_n);
                r->v.scope->yes[i] = wtk_lex_net_new_root(net, expr2);
            }
            for (i = 0, qn = expr->v.scope->no.pop; qn; qn = qn->next, ++i) {
                expr2 = data_offset2(qn, wtk_lex_expr_t, q_n);
                r->v.scope->no[i] = wtk_lex_net_new_root(net, expr2);
            }
            break;
        case WTK_LEX_EXPR_CMD:
            break;
    }
    return r;
}

int wtk_lex_node_out_arc_can_reach(wtk_lex_node_t *n, wtk_lex_node_t *node)
{
    wtk_queue_node_t *qn;
    wtk_lex_arc_t *arc;
    int b;

    for (qn = n->out_q.pop; qn; qn = qn->next) {
        arc = data_offset2(qn, wtk_lex_arc_t, out_n);
        b = 0;
        while (1) {
            if (arc->to == node) {
                b = 1;
                break;
            }
            if (!arc->to->out_q.pop) {
                break;
            }
            arc = data_offset2(arc->to->out_q.pop, wtk_lex_arc_t, out_n);
        }
        if (b == 0) {
            return 0;
        }
    }
    return 1;
}

wtk_lex_node_t* wtk_lex_node_next_merge_node(wtk_lex_node_t *n)
{
    wtk_lex_arc_t *arc;
    wtk_lex_node_t *t;
    int b;

    t = n;
    while (t->out_q.pop) {
        arc = data_offset2(t->out_q.pop, wtk_lex_arc_t, out_n);
        b = wtk_lex_node_out_arc_can_reach(n, arc->to);
        if (b) {
            return arc->to;
        }
        t = arc->to;
    }
    return NULL;
}

void wtk_lex_node_print3(wtk_lex_node_t *n)
{
    wtk_queue_node_t *qn;
    wtk_lex_arc_t *arc;

    if (n->out_q.pop) {
        for (qn = n->out_q.pop; qn; qn = qn->next) {
            arc = data_offset2(qn, wtk_lex_arc_t, out_n);
            if (qn != n->out_q.pop) {
                printf("|");
            }
            if (arc->value) {
                if (wtk_lex_expr_item_value_is_atom(arc->value)) {
                    wtk_lex_expr_item_value_print(arc->value);
                } else {
                    switch (arc->value->type) {
                        case WTK_LEX_VALUE_PARENTHESES:
                            printf("(");
                            break;
                        case WTK_LEX_VALUE_EXPR:
                            break;
                        default:
                            break;
                    }
                }
            }
            wtk_lex_node_print3(arc->to);
        }
    } else {
        printf("\n");
    }
}

void wtk_lex_node_print2(wtk_lex_node_t *n, wtk_lex_node_t *stop)
{
    wtk_queue_node_t *qn;
    wtk_lex_arc_t *arc;
    wtk_lex_node_t *merge;

//	if(n->complex_input_arc)
//	{
//		wtk_debug("found node(%p,%p)\n",n,stop);
//	}
    if (n == stop) {
        return;
    }
    if (n->out_q.pop) {
        if (wtk_queue2_len(&(n->out_q)) > 1) {
            merge = wtk_lex_node_next_merge_node(n);
            //merge=NULL;
        } else {
            merge = NULL;
        }
        for (qn = n->out_q.pop; qn; qn = qn->next) {
            arc = data_offset2(qn, wtk_lex_arc_t, out_n);
            if (qn != n->out_q.pop) {
                printf("|");
            }
            if (arc->value) {
                if (wtk_lex_expr_item_value_is_atom(arc->value)) {
                    wtk_lex_expr_item_value_print(arc->value);
                } else {
                    switch (arc->value->type) {
                        case WTK_LEX_VALUE_PARENTHESES:
                            printf("(");
                            break;
                        case WTK_LEX_VALUE_EXPR:
                            break;
                        default:
                            break;
                    }
                }
            }
            if (merge) {
                wtk_lex_node_print2(arc->to, merge);
            } else {
                if (!stop && arc->to->complex_input_arc
                        && arc->to->complex_input_arc->value->type
                                == WTK_LEX_VALUE_PARENTHESES) {
                    printf(")");
                }
                wtk_lex_node_print2(arc->to, stop);
            }
        }
        if (merge) {
            if (merge->complex_input_arc
                    && merge->complex_input_arc->value->type
                            == WTK_LEX_VALUE_PARENTHESES) {
                printf(")");
            }
            wtk_lex_node_print2(merge, stop);
        }
    } else {
        printf("\n");
    }
}

void wtk_lex_node_print(wtk_lex_node_t *n)
{
    wtk_lex_node_print2(n, NULL);
}

void wtk_lex_net_root_print(wtk_lex_net_root_t *root)
{
    int i;

    if (root->expr->type == WTK_LEX_EXPR_ITEM) {
        wtk_lex_node_print(root->v.node);
    } else {
        printf("if ... then\n");
        for (i = 0; i < root->v.scope->nyes; ++i) {
            wtk_lex_net_root_print(root->v.scope->yes[i]);
        }
        if (root->v.scope->nno > 0) {
            printf("else \n");
            for (i = 0; i < root->v.scope->nno; ++i) {
                wtk_lex_net_root_print(root->v.scope->no[i]);
            }
        }
        printf("end\n");
    }
    //wtk_lex_node_print2(root->v.node);
}

void wtk_lex_net_create(wtk_lex_net_t *net, wtk_lex_script_t *script)
{
    wtk_heap_t *heap = net->heap;
    wtk_queue_node_t *qn;
    wtk_lex_expr_t *expr;
    int i;

    net->nroot = script->main_lib->expr_q.length;
    net->roots = (wtk_lex_net_root_t**) wtk_heap_malloc(heap,
            sizeof(wtk_lex_net_root_t*) * net->nroot);
    for (i = 0, qn = script->main_lib->expr_q.pop; qn; qn = qn->next, ++i) {
        expr = data_offset2(qn, wtk_lex_expr_t, q_n);
        //wtk_lex_script_print_expr(expr);
        net->roots[i] = wtk_lex_net_new_root(net, expr);
        //exit(0);
        //wtk_lex_net_root_print(net->roots[i]);
        //exit(0);
    }
}

