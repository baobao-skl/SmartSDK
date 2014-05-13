#ifndef __LED_H__
#define __LED_H__

typedef enum{
	LED_FOR_ZIGBEE_NET = 0,
	LED_FOR_NETWORK,
	LED_FOR_ALARM_TIP,
	LED_FOR_FUTURE_USE,
	LED_MAX
}led_item;

typedef enum{
	ON   = 1,
	OFF = 0
}led_state;

#endif
