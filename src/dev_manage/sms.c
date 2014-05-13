#include "sms.h"

static BOOL sms_init(void);
static BOOL uart_config(void);
static BOOL gsm_init(void);
static BOOL sms_send(const char *_to,const char *_msg);
static BOOL write_to_uart(const char *final,int length);
static void number_handler(char *to);
static void center_next_handler(char *center);
message_info_t handler_receive_msg(char *src);
static BOOL sms_send_to_user(const char *_to,const char *_msg);
static BOOL sms_send_support_multi(const char *_to,const char *_msg);

sms_t sms = {
	0,
	sms_init,
	sms_send_to_user
};


static BOOL sms_init(void)
{
	if(uart_config()==FALSE)
		return FALSE;
	return gsm_init();
}


static BOOL sms_send_to_user(const char *_to,const char *_msg)
{
	if(_to == NULL || _msg == NULL)
		return FALSE;
	if(strlen(_msg)<=280){
		return sms_send(_to,_msg);
	}
	return sms_send_support_multi(_to,_msg);
}

static BOOL sms_send_support_multi(const char *_to,const char *_msg)
{
	int sms_lenght = 0,i = 0;
	char send_msg[290] = {0};
	sms_lenght = strlen(_msg);
	if(sms_lenght/280 > 0){
		for(i =0;i<(sms_lenght/280);i++)//290/280 = 1
		{
			substring(_msg, send_msg, i*280, 280);
			sms_send(_to,send_msg);
			memset(send_msg,0,sizeof(send_msg));
		}
	}
	substring(_msg, send_msg, i*280, sms_lenght-i*280);
	return sms_send(_to,send_msg);
}


static BOOL sms_send(const char *_to,const char *_msg)
{
	char to[20];
	char center[20];
	char msg[MAX_BUFFER_SIZE];
	char final[MAX_BUFFER_SIZE];
	//gsm_init();
	//handle receive number
	memset(to,0,sizeof(to));
	sprintf(to,"+86%s",_to);
	number_handler(to);

	//handle center number
	memset(center,0,sizeof(center));
	strcpy(center,CENTER_NUMBER);
	number_handler(center);
	center_next_handler(center);

	//handles msg
	memset(msg,0,sizeof(msg));

	sprintf(msg,"%.2X%s",strlen(_msg)/2,_msg);
	//zuhe
	memset(final,0,sizeof(final));
	sprintf(final,"1100%.2X91%s000800%s",strlen(_to)+2,to,msg);

	int length = strlen(final)/2;

	//write to uart
	if(write_to_uart(final,length)){
		return TRUE;
	}
	return FALSE;
}

static BOOL gsm_init(void)
{
	char buffer[MAX_BUFFER_SIZE]={0};
	strcpy(buffer,"AT\r");
	write(sms.fd, buffer , strlen(buffer));
	memset(buffer, 0 ,sizeof(buffer));
	//close echo back
	strcpy(buffer,"ATE0\r");
	write(sms.fd, buffer , strlen(buffer));
	memset(buffer, 0 ,sizeof(buffer));
	//set tip way
	strcpy(buffer,"AT+CNMI=2,1,0,2,1\r");
	write(sms.fd, buffer , strlen(buffer));
	memset(buffer, 0 ,sizeof(buffer));
	//set store way
	strcpy(buffer,"AT+CPMS=\"SM\",\"SM\",\"SM\"\r");
	write(sms.fd, buffer,strlen(buffer));
	memset(buffer, 0 ,sizeof(buffer));
	//set PDU mode
	strcpy(buffer,"AT+CMGF=0\r");
	write(sms.fd,buffer,strlen(buffer));
	return TRUE;
}

static BOOL uart_config(void)
{
	struct termios options;
	if((sms.fd =open(SMS_DEV_NAME,O_RDWR|O_NOCTTY|O_NDELAY))<0){
		close(sms.fd);
		return FALSE;
	}
	tcgetattr(sms.fd, &options);
	tcflush(sms.fd, TCIOFLUSH);
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	cfsetispeed(&options, B9600);
	cfsetospeed(&options, B9600);
 	options.c_cflag &= ~CSIZE;
 	options.c_cflag |= CS8;
 	options.c_cflag &= ~PARENB;
 	options.c_cflag &= ~INPCK;
 	options.c_cflag &= ~CSTOPB;
 	tcsetattr(sms.fd, TCSANOW, &options);
 	tcflush(sms.fd, TCIOFLUSH);
	return TRUE;
}


