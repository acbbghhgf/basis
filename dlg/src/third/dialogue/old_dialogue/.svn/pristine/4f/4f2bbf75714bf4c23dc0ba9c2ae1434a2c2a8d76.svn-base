#include <ctype.h>
#include "wtk_lex_script.h" 

wtk_lex_script_lib_t* wtk_lex_script_new_lib(wtk_lex_script_t *s)
{
    wtk_lex_script_lib_t *lib;

    lib = (wtk_lex_script_lib_t*) wtk_heap_malloc(s->heap,
            sizeof(wtk_lex_script_lib_t));
    lib->name = NULL;
    wtk_queue_init(&(lib->expr_q));
    wtk_queue_init(&(lib->local_q));
    return lib;
}

wtk_lex_script_t* wtk_lex_script_new()
{
    wtk_lex_script_t *s;

    s = (wtk_lex_script_t*) wtk_malloc(sizeof(wtk_lex_script_t));
    s->heap = wtk_heap_new(4096);
    wtk_queue_init(&(s->lib_q));
    s->main_lib = wtk_lex_script_new_lib(s);
    s->cache_search = NULL;
    s->hide = 0;
    s->sort_by_prob = 0;
    s->use_fast_match = 0;
    s->use_eng_word = 0;
    s->use_act = 1;
    s->use_nbest = 1;
    s->get_expr = NULL;
    s->get_expr_ths = NULL;
    return s;
}

void wtk_lex_script_delete(wtk_lex_script_t *s)
{

    wtk_heap_delete(s->heap);
    wtk_free(s);
}

void wtk_lex_script_add_lib(wtk_lex_script_t *s, char *data, int bytes)
{
    wtk_lex_script_lib_t *lib;

    lib = wtk_lex_script_new_lib(s);
    lib->name = wtk_heap_dup_string(s->heap, data, bytes);
    wtk_queue_push(&(s->lib_q), &(lib->q_n));
}

wtk_lex_script_lib_t* wtk_lex_script_get_lib(wtk_lex_script_t *s,
        wtk_string_t *nm)
{
    wtk_queue_node_t *qn;
    wtk_lex_script_lib_t *lib;

    if (nm) {
        if (s->cache_search
                && wtk_string_cmp(nm, s->cache_search->name->data,
                        s->cache_search->name->len) == 0) {
            return s->cache_search;
        }
        for (qn = s->lib_q.pop; qn; qn = qn->next) {
            lib = data_offset2(qn, wtk_lex_script_lib_t, q_n);
            if (lib->name
                    && wtk_string_cmp(nm, lib->name->data, lib->name->len)
                            == 0) {
                s->cache_search = lib;
                return lib;
            }
        }
        return NULL;
    } else {
        return s->main_lib;
    }
}

void wtk_lex_expr_repeat_init(wtk_lex_expr_repeat_t *r)
{
    r->max_count = 1;
    r->min_count = 1;
    r->greedy = 1;
}

void wtk_lex_expr_item_attr_init(wtk_lex_expr_item_attr_t *attr)
{
    attr->output = NULL;
    attr->txt = WTK_LEX_NORMAL;
    //wtk_lex_expr_repeat_init(&(attr->repeat));
    attr->min_wrd_count = -1;
    attr->max_wrd_count = -1;
    attr->skipws = 0;
    attr->match_more_wrd = 1;
    attr->match_more_var = 1;
    attr->match_start = 0;
    attr->match_end = 0;
    attr->export = 0;
    attr->step_search = 0;
    attr->check_skip = 0;
    attr->use_seg = 0;
}

void wtk_lex_expr_output_init(wtk_lex_expr_output_t *out)
{
    out->name = NULL;
    wtk_queue_init(&(out->item_q));
}

wtk_lex_expr_item_t* wtk_lex_script_new_expr_item(wtk_lex_script_t *s,
        char *name, int bytes)
{
    wtk_lex_expr_item_t *item;

    item = (wtk_lex_expr_item_t*) wtk_heap_malloc(s->heap,
            sizeof(wtk_lex_expr_item_t));
    item->key.name = wtk_heap_dup_string(s->heap, name, bytes);
    wtk_lex_expr_item_attr_init(&(item->attr));
    wtk_queue_init(&(item->value_q));
    wtk_queue_init(&(item->or_q));
    wtk_lex_expr_output_init(&(item->output));
    item->prob = 1.0;
    return item;
}

wtk_lex_expr_t* wtk_lex_script_new_expr2(wtk_lex_script_t *s, char *name,
        int bytes)
{
    wtk_lex_expr_t *expr;

    expr = (wtk_lex_expr_t*) wtk_heap_malloc(s->heap, sizeof(wtk_lex_expr_t));
    expr->type = WTK_LEX_EXPR_ITEM;
    expr->v.expr = wtk_lex_script_new_expr_item(s, name, bytes);
    return expr;
}

wtk_lex_expr_t* wtk_lex_script_new_expr(wtk_lex_script_t *s,
        wtk_string_t *libname, char *name, int bytes, int expt)
{
    wtk_lex_script_lib_t *lib;
    wtk_lex_expr_t *expr;

    lib = wtk_lex_script_get_lib(s, libname);
    if (!lib) {
        return NULL;
    }
    expr = (wtk_lex_expr_t*) wtk_heap_malloc(s->heap, sizeof(wtk_lex_expr_t));
    expr->type = WTK_LEX_EXPR_ITEM;
    expr->v.expr = wtk_lex_script_new_expr_item(s, name, bytes);
    expr->v.expr->attr.export = expt;
    if (expt) {
        wtk_queue_push(&(lib->expr_q), &(expr->q_n));
    } else {
        wtk_queue_push(&(lib->local_q), &(expr->q_n));
    }
    return expr;
}

wtk_lex_expr_scope_t* wtk_lex_script_new_scope2(wtk_lex_script_t *l)
{
    wtk_lex_expr_scope_t *s;

    s = (wtk_lex_expr_scope_t*) wtk_heap_malloc(l->heap,
            sizeof(wtk_lex_expr_scope_t));
    wtk_queue_init(&(s->condition));
    wtk_queue_init(&(s->yes));
    wtk_queue_init(&(s->no));
    s->and = 1;
    return s;
}

wtk_lex_expr_if_t* wtk_lex_script_new_if(wtk_lex_script_t *s, char *data,
        int bytes, int n)
{
    wtk_lex_expr_if_t *i;
    char *p;

    i = (wtk_lex_expr_if_t*) wtk_heap_malloc(s->heap,
            sizeof(wtk_lex_expr_if_t));
    i->not = n;
    i->value = NULL;
    p = wtk_str_chr(data, bytes, '.');
    if (p && p != data) {
        i->var = wtk_heap_dup_string(s->heap, p + 1, data + bytes - 1 - p);
        i->act = wtk_heap_dup_string(s->heap, data, p - data);
        //wtk_debug("[%.*s] [%.*s]\n",i->act->len,i->act->data,i->var->len,i->var->data);
    } else {
        i->var = wtk_heap_dup_string(s->heap, data, bytes);
        i->act = NULL;
    }
    return i;
}

wtk_lex_expr_t* wtk_lex_script_new_scope3(wtk_lex_script_t *s)
{
    wtk_lex_expr_t *expr;

    expr = (wtk_lex_expr_t*) wtk_heap_malloc(s->heap, sizeof(wtk_lex_expr_t));
    expr->type = WTK_LEX_EXPR_SCOPE;
    expr->v.scope = wtk_lex_script_new_scope2(s);
    return expr;
}

wtk_lex_expr_t* wtk_lex_script_new_scope(wtk_lex_script_t *s,
        wtk_string_t *libname)
{
    wtk_lex_script_lib_t *lib;
    wtk_lex_expr_t *expr;

    lib = wtk_lex_script_get_lib(s, libname);
    if (!lib) {
        return NULL;
    }
    expr = (wtk_lex_expr_t*) wtk_heap_malloc(s->heap, sizeof(wtk_lex_expr_t));
    expr->type = WTK_LEX_EXPR_SCOPE;
    expr->v.scope = wtk_lex_script_new_scope2(s);
    wtk_queue_push(&(lib->expr_q), &(expr->q_n));
    return expr;
}

void wtk_lex_script_cmd_set_del(wtk_lex_script_t *s, wtk_lex_expr_cmd_t *cmd,
        char *data, int bytes)
{
    wtk_string_t *act, *var;
    char *p;

    p = wtk_str_chr(data, bytes, '.');
    if (p) {
        var = wtk_heap_dup_string(s->heap, p + 1, data + bytes - 1 - p);
        act = wtk_heap_dup_string(s->heap, data, p - data);
    } else {
        var = wtk_heap_dup_string(s->heap, data, bytes);
        act = NULL;
    }
    cmd->type = WTK_LEX_EXPR_CMD_DEL;
    cmd->op1_act = act;
    cmd->op1_var = var;
}

void wtk_lex_script_cmd_set_add(wtk_lex_script_t *s, wtk_lex_expr_cmd_t *cmd,
        char *data, int bytes)
{
    wtk_string_t *act, *var;
    char *p;

    p = wtk_str_chr(data, bytes, '.');
    if (p) {
        var = wtk_heap_dup_string(s->heap, p + 1, data + bytes - 1 - p);
        act = wtk_heap_dup_string(s->heap, data, p - data);
    } else {
        var = wtk_heap_dup_string(s->heap, data, bytes);
        act = NULL;
    }
    cmd->type = WTK_LEX_EXPR_CMD_ADD;
    cmd->op1_act = act;
    cmd->op1_var = var;
}

void wtk_lex_script_cmd_set_value(wtk_lex_script_t *s, wtk_lex_expr_cmd_t *cmd,
        char *data, int bytes)
{
    cmd->op2_var = wtk_heap_dup_string(s->heap, data, bytes);
}

void wtk_lex_script_cmd_set_cpy(wtk_lex_script_t *s, wtk_lex_expr_cmd_t *cmd,
        char *data, int bytes)
{
    wtk_string_t *act, *var;
    char *p;

    p = wtk_str_chr(data, bytes, '.');
    if (p) {
        var = wtk_heap_dup_string(s->heap, p + 1, data + bytes - 1 - p);
        act = wtk_heap_dup_string(s->heap, data, p - data);
    } else {
        var = wtk_heap_dup_string(s->heap, data, bytes);
        act = NULL;
    }
    cmd->type = WTK_LEX_EXPR_CMD_CPY;
    cmd->op1_act = act;
    cmd->op1_var = var;
}

