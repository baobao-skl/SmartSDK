/******************************************************
Name:leddriver.c
Description:GPB Port drivers leds
Copyright:Honeydbb
*******************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/uaccess.h>   //copy_to_userº¯ÊýÓÃµ½
#include <plat/gpio-cfg.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>


#define DEVICE_NAME "led"  //device name

#define LED_ON 1
#define LED_OFF 0

#define LED_NUMNERS 4

static int led_table [LED_NUMNERS] ={
	S3C64XX_GPN(4),
	S3C64XX_GPN(2),
	S3C64XX_GPN(3),
	S3C64XX_GPN(1)
};


static long leds_ioctl(struct file *filp,unsigned int cmd,unsigned long arg)
{
	if(arg > (LED_NUMNERS -1))
		return -1;
	switch(cmd)
	{
		case LED_ON:
			gpio_set_value(led_table[arg], 0);      //low output,light the led;
			break;
		case LED_OFF:
			gpio_set_value(led_table[arg], 1);       //high output,dark the led;
			break;
		default:
			break;
	}
	return 0;
}

static struct file_operations leds_fops={
	.owner = THIS_MODULE,
	.unlocked_ioctl  = leds_ioctl
};

static struct miscdevice misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &leds_fops,
};

static int __init leds_init(void)
{
	int ret = 0, i =0;
	for(i = 0; i < LED_NUMNERS; i++)
	{
		s3c_gpio_cfgpin(led_table[i], S3C_GPIO_OUTPUT);
		gpio_set_value(led_table[i], 1);
	}
	ret =  misc_register(&misc);
	return ret;
}

static void __exit leds_exit(void)
{
	misc_deregister(&misc);	
}

module_init(leds_init);
module_exit(leds_exit);

MODULE_ALIAS("LED");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("FriendlyARM Inc.");

