#ifndef WTK_CORE_ZIP_WTK_UNZIP
#define WTK_CORE_ZIP_WTK_UNZIP
#include "wtk/core/wtk_type.h" 
#include "wtk_zip.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_unzip wtk_unzip_t;

typedef struct
{
	unsigned short prev;
	unsigned char c;
}wtk_unzip_item_t;

struct wtk_unzip
{
	int len;
	wtk_unzip_item_t *items;
	wtk_strbuf_t *buf;
	int cnt;
};


wtk_unzip_t* wtk_unzip_new();
void wtk_unzip_delete(wtk_unzip_t *z);
void wtk_unzip_reset(wtk_unzip_t *z);
int wtk_unzip_file(wtk_unzip_t *z,char *ifn,char *ofn);
#ifdef __cplusplus
};
#endif
#endif
