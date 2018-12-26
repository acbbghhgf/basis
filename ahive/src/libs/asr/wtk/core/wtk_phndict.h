#ifndef WTK_CORE_WTK_PHNDICT
#define WTK_CORE_WTK_PHNDICT
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_str_hash.h"
#include "wtk/core/wtk_strpool.h"
#include "wtk/core/wtk_os.h"
#include "wtk/core/cfg/wtk_source.h"
#include "wtk/core/wtk_str_parser.h"
#include "wtk/core/wtk_str.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_phndict wtk_phndict_t;

#define wtk_phndict_find_s(dict,w) wtk_phndict_find(dict,w,sizeof(w)-1)

typedef struct
{
	wtk_string_t *wrd;
	int nphn;
	wtk_strpool_xitem_t **phns;
}wtk_phndict_wrd_t;

struct wtk_phndict
{
	wtk_str_hash_t* hash;
	wtk_strpool_t* pool;
	int nphn;
};

wtk_phndict_t* wtk_phndict_new(char *fn);
void wtk_phndict_delete(wtk_phndict_t *phn);
wtk_phndict_wrd_t* wtk_phndict_find(wtk_phndict_t *dict,char *wrd,int bytes);

void wtk_phndict_wrd_print(wtk_phndict_wrd_t *wrd);
#ifdef __cplusplus
};
#endif
#endif
