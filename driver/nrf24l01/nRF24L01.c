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
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <linux/interrupt.h>

typedef unsigned int uint16 ;
typedef unsigned char uint8 ;

//和引脚相关的宏定义
#define CE    S3C64XX_GPE(1)
#define CSN   S3C64XX_GPE(3)
#define SCK   S3C64XX_GPM(0)
#define MOSI  S3C64XX_GPM(2)
#define MISO  S3C64XX_GPM(4)

#define DEVICE_NAME "nrf24l01"

//NRF24L01端口定义
#define CE_OUT s3c_gpio_cfgpin(CE, S3C_GPIO_OUTPUT) //数据线设置为输出
#define CE_UP s3c_gpio_setpull(CE, S3C_GPIO_PULL_UP) //打开上拉电阻
#define CE_L gpio_set_value(CE, 0) //拉低数据线电平
#define CE_H gpio_set_value(CE, 1) //拉高数据线电平

#define SCK_OUT s3c_gpio_cfgpin(SCK, S3C_GPIO_OUTPUT) //数据线设置为输出
#define SCK_UP s3c_gpio_setpull(SCK, S3C_GPIO_PULL_UP) //打开上拉电阻
#define SCK_L gpio_set_value(SCK, 0) //拉低数据线电平
#define SCK_H gpio_set_value(SCK, 1) //拉高数据线电平

#define MISO_IN s3c_gpio_cfgpin(MISO, S3C_GPIO_INPUT) //数据线设置为输ru 
#define MISO_UP s3c_gpio_setpull(MISO, S3C_GPIO_PULL_UP) //打开上拉电阻
#define MISO_STU (!!gpio_get_value(MISO)) //数据状态

#define MOSI_OUT s3c_gpio_cfgpin(MOSI, S3C_GPIO_OUTPUT) //数据线设置为输出
#define MOSI_UP s3c_gpio_setpull(MOSI, S3C_GPIO_PULL_UP) //打开上拉电阻
#define MOSI_L gpio_set_value(MOSI, 0) //拉低数据线电平
#define MOSI_H gpio_set_value(MOSI, 1) //拉高数据线电平

#define CSN_OUT s3c_gpio_cfgpin(CSN, S3C_GPIO_OUTPUT) //数据线设置为输出
#define CSN_UP s3c_gpio_setpull(CSN, S3C_GPIO_PULL_UP) //打开上拉电阻
#define CSN_L gpio_set_value(CSN, 0) //拉低数据线电平
#define CSN_H gpio_set_value(CSN, 1) //拉高数据线电平

//NRF24L01
#define TX_ADR_WIDTH 5 // 5 uint8s TX address width
#define RX_ADR_WIDTH 5 // 5 uint8s RX address width
uint8 TX_ADDRESS[TX_ADR_WIDTH]= {0x05,0x06,0x07,0x08,0x09}; //本地地址
uint8 RX_ADDRESS[RX_ADR_WIDTH]= {0x05,0x06,0x07,0x08,0x09}; //接收地址

//NRF24L01寄存器指令
#define READ_REG 0x00 // 读寄存器指令
#define WRITE_REG 0x20 // 写寄存器指令
#define RD_RX_PLOAD 0x61 // 读取接收数据指令
#define WR_TX_PLOAD 0xA0 // 写待发数据指令
#define FLUSH_TX 0xE1 // 冲洗发送 FIFO指令
#define FLUSH_RX 0xE2 // 冲洗接收 FIFO指令
#define REUSE_TX_PL 0xE3 // 定义重复装载数据指令
#define NOP 0xFF // 保留

