#ifndef WTK_LEX_FST_WTK_NLGFST2
#define WTK_LEX_FST_WTK_NLGFST2
#include "wtk/core/wtk_type.h" 
#include "wtk_nlgnet.h"
#include "wtk_nlgfst.h"
#include "wtk/lex/nlg/wtk_nlg2.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_nlgfst2 wtk_nlgfst2_t;

struct wtk_nlgfst2 {
    wtk_nlg2_t *nlg;
    wtk_nlgnet_t *net;
    wtk_nlgnet_state_t *cur_state;
    int cur_state_round;
    unsigned debug :1;
};

wtk_nlgfst2_t* wtk_nlgfst2_new(wtk_nlg2_t *nlg);
void wtk_nlgfst2_delete(wtk_nlgfst2_t *f);
void wtk_nlgfst2_reset(wtk_nlgfst2_t *f);
void wtk_nlgfst2_set_net(wtk_nlgfst2_t *f, wtk_nlgnet_t *net);
int wtk_nlgfst2_feed(wtk_nlgfst2_t *fst, wtk_nlgfst_act_t *act);
#ifdef __cplusplus
}
;
#endif
#endif
