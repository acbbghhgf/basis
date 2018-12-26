#include "wtk_calendar.h"
#include <stdio.h>
unsigned int wtk_lunar_info[199] = {
	0x04AE53,0x0A5748,0x5526BD,0x0D2650,0x0D9544,0x46AAB9,0x056A4D,0x09AD42,0x24AEB6,0x04AE4A,/*1901-1910*/
	0x6A4DBE,0x0A4D52,0x0D2546,0x5D52BA,0x0B544E,0x0D6A43,0x296D37,0x095B4B,0x749BC1,0x049754,/*1911-1920*/
	0x0A4B48,0x5B25BC,0x06A550,0x06D445,0x4ADAB8,0x02B64D,0x095742,0x2497B7,0x04974A,0x664B3E,/*1921-1930*/
	0x0D4A51,0x0EA546,0x56D4BA,0x05AD4E,0x02B644,0x393738,0x092E4B,0x7C96BF,0x0C9553,0x0D4A48,/*1931-1940*/
	0x6DA53B,0x0B554F,0x056A45,0x4AADB9,0x025D4D,0x092D42,0x2C95B6,0x0A954A,0x7B4ABD,0x06CA51,/*1941-1950*/
	0x0B5546,0x555ABB,0x04DA4E,0x0A5B43,0x352BB8,0x052B4C,0x8A953F,0x0E9552,0x06AA48,0x6AD53C,/*1951-1960*/
	0x0AB54F,0x04B645,0x4A5739,0x0A574D,0x052642,0x3E9335,0x0D9549,0x75AABE,0x056A51,0x096D46,/*1961-1970*/
	0x54AEBB,0x04AD4F,0x0A4D43,0x4D26B7,0x0D254B,0x8D52BF,0x0B5452,0x0B6A47,0x696D3C,0x095B50,/*1971-1980*/
	0x049B45,0x4A4BB9,0x0A4B4D,0xAB25C2,0x06A554,0x06D449,0x6ADA3D,0x0AB651,0x093746,0x5497BB,/*1981-1990*/
	0x04974F,0x064B44,0x36A537,0x0EA54A,0x86B2BF,0x05AC53,0x0AB647,0x5936BC,0x092E50,0x0C9645,/*1991-2000*/
	0x4D4AB8,0x0D4A4C,0x0DA541,0x25AAB6,0x056A49,0x7AADBD,0x025D52,0x092D47,0x5C95BA,0x0A954E,/*2001-2010*/
	0x0B4A43,0x4B5537,0x0AD54A,0x955ABF,0x04BA53,0x0A5B48,0x652BBC,0x052B50,0x0A9345,0x474AB9,/*2011-2020*/
	0x06AA4C,0x0AD541,0x24DAB6,0x04B64A,0x69573D,0x0A4E51,0x0D2646,0x5E933A,0x0D534D,0x05AA43,/*2021-2030*/
	0x36B537,0x096D4B,0xB4AEBF,0x04AD53,0x0A4D48,0x6D25BC,0x0D254F,0x0D5244,0x5DAA38,0x0B5A4C,/*2031-2040*/
	0x056D41,0x24ADB6,0x049B4A,0x7A4BBE,0x0A4B51,0x0AA546,0x5B52BA,0x06D24E,0x0ADA42,0x355B37,/*2041-2050*/
	0x09374B,0x8497C1,0x049753,0x064B48,0x66A53C,0x0EA54F,0x06B244,0x4AB638,0x0AAE4C,0x092E42,/*2051-2060*/
	0x3C9735,0x0C9649,0x7D4ABD,0x0D4A51,0x0DA545,0x55AABA,0x056A4E,0x0A6D43,0x452EB7,0x052D4B,/*2061-2070*/
	0x8A95BF,0x0A9553,0x0B4A47,0x6B553B,0x0AD54F,0x055A45,0x4A5D38,0x0A5B4C,0x052B42,0x3A93B6,/*2071-2080*/
	0x069349,0x7729BD,0x06AA51,0x0AD546,0x54DABA,0x04B64E,0x0A5743,0x452738,0x0D264A,0x8E933E,/*2081-2090*/
	0x0D5252,0x0DAA47,0x66B53B,0x056D4F,0x04AE45,0x4A4EB9,0x0A4D4C,0x0D1541,0x2D92B5          /*2091-2099*/
};

