#include "wtk_vecfn.h" 
#include "wtk/core/wtk_os.h"
#include <stdlib.h>

wtk_vecfn_node_t* wtk_vecfn_node_new(int vec_size)
{
	wtk_vecfn_node_t *node;

	node=(wtk_vecfn_node_t*)wtk_malloc(sizeof(wtk_vecfn_node_t));
	node->v=wtk_vecf_new(vec_size);
	node->pos=0;
	node->left=0;
	node->right=0;
	node->content_pos=0;
	return node;
}

void wtk_vecfn_node_delete(wtk_vecfn_node_t *node)
{
	wtk_vecf_delete(node->v);
	wtk_free(node);
}

void wtk_vecfn_node_cpy(wtk_vecfn_node_t *dst,wtk_vecfn_node_t *src)
{
	wtk_vecf_cpy(dst->v,src->v);
	dst->pos=src->pos;
	dst->left=src->left;
	dst->right=src->right;
	dst->content_pos=src->content_pos;
}

void wtk_vecfn_node_print(wtk_vecfn_node_t *node)
{
	wtk_debug("===============================\n");
	printf("pos: %u\n",node->pos);
	printf("left: %u\n",node->left);
	printf("right: %u\n",node->right);
	printf("content: %u\n",node->content_pos);
}

int wtk_vecfn_load_vec(wtk_vecfn_t *f,wtk_vecfn_node_t *node)
{
	int ret;

	ret=fseek(f->f,node->pos,SEEK_SET);
	if(ret!=0)
	{
		wtk_debug("seek to %d failed.\n",node->pos);
		perror("");
		goto end;
	}
	ret=fread(node->v->p,4,node->v->len,f->f);
	if(ret!=node->v->len)
	{
		wtk_debug("load vec data failed(len=%d).\n",node->v->len);
		ret=-1;goto end;
	}
	ret=fread(&(node->left),4,1,f->f);
	if(ret!=1)
	{
		wtk_debug("load left failed.\n");
		ret=-1;goto end;
	}
	ret=fread(&(node->right),4,1,f->f);
	if(ret!=1)
	{
		wtk_debug("load right failed.\n");
		ret=-1;goto end;
	}
	//wtk_debug("pos: tel=%ld\n",ftell(f->f));
	ret=fread(&(node->content_pos),4,1,f->f);
	if(ret!=1)
	{
		wtk_debug("load content failed.\n");
		ret=-1;goto end;
	}
	ret=0;
end:
	//wtk_vecfn_node_print(node);
	//wtk_debug("ret=%d content=%d\n",ret,node->content_pos);
	return ret;
}

int wtk_vecfn_load_vec_content(wtk_vecfn_t *f,wtk_vecfn_node_t *node,wtk_strbuf_t *buf)
{
	int ret;
	int v;

	wtk_strbuf_reset(buf);
	///wtk_debug("content=%d\n",node->content_pos);
	ret=fseek(f->f,node->content_pos,SEEK_SET);
	//wtk_debug("ret=%d\n",ret);
	if(ret!=0)
	{
		wtk_debug("seek content failed.\n");
		goto end;
	}
	ret=fread(&v,4,1,f->f);
	//wtk_debug("ret=%d,v=%d\n",ret,v);
	if(ret!=1)
	{
		wtk_debug("read content length failed.\n");
		ret=-1;goto end;
	}
	wtk_strbuf_expand(buf,v);
	ret=fread(buf->data,v,1,f->f);
	//wtk_debug("ret=%d\n",ret);
	if(ret!=1)
	{
		wtk_debug("read content data failed.\n");
		ret=-1;goto end;
	}
	//wtk_debug("[%.*s]\n",buf->pos,buf->data);
	buf->pos=v;
	ret=0;
end:
	//wtk_debug("ret=%d cpos=%d\n",ret,node->content_pos);
	return ret;
}


void wtk_vecfn_load(wtk_vecfn_t *f)
{
	wtk_vecfn_node_t *node;
	uint64_t v;

	v=file_length(f->f);
	//wtk_debug("v=%lu\n",v);
	if(v==0)
	{
		f->load=0;
		f->free_pos=0;
	}else
	{
		f->load=1;
		fseek(f->f,SEEK_SET,0);
		node=f->root;
		node->pos=0;
		wtk_vecfn_load_vec(f,node);
		f->free_pos=v;
	}
}




