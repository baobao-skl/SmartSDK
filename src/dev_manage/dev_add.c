#include "dev_add.h"
#include "dev_db.h"
#include "dev.h"

DEV_ADD_RETURN AddDevToStaticTable(const char *mac,const char *name,const char *unicode,const char*groupname,const unsigned char dev_number,const unsigned char type)
{
	int group_id = 0;
	DEV_ADD_RETURN ret = DEV_ADD_OK;
	if((ret = isMacOrNameExist(mac,name,dev_number))!=DEV_ADD_OK){
		return ret;
	}

	//if(isGroupNameExist(groupname)==FALSE){
	DEBUG_MSG("groupname->%s\n",groupname);
	AddDevGroupName(groupname);
	//}

	if((ret = GetGroupIDByName(groupname, &group_id))==FALSE){
		return DEV_ADD_FAIL;
	}

	DEBUG_MSG("group_id->%d\n",group_id);
	
	if(insertIntoStaticTable(mac,name,unicode,group_id,dev_number,type)== FALSE){
		return DEV_ADD_FAIL;
	}
	return DEV_ADD_OK;
}

BOOL AddDevToDynamicTable(const char *mac,const char *ip)
{
	if(isMacExistInDynamicTable(mac))
	{
		return UpdateDevIPByMac(mac, ip);
	}
	else
	{
		return insertIntoDynamicTable(mac, ip);
	}
}

