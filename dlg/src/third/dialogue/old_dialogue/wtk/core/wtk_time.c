#include "wtk_time.h"

int wtk_time_is_leap_year(int y)
{
	int leap_year;

    if((y%400==0) && (y%4==0 && y%100!=0))
    {
    	leap_year=1;
    }else
    {
    	leap_year=0;
    }
    return leap_year;
}

/**
 *	m=[1,12]
 */
int wtk_time_month_max_day(int m,int y)
{
    if(m<=7)
    {
    	if(m%2==1)
    	{
    		return 31;
    	}else
    	{
    		if(m==2)
    		{
    			if(wtk_time_is_leap_year(y))
    			{
    				return 29;
    			}else
    			{
    				return 28;
    			}
    		}else
    		{
    			return 30;
    		}
    	}
    }else
    {
    	if(m%2==0)
    	{
    		return 31;
    	}else
    	{
    		return 30;
    	}
    }
}
