#include "qtk_zeus_handle.h"
#include "wtk/core/wtk_alloc.h"
#include "qtk_zeus_server.h"


#define QTK_ZHANDLE_INIT_SIZE 16


static int _handle_name_cmp(void *n1, void *n2) {
    return strcmp(qtk_map_get_key(n1), qtk_map_get_key(n2));
}


static int _handle_name_del(void *key, void *value) {
    if (key) {
        wtk_free(key);
    }
    return 0;
}


static int _insert_name(qtk_zhandle_t *zh, const char *key, struct qtk_zcontext *ctx) {
    int ret = qtk_map_insert(zh->name, (char*)key, qtk_zcontext_handlep(ctx));
    if (!ret) {
        qtk_zcontext_set_name(ctx, key);
    }
    return ret;
}


static int _remove_name(qtk_zhandle_t *zh, struct qtk_zcontext *ctx) {
    char *name = (char*)qtk_zcontext_name(ctx);
    if (name) {
        qtk_map_remove(zh->name, (char*)qtk_zcontext_name(ctx));
        qtk_zcontext_set_name(ctx, NULL);
    }
    return 0;
}


static uint32_t _name_handle(qtk_zhandle_t *zh, const char *key) {
    uint32_t *handle;
    handle = (uint32_t*)qtk_map_elem(zh->name, (char*)key);
    return handle ? *handle : 0;
}


qtk_zhandle_t* qtk_zhandle_new() {
    qtk_zhandle_t *zh = wtk_malloc(sizeof(*zh));
    wtk_lock_init(&zh->lock);
    zh->handle_index = 1;
    zh->slot_cnt = 0;
    zh->slot_cap = QTK_ZHANDLE_INIT_SIZE;
    zh->slots = wtk_calloc(QTK_ZHANDLE_INIT_SIZE, sizeof(zh->slots[0]));
    zh->name = qtk_map_new(_handle_name_cmp, _handle_name_del);
    return zh;
}


int qtk_zhandle_delete(qtk_zhandle_t* zh) {
    wtk_lock_clean(&zh->lock);
    if (zh->slots) {
        wtk_free(zh->slots);
        zh->slots = NULL;
    }
    if (zh->name) {
        qtk_map_delete(zh->name);
        zh->name = NULL;
    }
    wtk_free(zh);
    return 0;
}


uint32_t qtk_zhandle_register(qtk_zhandle_t* zh, struct qtk_zcontext* ctx) {
    uint32_t handle;
    wtk_lock_lock(&zh->lock);
    if (zh->slot_cnt * 2 > zh->slot_cap) {
        struct qtk_zcontext **new_slot = wtk_calloc(zh->slot_cap*2, sizeof(new_slot[0]));
        int i;
        for (i = 0; i < zh->slot_cap; ++i) {
            if (zh->slots[i]) {
                int hash = qtk_zcontext_handle(zh->slots[i]) & (zh->slot_cap*2-1);
                new_slot[hash] = zh->slots[i];
            }
        }
        wtk_free(zh->slots);
        zh->slots = new_slot;
        zh->slot_cap *= 2;
    }
    for (;;) {
        int i;
        for (i = 0; i <= zh->slot_cnt; ++i) {
            handle = zh->handle_index + i;
            int hash = handle & (zh->slot_cap-1);
            if (NULL == zh->slots[hash]) {
                zh->slots[hash] = ctx;
                zh->handle_index = handle + 1;
                ++zh->slot_cnt;
                goto end;
            }
        }
    }
    handle = 0;
end:
    wtk_lock_unlock(&zh->lock);
    return handle;
}


int qtk_zhandle_retire(qtk_zhandle_t *zh, uint32_t handle) {
    uint32_t hash;
    struct qtk_zcontext *ctx;
    wtk_lock_lock(&zh->lock);
    hash = handle & (zh->slot_cap - 1);
    ctx = zh->slots[hash];
    if (ctx && qtk_zcontext_handle(ctx) == handle) {
        _remove_name(zh, ctx);
        zh->slots[hash] = NULL;
        --zh->slot_cnt;
    } else {
        ctx = NULL;
    }
    wtk_lock_unlock(&zh->lock);

    if (ctx) {
        qtk_zcontext_release(ctx);
    }
    return 0;
}


int qtk_zhandle_retireall(qtk_zhandle_t *zh) {
    int i;
    int finish = 0;
    for (;;) {
        for (i = 0; i < zh->slot_cap; ++i) {
            uint32_t handle = 0;
            wtk_lock_lock(&zh->lock);
            if (zh->slots[i]) {
                handle = qtk_zcontext_handle(zh->slots[i]);
            }
            if (zh->slot_cnt == 0) finish = 1;
            wtk_lock_unlock(&zh->lock);
            if (handle) {
                qtk_zhandle_retire(zh, handle);
            }
            if (finish) goto end;
        }
    }
end:
    return 0;
}


struct qtk_zcontext* qtk_zhandle_grab(qtk_zhandle_t *zh, uint32_t handle) {
    uint32_t hash;
    struct qtk_zcontext *ctx;
    wtk_lock_lock(&zh->lock);
    hash = handle & (zh->slot_cap - 1);
    ctx = zh->slots[hash];
    if (ctx && qtk_zcontext_handle(ctx) == handle) {
        qtk_zcontext_grab(ctx);
    } else {
        ctx = NULL;
    }
    wtk_lock_unlock(&zh->lock);
    return ctx;
}


uint32_t qtk_zhandle_findname(qtk_zhandle_t *zh, const char *name) {
    uint32_t handle;
    wtk_lock_lock(&zh->lock);
    handle = _name_handle(zh, name);
    wtk_lock_unlock(&zh->lock);
    return handle;
}


const char* qtk_zhandle_namehandle(qtk_zhandle_t *zh, uint32_t handle, const char *name) {
    uint32_t hash;
    struct qtk_zcontext *ctx;
    char *key;
    const char *result;
    int name_len = strlen(name);
    key = wtk_malloc(name_len+1);
    memcpy(key, name, name_len+1);
    wtk_lock_lock(&zh->lock);
    hash = handle & (zh->slot_cap - 1);
    ctx = zh->slots[hash];
    if (ctx && qtk_zcontext_handle(ctx) == handle) {
        if (_insert_name(zh, key, ctx)) {
            result = NULL;
        } else {
            result = key;
            key = NULL;
        }
    } else {
        result = NULL;
    }
    wtk_lock_unlock(&zh->lock);

    if (key) {
        wtk_free(key);
    }
    return result;
}
