#include "wtk_rbtree.h"

wtk_rbtree_t* wtk_rbtree_new()
{
	wtk_rbtree_t* t;

	t=(wtk_rbtree_t*)wtk_calloc(1,sizeof(wtk_rbtree_t));
	t->len=0;
	return t;
}

int wtk_rbtree_delete(wtk_rbtree_t* t)
{
	wtk_free(t);
	return 0;
}

void wtk_rbtree_init(wtk_rbtree_t *t)
{
	t->root=0;
	t->len=0;
}

void wtk_rbnode_insert(wtk_rbnode_t *r,wtk_rbnode_t *n)
{
	wtk_rbnode_t **p;

	for(;;)
	{
		//found node with empty child.
		p= (n->key < r->key) ? &(r->left) : &(r->right);
		if(!*p){break;}
		r=*p;
	}
	*p=n;
	n->parent=r;
	n->left=n->right=0;
	wtk_rbnode_red(n);
}

wtk_rbnode_t* wtk_rbtree_find(wtk_rbtree_t *t,wtk_rbnode_key_t key)
{
	wtk_rbnode_t *p,*n;
	float v;

	p=t->root;n=0;
	while(p)
	{
		v=key-p->key;
		if(v<0)
		{
			p=p->left;
		}else if(v>0)
		{
			p=p->right;
		}else
		{
			n=p;
			break;
		}
	}
	return n;
}

wtk_rbnode_t* wtk_rbtree_find_like(wtk_rbtree_t *t,wtk_rbnode_key_t key)
{
	wtk_rbnode_t *p,*n,*prev;
	float v;

	p=t->root;n=0;
	prev=p;
	while(p)
	{
		prev=p;
		v=key-p->key;
		if(v<0)
		{
			p=p->left;
		}else if(v>0)
		{
			p=p->right;
		}else
		{
			n=p;
			break;
		}
	}
	if(!n)
	{
		n=p?p:prev;
	}
	return n;
}

//i do not use macro or offset to make this code be shorter for want it
//be clear.
void wtk_rbtree_right_rotate(wtk_rbtree_t *t,wtk_rbnode_t *n)
{
	wtk_rbnode_t *l;

	l=n->left;
	n->left=l->right;
	if(l->right){l->right->parent=n;}
	l->parent=n->parent;
	if(n->parent)
	{
		if(n == n->parent->left)
		{
			n->parent->left=l;
		}else
		{
			n->parent->right=l;
		}
	}else
	{
		t->root=l;
	}
	l->right=n;
	n->parent=l;
}

void wtk_rbtree_left_rotate(wtk_rbtree_t *t,wtk_rbnode_t *n)
{
	wtk_rbnode_t *r;

	r=n->right;
	if(!r){return;}
	n->right=r->left;
	if(r->left){r->left->parent=n;}
	r->parent=n->parent;
	if(n->parent)
	{
		if(n == n->parent->left)
		{
			n->parent->left=r;
		}else
		{
			n->parent->right=r;
		}
	}else
	{
		t->root=r;
	}
	r->left=n;
	n->parent=r;
}

void wtk_rbtree_insert_fixup(wtk_rbtree_t *t,wtk_rbnode_t *n)
{
	wtk_rbnode_t *p,*pp,*rpp,*lpp;

	for(p=n->parent;p && wtk_rbnode_is_red(p);p=n->parent)
	{
		pp=p->parent;
		if(!pp){break;}
		//wtk_debug("pp=%p l=%p r=%p p=%p r=%p:%p\n",pp,pp->left,pp->right,p,t->root,t->root->parent);
		if(p == pp->left)
		{
			rpp=pp->right;
			if(wtk_rbnode_is_red(rpp))
			{
				wtk_rbnode_black(p);
				wtk_rbnode_black(rpp);
				wtk_rbnode_red(pp);
				n=pp;
				continue;
			}else
			{
				if(n == p->right)
				{
					wtk_rbtree_left_rotate(t,p);
					p=n;
				}
				wtk_rbnode_black(p);
				wtk_rbnode_red(pp);
				wtk_rbtree_right_rotate(t,pp);
				break;
			}
		}else
		{
			lpp=pp->left;
			if(wtk_rbnode_is_red(lpp))
			{
				wtk_rbnode_black(p);
				wtk_rbnode_black(lpp);
				wtk_rbnode_red(pp);
				n=pp;
				continue;
			}else
			{
				if( n == p->left)
				{
					wtk_rbtree_right_rotate(t,p);
					p=n;
				}
				wtk_rbnode_black(p);
				wtk_rbnode_red(pp);
				wtk_rbtree_left_rotate(t,pp);
				break;
			}
		}
	}
	wtk_rbnode_black(t->root);
}

