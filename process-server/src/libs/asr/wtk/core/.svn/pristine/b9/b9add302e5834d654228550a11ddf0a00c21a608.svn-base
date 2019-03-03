#include "wtk_splaytree.h"

wtk_splaytree_t* wtk_splaytree_new(wtk_delete_handler_t dispose,void *data)
{
	wtk_splaytree_t* t;

	t=(wtk_splaytree_t*)wtk_malloc(sizeof(*t));
	return t;
}

int wtk_splaytree_init(wtk_splaytree_t* t)//,dispose_handler dispose,void *data)
{
	t->root=0;
	return 0;
}

wtk_spnode_t* treenode_splay(wtk_spnode_t* root,wtk_cmp_handler_t cmp,void *value,int *cmp_ret)
{
	wtk_spnode_t n,*l,*r,*tmp,*t;
	int ret,found_value;

	found_value=-1;
	t=root;
	n.left=n.right=0;
	l=r=&n;
	ret=cmp(t->value,value);
	for(;;)
	{
		if(ret<0)
		{
			if(!t->left){break;}
			ret=cmp(t->left->value,value);
			if(ret<0)
			{
				tmp=t->left;t->left=tmp->right;tmp->right=t;t=tmp;
			}
			if(!t->left)
			{
				break;
			}else
			{
				ret=cmp(t->left->value,value);
			}
			r->left=t;r=t;t=t->left;
		}else if(ret>0)
		{
			if(!t->right){break;}
			ret=cmp(t->right->value,value);
			if(ret>0)
			{
				tmp=t->right;t->right=tmp->left;tmp->left=t;t=tmp;
			}
			if(!t->right)
			{
				break;
			}else
			{
				ret=cmp(t->right->value,value);
			}
			l->right=t;l=t;t=t->right;
		}else
		{
			found_value=0;
			break;
		}
	}
	l->right=t->left;r->left=t->right;
	t->left=n.right;t->right=n.left;
	if(cmp_ret)
	{
		*cmp_ret=found_value;
	}
	return t;
}

wtk_spnode_t* wtk_splaytree_splay(wtk_splaytree_t *t,wtk_cmp_handler_t cmp,void *value,int *cmp_value)
{
	wtk_spnode_t* n;

	if(t->root)
	{
		n=treenode_splay(t->root,cmp,value,cmp_value);
		t->root=n;
	}else
	{
		n=0;
		if(cmp_value){*cmp_value=-1;}
	}
	return n;
}

wtk_spnode_t* wtk_splaytree_find_node(wtk_splaytree_t *t,wtk_cmp_handler_t cmp,void *value)
{
	wtk_spnode_t* n;
	int ret;

	n=wtk_splaytree_splay(t,cmp,value,&ret);
	return ret==0 ? n:0;
}

void* wtk_splaytree_find(wtk_splaytree_t *t,wtk_cmp_handler_t cmp,void *value)
{
	wtk_spnode_t* node;

	node=wtk_splaytree_find_node(t,cmp,value);
	return node ? node->value : 0;
}

int wtk_splaytree_insert(wtk_splaytree_t *tree, wtk_cmp_handler_t cmp,void *cmp_param,wtk_spnode_t *node)
{
	wtk_spnode_t *root;
	int ret;

	ret=-1;
	root=wtk_splaytree_splay(tree,cmp,cmp_param,&ret);
	if(root)
	{
		if(ret==0)
		{
			wtk_debug("%p already exist.\n",node->value);
			ret=-1;
			goto end;
		}
		if(ret<0)
		{
			node->left=root;
			node->right=0;
		}else
		{
			node->right=root;
			node->left=0;
		}
		/*
		if(ret<0)
		{
			node->left=root->left;
			root->left=0;
			node->right=root;
		}else
		{
			node->right=root->right;
			root->right=0;
			node->left=root;
		}
		*/
	}else
	{
		node->left=node->right=0;
	}
	tree->root=node;
	ret=0;
end:
	return ret;
}

wtk_spnode_t* wtk_splaytree_remove(wtk_splaytree_t* tree,wtk_cmp_handler_t cmp,void *value)
{
	wtk_spnode_t* node;
	wtk_spnode_t* root;

	node=wtk_splaytree_find_node(tree,cmp,value);
	if(!node){goto end;}
	if(!node->left)
	{
		root=node->right;
	}else
	{
		root=treenode_splay(node->left,cmp,value,0);
		root->right=node->right;
	}
	tree->root=root;
end:
	return node;
}


int wtk_spnode_walk(wtk_spnode_t *t,wtk_walk_handler_t walk,void *user_data)
{
	int ret;

	if(!t){ret=0;goto end;}
	if(t->left)
	{
		printf("left: \n");
		ret=wtk_spnode_walk(t->left,walk,user_data);
		if(ret!=0){goto end;}
	}
	ret=walk(user_data,t->value);
	if(ret!=0){goto end;}
	if(t->right)
	{
		printf("right: \n");
		ret=wtk_spnode_walk(t->right,walk,user_data);
	}
end:
	return ret;
}

int wtk_splaytree_walk(wtk_splaytree_t* t,wtk_walk_handler_t walk,void *user_data)
{
	printf("############ %s ##############\n",__FUNCTION__);
	return wtk_spnode_walk(t->root,walk,user_data);
}

