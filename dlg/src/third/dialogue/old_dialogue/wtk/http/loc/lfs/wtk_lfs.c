#include "wtk_lfs.h"
#include "wtk/http/misc/wtk_http_util.h"

wtk_lfs_t* wtk_lfs_new(wtk_lfs_cfg_t *cfg)
{
	wtk_lfs_t *l;

	l=(wtk_lfs_t*)wtk_malloc(sizeof(*l));
	l->cfg=cfg;
	l->buf=wtk_strbuf_new(cfg->buf_size,cfg->buf_rate);
	l->tmp_buf=wtk_strbuf_new(256,1);
	l->param.type=WTK_STRING;
	l->param.is_ref=1;
	l->param.value.str.is_ref=1;
	return l;
}

void wtk_lfs_delete(wtk_lfs_t *l)
{
	if(l->buf)
	{
		wtk_strbuf_delete(l->buf);
	}
	if(l->tmp_buf)
	{
		wtk_strbuf_delete(l->tmp_buf);
	}
	wtk_free(l);
}

void wtk_loc_lfs_attach_404(wtk_strbuf_t *buf,wtk_request_t *req)
{
	wtk_strbuf_reset(buf);
	wtk_strbuf_push(buf,req->url.data,req->url.len);
	wtk_strbuf_push_s(buf," not found.");
	req->response->status=404;
}

#ifdef WIN32
#else
#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void wtk_loc_lfs_list_dir(char *path,wtk_strbuf_t *buf,wtk_string_t *url)
{
	DIR *dir;
	struct dirent *ent;

	dir=opendir(path);
	//wtk_debug("path: %s,%p\n",path,dir);
	wtk_strbuf_reset(buf);
	if(!dir){goto end;}
	wtk_strbuf_push_s(buf,"<html><head><title>Index</title></head><body><table><tr><td>Type</td><td>Name</td></tr>");
	while(1)
	{
		ent=readdir(dir);
		if(!ent){break;}
		if(ent->d_type==DT_DIR)
		{
			if(strcmp(ent->d_name,".")==0 || strcmp(ent->d_name,"..")==0)
			{
				continue;
			}else
			{
				wtk_strbuf_push_f(buf,"<tr><td>dir</td><td><a href=\"%.*s/%s\">%s</td></tr>",url->len,url->data,ent->d_name,ent->d_name);
			}
		}else
		{
			wtk_strbuf_push_f(buf,"<tr><td>file</td><td><a href=\"%.*s/%s\">%s</td></tr>",url->len,url->data,ent->d_name,ent->d_name);
		}
	}
	wtk_strbuf_push_s(buf,"</table></body></html>");
end:
	if(dir)
	{
		closedir(dir);
	}
}
#endif

int wtk_lfs_process(wtk_lfs_t *l,wtk_request_t *req)
{
	wtk_lfs_cfg_t *cfg=l->cfg;
	wtk_strbuf_t *buf=l->buf;
	int ret;
	wtk_string_t *ct;

	//wtk_request_print(req);
	wtk_strbuf_reset(buf);
	wtk_strbuf_push(buf,cfg->dir.data,cfg->dir.len);
    if(req->url.len>1)
    {
    	wtk_strbuf_t *tmp=l->tmp_buf;

    	wtk_http_url_decode(tmp,&(req->url));
	    //wtk_strbuf_push(buf,req->url.data,req->url.len);
    	wtk_strbuf_push(buf,tmp->data,tmp->pos);
    }
	wtk_strbuf_push_c(buf,0);
    //wtk_debug("load file: %s\n",buf->data);
#ifdef WIN32
    {
        struct _stat b;

        ret=_stat(buf->data,&b);
        if(ret==0)
        {
            if(b.st_mode & _S_IFDIR)
            {
                --buf->pos;
                wtk_strbuf_push_s(buf,"/index.html");
                wtk_strbuf_push_c(buf,0);
            }
        }else
        {
            --buf->pos;
            wtk_strbuf_push_s(buf,".html");
            wtk_strbuf_push_c(buf,0);
        }
    }
#else
	{
		struct stat b;

		ret=stat(buf->data,&b);
		if(ret==0)
		{
			if(S_ISDIR(b.st_mode))
			{
				int pos;

				--buf->pos;
				pos=buf->pos;
				wtk_strbuf_push_s(buf,"/index.html");
				wtk_strbuf_push_c(buf,0);
				if(wtk_file_exist(buf->data)!=0)
				{
					static wtk_string_t html=wtk_string("text/html");

					req->response->content_type=&html;
					buf->data[pos]=0;
					wtk_loc_lfs_list_dir(buf->data,buf,&(req->url));
					wtk_response_set_body(req->response,buf->data,buf->pos);
					return 0;
				}
			}
		}else
		{
			--buf->pos;
			wtk_strbuf_push_s(buf,".html");
			wtk_strbuf_push_c(buf,0);
		}
	}
#endif
    //print_data(buf->data,buf->pos);
	if((wtk_file_exist(buf->data)==0) && (wtk_file_readable(buf->data)==0))
	{
		ct=wtk_http_file2content(buf->data,buf->pos-1);
        //wtk_debug("%*.*s\n",ct->len,ct->len,ct->data);
		req->response->content_type=ct;
		ret=wtk_strbuf_read(buf,buf->data);
		if(ret!=0)
		{
			wtk_loc_lfs_attach_404(buf,req);
		}
	}else
	{
		wtk_loc_lfs_attach_404(buf,req);
	}
    wtk_response_set_body(req->response,buf->data,buf->pos);
	return 0;
}

