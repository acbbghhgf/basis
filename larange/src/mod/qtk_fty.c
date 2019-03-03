#include "qtk_fty.h"
#include "qtk/os/qtk_alloc.h"
#include "qtk/os/qtk_base.h"
#include "qtk/core/qtk_str_hashtable.h"

#include <assert.h>

#define INIT_SIZE   64
static qtk_abstruct_hashtable_t *FTY;

struct _item {
    qtk_hashnode_t node;
    wtk_queue_t free;
    wtk_queue_t use;
};

void qtk_fty_init()
{
    assert(FTY == NULL);
    assert(FTY = qtk_str_growhash_new(INIT_SIZE, offsetof(struct _item, node)));
}

qtk_variant_t *qtk_fty_pop(const char *k, size_t l)
{
    wtk_string_t key;
    wtk_string_set(&key, (char *)k, l);
    struct _item *item = qtk_hashtable_find(FTY, (qtk_hashable_t *)&key);
    if (NULL == item) return NULL;
    wtk_queue_node_t *node = wtk_queue_pop(&item->free);
    if (NULL == node) return NULL;
    qtk_variant_t *var = data_offset(node, qtk_variant_t, node);
    assert(var->in_useq == 0 && var->in_freeq == 1);
    var->in_useq = 1;
    var->in_freeq = 0;
    wtk_queue_push(&item->use, node);
    return var;
}

int qtk_fty_push(qtk_variant_t *var)
{
    assert(var->zygote == 0);
    struct _item *item = qtk_hashtable_find(FTY, (qtk_hashable_t *)&var->name);
    if (NULL == item) {
        item = qtk_calloc(1, sizeof(*item) + var->name.len);
        if (NULL == item) return -1;
        wtk_string_t *nodeKey = cast(wtk_string_t *, &item->node.key);
        nodeKey->len = var->name.len;
        nodeKey->data = cast(char *, item + 1);
        memcpy(nodeKey->data, var->name.data, nodeKey->len);
        wtk_queue_init(&item->free);
        wtk_queue_init(&item->use);
        qtk_hashtable_add(FTY, item);

        assert(var->in_useq == 0 && var->in_freeq == 0);
        var->in_freeq = 1;
        wtk_queue_push(&item->free, &var->node);
        return 0;
    }
    assert(var->in_freeq == 0);
    if (var->in_useq) {
        wtk_queue_remove(&item->use, &var->node);
        var->in_useq = 0;
    }
    var->in_freeq = 1;
    wtk_queue_push(&item->free, &var->node);
    return 0;
}

void qtk_fty_remove(qtk_variant_t *var)
{
    struct _item *item = qtk_hashtable_find(FTY, (qtk_hashable_t *)&var->name);
    if (NULL == item) return;
    if (var->in_useq)
        wtk_queue_remove(&item->use, &var->node);
    if (var->in_freeq)
        wtk_queue_remove(&item->free, &var->node);
    if (item->free.length == 0 && item->use.length == 0) {
        qtk_hashtable_remove(FTY, cast(qtk_hashable_t *, &var->name));
        qtk_free(item);
    }
}

static int _fty_clean_item(void *data, struct _item *item)
{
    unuse(data);
    wtk_queue_node_t *node;

    while ((node = wtk_queue_pop(&item->free))) {
        qtk_variant_t *v = data_offset(node, qtk_variant_t, node);
        qtk_variant_exit(v);
        qtk_variant_delete(v);
    }

    while ((node = wtk_queue_pop(&item->use))) {
        qtk_variant_t *v = data_offset(node, qtk_variant_t, node);
        qtk_variant_exit(v);
        qtk_variant_delete(v);
    }
    // program will exit, close notifier will not be called, so free now
    qtk_hashtable_remove(FTY, cast(qtk_hashable_t *, &item->node.key));
    qtk_free(item);
    return 0;
}

void qtk_fty_clean()
{
    qtk_hashtable_walk(FTY, cast(wtk_walk_handler_t, _fty_clean_item), NULL);
}

int qtk_fty_exist(const char *k, size_t l)
{
    wtk_string_t key = {cast(char *, k), l};
    return NULL == qtk_hashtable_find(FTY, (qtk_hashable_t *)&key) ? 0 : 1;
}

int qtk_fty_clear(const char *k, size_t l)
{
    wtk_string_t key = {cast(char *, k), l};
    struct _item *item = qtk_hashtable_find(FTY, (qtk_hashable_t *)&key);
    if (NULL == item) return 0;
    wtk_queue_node_t *node;
    while ((node = wtk_queue_pop(&item->free))) {
        qtk_variant_t *v = data_offset(node, qtk_variant_t, node);
        qtk_variant_exit(v);
    }
    while ((node = wtk_queue_pop(&item->use))) {
        qtk_variant_t *v = data_offset(node, qtk_variant_t, node);
        qtk_variant_exit(v);
    }
    return 0;
}