void wtk_rbtree_insert(wtk_rbtree_t *t,wtk_rbnode_t *n)
{
	//wtk_debug("node=%p\n",n);
	++t->len;
	n->left=n->right=n->parent=0;
	if(!t->root)
	{
		t->root=n;
		wtk_rbnode_black(n);
		return;
	}
	wtk_rbnode_insert(t->root,n);
	wtk_rbtree_insert_fixup(t,n);
	return;
}

void wtk_rbtree_delete_fixup(wtk_rbtree_t *t,wtk_rbnode_t *n,wtk_rbnode_t *p)
{
	wtk_rbnode_t *rp,*lp;

	for(;n!=t->root && wtk_rbnode_is_black(n);p=n->parent)
	{
		if(n == p->left)
		{
			rp=p->right;
			if(wtk_rbnode_is_red(rp))
			{
				wtk_rbnode_black(rp);
				wtk_rbnode_red(p);
				wtk_rbtree_left_rotate(t,p);
				rp=p->right;
			}
			if(!rp || (wtk_rbnode_is_black(rp->left) && wtk_rbnode_is_black(rp->right)))
			{
				if(rp){wtk_rbnode_red(rp);}
				n=p;
				continue;
			}
			if(wtk_rbnode_is_black(rp->right))
			{
				wtk_rbnode_black(rp->left);
				wtk_rbnode_red(rp);
				wtk_rbtree_right_rotate(t,rp);
				rp=p->right;
			}
			rp->color=p->color;
			wtk_rbnode_black(p);
			wtk_rbnode_black(rp->right);
			wtk_rbtree_left_rotate(t,p);
			n=t->root;
			break;
		}else
		{
			lp = p->left;
			if(wtk_rbnode_is_red(lp))
			{
				if(lp){wtk_rbnode_black(lp);}
				wtk_rbnode_red(p);
				wtk_rbtree_right_rotate(t,p);
				lp=p->left;
			}
			if(!lp || (wtk_rbnode_is_black(lp->left) && wtk_rbnode_is_black(lp->right)))
			{
				wtk_rbnode_red(lp);
				n=p;
				continue;
			}
			if(wtk_rbnode_is_black(lp->left))
			{
				wtk_rbnode_black(lp->right);
				wtk_rbnode_red(lp);
				wtk_rbtree_left_rotate(t,lp);
				lp=p->left;
			}
			lp->color=p->color;
			wtk_rbnode_black(p);
			wtk_rbnode_black(lp->left);
			wtk_rbtree_right_rotate(t,p);
			n=t->root;
			break;
		}
	}
	wtk_rbnode_black(n);
}