//SPI(nRF24L01)寄存器地址
#define CONFIG 0x00 // 配置收发状态，CRC校验模式以及收发状态响应方式
#define EN_AA 0x01 // 自动应答功能设置
#define EN_RXADDR 0x02 // 可用信道设置
#define SETUP_AW 0x03 // 收发地址宽度设置
#define SETUP_RETR 0x04 // 自动重发功能设置
#define RF_CH 0x05 // 工作频率设置
#define RF_SETUP 0x06 // 发射速率、功耗功能设置
#define STATUS 0x07 // 状态寄存器
#define OBSERVE_TX 0x08 // 发送监测功能
#define CD 0x09 // 地址检测
#define RX_ADDR_P0 0x0A // 频道0接收数据地址
#define RX_ADDR_P1 0x0B // 频道1接收数据地址
#define RX_ADDR_P2 0x0C // 频道2接收数据地址
#define RX_ADDR_P3 0x0D // 频道3接收数据地址
#define RX_ADDR_P4 0x0E // 频道4接收数据地址
#define RX_ADDR_P5 0x0F // 频道5接收数据地址
#define TX_ADDR 0x10 // 发送地址寄存器
#define RX_PW_P0 0x11 // 接收频道0接收数据长度
#define RX_PW_P1 0x12 // 接收频道0接收数据长度
#define RX_PW_P2 0x13 // 接收频道0接收数据长度
#define RX_PW_P3 0x14 // 接收频道0接收数据长度
#define RX_PW_P4 0x15 // 接收频道0接收数据长度
#define RX_PW_P5 0x16 // 接收频道0接收数据长度
#define FIFO_STATUS 0x17 // FIFO栈入栈出状态寄存器设置

//函数声明
uint8 Init_nRF24L01(void);
uint8 SPI_RW(uint8 tmp);
uint8 SPI_Read(uint8 reg);
uint8 SPI_RW_Reg(uint8 reg, uint8 value);
uint8 SPI_Write_Buf(uint8 reg, uint8 *pBuf, uint8 uchars);
void nRF24L01_TxPacket(unsigned char * tx_buf);
uint8 SPI_Read_Buf(uint8 reg, uint8 *pBuf, uint8 uchars);
static irqreturn_t nrf24l01_irq_handler(int irq, void *dev_id);

static struct fasync_struct *nrf24l01_async;

unsigned char ACTION;
unsigned char PLOAD_WIDTH = 6;

uint8 Init_nRF24L01(void)
{
	MISO_UP;
	CE_OUT;
	CSN_OUT;
	SCK_OUT;
	MOSI_OUT;
	MISO_IN;
	
	udelay(500);
	
	CE_L; // chip enable
	
	ndelay(60);
	CSN_H; // Spi disable
	
	ndelay(60);
	SCK_L; // Spi clock line init high
	
	ndelay(60);
	SPI_Write_Buf(WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH); // 写本地地址
	SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH); // 写接收端地址
	
	SPI_RW_Reg(WRITE_REG + EN_AA, 0x01);
	SPI_RW_Reg(WRITE_REG + EN_RXADDR, 0x01); // 允许接收地址只有频道0，如果需要多频道可以参考Page21
	SPI_RW_Reg(WRITE_REG + RF_CH, 0); // 设置信道工作为2.4GHZ，收发必须一致
	SPI_RW_Reg(WRITE_REG + RX_PW_P0, PLOAD_WIDTH); //设置接收数据长度，本次设置为1 字节
	SPI_RW_Reg(WRITE_REG + RF_SETUP, 0x07); //设置发射速率为1MHZ，发射功率为最大值0dB
	
	CE_H;
	ndelay(60);
  	return 1;
}
uint8 SPI_RW(uint8 tmp)
{
	uint8 bit_ctr;
	for(bit_ctr=0 ;bit_ctr<8 ;bit_ctr++) // output 8-bit
	{
		if(tmp & 0x80) 
			MOSI_H;// output 'tmp', MSB to MOSI
		else 
			MOSI_L;
		
		tmp <<= 1; // shift next bit into MSB..
		SCK_H; // Set SCK high..
		
		ndelay(60);
		
		if(MISO_STU!=0)	tmp |= 0x01;		 
		SCK_L; // ..then set SCK low again
		ndelay(60);
	}
	return(tmp); // return read tmp
}

