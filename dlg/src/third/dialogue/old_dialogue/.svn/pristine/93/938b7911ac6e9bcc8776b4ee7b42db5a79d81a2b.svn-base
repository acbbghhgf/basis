#ifndef WTK_CORE_CFG_WTK_MAIN_CFG_H_
#define WTK_CORE_CFG_WTK_MAIN_CFG_H_
#include "wtk/core/cfg/wtk_cfg_file.h"
#include "wtk/core/wtk_arg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_main_cfg wtk_main_cfg_t;
typedef int (*wtk_main_cfg_init_f)(void *cfg);
typedef int (*wtk_main_cfg_clean_f)(void *cfg);
typedef int (*wtk_main_cfg_update_local_f)(void *cfg,wtk_local_cfg_t *lc);
typedef int (*wtk_main_cfg_update_f)(void *cfg);
typedef int (*wtk_main_cfg_update2_f)(void *cfg,wtk_source_loader_t *sl);
typedef void (*wtk_main_cfg_update_arg_f)(void *cfg,wtk_arg_t *arg);

#define wtk_main_cfg_new_type(type,fn) wtk_main_cfg_new(sizeof(CAT(type,_t)), \
		(wtk_main_cfg_init_f)CAT(type,_init),\
		(wtk_main_cfg_clean_f)CAT(type,_clean),\
		(wtk_main_cfg_update_local_f)CAT(type,_update_local), \
		(wtk_main_cfg_update_f)CAT(type,_update),fn)

#define wtk_main_cfg_new_str_type(type,data,bytes,dn) wtk_main_cfg_new_str(sizeof(CAT(type,_t)), \
		(wtk_main_cfg_init_f)CAT(type,_init),\
		(wtk_main_cfg_clean_f)CAT(type,_clean),\
		(wtk_main_cfg_update_local_f)CAT(type,_update_local), \
		(wtk_main_cfg_update_f)CAT(type,_update),data,bytes,dn)

#define wtk_main_cfg_new_type2(type,fn,arg) wtk_main_cfg_new6(sizeof(CAT(type,_t)), \
		(wtk_main_cfg_init_f)CAT(type,_init),\
		(wtk_main_cfg_clean_f)CAT(type,_clean),\
		(wtk_main_cfg_update_local_f)CAT(type,_update_local), \
		(wtk_main_cfg_update_f)CAT(type,_update),\
		(wtk_main_cfg_update_arg_f)CAT(type,_update_arg),fn,arg,0)

#define wtk_main_cfg_new_type3(type,fn,section) wtk_main_cfg_new6(sizeof(CAT(type,_t)), \
		(wtk_main_cfg_init_f)CAT(type,_init),\
		(wtk_main_cfg_clean_f)CAT(type,_clean),\
		(wtk_main_cfg_update_local_f)CAT(type,_update_local), \
		(wtk_main_cfg_update_f)CAT(type,_update),\
		(wtk_main_cfg_update_arg_f)0,fn,0,section)

#define wtk_main_cfg_new_type4(type,dir,data,len)  wtk_main_cfg_new7(sizeof(CAT(type,_t)), \
		(wtk_main_cfg_init_f)CAT(type,_init),\
		(wtk_main_cfg_clean_f)CAT(type,_clean),\
		(wtk_main_cfg_update_local_f)CAT(type,_update_local), \
		(wtk_main_cfg_update_f)CAT(type,_update),dir,1,data,len)

struct wtk_main_cfg
{
	wtk_cfg_file_t *cfile;
	void* cfg;							//wtk_xxx_cfg_t, which size if cfg_bytes;
	int cfg_bytes;
	wtk_main_cfg_init_f init;
	wtk_main_cfg_clean_f clean;
	wtk_main_cfg_update_local_f update_lc;
	wtk_main_cfg_update_f update;
	wtk_main_cfg_update2_f update2;
	//wtk_main_cfg_update_arg_f update_arg;
};

wtk_main_cfg_t *wtk_main_cfg_new(int cfg_bytes,wtk_main_cfg_init_f init,
		wtk_main_cfg_clean_f clean,wtk_main_cfg_update_local_f update_lc,
		wtk_main_cfg_update_f update,char *fn);

/**
 * @brief create configure, update configure or not by update_cfg;
 */
wtk_main_cfg_t *wtk_main_cfg_new2(int cfg_bytes,wtk_main_cfg_init_f init,
		wtk_main_cfg_clean_f clean,wtk_main_cfg_update_local_f update_lc,
		wtk_main_cfg_update_f update,char *fn,int update_cfg);

/**
 * @brief update from argc,argv;
 */
wtk_main_cfg_t* wtk_main_cfg_new3(int cfg_bytes,wtk_main_cfg_init_f init,
		wtk_main_cfg_clean_f clean,wtk_main_cfg_update_local_f update_lc,
		wtk_main_cfg_update_f update,char *fn,int argc,char **argv);
/**
 * @brief update from arg;
 */
wtk_main_cfg_t *wtk_main_cfg_new4(int cfg_bytes,wtk_main_cfg_init_f init,
		wtk_main_cfg_clean_f clean,wtk_main_cfg_update_local_f update_lc,
		wtk_main_cfg_update_f update,char *fn,wtk_arg_t *arg);

/**
 *@param cfg_section, if cfg_section is "http:nk", update by nk local cfg;
 */
wtk_main_cfg_t* wtk_main_cfg_new5(int cfg_bytes,wtk_main_cfg_init_f init,
		wtk_main_cfg_clean_f clean,wtk_main_cfg_update_local_f update_lc,
		wtk_main_cfg_update_f update,char *fn,wtk_arg_t *arg,char *cfg_section);

wtk_main_cfg_t* wtk_main_cfg_new6(int cfg_bytes,wtk_main_cfg_init_f init,
		wtk_main_cfg_clean_f clean,wtk_main_cfg_update_local_f update_lc,
		wtk_main_cfg_update_f update,
		wtk_main_cfg_update_arg_f update_arg,
		char *fn,wtk_arg_t *arg,char *cfg_section);

wtk_main_cfg_t *wtk_main_cfg_new7(int cfg_bytes,wtk_main_cfg_init_f init,
		wtk_main_cfg_clean_f clean,wtk_main_cfg_update_local_f update_lc,
		wtk_main_cfg_update_f update,wtk_string_t *dir,int update_cfg,char *data,int len);


int wtk_main_cfg_delete(wtk_main_cfg_t *cfg);
void wtk_main_cfg_update(wtk_main_cfg_t *cfg,int argc,char **argv);
int wtk_main_cfg_update_cfg(wtk_main_cfg_t *cfg);

int wtk_main_cfg_bytes(wtk_main_cfg_t *cfg);
/**
 * @param section,if section is "httpd:nk",update by nk local cfg;
 */
int wtk_main_cfg_update_cfg2(wtk_main_cfg_t *cfg,char *section);


wtk_main_cfg_t *wtk_main_cfg_new_str(int cfg_bytes,wtk_main_cfg_init_f init,
		wtk_main_cfg_clean_f clean,wtk_main_cfg_update_local_f update_lc,
		wtk_main_cfg_update_f update,char *data,int bytes,char *dn);
#ifdef __cplusplus
};
#endif
#endif
