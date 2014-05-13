#ifndef __FAMILYGROUP_H__
#define __FAMILYGROUP_H__

#include "dev.h"

typedef struct{
	char group_id[512];
	char group_name[512];
}groupInfo_t;

BOOL SetFamilyGroupInfo(const groupInfo_t group_info);
BOOL GetFamilyGroupInfo(groupInfo_t *group_info);

#endif