uint8 SPI_Read(uint8 reg)
{
	uint8 reg_val;
	CSN_L; // CSN low, initialize SPI communication...
	ndelay(60);
	SPI_RW(reg); // Select register to read from..
	reg_val = SPI_RW(0); // ..then read registervalue
	CSN_H; // CSN high, terminate SPI communication
	ndelay(60); 
	return(reg_val); // return register value
}

uint8 SPI_RW_Reg(uint8 reg, uint8 value)
{
	uint8 status;
	CSN_L; // CSN low, init SPI transaction
	ndelay(60);
	status = SPI_RW(reg); // select register
	SPI_RW(value); // ..and write value to it..
	CSN_H; // CSN high again
	ndelay(60);
	return(status); // return nRF24L01 status uint8
}

uint8 SPI_Write_Buf(uint8 reg, uint8 *pBuf, uint8 uchars)
{
	uint8 status,uint8_ctr;
	CSN_L; //SPI使能
	ndelay(60);
	status = SPI_RW(reg);
	for(uint8_ctr=0; uint8_ctr<uchars; uint8_ctr++) //
	{
	    SPI_RW(*pBuf++);
	    ndelay(20);
	}
	CSN_H; //关闭SPI
	ndelay(60);
	return(status); //
}

uint8 SPI_Read_Buf(uint8 reg, uint8 *pBuf, uint8 uchars)
{
	uint8 status,uint8_ctr;
	CSN_L; // Set CSN low, init SPI tranaction
	ndelay(60);
	status = SPI_RW(reg); // Select register to write to and read status uint8
	for(uint8_ctr=0;uint8_ctr<uchars;uint8_ctr++)
	{
	    pBuf[uint8_ctr] = SPI_RW(0); 
	    ndelay(20);
	}
	CSN_H;
	ndelay(60);
	return(status); // return nRF24L01 status uint8
}

//函数：void nRF24L01_TxPacket(unsigned char * tx_buf)
//功能：发送 tx_buf中数据
void nRF24L01_TxPacket(unsigned char * tx_buf)
{
	PLOAD_WIDTH = 1;
	Init_nRF24L01();
	CE_L; //StandBy I模式
	ndelay(60);
	SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH); // 装载接收端地址
	SPI_Write_Buf(WR_TX_PLOAD, tx_buf, PLOAD_WIDTH); // 装载数据
	SPI_RW_Reg(WRITE_REG + CONFIG, 0x0e); // IRQ收发完成中断响应，16位CRC，主发送模式
	CE_H; //置高CE，激发数据发送
	udelay(10);
	SPI_RW(FLUSH_TX);
}

uint8 nRF24L01_RxPacket(uint8* rx_buf)
{
	uint8 status,revale=0;
	status = SPI_Read(STATUS);	// 读取状态寄存其来判断数据接收状况
	printk("status = %d\n",status);
	if(status&0x40)				// 判断是否接收到数据
	{
	    	CE_L;
		SPI_Read_Buf(RD_RX_PLOAD,rx_buf,PLOAD_WIDTH);// read receive payload from RX_FIFO buffer
		revale =1;			//读取数据完成标志
	}
	SPI_RW_Reg(WRITE_REG+STATUS,0xff);   //接收到数据后RX_DR,TX_DS,MAX_PT都置高为1，通过写1来其清除中断标志
	return revale;
}

void SetRX_Mode(void)
{
	int i = 0;
	for(i = TX_ADR_WIDTH;i>0;i--){
		TX_ADDRESS[i-1] =  i-1;
		RX_ADDRESS[i-1] =  i-1;
	}
	PLOAD_WIDTH = 6;
	Init_nRF24L01();
	CE_L;
	ndelay(60);
	SPI_RW_Reg(WRITE_REG + CONFIG, 0x0f);   		// IRQ收发完成中断响应，16位CRC	，主接收
	CE_H;
	udelay(130);
}

void substring(const char *src,char *dest,int index,int length)
{
	int count = 0;
	while(count<length){*(dest++) = *(src + index++);count++;}
	*(dest++) = '\0';
}

