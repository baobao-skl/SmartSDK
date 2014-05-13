#ifndef __YANG2YIN_H__
#define __YANG2YIN_H__
#define uint unsigned int
#define uchar unsigned char
typedef struct date_s
{
	uint year;
	uchar month;
	uchar day;
}date_t;
typedef struct Yang2Yin_s
{
	void (*GetYinLiDate)(date_t yanglidate,date_t *yinlidate);
}Yang2Yin_t;
extern Yang2Yin_t Yang2Yin;
#endif