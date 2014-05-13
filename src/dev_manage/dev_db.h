#ifndef __DATABASE_H__
#define __DATABASE_H__
#include "dev.h"

#define DEV_DB_NAME "/usr/dev/dev.db"

BOOL isNewNameExist(const char *new_name);
DEV_ADD_RETURN isMacOrNameExist(const char *mac,const char *name,const unsigned char devnumber);
BOOL isMacAndNameMatch(const char *mac,const char *name);
BOOL insertIntoStaticTable(const char *mac,const char *name,const char *unicode,int group_id,const unsigned char dev_number,const unsigned char type);
BOOL insertIntoDynamicTable(const char *mac,const char *ip);
BOOL deleteFromStaticTable(const char *mac,const char *name);
BOOL GetDevIPByMac(const char *mac,char *ip);
BOOL isMacExistInStaticTable(const char *mac);
BOOL isMacExistInDynamicTable(const char *mac);
BOOL UpdateDevNameByMacAndOldname(const char *mac,const char *oldname,const char *new_name,const char *unicode);
BOOL UpdateDevIPByMac(const char *mac,const char *new_ip);
BOOL GetDevNameByIP(const char *ip,char *name);
BOOL GetDevUnicodeNameByIP(const char *ip,char *unicode_name);
BOOL GetMacAndDevNumberByID(const char *id,char  *mac,unsigned char *devnumber);
BOOL GetIPAndDevNumberByID(const char *id,char *ip,unsigned char *devnumber);
BOOL AddDevGroupName(const char*group_name);
BOOL isGroupNameExist(const char *group_name);
BOOL GetGroupIDByName(const char *group_name,int *id);
BOOL UpdateDevGroupIDByMacAndName(const char *mac,const char *name,const int group_id);
unsigned char GetDevNumberByMacAndName(const char *mac,const char *name);
void GetDeviceList(void);
void GetUnicodeDeviceList(void);
void GetAllGroupName(void);
void ReOrderStaticTableItemID(void);
char *get_dev_list(void);
char *get_group_list(void);
char *get_unicode_dev_list(void);



#endif