#define WTK_CALENDAR_MIN_YEAR 1901
#define WTK_CALENDAR_MAX_YEAR 2099

int wtk_calendar_is_leap_year(int year)
{
	return (((year%400)==0) || ((year%4)==0 && (year%100)!=0) );
}

/**
 *	根据年份判断这一年的闰月
 *	@return >0，存在闰月，返回月份     0，不存在闰月
 */
static int wtk_canlendar_get_leap_month(int year)
{
	return ((wtk_lunar_info[year-WTK_CALENDAR_MIN_YEAR] & 0xf00000) >>20);
}

/**
 *	根据农历年份和月份得到该月有多少天
 *	@return -1 出错     >0 得到相应的天数
 */
static int wtk_calendar_lunar_month_days(int year,int month)
{
		return ( (wtk_lunar_info[year-WTK_CALENDAR_MIN_YEAR] & (0x80000>>(month-1)))? 30: 29);
}

/**
 * 根据年份得到正月初一对应的公历年月
 * @return 0 成功  -1 出错
 */
static int wtk_calendar_lunar_spring_date(int year,int *solar_m,int *solar_d)
{
	int ret;

	*solar_m=(wtk_lunar_info[year-WTK_CALENDAR_MIN_YEAR] & 0x0060) >> 5;
	*solar_d=wtk_lunar_info[year-WTK_CALENDAR_MIN_YEAR]&0x1f;
	ret=0;
	return ret;
}

/**
 *	检查农历日期是否合法,本模块只支持农历日期是从1901年到2099年
 */
static int wtk_calendar_lunar_datecheck(wtk_lunar_t *lunar)
{
	int ret;
	int month,xMonth;
	int day;

	if(lunar->year < WTK_CALENDAR_MIN_YEAR || lunar->year > WTK_CALENDAR_MAX_YEAR)
	{
		ret=-1;
		goto end;
	}
	if(lunar->month < 1 || lunar->month > 13)
	{
		ret=-1;
		goto end;
	}
	month=lunar->month;
	xMonth=wtk_canlendar_get_leap_month(lunar->year);
	if(xMonth != 0)
	{
		if(month > xMonth || (month==xMonth || lunar->is_leap==1))
		{
			month=month+1;
		}
	}
	day=wtk_calendar_lunar_month_days(lunar->year,month);
	if(lunar->day > day || lunar->day < 1)
	{
		ret=-1;
		goto end;
	}
	ret=0;
end:
	return ret;
}

/**
 * 检查日期是否合法,本模块只支持日期是从1901年到2099年
 */
static int wtk_calendar_solar_datecheck(wtk_solar_t *solar)
{
	int ret=0;
	int months[13]={0,31,28,31,30,31,30,31,31,30,31,30,31};

	if(solar->year<WTK_CALENDAR_MIN_YEAR || solar->year>WTK_CALENDAR_MAX_YEAR)
	{
		ret=-1;goto end;
	}
	if(solar->month<1 || solar->month>12)
	{
		ret=-1;goto end;
	}
	if(solar->day<1 || solar->day>months[solar->month])
	{
		if(solar->month==2 && wtk_calendar_is_leap_year(solar->year) && solar->day==29)
		{
			ret=0;
		}
		else{
			ret=-1;goto end;
		}
	}
end:
	return ret;
}

static int wtk_solar_month_common_total[13]={0,31,59,90,120,151,181,212,243,273,304,334,365};
static int wtk_solar_month_leap_total[13]={0,31,60,91,121,152,182,213,244,274,305,335,366};


