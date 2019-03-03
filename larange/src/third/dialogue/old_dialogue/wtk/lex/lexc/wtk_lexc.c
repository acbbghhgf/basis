#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>
#include "wtk_lexc.h" 
void wtk_lexc_set_state(wtk_lexc_t *l, wtk_lexc_state_t state);
int wtk_lexc_feed(wtk_lexc_t *l, wtk_string_t *v);
int wtk_lexc_include_file(wtk_lexc_t *l, char *fn);
wtk_lex_expr_t* wtk_lexc_add_new_cmd(wtk_lexc_t *l);

wtk_lexc_t* wtk_lexc_new(wtk_lexc_cfg_t *cfg)
{
    wtk_lexc_t *l;

    l = (wtk_lexc_t*) wtk_malloc(sizeof(wtk_lexc_t));
    l->cfg = cfg;
    l->rbin = NULL;
    l->heap = wtk_heap_new(4096);
    l->str_var_hash = wtk_str_hash_new(257);
    l->tok = wtk_strbuf_new(256, 1);
    l->buf = wtk_strbuf_new(256, 1);
    l->stk_pool = wtk_vpool_new(sizeof(wtk_lexc_env_stk_t), 10);
    l->scope_pool = wtk_vpool_new(sizeof(wtk_lexc_scope_stk_t), 10);
    l->get_expr = NULL;
    l->get_expr_ths = NULL;
    wtk_lexc_reset(l);
    return l;
}

void wtk_lexc_delete(wtk_lexc_t *l)
{
    wtk_vpool_delete(l->scope_pool);
    wtk_vpool_delete(l->stk_pool);
    wtk_strbuf_delete(l->buf);
    wtk_str_hash_delete(l->str_var_hash);
    wtk_strbuf_delete(l->tok);
    wtk_heap_delete(l->heap);
    wtk_free(l);
}

void wtk_lexc_reset(wtk_lexc_t *l)
{
    wtk_vpool_reset(l->scope_pool);
    wtk_vpool_reset(l->stk_pool);
    wtk_str_hash_reset(l->str_var_hash);
    wtk_heap_reset(l->heap);
    l->pre_dat = NULL;
    l->use_pre = 0;
    l->pwd = NULL;
    l->script = NULL;
    l->cur_expr = NULL;
    l->str_key = NULL;
    l->lib = NULL;
    l->cur_output_item = 0;
    wtk_lexc_set_state(l, WTK_LEXC_INIT);
    wtk_queue_init(&(l->value_stk_q));
    wtk_queue_init(&(l->scope_stk_q));

    l->trans_tk = NULL;
    wtk_queue_init(&(l->trans_stk_q));

}

void wtk_lexc_set_state(wtk_lexc_t *l, wtk_lexc_state_t state)
{
    l->state = state;
    l->sub_state = 0;
}

void wtk_lexc_set_string_state(wtk_lexc_t *l, wtk_lexc_state_t reback_state,
        int reback_sub_state)
{
    wtk_strbuf_reset(l->tok);
    l->string_env.back_state = reback_state;
    l->string_env.back_sub_state = reback_sub_state;
    l->string_env.quoted = 0;
    wtk_lexc_set_state(l, WTK_LEXC_STRING);
}

void wtk_lexc_restore_string_state(wtk_lexc_t *l)
{
    l->state = l->string_env.back_state;
    l->sub_state = l->string_env.back_sub_state;
}

int wtk_lexc_feed_init(wtk_lexc_t *l, wtk_string_t *v)
{
    int ret;
    char c;

    if (v->len == 1) {
        c = v->data[0];
        if (isspace(c)) {
            ret = 0;
            goto end;
        } else if (c == '#') {
            wtk_lexc_set_state(l, WTK_LEXC_COMMENT);
            ret = 0;
            goto end;
        } else if (c == '/') {
            wtk_lexc_set_state(l, WTK_LEXC_COMMENT2);
            ret = 0;
            goto end;
        }
    }
    wtk_strbuf_reset(l->tok);
    wtk_lexc_set_state(l, WTK_LEXC_EXPR_NAME);
    ret = wtk_lexc_feed(l, v);
end: 
	return ret;
}

