#ifndef __RING_H__
#define __RING_H__

typedef enum{
	DEV_ALARM_RING_CMD = 0,
	RING_MAX
}ring_type_t;

typedef struct{
	char path[50];
}ring_item;

void system_ring(ring_type_t type);

#endif
