#ifndef __SOCKETD_H__
#define __SOCKETD_H__
#include "dev.h"

#define DEV_MANAGE_PORT 2003
#define SOCKET_INFO_SEPARATOR "|"

#define SOCKET_RESPONSE_YES "YES"
#define SOCKET_RESPONSE_NO "NO"
#define SOCKET_RESPONSE_MAC_EXIST "MAC_EXIST"
#define SOCKET_RESPONSE_NAME_EXIST "NAME_EXIST"

void *server_thread( void *arg );
#endif