void parse_command(char *buffer)
{
	unsigned char i=0,k;
	char dest[2];
	for(i=0;i<5;i++){
		memset(dest,0,sizeof(dest));
		substring(buffer,dest,(i+1)*2,2);
		k = (unsigned char)simple_strtol(dest,NULL,16);
		TX_ADDRESS[i] = k;
		RX_ADDRESS[i] = k;
	}
	substring(buffer,dest,0,2);
	ACTION = (unsigned char)simple_strtol(dest,NULL,10);
 }

static ssize_t nrf24l01_write(struct file *filp, const char *buffer,size_t count, loff_t *ppos)
{
	char TxBuf[13];
	uint8 status,cnt;
	if( copy_from_user(TxBuf, buffer, 12) ){
		 printk("copy error!");
		 return -EFAULT;
	}

	TxBuf[12] = '\0';

	if(strlen(TxBuf)!=12){
		printk("send len is not 12\n");
		return -1;
	}

	if(strspn(TxBuf,"0123456789abcdefABCDEF")<12){
		printk("contain invalid character\n");
		return -1;
	}
	parse_command(TxBuf);	
	for(cnt = 0;cnt < 3;cnt++){
		nRF24L01_TxPacket(&ACTION);
		status=SPI_Read(STATUS);
		SPI_RW_Reg(WRITE_REG+STATUS,status);
		msleep(10);
	}
	//SetRX_Mode();
	if(status&0x20){
		return 0;
	}
	return -1;
}

static irqreturn_t nrf24l01_irq_handler(int irq, void *dev_id)
{
	if(PLOAD_WIDTH==6)
	{
		disable_irq_nosync(IRQ_EINT(11));
		kill_fasync (&nrf24l01_async, SIGIO, POLL_IN);
	}
	return IRQ_RETVAL(IRQ_HANDLED);
}

ssize_t nrf24l01_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	unsigned char ret = 0;
	unsigned char rcv[6]={0};
	nRF24L01_RxPacket(rcv);
	SPI_RW_Reg(FLUSH_RX,0);
	mdelay(10);
	CE_L;
	ndelay(60);
	SPI_RW_Reg(WRITE_REG + CONFIG, 0x0f);   		// IRQ收发完成中断响应，16位CRC	，主接收
	CE_H;
	udelay(130);
	ret = copy_to_user(buf, rcv, 6);
	enable_irq(IRQ_EINT(11));
	return ret ? -EFAULT : PLOAD_WIDTH;
}

static int nrf24l01_open(struct inode *node, struct file *file)
{
	int ret = request_irq(IRQ_EINT(11), nrf24l01_irq_handler, IRQ_TYPE_EDGE_FALLING, "nrf24l01",(void *)NULL);
	if(ret)return -1;
	//SetRX_Mode();
	return 0;
}

static int nrf24l01_close(struct inode *node, struct file *file)
{
	free_irq(IRQ_EINT(11),(void *)NULL);
	return 0;
}

static int nrf24l01_fasync (int fd, struct file *filp, int on)
{
	return fasync_helper (fd, filp, on, &nrf24l01_async);
}



static struct file_operations nrf24l01_fops = {
	.owner 	= 	THIS_MODULE,
	.open  	= 	nrf24l01_open,
	.write 	= 	nrf24l01_write,
	.read		=	nrf24l01_read,
	.release	=	nrf24l01_close,
	.fasync	= 	nrf24l01_fasync,
};

static struct miscdevice misc = {	
	.minor = MISC_DYNAMIC_MINOR,	
	.name  = DEVICE_NAME,
	.fops  = &nrf24l01_fops,
};

static int __init nrf24l01_init(void)
{
	int ret;
	ret =  misc_register(&misc);
	printk (DEVICE_NAME"\tinitialized\n");
	return ret;
}

static void __exit nrf24l01_exit(void)
{
	misc_deregister(&misc);	
	printk(DEVICE_NAME"\texit\n");
}


module_init(nrf24l01_init);
module_exit(nrf24l01_exit);

MODULE_DESCRIPTION("nrf24l01 driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("<baobao@sky-light.com.hk>");
