#include "qtk_inst_ctl.h"
#include "qtk_incub.h"
#include "qtk_fty.h"
#include "qtk/os/qtk_base.h"
#include "qtk/os/qtk_alloc.h"
#include "qtk/core/qtk_str_hashtable.h"
#include "lua/qtk_lco.h"

#include "third/lua5.3.4/src/lualib.h"
#include "third/lua5.3.4/src/lauxlib.h"

#include <assert.h>

#define INIT_SIZE   64
static qtk_abstruct_hashtable_t *H;
static qtk_dlg_worker_t *DW;

struct _pending_item {
    int ref;
    wtk_string_t ud;
    wtk_queue_node_t node;
};

struct _item {
    qtk_hashnode_t node;
    wtk_queue_t pending;
    int cnt;
};

static struct _item *_new_item(const char *n, size_t l)
{
    struct _item *item = qtk_calloc(1, sizeof(*item) + l);
    if (NULL == item) return NULL;
    wtk_string_t *key = cast(wtk_string_t *, &item->node.key);
    key->len = l;
    key->data = cast(char *, item + 1);
    memcpy(key->data, n, l);
    item->cnt = 0;
    wtk_queue_init(&item->pending);
    return item;
}

static struct _item *_get_item(const char *n, size_t l)
{
    wtk_string_t key;
    wtk_string_set(&key, cast(char *, n), l);
    return qtk_hashtable_find(H, (qtk_hashable_t *)&key);
}

int qtk_ictl_init(qtk_dlg_worker_t *dw)
{
    assert(H == NULL);
    assert(DW == NULL);
    assert(H = qtk_str_growhash_new(INIT_SIZE, offsetof(struct _item, node)));
    DW = dw;
    return 0;
}

int qtk_ictl_update_cnt(const char *n, size_t l, int diff)
{
    struct _item *item = _get_item(n, l);
    if (item) {
        item->cnt += diff;
    } else {
        assert(diff > 0);
        item = _new_item(n, l);
        if (item == NULL) return -1;
        item->cnt = diff;
        qtk_hashtable_add(H, item);
    }
    return 0;
}

int qtk_ictl_get_cnt(const char *n, size_t l)
{
    struct _item *item = _get_item(n, l);
    return item ? item->cnt : -1;
}

static int _wakeup_pending(struct _pending_item *item)
{
    lua_State *l = DW->L;
    lua_State *co = qtk_lget_yield_co(l, item->ref);
    if (co == NULL) {
        lua_pop(l, 1);
        return 0;
    }
    lua_pushlstring(co, item->ud.data, item->ud.len);
    if (lua_resume(co, NULL, 1) > LUA_YIELD) {
        qtk_debug("Resume Error %s\n", lua_tostring(co, -1));
        lua_pop(l, 1);
        qtk_lunreg_yield_co(l, item->ref);
        return -1;
    }
    qtk_lunreg_yield_co(l, item->ref);
    lua_pop(l, 1);
    return 0;
}

static int _clean_item(void *ud, struct _item *item)
{
    wtk_queue_node_t *node, *next;
    for (node=item->pending.pop; node; node=next) {
        struct _pending_item *item = data_offset(node, struct _pending_item, node);
        next = node->next;
        qtk_free(item);
    }
    qtk_free(item);
    return 0;
}

void qtk_ictl_clean()
{
    qtk_hashtable_walk(H, cast(wtk_walk_handler_t, _clean_item), NULL);
}

int qtk_ictl_pend(const char *n, size_t l, int ref, wtk_string_t *ud)
{
    struct _item *item = _get_item(n, l);
    if (item == NULL) item = _new_item(n, l);
    struct _pending_item *pi = qtk_calloc(1, sizeof(*pi) + ud->len);
    pi->ref = ref;
    pi->ud.len = ud->len;
    pi->ud.data = cast(char *, pi + 1);
    memcpy(pi->ud.data, ud->data, ud->len);
    wtk_queue_push(&item->pending, &pi->node);
    return 0;
}

int qtk_ictl_flush(const char *n, size_t l)
{
    struct _item *item = _get_item(n, l);
    if (NULL == item) return 0;
    wtk_queue_node_t *node = wtk_queue_pop(&item->pending);
    if (NULL == node) return 0;
    struct _pending_item *pi = data_offset(node, struct _pending_item, node);
    int ret =  _wakeup_pending(pi);
    qtk_free(pi);
    return ret;
}

void qtk_ictl_try_remove(const char *n, size_t l)
{
    if (!qtk_incub_exist(n, l) && !qtk_fty_exist(n, l)) {
        wtk_string_t key = {cast(char *, n), l};
        struct _item *item = qtk_hashtable_remove(H, cast(qtk_hashable_t *, &key));
        if (item) qtk_free(item);
    }
}

int qtk_ictl_get_pending_cnt(const char *n, size_t l)
{
    struct _item *item = _get_item(n, l);
    return item == NULL ? 0 : item->pending.length;
}
