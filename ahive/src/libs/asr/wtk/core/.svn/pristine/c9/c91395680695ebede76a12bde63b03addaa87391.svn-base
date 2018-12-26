#ifndef WTK_CORE_RBIN_WTK_UBIN_H_
#define WTK_CORE_RBIN_WTK_UBIN_H_
#include "wtk/core/wtk_heap.h"
#include "wtk/core/wtk_os.h"
#include "wtk/core/cfg/wtk_source.h"
#include "wtk/core/wtk_str_hash.h"
#include "wtk/core/rbin/wtk_rbin.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct wtk_ubin wtk_ubin_t;

#define WTK_UBIN_ATTR_VALID 0x00
#define WTK_UBIN_ATTR_INVALID 0x01

typedef struct wtk_ubin_item
{
	wtk_string_t *fn;
	wtk_string_t *data;
	int seek_pos;
	char attr;
}wtk_ubin_item_t;

struct wtk_ubin
{
	char *fn;
	int item_len;
	int item_num;
	wtk_heap_t *heap;
	wtk_str_hash_t *hash;
};

#define wtk_ubin_new_item(b) wtk_heap_malloc((b)->heap,sizeof(wtk_ubin_item_t))
wtk_ubin_t* wtk_ubin_new(unsigned int slot);
void wtk_ubin_delete(wtk_ubin_t *b);
void wtk_ubin_reset(wtk_ubin_t *b);
int wtk_ubin_read(wtk_ubin_t *b,char *fn,int read_data);
int wtk_ubin_read_all_data(wtk_ubin_t *b);
int wtk_ubin_read_data(wtk_ubin_t *b,wtk_ubin_item_t *item);
int wtk_ubin_write_all(wtk_ubin_t *b,char *fn);
int wtk_ubin_write_item(wtk_ubin_t *b,FILE *f,wtk_ubin_item_t *item);
int wtk_ubin_write_data(wtk_ubin_t *b,FILE *f,wtk_ubin_item_t *item);
int wtk_ubin_write_attr(wtk_ubin_t *b,FILE *f,wtk_ubin_item_t *item);
wtk_ubin_item_t* wtk_ubin_add_item(wtk_ubin_t *b,wtk_string_t *fn,wtk_string_t *data,char attr);
int wtk_ubin_append(wtk_ubin_t *b,wtk_string_t *fn,wtk_string_t *data,char attr);
wtk_ubin_item_t* wtk_ubin_find_item(wtk_ubin_t *b,wtk_string_t *fn);
#ifdef __cplusplus
}
#endif
#endif
