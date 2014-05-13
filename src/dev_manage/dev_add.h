#ifndef __DEV_ADD_H__
#define __DEV_ADD_H__

#include "dev.h"

DEV_ADD_RETURN AddDevToStaticTable(const char *mac,const char *name,const char *unicode,const char*groupname,const unsigned char dev_number,const unsigned char type);
BOOL AddDevToDynamicTable(const char *mac,const char *ip);

#endif

