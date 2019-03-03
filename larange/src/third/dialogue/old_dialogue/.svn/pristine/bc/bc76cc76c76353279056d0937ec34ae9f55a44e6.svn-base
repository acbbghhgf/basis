#include "wtk_treenode.h"

wtk_treenode_t* wtk_treenode_min(wtk_treenode_t *t)
{
	while(t->left)
	{
		t=t->left;
	}
	return t;
}

wtk_treenode_t* wtk_treenode_max(wtk_treenode_t *t)
{
	while(t->right)
	{
		t=t->right;
	}
	return t;
}

wtk_treenode_t* wtk_treenode_successor(wtk_treenode_t *t)
{
	wtk_treenode_t *y;

	if(t->right)
	{
		return wtk_treenode_min(t->right);
	}
	y=t->parent;
	while(y && (t==y->right))
	{
		t=y;y=t->parent;
	}
	return y;
}

int wtk_treenode_traverse(wtk_treenode_t *t,WtkTreeNodeTraverseFunc traverse,void *data)
{
	int ret;

	if(!t){ret=0;goto end;}
	if(t->left)
	{
		ret=wtk_treenode_traverse(t->left,traverse,data);
		if(ret!=0){goto end;}
	}
	ret=traverse(data,t);
	if(ret!=0){goto end;}
	if(t->right)
	{
		ret=wtk_treenode_traverse(t->right,traverse,data);
	}
end:
	return ret;
}

void wtk_treenode_print(wtk_treenode_t *n,int depth,WtkTreeNodePrintFunc print)
{
	#define PAD "   "
	int i;

	if(!n){return;}
	if(n->left)
	{
		wtk_treenode_print(n->left,depth+1,print);
	}
	for(i=0;i<depth;++i)
	{
		printf(PAD);
	}
	print(n);
	if(n->right)
	{
		wtk_treenode_print(n->right,depth+1,print);
	}
}

int wtk_treenode_depth(wtk_treenode_t* n,int depth)
{
	int m,t;

	if(!n){return depth;}
	m=depth+1;
	if(n->left)
	{
		t=wtk_treenode_depth(n->left,depth+1);
		if(t>m){m=t;}
	}
	if(n->right)
	{
		t=wtk_treenode_depth(n->right,depth+1);
		if(t>m){m=t;}
	}
	return m;
}

int wtk_treenode_len(wtk_treenode_t *n)
{
	int cnt=0;

	if(!n){return cnt;}
	if(n->left)
	{
		cnt+=wtk_treenode_len(n->left);
	}
	cnt+=1;
	if(n->right)
	{
		cnt+=wtk_treenode_len(n->right);
	}
	return cnt;
}