void wtk_rbtree_remove(wtk_rbtree_t *t,wtk_rbnode_t *n)
{
	wtk_rbnode_t *subst,*tmp,*p;
	int red;

	//wtk_debug("root=%p n=%p l=%p r=%p\n",t->root,n,n->left,n->right);
	--t->len;
	p=0;
	if(!n->left)
	{
		tmp=n->right;
		subst=n;
	}else if(!n->right)
	{
		tmp=n->left;
		subst=n;
	}else
	{
		subst=(wtk_rbnode_t*)wtk_treenode_min((wtk_treenode_t*)n->right);
		if(subst->left)
		{
			tmp=subst->left;
		}else
		{
			tmp=subst->right;
		}
	}
	if(subst == t->root)
	{
		t->root=tmp;
		wtk_rbnode_black(tmp);
		n->left=n->parent=n->right=0;
		if(tmp)
		{
			tmp->parent=NULL;
		}
		return;
	}
	red=wtk_rbnode_is_red(subst);
	if(subst == subst->parent->left)
	{
		subst->parent->left=tmp;
	}else
	{
		subst->parent->right=tmp;
	}
	if(subst == n)
	{
		if(tmp)
		{
			tmp->parent=subst->parent;
		}else
		{
			p=subst->parent;
		}
	}else
	{
		if(subst->parent == n)
		{
			if(tmp)
			{
				tmp->parent=subst;
			}else
			{
				p=subst;
			}
		}else
		{
			if(tmp)
			{
				tmp->parent=subst->parent;
			}else
			{
				p=subst->parent;
			}
		}
		subst->left=n->left;
		subst->right=n->right;
		subst->parent=n->parent;
		subst->color=n->color;
		if(n==t->root)
		{
			t->root=subst;
			if(subst && subst->parent)
			{
				subst->parent=NULL;
			}
		}else
		{
			if(n==n->parent->left)
			{
				n->parent->left=subst;
			}else
			{
				n->parent->right=subst;
			}
		}
		if(subst->left){subst->left->parent=subst;}
		if(subst->right){subst->right->parent=subst;}
	}
	n->left=n->parent=n->right=0;
	//printf("subst=%d,tmp=%p,p=%d\n",subst->key,tmp,p->key);
	//wtk_rbtree_print(t);
	//wtk_debug("root=%p\n",t->root);
	if(red){return;}
	if(tmp)
	{
		wtk_rbtree_delete_fixup(t,tmp,tmp->parent);
	}else
	{
		wtk_rbtree_delete_fixup(t,tmp,p);
	}
	return;
}

int wtk_rbnode_valid(void* data,wtk_rbnode_t *n)
{
	int ret;

	if(!wtk_rbnode_is_red(n)){ret=0;goto end;}
	ret=-1;
	if(wtk_rbnode_is_red(n->left))
	{
		printf("left node is red.\n");
		goto end;
	}
	if(wtk_rbnode_is_red(n->right))
	{
		printf("right node is red.\n");
		goto end;
	}
	ret=0;
end:
	return ret;
}

int wtk_rbnode_black_depth(wtk_rbnode_t *n,int n_depth)
{
	int d_left,d_right;

	if(!n){goto end;}
	if(wtk_rbnode_is_black(n))
	{
		n_depth += 1;
	}
	d_left=wtk_rbnode_black_depth(n->left,0);
	if(d_left<0){n_depth=-1;goto end;}
	d_right=wtk_rbnode_black_depth(n->right,0);
	if(d_right<0){n_depth=-1;goto end;}
	if(d_left != d_right)
	{
		printf("%f(left=%d,right=%d) depth not equal.\n",n->key,d_left,d_right);
		n_depth=-1;
		goto end;
	}
	n_depth += d_left;
end:
	return n_depth;
}

int wtk_rbtree_is_valid(wtk_rbtree_t *t)
{
	int ret;
	int len;

	if(t->root && t->root->parent)
	{
		wtk_debug("found parent\n");
		ret=0;
		goto end;
	}
	len=wtk_treenode_len((wtk_treenode_t*)t->root);
	if(len!=t->len)
	{
		//wtk_debug("len[%d/%d] not eq\n",len,t->len);
		ret=0;
		goto end;
	}
	if(!t->root)
	{
		if(t->len==0)
		{
			ret=1;
		}else
		{
			ret=0;
		}
		goto end;
	}
	ret=0;
	if(wtk_rbnode_is_red(t->root)){goto end;}
	ret=wtk_treenode_traverse((wtk_treenode_t*)t->root,(WtkTreeNodeTraverseFunc)wtk_rbnode_valid,0);
	if(ret!=0)
	{
		printf("red-black tree color is not right.\n");
		goto end;
	}
	ret=wtk_rbnode_black_depth(t->root,0);
	ret=ret > 0 ? 1 : 0;
end:
	if(ret!=1)
	{
		exit(0);
	}
	return ret;
}

void wtk_rbnode_print(wtk_rbnode_t *n)
{
	printf("%f(%s:%p)\n",n->key,n->color?"r":"b",(n));
}

void wtk_rbtree_print(wtk_rbtree_t *t)
{
	printf("##############################################\n");
	wtk_treenode_print((wtk_treenode_t*)t->root,0,(WtkTreeNodePrintFunc)wtk_rbnode_print);
	printf("##############################################\n");
}

int wtk_rbtree_depth(wtk_rbtree_t *t)
{
	return wtk_treenode_depth((wtk_treenode_t*)t->root,0);
}


