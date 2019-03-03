#ifndef WTK_CORE_RBIN_WTK_RBIN2_H_
#define WTK_CORE_RBIN_WTK_RBIN2_H_
#include "wtk/core/rbin/wtk_rbin.h"
#include "wtk/core/wtk_heap.h"
#include "wtk/core/wtk_os.h"
#include "wtk/core/cfg/wtk_source.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_rbin2 wtk_rbin2_t;
#define wtk_rbin2_get_s(rb,name) wtk_rbin2_get(rb,name,sizeof(name)-1)
#define wtk_rbin2_get2_s(rb,name) wtk_rbin2_get2(rb,name,sizeof(name)-1)

typedef struct
{
	wtk_queue_node_t q_n;
	wtk_rbin2_t *rb;
	wtk_string_t *fn;			//file name;
	wtk_string_t *data;
	int pos;		//file pos;
	int len;		//data len
	unsigned char *s;
	unsigned char *e;
	int seek_pos;	//file pos;
	int buf_pos;	//buf pos;
	unsigned reverse:1;
}wtk_rbin2_item_t;

struct wtk_rbin2
{
	wtk_queue_t list;
	wtk_heap_t *heap;
	FILE *f;
	char *fn;
	wtk_strbuf_t *buf;
	int version;
};

wtk_rbin2_t* wtk_rbin2_new();
void wtk_rbin2_delete(wtk_rbin2_t *rb);

//-------------------- write  --------------------------------
int wtk_rbin2_add(wtk_rbin2_t *rb,wtk_string_t *name,char *realname);
void wtk_rbin2_add2(wtk_rbin2_t *rb,wtk_string_t *name,char *data,int len);
int wtk_rbin2_write(wtk_rbin2_t *rb,char *fn);

//------------------ read -------------------------
int wtk_rbin2_read(wtk_rbin2_t *rb,char *fn);
int wtk_rbin2_read2(wtk_rbin2_t *rb,char *fn,unsigned int seek_pos);
int wtk_rbin2_extract(wtk_rbin2_t *rb,char *dn);

FILE* wtk_rbin2_get_file(wtk_rbin2_t *rb,char *name);
wtk_rbin2_item_t* wtk_rbin2_get(wtk_rbin2_t *rb,char *name,int len);
wtk_rbin2_item_t* wtk_rbin2_get2(wtk_rbin2_t *rb,char *name,int len);
wtk_rbin2_item_t* wtk_rbin2_get3(wtk_rbin2_t *rb,char *name,int len,int use_heap);
int wtk_rbin2_load_item(wtk_rbin2_t *rb,wtk_rbin2_item_t *item,int use_heap);
void wtk_rbin2_item_clean(wtk_rbin2_item_t *item);
void wtk_rbin2_print(wtk_rbin2_t *rb);


int wtk_rbin2_load_file(wtk_rbin2_t *rb,void *data,wtk_source_load_handler_t loader,char *fn);
void wtk_source_init_rbin2(wtk_source_t* s,wtk_rbin2_item_t *i);
void wtk_source_init_rbin2_x(wtk_source_t* s,wtk_rbin2_item_t *i);
int wtk_rbin2_file_exist(wtk_rbin2_t *rb,char *name,int len);
#ifdef __cplusplus
};
#endif
#endif
