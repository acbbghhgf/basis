#include "qtk_incub.h"
#include "qtk/core/qtk_str_hashtable.h"
#include "wtk/core/wtk_str.h"
#include "qtk/os/qtk_alloc.h"
#include "qtk/os/qtk_base.h"

#include <assert.h>

#define INIT_SIZE   64
static qtk_abstruct_hashtable_t *INCUB;

struct _item {
    qtk_hashnode_t node;
    qtk_variant_t *var;
};

void qtk_incub_init()
{
    assert(INCUB == NULL);
    assert(INCUB = qtk_str_growhash_new(INIT_SIZE, offsetof(struct _item, node)));
}

int qtk_incub_add(qtk_variant_t *var)
{
    if (qtk_incub_find(var->name.data, var->name.len)) return -1;
    struct _item *item = qtk_calloc(1, sizeof(*item) + var->name.len);
    if (NULL == item) return -1;
    item->var = var;
    wtk_string_t *key = (wtk_string_t *)&item->node.key;
    key->len = var->name.len;
    key->data = cast(char *, item + 1);
    memcpy(key->data, var->name.data, key->len);
    var->zygote = 1;
    return qtk_hashtable_add(INCUB, item);
}

qtk_variant_t *qtk_incub_remove(const char *k, size_t l)
{
    wtk_string_t key;
    wtk_string_set(&key, cast(char *, k), l);
    struct _item *item = qtk_hashtable_remove(INCUB, (qtk_hashable_t *)&key);
    if (item) {
        qtk_variant_t *var = item->var;
        qtk_free(item);
        return var;
    }
    return NULL;
}

qtk_variant_t *qtk_incub_find(const char *k, size_t l)
{
    wtk_string_t key;
    wtk_string_set(&key, cast(char *, k), l);
    struct _item *item = qtk_hashtable_find(INCUB, (qtk_hashable_t *)&key);
    return item ? item->var : NULL;
}

static int _clean_item(void *ud, struct _item *item)
{
    qtk_variant_t *var = item->var;
    qtk_variant_t *v = qtk_incub_remove(var->name.data, var->name.len);
    assert(v == var);
    qtk_variant_exit(var);
    qtk_variant_delete(var); // program will exit, close notifier will not be called, so free now
    return 0;
}

void qtk_incub_clean()
{
    qtk_hashtable_walk(INCUB, cast(wtk_walk_handler_t, _clean_item), NULL);
}

int qtk_incub_exist(const char *k, size_t l)
{
    return NULL == qtk_incub_find(k, l) ? 0 : 1;
}

int qtk_incub_clear(const char *k, size_t l)
{
    qtk_variant_t *var = qtk_incub_find(k, l);
    if (var) qtk_variant_exit(var);
    return 0;
}