int wtk_calendar_to_solar(wtk_lunar_t *lunar,wtk_solar_t *solar)
{
	int ret;
	int byNow=0;
	int spring_m,spring_d;
	int month,xMonth;
	int i;
	int is_year_leap;
	int *month_total;

	if((!solar) || (!lunar))
	{
		ret=-1;goto end;
	}
	if((ret=wtk_calendar_lunar_datecheck(lunar)) == -1)
	{
		ret=-1;goto end;
	}
	wtk_calendar_lunar_spring_date(lunar->year,&spring_m,&spring_d);
	byNow=spring_d - 1;
	if(spring_m == 2)
	{
		byNow += 31;
	}
	month=lunar->month;
	xMonth=wtk_canlendar_get_leap_month(lunar->year);
	if(xMonth != 0)
	{
		if(month > xMonth || (month==xMonth && lunar->is_leap==1))
		{
			month=month+1;
		}
	}
	for(i=1;i<month;i++)
	{
		byNow += wtk_calendar_lunar_month_days(lunar->year,i);
		//wtk_debug("mm = %d\tdays=%d\n",i,wtk_calendar_lunar_month_days(lunar->year,i));
	}
	byNow+=lunar->day;
	solar->year=lunar->year;
	if(byNow > 366 || (lunar->year%4!=0 && byNow == 365))
	{
		solar->year += 1;
		if(lunar->year%4==0)
		{
			byNow -= 366;
		}
		else{
			byNow -= 365;
		}
	}
	is_year_leap=wtk_calendar_is_leap_year(lunar->year);
	if(is_year_leap)
	{
		month_total=wtk_solar_month_leap_total;
	}
	else{
		month_total=wtk_solar_month_common_total;
	}
	for(i=1;i<=12;i++)
	{
		if(month_total[i]>=byNow)
		{
			solar->month=i;
			break;
		}
	}
	solar->day=byNow - month_total[solar->month-1];
	ret=0;
end:
	return ret;
}


int wtk_calendar_to_lunar(wtk_solar_t *solar,wtk_lunar_t *lunar)
{
	int ret;
	int spring_m,spring_d;
	int bySpring,bySolar;
	int dayspermonth;
	int *month_total;
	int i;
	int leap_m;
	int xMonth;

	if((!solar) || (!lunar))
	{
		ret=-1;goto end;
	}
	if(wtk_calendar_solar_datecheck(solar)==-1)
	{
		ret=-1;goto end;
	}
	wtk_calendar_lunar_spring_date(solar->year,&spring_m,&spring_d);
	//wtk_debug("[%d] spring_month=%d\tspring_day=%d\n",solar->year,spring_m,spring_d);
	if(spring_m==1)
	{
		bySpring=spring_d-1;
	}
	else{
		bySpring=spring_d+31-1;
	}
	if(wtk_calendar_is_leap_year(solar->year))
	{
		month_total=wtk_solar_month_leap_total;
	}
	else{
		month_total=wtk_solar_month_common_total;
	}
	bySolar=month_total[solar->month-1]+solar->day-1;
	lunar->is_leap=0;
	if(bySolar>=bySpring)
	{
		//wtk_debug("Spring later\n");
		bySolar-=bySpring;
		lunar->year=solar->year;
		lunar->month=1;
		for(i=1;i<=13;i++)
		{
			dayspermonth=wtk_calendar_lunar_month_days(lunar->year,lunar->month);
			if(bySolar>=dayspermonth)
			{
				bySolar-=dayspermonth;
				lunar->month++;
			}
			else
			{
				break;
			}
		}
		lunar->day=bySolar+1;
		leap_m=wtk_canlendar_get_leap_month(lunar->year);
		//wtk_debug("[%d] leap_month=%d\n",lunar->year,leap_m);
		if(leap_m)
		{
			if(lunar->month==leap_m)
			{
				lunar->is_leap=0;
			}
			else if((lunar->month-leap_m) == 1)
			{
				lunar->month--;
				lunar->is_leap=1;
			}
			else if(lunar->month > leap_m)
			{
				lunar->month--;
			}
		}
	}
	else{
		//wtk_debug("Spring before\n");
		lunar->year=solar->year-1;
		bySpring-=bySolar;
		leap_m=wtk_canlendar_get_leap_month(lunar->year);
		//wtk_debug("[%d] leap month = %d\n",lunar->year,leap_m);
		xMonth=12;
		if(leap_m)
		{
			xMonth++;
		}
		for(i=xMonth;i>10;i--)
		{
			dayspermonth=wtk_calendar_lunar_month_days(lunar->year,xMonth);
			if(bySpring>dayspermonth)
			{
				bySpring=bySpring-dayspermonth;
				xMonth--;
			}
			else
			{
				break;
			}
		}
		lunar->day=dayspermonth-bySpring+1;
		if(leap_m)
		{
			if(xMonth==leap_m)
			{
				lunar->month=xMonth;
				lunar->is_leap=0;
			}
			else if((xMonth-leap_m)==1)
			{
				lunar->month=xMonth-1;
				lunar->is_leap=1;
			}
			else if(xMonth>leap_m)
			{
				lunar->month=xMonth-1;
			}
		}
	}
	ret=0;
end:
	return ret;
}

