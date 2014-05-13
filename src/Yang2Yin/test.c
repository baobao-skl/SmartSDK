#include <stdio.h>
#include "Yang2Yin.h"

int main(int argc,char **argv)
{
	date_t t_now = {2013,7,7};
	date_t y_now;
	Yang2Yin.GetYinLiDate(t_now,&y_now);
	printf("*****welcome to Yang2Yin Test****\n");
	printf("*********Yanli:%d-%d-%d**********\n",t_now.year,t_now.month,t_now.day);
	printf("*********Yinli:%d-%d-%d**********\n",y_now.year,y_now.month,y_now.day);
}
