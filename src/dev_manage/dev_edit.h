#ifndef __DEV_EDIT_H__
#define __DEV_EDIT_H__
#include "dev.h"


BOOL UpdateDevDisplayName(const char *mac,const char *oldname,const char *new_name,const char *unicode);
BOOL UpdateDevGroupName(const char *mac,const char *name,const char *new_group_name);

#endif