wtk_vecfn_t* wtk_vecfn_new(char *fn,int vec_size)
{
	wtk_vecfn_t *f;

	f=(wtk_vecfn_t*)wtk_malloc(sizeof(wtk_vecfn_t));
	f->buf=wtk_strbuf_new(256,1);
	f->root=wtk_vecfn_node_new(vec_size);
	f->tmp=wtk_vecfn_node_new(vec_size);
	f->left=wtk_vecfn_node_new(vec_size);
	f->right=wtk_vecfn_node_new(vec_size);
	wtk_mkdir_p(fn,'/',0);
	//f->f=fopen(fn,"a+b");
	if(wtk_file_exist(fn)==0)
	{
		f->f=fopen(fn,"rb");
	}else
	{
		f->f=fopen(fn,"wb");
	}
	wtk_vecfn_load(f);
	return f;
}

void wtk_vecfn_delete(wtk_vecfn_t *f)
{
	wtk_vecfn_node_delete(f->left);
	wtk_vecfn_node_delete(f->right);
	wtk_vecfn_node_delete(f->tmp);
	wtk_vecfn_node_delete(f->root);
	wtk_strbuf_delete(f->buf);
	if(f->f)
	{
		fclose(f->f);
	}
	wtk_free(f);
}

int wtk_vecfn_write_string(FILE *f,char *s,int s_bytes)
{
	int ret;

	ret=fwrite((char*)&(s_bytes),4,1,f);
	if(ret!=1){ret=-1;goto end;}
	ret=fwrite(s,s_bytes,1,f);
	if(ret!=1){ret=-1;goto end;}
	ret=0;
end:
	return ret;
}


int wtk_vecfn_save(wtk_vecfn_t *f,wtk_vecf_t *v,char *q,int q_bytes,char *a,int a_bytes,unsigned int *pos)
{
	int n;
	int ret;

	if(pos)
	{
		*pos=f->free_pos;
	}
	ret=fseek(f->f,f->free_pos,SEEK_SET);
	if(ret!=0){goto end;}
	n=v->len*sizeof(float);
	ret=fwrite(v->p,n,1,f->f);
	if(ret!=1){ret=-1;goto end;}
	f->free_pos+=n;
	n=0;
	ret=fwrite((char*)&n,4,1,f->f);
	if(ret!=1){ret=-1;goto end;}
	f->free_pos+=4;
	n=0;
	ret=fwrite((char*)&n,4,1,f->f);
	if(ret!=1){ret=-1;goto end;}
	f->free_pos+=4+4+q_bytes+4;
	//wtk_debug("content=%u\n",f->free_pos);
	ret=fwrite((char*)&(f->free_pos),4,1,f->f);
	if(ret!=1){ret=-1;goto end;}
	ret=wtk_vecfn_write_string(f->f,q,q_bytes);
	if(ret!=0){goto end;}
	ret=wtk_vecfn_write_string(f->f,a,a_bytes);
	f->free_pos+=a_bytes+4;
	if(ret!=0){goto end;}
	ret=0;
end:
	return ret;
}

int wtk_vecfn_save_vec_content(wtk_vecfn_t *f,wtk_vecfn_node_t *node,char *a,int a_bytes)
{
	int ret;
	int v;

	//wtk_debug("file: %ld\n",file_length(f->f));
	v=node->pos+sizeof(float)*node->v->len+4+4;
	ret=fseek(f->f,v,SEEK_SET);
	if(ret!=0){goto end;}
	//wtk_debug("seek to %d tel=%ld\n",v,ftell(f->f));
	v=f->free_pos;
	//wtk_debug("write free_pos=%d/%d\n",f->free_pos,v);
	ret=fwrite((char*)&(v),4,1,f->f);
	//wtk_debug("ret=%d\n",ret);
	if(ret!=1){ret=-1;goto end;}
	node->content_pos=v;
	if(node->pos==0)
	{
		f->root->content_pos=v;
	}
	//wtk_debug("content=%d\n",node->content_pos);
	ret=fseek(f->f,f->free_pos,SEEK_SET);
	if(ret!=0){goto end;}
	//wtk_debug("[%.*s]\n",a_bytes,a);
	//wtk_debug("abytes=%d\n",a_bytes);
	ret=wtk_vecfn_write_string(f->f,a,a_bytes);
	f->free_pos+=a_bytes+4;
	if(ret!=0){goto end;}
	ret=0;
end:
	//wtk_vecfn_load_vec(f,node);
	return ret;
}

int wtk_vecfn_save_left_child(wtk_vecfn_t *f,wtk_vecfn_node_t *node,unsigned int left_pos)
{
	int ret;
	int v;

	node->left=left_pos;
	if(node->pos==0)
	{
		f->root->left=left_pos;
	}
	v=node->pos+sizeof(float)*node->v->len;
	ret=fseek(f->f,v,SEEK_SET);
	if(ret!=0){goto end;}
	v=left_pos;
	ret=fwrite((char*)&(v),4,1,f->f);
	if(ret!=1){ret=-1;goto end;}
	ret=0;
end:
	//wtk_vecfn_load_vec(f,node);
	return ret;
}