message_info_t handler_receive_msg(char *src)
{
	message_info_t rcv_info_t;
	time_info_t time_info;
	char t[2] = {0};
	char len[2] = {0};
	char temp_charset[3] ={0};
	int length = 0,pre_length = 0;
	//get center number
	substring(src,rcv_info_t.center_number,6,12);
	number_handler(rcv_info_t.center_number);
	rcv_info_t.center_number[11] = '\0';
	//get send number
	substring(src,len,20,2);
	length = strtol(len,NULL,16);
	pre_length = length;
	if(pre_length%2) length+=1;
	if(*(src+22)=='9' && *(src+23)=='1')
	{
		substring(src,rcv_info_t.send_number,26,length-2);
	}
	else
	{
		substring(src,rcv_info_t.send_number,24,length);
	}
	number_handler(rcv_info_t.send_number);
	if(pre_length%2) 
	{
		if(*(src+22)=='9' && *(src+23)=='1')
		{
			rcv_info_t.send_number[pre_length-2] = '\0';
		}
		else
		{
			rcv_info_t.send_number[pre_length] = '\0';
		}
	}
	substring(src,temp_charset,26+length,2);
	if(strstr(temp_charset,"08")){
		DEBUG_MSG("CHARSET_UCS2\n");
		rcv_info_t.charset = CHARSET_UCS2;
	}else if(strstr(temp_charset,"00")){
		DEBUG_MSG("CHARSET_7BITS\n");
		rcv_info_t.charset = CHARSET_7BITS;
	}else if(strstr(temp_charset,"04")){
		DEBUG_MSG("CHARSET_8BITS:%s\n",temp_charset);
		rcv_info_t.charset = CHARSET_8BITS;
	}
	//get send time
	substring(src,rcv_info_t.send_time,28+length,12);
	number_handler(rcv_info_t.send_time);
	substring(rcv_info_t.send_time, t, 0, 2);
	time_info.year = atoi(t);
	substring(rcv_info_t.send_time, t, 2, 2);
	time_info.month= atoi(t);	
	substring(rcv_info_t.send_time, t, 4, 2);
	time_info.day = atoi(t);
	substring(rcv_info_t.send_time, t, 6, 2);
	time_info.hour = atoi(t);
	substring(rcv_info_t.send_time, t, 8, 2);
	time_info.minute = atoi(t);
	substring(rcv_info_t.send_time, t, 10, 2);
	time_info.second = atoi(t);
	sprintf(rcv_info_t.send_time,"20%.2d-%.2d-%.2d %.2d:%.2d:%.2d",
		time_info.year,time_info.month,time_info.day,time_info.hour,time_info.minute,time_info.second);
	//get content length
	substring(src, len, 42+length, 2);	
	//get content
	substring(src,rcv_info_t.send_content,44+length,strtol(len,NULL,16)*2);
	return rcv_info_t;
}

static BOOL write_to_uart(const char *final,int length)
{
	int writelen = 0;
	char buffer[MAX_BUFFER_SIZE]={0};
	//send message length
	sprintf(buffer,"AT+CMGS=%.2d\r",length);
	write(sms.fd,buffer,strlen(buffer));
	memset(buffer, 0, sizeof(buffer));
	//send message content
	sprintf(buffer,"%s\x1A",final);
	writelen = write(sms.fd,buffer,strlen(buffer));
	DEBUG_MSG("sms len = %d send= %s--\n",writelen,buffer);
	return TRUE;
}

static void number_handler(char *to)
{
	unsigned int i = 0,str_len=0;
	char tmp;
	if(*to == '+'){
		for(i = 0;i <(strlen(to)-1);i++){
			*(to + i) = *(to + i +1);
		} 
		*(to + i) = '\0';
	}	
	str_len = strlen(to);
	if(str_len%2==1){
		*(to + str_len) = 'F';
		*(to + str_len + 1) = '\0';
	}
	for(i = 0;i < strlen(to);i+=2){
		tmp = *(to + i);
		*(to + i) = *(to + i + 1);
		*(to + i + 1) = tmp;	
	}
}

static void center_next_handler(char *center)
{
	unsigned int str_len = 0,i = 0,tmp = 0;
	str_len = strlen(center);
	for(i = str_len+2;i>=2;i--){
		*(center+i) = *(center+i-2);
	}
	*(center+str_len+3) = '\0';
	strncpy(center,"91",2);

	tmp = strlen(center)/2;
	
	str_len = strlen(center);
	for(i = str_len+2;i>=2;i--){
		*(center+i) = *(center+i-2);
	}
	*(center+str_len+3) = '\0';
	*center = (char)(tmp/10) + 0x30;
	*(center + 1) = (char)(tmp%10) + 0x30;
}

void substring(const char *src,char *dest,int index,int length)
{
	int count = 0;
	while(count<length){*(dest++) = *(src + index++);count++;}
	*(dest++) = '\0';
}

BOOL save_gsm_log(message_info_t history_info)
{
	FILE *fid = fopen(GSM_LOG_PATH,"at+");
	char line[1024] = {0};
	sprintf(line,"%-20s%-20s%-25s%s\n",
		history_info.center_number,history_info.send_number,history_info.send_time,history_info.send_content);
	if(!fid){DEBUG_MSG("open "GSM_LOG_PATH" error!\n");return FALSE;}
	fwrite(line,strlen(line),1,fid);
	fclose(fid);
	return TRUE;
}

