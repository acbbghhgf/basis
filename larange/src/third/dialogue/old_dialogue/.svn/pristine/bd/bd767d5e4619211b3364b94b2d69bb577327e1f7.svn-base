#include "wtk_txtree.h"
#include "wtk/core/cfg/wtk_source.h"

void wtk_tnode_print(wtk_tnode_t *n)
{
	wtk_queue_node_t *qn;
	wtk_tnode_t *n2;

	wtk_debug("=============== %p:child=%d ===================\n",n,wtk_queue2_len(&(n->child)));
	if(n->len>0)
	{
		printf("%.*s\n",n->len,n->data);
	}else
	{
		printf("root\n");
	}
	for(qn=n->child.pop;qn;qn=qn->next)
	{
		n2=data_offset(qn,wtk_tnode_t,q_n);
		wtk_tnode_print(n2);
	}
}

wtk_tnode_t* wtk_txtree_new_node(wtk_txtree_t *t,char *data,int len)
{
	wtk_tnode_t *n;
	int i;

	n=(wtk_tnode_t*)wtk_heap_malloc(t->heap,sizeof(wtk_tnode_t));
	wtk_queue2_init(&(n->child));
	n->is_leaf=0;
	n->len=len;
	for(i=0;i<len;++i)
	{
		n->data[i]=data[i];
	}
	return n;
}

wtk_txtree_t* wtk_txtree_new()
{
	wtk_txtree_t *t;

	t=(wtk_txtree_t*)wtk_malloc(sizeof(*t));
	t->heap=wtk_heap_new(4096);
	t->root=wtk_txtree_new_node(t,0,0);
	return t;
}

void wtk_txtree_delete(wtk_txtree_t *t)
{
	wtk_heap_delete(t->heap);
	wtk_free(t);
}

wtk_tnode_t* wtk_tnode_find_child(wtk_tnode_t *p,char *data,int bytes)
{
	wtk_queue_node_t *n;
	wtk_tnode_t *n1;

	for(n=p->child.pop;n;n=n->next)
	{
		n1=data_offset(n,wtk_tnode_t,q_n);
		if(n1->len==bytes && strncmp(data,n1->data,bytes)==0)
		{
			return n1;
			break;
		}
	}
	return 0;
}

int wtk_tnode_is_text_in(wtk_txtree_t *t,char *txt,int bytes)
{
	char *s,*e;
	int n;
	wtk_tnode_t *parent,*node;

	parent=t->root;
	s=txt;e=s+bytes;
	while(s<e)
	{
		n=wtk_utf8_bytes(*s);
		//wtk_tnonde_print(parent);
		node=wtk_tnode_find_child(parent,s,n);
		if(!node)
		{
			return 0;
		}
		parent=node;
		//wtk_debug("[%.*s]\n",n,s);
		s+=n;
	}
	return parent->is_leaf?-1:1;
}

void wtk_txtree_add_text(wtk_txtree_t *t,char *txt,int bytes)
{
	char *s,*e;
	int n;
	wtk_tnode_t *parent,*node;

	parent=t->root;
	s=txt;e=s+bytes;
	while(s<e)
	{
		n=wtk_utf8_bytes(*s);
		//wtk_tnonde_print(parent);
		node=wtk_tnode_find_child(parent,s,n);
		if(!node)
		{
			node=wtk_txtree_new_node(t,s,n);
			wtk_queue2_push(&(parent->child),&(node->q_n));
		}
		parent=node;
		//wtk_debug("[%.*s]\n",n,s);
		s+=n;
		if(s>=e)
		{
			node->is_leaf=1;
		}
	}
}

int wtk_txtree_load(wtk_txtree_t *t,wtk_source_t *src)
{
	wtk_strbuf_t *buf;
	int ret;

	//wtk_debug("load src\n");
	ret=0;
	buf=wtk_strbuf_new(256,1);
	while(1)
	{
		ret=wtk_source_read_string(src,buf);
		//wtk_debug("ret=%d\n",ret);
		if(ret!=0){ret=0;goto end;}
		//wtk_debug("%.*s\n",buf->pos,buf->data);
		wtk_txtree_add_text(t,buf->data,buf->pos);
	}
end:
	wtk_strbuf_delete(buf);
	return ret;
}


int wtk_txtree_load_file(wtk_txtree_t *t,char *fn)
{
	return wtk_source_load_file(t,(wtk_source_load_handler_t)wtk_txtree_load,fn);
}
