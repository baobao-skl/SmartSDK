#include "familygroup.h"
#include <string.h>

#define GROUP_INFO_STORE_PATH "/usr/dev/family_info.ini"

BOOL SaveFamilyGroupInfo(const groupInfo_t group_info);

BOOL SetFamilyGroupInfo(const groupInfo_t group_info)
{
	DEBUG_MSG("GroupID = %s\n",group_info.group_id);
	DEBUG_MSG("GroupName = %s\n",group_info.group_name);
	return SaveFamilyGroupInfo(group_info);
}

BOOL SaveFamilyGroupInfo(const groupInfo_t group_info)
{
	FILE *fid = fopen(GROUP_INFO_STORE_PATH,"at+");
	char line[1024] = {0};
	sprintf(line,"%s\r\n%s\r\n",group_info.group_id,group_info.group_name);
	if(!fid){DEBUG_MSG("open "GROUP_INFO_STORE_PATH" error!\n");return FALSE;}
	fwrite(line,strlen(line),1,fid);
	fclose(fid);
	return TRUE;
}

BOOL GetFamilyGroupInfo(groupInfo_t *group_info)
{
	FILE *fid = fopen(GROUP_INFO_STORE_PATH,"r");
	if (fid==NULL) return FALSE;
	char *line = NULL;
	size_t len;
	size_t size;
	size = getline(&line,&len,fid);
	if (size==-1)
		goto fail;
	
	strncpy(group_info->group_id,line,strlen(line));
	group_info->group_id[strlen(group_info->group_id)-2]='\0';
	DEBUG_MSG("id = %s\n",group_info->group_id);
	memset(line,0,sizeof(line));
	size = getline(&line,&len,fid);
	if(size==-1)
		goto fail;
	strncpy(group_info->group_name,line,strlen(line));
	group_info->group_name[strlen(group_info->group_name)-2]='\0';
	DEBUG_MSG("name = %s\n",group_info->group_name);
	fclose(fid);
	return TRUE;
fail:
	fclose(fid);
	return FALSE;
}

