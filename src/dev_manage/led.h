#ifndef __LED_H__
#define __LED_H__

typedef enum{
	LED_FOR_PROGRAM_RUN = 0,
	LED_FOR_NETWORK,
	LED_FOR_ZIGBEE_NET,
	LED_FOR_FUTURE_USE,
	LED_MAX
}led_item;

typedef enum{
	ON   = 1,
	OFF = 0
}led_state;

void led_control(led_item item,led_state state);
void led_flash(led_item item,int interval,int times);

#endif