void wtk_lex_script_cmd_set_cpy_value(wtk_lex_script_t *s,
        wtk_lex_expr_cmd_t *cmd, char *data, int bytes)
{
    wtk_string_t *act, *var;
    char *p;

    p = wtk_str_chr(data, bytes, '.');
    if (p) {
        var = wtk_heap_dup_string(s->heap, p + 1, data + bytes - 1 - p);
        act = wtk_heap_dup_string(s->heap, data, p - data);
    } else {
        var = wtk_heap_dup_string(s->heap, data, bytes);
        act = NULL;
    }
    cmd->op2_act = act;
    cmd->op2_var = var;
}

void wtk_lex_script_cmd_set_mv(wtk_lex_script_t *s, wtk_lex_expr_cmd_t *cmd,
        char *data, int bytes)
{
    wtk_string_t *act, *var;
    char *p;

    p = wtk_str_chr(data, bytes, '.');
    if (p) {
        var = wtk_heap_dup_string(s->heap, p + 1, data + bytes - 1 - p);
        act = wtk_heap_dup_string(s->heap, data, p - data);
    } else {
        var = wtk_heap_dup_string(s->heap, data, bytes);
        act = NULL;
    }
    cmd->type = WTK_LEX_EXPR_CMD_MV;
    cmd->op1_act = act;
    cmd->op1_var = var;
}

void wtk_lex_script_cmd_set_mv_value(wtk_lex_script_t *s,
        wtk_lex_expr_cmd_t *cmd, char *data, int bytes)
{
    wtk_string_t *act, *var;
    char *p;

    p = wtk_str_chr(data, bytes, '.');
    if (p) {
        var = wtk_heap_dup_string(s->heap, p + 1, data + bytes - 1 - p);
        act = wtk_heap_dup_string(s->heap, data, p - data);
    } else {
        var = wtk_heap_dup_string(s->heap, data, bytes);
        act = NULL;
    }
    cmd->op2_act = act;
    cmd->op2_var = var;
}

wtk_lex_expr_cmd_t* wtk_lex_script_new_cmd1(wtk_lex_script_t *s)
{
    wtk_lex_expr_cmd_t *cmd;

    cmd = (wtk_lex_expr_cmd_t*) wtk_heap_zalloc(s->heap,
            sizeof(wtk_lex_expr_cmd_t));
    return cmd;
}

wtk_lex_expr_t* wtk_lex_script_new_cmd2(wtk_lex_script_t *s)
{
    wtk_lex_expr_t *expr;

    expr = (wtk_lex_expr_t*) wtk_heap_malloc(s->heap, sizeof(wtk_lex_expr_t));
    expr->type = WTK_LEX_EXPR_CMD;
    expr->v.cmd = wtk_lex_script_new_cmd1(s);
    return expr;
}

wtk_lex_expr_t* wtk_lex_script_new_cmd(wtk_lex_script_t *s,
        wtk_string_t *libname)
{
    wtk_lex_script_lib_t *lib;
    wtk_lex_expr_t *expr;

    lib = wtk_lex_script_get_lib(s, libname);
    if (!lib) {
        return NULL;
    }
    expr = (wtk_lex_expr_t*) wtk_heap_malloc(s->heap, sizeof(wtk_lex_expr_t));
    expr->type = WTK_LEX_EXPR_CMD;
    expr->v.cmd = wtk_lex_script_new_cmd1(s);
    wtk_queue_push(&(lib->expr_q), &(expr->q_n));
    return expr;
}

wtk_lex_expr_t* wtk_lex_script_get_expr2(wtk_lex_script_t *s,
        wtk_string_t *libname, char *name, int bytes)
{
    wtk_lex_script_lib_t *lib;
    wtk_lex_expr_t *expr;
    wtk_queue_node_t *qn;

    lib = wtk_lex_script_get_lib(s, libname);
    if (!lib) {
        goto end;
    }
    for (qn = lib->local_q.pop; qn; qn = qn->next) {
        expr = data_offset2(qn, wtk_lex_expr_t, q_n);
        if (expr->type == WTK_LEX_EXPR_ITEM && expr->v.expr->key.name
                && wtk_string_cmp(expr->v.expr->key.name, name, bytes) == 0) {
            return expr;
        }
    }
    for (qn = lib->expr_q.pop; qn; qn = qn->next) {
        expr = data_offset2(qn, wtk_lex_expr_t, q_n);
        if (expr->type == WTK_LEX_EXPR_ITEM && expr->v.expr->key.name
                && wtk_string_cmp(expr->v.expr->key.name, name, bytes) == 0) {
            return expr;
        }
    }
    end: if (s->get_expr) {
        return s->get_expr(s->get_expr_ths, libname, name, bytes);
    }
    return NULL;
}

wtk_lex_expr_t* wtk_lex_script_get_expr(wtk_lex_script_t *s,
        wtk_string_t *libname, char *name, int bytes)
{
    wtk_lex_script_lib_t *lib;
    wtk_lex_expr_t *expr;
    wtk_queue_node_t *qn;

    lib = wtk_lex_script_get_lib(s, libname);
    if (!lib) {
        goto end;
    }
    for (qn = lib->local_q.push; qn; qn = qn->prev) {
        expr = data_offset2(qn, wtk_lex_expr_t, q_n);
        if (expr->type == WTK_LEX_EXPR_ITEM && expr->v.expr->key.name
                && wtk_string_cmp(expr->v.expr->key.name, name, bytes) == 0) {
            return expr;
        }
    }
    for (qn = lib->expr_q.push; qn; qn = qn->prev) {
        expr = data_offset2(qn, wtk_lex_expr_t, q_n);
        if (expr->type == WTK_LEX_EXPR_ITEM && expr->v.expr->key.name
                && wtk_string_cmp(expr->v.expr->key.name, name, bytes) == 0) {
            return expr;
        }
    }
    end:
    //wtk_debug("[%.*s] %p\n",bytes,name,s->get_expr);
    if (s->get_expr) {
        return s->get_expr(s->get_expr_ths, libname, name, bytes);
    }
    return NULL;
}

void wtk_lex_expr_item_value_attr_init(wtk_lex_expr_item_value_attr_t *attr)
{
    attr->txt = WTK_LEX_NORMAL;
    wtk_lex_expr_repeat_init(&(attr->repeat));
    attr->min_wrd_count = -1;
    attr->max_wrd_count = -1;
    attr->skip = NULL;
    attr->skipws = 0;
    attr->chn2num = 0;
    attr->match_weight = 1.0;
    attr->pre = NULL;
    attr->output = NULL;
    attr->pos = NULL;
    attr->not = NULL;
    attr->not_pre = NULL;
    attr->suc = NULL;
    attr->not_suc = NULL;
    attr->like = NULL;
    attr->replace = NULL;
    attr->ner = NULL;
    attr->use_py = 0;
}

wtk_lex_expr_item_value_t* wtk_lex_script_new_value(wtk_lex_script_t *s,
        wtk_lex_expr_item_value_type_t type)
{
    wtk_lex_expr_item_value_t *v;

    v = (wtk_lex_expr_item_value_t*) wtk_heap_malloc(s->heap,
            sizeof(wtk_lex_expr_item_value_t));
    v->type = type;
    wtk_lex_expr_item_value_attr_init(&(v->attr));
    return v;
}

wtk_lex_expr_item_value_t* wtk_lex_script_new_value_str(wtk_lex_script_t *s,
        char *data, int bytes)
{
    wtk_lex_expr_item_value_t *v;

    //wtk_debug("[%.*s]\n",bytes,data);
    v = wtk_lex_script_new_value(s, WTK_LEX_VALUE_STRING);
    v->v.str = wtk_heap_dup_string(s->heap, data, bytes);
    return v;
}

wtk_lex_expr_item_value_t* wtk_lex_script_new_value_dot(wtk_lex_script_t *s)
{
    wtk_lex_expr_item_value_t *v;

    v = wtk_lex_script_new_value(s, WTK_LEX_VALUE_PERIOD);
    return v;
}

wtk_lex_expr_item_value_t* wtk_lex_script_new_value_expr(wtk_lex_script_t *s,
        wtk_string_t *lib, char *name, int bytes)
{
    wtk_lex_expr_item_value_t *v;
    wtk_lex_expr_t *expr;
    wtk_string_t vx;
    char *p;

    if (name[0] == '.') {
        v = wtk_lex_script_new_value(s, WTK_LEX_VALUE_BUILDIN_VAR);
        v->v.buildin_var = wtk_heap_dup_string(s->heap, name + 1, bytes - 1);
    } else {
        p = wtk_str_chr(name, bytes, '.');
        if (p) {
            wtk_string_set(&(vx), name, p - name);
            lib = &(vx);
            bytes = name + bytes - p - 1;
            name = p + 1;
        }
        expr = wtk_lex_script_get_expr(s, lib, name, bytes);
        if (!expr) {
            return NULL;
        }
        v = wtk_lex_script_new_value(s, WTK_LEX_VALUE_EXPR);
        v->v.expr = expr->v.expr;
    }
    return v;
}

wtk_lex_expr_item_value_t* wtk_lex_script_new_parentheses(wtk_lex_script_t *s)
{
    wtk_lex_expr_item_value_t *v;

    v = wtk_lex_script_new_value(s, WTK_LEX_VALUE_PARENTHESES);
    v->v.parentheses = (wtk_lex_expr_parentheses_t*) wtk_heap_malloc(s->heap,
            sizeof(wtk_lex_expr_parentheses_t));
    wtk_queue_init(&(v->v.parentheses->value_q));
    wtk_queue_init(&(v->v.parentheses->or_q));
    v->v.parentheses->capture = 1;
    return v;
}

