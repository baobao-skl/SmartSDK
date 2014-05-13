#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>
#include <linux/ioctl.h>
#include <linux/clk.h>
#include <linux/fs.h>
#include <linux/pagemap.h>
#include <linux/slab.h>
#include <linux/ioctl.h>

#include <mach/hardware.h>
#include <mach/map.h>
#include <mach/gpio.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <asm/signal.h>
#include <plat/regs-timer.h>
#include <mach/regs-clock.h>
#include <plat/gpio-cfg.h>
#include <mach/gpio-bank-l.h>
#include <linux/miscdevice.h>
#define DEVICE_NAME "dht11"

#define PIN_OUT S3C64XX_GPL(11)

static char DHT11_read_byte ( void )
{
	char DHT11_byte ;
	unsigned char i ;
	unsigned char temp ;
	
	DHT11_byte = 0 ;
	for ( i = 0 ; i < 8 ; i ++ )
	{
		temp = 0 ;
		while ( ! (gpio_get_value ( PIN_OUT ) ) )
		{
			temp ++ ;
			if ( temp > 12 )
				return 1 ;
			udelay ( 5 ) ;
		}
		temp = 0 ;
		while ( gpio_get_value ( PIN_OUT ) )
		{
			temp ++ ;
			if ( temp > 20 )
				return 1 ;
			udelay ( 5 ) ;
		}
		if ( temp > 6 )
		{
			DHT11_byte <<= 1 ;
			DHT11_byte |= 1 ;
		} 
		else
		{
			DHT11_byte <<= 1 ;
			DHT11_byte |= 0 ;
		}
	}
	return DHT11_byte ;
}

static ssize_t DHT11_read ( struct file* filp, char __user* buf, size_t count, loff_t* f_pos )
{
	unsigned char DataTemp;
	unsigned char i;
	unsigned char err;
	char tempBuf[5];
//	loff_t pos = *f_pos ;
	
	err = 0 ;

	s3c_gpio_cfgpin ( PIN_OUT , S3C_GPIO_OUTPUT );
	gpio_set_value ( PIN_OUT , 0 );
	msleep ( 18 );
//	mdelay ( 18 );
	gpio_set_value ( PIN_OUT , 1 );
	udelay ( 40 );
	s3c_gpio_cfgpin ( PIN_OUT , S3C_GPIO_INPUT );
	if ( !err )
	{
		DataTemp = 10 ;
		while ( !( gpio_get_value ( PIN_OUT ) ) && DataTemp )
		{
			DataTemp --;
			udelay ( 10 );
		}
		if ( !DataTemp )
		{
			err = 1;
			count = -EFAULT;
		}
	}
	if ( !err )
	{
		DataTemp = 10 ;
		while ( ( gpio_get_value ( PIN_OUT ) ) && DataTemp )
		{
			DataTemp --;
			udelay ( 10 );
		}
		if ( !DataTemp )
		{
			err = 1;
			count = -EFAULT;
		}
	}
	if ( !err )
	{
		for ( i = 0; i < 5; i ++ )
		{
			tempBuf[i] = DHT11_read_byte () ;
		}
		DataTemp = 0 ;
		for ( i = 0; i < 4; i ++ )
		{
			DataTemp += tempBuf[i] ;
		}
		if ( DataTemp != tempBuf[4] )
		{
			count = -EFAULT;
			return 0;
		}
//		if ( count > ( 5 - pos ) )
//		{
//			count = 5 - pos ;
//		}
		if ( count > 5 )
		{
			count = 5 ;
		}

	// 	pos += count;
	//	if ( copy_to_user ( buf , tempBuf + *f_pos , count ) )
		if ( copy_to_user ( buf , tempBuf , count ) )
		{
			count = -EFAULT ;
		}
	//	*f_pos = pos;
	}
	s3c_gpio_cfgpin ( PIN_OUT , S3C_GPIO_OUTPUT );
	gpio_set_value ( PIN_OUT , 1 );
	return count;
}


static struct file_operations dev_fops = {
	.owner = THIS_MODULE,
	.read = DHT11_read,
	};
static struct miscdevice misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &dev_fops,
	};

static int __init DHT11_init_module ( void )
{
	int ret;
	s3c_gpio_cfgpin ( PIN_OUT , S3C_GPIO_OUTPUT );
	gpio_set_value ( PIN_OUT , 1 );
	ret = misc_register(&misc);
	printk (DEVICE_NAME"\tinitialized\n");
	return ret;
}

static void __exit DHT11_exit_module ( void )
{
	misc_deregister(&misc);
}

module_init ( DHT11_init_module );
module_exit ( DHT11_exit_module );

MODULE_LICENSE ( "GPL" ) ;
MODULE_AUTHOR ( "LiQiang Lin (MMC)" ) ;

