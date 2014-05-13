/******************************************************
Name:DS18B20driver.c
Description:GPB Port drivers DS18B20
Copyright:Honeydbb
*******************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/uaccess.h>   //copy_to_user函数用到
#include <plat/gpio-cfg.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>

#define DEVICE_NAME "ds18b20"  //device name

#define DQ S3C64XX_GPE(2)

static unsigned char tempH;
static unsigned char tempL;

void Set_DQ_OUTPUT(void)
{
	s3c_gpio_cfgpin(DQ,S3C_GPIO_OUTPUT);
}

void Set_DQ_OUTPUT_VALUE(int value)
{
	gpio_set_value(DQ,value);
}

void Set_DQ_INPUT(void)
{
	s3c_gpio_cfgpin(DQ,S3C_GPIO_INPUT);
}

int Get_DQ_INPUT_VALUE(void)
{
	return !!gpio_get_value(DQ);
}


unsigned char ds18b20_init(void)
{
	//reset ds18b20
	Set_DQ_OUTPUT();
	Set_DQ_OUTPUT_VALUE(1);
	udelay(10);

	Set_DQ_OUTPUT_VALUE(0);
	udelay(700);//min 480us

	Set_DQ_OUTPUT_VALUE(1);
	udelay(30); //15-60us

	Set_DQ_INPUT();
	udelay(100);
	if(Get_DQ_INPUT_VALUE())
	{
//		printk("reset ds18b20 failed!\n");
		return 1;
	}
	return 0;
}

unsigned char ReadByte(void)
{
	unsigned char byte=0;
	unsigned char i;
	for(i=8;i>0;i--)
	{
		Set_DQ_OUTPUT();
		Set_DQ_OUTPUT_VALUE(0);
		udelay(1);
		Set_DQ_OUTPUT_VALUE(1);
		
		
		Set_DQ_INPUT();
		byte>>=1;
		if(Get_DQ_INPUT_VALUE()) byte|=0x80;
		
		udelay(60);
	}
	return byte;
}

void WriteByte(unsigned char byte)
{
	unsigned char i;
	Set_DQ_OUTPUT();
	for(i = 0;i < 8;i++)
	{
		Set_DQ_OUTPUT_VALUE(0);
		udelay(1);
		if(byte&0x01)
		{
			Set_DQ_OUTPUT_VALUE(1);
		}
		udelay(60);
		Set_DQ_OUTPUT_VALUE(1);
		udelay(15);
		byte >>= 1;
	}
	Set_DQ_OUTPUT_VALUE(1);
}
void ds18b20_proc(void)
{
	ds18b20_init();
	udelay(400);
	WriteByte(0xcc);
	WriteByte(0x44);

	mdelay(750);
	
	ds18b20_init();
	udelay(400);
	WriteByte(0xcc);
	WriteByte(0xbe);

	tempL=ReadByte();
	tempH=ReadByte();
}

static ssize_t ds18b20_read(struct file * file,char __user * buf,size_t nbytes,loff_t * ppos)
{
	ds18b20_proc();
	if((tempH*256+tempL)<=2000)
	{
		buf[0]  =	tempL;
		buf[1]  =	tempH;
	}
	return 0;
}

static struct file_operations ds18b20_fops={
	.owner =  THIS_MODULE,
	.read    =  ds18b20_read,
};

static struct miscdevice misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops   = &ds18b20_fops,
};

static int __init drv_init(void)              //注册设备驱动
{
	int ret;
	Set_DQ_OUTPUT();
	ret =  misc_register(&misc);
	printk (DEVICE_NAME"\tinitialized\n");
	return ret;
}

static void __exit drv_exit(void)
{
	misc_deregister(&misc);	
	printk(DEVICE_NAME"\texit\n");
}

module_init(drv_init);
module_exit(drv_exit);

MODULE_ALIAS("DS18B20");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("honeydbb <baobao@sky-light.com.hk>");