wtk_lex_expr_item_value_t* wtk_lex_script_new_bracket(wtk_lex_script_t *s)
{
    wtk_lex_expr_item_value_t *v;

    v = wtk_lex_script_new_value(s, WTK_LEX_VALUE_BRACKET);
    v->v.bracket = (wtk_lex_expr_bracket_t*) wtk_heap_malloc(s->heap,
            sizeof(wtk_lex_expr_bracket_t));
    v->v.bracket->caret = 0;
    wtk_queue_init(&(v->v.bracket->or_q));
    return v;
}

wtk_lex_expr_item_value_t* wtk_lex_script_new_esc(wtk_lex_script_t *s,
        wtk_lex_expr_escape_t esc)
{
    wtk_lex_expr_item_value_t *v;

    v = wtk_lex_script_new_value(s, WTK_LEX_VALUE_ESCAPE);
    v->v.esc = esc;
    return v;
}

wtk_lex_expr_item_value_t* wtk_lex_script_expr_add_str(wtk_lex_script_t *s,
        wtk_lex_expr_t *expr, char *data, int bytes)
{
    wtk_lex_expr_item_value_t *v;

    v = wtk_lex_script_new_value_str(s, data, bytes);
    wtk_queue_push(&(expr->v.expr->value_q), &(v->q_n));
    return v;
}

wtk_lex_expr_branch_t* wtk_lex_expr_script_new_branch(wtk_lex_script_t *s)
{
    wtk_lex_expr_branch_t *b;

    b = (wtk_lex_expr_branch_t*) wtk_heap_malloc(s->heap,
            sizeof(wtk_lex_expr_branch_t));
    wtk_queue_init(&(b->value_q));
    return b;
}

void wtk_lex_script_set_output_item_value(wtk_lex_script_t *s,
        wtk_lex_expr_output_item_t *output, char *k, int k_len)
{
    char *ps;
    int v;

    ps = wtk_str_chr(k, k_len, '$');
    if (ps) {
        //wtk_debug("[%.*s]\n",(int)(ps-k),k);
        output->type = WTK_LEX_OUTPUT_ITEM_VARSTR;
        output->v.varstr.pre = NULL;
        output->v.varstr.pst = NULL;
        v = (int) (ps - k);
        if (v > 0) {
            output->v.varstr.pre = wtk_heap_dup_string(s->heap, k, v);
        }
        v = ps[1] - '0';
        output->v.varstr.cap_index = v;
        ps = ps + 2;
        v = k + k_len - ps;
        //wtk_debug("v[%d]\n",v);
        if (v > 0) {
            output->v.varstr.pst = wtk_heap_dup_string(s->heap, ps, v);
            //wtk_debug("[%.*s]\n",output->v.varstr.pst->len,output->v.varstr.pst->data);
        }
        //exit(0);
    } else {
        output->type = WTK_LEX_OUTPUT_ITEM_STR;
        output->v.str = wtk_heap_dup_string(s->heap, k, k_len);
    }

}

wtk_lex_expr_output_item_t* wtk_lex_script_new_output_item(wtk_lex_script_t *s,
        char *data, int len)
{
    wtk_lex_expr_output_item_t *item;

    item = (wtk_lex_expr_output_item_t*) wtk_heap_malloc(s->heap,
            sizeof(wtk_lex_expr_output_item_t));
    if (len > 0) {
        item->k = wtk_heap_dup_string(s->heap, data, len);
    } else {
        item->k = NULL;
    }
    //wtk_debug("[%.*s]\n",len,data);
    item->type = WTK_LEX_OUTPUT_ITEM_NONE;
    item->v.str = NULL;
    item->hook = NULL;
    item->post = NULL;
    return item;
}

wtk_lex_expr_output_trans_item_t* wtk_lex_script_new_output_trans_item(
        wtk_lex_script_t *s)
{
    wtk_lex_expr_output_trans_item_t *item;

    item = (wtk_lex_expr_output_trans_item_t*) wtk_heap_malloc(s->heap,
            sizeof(wtk_lex_expr_output_trans_item_t));
    item->v.str = NULL;
    item->type = WTK_LEX_OUTPUT_TRANS_NONE;
    wtk_queue_init(&(item->filter_q));
    return item;
}

wtk_lex_expr_output_trans_item_t* wtk_lex_script_new_output_trans_str_item(
        wtk_lex_script_t *s, char *data, int len)
{
    wtk_lex_expr_output_trans_item_t *item;

    item =
            (wtk_lex_expr_output_trans_item_t*) wtk_lex_script_new_output_trans_item(
                    s);
    item->v.str = wtk_heap_dup_string(s->heap, data, len);
    item->type = WTK_LEX_OUTPUT_TRANS_STR;
    return item;
}

wtk_lex_expr_output_trans_item_t* wtk_lex_script_new_output_trans_cap_item(
        wtk_lex_script_t *s, int cap)
{
    wtk_lex_expr_output_trans_item_t *item;

    item =
            (wtk_lex_expr_output_trans_item_t*) wtk_lex_script_new_output_trans_item(
                    s);
    item->v.cap_index = cap;
    item->type = WTK_LEX_OUTPUT_TRANS_VAR;
    return item;
}

void wtk_lex_expr_output_trans_filter_item_init(
        wtk_lex_expr_output_trans_filter_item_t *item)
{
    item->leaf = 1;
    item->v.func.type = WTK_LEX_OUTPUT_TRANS_FILTER_NONE;
    item->v.func.pre = NULL;
    item->v.func.suc = NULL;
    item->v.func.str = NULL;
}

wtk_lex_expr_output_trans_filter_t* wtk_lex_script_new_trans_filter(
        wtk_lex_script_t *s)
{
    wtk_lex_expr_output_trans_filter_t *filter;

    filter = (wtk_lex_expr_output_trans_filter_t*) wtk_heap_malloc(s->heap,
            sizeof(wtk_lex_expr_output_trans_filter_t));
    wtk_queue_init(&(filter->if_q));
    wtk_lex_expr_output_trans_filter_item_init(&(filter->yes));
    wtk_lex_expr_output_trans_filter_item_init(&(filter->no));
    filter->or = 0;
    return filter;
}

wtk_lex_expr_output_trans_filter_type_t wtk_lex_expr_output_trans_filter_type_get(
        char *data, int len)
{
    static wtk_string_t nms[] = {
    wtk_string("tonumber"),
    wtk_string("num2chn"),
    wtk_string("num2tel"),
    wtk_string("upper"),
    wtk_string("lower"),
    wtk_string("nil"),
    wtk_string("skip"), };
    static wtk_lex_expr_output_trans_filter_type_t filter[] = {
            WTK_LEX_OUTPUT_TRANS_FILTER_TONUMBER,
            WTK_LEX_OUTPUT_TRANS_FILTER_NUM2CHN,
            WTK_LEX_OUTPUT_TRANS_FILTER_NUM2TEL,
            WTK_LEX_OUTPUT_TRANS_FILTER_UPPER,
            WTK_LEX_OUTPUT_TRANS_FILTER_LOWER, WTK_LEX_OUTPUT_TRANS_FILTER_NIL,
            WTK_LEX_OUTPUT_TRANS_FILTER_SKIP, };
    int i;
    int n;

    n = sizeof(filter) / sizeof(wtk_lex_expr_output_trans_filter_type_t);
    for (i = 0; i < n; ++i) {
        if (wtk_str_equal(data, len, nms[i].data, nms[i].len)) {
            return filter[i];
        }
    }
    return WTK_LEX_OUTPUT_TRANS_FILTER_NONE;
}

void wtk_lex_expr_output_trans_filter_item_set_func(wtk_lex_script_t *script,
        wtk_lex_expr_output_trans_filter_item_t *item, char *data, int len)
{
    //wtk_debug("[%.*s]\n",len,data);
    item->leaf = 1;
    item->v.func.str = NULL;
    item->v.func.type = wtk_lex_expr_output_trans_filter_type_get(data, len);
    if (item->v.func.type == WTK_LEX_OUTPUT_TRANS_FILTER_NONE) {
        item->v.func.type = WTK_LEX_OUTPUT_TRANS_FILTER_STRING;
        if (len > 2 && data[0] == '\"' && data[len - 1] == '\"') {
            data += 1;
            len -= 2;
        }
        item->v.func.str = wtk_heap_dup_string(script->heap, data, len);
    }
    item->v.func.pre = NULL;
    item->v.func.suc = NULL;
}

void wtk_lex_expr_output_trans_filter_item_set_filter(
        wtk_lex_expr_output_trans_filter_item_t *item,
        wtk_lex_expr_output_trans_filter_t *filter)
{
    //wtk_debug("[%.*s]\n",len,data);
    item->leaf = 0;
    item->v.filter = filter;
}

void wtk_lex_expr_output_trans_if_set_value(wtk_lex_script_t *l,
        wtk_lex_expr_output_trans_if_t *xif, char *data, int len)
{
    //wtk_debug("[%.*s]\n",len,data);
    if (len > 1 && data[0] == '\"' && data[len - 1] == '\"') {
        xif->value_type = WTK_LEX_EXPR_OUTPUT_TRANS_IF_STR;
        xif->v.str = wtk_heap_dup_string(l->heap, data + 1, len - 2);
    } else {
        xif->value_type = WTK_LEX_EXPR_OUTPUT_TRANS_IF_NUMBER;
        xif->v.f = wtk_str_atof(data, len);
    }
}

wtk_lex_expr_output_trans_if_t* wtk_lex_script_new_output_trans_if(
        wtk_lex_script_t *s)
{
    wtk_lex_expr_output_trans_if_t *xif;

    xif = (wtk_lex_expr_output_trans_if_t*) wtk_heap_malloc(s->heap,
            sizeof(wtk_lex_expr_output_trans_if_t));
    return xif;
}

int wtk_lex_expr_output_trans_if_is_num_yes(wtk_lex_expr_output_trans_if_t *xif,
        char *data, int len)
{
    float v1, v2;

    v1 = wtk_str_atof(data, len);
    v2 = xif->v.f;
    //wtk_debug("v1=%f/%f type=%d\n",v1,v2,xif->if_type);
    switch (xif->if_type) {
        case WTK_LEX_EXPR_OUTPUT_TRANS_IF_LT:
            return v1 < v2 ? 1 : 0;
            break;
        case WTK_LEX_EXPR_OUTPUT_TRANS_IF_GT:
            return v1 > v2 ? 1 : 0;
            break;
        case WTK_LEX_EXPR_OUTPUT_TRANS_IF_EQ:
            return v1 == v2 ? 1 : 0;
            break;
        case WTK_LEX_EXPR_OUTPUT_TRANS_IF_LE:
            return v1 <= v2 ? 1 : 0;
            break;
        case WTK_LEX_EXPR_OUTPUT_TRANS_IF_GE:
            return v1 >= v2 ? 1 : 0;
            break;
    }
    return 0;
}

