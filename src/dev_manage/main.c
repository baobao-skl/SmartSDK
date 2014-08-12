#include "socketd.h"
#include <unistd.h>
#include <pthread.h>
#include "uart.h"
#include "sms.h"
#include "dev.h"
#include <stdio.h>
#include "uart_common.h"
#include "sms_common.h"
#include "sms_control.h"
#include "led.h"
#include "timertask.h"
#include "signal.h"

extern uart_t uart;
extern sms_t sms;

app_status_t app_status = {0};

void app_init(void)
{
	unsigned char weekday = 0;
	led_control(LED_FOR_PROGRAM_RUN, ON);
	app_status.isTimerTaskRun = TRUE;
	app_status.isHasTimerTasks = TRUE;
	app_status.is_uart_ok = uart.init();
	app_status.is_sms_ok = sms.init();

	GetCurrentTime(app_status.CurrentTime, &weekday);
	DEBUG_MSG("current-time: %s week:%d\n", app_status.CurrentTime, weekday);

	signal(SIGPIPE, SIG_IGN);//ingnore sigpipe signal
}

int main(int argc,char **argv)
{
	pthread_t Socket_ThreadId,Uart_ThreadId,Sms_ThreadId,Timer_ThreadId;
	app_init();
	if(app_status.isTimerTaskRun){
		pthread_create(&Timer_ThreadId, NULL, TimerTaskRun, NULL);
		pthread_detach(Timer_ThreadId);
	}
	if(app_status.is_uart_ok){
		pthread_create(&Uart_ThreadId, NULL, uart_thread, NULL);
  		pthread_detach(Uart_ThreadId);
	}else{
		DEBUG_MSG("Zigbee Service is not available!!\n");
	}
	if(app_status.is_sms_ok){
		pthread_create(&Sms_ThreadId, NULL, sms_read_thread, handler_on_user_message);
  		pthread_detach(Sms_ThreadId);
	}else{
		DEBUG_MSG("SMS Service is not available!!\n");
	}
	//create a thread to receive remote command
	pthread_create(&Socket_ThreadId, NULL, server_thread, NULL);
  	pthread_detach(Socket_ThreadId);

	pause();//pause wil be waken up by signal
	return 0;
}
