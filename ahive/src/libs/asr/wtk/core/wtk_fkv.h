#ifndef WTK_CORE_WTK_FKV_H_
#define WTK_CORE_WTK_FKV_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_str_hash.h"
#include "wtk/core/wtk_strbuf.h"
#include "wtk/core/rbin/wtk_rbin2.h"
#ifdef __cplusplus
extern "C" {
#endif
#define wtk_fkv_get_int_s(kv,k,found) wtk_fkv_get_int(kv,k,sizeof(k)-1,found)
typedef struct wtk_fkv wtk_fkv_t;

/**
 * svn://192.168.0.107/speech/trunk/semdlg-lz
 * python py/wtk/kv2bin.py -i a.txt -o a.bin -t lstring
 */
typedef enum
{
	WTK_FKV_INT=0,
	WTK_FKV_BIN,
	WTK_FKV_LSTRING,
}wtk_fkv_type_t;

struct wtk_fkv
{
	wtk_strbuf_t *buf;
	FILE *f;
	wtk_str_hash_t *hash;
	unsigned int *idx;
	int nid;
	unsigned int data_offset;
	int f_of;
	int f_len;
	wtk_fkv_type_t type;
	unsigned file_want_close:1;
	unsigned str_use_byte:1;	//single char or not;
};

wtk_fkv_t* wtk_fkv_new4(wtk_rbin2_t *rbin,char *fn,int hash_hint);
wtk_fkv_t* wtk_fkv_new3(char *fn);
wtk_fkv_t* wtk_fkv_new(char *fn,int hash_hint);
wtk_fkv_t* wtk_fkv_new2(int hash_hint,FILE *f,int of,int len,int file_want_close);
void wtk_fkv_delete(wtk_fkv_t *kv);
void wtk_fkv_reset(wtk_fkv_t *kv);

wtk_string_t wtk_fkv_get_item_str(wtk_fkv_t *kv,uint32_t of,char *k,int k_len);
int wtk_fkv_get_int(wtk_fkv_t *kv,char *k,int k_len,int *found);
wtk_string_t* wtk_fkv_get_str(wtk_fkv_t *kv,char *k,int k_len);
wtk_string_t wtk_fkv_get_str2(wtk_fkv_t *kv,char *k,int k_len);

void wtk_fkv_load_all(wtk_fkv_t *kv);
wtk_string_t* wtk_fkv_get_int_key(wtk_fkv_t *kv,int id);
#ifdef __cplusplus
};
#endif
#endif