int wtk_lex_expr_output_trans_if_is_str_yes(wtk_lex_expr_output_trans_if_t *xif,
        char *data, int len)
{
    wtk_string_t *v;

    v = xif->v.str;
    switch (xif->if_type) {
        case WTK_LEX_EXPR_OUTPUT_TRANS_IF_LT:
            if (len < v->len) {
                return 1;
            } else if (len == v->len) {
                return strncmp(data, v->data, len) < 0 ? 1 : 0;
            } else {
                return 0;
            }
            break;
        case WTK_LEX_EXPR_OUTPUT_TRANS_IF_GT:
            if (len < v->len) {
                return 0;
            } else if (len == v->len) {
                return strncmp(data, v->data, len) > 0 ? 1 : 0;
            } else {
                return 1;
            }
            break;
        case WTK_LEX_EXPR_OUTPUT_TRANS_IF_EQ:
            if (len == v->len) {
                return strncmp(data, v->data, len) == 0 ? 1 : 0;
            } else {
                return 0;
            }
            break;
        case WTK_LEX_EXPR_OUTPUT_TRANS_IF_LE:
            if (len < v->len) {
                return 1;
            } else if (len == v->len) {
                return strncmp(data, v->data, len) <= 0 ? 1 : 0;
            } else {
                return 0;
            }
            break;
        case WTK_LEX_EXPR_OUTPUT_TRANS_IF_GE:
            if (len < v->len) {
                return 0;
            } else if (len == v->len) {
                return strncmp(data, v->data, len) >= 0 ? 1 : 0;
            } else {
                return 1;
            }
            break;
    }
    return 0;
}

int wtk_lex_expr_output_trans_if_is_yes(wtk_lex_expr_output_trans_if_t *xif,
        char *data, int len)
{
    switch (xif->value_type) {
        case WTK_LEX_EXPR_OUTPUT_TRANS_IF_NUMBER:
            return wtk_lex_expr_output_trans_if_is_num_yes(xif, data, len);
            break;
        case WTK_LEX_EXPR_OUTPUT_TRANS_IF_STR:
            return wtk_lex_expr_output_trans_if_is_str_yes(xif, data, len);
            break;
    }
    return 0;
}

int wtk_lex_expr_output_trans_filter_is_yes(
        wtk_lex_expr_output_trans_filter_t *filter, char *data, int len)
{
    wtk_queue_node_t *qn;
    wtk_lex_expr_output_trans_if_t *xif;
    int b;

    if (filter->if_q.length == 0) {
        return 1;
    }
    if (filter->or) {
        for (qn = filter->if_q.pop; qn; qn = qn->next) {
            xif = data_offset2(qn, wtk_lex_expr_output_trans_if_t, q_n);
            b = wtk_lex_expr_output_trans_if_is_yes(xif, data, len);
            if (b) {
                return 1;
            }
        }
        return 0;
    } else {
        for (qn = filter->if_q.pop; qn; qn = qn->next) {
            xif = data_offset2(qn, wtk_lex_expr_output_trans_if_t, q_n);
            b = wtk_lex_expr_output_trans_if_is_yes(xif, data, len);
            if (b == 0) {
                return 0;
            }
        }
        return 1;
    }
}

void wtk_lex_expr_output_trans_filter_func_process(
        wtk_lex_expr_output_trans_filter_func_t *func, char *data, int len,
        wtk_strbuf_t *buf)
{
    int i;

    //wtk_debug("[%.*s] pre=%p [%.*s]\n",len,data,func->pre,func->pre->len,func->pre->data);
    wtk_strbuf_reset(buf);
    switch (func->type) {
        case WTK_LEX_OUTPUT_TRANS_FILTER_SKIP:
            break;
        case WTK_LEX_OUTPUT_TRANS_FILTER_NIL:
            break;
        case WTK_LEX_OUTPUT_TRANS_FILTER_NONE:
            wtk_strbuf_push(buf, data, len);
            break;
        case WTK_LEX_OUTPUT_TRANS_FILTER_TONUMBER:
            i = wtk_str_atoi(data, len);
            wtk_itochn(buf, i);
            break;
        case WTK_LEX_OUTPUT_TRANS_FILTER_NUM2CHN:
            wtk_stochn(buf, data, len);
            //exit(0);
            break;
        case WTK_LEX_OUTPUT_TRANS_FILTER_NUM2TEL:
            wtk_stotel(buf, data, len);
            break;
        case WTK_LEX_OUTPUT_TRANS_FILTER_UPPER:
            for (i = 0; i < len; ++i) {
                wtk_strbuf_push_c(buf, toupper(data[i]));
            }
            break;
        case WTK_LEX_OUTPUT_TRANS_FILTER_LOWER:
            for (i = 0; i < len; ++i) {
                wtk_strbuf_push_c(buf, tolower(data[i]));
            }
            break;
        case WTK_LEX_OUTPUT_TRANS_FILTER_STRING:
            if (func->str) {
                wtk_strbuf_push(buf, func->str->data, func->str->len);
            }
            break;
    }
    if (func->pre) {
        wtk_strbuf_push_front(buf, func->pre->data, func->pre->len);
    }
    if (func->suc) {
        wtk_strbuf_push(buf, func->suc->data, func->suc->len);
    }
    //wtk_debug("[%.*s]\n",buf->pos,buf->data);
}

int wtk_lex_expr_output_trans_filter_process(
        wtk_lex_expr_output_trans_filter_t *filter, char *data, int len,
        wtk_strbuf_t *buf)
{
    int yes;
    wtk_lex_expr_output_trans_filter_item_t *item;

    yes = wtk_lex_expr_output_trans_filter_is_yes(filter, data, len);
    item = yes ? (&(filter->yes)) : (&(filter->no));
    if (item->leaf) {
        if (item->v.func.type == WTK_LEX_OUTPUT_TRANS_FILTER_NIL) {
            return -1;
        }
        wtk_lex_expr_output_trans_filter_func_process(&(item->v.func), data,
                len, buf);
        return 0;
    } else {
        return wtk_lex_expr_output_trans_filter_process(item->v.filter, data,
                len, buf);
    }
}

void wtk_lex_script_close_parentheses(wtk_lex_script_t *s,
        wtk_lex_expr_parentheses_t *item)
{
    wtk_lex_expr_branch_t *b;

    if (item->or_q.length == 0) {
        return;
    }
    b = wtk_lex_expr_script_new_branch(s);
    b->value_q = item->value_q;
    wtk_queue_push(&(item->or_q), &(b->q_n));
    wtk_queue_init(&(item->value_q));
}

void wtk_lex_script_close_expr(wtk_lex_script_t *s, wtk_lex_expr_item_t *item)
{
    wtk_lex_expr_branch_t *b;

    if (item->or_q.length == 0 || item->value_q.length == 0) {
        goto end;
    }
    b = wtk_lex_expr_script_new_branch(s);
    b->value_q = item->value_q;
    wtk_queue_push(&(item->or_q), &(b->q_n));
    wtk_queue_init(&(item->value_q));
    end:
//	{
//		static int ki=0;
//
//		wtk_debug("ki=%d\n",++ki);
//		wtk_lex_expr_item_print(item);
//		if(ki==2)
//		{
//			exit(0);
//		}
//	}
    return;
}

int wtk_lex_expr_item_value_is_atom(wtk_lex_expr_item_value_t *v)
{
    switch (v->type) {
        case WTK_LEX_VALUE_STRING:
        case WTK_LEX_VALUE_BRACKET:
        case WTK_LEX_VALUE_PERIOD:
        case WTK_LEX_VALUE_RANGE:
        case WTK_LEX_VALUE_ESCAPE:
        case WTK_LEX_VALUE_BUILDIN_VAR:
            return 1;
            break;
        case WTK_LEX_VALUE_PARENTHESES:
        case WTK_LEX_VALUE_EXPR:
            return 0;
            break;
    }
    return -1;
}

int wtk_lex_expr_item_value_can_be_nil(wtk_lex_expr_item_value_t *v)
{
    return v->attr.repeat.min_count == 0 ? 1 : 0;
}

void wtk_lex_expr_item_print_value_q(wtk_queue_t *q);
void wtk_lex_expr_item_print_or_q(wtk_queue_t *q);

