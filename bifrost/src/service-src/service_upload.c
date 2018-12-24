#include "main/qtk_bifrost_main.h"
#include "main/qtk_bifrost_core.h"
#include "main/qtk_context.h"
#include "main/qtk_env.h"
#include "os/qtk_alloc.h"
#include "os/qtk_base.h"

#include "third/lua5.3.4/src/lua.h"
#include "third/lua5.3.4/src/lualib.h"
#include "third/lua5.3.4/src/lauxlib.h"

#include "zzip/zzip.h"
#include "zzip/plugin.h"

#include <assert.h>


typedef struct up_log{
    FILE* f;
    char fn[128];
    char buf[128];
}up_log_t;

typedef struct upload{
    qtk_thread_t route;
    qtk_upload_mode_t *up;
    qtk_context_t *ctx;
    up_log_t *log_up;
    lua_State *L;
    unsigned run : 1;
}upload_t;

#define upload_log_log(P,fmt) fprintf(P->log_up->f, "%s%d%s\n", __FUNCTION__, __LINE__, fmt)
static void qtk_bifrost_upload_free(upload_t *P);

static void upload_dir_create(upload_t *P){
    char dir_buf[20];
    int ret;
    DIR *dirptr = NULL;
    struct dirent * entry;
    const char *upload_dir = qtk_optstring("uploadDIR", "");
    dirptr = opendir(upload_dir);
    if(dirptr){//clear dir file
        while((entry = readdir(dirptr)) != 0){
            if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")){
                continue;
            }
            else{
                sprintf(dir_buf, "%s/%s",upload_dir, entry->d_name);
                ret = remove(dir_buf);
                if(ret == -1){
                    perror("remove fail\n");
                    exit(2);
                }
            }
        }
    }
    else{//create new
        ret = mkdir(upload_dir, 0777);
        if(ret == -1){
            qtk_debug("create upload dir fail\n");
            perror("mkdir fail");
            return;
        }
    } 
    snprintf(P->log_up->fn, sizeof(P->log_up->fn), "%s.log", (char*)upload_dir);
    P->log_up->f = fopen(P->log_up->fn, "a");
    if(P->log_up->f == NULL){
        wtk_debug("open upload log fail\n");
    }
}

static void qtk_upload_mode_init(upload_t *P){
    //create upload read queue and write queue,init queue,
    //create file dir and clear dir   
    P->up->upload_q_write = wtk_malloc(sizeof(wtk_blockqueue_t));
    wtk_blockqueue_init(P->up->upload_q_write);
    P->up->upload_q_read = wtk_malloc(sizeof(wtk_blockqueue_t));    
    wtk_blockqueue_init(P->up->upload_q_read);
    P->up->upload_q_send = wtk_malloc(sizeof(wtk_blockqueue_t));    
    wtk_blockqueue_init(P->up->upload_q_send);
    upload_dir_create(P);

}

void *_upload_route(void *arg)
{
    int fil_num;
    upload_t *P = arg;
    qtk_bifrost_upload_queue_t *item;
    int buf_count = 0;
    wtk_queue_node_t *node;
    char buf[BUFSIZ];
    const char *ppath = qtk_optstring("serviceBinPath", "");
    char loader[256];
    snprintf(loader, sizeof(loader), "%s&service/upload.lua", ppath);
    P->L = luaL_newstate();
    luaL_openlibs(P->L);
    int r = luaL_loadfileq(P->L, loader);
	if (r != LUA_OK) {
		qtk_debug("Can't load %s : %s\n", loader, lua_tostring(P->L, 0));
        lua_pop(P->L, 1);
        r = luaL_dofile(P->L, "service/upload.lua");
        if (r != LUA_OK) {
            qtk_debug("Retry failed: %s\n", lua_tostring(P->L, -1));
            return NULL;
        }
        qtk_debug("load lua success!\n");
	}
    else{
        lua_pcall(P->L, 0, LUA_MULTRET, 0);
    }
    while(P->run){
        node = wtk_blockqueue_pop(P->up->upload_q_send, 10000, 0);
        while(node == NULL && P->run == 1){
            if(P->up->upload_q_send->length > 0){
                node = wtk_blockqueue_pop(P->up->upload_q_send, -1, 0);
            }
            else if(!node && P->up->upload_q_read->length > 0){
                node = wtk_blockqueue_pop(P->up->upload_q_read, 10000, 0);
                if(node){                
                    item = data_offset2(node, qtk_bifrost_upload_queue_t, q_n);
                    if(!item->filename){
                        qtk_debug("filename is empty!\n");
                        goto end;
                    }
                    item->msg_strbuf = qtk_strbuf_new(128, -1);
                    fil_num= open(item->filename, O_RDONLY , S_IRWXU);
                    item->file = fil_num;
                    if(!item->file){
                        wtk_debug("open file fail\n");
                        goto end;
                    }
                    buf_count = 0;
                    buf_count = read(fil_num, buf, BUFSIZ-1);
                    while(buf_count != 0){
                        qtk_strbuf_append(item->msg_strbuf, buf, buf_count);
                        memset(buf, 0, sizeof(buf));
                        buf_count = read(fil_num, buf, BUFSIZ-1);
                    }
                    wtk_blockqueue_push(P->up->upload_q_send, &(item->q_n));
                    node = NULL;
                    close(fil_num);
                }
            }
            else{
                usleep(1000000);
            }
        }
        if(!node){  //exit
            break;
        }
        item = data_offset2(node, qtk_bifrost_upload_queue_t, q_n);
        if(!(item->msg_strbuf->data)){
            qtk_debug("send msg strbuf is empty!\n");
            goto end;
        }
        _get_file_processer(P->L);
        lua_pushlstring(P->L, item->msg_strbuf->data, item->msg_strbuf->len);
        int ret = lua_pcall(P->L, 1, 0, 0);
        if (ret != LUA_OK){
            qtk_debug("error run: %s\n", lua_tostring(P->L, -1));
            goto end;
        }
        memset(item->msg_strbuf->data, 0, item->msg_strbuf->len);
        item->msg_strbuf->len = 0;
        qtk_strbuf_delete(item->msg_strbuf);
        if(item->file){
            wtk_blockqueue_push(P->up->upload_q_write, &(item->q_n));
        }
        else{
            wtk_free(item);
        }
        
    end:
        continue;
    }
    //pop queue until empty and free
    qtk_bifrost_upload_free(P);
    qtk_debug("upload thread exit\n");
    return NULL;
}


