typedef struct qtk_aiexc qtk_aiexc_t;
typedef struct qtk_vm qtk_vm_t


struct qtk_vm {
    lua_State *L;

};


struct qtk_aiexc {

};


static int aiexc_cb(struct qtk_zcontext *ctx, void *ud, int type, int session, uint32_t src,
                    const char *data, size_t sz) {
    lua_State *L = l
    return 0;
}


qtk_aiexc_t *aisrv_create(void) {
    qtk_aiexc_t *inst = wtk_malloc(sizeof(*inst));
    return inst;
}


void aiexc_release(qtk_aiexc_t *exc) {
    wtk_free(exc);
}


int aiexc_init(qtk_aiexc_t *exc, struct qtk_zcontext *ctx, const char *param) {

}
