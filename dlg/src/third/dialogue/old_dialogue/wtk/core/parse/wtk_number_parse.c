#include "wtk_number_parse.h"
#include <math.h>

void wtk_number_parse_init(wtk_number_parse_t *p)
{
	p->state=WTK_NUMBER_PARSE_WAIT;
	p->vflag=1;
	p->eflag=1;
	p->integer=0;
	p->decimal=0;
	p->nchar=0;
	p->e=0;
	p->num_decimal=0;
	p->has_decimal=0;
	p->has_e=0;
}

/*
 * 	expr=(e|E)[+|-]*[\d]*
 * 	dec=(0|[1-9]+).[\d]+
 *	[+|-]*(dec|[\d]+)expr*
 *
 *	=>+|-|0|1-9
 */
int wtk_number_parse_can_parse(char c)
{
	if(c=='-' || c=='+')
	{
		return 1;
	}
	if(c>='0' && c<='9')
	{
		return 1;
	}
	return 0;
}

double wtk_number_parse_to_value(wtk_number_parse_t *p)
{
	double f;
	double e;

	f=p->integer;
	if(p->has_decimal)
	{
		f+=p->decimal;
	}
	//wtk_debug("float=%f,e=%d\n",f,p->e);
	if(p->has_e && p->e>0)
	{
		e=pow(10,p->e);
		//wtk_debug("e=%f\n",e);
		if(p->eflag)
		{
			f*=e;
		}else
		{
			f/=e;
		}
	}
	if(!p->vflag)
	{
		f=-f;
	}
	return f;
}

int wtk_number_parse_feed(wtk_number_parse_t *p,char c)
{
	int v;

	switch(p->state)
	{
	case WTK_NUMBER_PARSE_WAIT:
		switch(c)
		{
		case '-':
			p->vflag=0;
			break;
		case '+':
			p->vflag=1;
			break;
		case '0':
			p->state=WTK_NUMBER_PARSE_V_WAIT_DOT;
			break;
		default:
			if(c>='1' && c<='9')
			{
				p->integer=c-'0';
				p->state=WTK_NUMBER_PARSE_V_VALUE;
			}else
			{
				return -1;
			}
			break;
		}
		break;
	case WTK_NUMBER_PARSE_V_VALUE:
		v=c-'0';
		if(v>=0 && v<=9)
		{
			//wtk_debug("v=%d\n",v);
			p->integer=p->integer*10+v;
		}else if(c=='.')
		{
			p->state=WTK_NUMBER_PARSE_V_DECIMAL;
			p->num_decimal=0;
			p->has_decimal=1;
		}else if(c=='e' || c=='E')
		{
			p->state=WTK_NUMBER_PARSE_E_FLAG;
		}else
		{
			return -1;
		}
		break;
	case WTK_NUMBER_PARSE_V_WAIT_DOT:
		if(c=='.')
		{
			p->state=WTK_NUMBER_PARSE_V_DECIMAL;
			p->num_decimal=0;
			p->has_decimal=1;
		}else
		{
			return -1;
		}
		break;
	case WTK_NUMBER_PARSE_V_DECIMAL:
		v=c-'0';
		if(v>=0 && v<=9)
		{
			//wtk_debug("v=%d\n",v);
			++p->num_decimal;
			p->decimal+=v*1.0/pow(10,p->num_decimal);
		}else if(c=='e' || c=='E')
		{
			p->state=WTK_NUMBER_PARSE_E_FLAG;
		}else
		{
			return -1;
		}
		break;
	case WTK_NUMBER_PARSE_E_FLAG:
		switch(c)
		{
		case '-':
			p->eflag=0;
			break;
		case '+':
			p->eflag=1;
			break;
		default:
			return -1;
		}
		p->has_e=1;
		p->state=WTK_NUMBER_PARSE_E_VALUE;
		break;
	case WTK_NUMBER_PARSE_E_VALUE:
		v=c-'0';
		if(v>=0 && v<=9)
		{
			//wtk_debug("v=%d\n",v);
			p->e=p->e*10+v;
		}else
		{
			return -1;
		}
		break;
	default:
		return -1;
		break;
	}
	++p->nchar;
	return 0;
}

double wtk_aton(char *data,int bytes)
{
	wtk_number_parse_t p;
	char *s,*e;
	int ret;
	double f;

	wtk_number_parse_init(&(p));
	s=data;e=s+bytes;
	while(s<e)
	{
		ret=wtk_number_parse_feed(&(p),*s);
		if(ret!=0)
		{
			break;
		}
		++s;
	}
	f=wtk_number_parse_to_value(&(p));
	return f;
}


//----------------- test number ------------------
void test_number_parse()
{
	char *t;
	double f;

	t="-0.5";
	t="15";
	t="+10.5";
	t="-10.578e+10";
	t="-0.578e-5";
	t="10.5556";
	t="0";
	f=wtk_aton(t,strlen(t));
	wtk_debug("f=%f\n",f);
	//exit(0);
}