void wtk_lex_expr_item_value_print(wtk_lex_expr_item_value_t *v)
{
    wtk_queue_node_t *qn;
    wtk_lex_expr_item_value_t *vx;

    switch (v->type) {
        case WTK_LEX_VALUE_STRING:
            printf("%.*s", v->v.str->len, v->v.str->data);
            break;
        case WTK_LEX_VALUE_PARENTHESES:
            printf("(");
            //wtk_debug("[%d]\n",v->v.parentheses->or_q.length);
            if (v->v.parentheses->or_q.length > 0) {
                wtk_lex_expr_item_print_or_q(&(v->v.parentheses->or_q));
            } else {
                wtk_lex_expr_item_print_value_q(&(v->v.parentheses->value_q));
            }
            printf(")");
            break;
        case WTK_LEX_VALUE_BRACKET:
            printf("[");
            if (v->v.bracket->caret) {
                printf("^");
            }
            for (qn = v->v.bracket->or_q.pop; qn; qn = qn->next) {
                vx = data_offset2(qn, wtk_lex_expr_item_value_t, q_n);
                wtk_lex_expr_item_value_print(vx);
            }
            printf("]");
            break;
        case WTK_LEX_VALUE_ESCAPE:
            switch (v->v.esc) {
                case WTK_RE_ESC_d:
                    printf("\\d");
                    break;
                case WTK_RE_ESC_D:
                    printf("\\D");
                    break;
                case WTK_RE_ESC_s:
                    printf("\\s");
                    break;
                case WTK_RE_ESC_S:
                    printf("\\S");
                    break;
                case WTK_RE_ESC_w:
                    printf("\\w");
                    break;
                case WTK_RE_ESC_W:
                    printf("\\W");
                    break;
                case WTK_RE_ESC_h:
                    printf("\\h");
                    break;
                case WTK_RE_ESC_H:
                    printf("\\H");
                    break;
                case WTK_RE_ESC_e:
                    printf("\\e");
                    break;
                case WTK_RE_ESC_E:
                    printf("\\E");
                    break;
            }
            break;
        case WTK_LEX_VALUE_RANGE:
            printf("%c-%c", v->v.range->from, v->v.range->to);
            break;
        case WTK_LEX_VALUE_EXPR:
            printf("${%.*s}", v->v.expr->key.name->len,
                    v->v.expr->key.name->data);
            break;
        case WTK_LEX_VALUE_BUILDIN_VAR:
            printf("${.%.*s}", v->v.buildin_var->len, v->v.buildin_var->data);
            break;
        case WTK_LEX_VALUE_PERIOD:
            printf(".");
            break;
        default:
            wtk_debug("type=%d\n", v->type)
            ;
            exit(0);
            break;
    }
    if (v->attr.repeat.min_count != 1 || v->attr.repeat.max_count != 1) {
        if (v->attr.repeat.min_count == 0 && v->attr.repeat.max_count == -1) {
            printf("*");
        } else if (v->attr.repeat.min_count == 0
                && v->attr.repeat.max_count == 1) {
            printf("?");
        } else if (v->attr.repeat.min_count == 1
                && v->attr.repeat.max_count == -1) {
            printf("+");
        } else {
            printf("{%d,%d}", v->attr.repeat.min_count,
                    v->attr.repeat.max_count);
        }
    }
    if (v->attr.txt != WTK_LEX_NORMAL || v->attr.min_wrd_count != -1
            || v->attr.max_wrd_count != -1 || v->attr.skip || v->attr.chn2num
            || v->attr.skipws) {
        int pad = 0;

        printf("/");
        if (v->attr.txt != WTK_LEX_NORMAL) {
            if (v->attr.txt == WTK_LEX_LOWER) {
                printf("lower");
            } else {
                printf("upper");
            }
            pad = 1;
        }
        if (v->attr.min_wrd_count != -1 || v->attr.max_wrd_count != -1) {
            if (pad) {
                printf(",");
            }
            printf("min_wrd_count=%d,max_wrd_count=%d", v->attr.min_wrd_count,
                    v->attr.max_wrd_count);
            pad = 1;
        }
        if (v->attr.skip) {
            if (pad) {
                printf(",");
            }
            printf("skip=\"%.*s\"", v->attr.skip->len, v->attr.skip->data);
            pad = 1;
        }
        if (v->attr.like) {
            if (pad) {
                printf(",");
            }
            {
                wtk_queue_node_t *qn;
                wtk_lex_like_item_t *item;

                for (qn = v->attr.like->pop; qn; qn = qn->next) {
                    item = data_offset2(qn, wtk_lex_like_item_t, q_n);
                    if (qn != v->attr.like->pop) {
                        printf(",");
                    }
                    printf("like=\"%.*s\"", item->like->len, item->like->data);
                    if (item->thresh != 0) {
                        printf(",like_thresh=%.3f", item->thresh);
                    }
                }
            }
            //printf("like=\"%.*s\"",v->attr.like->len,v->attr.like->data);
            pad = 1;
        }
        if (v->attr.chn2num) {
            if (pad) {
                printf(",");
            }
            printf("chn2num");
            pad = 1;
        }
        if (v->attr.skipws) {
            if (pad) {
                printf(",");
            }
            printf("skipws");
            pad = 1;
        }
        printf("/");
    }
}

void wtk_lex_expr_item_print_value_q(wtk_queue_t *q)
{
    wtk_queue_node_t *qn;
    wtk_lex_expr_item_value_t *v;

    for (qn = q->pop; qn; qn = qn->next) {
        v = data_offset2(qn, wtk_lex_expr_item_value_t, q_n);
        wtk_lex_expr_item_value_print(v);
    }
}

void wtk_lex_expr_item_print_or_q(wtk_queue_t *q)
{
    wtk_queue_node_t *qn;
    wtk_lex_expr_branch_t *b;

    //wtk_debug("len=%d\n",q->length);
    for (qn = q->pop; qn; qn = qn->next) {
        b = data_offset2(qn, wtk_lex_expr_branch_t, q_n);
        if (qn != q->pop) {
            printf("|");
        }
        wtk_lex_expr_item_print_value_q(&(b->value_q));
    }
}

void wtk_lex_expr_output_item_print(wtk_lex_expr_output_item_t *item)
{
    if (item->k) {
        printf("%.*s", item->k->len, item->k->data);
    }
    switch (item->type) {
        case WTK_LEX_OUTPUT_ITEM_NONE:
            break;
        case WTK_LEX_OUTPUT_ITEM_VARSTR:
            printf("=\"");
            if (item->v.varstr.pre) {
                printf("%.*s", item->v.varstr.pre->len,
                        item->v.varstr.pre->data);
            }
            printf("$%d", item->v.varstr.cap_index);
            if (item->v.varstr.pst) {
                printf("%.*s", item->v.varstr.pst->len,
                        item->v.varstr.pst->data);
            }
            printf("\"");
            if (item->hook || item->miss_pen != 0) {
                int pad = 0;

                printf("/");
                if (item->miss_pen != 0) {
                    if (pad) {
                        printf(",");
                    }
                    printf("pen=%f", item->miss_pen);
                    pad = 1;
                }
                if (item->hook) {
                    if (pad) {
                        printf(",");
                    }
                    printf("hook=\"%.*s\"", item->hook->len, item->hook->data);
                }
                printf("/");
            }
            break;
        case WTK_LEX_OUTPUT_ITEM_STR:
            printf("=\"%.*s\"", item->v.str->len, item->v.str->data);
            if (item->hook || item->miss_pen != 0) {
                int pad = 0;

                printf("/");
                if (item->miss_pen != 0) {
                    if (pad) {
                        printf(",");
                    }
                    printf("pen=%f", item->miss_pen);
                    pad = 1;
                }
                if (item->hook) {
                    if (pad) {
                        printf(",");
                    }
                    printf("hook=\"%.*s\"", item->hook->len, item->hook->data);
                }
                printf("/");
            }
            break;
        case WTK_LEX_OUTPUT_ITEM_VAR:
            printf("=\"$%d\"", item->v.var.cap_index);
            if (item->hook || item->v.var.def || item->miss_pen != 0) {
                int pad = 0;

                printf("/");
                if (item->v.var.def) {
                    printf("def=\"%.*s\"", item->v.var.def->len,
                            item->v.var.def->data);
                    pad = 1;
                }
                if (item->miss_pen != 0) {
                    if (pad) {
                        printf(",");
                    }
                    printf("pen=%f", item->miss_pen);
                    pad = 1;
                }
                if (item->hook) {
                    if (pad) {
                        printf(",");
                    }
                    printf("hook=\"%.*s\"", item->hook->len, item->hook->data);
                }
                printf("/");
            }
            break;
    }
}

void wtk_lex_expr_output_trans_filter_func_print(
        wtk_lex_expr_output_trans_filter_func_t *func)
{
    switch (func->type) {
        case WTK_LEX_OUTPUT_TRANS_FILTER_NONE:
            printf("NONE");
            break;
        case WTK_LEX_OUTPUT_TRANS_FILTER_NIL:
            printf("NIL");
            break;
        case WTK_LEX_OUTPUT_TRANS_FILTER_SKIP:
            printf("skip");
            break;
        case WTK_LEX_OUTPUT_TRANS_FILTER_TONUMBER:
            printf("tonumber");
            break;
        case WTK_LEX_OUTPUT_TRANS_FILTER_NUM2CHN:
            printf("num2chn");
            break;
        case WTK_LEX_OUTPUT_TRANS_FILTER_NUM2TEL:
            printf("num2tel");
            break;
        case WTK_LEX_OUTPUT_TRANS_FILTER_UPPER:
            printf("upper");
            break;
        case WTK_LEX_OUTPUT_TRANS_FILTER_LOWER:
            printf("lower");
            break;
        case WTK_LEX_OUTPUT_TRANS_FILTER_STRING:
            printf("%.*s", func->str->len, func->str->data);
            break;
    }
    if (func->suc || func->pre) {
        printf("[");
        if (func->pre) {
            printf("pre=\"%.*s\"", func->pre->len, func->pre->data);
        }
        if (func->suc) {
            if (func->pre) {
                printf(";");
            }
            printf("suc=\"%.*s\"", func->suc->len, func->suc->data);
        }
        printf("]");
    }
}

void wtk_lex_expr_output_trans_filter_print(
        wtk_lex_expr_output_trans_filter_t *filter);

void wtk_lex_expr_output_trans_filter_item_print(
        wtk_lex_expr_output_trans_filter_item_t *item)
{
    if (item->leaf) {
        wtk_lex_expr_output_trans_filter_func_print(&(item->v.func));
    } else {
        printf("(");
        wtk_lex_expr_output_trans_filter_print(item->v.filter);
        printf(")");
    }
}

