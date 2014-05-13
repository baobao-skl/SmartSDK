#ifndef __NET_BROADCAST_H__
#define __NET_BROADCAST_H__
#include "dev.h"

#define BROADCAST_PORT 9090

void send_broadcast(const char *who,USER_ACTION user_action,char *dev_name);
#endif

