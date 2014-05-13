#include "dev_edit.h"
#include "dev_db.h"


BOOL UpdateDevDisplayName(const char *mac,const char *oldname,const char *new_name,const char *unicode)
{
	if(isMacAndNameMatch(mac,oldname)){
		if(isNewNameExist(new_name)){
			return FALSE;
		}
		if(UpdateDevNameByMacAndOldname(mac,oldname,new_name,unicode)){
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

BOOL UpdateDevGroupName(const char *mac,const char *name,const char *new_group_name)
{
	int group_id= 0;
	if(isMacAndNameMatch(mac,name)==FALSE){
		return FALSE;
	}
	if(GetGroupIDByName(new_group_name, &group_id)==FALSE){
		return FALSE;
	}
	return UpdateDevGroupIDByMacAndName(mac,name,group_id);
}