static void qtk_bifrost_upload_free(upload_t *P){
    int fil_num;
    int buf_count = 0;
    wtk_queue_node_t *node = NULL;
    qtk_bifrost_upload_queue_t *item = NULL;
    qtk_strbuf_t *strbuf_msg = qtk_strbuf_new(128, -1);
    char buf[BUFSIZ];
    if(P->up->upload_q_read->length > 0){
        node = wtk_blockqueue_pop(P->up->upload_q_read, -1, 0);
        while(node){
            item = data_offset2(node, qtk_bifrost_upload_queue_t, q_n);
            if(!item->filename){
                qtk_debug("filename is empty!\n");
                break;
            }
            fil_num= open(item->filename, O_RDONLY , S_IRWXU);
            item->file = fil_num;
            if(!item->file){
                wtk_debug("open file fail\n");
                break;
            }
            //read dota to msg_strbuf from file
            buf_count = 0;
            buf_count = read(fil_num, buf, BUFSIZ-1);
            while(buf_count != 0){      //read file full
                qtk_strbuf_append(strbuf_msg, buf, buf_count);
                memset(buf, 0, sizeof(buf));
                buf_count = read(fil_num, buf, BUFSIZ-1);
            }
            
            lua_getglobal(P->L, "loadfilprocesser");
            lua_pushstring(P->L, strbuf_msg->data);
            int ret = lua_pcall(P->L, 1, 0, 0);
            if (ret != LUA_OK){
                qtk_debug("error run: %s\n", lua_tostring(P->L, -1));
                break;
            }
            close(fil_num);
            wtk_free(item);
            node = wtk_blockqueue_pop(P->up->upload_q_read, -1, 0);
        }
    }
    else if(P->up->upload_q_write->length > 0){
        node = wtk_blockqueue_pop(P->up->upload_q_write, -1, 0);
        while(node){
            item = data_offset2(node, qtk_bifrost_upload_queue_t, q_n);
            wtk_free(item);
            node = wtk_blockqueue_pop(P->up->upload_q_write, -1, 0);
        }
    }
}



upload_t *upload_create(void)
{
    upload_t *P = wtk_malloc(sizeof(struct upload));
    memset(P, 0, sizeof(*P));
    P->log_up = wtk_malloc(sizeof(up_log_t));
    upload_dir_create(P);
    return P;
}

int upload_init(upload_t *P, qtk_context_t *ctx, const char *param)
{
    P->up = qtk_bifrost_self()->up;
    P->ctx = ctx;
    qtk_thread_init(&P->route, _upload_route, P);
    P->run = 1;
    qtk_upload_mode_init(P);
    qtk_thread_start(&P->route);
    return 0;
}

static void _upload_clean(upload_t *P)
{
    if (P) {
        if (P->L) { lua_close(P->L); }
        qtk_free(P);
    }
    qtk_debug("cleannig\n");
}

void upload_release(upload_t *P)
{
    
    P->run = 0;
    wtk_blockqueue_wake(P->up->upload_q_read);
    wtk_blockqueue_wake(P->up->upload_q_write);
    wtk_blockqueue_wake(P->up->upload_q_send);
    qtk_thread_join(&P->route);
    wtk_blockqueue_clean(P->up->upload_q_read);
    wtk_blockqueue_clean(P->up->upload_q_write);
    wtk_blockqueue_clean(P->up->upload_q_send);
    qtk_free(P->up->upload_q_read);
    qtk_free(P->up->upload_q_write);
    qtk_free(P->up->upload_q_send);
    qtk_free(P->log_up);
    _upload_clean(P);
}

void upload_signal(upload_t *P, int signal)
{
    qtk_debug("recv a signal: %d\n", signal);
}