void wtk_lex_expr_output_trans_filter_print(
        wtk_lex_expr_output_trans_filter_t *filter)
{
    wtk_queue_node_t *qn;
    wtk_lex_expr_output_trans_if_t *xif;

    if (filter->if_q.length > 1) {
        printf("(");
    }
    for (qn = filter->if_q.pop; qn; qn = qn->next) {
        if (qn != filter->if_q.pop) {
            printf("%s", filter->or ? "|" : "&");
        }
        xif = data_offset2(qn, wtk_lex_expr_output_trans_if_t, q_n);
        printf("$");
        switch (xif->if_type) {
            case WTK_LEX_EXPR_OUTPUT_TRANS_IF_LT:
                printf("<");
                break;
            case WTK_LEX_EXPR_OUTPUT_TRANS_IF_GT:
                printf(">");
                break;
            case WTK_LEX_EXPR_OUTPUT_TRANS_IF_EQ:
                printf("=");
                break;
            case WTK_LEX_EXPR_OUTPUT_TRANS_IF_LE:
                printf("<=");
                break;
            case WTK_LEX_EXPR_OUTPUT_TRANS_IF_GE:
                printf(">=");
                break;
        }
        switch (xif->value_type) {
            case WTK_LEX_EXPR_OUTPUT_TRANS_IF_NUMBER:
                if (xif->v.f == (int) xif->v.f) {
                    printf("%d", (int) xif->v.f);
                } else {
                    printf("%.3f", xif->v.f);
                }
                break;
            case WTK_LEX_EXPR_OUTPUT_TRANS_IF_STR:
                printf("%.*s", xif->v.str->len, xif->v.str->data);
                break;
        }
        //printf("?");
    }
    if (filter->if_q.length > 1) {
        printf(")");
    }
    if (filter->if_q.length > 0) {
        printf("?");
        wtk_lex_expr_output_trans_filter_item_print(&(filter->yes));
        printf(":");
        wtk_lex_expr_output_trans_filter_item_print(&(filter->no));
    } else {
        wtk_lex_expr_output_trans_filter_item_print(&(filter->yes));
    }
}

void wtk_lex_expr_output_trans_item_print(
        wtk_lex_expr_output_trans_item_t *item)
{
    wtk_queue_node_t *qn;
    wtk_lex_expr_output_trans_filter_t *filter;

    if (item->type == WTK_LEX_OUTPUT_TRANS_STR) {
        printf("%.*s", item->v.str->len, item->v.str->data);
    } else {
        printf("$%d", item->v.cap_index);
    }
    if (item->filter_q.length > 0) {
        printf("/");
        for (qn = item->filter_q.pop; qn; qn = qn->next) {
            filter = data_offset2(qn, wtk_lex_expr_output_trans_filter_t, q_n);
            wtk_lex_expr_output_trans_filter_print(filter);
            printf(";");
        }
        printf("/");
    }

}

void wtk_lex_expr_output_print_trans(wtk_lex_expr_output_t *output)
{
    wtk_queue_node_t *qn;
    wtk_lex_expr_output_trans_item_t *item;

    for (qn = output->item_q.pop; qn; qn = qn->next) {
        item = data_offset2(qn, wtk_lex_expr_output_trans_item_t, q_n);
        wtk_lex_expr_output_trans_item_print(item);
    }
}

void wtk_lex_expr_item_print(wtk_lex_expr_item_t *item)
{
    wtk_queue_node_t *qn;
    wtk_lex_expr_output_item_t *oi;
    wtk_lex_expr_item_t *v;

    if (item->attr.export) {
        printf("export ");
    }
    printf("%.*s=", item->key.name->len, item->key.name->data);
    if (item->attr.match_start) {
        printf("^");
    }
    if (item->or_q.length > 0) {
        wtk_lex_expr_item_print_or_q(&(item->or_q));
    } else {
        wtk_lex_expr_item_print_value_q(&(item->value_q));
    }
    if (item->attr.match_end) {
        printf("$");
    }
    if (item->output.name || item->output.item_q.pop) {
        int b = 0;

        printf(" => ");
        if (item->output.item_q.length > 0) {
            oi = data_offset2(item->output.item_q.pop,
                    wtk_lex_expr_output_item_t, q_n);
            if (oi->type > WTK_LEX_OUTPUT_ITEM_VAR) {
                b = 1;
            }
        }
        if (b) {
            wtk_lex_expr_output_print_trans(&(item->output));
        } else {
            if (item->output.name) {
                printf("%.*s", item->output.name->len, item->output.name->data);
            }
            printf("(");
            for (qn = item->output.item_q.pop; qn; qn = qn->next) {
                if (qn != item->output.item_q.pop) {
                    printf(",");
                }
                oi = data_offset2(qn, wtk_lex_expr_output_item_t, q_n);
                wtk_lex_expr_output_item_print(oi);
            }
            printf(")");
        }
    }
    v = item;
    if (v->attr.txt != WTK_LEX_NORMAL || v->attr.min_wrd_count != -1
            || v->attr.max_wrd_count != -1 || !v->attr.match_more_wrd
            || v->attr.skipws) {
        int pad = 0;

        printf("/");
        if (v->attr.txt != WTK_LEX_NORMAL) {
            if (v->attr.txt == WTK_LEX_LOWER) {
                printf("lower");
            } else {
                printf("upper");
            }
            pad = 1;
        }
        if (v->attr.min_wrd_count != -1 || v->attr.max_wrd_count != -1) {
            if (pad) {
                printf(",");
            }
            printf("min_wrd_count=%d,max_wrd_count=%d", v->attr.min_wrd_count,
                    v->attr.max_wrd_count);
            pad = 1;
        }
        //wtk_debug("skipws=%d\n",v->attr.skipws);
        if (v->attr.skipws) {
            if (pad) {
                printf(",");
            }
            printf("skipws");
            pad = 1;
        }
        if (!v->attr.match_more_wrd) {
            if (pad) {
                printf(",");
            }
            printf("match_more_wrd=0");
            pad = 1;
        }
        printf("/");
    }
    if (v->prob != 1.0) {
        printf("<%f>", v->prob);
    }
    printf(" ;\n");
}

void wtk_lex_expr_cmd_print(wtk_lex_expr_cmd_t *cmd)
{
    switch (cmd->type) {
        case WTK_LEX_EXPR_CMD_DEL:
            if (cmd->op1_act) {
                printf("del %.*s.%.*s;\n", cmd->op1_act->len,
                        cmd->op1_act->data, cmd->op1_var->len,
                        cmd->op1_var->data);
            } else {
                printf("del %.*s;\n", cmd->op1_var->len, cmd->op1_var->data);
            }
            break;
        case WTK_LEX_EXPR_CMD_ADD:
            if (cmd->op1_act) {
                printf("add %.*s.%.*s %.*s;\n", cmd->op1_act->len,
                        cmd->op1_act->data, cmd->op1_var->len,
                        cmd->op1_var->data, cmd->op1_var->len,
                        cmd->op1_var->data);
            } else {
                printf("add %.*s %.*s;\n", cmd->op1_var->len,
                        cmd->op1_var->data, cmd->op1_var->len,
                        cmd->op1_var->data);
            }
            break;
        case WTK_LEX_EXPR_CMD_MV:
            if (cmd->op1_act) {
                printf("mv %.*s.%.*s;\n", cmd->op1_act->len, cmd->op1_act->data,
                        cmd->op1_var->len, cmd->op1_var->data);
            } else {
                printf("mv %.*s;\n", cmd->op1_var->len, cmd->op1_var->data);
            }
            break;
        case WTK_LEX_EXPR_CMD_CPY:
            if (cmd->op1_act) {
                if (cmd->op2_act) {
                    printf("cpy %.*s.%.*s %.*s.%.*s;\n", cmd->op1_act->len,
                            cmd->op1_act->data, cmd->op1_var->len,
                            cmd->op1_var->data, cmd->op2_act->len,
                            cmd->op2_act->data, cmd->op1_var->len,
                            cmd->op1_var->data);
                } else {
                    printf("cpy %.*s.%.*s %.*s;\n", cmd->op1_act->len,
                            cmd->op1_act->data, cmd->op1_var->len,
                            cmd->op1_var->data, cmd->op1_var->len,
                            cmd->op1_var->data);
                }
            } else {
                if (cmd->op2_act) {
                    printf("cpy %.*s %.*s.%.*s;\n", cmd->op1_var->len,
                            cmd->op1_var->data, cmd->op2_act->len,
                            cmd->op2_act->data, cmd->op1_var->len,
                            cmd->op1_var->data);
                } else {
                    printf("add %.*s %.*s;\n", cmd->op1_var->len,
                            cmd->op1_var->data, cmd->op1_var->len,
                            cmd->op1_var->data);
                }
            }
            break;
        case WTK_LEX_EXPR_CMD_DEBUG:
            printf("debug\n");
            break;
        case WTK_LEX_EXPR_CMD_RETURN:
            printf("return\n");
            break;
    }
}

void wtk_lex_script_print_expr2(wtk_lex_expr_t *expr, int depth)
{
    wtk_queue_node_t *qn;
    wtk_lex_expr_if_t *i;
    wtk_lex_expr_t *expr2;
    int j;

    switch (expr->type) {
        case WTK_LEX_EXPR_ITEM:
            wtk_lex_expr_item_print(expr->v.expr);
            break;
        case WTK_LEX_EXPR_SCOPE:
//		for(j=0;j<depth-1;++j)
//		{
//			printf(" ");
//		}
            printf("if");
            for (qn = expr->v.scope->condition.pop; qn; qn = qn->next) {
                i = data_offset2(qn, wtk_lex_expr_if_t, q_n);
                if (qn != expr->v.scope->condition.pop) {
                    printf(" and");
                }
                if (i->not) {
                    printf(" not");
                }
                printf(" %.*s", i->var->len, i->var->data);
            }
            printf(" then\n"); //[%d/%d]\n",expr->v.scope->yes.length,expr->v.scope->no.length);
            if (expr->v.scope->yes.length > 0) {
                for (qn = expr->v.scope->yes.pop; qn; qn = qn->next) {
                    expr2 = data_offset2(qn, wtk_lex_expr_t, q_n);
                    for (j = 0; j < depth; ++j) {
                        printf(" ");
                    }
                    wtk_lex_script_print_expr2(expr2, depth + 1);
                    //printf("\n");
                }
            }
            if (expr->v.scope->no.length > 0) {
                for (j = 0; j < depth - 1; ++j) {
                    printf(" ");
                }
                printf("else\n");
                for (qn = expr->v.scope->no.pop; qn; qn = qn->next) {
                    expr2 = data_offset2(qn, wtk_lex_expr_t, q_n);
                    for (j = 0; j < depth; ++j) {
                        printf(" ");
                    }
                    wtk_lex_script_print_expr2(expr2, depth + 1);
                    //printf("\n");
                }
            }
            for (j = 0; j < depth - 1; ++j) {
                printf(" ");
            }
            printf("end\n");
            break;
        case WTK_LEX_EXPR_CMD:
            wtk_lex_expr_cmd_print(expr->v.cmd);
            break;
    }
}

void wtk_lex_script_print_expr(wtk_lex_expr_t *expr)
{
    wtk_lex_script_print_expr2(expr, 1);
}

