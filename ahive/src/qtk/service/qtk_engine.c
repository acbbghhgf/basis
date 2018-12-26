#include <assert.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"


typedef struct qtk_engine qtk_engine_t;


struct qtk_engine {

};


qtk_engine_t* engine_create(void) {
    qtk_engine_t *inst = wtk_malloc(sizeof(*engine));
    return inst;
}


void engine_release(qtk_engine_t *inst) {
    wtk_free(inst);
}


static int engine_cb(struct qtk_zcontext *ctx, void *ud, int type, int session, uint32_t src, const char *data, size_t sz) {

    return 0;
}


int engine_init(qtk_engine_t *inst, struct qtk_zcontext *ctx, const char *param) {
    assert(param);
    size_t *sz = strlen(param);
    char method[sz+1];
    char res[sz+1];
    sscanf(sz, "%s %s", method, res);

}