int wtk_lexc_feed_comment(wtk_lexc_t *l, wtk_string_t *v)
{
    enum {
        WTK_LEXC_COMMENT_INIT = 0,
        WTK_LEXC_COMMENT_CMD,
        WTK_LEXC_COMMENT_WAIT_END,
    };
    wtk_strbuf_t *buf = l->tok;

    switch (l->sub_state) {
        case WTK_LEXC_COMMENT_INIT:
            if (v->len == 1) {
                if (isspace(v->data[0])) {
                    l->sub_state = WTK_LEXC_COMMENT_WAIT_END;
                } else {
                    l->sub_state = WTK_LEXC_COMMENT_CMD;
                    wtk_strbuf_reset(buf);
                    wtk_strbuf_push(buf, v->data, v->len);
                }
            } else {
                l->sub_state = WTK_LEXC_COMMENT_WAIT_END;
            }
            break;
        case WTK_LEXC_COMMENT_CMD:
            if (v->len == 1 && (isspace(v->data[0]) || v->data[0] == ';')) {
                //wtk_debug("[%.*s]\n",buf->pos,buf->data);
                if (wtk_str_equal_s(buf->data, buf->pos, "include")) {
                    wtk_lexc_set_state(l, WTK_LEXC_INCLUDE);
                } else if (wtk_str_equal_s(buf->data, buf->pos, "import")) {
                    wtk_lexc_set_state(l, WTK_LEXC_IMPORT);
                } else if (wtk_str_equal_s(buf->data, buf->pos, "end")) {
                    wtk_lexc_set_state(l, WTK_LEXC_CMD_END);
                } else if (wtk_str_equal_s(buf->data, buf->pos, "exit")) {
                    wtk_lex_script_print(l->script);
                    exit(0);
                } else if (wtk_str_equal_s(buf->data, buf->pos,
                        "sort_by_prob")) {
                    l->script->sort_by_prob = 1;
                    l->sub_state = WTK_LEXC_COMMENT_WAIT_END;
                    return wtk_lexc_feed_comment(l, v);
                } else if (wtk_str_equal_s(buf->data, buf->pos,
                        "use_fast_match") || wtk_str_equal_s(buf->data,buf->pos,"use_fast_match=1")||wtk_str_equal_s(buf->data,buf->pos,"use_fast_match=1;")) {
                    ///wtk_debug("use_fast_match ------------\n");
                    l->script->use_fast_match = 1;
                    l->sub_state = WTK_LEXC_COMMENT_WAIT_END;
                    return wtk_lexc_feed_comment(l, v);
                } else if (wtk_str_equal_s(buf->data, buf->pos,
                        "use_fast_match=0")||wtk_str_equal_s(buf->data,buf->pos,"use_fast_match=0;")) {
                    l->script->use_fast_match = 0;
                    l->sub_state = WTK_LEXC_COMMENT_WAIT_END;
                    return wtk_lexc_feed_comment(l, v);
                } else if (wtk_str_equal_s(buf->data, buf->pos,
                        "use_nbest=0") || wtk_str_equal_s(buf->data,buf->pos,"use_nbest=0;")) {
                    l->script->use_nbest = 0;
                    l->sub_state = WTK_LEXC_COMMENT_WAIT_END;
                    return wtk_lexc_feed_comment(l, v);
                } else if (wtk_str_equal_s(buf->data, buf->pos,
                        "use_nbest=1") || wtk_str_equal_s(buf->data,buf->pos,"use_nbest=1;")) {
                    l->script->use_nbest = 1;
                    l->sub_state = WTK_LEXC_COMMENT_WAIT_END;
                    return wtk_lexc_feed_comment(l, v);
                } else {
                    l->sub_state = WTK_LEXC_COMMENT_WAIT_END;
                    return wtk_lexc_feed_comment(l, v);
                }
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
        case WTK_LEXC_COMMENT_WAIT_END:
            if (v->len == 1 && v->data[0] == '\n') {
                wtk_lexc_set_state(l, WTK_LEXC_INIT);
            }
            break;
    }
    return 0;
}

int wtk_lexc_feed_comment2(wtk_lexc_t *l, wtk_string_t *v)
{
    enum {
        WTK_LEXC_COMMENT2_INIT = 0,
        WTK_LEXC_COMMENT2_SINGLE,
        WTK_LEXC_COMMENT2_MULTI,
        WTK_LEXC_COMMENT2_MULTI_WAIT_END,
    };
    switch (l->sub_state) {
        case WTK_LEXC_COMMENT2_INIT:
            if (v->len == 1) {
                if (v->data[0] == '*') {
                    l->sub_state = WTK_LEXC_COMMENT2_MULTI;
                    break;
                } else if (v->data[0] == '/') {
                    l->sub_state = WTK_LEXC_COMMENT2_SINGLE;
                    break;
                }
            }
            wtk_strbuf_reset(l->tok);
            wtk_strbuf_push(l->tok, v->data, v->len);
            wtk_lexc_set_state(l, WTK_LEXC_EXPR_NAME);
            break;
        case WTK_LEXC_COMMENT2_SINGLE:
            if (v->len == 1 && v->data[0] == '\n') {
                wtk_lexc_set_state(l, WTK_LEXC_INIT);
            }
            break;
        case WTK_LEXC_COMMENT2_MULTI:
            if (v->len == 1 && v->data[0] == '*') {
                l->sub_state = WTK_LEXC_COMMENT2_MULTI_WAIT_END;
            }
            break;
        case WTK_LEXC_COMMENT2_MULTI_WAIT_END:
            if (v->len == 1 && v->data[0] == '/') {
                wtk_lexc_set_state(l, WTK_LEXC_INIT);
            } else {
                l->sub_state = WTK_LEXC_COMMENT2_MULTI;
            }
            break;
    }
    return 0;
}

int wtk_lexc_file_exist(wtk_lexc_t *lex, char *fn)
{
    if (!lex->rbin) {
        return wtk_file_exist(fn);
    } else {
        wtk_rbin2_item_t *item;

        item = wtk_rbin2_get(lex->rbin, fn, strlen(fn));
//		{
//			static int ki=0;
//
//			++ki;
//			wtk_debug("v[%d],%s:%p\n",ki,fn,item);
//			if(ki==5)
//			{
//				exit(0);
//			}
//		}
        return item ? 0 : -1;
    }
}

char* wtk_lexc_get_real_file_name(wtk_lexc_t *lex, char *fn)
{
    wtk_array_t *a = lex->cfg->include_path;
    wtk_string_t **strs;
    int i;
    wtk_strbuf_t *buf; //wtk_strbuf_new(256,1);
    int ret;
    int len;

    //wtk_debug("[%s],a=%p\n",fn,a);
    if (!a) {
        return fn;
    }
    ret = wtk_lexc_file_exist(lex, fn);
    if (ret == 0) {
        return fn;
    }
    //wtk_debug("[%s],a=%p\n",fn,a);
    len = strlen(fn);
    buf = wtk_strbuf_new(256, 1);
    strs = (wtk_string_t**) a->slot;
    for (i = 0; i < a->nslot; ++i) {
        //wtk_debug("[%.*s]\n",strs[i]->len,strs[i]->data);
        wtk_strbuf_reset(buf);
        wtk_strbuf_push(buf, strs[i]->data, strs[i]->len);
        wtk_strbuf_push_c(buf, '/');
        wtk_strbuf_push(buf, fn, len);
        wtk_strbuf_push_c(buf, 0);
        ret = wtk_lexc_file_exist(lex, buf->data);
        //wtk_debug("[%s,ret=%d]\n",buf->data,ret);
        if (ret == 0) {
            char *p;

            p = wtk_heap_dup_str2(lex->heap, buf->data, buf->pos);
            wtk_strbuf_delete(buf);
            return p;
        }
    }
    wtk_strbuf_delete(buf);
    return 0;
}

void wtk_lexc_set_pwd(wtk_lexc_t *l, char *data, int bytes)
{
    //wtk_debug("[%.*s]\n",bytes,data);
    l->pwd = wtk_heap_dup_string(l->heap, data, bytes);
    wtk_str_hash_remove_s(l->str_var_hash, "pwd");
    wtk_str_hash_add_s(l->str_var_hash, "pwd", l->pwd);
}

int wtk_lexc_include_dir(wtk_lexc_t *l, char *dir);

int wtk_lexc_include_dir_file_name(wtk_lexc_t *l, char *fn)
{
    int len;

    len = strlen(fn);
    if (len > sizeof(".lex")
            && strcmp(fn + len - sizeof(".lex") + 1, ".lex") == 0) {
        return wtk_lexc_include_file(l, fn);
    }
    return 0;
}

int wtk_lexc_include_dir_file(wtk_lexc_t *l, char *dir)
{
    dir = wtk_heap_dup_str(l->heap, dir);

#ifdef WIN32
	return wtk_dir_walk(dir, (wtk_dir_walk_handler_t)wtk_lexc_include_dir_file_name, l);
#else
    return wtk_dir_walk2(dir, l,
            (wtk_dir_walk2_f) wtk_lexc_include_dir_file_name);
#endif
}

int wtk_lexc_include_dir_file2(wtk_lexc_t *l, char *dir)
{
    char *p;
    DIR *d;
    struct dirent *ent;
    int ret;
    wtk_strbuf_t *buf;
    wtk_string_t *dx;

    //wtk_debug("import dir:%s\n",dir);
    //dir=wtk_heap_dup_str(re->label->hash->heap,dir);
    dx = wtk_heap_dup_string(l->heap, dir, strlen(dir));
    buf = wtk_strbuf_new(1024, 1);
    d = opendir(dir);
    if (!d) {
        ret = -1;
        goto end;
    }
    ret = 0;
    while (1) {
        ent = readdir(d);
        if (!ent) {
            break;
        }
        //wtk_debug("[%s]\n",ent->d_name);
        if (ent->d_type == DT_REG) {
            wtk_strbuf_reset(buf);
            //wtk_strbuf_push_string(buf,dir);
            wtk_strbuf_push(buf, dx->data, dx->len);
            wtk_strbuf_push_c(buf, '/');
            wtk_strbuf_push_string(buf, ent->d_name);
            wtk_strbuf_push_c(buf, 0);
            //wtk_debug("%s\n",buf->data);
            //wtk_debug("cmp: %s:%d\n",buf->data+buf->pos-sizeof(".lex"),strcmp(buf->data+buf->pos-sizeof(".lex"),".lex"));
            if (buf->pos > sizeof(".lex")
                    && strcmp(buf->data + buf->pos - sizeof(".lex"), ".lex")
                            == 0) {
                p = wtk_heap_dup_str2(l->heap, buf->data, buf->pos);
                ret = wtk_lexc_include_file(l, p);
            } else {
                ret = 0;
            }
        } else if (ent->d_type == DT_DIR) {
            if (strcmp(ent->d_name, ".") == 0
                    || strcmp(ent->d_name, "..") == 0) {
                ret = 0;
            } else {
                wtk_strbuf_reset(buf);
                //wtk_strbuf_push_string(buf,dir);
                wtk_strbuf_push(buf, dx->data, dx->len);
                wtk_strbuf_push_c(buf, '/');
                wtk_strbuf_push_string(buf, ent->d_name);
                wtk_strbuf_push_c(buf, 0);
                //wtk_debug("%s[%s:%s]\n",buf->data,ent->d_name,dir);
                p = wtk_heap_dup_str2(l->heap, buf->data, buf->pos);
                ret = wtk_lexc_include_dir(l, p);
            }
        } else {
            ret = 0;
        }
        //wtk_debug("ret=%d\n",ret);
        if (ret != 0) {
            goto end;
        }
    }
    end:
    //exit(0);
    if (d) {
        closedir(d);
    }
    //wtk_debug("ret=%d\n",ret);
    wtk_strbuf_delete(buf);
    return ret;
}

int wtk_lexc_include_dir_rbin(wtk_lexc_t *l, char *dir)
{
    int ret;
    wtk_strbuf_t *buf;
    wtk_rbin2_t *rbin = l->rbin;
    wtk_queue_node_t *qn;
    wtk_rbin2_item_t *item;
    wtk_string_t v;
    wtk_string_t *vx;

    buf = wtk_strbuf_new(256, 1);
    vx = wtk_string_dup_data(dir, strlen(dir));
    //wtk_debug("import dir:%s\n",dir);
    for (qn = rbin->list.pop; qn; qn = qn->next) {
        item = data_offset(qn, wtk_rbin2_item_t, q_n);
        //wtk_debug("[%.*s],pos=%d,len=%d\n",item->fn->len,item->fn->data,item->pos,item->len);
        v = wtk_dir_name2(item->fn->data, item->fn->len, '/');
        //wtk_debug("[%.*s]=[%s]\n",v.len,v.data,dir);
        if (wtk_str_equal(v.data, v.len, vx->data, vx->len)) {
            //wtk_debug("[%.*s],pos=%d,len=%d\n",item->fn->len,item->fn->data,item->pos,item->len);
            wtk_strbuf_reset(buf);
            wtk_strbuf_push(buf, item->fn->data, item->fn->len);
            wtk_strbuf_push_c(buf, 0);
            ret = wtk_lexc_include_file(l, buf->data);
            //wtk_debug("ret=%d\n",ret);
            if (ret != 0) {
                goto end;
            }
        }
    }
    ret = 0;
    end: wtk_string_delete(vx);
    //exit(0);
    wtk_strbuf_delete(buf);
    return ret;
}

int wtk_lexc_include_dir(wtk_lexc_t *l, char *dir)
{
    if (l->rbin) {
        return wtk_lexc_include_dir_rbin(l, dir);
    } else {
        return wtk_lexc_include_dir_file(l, dir);
    }
}

int wtk_lexc_include_file(wtk_lexc_t *l, char *fn)
{
    wtk_lex_script_t *script;
    wtk_string_t *pwd;
    //wtk_string_t v;
    char *p;
    //char *data;
    //int len;
    int ret = -1;

    //wtk_debug("include %s\n",fn);
    if (l->rbin) {
		//sys 判断是否是lex
        if (!wtk_str_end_with_s(fn, strlen(fn), ".lex")) {
            ret = wtk_lexc_include_dir(l, fn);
            return ret;
        }
    } else {
        if (wtk_is_dir(fn)) {
            ret = wtk_lexc_include_dir(l, fn);
            return ret;
        }
    }
    p = wtk_lexc_get_real_file_name(l, fn);
    if (!p) {
        wtk_debug("load %s failed.\n", fn);
        return -1;
    }
    ///wtk_debug("%s\n",p);
    fn = p;
    pwd = l->pwd;
//	v=wtk_dir_name2(fn,strlen(fn),'/');
//	l->pwd=wtk_heap_dup_string(l->heap,v.data,v.len);
//	data=file_read_buf(fn,&len);
//	if(!data || len==0)
//	{
//		wtk_debug("load %s failed.\n",fn);
//		goto end;
//	}
    wtk_lexc_set_state(l, WTK_LEXC_INIT);
    //script=wtk_lexc_compile(l,data,len);
    script = wtk_lexc_compile_file(l, fn);
    if (!script) {
        ret = -1;
        goto end;
    }
    wtk_lexc_set_state(l, WTK_LEXC_INIT);
    l->pwd = pwd;
    wtk_lexc_set_pwd(l, pwd->data, pwd->len);
    ret = 0;
    end:
//	if(data)
//	{
//		wtk_free(data);
//	}
    return ret;
}

int wtk_lexc_feed_include(wtk_lexc_t *l, wtk_string_t *v)
{
    enum {
        WTK_LEXC_INCLUDE_INIT = 0, WTK_LEXC_INCLUDE_STRING,
    };

    switch (l->sub_state) {
        case WTK_LEXC_INCLUDE_INIT:
            wtk_lexc_set_string_state(l, WTK_LEXC_INCLUDE,
                    WTK_LEXC_INCLUDE_STRING);
            return wtk_lexc_feed(l, v);
            break;
        case WTK_LEXC_INCLUDE_STRING:
            //wtk_debug("[%.*s]\n",l->tok->pos,l->tok->data);
            wtk_strbuf_push_c(l->tok, 0);
            return wtk_lexc_include_file(l, l->tok->data);
            break;
    }
    return 0;
}

int wtk_lexc_feed_import(wtk_lexc_t *l, wtk_string_t *v)
{
    enum {
        WTK_LEXC_IMPORT_INIT = 0,
        WTK_LEXC_IMPORT_STRING,
        WTK_LEXC_IMPORT_AS,
        WTK_LEXC_IMPORT_NAME,
    };
    wtk_string_t *lib;
    int ret;

    switch (l->sub_state) {
        case WTK_LEXC_IMPORT_INIT:
            wtk_lexc_set_string_state(l, WTK_LEXC_IMPORT,
                    WTK_LEXC_IMPORT_STRING);
            return wtk_lexc_feed(l, v);
            break;
        case WTK_LEXC_IMPORT_STRING:
//		wtk_debug("[%.*s]\n",l->tok->pos,l->tok->data);
            wtk_strbuf_push_c(l->tok, 0);
            l->str_key = wtk_heap_dup_string(l->heap, l->tok->data,
                    l->tok->pos);
            //return wtk_lexc_include_file(l,l->tok->data);
            wtk_lexc_set_string_state(l, WTK_LEXC_IMPORT, WTK_LEXC_IMPORT_AS);
            return wtk_lexc_feed(l, v);
            break;
        case WTK_LEXC_IMPORT_AS://限制头
            if (wtk_str_equal_s(l->tok->data, l->tok->pos, "as")) {
                wtk_lexc_set_string_state(l, WTK_LEXC_IMPORT,
                        WTK_LEXC_IMPORT_NAME);
                return wtk_lexc_feed(l, v);
            } else {
                return -1;
            }
            break;
        case WTK_LEXC_IMPORT_NAME:
            lib = l->lib;
            l->lib = wtk_heap_dup_string(l->heap, l->tok->data, l->tok->pos);
            wtk_lex_script_add_lib(l->script, l->lib->data, l->lib->len);
            ret = wtk_lexc_include_file(l, l->str_key->data);
            l->lib = lib;
            return ret;
            break;
    }
    return 0;
}

wtk_string_t* wtk_lexc_get_string_var(wtk_lexc_t *l, char *data, int len)
{
    return (wtk_string_t*) wtk_str_hash_find(l->str_var_hash, data, len);
}

int wtk_lexc_feed_string(wtk_lexc_t *l, wtk_string_t *v)
{
    enum {
        WTK_LEXC_STRING_INIT = 0,
        WTK_LEXC_STRING_WAIT_END,
        WTK_LEXC_STRING_WAIT_QUOTE_END,
        WTK_LEXC_STRING_ESC,
        WTK_LEXC_STRING_ESC_X1,
        WTK_LEXC_STRING_ESC_X2,
        WTK_LEXC_STRING_DOLLAR,
        WTK_LEXC_STRING_WAIT_VAR,
        WTK_LEXC_STRING_VAR,
        WTK_LEXC_STRING_VAR_WAIT_END,
    };
    wtk_strbuf_t *buf = l->tok;
    wtk_lexc_state_string_env_t *env;
    char c;
    int ret;

    env = &(l->string_env);
    switch (l->sub_state) {
        case WTK_LEXC_STRING_INIT:
            if (v->len > 1 || !isspace(v->data[0])) {
                wtk_strbuf_reset(buf);
                if (v->len == 1 && (v->data[0] == '\'' || v->data[0] == '"')) {
                    env->quoted = 1;
                    env->quoted_char = v->data[0];
                    l->sub_state = WTK_LEXC_STRING_WAIT_QUOTE_END;
                } else {
                    env->quoted = 0;
                    //wtk_strbuf_push(buf,v->data,v->len);
                    l->sub_state = WTK_LEXC_STRING_WAIT_END;
                    wtk_lexc_feed(l, v);
                }
            }
            break;
        case WTK_LEXC_STRING_WAIT_END:
            if (v->len == 1) {
                c = v->data[0];
                //wtk_debug("%c\n",c);
                if (c == '$') {
                    l->sub_state = WTK_LEXC_STRING_DOLLAR;
                } else if (c == '\\') {
                    l->sub_state = WTK_LEXC_STRING_ESC;
                } else if (isspace(c) || c == '=' || c == ')' || c == '('
                        || c == '[' || c == ']' || c == '{' || c == '}'
                        || c == '<' || c == '>' || c == ';' || c == ','
                        || c == '/') {
                    //wtk_debug("[%.*s]\n",buf->pos,buf->data);
                    wtk_lexc_restore_string_state(l);
                    wtk_lexc_feed(l, v);
                    return 0;
                } else {
                    wtk_strbuf_push(buf, v->data, v->len);
                }
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
        case WTK_LEXC_STRING_WAIT_QUOTE_END:
            if (v->len == 1) {
                if (v->data[0] == '$') {
                    l->sub_state = WTK_LEXC_STRING_DOLLAR;
                } else if (v->data[0] == '\\') {
                    l->sub_state = WTK_LEXC_STRING_ESC;
                } else if (v->data[0] == env->quoted_char) {
                    //wtk_debug("[%.*s]\n",buf->pos,buf->data);
                    wtk_lexc_restore_string_state(l);
                    return 0;
                } else {
                    wtk_strbuf_push(buf, v->data, v->len);
                }
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
        case WTK_LEXC_STRING_ESC:
            if (v->len == 1) {
                c = v->data[0];
                switch (c) {
                    case 'x':
                    case 'X':
                        l->sub_state = WTK_LEXC_STRING_ESC_X1;
                        break;
                    case 'n':
                        wtk_strbuf_push_c(buf, '\n')
                        ;
                        l->sub_state =
                                env->quoted ?
                                        WTK_LEXC_STRING_WAIT_QUOTE_END :
                                        WTK_LEXC_STRING_WAIT_END;
                        break;
                    case 'r':
                        wtk_strbuf_push_c(buf, '\r')
                        ;
                        l->sub_state =
                                env->quoted ?
                                        WTK_LEXC_STRING_WAIT_QUOTE_END :
                                        WTK_LEXC_STRING_WAIT_END;
                        break;
                    case 't':
                        wtk_strbuf_push_c(buf, '\t')
                        ;
                        l->sub_state =
                                env->quoted ?
                                        WTK_LEXC_STRING_WAIT_QUOTE_END :
                                        WTK_LEXC_STRING_WAIT_END;
                        break;
                    case 'b':
                        wtk_strbuf_push_c(buf, '\b')
                        ;
                        l->sub_state =
                                env->quoted ?
                                        WTK_LEXC_STRING_WAIT_QUOTE_END :
                                        WTK_LEXC_STRING_WAIT_END;
                        break;
                    case 'f':
                        wtk_strbuf_push_c(buf, '\f')
                        ;
                        l->sub_state =
                                env->quoted ?
                                        WTK_LEXC_STRING_WAIT_QUOTE_END :
                                        WTK_LEXC_STRING_WAIT_END;
                        break;
                    case '\\':
                        wtk_strbuf_push_c(buf, '\\')
                        ;
                        l->sub_state =
                                env->quoted ?
                                        WTK_LEXC_STRING_WAIT_QUOTE_END :
                                        WTK_LEXC_STRING_WAIT_END;
                        break;
                    default:
                        wtk_strbuf_push_c(buf, c)
                        ;
                        l->sub_state =
                                env->quoted ?
                                        WTK_LEXC_STRING_WAIT_QUOTE_END :
                                        WTK_LEXC_STRING_WAIT_END;
                        break;
                }
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
                l->sub_state =
                        env->quoted ?
                                WTK_LEXC_STRING_WAIT_QUOTE_END :
                                WTK_LEXC_STRING_WAIT_END;
            }
            break;
        case WTK_LEXC_STRING_ESC_X1:
            if (v->len > 1) {
                return -1;
            }
            c = v->data[0];
            ret = wtk_char_to_hex(c);
            if (ret == -1) {
                return -1;
            }
            env->hex1 = ret;
            l->sub_state = WTK_LEXC_STRING_ESC_X2;
            break;
        case WTK_LEXC_STRING_ESC_X2:
            if (v->len > 1) {
                return -1;
            }
            c = v->data[0];
            ret = wtk_char_to_hex(c);
            if (ret == -1) {
                return -1;
            }
            c = (env->hex1 << 4) + ret;
            wtk_strbuf_push_c(buf, c)
            ;
            l->sub_state =
                    env->quoted ?
                            WTK_LEXC_STRING_WAIT_QUOTE_END :
                            WTK_LEXC_STRING_WAIT_END;
            break;
        case WTK_LEXC_STRING_DOLLAR:
            if (v->len == 1 && v->data[0] == '{') {
                l->sub_state = WTK_LEXC_STRING_WAIT_VAR;
                return 0;
            } else {
                l->sub_state =
                        env->quoted ?
                                WTK_LEXC_STRING_WAIT_QUOTE_END :
                                WTK_LEXC_STRING_WAIT_END;
                wtk_strbuf_push_c(buf, '$');
                return wtk_lexc_feed_string(l, v);
            }
            break;
        case WTK_LEXC_STRING_WAIT_VAR:
            if (v->len > 1 || !isspace(v->data[0])) {
                wtk_strbuf_reset(l->buf);
                wtk_strbuf_push(l->buf, v->data, v->len);
                l->sub_state = WTK_LEXC_STRING_VAR;
            }
            break;
        case WTK_LEXC_STRING_VAR:
            c = v->data[0];
            if (v->len == 1 && (c == '}' || isspace(c))) {
                if (c == '}') {
                    v = wtk_lexc_get_string_var(l, l->buf->data, l->buf->pos);//这里会进行扩展
                    if (!v) {
                        wtk_debug("var [%.*s] not found\n", l->buf->pos,
                                l->buf->data);
                        return -1;
                    }
                    //wtk_debug("[%.*s]\n",v->len,v->data);
                    wtk_strbuf_push(buf, v->data, v->len);
                    l->sub_state =
                            env->quoted ?
                                    WTK_LEXC_STRING_WAIT_QUOTE_END :
                                    WTK_LEXC_STRING_WAIT_END;
                } else {
                    l->sub_state = WTK_LEXC_STRING_VAR_WAIT_END;
                }
            } else {
                wtk_strbuf_push(l->buf, v->data, v->len);
            }
            break;
        case WTK_LEXC_STRING_VAR_WAIT_END:
            if (v->len == 1 && v->data[0] == '}') {
                v = wtk_lexc_get_string_var(l, l->buf->data, l->buf->pos);
                if (!v) {
                    wtk_debug("var [%.*s] not found\n", l->buf->pos,
                            l->buf->data);
                    return -1;
                }
                //wtk_debug("[%.*s]\n",v->len,v->data);
                wtk_strbuf_push(buf, v->data, v->len);
                l->sub_state =
                        env->quoted ?
                                WTK_LEXC_STRING_WAIT_QUOTE_END :
                                WTK_LEXC_STRING_WAIT_END;
            }
            break;
    }
    return 0;
}

wtk_lexc_env_stk_t* wtk_lexc_pop_env_stk(wtk_lexc_t *l)
{
    return (wtk_lexc_env_stk_t*) wtk_vpool_pop(l->stk_pool);
}

void wtk_lexc_push_env_stk(wtk_lexc_t *l, wtk_lexc_env_stk_t *stk)
{
    wtk_vpool_push(l->stk_pool, stk);
}

wtk_lexc_scope_stk_t* wtk_lexc_pop_scope_stk(wtk_lexc_t *l)
{
    return (wtk_lexc_scope_stk_t*) wtk_vpool_pop(l->scope_pool);
}

void wtk_lexc_push_scope_stk(wtk_lexc_t *l, wtk_lexc_scope_stk_t *stk)
{
    wtk_vpool_push(l->scope_pool, stk);
}

wtk_lex_expr_item_value_t* wtk_lexc_get_last_value(wtk_lexc_t *l)
{
    wtk_lexc_env_stk_t *stk;
    wtk_lex_expr_item_value_t *value;

    stk = data_offset2(l->value_stk_q.push, wtk_lexc_env_stk_t, q_n);
    value = data_offset2(stk->value_q->push, wtk_lex_expr_item_value_t, q_n);
    return value;
}

int wtk_lexc_close_expr_value(wtk_lexc_t *l)
{
    wtk_lexc_env_stk_t *stk;
    wtk_queue_node_t *qn;

    if (l->value_stk_q.length > 1) {
        return -1;
    }
    //if(l->value_stk_q.length==0){return 0;};
    qn = wtk_queue_pop_back(&(l->value_stk_q));
    stk = data_offset2(qn, wtk_lexc_env_stk_t, q_n);
    wtk_lex_script_close_expr(l->script, l->cur_expr->v.expr);
    wtk_lexc_push_env_stk(l, stk);
    //wtk_lex_expr_item_print(l->cur_expr->v.expr);
    return 0;
}

char* wtk_lexc_get_expr_state(int s)
{
    static char* ns[] = { "WTK_LEXC_EXPR_NAME_INIT", "WTK_LEXC_EXPR_NAME_END",
            "WTK_LEXC_EXPR_EXPORT_NAME_END", "WTK_LEXC_EXPR_WAIT_EQ",
            "WTK_LEXC_EXPR_WAIT_VALUE", "WTK_LEXC_EXPR_VALUE",
            "WTK_LEXC_EXPR_VALUE_COUNT_WAIT_MIN",
            "WTK_LEXC_EXPR_VALUE_COUNT_WAIT_COMMER",
            "WTK_LEXC_EXPR_VALUE_COUNT_WAIT_MAX",
            "WTK_LEXC_EXPR_VALUE_COUNT_WAIT_END", "WTK_LEXC_EXPR_VALUE_BRACKET",
            "WTK_LEXC_EXPR_VALUE_BRACKET_ESC",
            "WTK_LEXC_EXPR_VALUE_BRACKET_RANGE", "WTK_LEXC_EXPR_VALUE_HEX",
            "WTK_LEXC_EXPR_VALUE_HEX1", "WTK_LEXC_EXPR_VALUE_HEX2",
            "WTK_LEXC_EXPR_VALUE_ATTR", "WTK_LEXC_EXPR_VALUE_ATTR_WAIT",
            "WTK_LEXC_EXPR_VALUE_ATTR_WAIT_VALUE",
            "WTK_LEXC_EXPR_VALUE_ATTR_VALUE", "WTK_LEXC_EXPR_VALUE_DOLLAR",
            "WTK_LEXC_EXPR_VALUE_WIAT_OUTPUT",
            "WTK_LEXC_EXPR_VALUE_OUTPUT_NAME",
            "WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_PARAM",
            "WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_KV",
            "WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_KV_EQ",
            "WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_VALUE",
            "WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_VALUE_ATTR",
            "WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_VALUE_ATTR_INIT",
            "WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_VALUE_ATTR_KEY",
            "WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_VALUE_ATTR_EQ",
            "WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_VALUE_ATTR_V",
            "WTK_LEX_EXPR_WAIT_ATTR", "WTK_LEX_EXPR_WAIT_PROB",
            "WTK_LEX_EXPR_WAIT_PROB_END", "WTK_LEX_EXPR_ATTR_WAIT_KEY",
            "WTK_LEX_EXPR_ATTR_KEY", "WTK_LEX_EXPR_ATTR_WAIT_VALUE",
            "WTK_LEX_EXPR_ATTR_VALUE", "WTK_LEXC_EXPR_VALUE_DOLLAR_VALUE",
            "WTK_LEXC_EXPR_VALUE_DOLLAR_VALUE_WAIT_END", };
    return ns[s];
}

void wtk_lexc_finish_expr(wtk_lexc_t *l)
{
    wtk_lex_script_close_expr(l->script, l->cur_expr->v.expr);
    l->cur_expr = NULL;
    wtk_lexc_set_state(l, WTK_LEXC_INIT);
    //wtk_lex_script_print(l->script);
}

void wtk_lexc_add_new_expr(wtk_lexc_t *l, char *name, int bytes, int expt)
{
    wtk_lexc_scope_stk_t *stk;

    //wtk_debug("[%.*s]=%d\n",l->tok->pos,l->tok->data,l->scope_stk_q.length);
    if (l->scope_stk_q.length > 0) {
        stk = data_offset2(l->scope_stk_q.push, wtk_lexc_scope_stk_t, q_n);
        l->cur_expr = wtk_lex_script_new_expr2(l->script, name, bytes);
        if (stk->yes) {
            wtk_queue_push(&(stk->scope->yes), &(l->cur_expr->q_n));
        } else {
            wtk_queue_push(&(stk->scope->no), &(l->cur_expr->q_n));
        }
    } else {
        l->cur_expr = wtk_lex_script_new_expr(l->script, l->lib, name, bytes,
                expt);
    }
}

void wtk_lexc_attrs_str_notify(wtk_array_t *a, wtk_string_t *k, wtk_string_t *v)
{
    //wtk_debug("k=%p v=%p\n",k,v);
    //wtk_debug("[%.*s]\n",k->len,k->data);
    v = wtk_heap_dup_string(a->heap, k->data, k->len);
    //wtk_debug("v=%.*s v=%p a=%p\n",v->len,v->data,v,a)
    wtk_array_push2(a, &(v));
}

wtk_array_t* wtk_lexc_get_attr_strs(wtk_lexc_t *l, char *data, int bytes)
{
    wtk_heap_t *heap = l->script->heap;
    wtk_strbuf_t* buf;
    wtk_array_t *a;

    //wtk_debug("[%.*s]\n",bytes,data);
    a = wtk_array_new_h(heap, bytes / 3, sizeof(wtk_string_t*));
    buf = wtk_strbuf_new(256, 1);
    wtk_strbuf_push_s(buf, "[");
    wtk_strbuf_push(buf, data, bytes);
    wtk_strbuf_push_s(buf, "]");
    wtk_str_attr_parse(buf->data, buf->pos, a,
            (wtk_str_attr_f) wtk_lexc_attrs_str_notify);
    wtk_strbuf_delete(buf);
    //exit(0);
    return a;
}

typedef enum {
    WTK_LEXC_EXPR_NAME_INIT = 0,
    WTK_LEXC_EXPR_NAME_END,
    WTK_LEXC_EXPR_EXPORT_NAME_END,
    WTK_LEXC_EXPR_WAIT_EQ,
    WTK_LEXC_EXPR_WAIT_VALUE,
    WTK_LEXC_EXPR_VALUE,
    WTK_LEXC_EXPR_VALUE_ENG,
    WTK_LEXC_EXPR_VALUE_QUOTE,
    WTK_LEXC_EXPR_VALUE_COUNT_WAIT_MIN,
    WTK_LEXC_EXPR_VALUE_COUNT_WAIT_COMMER,
    WTK_LEXC_EXPR_VALUE_COUNT_WAIT_MAX,
    WTK_LEXC_EXPR_VALUE_COUNT_WAIT_END,
    WTK_LEXC_EXPR_VALUE_BRACKET,
    WTK_LEXC_EXPR_VALUE_BRACKET_ESC,
    WTK_LEXC_EXPR_VALUE_BRACKET_RANGE,
    WTK_LEXC_EXPR_VALUE_HEX,
    WTK_LEXC_EXPR_VALUE_HEX1,
    WTK_LEXC_EXPR_VALUE_HEX2,
    WTK_LEXC_EXPR_VALUE_ATTR,
    WTK_LEXC_EXPR_VALUE_ATTR_WAIT,
    WTK_LEXC_EXPR_VALUE_ATTR_WAIT_VALUE,
    WTK_LEXC_EXPR_VALUE_ATTR_VALUE,
    WTK_LEXC_EXPR_VALUE_DOLLAR,
    WTK_LEXC_EXPR_VALUE_WIAT_OUTPUT,
    WTK_LEXC_EXPR_VALUE_OUTPUT_NAME,
    WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_PARAM,
    WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_KV,
    WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_KV_EQ,
    WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_VALUE,
    WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_VALUE_ATTR,
    WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_VALUE_ATTR_INIT,
    WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_VALUE_ATTR_KEY,
    WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_VALUE_ATTR_EQ,
    WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_VALUE_ATTR_V,
    WTK_LEX_EXPR_WAIT_ATTR,
    WTK_LEX_EXPR_WAIT_PROB,
    WTK_LEX_EXPR_WAIT_PROB_END,
    WTK_LEX_EXPR_ATTR_WAIT_KEY,
    WTK_LEX_EXPR_ATTR_KEY,
    WTK_LEX_EXPR_ATTR_WAIT_VALUE,
    WTK_LEX_EXPR_ATTR_VALUE,
    WTK_LEXC_EXPR_VALUE_DOLLAR_VALUE,
    WTK_LEXC_EXPR_VALUE_DOLLAR_VALUE_WAIT_END,
} wtk_lexc_expr_state_t;

void wtk_lex_expr_output_trans_filter_func_set_attr(wtk_lexc_t *l,
        wtk_string_t *k, wtk_string_t *v)
{
    wtk_lex_expr_output_trans_filter_func_t *func;

    if (l->trans_tk->output_if_yes) {
        func = &(l->trans_tk->cur_trans_filter->yes.v.func);
    } else {
        func = &(l->trans_tk->cur_trans_filter->no.v.func);
    }
    //wtk_debug("[%.*s]=[%.*s]\n",k->len,k->data,v->len,v->data);
    if (wtk_string_cmp_s(k,"pre") == 0) {
        func->pre = wtk_heap_dup_string(l->script->heap, v->data, v->len);
        //wtk_debug("%p,[%.*s]\n",func,func->pre->len,func->pre->data);
    } else if (wtk_string_cmp_s(k,"suc") == 0) {
        func->suc = wtk_heap_dup_string(l->script->heap, v->data, v->len);
    }
}

wtk_lexc_trans_stk_t* wtk_lexc_new_trans_stk(wtk_lexc_t *l)
{
    wtk_lexc_trans_stk_t *stk;

    stk = (wtk_lexc_trans_stk_t*) wtk_heap_malloc(l->heap,
            sizeof(wtk_lexc_trans_stk_t));
    stk->cur_trans_filter = NULL;
    stk->cur_trans_if = NULL;
    stk->output_if_cap = 0;
    stk->output_if_yes = 0;
    return stk;
}

void wtk_lexc_push_trans_stk(wtk_lexc_t *l, int sub_state)
{
    l->trans_tk->sub_state = sub_state;
    wtk_queue_push(&(l->trans_stk_q), &(l->trans_tk->q_n));
    l->trans_tk = wtk_lexc_new_trans_stk(l);
}

wtk_lexc_trans_stk_t* wtk_lexc_pop_trans_stk(wtk_lexc_t *l)
{
    wtk_queue_node_t *qn;
    wtk_lexc_trans_stk_t *stk;

    qn = wtk_queue_pop(&(l->trans_stk_q));
    stk = data_offset2(qn, wtk_lexc_trans_stk_t, q_n);
    l->sub_state = stk->sub_state;
    return stk;
}

int wtk_lexc_feed_expr_output_trans(wtk_lexc_t *l, wtk_string_t *v)
{
    enum wtk_lexc_expr_state_t {
        WTK_LEXC_EXPR_OUTPUT_WAIT = 0,
        WTK_LEXC_EXPR_OUTPUT_VALUE,
        WTK_LEXC_EXPR_OUTPUT_CAP,
        WTK_LEXC_EXPR_OUTPUT_ESC,
        WTK_LEXC_EXPR_OUTPUT_ATTR,
        WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_NAME,
        WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_WAIT_DOLLAR,
        WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_WAIT_JUDGE,
        WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_WAIT_JUDGE2,
        WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_WAIT_JUDGE_VALUE,
        WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_JUDGE_VALUE,
        WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_JUDGE_VALUE_ESC,
        WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_JUDGE_VALUE_END,
        WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_WAIT_COND,
        WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_WAIT_EXPR,
        WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_VALUE,
        WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_VALUE_END,
        WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_VALUE_ATTR,
    };
    wtk_strbuf_t *tok = l->tok;
    wtk_lex_expr_output_t *output = &(l->cur_expr->v.expr->output);
    wtk_lex_expr_output_trans_item_t *item;
    wtk_lex_expr_output_trans_filter_t *filter;
    wtk_lexc_trans_stk_t *stk;
    char c;
    int i;

    stk = l->trans_tk;
    //wtk_debug("[%.*s]=%d\n",v->len,v->data,l->sub_state);
    switch (l->sub_state) {
        case WTK_LEXC_EXPR_OUTPUT_WAIT:
            if (v->len > 1 || !isspace(*v->data)) {
                l->sub_state = WTK_LEXC_EXPR_OUTPUT_VALUE;
                l->trans_tk = wtk_lexc_new_trans_stk(l);
                wtk_strbuf_reset(tok);
                return wtk_lexc_feed_expr_output_trans(l, v);
            }
            break;
        case WTK_LEXC_EXPR_OUTPUT_VALUE:
            if (v->len > 1) {
                wtk_strbuf_push(tok, v->data, v->len);
            } else {
                c = v->data[0];
                if (c == '\n' || c == ';') {
                    if (tok->pos > 0) {
                        item = wtk_lex_script_new_output_trans_str_item(
                                l->script, tok->data, tok->pos);
                        wtk_queue_push(&(output->item_q), &(item->q_n));
                    }
                    wtk_lexc_finish_expr(l);
                } else if (isspace(c)) {
                    if (tok->pos > 0) {
                        item = wtk_lex_script_new_output_trans_str_item(
                                l->script, tok->data, tok->pos);
                        wtk_queue_push(&(output->item_q), &(item->q_n));
                    }
                    wtk_lexc_set_state(l, WTK_LEXC_EXPR_NAME);
                    l->sub_state = WTK_LEX_EXPR_WAIT_ATTR;
                    if (l->trans_stk_q.length > 0) {
                        return -1;
                    }
                } else if (c == '$') {
                    if (tok->pos > 0) {
                        item = wtk_lex_script_new_output_trans_str_item(
                                l->script, tok->data, tok->pos);
                        wtk_queue_push(&(output->item_q), &(item->q_n));
                    }
                    wtk_strbuf_reset(tok);
                    l->sub_state = WTK_LEXC_EXPR_OUTPUT_CAP;
                } else if (c == '\\') {
                    l->sub_state = WTK_LEXC_EXPR_OUTPUT_ESC;
                } else if (c == '/') {
                    l->sub_state = WTK_LEXC_EXPR_OUTPUT_ATTR;
                } else {
                    wtk_strbuf_push(tok, v->data, v->len);
                }
            }
            break;
        case WTK_LEXC_EXPR_OUTPUT_ESC:
            wtk_strbuf_push(tok, v->data, v->len);
            l->sub_state = WTK_LEXC_EXPR_OUTPUT_VALUE;
            break;
        case WTK_LEXC_EXPR_OUTPUT_CAP:
            if (v->len == 1 && isdigit(v->data[0])) {
                wtk_strbuf_push(tok, v->data, v->len);
            } else {
                wtk_strbuf_push_c(tok, 0);
                i = atoi(tok->data);
                item = wtk_lex_script_new_output_trans_cap_item(l->script, i);
                wtk_queue_push(&(output->item_q), &(item->q_n));
                l->sub_state = WTK_LEXC_EXPR_OUTPUT_VALUE;
                wtk_strbuf_reset(tok);
                return wtk_lexc_feed_expr_output_trans(l, v);
            }
            break;
        case WTK_LEXC_EXPR_OUTPUT_ATTR:
            if (v->len == 1 || !isspace(v->data[0])) {
                c = v->data[0];
                //wtk_debug("c=%.*s\n",v->len,v->data);
                if (c == '/') {
                    wtk_strbuf_reset(tok);
                    l->sub_state = WTK_LEXC_EXPR_OUTPUT_VALUE;
                } else {
                    filter = wtk_lex_script_new_trans_filter(l->script);
                    item = data_offset2(output->item_q.push,
                            wtk_lex_expr_output_trans_item_t, q_n);
                    wtk_queue_push(&(item->filter_q), &(filter->q_n));
                    stk->cur_trans_filter = filter;
                    switch (c) {
                        case '(':
                            stk->output_if_cap = 1;
                            l->sub_state =
                                    WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_WAIT_DOLLAR;
                            break;
                        case '$':
                            stk->output_if_cap = 0;
                            l->sub_state =
                                    WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_WAIT_DOLLAR;
                            return wtk_lexc_feed_expr_output_trans(l, v);
                            break;
                        default:
                            l->sub_state = WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_NAME;
                            wtk_strbuf_reset(tok);
                            wtk_strbuf_push(tok, v->data, v->len);
                            break;
                    }
                }
            }
            break;
        case WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_NAME:
            c = v->data[0];
            if (v->len == 1 && (c == ';' || c == '/')) {
                wtk_lex_expr_output_trans_filter_item_set_func(l->script,
                        &(stk->cur_trans_filter->yes), tok->data, tok->pos);
                wtk_strbuf_reset(tok);
                if (c == ';') {
                    l->sub_state = WTK_LEXC_EXPR_OUTPUT_ATTR;
                } else {
                    l->sub_state = WTK_LEXC_EXPR_OUTPUT_VALUE;
                }
            } else {
                if (v->len > 1 || !isspace(c)) {
                    wtk_strbuf_push(tok, v->data, v->len);
                }
            }
            break;
        case WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_WAIT_DOLLAR:
            if (v->len == 1 && v->data[0] == '$') {
                stk->cur_trans_if = wtk_lex_script_new_output_trans_if(
                        l->script);
                l->sub_state = WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_WAIT_JUDGE;
                //wtk_debug("stk=%p\n",stk);
                wtk_queue_push(&(stk->cur_trans_filter->if_q),
                        &(stk->cur_trans_if->q_n));
            }
            break;
        case WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_WAIT_JUDGE:
            if (v->len == 1) {
                c = v->data[0];
                switch (c) {
                    case '>':
                        stk->cur_trans_if->if_type =
                                WTK_LEX_EXPR_OUTPUT_TRANS_IF_GT;
                        l->sub_state =
                                WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_WAIT_JUDGE2;
                        break;
                    case '=':
                        stk->cur_trans_if->if_type =
                                WTK_LEX_EXPR_OUTPUT_TRANS_IF_EQ;
                        l->sub_state =
                                WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_WAIT_JUDGE2;
                        break;
                    case '<':
                        stk->cur_trans_if->if_type =
                                WTK_LEX_EXPR_OUTPUT_TRANS_IF_LT;
                        l->sub_state =
                                WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_WAIT_JUDGE2;
                        break;
                    default:
                        break;
                }
            }
            break;
        case WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_WAIT_JUDGE2:
            if (v->len == 1 && v->data[0] == '=') {
                switch (stk->cur_trans_if->if_type) {
                    case WTK_LEX_EXPR_OUTPUT_TRANS_IF_LT:
                        stk->cur_trans_if->if_type =
                                WTK_LEX_EXPR_OUTPUT_TRANS_IF_LE;
                        break;
                    case WTK_LEX_EXPR_OUTPUT_TRANS_IF_GT:
                        stk->cur_trans_if->if_type =
                                WTK_LEX_EXPR_OUTPUT_TRANS_IF_GE;
                        break;
                    default:
                        break;
                }
                l->sub_state =
                        WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_WAIT_JUDGE_VALUE;
            } else {
                l->sub_state =
                        WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_WAIT_JUDGE_VALUE;
                return wtk_lexc_feed_expr_output_trans(l, v);
            }
            break;
        case WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_WAIT_JUDGE_VALUE:
            if (v->len > 1 || !isspace(v->data[0])) {
                wtk_strbuf_reset(tok);
                l->sub_state = WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_JUDGE_VALUE;
                return wtk_lexc_feed_expr_output_trans(l, v);
            }
            break;
        case WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_JUDGE_VALUE:
            if (v->len > 1) {
                wtk_strbuf_push(tok, v->data, v->len);
            } else {
                c = v->data[0];
                if (c == ')' || c == '?' || c == '|' || c == '&') {
                    wtk_lex_expr_output_trans_if_set_value(l->script,
                            stk->cur_trans_if, tok->data, tok->pos);
                    l->sub_state =
                            WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_JUDGE_VALUE_END;
                    return wtk_lexc_feed_expr_output_trans(l, v);
                } else if (isspace(c)) {
                    wtk_lex_expr_output_trans_if_set_value(l->script,
                            stk->cur_trans_if, tok->data, tok->pos);
                    l->sub_state =
                            WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_JUDGE_VALUE_END;
                } else {
                    wtk_strbuf_push(tok, v->data, v->len);
                }
            }
            break;
        case WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_JUDGE_VALUE_ESC:
            wtk_strbuf_push(tok, v->data, v->len);
            l->sub_state = WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_JUDGE_VALUE;
            break;
        case WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_JUDGE_VALUE_END:
            c = v->data[0];
            switch (c) {
                case ')':
                    if (stk->output_if_cap) {
                        l->sub_state =
                                WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_WAIT_COND;
                    } else {
                        return -1;
                    }
                    break;
                case '?':
                    stk->output_if_yes = 1;
                    l->sub_state = WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_WAIT_EXPR;
                    break;
                case '|':
                    stk->cur_trans_filter->or = 1;
                    l->sub_state =
                            WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_WAIT_DOLLAR;
                    break;
                case '&':
                    stk->cur_trans_filter->or = 0;
                    l->sub_state =
                            WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_WAIT_DOLLAR;
                    break;
            }
            break;
        case WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_WAIT_COND:
            c = v->data[0];
            if (c == '?') {
                stk->output_if_yes = 1;
                l->sub_state = WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_WAIT_EXPR;
            }
            break;
        case WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_WAIT_EXPR:
            c = v->data[0];
            if (c == '(') {
                filter = wtk_lex_script_new_trans_filter(l->script);
                if (stk->output_if_yes) {
                    wtk_lex_expr_output_trans_filter_item_set_filter(
                            &(stk->cur_trans_filter->yes), filter);
                } else {
                    wtk_lex_expr_output_trans_filter_item_set_filter(
                            &(stk->cur_trans_filter->no), filter);
                }
                wtk_lexc_push_trans_stk(l,
                        WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_VALUE_END);
                stk = l->trans_tk;
                stk->cur_trans_filter = filter;
                //wtk_debug("[%.*s]\n",v->len,v->data);
                l->sub_state = WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_WAIT_DOLLAR;
            } else if (!isspace(c)) {
                l->sub_state = WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_VALUE;
                wtk_strbuf_reset(tok);
                wtk_strbuf_push(tok, v->data, v->len);
            }
            break;
        case WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_VALUE:
            if (v->len == 1) {
                c = v->data[0];
                if (c == '[' || c == ':' || c == ';' || c == ')' || c == '/') {
                    if (stk->output_if_yes) {
                        wtk_lex_expr_output_trans_filter_item_set_func(
                                l->script, &(stk->cur_trans_filter->yes),
                                tok->data, tok->pos);
                    } else {
                        wtk_lex_expr_output_trans_filter_item_set_func(
                                l->script, &(stk->cur_trans_filter->no),
                                tok->data, tok->pos);
                    }
                    wtk_strbuf_reset(tok);
                    switch (c) {
                        case ')':
                            wtk_lexc_pop_trans_stk(l);
                            break;
                        case '[':
                            l->sub_state =
                                    WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_VALUE_ATTR;
                            break;
                        case ':':
                            if (stk->output_if_yes) {
                                stk->output_if_yes = 0;
                                l->sub_state =
                                        WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_WAIT_EXPR;
                            } else {
                                return -1;
                            }
                            break;
                        case ';':
                            l->sub_state = WTK_LEXC_EXPR_OUTPUT_ATTR;
                            break;
                        case '/':
                            l->sub_state = WTK_LEXC_EXPR_OUTPUT_VALUE;
                            break;
                    }
                } else {
                    if (!isspace(c)) {
                        wtk_strbuf_push(tok, v->data, v->len);
                    }
                }
            } else {
                wtk_strbuf_push(tok, v->data, v->len);
            }
            break;
        case WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_VALUE_END:
            if (v->len == 1) {
                c = v->data[0];
                //wtk_debug("yes=%d\n",l->output_if_yes);
                if (c == ':') {
                    if (stk->output_if_yes) {
                        stk->output_if_yes = 0;
                        l->sub_state =
                                WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_WAIT_EXPR;
                    } else {
                        return -1;
                    }
                } else if (c == ';') {
                    wtk_strbuf_reset(tok);
                    l->sub_state = WTK_LEXC_EXPR_OUTPUT_ATTR;
                }
            }
            break;
        case WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_VALUE_ATTR:
            if (v->len == 1 && v->data[0] == ']') {
                wtk_strbuf_push_front_s(tok, "[");
                wtk_strbuf_push_s(tok, "]");
                //wtk_debug("[%.*s]\n",tok->pos,tok->data);
                wtk_str_attr_parse(tok->data, tok->pos, l,
                        (wtk_str_attr_f) wtk_lex_expr_output_trans_filter_func_set_attr);
                wtk_strbuf_reset(tok);
                l->sub_state = WTK_LEXC_EXPR_OUTPUT_ATTR_FUNC_IF_VALUE_END;
            } else {
                wtk_strbuf_push(tok, v->data, v->len);
            }
            break;
    }
    return 0;
}

int wtk_lexc_feed_expr_name(wtk_lexc_t *l, wtk_string_t *v)
{
    wtk_lexc_env_stk_t *stk;
    wtk_lex_expr_item_value_t *value, *value2;
    wtk_lex_expr_branch_t *branch;
    wtk_queue_node_t *qn;
    wtk_lex_expr_repeat_t *repeat;
    char c;
    int ret = 0;
    wtk_heap_t *heap = l->script->heap;
    wtk_strbuf_t *tok = l->tok;

    //wtk_debug("[%.*s]=%d /%d\n",v->len,v->data,l->sub_state,WTK_LEX_EXPR_WAIT_PROB);//wtk_lexc_get_expr_state(l->sub_state));
    //wtk_debug("[%.*s]=%d\n",v->len,v->data,l->sub_state);
    switch (l->sub_state) {
        case WTK_LEXC_EXPR_NAME_INIT:
            wtk_lexc_set_string_state(l, WTK_LEXC_EXPR_NAME,
                    WTK_LEXC_EXPR_NAME_END);
            return wtk_lexc_feed(l, v);
            break;
        case WTK_LEXC_EXPR_NAME_END:
            //wtk_debug("[%.*s]\n",l->tok->pos,l->tok->data);
            if (wtk_str_equal_s(l->tok->data, l->tok->pos, "def")) {
                wtk_lexc_set_state(l, WTK_LEXC_DEF);
                return wtk_lexc_feed(l, v);
            } else if (wtk_str_equal_s(l->tok->data, l->tok->pos, "export")) {
                //wtk_debug("[%.*s]\n",l->tok->pos,l->tok->data);
                wtk_lexc_set_string_state(l, WTK_LEXC_EXPR_NAME,
                        WTK_LEXC_EXPR_EXPORT_NAME_END);
                return wtk_lexc_feed(l, v);
            } else if (wtk_str_equal_s(l->tok->data, l->tok->pos, "if")) {
                wtk_lexc_set_state(l, WTK_LEXC_IF);
            } else if (wtk_str_equal_s(l->tok->data, l->tok->pos, "else")) {
                wtk_lexc_set_state(l, WTK_LEXC_ELSE);
            } else if (wtk_str_equal_s(l->tok->data, l->tok->pos, "end")) {
                wtk_lexc_set_state(l, WTK_LEXC_IF_END);
            } else if (wtk_str_equal_s(l->tok->data, l->tok->pos, "del")) {
                wtk_lexc_set_state(l, WTK_LEXC_CMD_DEL);
                return wtk_lexc_feed(l, v);
            } else if (wtk_str_equal_s(l->tok->data, l->tok->pos, "add")) {
                wtk_lexc_set_state(l, WTK_LEXC_CMD_ADD);
                return wtk_lexc_feed(l, v);
            } else if (wtk_str_equal_s(l->tok->data, l->tok->pos, "cpy")) {
                wtk_lexc_set_state(l, WTK_LEXC_CMD_CPY);
                return wtk_lexc_feed(l, v);
            } else if (wtk_str_equal_s(l->tok->data, l->tok->pos, "mv")) {
                wtk_lexc_set_state(l, WTK_LEXC_CMD_MV);
                return wtk_lexc_feed(l, v);
            } else if (wtk_str_equal_s(l->tok->data, l->tok->pos, "debug")) {
                wtk_lex_expr_t *expr;

                expr = wtk_lexc_add_new_cmd(l);
                expr->v.cmd->type = WTK_LEX_EXPR_CMD_DEBUG;
                wtk_lexc_set_state(l, WTK_LEXC_INIT);
                return wtk_lexc_feed(l, v);
            } else if (wtk_str_equal_s(l->tok->data, l->tok->pos, "return")) {
                wtk_lex_expr_t *expr;

                expr = wtk_lexc_add_new_cmd(l);
                expr->v.cmd->type = WTK_LEX_EXPR_CMD_RETURN;
                wtk_lexc_set_state(l, WTK_LEXC_INIT);
                return wtk_lexc_feed(l, v);
            } else {
                //wtk_debug("[%.*s]=%d\n",l->tok->pos,l->tok->data,l->scope_stk_q.length);
                wtk_lexc_add_new_expr(l, l->tok->data, l->tok->pos, 0);
                //wtk_debug("[%p]\n",l->cur_expr);
                wtk_queue_init(&(l->value_stk_q));
                stk = wtk_lexc_pop_env_stk(l);
                stk->value_q = &(l->cur_expr->v.expr->value_q);
                stk->or_q = &(l->cur_expr->v.expr->or_q);
                stk->v.expr = l->cur_expr;
                wtk_queue_push(&(l->value_stk_q), &(stk->q_n));
                l->sub_state = WTK_LEXC_EXPR_WAIT_EQ;
                //exit(0);
                ret = wtk_lexc_feed_expr_name(l, v);
            }
            break;
        case WTK_LEXC_EXPR_EXPORT_NAME_END:
            wtk_lexc_add_new_expr(l, l->tok->data, l->tok->pos, 1);
            //l->cur_expr=wtk_lex_script_new_expr(l->script,l->lib,l->tok->data,l->tok->pos,1);
            //wtk_debug("[%.*s]=%p\n",v->len,v->data,l->cur_expr->v.expr);
            //wtk_debug("[%.*s]=[%.*s]\n",l->tok->pos,l->tok->data,v->len,v->data);
            wtk_queue_init(&(l->value_stk_q));
            stk = wtk_lexc_pop_env_stk(l);
            stk->value_q = &(l->cur_expr->v.expr->value_q);
            stk->or_q = &(l->cur_expr->v.expr->or_q);
            stk->v.expr = l->cur_expr;
            wtk_queue_push(&(l->value_stk_q), &(stk->q_n));
            l->sub_state = WTK_LEXC_EXPR_WAIT_EQ;
            ret = wtk_lexc_feed_expr_name(l, v);
            break;
        case WTK_LEXC_EXPR_WAIT_EQ:
            if (v->len == 1 && v->data[0] == '=') {
                l->sub_state = WTK_LEXC_EXPR_WAIT_VALUE;
                ret = 0;
            } else {
                ret = -1;
            }
            break;
        case WTK_LEXC_EXPR_WAIT_VALUE:
            if (v->len == 1 && isspace(v->data[0])) {
                //continue;
                ret = 0;
            } else {
                l->sub_state = WTK_LEXC_EXPR_VALUE;
                ret = wtk_lexc_feed_expr_name(l, v);
            }
            break;
        case WTK_LEXC_EXPR_VALUE:
            // ()
            // []
            // /min,max,upper,lower,chn2num,skip,skipws/
            // *
            // +
            // ?
            // \d
            // ( ... | ... )
            //wtk_debug("[%.*s]=%p\n",v->len,v->data,l->cur_expr->v.expr);
            if (v->len == 1) {
                //wtk_debug("[%.*s]=%p\n",v->len,v->data,l->cur_expr->v.expr);
                c = v->data[0];
                switch (c) {
                    case '(':
                        //wtk_lex_expr_item_print(l->cur_expr->v.expr);
                        value = wtk_lex_script_new_parentheses(l->script);
                        stk = data_offset2(l->value_stk_q.push,
                                wtk_lexc_env_stk_t, q_n);
                        wtk_queue_push(stk->value_q, &(value->q_n));
                        stk = wtk_lexc_pop_env_stk(l);
                        stk->value_q = &(value->v.parentheses->value_q);
                        stk->or_q = &(value->v.parentheses->or_q);
                        stk->v.value = value;
                        wtk_queue_push(&(l->value_stk_q), &(stk->q_n));
                        break;
                    case ')':
                        qn = wtk_queue_pop_back(&(l->value_stk_q));
                        stk = data_offset2(qn, wtk_lexc_env_stk_t, q_n);
                        wtk_lex_script_close_parentheses(l->script,
                                stk->v.value->v.parentheses);
                        wtk_lexc_push_env_stk(l, stk);
                        //wtk_lex_expr_item_print(l->cur_expr->v.expr);
                        break;
                    case '[':
                        //bracket
                        l->sub_state = WTK_LEXC_EXPR_VALUE_BRACKET;
                        value = wtk_lex_script_new_bracket(l->script);
                        stk = data_offset2(l->value_stk_q.push,
                                wtk_lexc_env_stk_t, q_n);
                        wtk_queue_push(stk->value_q, &(value->q_n));
                        //wtk_lex_expr_item_print(l->cur_expr->v.expr);
                        break;
                    case '{':
                        //l->sub_state=WTK_LEXC_EXPR_VALUE_COUNT_WAIT_MIN;
                        wtk_lexc_set_string_state(l, WTK_LEXC_EXPR_NAME,
                                WTK_LEXC_EXPR_VALUE_COUNT_WAIT_MIN);
                        break;
                    case '*':
                        value = wtk_lexc_get_last_value(l);
                        repeat = &(value->attr.repeat);
                        repeat->min_count = 0;
                        repeat->max_count = -1;
                        //wtk_lex_expr_item_print(l->cur_expr->v.expr);
                        break;
                    case '+':
                        value = wtk_lexc_get_last_value(l);
                        repeat = &(value->attr.repeat);
                        repeat->min_count = 1;
                        repeat->max_count = -1;
                        break;
                    case '?':
                        value = wtk_lexc_get_last_value(l);
                        repeat = &(value->attr.repeat);
                        repeat->min_count = 0;
                        repeat->max_count = 1;
                        break;
                    case '\\':
                        l->sub_state = WTK_LEXC_EXPR_VALUE_HEX;
                        break;
                    case '|':
                        //wtk_lex_expr_item_print(l->cur_expr->v.expr);
                        branch = wtk_lex_expr_script_new_branch(l->script);
                        stk = data_offset2(l->value_stk_q.push,
                                wtk_lexc_env_stk_t, q_n);
                        branch->value_q = *stk->value_q;
                        wtk_queue_push(stk->or_q, &(branch->q_n));
                        wtk_queue_init(stk->value_q);
                        break;
                    case '/':
                        wtk_lexc_set_string_state(l, WTK_LEXC_EXPR_NAME,
                                WTK_LEXC_EXPR_VALUE_ATTR);
                        break;
                    case '^':
                        if (l->cur_expr->v.expr->value_q.length == 0
                                && l->cur_expr->v.expr->or_q.length == 0) {
                            l->cur_expr->v.expr->attr.match_start = 1;
                        } else {
                            value = wtk_lex_script_new_value_str(l->script,
                                    v->data, v->len);
                            stk = data_offset2(l->value_stk_q.push,
                                    wtk_lexc_env_stk_t, q_n);
                            wtk_queue_push(stk->value_q, &(value->q_n));
                        }
                        break;
                    case '$':
                        l->sub_state = WTK_LEXC_EXPR_VALUE_DOLLAR;
                        break;
                    case '=':
                        l->sub_state = WTK_LEXC_EXPR_VALUE_WIAT_OUTPUT;
                        ret = wtk_lexc_close_expr_value(l);
                        if (ret != 0) {
                            return ret;
                        }
                        break;
                    case '<':
                        ret = wtk_lexc_close_expr_value(l);
                        if (ret != 0) {
                            return ret;
                        }
                        wtk_lexc_set_string_state(l, WTK_LEXC_EXPR_NAME,
                                WTK_LEX_EXPR_WAIT_PROB);
                        break;
                    case ';':
                    case '\n':
                        wtk_lexc_finish_expr(l);
                        break;
                    case '.':
                        value = wtk_lex_script_new_value_dot(l->script);
                        stk = data_offset2(l->value_stk_q.push,
                                wtk_lexc_env_stk_t, q_n);
                        wtk_queue_push(stk->value_q, &(value->q_n));
                        break;
                    case '"':
                        wtk_strbuf_reset(tok);
                        l->sub_state = WTK_LEXC_EXPR_VALUE_QUOTE;
                        break;
                    default:
                        if (isspace(c)) {
                            if (!l->cfg->use_eng_word) {
                                l->sub_state = WTK_LEXC_EXPR_VALUE_WIAT_OUTPUT;
                                ret = wtk_lexc_close_expr_value(l);
                                if (ret != 0) {
                                    return ret;
                                }
                            }
                        } else {
                            if (l->cfg->use_eng_word) {
                                l->sub_state = WTK_LEXC_EXPR_VALUE_ENG;
                                wtk_strbuf_reset(tok);
                                wtk_strbuf_push(tok, v->data, v->len);
                            } else {
                                value = wtk_lex_script_new_value_str(l->script,
                                        v->data, v->len);
                                stk = data_offset2(l->value_stk_q.push,
                                        wtk_lexc_env_stk_t, q_n);
                                wtk_queue_push(stk->value_q, &(value->q_n));
                            }
                        }
                        break;
                }
                ret = 0;
            } else {
                value = wtk_lex_script_new_value_str(l->script, v->data,
                        v->len);
                stk = data_offset2(l->value_stk_q.push, wtk_lexc_env_stk_t,
                        q_n);
                wtk_queue_push(stk->value_q, &(value->q_n));
                ret = 0;
            }
            break;
        case WTK_LEXC_EXPR_VALUE_QUOTE:
            if (v->len == 1 && v->data[0] == '"') {
                value = wtk_lex_script_new_value_str(l->script, tok->data,
                        tok->pos);
                stk = data_offset2(l->value_stk_q.push, wtk_lexc_env_stk_t,
                        q_n);
                wtk_queue_push(stk->value_q, &(value->q_n));
                l->sub_state = WTK_LEXC_EXPR_VALUE;
                //wtk_debug("[%.*s]\n",tok->pos,tok->data);
                wtk_strbuf_reset(tok);
            } else {
                wtk_strbuf_push(tok, v->data, v->len);
            }
            break;
        case WTK_LEXC_EXPR_VALUE_ENG:
            //wtk_debug("[%.*s]\n",v->len,v->data);
            if (v->len == 1 && isalpha(v->data[0])) {
                wtk_strbuf_push(tok, v->data, v->len);
            } else {
                value = wtk_lex_script_new_value_str(l->script, tok->data,
                        tok->pos);
                stk = data_offset2(l->value_stk_q.push, wtk_lexc_env_stk_t,
                        q_n);
                wtk_queue_push(stk->value_q, &(value->q_n));
                l->sub_state = WTK_LEXC_EXPR_VALUE;
                wtk_strbuf_reset(tok);
                return wtk_lexc_feed_expr_name(l, v);
            }
            break;
        case WTK_LEXC_EXPR_VALUE_COUNT_WAIT_MIN:
            //wtk_debug("[%.*s]\n",l->tok->pos,l->tok->data);
            value = wtk_lexc_get_last_value(l);
            value->attr.repeat.min_count = wtk_str_atoi(l->tok->data,
                    l->tok->pos);
            if (value->attr.repeat.min_count > 0
                    && value->attr.repeat.max_count
                            < value->attr.repeat.min_count) {
                value->attr.repeat.max_count = value->attr.repeat.min_count;
            }
            //wtk_debug("[%d/%d]\n",value->attr.repeat.min_count,value->attr.repeat.max_count);
            l->sub_state = WTK_LEXC_EXPR_VALUE_COUNT_WAIT_COMMER;
            ret = wtk_lexc_feed_expr_name(l, v);
            break;
        case WTK_LEXC_EXPR_VALUE_COUNT_WAIT_COMMER:
            if (v->len > 1) {
                return -1;
            }
            if (isspace(v->data[0])) {
                ret = 0;
            } else if (v->data[0] == ',') {
                wtk_lexc_set_string_state(l, WTK_LEXC_EXPR_NAME,
                        WTK_LEXC_EXPR_VALUE_COUNT_WAIT_MAX);
                ret = 0;
            } else if (v->data[0] == '}') {
                l->sub_state = WTK_LEXC_EXPR_VALUE;
                ret = 0;
            } else {
                ret = -1;
            }
            break;
        case WTK_LEXC_EXPR_VALUE_COUNT_WAIT_MAX:
            value = wtk_lexc_get_last_value(l);
            value->attr.repeat.max_count = wtk_str_atoi(l->tok->data,
                    l->tok->pos);
            l->sub_state = WTK_LEXC_EXPR_VALUE_COUNT_WAIT_END;
            ret = wtk_lexc_feed_expr_name(l, v);
            break;
        case WTK_LEXC_EXPR_VALUE_COUNT_WAIT_END:
            if (v->len > 1) {
                return -1;
            }
            if (isspace(v->data[0])) {
                ret = 0;
            } else if (v->data[0] == '}') {
                l->sub_state = WTK_LEXC_EXPR_VALUE;
                ret = 0;
            } else {
                ret = -1;
            }
            break;
        case WTK_LEXC_EXPR_VALUE_BRACKET:
            if (v->len == 1 && v->data[0] == '^' && (value =
                    wtk_lexc_get_last_value(l))
                    && (value->v.bracket->or_q.length == 0)) {
                value->v.bracket->caret = 1;
            } else if (v->len == 1
                    && (v->data[0] == '-' || v->data[0] == ']'
                            || v->data[0] == '\\')) {
                if (v->data[0] == '\\') {
                    l->sub_state = WTK_LEXC_EXPR_VALUE_BRACKET_ESC;
                } else if (v->data[0] == '-') {
                    l->sub_state = WTK_LEXC_EXPR_VALUE_BRACKET_RANGE;
                } else {
                    l->sub_state = WTK_LEXC_EXPR_VALUE;
                }
            } else {
                value = wtk_lex_script_new_value_str(l->script, v->data,
                        v->len);
                qn = &(value->q_n);
                value = wtk_lexc_get_last_value(l);
                wtk_queue_push(&(value->v.bracket->or_q), qn);
            }
            ret = 0;
            break;
        case WTK_LEXC_EXPR_VALUE_BRACKET_ESC:
            if (v->len > 1) {
                return -1;
            }
            c = v->data[0];
            switch (c) {
                case 'd':
                    value = wtk_lex_script_new_esc(l->script, WTK_RE_ESC_d);
                    break;
                case 'D':
                    value = wtk_lex_script_new_esc(l->script, WTK_RE_ESC_D);
                    break;
                case 's':
                    value = wtk_lex_script_new_esc(l->script, WTK_RE_ESC_s);
                    break;
                case 'S':
                    value = wtk_lex_script_new_esc(l->script, WTK_RE_ESC_S);
                    break;
                case 'w':
                    value = wtk_lex_script_new_esc(l->script, WTK_RE_ESC_w);
                    break;
                case 'W':
                    value = wtk_lex_script_new_esc(l->script, WTK_RE_ESC_W);
                    break;
                case 'h':
                    value = wtk_lex_script_new_esc(l->script, WTK_RE_ESC_h);
                    break;
                case 'H':
                    value = wtk_lex_script_new_esc(l->script, WTK_RE_ESC_H);
                    break;
                case 'e':
                    value = wtk_lex_script_new_esc(l->script, WTK_RE_ESC_e);
                    break;
                case 'E':
                    value = wtk_lex_script_new_esc(l->script, WTK_RE_ESC_E);
                    break;
                default:
                    //return -1;
                    value = wtk_lex_script_new_value_str(l->script, v->data,
                            v->len);
                    break;
            }
            qn = &(value->q_n);
            value = wtk_lexc_get_last_value(l);
            //wtk_lex_expr_item_print(l->cur_expr->v.expr);
            wtk_queue_push(&(value->v.bracket->or_q), qn);
            l->sub_state = WTK_LEXC_EXPR_VALUE_BRACKET;
            ret = 0;
            break;
        case WTK_LEXC_EXPR_VALUE_BRACKET_RANGE:
            value = wtk_lexc_get_last_value(l);
            if (value->v.bracket->or_q.length <= 0) {
                return -1;
            }
            qn = value->v.bracket->or_q.push;
            value2 = data_offset2(qn, wtk_lex_expr_item_value_t, q_n);
            if (value2->type != WTK_LEX_VALUE_STRING) {
                return -1;
            }
            value2->type = WTK_LEX_VALUE_RANGE;
            value2->v.range->from = wtk_string_to_ord(value2->v.str);
            value2->v.range->to = wtk_string_to_ord(v);
            l->sub_state = WTK_LEXC_EXPR_VALUE_BRACKET;
            ret = 0;
            break;
        case WTK_LEXC_EXPR_VALUE_HEX:
            if (v->len == 1 && v->data[0] == 'x') {
                ret = 0;
                l->sub_state = WTK_LEXC_EXPR_VALUE_HEX1;
            } else {
                c = v->data[0];
                //wtk_debug("c=%d\n",c);
                switch (c) {
                    case 'd':
                        value = wtk_lex_script_new_esc(l->script, WTK_RE_ESC_d);
                        break;
                    case 'D':
                        value = wtk_lex_script_new_esc(l->script, WTK_RE_ESC_D);
                        break;
                    case 's':
                        value = wtk_lex_script_new_esc(l->script, WTK_RE_ESC_s);
                        break;
                    case 'S':
                        value = wtk_lex_script_new_esc(l->script, WTK_RE_ESC_S);
                        break;
                    case 'w':
                        value = wtk_lex_script_new_esc(l->script, WTK_RE_ESC_w);
                        break;
                    case 'W':
                        value = wtk_lex_script_new_esc(l->script, WTK_RE_ESC_W);
                        break;
                    case 'h':
                        value = wtk_lex_script_new_esc(l->script, WTK_RE_ESC_h);
                        break;
                    case 'H':
                        value = wtk_lex_script_new_esc(l->script, WTK_RE_ESC_H);
                        break;
                    case 'e':
                        value = wtk_lex_script_new_esc(l->script, WTK_RE_ESC_e);
                        break;
                    case 'E':
                        value = wtk_lex_script_new_esc(l->script, WTK_RE_ESC_E);
                        break;
                    default:
                        //return -1;
                        value = wtk_lex_script_new_value_str(l->script, v->data,
                                v->len);
                        break;
                }
                //value=wtk_lex_script_new_value_str(l->script,v->data,v->len);
                stk = data_offset2(l->value_stk_q.push, wtk_lexc_env_stk_t,
                        q_n);
                wtk_queue_push(stk->value_q, &(value->q_n));
                l->sub_state = WTK_LEXC_EXPR_VALUE;
                ret = 0;
            }
            break;
        case WTK_LEXC_EXPR_VALUE_HEX1:
            if (v->len == 1 && isxdigit(v->data[0])) {
                l->string_env.hex1 = wtk_char_to_hex(v->data[0]);
                l->sub_state = WTK_LEXC_EXPR_VALUE_HEX2;
                ret = 0;
            } else {
                ret = -1;
            }
            break;
        case WTK_LEXC_EXPR_VALUE_HEX2:
            if (v->len == 1 && isxdigit(v->data[0])) {
                ret = wtk_char_to_hex(v->data[0]);
                if (ret == -1) {
                    return -1;
                }
                c = (l->string_env.hex1 << 4) + ret;
                value = wtk_lex_script_new_value_str(l->script, &c, 1);
                stk = data_offset2(l->value_stk_q.push, wtk_lexc_env_stk_t,
                        q_n);
                wtk_queue_push(stk->value_q, &(value->q_n));
                l->sub_state = WTK_LEXC_EXPR_VALUE;
                ret = 0;
            } else {
                ret = -1;
            }
            break;
        case WTK_LEXC_EXPR_VALUE_ATTR:
            //wtk_lex_expr_item_print(l->cur_expr->v.expr);
            value = wtk_lexc_get_last_value(l);
            if (wtk_str_equal_s(l->tok->data, l->tok->pos, "lower")) {
                value->attr.txt = WTK_LEX_LOWER;
                l->sub_state = WTK_LEXC_EXPR_VALUE_ATTR_WAIT;
            } else if (wtk_str_equal_s(l->tok->data, l->tok->pos, "upper")) {
                value->attr.txt = WTK_LEX_UPPER;
                l->sub_state = WTK_LEXC_EXPR_VALUE_ATTR_WAIT;
            } else if (wtk_str_equal_s(l->tok->data, l->tok->pos, "chn2num")) {
                value->attr.chn2num = 1;
                l->sub_state = WTK_LEXC_EXPR_VALUE_ATTR_WAIT;
            } else if (wtk_str_equal_s(l->tok->data, l->tok->pos, "skipws")) {
                value->attr.skipws = 1;
                l->sub_state = WTK_LEXC_EXPR_VALUE_ATTR_WAIT;
            } else {
                l->str_key = wtk_heap_dup_string(l->heap, l->tok->data,
                        l->tok->pos);
                l->sub_state = WTK_LEXC_EXPR_VALUE_ATTR_WAIT_VALUE;
            }
            ret = wtk_lexc_feed_expr_name(l, v);
            break;
        case WTK_LEXC_EXPR_VALUE_ATTR_WAIT:
            if (v->len == 1) {
                if (v->data[0] == ',') {
                    wtk_lexc_set_string_state(l, WTK_LEXC_EXPR_NAME,
                            WTK_LEXC_EXPR_VALUE_ATTR);
                } else if (v->data[0] == '/') {
                    l->sub_state = WTK_LEXC_EXPR_VALUE;
                }
            }
            ret = 0;
            break;
        case WTK_LEXC_EXPR_VALUE_ATTR_WAIT_VALUE:
            if (v->len == 1 && v->data[0] == '=') {
                wtk_lexc_set_string_state(l, WTK_LEXC_EXPR_NAME,
                        WTK_LEXC_EXPR_VALUE_ATTR_VALUE);
            }
            ret = 0;
            break;
        case WTK_LEXC_EXPR_VALUE_ATTR_VALUE:
            //wtk_debug("[%.*s] [%.*s]=[%.*s]\n",l->str_key->len,l->str_key->data,l->tok->pos,l->tok->data,v->len,v->data);
            value = wtk_lexc_get_last_value(l);
            if (l->str_key->len <= 0) {
                wtk_debug("found bug\n");
                wtk_debug("[%.*s] [%.*s]=[%.*s]\n", l->str_key->len,
                        l->str_key->data, l->tok->pos, l->tok->data, v->len,
                        v->data);
                //exit(0);
                return -1;
            }
            if (wtk_str_equal_s(l->str_key->data, l->str_key->len,
                    "min_wrd_count")) {
                value->attr.min_wrd_count = wtk_str_atoi(l->tok->data,
                        l->tok->pos);
            } else if (wtk_str_equal_s(l->str_key->data, l->str_key->len,
                    "max_wrd_count")) {
                value->attr.max_wrd_count = wtk_str_atoi(l->tok->data,
                        l->tok->pos);
            } else if (wtk_str_equal_s(l->str_key->data, l->str_key->len,
                    "min")) {
                value->attr.repeat.min_count = wtk_str_atoi(l->tok->data,
                        l->tok->pos);
            } else if (wtk_str_equal_s(l->str_key->data, l->str_key->len,
                    "max")) {
                value->attr.repeat.max_count = wtk_str_atoi(l->tok->data,
                        l->tok->pos);
            } else if (wtk_str_equal_s(l->str_key->data, l->str_key->len,
                    "match_weight")) {
                value->attr.match_weight = wtk_str_atof(l->tok->data,
                        l->tok->pos);
            } else if (wtk_str_equal_s(l->str_key->data, l->str_key->len,
                    "skip")) {
                value->attr.skip = wtk_heap_dup_string(heap, l->tok->data,
                        l->tok->pos);
                if (l->cur_expr->type == WTK_LEX_EXPR_ITEM) {
                    l->cur_expr->v.expr->attr.check_skip = 1;
                }
            } else if (wtk_str_equal_s(l->str_key->data, l->str_key->len,
                    "pre")) {
                //wtk_debug("[%.*s] [%.*s]=[%.*s]\n",l->str_key->len,l->str_key->data,l->tok->pos,l->tok->data,v->len,v->data);
                value->attr.pre = wtk_lexc_get_attr_strs(l, l->tok->data,
                        l->tok->pos);
                if (value->type == WTK_LEX_VALUE_PARENTHESES) {
                    wtk_lex_script_expr_add_pre(l->script, value,
                            value->attr.pre);
                }
            } else if (wtk_str_equal_s(l->str_key->data, l->str_key->len,
                    "not_pre")) {
                //wtk_debug("[%.*s] [%.*s]=[%.*s]\n",l->str_key->len,l->str_key->data,l->tok->pos,l->tok->data,v->len,v->data);
                value->attr.not_pre = wtk_lexc_get_attr_strs(l, l->tok->data,
                        l->tok->pos);
                if (value->type == WTK_LEX_VALUE_PARENTHESES) {
                    wtk_lex_script_expr_add_not_pre(l->script, value,
                            value->attr.not_pre);
                }
            } else if (wtk_str_equal_s(l->str_key->data, l->str_key->len,
                    "suc")) {
                value->attr.suc = wtk_lexc_get_attr_strs(l, l->tok->data,
                        l->tok->pos);
                if (value->type == WTK_LEX_VALUE_PARENTHESES) {
                    wtk_lex_script_expr_add_suc(l->script, value,
                            value->attr.suc);
                }
            } else if (wtk_str_equal_s(l->str_key->data, l->str_key->len,
                    "not_suc")) {
                value->attr.not_suc = wtk_lexc_get_attr_strs(l, l->tok->data,
                        l->tok->pos);
                if (value->type == WTK_LEX_VALUE_PARENTHESES) {
                    wtk_lex_script_expr_add_not_suc(l->script, value,
                            value->attr.not_suc);
                }
            } else if (wtk_str_equal_s(l->str_key->data, l->str_key->len,
                    "o") || wtk_str_equal_s(l->str_key->data,l->str_key->len,"output")) {
                value->attr.output = wtk_heap_dup_string(heap, l->tok->data,
                        l->tok->pos);
            } else if (wtk_str_equal_s(l->str_key->data, l->str_key->len,
                    "like")) {
                //value->attr.like=wtk_heap_dup_string(heap,l->tok->data,l->tok->pos);
                wtk_lex_expr_item_value_attr_add_like(&(value->attr), heap,
                        l->tok->data, l->tok->pos);
            } else if (wtk_str_equal_s(l->str_key->data, l->str_key->len,
                    "like_thresh")) {
                //value->attr.like_thresh=wtk_str_atof(l->tok->data,l->tok->pos);
                wtk_lex_expr_item_value_attr_add_like_thresh(&(value->attr),
                        wtk_str_atof(l->tok->data, l->tok->pos));
            } else if (wtk_str_equal_s(l->str_key->data, l->str_key->len,
                    "pos")) {
                value->attr.pos = wtk_lexc_get_attr_strs(l, l->tok->data,
                        l->tok->pos);
            } else if (wtk_str_equal_s(l->str_key->data, l->str_key->len,
                    "not")) {
                value->attr.not = wtk_lexc_get_attr_strs(l, l->tok->data,
                        l->tok->pos);
            } else if (wtk_str_equal_s(l->str_key->data, l->str_key->len,
                    "ner")) {
                wtk_lexr_ner_item_t *ner;

                ner = wtk_str_hash_find(l->cfg->ner.hash, l->tok->data,
                        l->tok->pos);
                if (ner) {
                    wtk_lex_expr_item_value_attr_add_ner(&(value->attr), heap,
                            ner);
                } else {
                    return -1;
                }
            } else if (wtk_str_equal_s(l->str_key->data, l->str_key->len,
                    "ner_wrd_pen")) {
                wtk_lex_expr_item_value_attr_add_ner_wrd_pen(&(value->attr),
                        wtk_str_atof(l->tok->data, l->tok->pos));
            } else if (wtk_str_equal_s(l->str_key->data, l->str_key->len,
                    "ner_prune_thresh")) {
                wtk_lex_expr_item_value_attr_add_ner_prune_thresh(
                        &(value->attr),
                        wtk_str_atof(l->tok->data, l->tok->pos));
            } else if (wtk_str_equal_s(l->str_key->data, l->str_key->len,
                    "ner_use_search")) {
                wtk_lex_expr_item_value_attr_add_ner_use_search(&(value->attr),
                        wtk_str_atoi(l->tok->data, l->tok->pos));
            } else if (wtk_str_equal_s(l->str_key->data, l->str_key->len,
                    "ner_conf_thresh")) {
                wtk_lex_expr_item_value_attr_add_ner_conf_thresh(&(value->attr),
                        wtk_str_atof(l->tok->data, l->tok->pos));
            } else if (wtk_str_equal_s(l->str_key->data, l->str_key->len,
                    "use_py")) {
                //wtk_debug("update use py type=%d\n",value->type);
                value->attr.use_py = wtk_str_atoi(l->tok->data, l->tok->pos);
                wtk_lex_expr_item_value_update_py(value, value->attr.use_py);
            } else if (l->str_key->data[0] == '.') {
                if (!value->attr.replace) {
                    value->attr.replace = (wtk_queue3_t*) wtk_heap_malloc(heap,
                            sizeof(wtk_queue3_t));
                    wtk_queue3_init(value->attr.replace);
                }
                {
                    wtk_lex_replace_item_t *item;

//				wtk_debug("[%.*s]\n",l->str_key->len,l->str_key->data);
//				wtk_debug("[%.*s]\n",l->tok->pos,l->tok->data);
                    item = (wtk_lex_replace_item_t*) wtk_heap_malloc(heap,
                            sizeof(wtk_lex_replace_item_t));
                    item->f = wtk_heap_dup_string(heap, l->str_key->data + 1,
                            l->str_key->len - 1);
                    item->t = wtk_heap_dup_string(heap, l->tok->data,
                            l->tok->pos);
                    wtk_queue3_push(value->attr.replace, &(item->q_n));
                }
            } else {
                ///wtk_debug("[%.*s]\n",l->str_key->len,l->str_key->data);
                if (value->type == WTK_LEX_VALUE_PARENTHESES) {
                    if (wtk_str_equal_s(l->str_key->data, l->str_key->len,
                            "capture")) {
                        value->v.parentheses->capture = wtk_str_atoi(
                                l->tok->data, l->tok->pos);
                    }
                }
            }
            l->sub_state = WTK_LEXC_EXPR_VALUE_ATTR_WAIT;
            ret = wtk_lexc_feed_expr_name(l, v);
            break;
        case WTK_LEXC_EXPR_VALUE_DOLLAR:
            c = v->data[0];
            if (v->len == 1 && (c == '{' || isspace(c) || c == '=')) {
                if (c == '{') {
                    //wtk_lex_expr_item_print(l->cur_expr->v.expr);
                    wtk_lexc_set_string_state(l, WTK_LEXC_EXPR_NAME,
                            WTK_LEXC_EXPR_VALUE_DOLLAR_VALUE);
                } else {
                    l->cur_expr->v.expr->attr.match_end = 1;
                    l->sub_state = WTK_LEXC_EXPR_VALUE_WIAT_OUTPUT;
                    ret = wtk_lexc_close_expr_value(l);
                    if (ret != 0) {
                        return ret;
                    }
                }
            } else {
                value = wtk_lex_script_new_value_str(l->script, v->data,
                        v->len);
                stk = data_offset2(l->value_stk_q.push, wtk_lexc_env_stk_t,
                        q_n);
                wtk_queue_push(stk->value_q, &(value->q_n));
                l->sub_state = WTK_LEXC_EXPR_VALUE;
            }
            ret = 0;
            break;
        case WTK_LEXC_EXPR_VALUE_DOLLAR_VALUE:
            //wtk_debug("[%.*s]=[%.*s]\n",l->tok->pos,l->tok->data,v->len,v->data);
            value = wtk_lex_script_new_value_expr(l->script, l->lib,
                    l->tok->data, l->tok->pos);
            if (!value) {
                wtk_debug("var[%.*s] not exist.\n", l->tok->pos, l->tok->data);
                return -1;
            }
            stk = data_offset2(l->value_stk_q.push, wtk_lexc_env_stk_t, q_n);
            wtk_queue_push(stk->value_q, &(value->q_n));
            l->sub_state = WTK_LEXC_EXPR_VALUE_DOLLAR_VALUE_WAIT_END;
            ret = wtk_lexc_feed_expr_name(l, v);
            break;
        case WTK_LEXC_EXPR_VALUE_DOLLAR_VALUE_WAIT_END:
            if (v->len == 1 && v->data[0] == '}') {
                l->sub_state = WTK_LEXC_EXPR_VALUE;
            }
            ret = 0;
            break;
        case WTK_LEXC_EXPR_VALUE_WIAT_OUTPUT:
            if (v->len == 1) {
                c = v->data[0];
                if (c == '>') {
                    if (l->cfg->use_act) {
                        wtk_lexc_set_string_state(l, WTK_LEXC_EXPR_NAME,
                                WTK_LEXC_EXPR_VALUE_OUTPUT_NAME);
                    } else {
                        wtk_lexc_set_state(l, WTK_LEXC_OUTPUT_TRANS);
                    }
                } else if (c == ';' || c == '\n') {
                    wtk_lexc_finish_expr(l);
                }
            }
            ret = 0;
            break;
        case WTK_LEXC_EXPR_VALUE_OUTPUT_NAME:
            if (l->tok->pos > 0) {
                l->cur_expr->v.expr->output.name = wtk_heap_dup_string(
                        l->script->heap, l->tok->data, l->tok->pos);
            } else {
                l->cur_expr->v.expr->output.name = NULL;
            }
            l->sub_state = WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_PARAM;
            ret = wtk_lexc_feed_expr_name(l, v);
            break;
        case WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_PARAM:
            if (v->len == 1 && v->data[0] == '(') {
                wtk_lexc_set_string_state(l, WTK_LEXC_EXPR_NAME,
                        WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_KV);
            }
            ret = 0;
            break;
        case WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_KV:
            //wtk_debug("[%.*s]=[%.*s]\n",l->tok->pos,l->tok->data,v->len,v->data);
            if (l->tok->pos > 0 && l->tok->data[0] == '$') {
                l->cur_output_item = wtk_lex_script_new_output_item(l->script,
                        0, 0);
                wtk_queue_push(&(l->cur_expr->v.expr->output.item_q),
                        &(l->cur_output_item->q_n));

                l->cur_output_item->type = WTK_LEX_OUTPUT_ITEM_VAR;
                l->cur_output_item->v.var.cap_index = wtk_str_atoi(
                        l->tok->data + 1, l->tok->pos - 1);
                l->cur_output_item->v.var.def = NULL;
                l->sub_state = WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_VALUE_ATTR_INIT;
            } else {
                l->cur_output_item = wtk_lex_script_new_output_item(l->script,
                        l->tok->data, l->tok->pos);
                wtk_queue_push(&(l->cur_expr->v.expr->output.item_q),
                        &(l->cur_output_item->q_n));
                l->sub_state = WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_KV_EQ;
            }
            l->cur_output_item->miss_pen = l->cfg->var_miss_pen;
            ret = wtk_lexc_feed_expr_name(l, v);
            break;
        case WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_KV_EQ:
            if (v->len == 1) {
                c = v->data[0];
                switch (c) {
                    case ',':
                        wtk_lexc_set_string_state(l, WTK_LEXC_EXPR_NAME,
                                WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_KV);
                        break;
                    case '=':
                        wtk_lexc_set_string_state(l, WTK_LEXC_EXPR_NAME,
                                WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_VALUE);
                        break;
                    case ')':
                        l->sub_state = WTK_LEX_EXPR_WAIT_ATTR;
                        break;
                }
            }
            ret = 0;
            break;
        case WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_VALUE:
            //wtk_debug("[%.*s]=[%.*s]\n",l->tok->pos,l->tok->data,v->len,v->data);
            if (l->tok->data[0] == '$') {
                l->cur_output_item->type = WTK_LEX_OUTPUT_ITEM_VAR;
                l->cur_output_item->v.var.cap_index = wtk_str_atoi(
                        l->tok->data + 1, l->tok->pos - 1);
                l->cur_output_item->v.var.def = NULL;
            } else {
                wtk_lex_script_set_output_item_value(l->script,
                        l->cur_output_item, l->tok->data, l->tok->pos);
//			l->cur_output_item->type=WTK_LEX_OUTPUT_ITEM_STR;
//			l->cur_output_item->v.str=wtk_heap_dup_string(heap,l->tok->data,l->tok->pos);
            }
            l->cur_output_item->miss_pen = l->cfg->var_miss_pen;
            //wtk_debug("type=%p:%d\n",l->cur_output_item,l->cur_output_item->type);
            l->sub_state = WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_VALUE_ATTR_INIT;
            ret = wtk_lexc_feed_expr_name(l, v);
            break;
        case WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_VALUE_ATTR_INIT:
            if (v->len == 1) {
                c = v->data[0];
                switch (c) {
                    case '/':
                        wtk_lexc_set_string_state(l, WTK_LEXC_EXPR_NAME,
                                WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_VALUE_ATTR_KEY);
                        break;
                    case ',':
                        wtk_lexc_set_string_state(l, WTK_LEXC_EXPR_NAME,
                                WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_KV);
                        break;
                    case ')':
                        l->sub_state = WTK_LEX_EXPR_WAIT_ATTR;
                        break;
                }
            }
            ret = 0;
            break;
        case WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_VALUE_ATTR:
            if (v->len == 1) {
                c = v->data[0];
                switch (c) {
                    case ',':
                        wtk_lexc_set_string_state(l, WTK_LEXC_EXPR_NAME,
                                WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_VALUE_ATTR_KEY);
                        break;
                    case '/':
                        l->sub_state = WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_KV_EQ;
                        break;
                }

            }
            ret = 0;
            break;
        case WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_VALUE_ATTR_KEY:
            //wtk_debug("[%.*s]=[%.*s]\n",l->tok->pos,l->tok->data,v->len,v->data);
//		if(wtk_str_equal_s(l->tok->data,l->tok->pos,"pen"))
//		{
//			l->str_key=wtk_heap_dup_string(l->heap,l->tok->data,l->tok->pos);
//		}else if(wtk_str_equal_s(l->tok->data,l->tok->pos,"def"))
//		{
//			l->str_key=wtk_heap_dup_string(heap,l->tok->data,l->tok->pos);
//		}else if(wtk_str_equal_s(l->tok->data,l->tok->pos,"hook"))
//		{
//			l->str_key=wtk_heap_dup_string(heap,l->tok->data,l->tok->pos);
//		}else
//		{
//
//		}
            l->str_key = wtk_heap_dup_string(heap, l->tok->data, l->tok->pos);
            l->sub_state = WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_VALUE_ATTR_EQ;
            ret = wtk_lexc_feed_expr_name(l, v);
            break;
        case WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_VALUE_ATTR_EQ:
            if (v->len == 1 && v->data[0] == '=') {
                wtk_lexc_set_string_state(l, WTK_LEXC_EXPR_NAME,
                        WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_VALUE_ATTR_V);
            }
            ret = 0;
            break;
        case WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_VALUE_ATTR_V:
            //wtk_debug("[%.*s]=[%.*s]\n",l->tok->pos,l->tok->data,v->len,v->data);
//		wtk_debug("type=%p:%d\n",l->cur_output_item,l->cur_output_item->type);
            if (wtk_str_equal_s(l->str_key->data, l->str_key->len, "pen")) {
                l->cur_output_item->miss_pen = wtk_str_atof(l->tok->data,
                        l->tok->pos);
            } else if (wtk_str_equal_s(l->str_key->data, l->str_key->len,
                    "def")) {
                l->cur_output_item->v.var.def = wtk_heap_dup_string(heap,
                        l->tok->data, l->tok->pos);
            } else if (wtk_str_equal_s(l->str_key->data, l->str_key->len,
                    "hook")) {
                l->cur_output_item->hook = wtk_heap_dup_string(heap,
                        l->tok->data, l->tok->pos);
            } else if (wtk_str_equal_s(l->str_key->data, l->str_key->len,
                    "post")) {
                l->cur_output_item->post = wtk_heap_dup_string(heap,
                        l->tok->data, l->tok->pos);
            } else {
                wtk_debug("unk [%.*s]=[%.*s]\n", l->str_key->len,
                        l->str_key->data, l->tok->pos, l->tok->data);
                return -1;
            }
            //wtk_lex_expr_item_print(l->cur_expr->v.expr);
            l->sub_state = WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_VALUE_ATTR;
            //exit(0);
            ret = wtk_lexc_feed_expr_name(l, v);
            break;
        case WTK_LEX_EXPR_WAIT_ATTR:
            if (v->len != 1) {
                return -1;
            }
            c = v->data[0];
            if (c == ';') {
                wtk_lexc_finish_expr(l);
            } else if (c == '/') {
                wtk_lexc_set_string_state(l, WTK_LEXC_EXPR_NAME,
                        WTK_LEX_EXPR_ATTR_KEY);
            } else if (c == '<') {
                wtk_lexc_set_string_state(l, WTK_LEXC_EXPR_NAME,
                        WTK_LEX_EXPR_WAIT_PROB);
            }
            ret = 0;
            break;
        case WTK_LEX_EXPR_ATTR_WAIT_KEY:
            if (v->len == 1) {
                c = v->data[0];
                switch (c) {
                    case ',':
                        wtk_lexc_set_string_state(l, WTK_LEXC_EXPR_NAME,
                                WTK_LEX_EXPR_ATTR_KEY);
                        break;
                    case '/':
                        l->sub_state = WTK_LEX_EXPR_WAIT_ATTR;
                        break;
                }
            }
            ret = 0;
            break;
        case WTK_LEX_EXPR_ATTR_KEY:
            //wtk_debug("[%.*s]=[%.*s]\n",l->tok->pos,l->tok->data,v->len,v->data);
            if (wtk_str_equal_s(l->tok->data, l->tok->pos, "lower")) {
                l->cur_expr->v.expr->attr.txt = WTK_LEX_LOWER;
                l->sub_state = WTK_LEX_EXPR_ATTR_WAIT_KEY;
            } else if (wtk_str_equal_s(l->tok->data, l->tok->pos, "upper")) {
                l->cur_expr->v.expr->attr.txt = WTK_LEX_UPPER;
                l->sub_state = WTK_LEX_EXPR_ATTR_WAIT_KEY;
            } else if (wtk_str_equal_s(l->tok->data, l->tok->pos, "skipws")) {
                l->cur_expr->v.expr->attr.skipws = 1;
                l->sub_state = WTK_LEX_EXPR_ATTR_WAIT_KEY;
                //wtk_lex_expr_item_print(l->cur_expr->v.expr);
                //exit(0);
            } else {
                l->str_key = wtk_heap_dup_string(l->heap, l->tok->data,
                        l->tok->pos);
                l->sub_state = WTK_LEX_EXPR_ATTR_WAIT_VALUE;
            }
            ret = wtk_lexc_feed_expr_name(l, v);
            break;
        case WTK_LEX_EXPR_ATTR_WAIT_VALUE:
            if (v->len == 1 && v->data[0] == '=') {
                wtk_lexc_set_string_state(l, WTK_LEXC_EXPR_NAME,
                        WTK_LEX_EXPR_ATTR_VALUE);
            }
            ret = 0;
            break;
        case WTK_LEX_EXPR_ATTR_VALUE:
            //wtk_debug("[%.*s]=[%.*s]\n",l->tok->pos,l->tok->data,v->len,v->data);
            //exit(0);
            if (wtk_str_equal_s(l->str_key->data, l->str_key->len,
                    "min_wrd_count")) {
                l->cur_expr->v.expr->attr.min_wrd_count = wtk_str_atoi(
                        l->tok->data, l->tok->pos);
            } else if (wtk_str_equal_s(l->str_key->data, l->str_key->len,
                    "max_wrd_count")) {
                l->cur_expr->v.expr->attr.max_wrd_count = wtk_str_atoi(
                        l->tok->data, l->tok->pos);
            } else if (wtk_str_equal_s(l->str_key->data, l->str_key->len,
                    "match_more_wrd")) {
                //wtk_debug("set %d....\n",l->cur_expr->v.expr->attr.nbest_short_expr_item);
                l->cur_expr->v.expr->attr.match_more_wrd = wtk_str_atoi(
                        l->tok->data, l->tok->pos);
                //wtk_debug("set %d....\n",l->cur_expr->v.expr->attr.nbest_short_expr_item);
            } else if (wtk_str_equal_s(l->str_key->data, l->str_key->len,
                    "match_more_var")) {
                //wtk_debug("set %d....\n",l->cur_expr->v.expr->attr.nbest_short_expr_item);
                l->cur_expr->v.expr->attr.match_more_var = wtk_str_atoi(
                        l->tok->data, l->tok->pos);
                //wtk_debug("set %d....\n",l->cur_expr->v.expr->attr.nbest_short_expr_item);
            } else if (wtk_str_equal_s(l->str_key->data, l->str_key->len,
                    "step_search")) {
                //wtk_debug("set %d....\n",l->cur_expr->v.expr->attr.nbest_short_expr_item);
                l->cur_expr->v.expr->attr.step_search = wtk_str_atoi(
                        l->tok->data, l->tok->pos);
                //wtk_debug("set %d....\n",l->cur_expr->v.expr->attr.nbest_short_expr_item);
            } else if (wtk_str_equal_s(l->str_key->data, l->str_key->len,
                    "o") || wtk_str_equal_s(l->str_key->data,l->str_key->len,"output")) {
                l->cur_expr->v.expr->attr.output = wtk_heap_dup_string(
                        l->script->heap, l->tok->data, l->tok->pos);
            } else if (wtk_str_equal_s(l->str_key->data, l->str_key->len,
                    "use_seg")) {
                l->cur_expr->v.expr->attr.use_seg = wtk_str_atoi(l->tok->data,
                        l->tok->pos);
            }
            l->sub_state = WTK_LEX_EXPR_ATTR_WAIT_KEY;
            ret = wtk_lexc_feed_expr_name(l, v);
            break;
        case WTK_LEX_EXPR_WAIT_PROB:
            l->cur_expr->v.expr->prob = wtk_str_atof(l->tok->data, l->tok->pos);
            l->sub_state = WTK_LEX_EXPR_WAIT_PROB_END;
            ret = wtk_lexc_feed_expr_name(l, v);
            break;
        case WTK_LEX_EXPR_WAIT_PROB_END:
            if (v->len == 1 && v->data[0] == '>') {
                l->sub_state = WTK_LEX_EXPR_WAIT_ATTR;
            }
            break;
    }
    //wtk_lex_expr_item_print(l->cur_expr->v.expr);
    //wtk_debug("state=%d/%d\n",WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_VALUE,WTK_LEXC_EXPR_VALUE_OUTPUT_WAIT_KV);
    return ret;
}

int wtk_lexc_feed_def(wtk_lexc_t *l, wtk_string_t *v)
{
    enum {
        WTK_LEXC_DEF_INIT = 0,
        WTK_LEX_DEF_NAME,
        WTK_LEX_DEF_WAIT_EQ,
        WTK_LEX_DEF_WAIT_VALUE,
        WTK_LEX_DEF_WAIT_END,
    };
    wtk_string_t *v1;
    int ret;

    switch (l->sub_state) {
        case WTK_LEXC_DEF_INIT:
            wtk_lexc_set_string_state(l, WTK_LEXC_DEF, WTK_LEX_DEF_NAME);
            ret = wtk_lexc_feed(l, v);
            break;
        case WTK_LEX_DEF_NAME:
            //wtk_debug("[%.*s]\n",l->tok->pos,l->tok->data);
            l->str_key = wtk_heap_dup_string(l->heap, l->tok->data,
                    l->tok->pos);
            l->sub_state = WTK_LEX_DEF_WAIT_EQ;
            ret = wtk_lexc_feed(l, v);
            break;
        case WTK_LEX_DEF_WAIT_EQ:
            if (v->len == 1) {
                if (isspace(v->data[0])) {
                    ret = 0;
                } else if (v->data[0] == '=') {
                    wtk_lexc_set_string_state(l, WTK_LEXC_DEF,
                            WTK_LEX_DEF_WAIT_VALUE);
                    //l->sub_state=WTK_LEX_DEF_WAIT_VALUE;
                    ret = 0;
                } else {
                    ret = -1;
                }
            } else {
                ret = -1;
            }
            break;
        case WTK_LEX_DEF_WAIT_VALUE:
            //wtk_debug("[%.*s] [%.*s]\n",l->tok->pos,l->tok->data,v->len,v->data);
            v1 = wtk_heap_dup_string(l->heap, l->tok->data, l->tok->pos);
            wtk_str_hash_add(l->str_var_hash, l->str_key->data, l->str_key->len,
                    v1);
            if (v->len == 1 && (v->data[0] == ';' || isspace(v->data[0]))) {
                if (v->data[0] == ';') {
                    wtk_lexc_set_state(l, WTK_LEXC_INIT);
                } else {
                    l->sub_state = WTK_LEX_DEF_WAIT_END;
                }
                ret = 0;
            } else {
                wtk_debug("must end with ;\n");
                ret = -1;
            }
            break;
        case WTK_LEX_DEF_WAIT_END:
            if (v->len == 1 && (v->data[0] == ';' || isspace(v->data[0]))) {
                if (v->data[0] == ';') {
                    wtk_lexc_set_state(l, WTK_LEXC_INIT);
                }
                ret = 0;
            } else {
                wtk_debug("must end with ;\n");
                ret = -1;
            }
            break;
        default:
            ret = -1;
            break;
    }
    return ret;
}

wtk_lex_expr_t* wtk_lexc_add_new_scope(wtk_lexc_t *l)
{
    wtk_lexc_scope_stk_t *stk;
    wtk_lex_expr_t *expr;

    //wtk_debug("[%.*s]=%d\n",l->tok->pos,l->tok->data,l->scope_stk_q.length);
    if (l->scope_stk_q.length > 0) {
        stk = data_offset2(l->scope_stk_q.push, wtk_lexc_scope_stk_t, q_n);
        expr = wtk_lex_script_new_scope3(l->script);
        if (stk->yes) {
            wtk_queue_push(&(stk->scope->yes), &(expr->q_n));
        } else {
            wtk_queue_push(&(stk->scope->no), &(expr->q_n));
        }
    } else {
        expr = wtk_lex_script_new_scope(l->script, l->lib);
        //l->cur_expr=wtk_lex_script_new_expr(l->script,l->lib,name,bytes,expt);
    }
    return expr;
}

int wtk_lexc_feed_if(wtk_lexc_t *l, wtk_string_t *v)
{
    enum {
        WTK_LEXC_IF_INIT = 0,
        WTK_LEXC_IF_COND,
        WTK_LEXC_IF_COND_VALUE,
        WTK_LEXC_IF_COND_NOT_ATTR,
    };
    wtk_lex_expr_t *expr;
    wtk_lex_expr_if_t *i;
    wtk_lexc_scope_stk_t *stk;

    switch (l->sub_state) {
        case WTK_LEXC_IF_INIT:
            expr = wtk_lexc_add_new_scope(l);
            //expr=wtk_lex_script_new_scope(l->script,l->lib);

            stk = wtk_lexc_pop_scope_stk(l);
            stk->scope = expr->v.scope;
            stk->yes = 1;
            wtk_queue_push(&(l->scope_stk_q), &(stk->q_n));
            wtk_lexc_set_string_state(l, WTK_LEXC_IF, WTK_LEXC_IF_COND);
            return wtk_lexc_feed(l, v);
            break;
        case WTK_LEXC_IF_COND:
            if (wtk_str_equal_s(l->tok->data, l->tok->pos, "not")) {
                wtk_lexc_set_string_state(l, WTK_LEXC_IF,
                        WTK_LEXC_IF_COND_NOT_ATTR);
            } else if (wtk_str_equal_s(l->tok->data, l->tok->pos, "and")) {
                stk = data_offset2(l->scope_stk_q.push, wtk_lexc_scope_stk_t,
                        q_n);
                stk->scope->and = 1;
                wtk_lexc_set_string_state(l, WTK_LEXC_IF, WTK_LEXC_IF_COND);
            } else if (wtk_str_equal_s(l->tok->data, l->tok->pos, "or")) {
                stk = data_offset2(l->scope_stk_q.push, wtk_lexc_scope_stk_t,
                        q_n);
                stk->scope->and = 0;
                wtk_lexc_set_string_state(l, WTK_LEXC_IF, WTK_LEXC_IF_COND);
            } else if (wtk_str_equal_s(l->tok->data, l->tok->pos, "then")) {
                //wtk_debug("[%.*s]=[%.*s]\n",l->tok->pos,l->tok->data,v->len,v->data);
                //wtk_lex_script_print(l->script);
                wtk_lexc_set_state(l, WTK_LEXC_INIT);
            } else if (wtk_str_equal_s(l->tok->data, l->tok->pos, "then")) {
                wtk_lexc_set_string_state(l, WTK_LEXC_IF,
                        WTK_LEXC_IF_COND_VALUE);
                return 0;
            } else {
                //wtk_debug("[%.*s]=[%.*s]\n",l->tok->pos,l->tok->data,v->len,v->data);
                i = wtk_lex_script_new_if(l->script, l->tok->data, l->tok->pos,
                        0);
                stk = data_offset2(l->scope_stk_q.push, wtk_lexc_scope_stk_t,
                        q_n);
                wtk_queue_push(&(stk->scope->condition), &(i->q_n));
                //exit(0);
                if (wtk_string_cmp_s(v,"=") == 0) {
                    wtk_lexc_set_string_state(l, WTK_LEXC_IF,
                            WTK_LEXC_IF_COND_VALUE);
                    return 0;
                } else {
                    wtk_lexc_set_string_state(l, WTK_LEXC_IF, WTK_LEXC_IF_COND);
                }
            }
            return wtk_lexc_feed(l, v);
            break;
        case WTK_LEXC_IF_COND_VALUE:
            //wtk_debug("[%.*s]=[%.*s]\n",l->tok->pos,l->tok->data,v->len,v->data);
            stk = data_offset2(l->scope_stk_q.push, wtk_lexc_scope_stk_t, q_n);
            i = data_offset2(stk->scope->condition.push, wtk_lex_expr_if_t,
                    q_n);
            i->value = wtk_heap_dup_string(l->script->heap, l->tok->data,
                    l->tok->pos);
            //wtk_debug("i=%p\n",i);
            //exit(0);
            wtk_lexc_set_string_state(l, WTK_LEXC_IF, WTK_LEXC_IF_COND);
            return wtk_lexc_feed(l, v);
            break;
        case WTK_LEXC_IF_COND_NOT_ATTR:
            //wtk_debug("[%.*s]=[%.*s]\n",l->tok->pos,l->tok->data,v->len,v->data);
            i = wtk_lex_script_new_if(l->script, l->tok->data, l->tok->pos, 1);
            stk = data_offset2(l->scope_stk_q.push, wtk_lexc_scope_stk_t, q_n);
            wtk_queue_push(&(stk->scope->condition), &(i->q_n));
            wtk_lexc_set_string_state(l, WTK_LEXC_IF, WTK_LEXC_IF_COND);
            return wtk_lexc_feed(l, v);
            break;
    }
    return 0;
}

int wtk_lexc_feed_else(wtk_lexc_t *l, wtk_string_t *v)
{
    wtk_lexc_scope_stk_t *stk;

    stk = data_offset2(l->scope_stk_q.push, wtk_lexc_scope_stk_t, q_n);
    stk->yes = 0;
    wtk_lexc_set_state(l, WTK_LEXC_INIT);
    return wtk_lexc_feed(l, v);
}

int wtk_lexc_feed_if_end(wtk_lexc_t *l, wtk_string_t *v)
{
    wtk_queue_node_t *qn;
    wtk_lexc_scope_stk_t *stk;

    //wtk_debug("pop back\n");
    qn = wtk_queue_pop_back(&(l->scope_stk_q));
    stk = data_offset2(qn, wtk_lexc_scope_stk_t, q_n);
    wtk_lexc_push_scope_stk(l, stk);
    wtk_lexc_set_state(l, WTK_LEXC_INIT);
    return wtk_lexc_feed(l, v);
}

char* wtk_lexc_get_state(wtk_lexc_t *l)
{
    static char* ns[] = { "WTK_LEXC_INIT",
            "WTK_LEXC_COMMENT",	//"#"
            "WTK_LEXC_COMMENT2",	// "/"
            "WTK_LEXC_INCLUDE", "WTK_LEXC_IMPORT", "WTK_LEXC_EXPR_NAME",
            "WTK_LEXC_OUTPUT_TRANS", "WTK_LEXC_STRING", "WTK_LEXC_DEF",
            "WTK_LEXC_IF", "WTK_LEXC_ELSE", "WTK_LEXC_IF_END",
            "WTK_LEXC_CMD_END", "WTK_LEXC_CMD_DEL", "WTK_LEXC_CMD_ADD",
            "WTK_LEXC_CMD_CPY", "WTK_LEXC_CMD_MV", };

    return ns[l->state];
}

wtk_lex_expr_t* wtk_lexc_add_new_cmd(wtk_lexc_t *l)
{
    wtk_lexc_scope_stk_t *stk;
    wtk_lex_expr_t *expr;

    //wtk_debug("[%.*s]=%d\n",l->tok->pos,l->tok->data,l->scope_stk_q.length);
    if (l->scope_stk_q.length > 0) {
        stk = data_offset2(l->scope_stk_q.push, wtk_lexc_scope_stk_t, q_n);
        expr = wtk_lex_script_new_cmd2(l->script);
        if (stk->yes) {
            wtk_queue_push(&(stk->scope->yes), &(expr->q_n));
        } else {
            wtk_queue_push(&(stk->scope->no), &(expr->q_n));
        }
    } else {
        expr = wtk_lex_script_new_cmd(l->script, l->lib);
        //l->cur_expr=wtk_lex_script_new_expr(l->script,l->lib,name,bytes,expt);
    }
    return expr;
}

int wtk_lexc_feed_cmd_del(wtk_lexc_t *l, wtk_string_t *v)
{
    enum {
        WTK_LEX_CMD_DEL_INIT = 0, WTK_LEX_CMD_DEL_NAME,
    };

    switch (l->sub_state) {
        case WTK_LEX_CMD_DEL_INIT:
            wtk_lexc_set_string_state(l, WTK_LEXC_CMD_DEL,
                    WTK_LEX_CMD_DEL_NAME);
            return wtk_lexc_feed(l, v);
            break;
        case WTK_LEX_CMD_DEL_NAME:
            //wtk_debug("[%.*s]=[%.*s]\n",l->tok->pos,l->tok->data,v->len,v->data);
            l->cur_expr = wtk_lexc_add_new_cmd(l);
            wtk_lex_script_cmd_set_del(l->script, l->cur_expr->v.cmd,
                    l->tok->data, l->tok->pos);
            wtk_lexc_set_state(l, WTK_LEXC_INIT);
            break;
    }
    return 0;
}

int wtk_lexc_feed_cmd_add(wtk_lexc_t *l, wtk_string_t *v)
{
    enum {
        WTK_LEX_CMD_ADD_INIT = 0, WTK_LEX_CMD_ADD_NAME, WTK_LEX_CMD_ADD_VALUE,
    };

    switch (l->sub_state) {
        case WTK_LEX_CMD_ADD_INIT:
            wtk_lexc_set_string_state(l, WTK_LEXC_CMD_ADD,
                    WTK_LEX_CMD_ADD_NAME);
            return wtk_lexc_feed(l, v);
            break;
        case WTK_LEX_CMD_ADD_NAME:
            //wtk_debug("[%.*s]=[%.*s]\n",l->tok->pos,l->tok->data,v->len,v->data);
            l->cur_expr = wtk_lexc_add_new_cmd(l);
            wtk_lex_script_cmd_set_add(l->script, l->cur_expr->v.cmd,
                    l->tok->data, l->tok->pos);
            wtk_lexc_set_string_state(l, WTK_LEXC_CMD_ADD,
                    WTK_LEX_CMD_ADD_VALUE);
            break;
        case WTK_LEX_CMD_ADD_VALUE:
            wtk_lex_script_cmd_set_value(l->script, l->cur_expr->v.cmd,
                    l->tok->data, l->tok->pos);
            wtk_lexc_set_state(l, WTK_LEXC_INIT);
            break;
    }
    return 0;
}

int wtk_lexc_feed_cmd_cpy(wtk_lexc_t *l, wtk_string_t *v)
{
    enum {
        WTK_LEX_CMD_CPY_INIT = 0, WTK_LEX_CMD_CPY_NAME, WTK_LEX_CMD_CPY_VALUE,
    };

    switch (l->sub_state) {
        case WTK_LEX_CMD_CPY_INIT:
            wtk_lexc_set_string_state(l, WTK_LEXC_CMD_CPY,
                    WTK_LEX_CMD_CPY_NAME);
            return wtk_lexc_feed(l, v);
            break;
        case WTK_LEX_CMD_CPY_NAME:
            //wtk_debug("[%.*s]=[%.*s]\n",l->tok->pos,l->tok->data,v->len,v->data);
            l->cur_expr = wtk_lexc_add_new_cmd(l);
            wtk_lex_script_cmd_set_cpy(l->script, l->cur_expr->v.cmd,
                    l->tok->data, l->tok->pos);
            wtk_lexc_set_string_state(l, WTK_LEXC_CMD_CPY,
                    WTK_LEX_CMD_CPY_VALUE);
            break;
        case WTK_LEX_CMD_CPY_VALUE:
            wtk_lex_script_cmd_set_cpy_value(l->script, l->cur_expr->v.cmd,
                    l->tok->data, l->tok->pos);
            wtk_lexc_set_state(l, WTK_LEXC_INIT);
            break;
    }
    return 0;
}

int wtk_lexc_feed_cmd_mv(wtk_lexc_t *l, wtk_string_t *v)
{
    enum {
        WTK_LEX_CMD_MV_INIT = 0, WTK_LEX_CMD_MV_NAME, WTK_LEX_CMD_MV_VALUE,
    };

    switch (l->sub_state) {
        case WTK_LEX_CMD_MV_INIT:
            wtk_lexc_set_string_state(l, WTK_LEXC_CMD_MV, WTK_LEX_CMD_MV_NAME);
            return wtk_lexc_feed(l, v);
            break;
        case WTK_LEX_CMD_MV_NAME:
            //wtk_debug("[%.*s]=[%.*s]\n",l->tok->pos,l->tok->data,v->len,v->data);
            l->cur_expr = wtk_lexc_add_new_cmd(l);
            wtk_lex_script_cmd_set_mv(l->script, l->cur_expr->v.cmd,
                    l->tok->data, l->tok->pos);
            wtk_lexc_set_string_state(l, WTK_LEXC_CMD_MV, WTK_LEX_CMD_MV_VALUE);
            break;
        case WTK_LEX_CMD_MV_VALUE:
            wtk_lex_script_cmd_set_mv_value(l->script, l->cur_expr->v.cmd,
                    l->tok->data, l->tok->pos);
            wtk_lexc_set_state(l, WTK_LEXC_INIT);
            break;
    }
    return 0;
}

int wtk_lexc_feed(wtk_lexc_t *l, wtk_string_t *v)
{
    int ret;

    //wtk_debug("[%.*s] state=%d/%d\n",v->len,v->data,l->state,l->sub_state);
    switch (l->state) {
        case WTK_LEXC_INIT:
            ret = wtk_lexc_feed_init(l, v);
            break;
        case WTK_LEXC_COMMENT://# 单行注释
            ret = wtk_lexc_feed_comment(l, v);
            break;
        case WTK_LEXC_COMMENT2:// / 注释
            ret = wtk_lexc_feed_comment2(l, v);
            break;
        case WTK_LEXC_INCLUDE://#inclide
            ret = wtk_lexc_feed_include(l, v);
            break;
        case WTK_LEXC_IMPORT:
            ret = wtk_lexc_feed_import(l, v);
            break;
        case WTK_LEXC_STRING:
            ret = wtk_lexc_feed_string(l, v);
            break;
        case WTK_LEXC_EXPR_NAME:
            ret = wtk_lexc_feed_expr_name(l, v);
            break;
        case WTK_LEXC_OUTPUT_TRANS:
            ret = wtk_lexc_feed_expr_output_trans(l, v);
            break;
        case WTK_LEXC_DEF:
            ret = wtk_lexc_feed_def(l, v);
            break;
        case WTK_LEXC_IF:
            ret = wtk_lexc_feed_if(l, v);
            break;
        case WTK_LEXC_ELSE:
            ret = wtk_lexc_feed_else(l, v);
            break;
        case WTK_LEXC_IF_END:
            ret = wtk_lexc_feed_if_end(l, v);
            break;
        case WTK_LEXC_CMD_END:
            ret = 0;
            break;
        case WTK_LEXC_CMD_DEL:
            ret = wtk_lexc_feed_cmd_del(l, v);
            break;
        case WTK_LEXC_CMD_ADD:
            ret = wtk_lexc_feed_cmd_add(l, v);
            break;
        case WTK_LEXC_CMD_CPY:
            ret = wtk_lexc_feed_cmd_cpy(l, v);
            break;
        case WTK_LEXC_CMD_MV:
            ret = wtk_lexc_feed_cmd_mv(l, v);
            break;
        default:
            wtk_debug("[%.*s]\n", v->len, v->data);
            wtk_debug("exit 0\n");
            exit(0);
            break;
    }
    return ret;
}

void wtk_lexc_set_get_expr(wtk_lexc_t *l, void *ths,
        wtk_lexc_get_expr_f get_expr)
{
    l->get_expr = get_expr;
    l->get_expr_ths = ths;
}

wtk_lex_script_t* wtk_lexc_compile_source(wtk_lexc_t *l, wtk_source_t *src)
{
    enum wtk_lex_str_state_t {
        WTK_LEX_STR_INIT, WTK_LEX_STR_ENG,
    };
    wtk_string_t v;
    int ret;
    int b = 0;
    wtk_strbuf_t *buf;

    buf = wtk_strbuf_new(256, 1);
    wtk_lexc_set_state(l, WTK_LEXC_INIT);
    if (!l->script) {
        b = 1;
        l->script = wtk_lex_script_new();
        l->script->get_expr_ths = l->get_expr_ths;
        l->script->get_expr = l->get_expr;
        l->script->use_eng_word = l->cfg->use_eng_word;
        l->script->use_act = l->cfg->use_act;
        if (l->script->use_eng_word) {
            l->script->sort_by_prob = 1;
        }
    }
    while (1) {
        ret = wtk_source_read_utf8_char(src, buf);
        if (ret != 0) {
            ret = 0;
            break;
        }
        wtk_string_set(&(v), buf->data, buf->pos);
        ret = wtk_lexc_feed(l, &v);
        if (ret != 0) {
            wtk_debug("%s:%d\n", wtk_lexc_get_state(l), l->sub_state);
            //wtk_debug("error: [%.*s]\n",(int)(e-s),s);
            goto end;
        }
        if (l->state == WTK_LEXC_CMD_END) {
            ret = 0;
            break;
        }
    }
    wtk_string_set_s(&(v), "\n");
    ret = wtk_lexc_feed(l, &v);
    if (ret != 0) {
        goto end;
    }
    ret = 0;
    if (b) {
        wtk_lex_script_update(l->script);
    }
    end: wtk_strbuf_delete(buf);
    if (ret != 0) {
        if (l->script) {
            wtk_lex_script_delete(l->script);
            l->script = NULL;
        }
    }
    return l->script;
}

int wtk_lexc_compile_str_dat(wtk_lexc_t *l, char *data, int bytes)
{
    char *s, *e;
    wtk_string_t v;
    int n;
    int ret;

    s = data;
    e = s + bytes;
    while (s < e) {
        n = wtk_utf8_bytes(*s);
        wtk_string_set(&(v), s, n);
        ret = wtk_lexc_feed(l, &v);
        if (ret != 0) {
            wtk_debug("%s:%d\n", wtk_lexc_get_state(l), l->sub_state);
            wtk_debug("error: [%.*s]\n", (int)(e - s), s);
            goto end;
        }
        if (l->state == WTK_LEXC_CMD_END) {
            ret = 0;
            break;
        }
        s += n;
    }
    ret = 0;
end: 
	return ret;
}

wtk_lex_script_t* wtk_lexc_compile(wtk_lexc_t *l, char *data, int bytes)
{
    enum wtk_lex_str_state_t {
        WTK_LEX_STR_INIT, WTK_LEX_STR_ENG,
    };
    wtk_string_t v;
    int ret;
    int b = 0;

    //wtk_debug("[%.*s]\n",bytes,data);
    wtk_lexc_set_state(l, WTK_LEXC_INIT);
    if (!l->script) {
        b = 1;
        l->script = wtk_lex_script_new();
        l->script->get_expr_ths = l->get_expr_ths;
        l->script->get_expr = l->get_expr;
        l->script->use_eng_word = l->cfg->use_eng_word;
        l->script->use_act = l->cfg->use_act;
        if (l->script->use_eng_word) {
            l->script->sort_by_prob = 1;
        }
    }
    if (l->pre_dat && l->use_pre == 0) {
        //wtk_debug("compile pre\n");
        //wtk_debug("compile [%.*s]\n",l->pre_dat->len,l->pre_dat->data);
        ret = wtk_lexc_compile_str_dat(l, l->pre_dat->data, l->pre_dat->len);
        if (ret != 0) {
            wtk_debug("compile failed.\n");
            goto end;
        }
        l->use_pre = 1;
    }
    ret = wtk_lexc_compile_str_dat(l, data, bytes);
    if (ret != 0) {
        goto end;
    }
    wtk_string_set_s(&(v), "\n");
    ret = wtk_lexc_feed(l, &v);
    if (ret != 0) {
        goto end;
    }
    ret = 0;
    if (b) {
        wtk_lex_script_update(l->script);
    }
end: 
	if (ret != 0) {
        if (l->script) {
            wtk_lex_script_delete(l->script);
            l->script = NULL;
        }
    }
    return l->script;
}

wtk_lex_script_t* wtk_lexc_compile_file(wtk_lexc_t *l, char *fn)
{
    wtk_lex_script_t *script = NULL;
    char *data = NULL;
    int len;
    wtk_string_t *v;
    wtk_rbin2_item_t *ritem = NULL;

    //wtk_debug("compile %s rbin=%p\n",fn,l->rbin);
    if (l->rbin) {
        ritem = wtk_rbin2_get3(l->rbin, fn, strlen(fn), 0);
        if (!ritem) {
            goto end;
        }
        data = ritem->data->data;
        len = ritem->data->len;
    } else {
        data = file_read_buf(fn, &len);
    }
    if (!data) {
        wtk_debug("%s not found.\n", fn);
        goto end;
    }
    //wtk_debug("[%.*s]\n",len,data);
    v = wtk_dir_name(fn, '/');
    //wtk_debug("[%.*s] %s\n",v->len,v->data,fn);
    wtk_lexc_set_pwd(l, v->data, v->len);
    wtk_string_delete(v);
    //wtk_debug("pre=%p\n",l->cfg->pre_dat);
    //exit(0);
    //wtk_debug("compile dat\n");
    script = wtk_lexc_compile(l, data, len);
    if (!script) {
        wtk_debug("compile %s failed.\n", fn);
    }
    end: if (l->rbin) {
        if (ritem) {
            wtk_rbin2_item_clean(ritem);
        }
    } else {
        if (data) {
            wtk_free(data);
        }
    }
//	wtk_lex_script_print(script);
//	exit(0);
    return script;
}

wtk_lex_net_t* wtk_lexc_compile_file2(wtk_lexc_t *l, char *fn)
{
    wtk_lex_script_t *script;

    script = wtk_lexc_compile_file(l, fn);
    if (script) {
        //wtk_lex_script_print(script);
        wtk_lexc_reset(l);
        script->hide = 1;
        return wtk_lex_net_new(script);
    } else {
        wtk_lexc_reset(l);
        return NULL;
    }
}

wtk_lex_net_t* wtk_lexc_compile_str(wtk_lexc_t *l, char *data, int bytes)
{
    wtk_lex_script_t* script;

    script = wtk_lexc_compile(l, data, bytes);
    if (script) {
        wtk_lexc_reset(l);
        script->hide = 1;
        return wtk_lex_net_new(script);
    } else {
        wtk_lexc_reset(l);
        return NULL;
    }
}

wtk_lex_script_t* wtk_lexc_compile_str_script(wtk_lexc_t *l, char *data,
        int bytes)
{
    wtk_lex_script_t* script;

    script = wtk_lexc_compile(l, data, bytes);
    wtk_lexc_reset(l);
    return script;
}