void wtk_lex_script_print_lib(wtk_lex_script_lib_t *lib)
{
    wtk_queue_node_t *qn;
    wtk_lex_expr_t *expr;

    for (qn = lib->local_q.pop; qn; qn = qn->next) {
        expr = data_offset2(qn, wtk_lex_expr_t, q_n);
        wtk_lex_script_print_expr(expr);
    }
    for (qn = lib->expr_q.pop; qn; qn = qn->next) {
        expr = data_offset2(qn, wtk_lex_expr_t, q_n);
        wtk_lex_script_print_expr(expr);
    }
}

void wtk_lex_script_print(wtk_lex_script_t *s)
{
    wtk_queue_node_t *qn;
    wtk_lex_script_lib_t *lib;

    wtk_debug("================== script ================\n");
    for (qn = s->lib_q.pop; qn; qn = qn->next) {
        lib = data_offset2(qn, wtk_lex_script_lib_t, q_n);
        printf("------- %.*s --------\n", lib->name->len, lib->name->data);
        wtk_lex_script_print_lib(lib);
    }
    printf("---------- main(%d/%d) ------------\n", s->main_lib->local_q.length,
            s->main_lib->expr_q.length);
    wtk_lex_script_print_lib(s->main_lib);
}

int wtk_lex_expr_item_value_match_string(wtk_lex_expr_item_value_t *item,
        wtk_string_t *tok)
{
    return wtk_string_cmp(item->v.str, tok->data, tok->len) == 0;
}

int wtk_lex_expr_item_value_match_range(wtk_lex_expr_item_value_t *item,
        wtk_string_t *tok)
{
    unsigned int v;

    v = wtk_string_to_ord(tok);
    return ((v >= item->v.range->from) && (v <= item->v.range->to));
}

/*
 * [0-9]
 */
int wtk_lex_expr_item_match_esc_d(wtk_lex_expr_item_value_t *item,
        wtk_string_t *tok)
{
    char c;

    if (tok->len == 1) {
        c = tok->data[0];
        if (c >= '0' && c <= '9') {
            return 1;
        }
    }
    return 0;
}

/*
 * [ \t\n\r\f]
 */
int wtk_lex_expr_item_match_esc_s(wtk_lex_expr_item_value_t *item,
        wtk_string_t *tok)
{
    char c;

    if (tok->len == 1) {
        c = tok->data[0];
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f') {
            return 1;
        }
    }
    return 0;
}

/*
 *[a-zA-Z0-9_]
 */
int wtk_lex_expr_item_match_esc_w(wtk_lex_expr_item_value_t *item,
        wtk_string_t *tok)
{
    char c;

    if (tok->len == 1) {
        c = tok->data[0];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
                || (c >= '0' && c <= '9') || c == '_') {
            return 1;
        }
    }
    return 0;
}

int wtk_lex_expr_item_match_esc_h(wtk_lex_expr_item_value_t *item,
        wtk_string_t *tok)
{
    char c;

    if (tok->len == 1) {
        return 0;
    } else {
        c = wtk_utf8_bytes(tok->data[0]);
        return c > 1 ? 1 : 0;
    }
}

int wtk_lex_expr_item_match_esc_e(wtk_lex_expr_item_value_t *item,
        wtk_string_t *tok)
{
    char *s, *e;
    int n;

    s = tok->data;
    e = s + tok->len;
    while (s < e) {
        n = wtk_utf8_bytes(*s);
        if (n > 1) {
            return 0;
        }
        if (!isalpha(*s)) {
            return 0;
        }
        s += n;
    }
    return 1;
}

int wtk_lex_expr_item_value_match_esc(wtk_lex_expr_item_value_t *item,
        wtk_string_t *tok)
{
    int match = 0;

    switch (item->v.esc) {
        case WTK_RE_ESC_d:    //\d [0-9]
            match = wtk_lex_expr_item_match_esc_d(item, tok);
            break;
        case WTK_RE_ESC_D:	 //\D [^0-9]
            match = !wtk_lex_expr_item_match_esc_d(item, tok);
            break;
        case WTK_RE_ESC_s:	 //\s  [ \t\n\r\f]
            match = wtk_lex_expr_item_match_esc_s(item, tok);
            break;
        case WTK_RE_ESC_S:	 //\S
            match = !wtk_lex_expr_item_match_esc_s(item, tok);
            break;
        case WTK_RE_ESC_w:	 //\w	[a-zA-Z0-9_]
            match = wtk_lex_expr_item_match_esc_w(item, tok);
            break;
        case WTK_RE_ESC_W:	 //\W
            match = !wtk_lex_expr_item_match_esc_w(item, tok);
            break;
        case WTK_RE_ESC_h:
            match = wtk_lex_expr_item_match_esc_h(item, tok);
            break;
        case WTK_RE_ESC_H:
            match = !wtk_lex_expr_item_match_esc_h(item, tok);
            break;
        case WTK_RE_ESC_e:
            match = wtk_lex_expr_item_match_esc_e(item, tok);
            break;
        case WTK_RE_ESC_E:
            match = !wtk_lex_expr_item_match_esc_e(item, tok);
            break;
    }
    return match;
}

int wtk_lex_expr_item_value_match_bracket(wtk_lex_expr_item_value_t *item,
        wtk_string_t *tok)
{
    wtk_lex_expr_bracket_t *bracket = item->v.bracket;
    wtk_queue_node_t *n;
    int match = 0;

    //wtk_debug("[%.*s]\n",tok->len,tok->data);
    for (n = bracket->or_q.pop; n && !match; n = n->next) {
        item = data_offset(n, wtk_lex_expr_item_value_t, q_n);
        switch (item->type) {
            case WTK_LEX_VALUE_STRING:
                match = wtk_lex_expr_item_value_match_string(item, tok);
                //wtk_debug("match %d [%.*s]\n",match,item->v.str->len,item->v.str->data);
//			if(bracket->caret)
//			{
//				match=match?0:1;
//			}
                break;
            case WTK_LEX_VALUE_RANGE:
                match = wtk_lex_expr_item_value_match_range(item, tok);
//			if(bracket->caret)
//			{
//				match=match?0:1;
//			}
                break;
            case WTK_LEX_VALUE_ESCAPE:
                match = wtk_lex_expr_item_value_match_esc(item, tok);
//			if(bracket->caret)
//			{
//				match=match?0:1;
//			}
                break;
            case WTK_LEX_VALUE_PERIOD:
                match = 1;
//			if(bracket->caret)
//			{
//				match=match?0:1;
//			}
                break;
            default:
                wtk_debug("invalid type found.\n")
                ;
                break;
        }
        if (match) {
            break;
        }
    }
    if (bracket->caret) {
        match = match ? 0 : 1;
    }
    //wtk_debug("match=%d/%d,[%.*s]\n",match,bracket->caret,tok->len,tok->data);
    return match;
}

int wtk_lex_expr_item_value_match(wtk_lex_expr_item_value_t *item,
        wtk_string_t *tok)
{
    int match = 0;

    switch (item->type) {
        case WTK_LEX_VALUE_STRING:
            match = wtk_lex_expr_item_value_match_string(item, tok);
            //wtk_debug("match string [%.*s]=[%.*s] match=%d\n",item->v.str->len,item->v.str->data,tok->len,tok->data,match);
            break;
        case WTK_LEX_VALUE_PERIOD:		//.
            match = 1;
            break;
        case WTK_LEX_VALUE_RANGE:		 //a-z
            match = wtk_lex_expr_item_value_match_range(item, tok);
            break;
        case WTK_LEX_VALUE_ESCAPE:		//\d \D \s \S \w \W
            match = wtk_lex_expr_item_value_match_esc(item, tok);
            break;
        case WTK_LEX_VALUE_BRACKET:		//[]
            //wtk_debug("expand [] {%d,%d}=%d...\n",item->repeat.min_count,item->repeat.max_count,net_n->repeat);
            match = wtk_lex_expr_item_value_match_bracket(item, tok);
            break;
        default:
            wtk_debug("not supprt match type\n")
            ;
            break;
    }
    return match;
}

int wtk_lex_expr_output_has_redirect(wtk_lex_expr_output_t *out)
{
    return (out->name || out->item_q.length > 0) ? 1 : 0;
}

int wtk_lex_expr_output_is_alias(wtk_lex_expr_output_t *out)
{
    wtk_lex_expr_output_item_t *output;

    if (!out->name && out->item_q.length == 1) {
        output = data_offset(out->item_q.pop, wtk_lex_expr_output_item_t, q_n);
        return output->type == WTK_LEX_OUTPUT_ITEM_NONE ? 1 : 0;
    } else {
        return 0;
    }
}

wtk_string_t* wtk_lex_expr_output_get_alias(wtk_lex_expr_output_t *out)
{
    wtk_lex_expr_output_item_t *output;

    if (!out->name && out->item_q.length == 1) {
        output = data_offset(out->item_q.pop, wtk_lex_expr_output_item_t, q_n);
        if (output->type == WTK_LEX_OUTPUT_ITEM_NONE) {
            return output->k;
        }
    }
    return NULL;
}

int wtk_lex_expr_cmp_prob(wtk_queue_node_t *qn1, wtk_queue_node_t *qn2)
{
    wtk_lex_expr_t *expr1, *expr2;

    expr1 = data_offset2(qn1, wtk_lex_expr_t, q_n);
    expr2 = data_offset2(qn2, wtk_lex_expr_t, q_n);
    if (expr1->type != expr2->type) {
        return expr2->type - expr1->type;
    }
    if (expr1->type != WTK_LEX_EXPR_ITEM) {
        return 0;
    }
    return (expr1->v.expr->prob - expr2->v.expr->prob) > 0 ? 1 : -1;
}

void wtk_lex_script_sort_prob(wtk_lex_script_t *script)
{
    //wtk_debug("update ...\n");
    wtk_queue_sort(&(script->main_lib->expr_q),
            (wtk_queue_node_cmp_f) wtk_lex_expr_cmp_prob);
    //wtk_lex_script_print(script);
    //exit(0);
}

