#ifndef WTK_LEX_NER_WTK_HMMNE
#define WTK_LEX_NER_WTK_HMMNE
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_str_hash.h"
#include "wtk/core/wtk_str_parser.h"
#include "wtk/core/cfg/wtk_source.h"
#include "math.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_hmmne wtk_hmmne_t;
typedef struct {
    wtk_string_t *wrd;
    float b;
    float b_m;
    float m_m;
    float b_e;
    float m_e;
    float s;
} wtk_hmmnr_wrd_t;

#ifdef M_E
#undef M_E
#endif

struct wtk_hmmne {
    wtk_str_hash_t *hash;
    float B;	//B_COUNT;
    float M; 	//M_COUNT;
    float M1;	//M1_COUNT;
    float E;	//E_COUNT
    float S;	//S_COUNT;
    float B_M;	//B=>M
    float B_E;	//B=>E
    float M_M;	//M=>M
    float M_E;	//M=>E
    float unk_b;
    float unk_be;
    float unk_me;
    float unk_bm;
    float unk_mm;
};

wtk_hmmne_t* wtk_hmmne_new(int hash_hint);
wtk_hmmne_t* wtk_hmmne_new2(char *fn);
void wtk_hmmne_delete(wtk_hmmne_t *ne);
int wtk_hmmne_load(wtk_hmmne_t *ne, wtk_source_t *src);
int wtk_hmmne_load_file(wtk_hmmne_t *ne, char *fn);
#ifdef __cplusplus
}
;
#endif
#endif