int wtk_vecfn_save_right_child(wtk_vecfn_t *f,wtk_vecfn_node_t *node,unsigned int right_pos)
{
	int ret;
	int v;

	node->right=right_pos;
	if(node->pos==0)
	{
		f->root->right=right_pos;
	}
	v=node->pos+sizeof(float)*node->v->len+4;
	ret=fseek(f->f,v,SEEK_SET);
	if(ret!=0){goto end;}
	v=right_pos;
	ret=fwrite((char*)&(v),4,1,f->f);
	if(ret!=1){ret=-1;goto end;}
	ret=0;
end:
	//wtk_vecfn_load_vec(f,node);
	return ret;
}


wtk_string_t wtk_vecfn_get(wtk_vecfn_t *f,wtk_vecf_t *v1,float like_thresh,char *q,int q_bytes,char *a,int a_bytes,int add)
{
	wtk_string_t v;
	wtk_vecfn_node_t *node;
	wtk_vecfn_node_t *left;
	wtk_vecfn_node_t *right;
	float f1,f2;
	float t;
	int ret;
	unsigned int pos;

	//wtk_debug("Q: %.*s\n",q_bytes,q);
	wtk_string_set(&(v),0,0);
	if(!f->load)
	{
		if(add)
		{
			ret=wtk_vecfn_save(f,v1,q,q_bytes,a,a_bytes,&pos);
			if(ret==0)
			{
				wtk_vecf_cpy(f->root->v,v1);
				f->root->pos=0;
				f->root->left=0;
				f->root->right=0;
				f->root->content_pos=pos+sizeof(float)*v1->len+4+4+4+4+q_bytes;
				//wtk_debug("pos=%d\n",f->root->content_pos);
				f->load=1;
			}
		}
		goto end;
	}
	node=f->tmp;
	left=f->left;
	right=f->right;
	wtk_vecfn_node_cpy(node,f->root);
	t=wtk_vecf_cos(v1,node->v);
	while(1)
	{
		//wtk_debug("t=%f left=%d right=%d\n",t,node->left,node->right);
		if(t>=like_thresh)
		{
			//wtk_debug("pos=%d-%d\n",node->pos,node->content_pos);
			ret=wtk_vecfn_load_vec_content(f,node,f->buf);
			if(ret!=0)
			{
				goto end;
			}
			//wtk_debug("load %.*s\n",f->buf->pos,f->buf->data);
			wtk_string_set(&(v),f->buf->data,f->buf->pos);
			if(add)
			{
				if(wtk_string_cmp(&v,a,a_bytes)!=0)
				{
					//wtk_debug("save %.*s\n",a_bytes,a);
					wtk_vecfn_save_vec_content(f,node,a,a_bytes);
				}
			}
			break;
		}

		if(node->left>0 && node->right>0)
		{
			left->pos=node->left;
			wtk_vecfn_load_vec(f,left);
			right->pos=node->right;
			wtk_vecfn_load_vec(f,right);
			f1=wtk_vecf_dist(left->v,v1);
			f2=wtk_vecf_dist(right->v,v1);
			//wtk_debug("f1=%f f2=%f\n",f1,f2);
			if(f1<f2)
			{
				node=left;
			}else
			{
				node=right;
			}
			t=wtk_vecf_cos(v1,node->v);
		}else if(node->left>0)
		{
			left->pos=node->left;
			wtk_vecfn_load_vec(f,left);
			//wtk_vecf_print(left->v);
			f1=wtk_vecf_cos(left->v,v1);
			//wtk_debug("f1=%f t=%f\n",f1,t);
			if(f1>t)
			{
				node=left;
				t=f1;
			}else
			{
				if(add)
				{
					wtk_vecfn_save(f,v1,q,q_bytes,a,a_bytes,&pos);
					wtk_vecfn_save_right_child(f,node,pos);
				}
				//not found;
				goto end;
			}
		}else if(node->right>0)
		{
			right->pos=node->right;
			wtk_vecfn_load_vec(f,right);
			f1=wtk_vecf_cos(right->v,v1);
			if(f1>t)
			{
				node=right;
				t=f1;
			}else
			{
				if(add)
				{
					wtk_vecfn_save(f,v1,q,q_bytes,a,a_bytes,&pos);
					wtk_vecfn_save_left_child(f,node,pos);
				}
				//not found;
				goto end;
			}
		}else
		{
			if(add)
			{
				//wtk_vecf_print(v1);
				wtk_vecfn_save(f,v1,q,q_bytes,a,a_bytes,&pos);
				//wtk_debug("pos=%d\n",pos);
				wtk_vecfn_save_left_child(f,node,pos);
			}
			//not found;
			goto end;
		}
	}
end:
	return v;
}