void wtk_lex_script_update(wtk_lex_script_t *script)
{
    if (script->main_lib->expr_q.length <= 0) {
        return;
    }
    if (script->sort_by_prob) {
        wtk_lex_script_sort_prob(script);
    }
}

void wtk_lex_expr_item_value_attr_add_ner(wtk_lex_expr_item_value_attr_t *attr,
        wtk_heap_t *heap, wtk_lexr_ner_item_t *ner)
{
    wtk_lex_ner_item_t *item;

    //wtk_debug("[%.*s]\n",bytes,data);
    item = (wtk_lex_ner_item_t*) wtk_heap_malloc(heap,
            sizeof(wtk_lex_ner_item_t));
    item->ner = ner;
    item->prune_thresh = ner->prune_thresh;
    item->conf_thresh = ner->conf_thresh;
    item->wrd_pen = ner->wrd_pen;
    item->use_search = 0;
    if (!attr->like) {
        attr->ner = (wtk_queue_t*) wtk_heap_malloc(heap, sizeof(wtk_queue_t));
        wtk_queue_init(attr->ner);
    }
    wtk_queue_push(attr->ner, &(item->q_n));
}

void wtk_lex_expr_item_value_attr_add_ner_wrd_pen(
        wtk_lex_expr_item_value_attr_t *attr, float f)
{
    wtk_lex_ner_item_t *item;

    if (attr->ner && attr->ner->pop) {
        item = data_offset2(attr->ner->push, wtk_lex_ner_item_t, q_n);
        item->wrd_pen = f;
    }
}

void wtk_lex_expr_item_value_attr_add_ner_prune_thresh(
        wtk_lex_expr_item_value_attr_t *attr, float f)
{
    wtk_lex_ner_item_t *item;

    if (attr->ner && attr->ner->pop) {
        item = data_offset2(attr->ner->push, wtk_lex_ner_item_t, q_n);
        item->prune_thresh = f;
    }
}

void wtk_lex_expr_item_value_attr_add_ner_conf_thresh(
        wtk_lex_expr_item_value_attr_t *attr, float f)
{
    wtk_lex_ner_item_t *item;

    if (attr->ner && attr->ner->pop) {
        item = data_offset2(attr->ner->push, wtk_lex_ner_item_t, q_n);
        item->conf_thresh = f;
    }
}

void wtk_lex_expr_item_value_attr_add_ner_use_search(
        wtk_lex_expr_item_value_attr_t *attr, int u)
{
    wtk_lex_ner_item_t *item;

    if (attr->ner && attr->ner->pop) {
        item = data_offset2(attr->ner->push, wtk_lex_ner_item_t, q_n);
        item->use_search = u;
    }
}

void wtk_lex_script_expr_add_not_pre(wtk_lex_script_t *script,
        wtk_lex_expr_item_value_t *value, wtk_array_t *a)
{
    wtk_queue_node_t *qn;
    wtk_lex_expr_branch_t *b;
    wtk_lex_expr_item_value_t *v;
//	wtk_string_t **strs;
//	int i;

    if (value->type != WTK_LEX_VALUE_PARENTHESES) {
        return;
    }
    //strs=(wtk_string_t**)(a->slot);
    for (qn = value->v.parentheses->or_q.pop; qn; qn = qn->next) {
        b = data_offset2(qn, wtk_lex_expr_branch_t, q_n);
        if (b->value_q.pop) {
            v = data_offset2(b->value_q.pop, wtk_lex_expr_item_value_t, q_n);
            switch (v->type) {
                case WTK_LEX_VALUE_STRING:
                    v->attr.not_pre = a;
//				if(!v->attr.not_pre)
//				{
//					v->attr.not_pre
//				}
//				for(i=0;i<a->nslot;++i)
//				{
//				}
                    break;
                case WTK_LEX_VALUE_PARENTHESES:
                    wtk_lex_script_expr_add_not_pre(script, v, a);
                    break;
                default:
                    break;
            }
        }
    }
}

void wtk_lex_script_expr_add_pre(wtk_lex_script_t *script,
        wtk_lex_expr_item_value_t *value, wtk_array_t *a)
{
    wtk_queue_node_t *qn;
    wtk_lex_expr_branch_t *b;
    wtk_lex_expr_item_value_t *v;

    if (value->type != WTK_LEX_VALUE_PARENTHESES) {
        return;
    }
    //strs=(wtk_string_t**)(a->slot);
    for (qn = value->v.parentheses->or_q.pop; qn; qn = qn->next) {
        b = data_offset2(qn, wtk_lex_expr_branch_t, q_n);
        if (b->value_q.pop) {
            v = data_offset2(b->value_q.pop, wtk_lex_expr_item_value_t, q_n);
            switch (v->type) {
                case WTK_LEX_VALUE_STRING:
                    v->attr.pre = a;
                    break;
                case WTK_LEX_VALUE_PARENTHESES:
                    wtk_lex_script_expr_add_pre(script, v, a);
                    break;
                default:
                    break;
            }
        }
    }
}

void wtk_lex_script_expr_add_suc(wtk_lex_script_t *script,
        wtk_lex_expr_item_value_t *value, wtk_array_t *a)
{
    wtk_queue_node_t *qn;
    wtk_lex_expr_branch_t *b;
    wtk_lex_expr_item_value_t *v;

    if (value->type != WTK_LEX_VALUE_PARENTHESES) {
        return;
    }
    //strs=(wtk_string_t**)(a->slot);
    for (qn = value->v.parentheses->or_q.pop; qn; qn = qn->next) {
        b = data_offset2(qn, wtk_lex_expr_branch_t, q_n);
        if (b->value_q.pop) {
            v = data_offset2(b->value_q.pop, wtk_lex_expr_item_value_t, q_n);
            switch (v->type) {
                case WTK_LEX_VALUE_STRING:
                    v->attr.suc = a;
                    break;
                case WTK_LEX_VALUE_PARENTHESES:
                    wtk_lex_script_expr_add_suc(script, v, a);
                    break;
                default:
                    break;
            }
        }
    }
}

void wtk_lex_script_expr_add_not_suc(wtk_lex_script_t *script,
        wtk_lex_expr_item_value_t *value, wtk_array_t *a)
{
    wtk_queue_node_t *qn;
    wtk_lex_expr_branch_t *b;
    wtk_lex_expr_item_value_t *v;

    if (value->type != WTK_LEX_VALUE_PARENTHESES) {
        return;
    }
    //strs=(wtk_string_t**)(a->slot);
    for (qn = value->v.parentheses->or_q.pop; qn; qn = qn->next) {
        b = data_offset2(qn, wtk_lex_expr_branch_t, q_n);
        if (b->value_q.pop) {
            v = data_offset2(b->value_q.pop, wtk_lex_expr_item_value_t, q_n);
            switch (v->type) {
                case WTK_LEX_VALUE_STRING:
                    v->attr.not_suc = a;
                    break;
                case WTK_LEX_VALUE_PARENTHESES:
                    wtk_lex_script_expr_add_not_suc(script, v, a);
                    break;
                default:
                    break;
            }
        }
    }
}

void wtk_lex_expr_item_value_attr_add_like(wtk_lex_expr_item_value_attr_t *attr,
        wtk_heap_t *heap, char *data, int bytes)
{
    wtk_lex_like_item_t *item;

    //wtk_debug("[%.*s]\n",bytes,data);
    item = (wtk_lex_like_item_t*) wtk_heap_malloc(heap,
            sizeof(wtk_lex_like_item_t));
    item->like = wtk_heap_dup_string(heap, data, bytes);
    item->thresh = 0;
    if (!attr->like) {
        attr->like = (wtk_queue_t*) wtk_heap_malloc(heap, sizeof(wtk_queue_t));
        wtk_queue_init(attr->like);
    }
    wtk_queue_push(attr->like, &(item->q_n));
}

void wtk_lex_expr_item_value_attr_add_like_thresh(
        wtk_lex_expr_item_value_attr_t *attr, float f)
{
    wtk_lex_like_item_t *item;

    if (attr->like && attr->like->pop) {
        item = data_offset2(attr->like->push, wtk_lex_like_item_t, q_n);
        item->thresh = f;
    }
}

void wtk_lex_expr_item_value_update_py(wtk_lex_expr_item_value_t *v, int py)
{
    wtk_queue_node_t *qn, *qn2;
    wtk_lex_expr_branch_t *branch;
    //wtk_lex_expr_item_value_t *value;

    switch (v->type) {
        case WTK_LEX_VALUE_STRING:
            //wtk_debug("[%.*s]\n",v->v.str->len,v->v.str->data);
            v->attr.use_py = py;
            break;
        case WTK_LEX_VALUE_PARENTHESES: {
            wtk_lex_expr_parentheses_t *ps;

            ps = v->v.parentheses;
            //wtk_debug("value=%d or=%d\n",ps->or_q.length,ps->value_q.length);
            for (qn2 = ps->value_q.pop; qn2; qn2 = qn2->next) {
                v = data_offset2(qn2, wtk_lex_expr_item_value_t, q_n);
                wtk_lex_expr_item_value_update_py(v, py);
            }
            for (qn = ps->or_q.pop; qn; qn = qn->next) {
                branch = data_offset2(qn, wtk_lex_expr_branch_t, q_n);
                for (qn2 = branch->value_q.pop; qn2; qn2 = qn2->next) {
                    v = data_offset2(qn2, wtk_lex_expr_item_value_t, q_n);
                    wtk_lex_expr_item_value_update_py(v, py);
                }
            }
        }
            break;
        case WTK_LEX_VALUE_EXPR: {
            wtk_lex_expr_item_t *item;

            item = v->v.expr;
            for (qn2 = item->value_q.pop; qn2; qn2 = qn2->next) {
                v = data_offset2(qn2, wtk_lex_expr_item_value_t, q_n);
                wtk_lex_expr_item_value_update_py(v, py);
            }
            for (qn = item->or_q.pop; qn; qn = qn->next) {
                branch = data_offset2(qn, wtk_lex_expr_branch_t, q_n);
                for (qn2 = branch->value_q.pop; qn2; qn2 = qn2->next) {
                    v = data_offset2(qn2, wtk_lex_expr_item_value_t, q_n);
                    wtk_lex_expr_item_value_update_py(v, py);
                }
            }
        }
            break;
        default:
            break;
    }
}
