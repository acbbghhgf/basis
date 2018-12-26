#ifndef WTK_RBIN_WTK_RBIN_H_
#define WTK_RBIN_WTK_RBIN_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_os.h"
#include "wtk/core/cfg/wtk_source.h"
#include "wtk_flist.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_rbin wtk_rbin_t;
#define wtk_rbin_find_s(rb,s) wtk_rbin_find(rb,s,sizeof(s)-1)
/**
 * HDR_ENTRYS(8)
 * ENTRY: FN_SIZE:FN:OFFSET:LENGTH
 */
typedef struct
{
	wtk_queue_node_t q_n;
	wtk_string_t *fn;			//file name;
	wtk_string_t data;			//
	int pos;
}wtk_ritem_t;

struct wtk_rbin
{
	wtk_queue_t list;
	wtk_flist_t *fl;
	wtk_strbuf_t *buf;
};

wtk_rbin_t* wtk_rbin_new();
int wtk_rbin_delete(wtk_rbin_t *rb);
int wtk_rbin_read_scp(wtk_rbin_t *rb,char *fn);
int wtk_rbin_write(wtk_rbin_t *rb,char *res_dn,char *bin);
int wtk_rbin_read(wtk_rbin_t *rb,char *bin);
int wtk_rbin_extract(wtk_rbin_t *rb,char *dn);
wtk_ritem_t* wtk_rbin_find(wtk_rbin_t *rb,char *data,int len);
void wtk_ritem_print(wtk_ritem_t *i);
int wtk_ritem_get(wtk_ritem_t *i);
int wtk_ritem_unget(wtk_ritem_t *i,int c);
int wtk_rbin_load_file(wtk_rbin_t *rb,void *data,wtk_source_load_handler_t loader,char *fn);
void wtk_file_write_reverse(FILE* f,char *data,int len);
void wtk_rbin_reverse_data(unsigned char *p,int len);
#ifdef __cplusplus
};
#endif
#endif
