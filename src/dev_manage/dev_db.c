#include "dev_db.h"
#include "dev.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "version_manager.h"
#include "include/sqlite3.h"

static void CreateStaticTable(void);
static void CreateDynamicTable(void);
static void CreateDevGroupTable(void);

sqlite3 *db_dev = NULL;
static char g_dev_list[1024*4] = {0};
static char g_unicode_dev_list[1024*16] ={0};
static char g_group_name[1024*4] = {0};

char *get_group_list(void)
{
	return g_group_name;
}

char *get_unicode_dev_list(void)
{
	return g_unicode_dev_list;
}

char *get_dev_list(void)
{
	return g_dev_list;
}

/*
  * open database
  */
static BOOL database_open(void)
{
	int rc = SQLITE_OK;
	rc = sqlite3_open(DEV_DB_NAME,&db_dev);
	if(rc == SQLITE_ERROR){
		DEBUG_MSG("open database error!!\n");
		return FALSE;
	}
	CreateStaticTable();
	CreateDynamicTable();
	CreateDevGroupTable();
	return TRUE;
}
/*
  * close database
  */
static void database_close(void)
{
	if(db_dev!=NULL)
		sqlite3_close(db_dev);
}

/*
  * create a table about device name and mac 
  */
static void CreateStaticTable(void)
{
	char *sql = "create table statictable(F_ID INTEGER PRIMARY KEY AUTOINCREMENT,F_MAC VARCHAR(30),F_NAME VARCHAR(50),F_UNICODE VARCHAR(200),F_GROUPID INTEGER DEFAULT 1,F_DEVNUMBER INTEGER DEFAULT 0,F_DEVTYPE INTEGER DEFAULT 0);" ;
	sqlite3_exec(db_dev, sql,NULL,NULL,NULL);
}
/*
  * create a table about device ip  and mac 
  */
static void CreateDynamicTable(void)
{
	char *sql = "create table dynamictable(F_MAC VARCHAR(30),F_IP VARCHAR(20));" ;
	sqlite3_exec(db_dev, sql,NULL,NULL,NULL);
}

static void CreateDevGroupTable(void)
{
	char *sql = "create table grouptable(F_ID INTEGER PRIMARY KEY AUTOINCREMENT,F_GROUPNAME VARCHAR(200));" ;
	sqlite3_exec(db_dev, sql,NULL,NULL,NULL);
}

BOOL isGroupNameExist(const char *group_name)
{
	char **azResult;
	int row_num = 0,column_num = 0;
	char sql[1024] = {0};
	if(database_open() == FALSE) return FALSE;
	sprintf(sql,"select * from grouptable where F_GROUPNAME=%s",group_name);
	sqlite3_get_table(db_dev,sql,  &azResult, &row_num, &column_num, NULL);
	if(row_num>=1){
		sqlite3_free_table(azResult);
		database_close();
		return TRUE;
	}
	database_close();
	return FALSE;
}

BOOL GetGroupIDByName(const char *group_name,int *id)
{
	char **azResult;
	int row_num = 0,column_num = 0;
	char sql[1024] = {0};
	if(database_open() == FALSE) return FALSE;
	sprintf(sql,"select F_ID from grouptable where F_GROUPNAME='%s'",group_name);
	DEBUG_MSG("GetGroupIDByName-->%s\n",sql);
	sqlite3_get_table(db_dev,sql,  &azResult, &row_num, &column_num, NULL);
	if(row_num>=1){
		*id = atoi(azResult[1]);
		DEBUG_MSG("GroupID = %d\n",*id);
		sqlite3_free_table(azResult);
		database_close();
		return TRUE;
	}
	database_close();
	return FALSE;
}

BOOL UpdateDevGroupIDByMacAndName(const char *mac,const char *name,const int group_id)
{
	char sql[1024] = {0};
	if(database_open()==FALSE) return FALSE;
	sprintf(sql,"update statictable set F_GROUPID=%d where F_MAC='%s' and F_NAME='%s'",group_id,mac,name);
	if(sqlite3_exec(db_dev, sql,NULL,NULL,NULL)==SQLITE_OK)
	{
		database_close();
		return TRUE;
	}
	database_close();
	return FALSE;
}

BOOL AddDevGroupName(const char*group_name)
{
	int row_num = 0,column_num = 0;
	char sql[1024] = {0};
	char **azResult;
	if(database_open()==FALSE) return FALSE;
	sprintf(sql,"select * from grouptable where F_GROUPNAME='%s'",group_name);
	sqlite3_get_table(db_dev,sql,  &azResult, &row_num, &column_num, NULL);
	if(row_num>0){
		sqlite3_free_table(azResult);
		database_close();
		return FALSE;
	}
	sprintf(sql,"insert into grouptable(F_GROUPNAME) values('%s')",group_name);
	if(sqlite3_exec(db_dev, sql,NULL, NULL,NULL)==SQLITE_OK){
		database_close();
		return TRUE;
	}
	database_close();
	return FALSE;
}

void GetAllGroupName(void)
{
	if(database_open()==FALSE) return;

	char *sql = "select F_GROUPNAME from grouptable";
	int row_num = 0,column_num = 0,i = 0;
	char **azResult;
	sqlite3_get_table(db_dev,sql,  &azResult, &row_num, &column_num, NULL);
	memset(g_group_name,0,sizeof(g_group_name));
	if(row_num >= 1){
		for(i = 1;i<row_num+1;i++)
		{
			strcat(g_group_name,azResult[ i*column_num]);
			strcat(g_group_name,",");
		}
		g_group_name[strlen(g_group_name)-1]='\0';
		strcat(g_group_name,"\0");
	}
	else{
		strcpy(g_group_name,"_NULL_");
	}
	sqlite3_free_table(azResult);
	database_close();
}

/*
  * get device list
  */
  //select F_MAC,F_NAME,F_GROUPNAME from statictable a inner join grouptable b on a.F_ID=b.F_ID;
/*
strcat(g_dev_list,azResult[ i*column_num]);
strcat(g_dev_list,":");
strcat(g_dev_list,azResult[ i*column_num+1]);
strcat(g_dev_list,":");
strcat(g_dev_list,azResult[ i*column_num+2]);
strcat(g_dev_list,",");
*/
void GetDeviceList(void)
{
	//01010000:厨房,01020000:电视机
	if(database_open()==FALSE) return;
	char buffer[1024*2] = {0};
	char temp_ver[512] = {0};
	char *sql = "select F_MAC,F_NAME,F_GROUPNAME,F_DEVTYPE,F_DEVNUMBER from statictable a inner join grouptable b on a.F_GROUPID=b.F_ID";
	int row_num = 0,column_num = 0,i = 0;
	char **azResult;
	sqlite3_get_table(db_dev,sql,  &azResult, &row_num, &column_num, NULL);
	memset(g_dev_list,0,sizeof(g_dev_list));
	if(row_num >= 1){
		strcat(g_dev_list,"{\"ver\":\"");
		GetDevVersion(temp_ver);
		strcat(g_dev_list,temp_ver);
		strcat(g_dev_list,"\",\"dev\":[");
		for(i = 1;i<row_num+1;i++)
		{
			sprintf(buffer,"{\"mac\":\"%s\",\"name\":\"%s\",\"group_name\":\"%s\",\"type\":\"%s\",\"devnumber\":\"%s\"},",
						azResult[ i*column_num],
						azResult[ i*column_num+1],
						azResult[ i*column_num+2],
						azResult[ i*column_num+3],
						azResult[ i*column_num+4]);
			strcat(g_dev_list,buffer);
		}
		g_dev_list[strlen(g_dev_list)-1] = ']';
		strcat(g_dev_list,",\"group\":[");
		sql = "select F_GROUPNAME from grouptable";
		sqlite3_get_table(db_dev,sql,  &azResult, &row_num, &column_num, NULL);
		if(row_num >= 1){
			for(i = 1;i < row_num+1; i++)
			{
				sprintf(buffer,"{\"name\":\"%s\"},",azResult[ i*column_num]);
				strcat(g_dev_list,buffer);
			}
		}
		g_dev_list[strlen(g_dev_list)-1] = ']';
		strcat(g_dev_list,"}\0");
	}
	else{
		strcpy(g_dev_list,"_NULL_");
	}
	sqlite3_free_table(azResult);
	database_close();
}

void GetUnicodeDeviceList(void)
{
	//01010000:厨房,01020000:电视机
	//ReOrderStaticTableItemID();
	if(database_open()==FALSE) return;
	char *sql = "select F_ID,F_UNICODE from statictable";
	char index[48] = {0};
	int row_num = 0,column_num = 0,i = 0,count = 0;
	char **azResult;
	sqlite3_get_table(db_dev,sql,  &azResult, &row_num, &column_num, NULL);
	memset(g_unicode_dev_list,0,sizeof(g_unicode_dev_list));
	if(row_num >= 1){
		for(i = 1;i<row_num+1;i++)
		{
			count = atoi(azResult[ i*column_num]);
			if(count < 10){
				sprintf(index,"%.4X002E",0x30+count);
			}else if(count < 100){
				sprintf(index,"%.4X%.4X002E",0x30+count/10,0x30+count%10);
			}else if(count < 1000){
				sprintf(index,"%.4X%.4X%.4X002E",0x30+count/100,0x30+count%100/10,0x30+count%10);
			}
			strcat(g_unicode_dev_list,index);
			strcat(g_unicode_dev_list,azResult[ i*column_num+1]);
			strcat(g_unicode_dev_list,"000D000A");
		}
		strcat(g_unicode_dev_list,"8BF756DE590D7F1653F7002B0030517395ED002C7F1653F7002B003162535F00002C7F1653F7002B003283B753D672B66001002C8C228C220021");
	}
	else{
		strcpy(g_unicode_dev_list,"_NULL_");
	}
	sqlite3_free_table(azResult);
	database_close();
}

BOOL isNewNameExist(const char *new_name)
{
	if(new_name == NULL){
		return FALSE;
	}
	BOOL ret = FALSE;
	int row_num = 0,column_num = 0;
	char sql[1024] = {0};
	char **azResult;
	if(database_open()==FALSE) return FALSE;
	sprintf(sql,"select * from statictable where F_NAME='%s'",new_name);
	sqlite3_get_table(db_dev,sql, &azResult, &row_num, &column_num, NULL);
	if(row_num>0)	{
		ret = TRUE;
	}
	database_close();
	return ret;
}

//this is for add_dev----------------start
DEV_ADD_RETURN isMacOrNameExist(const char *mac,const char *name,const unsigned char devnumber)
{
	int row_num = 0,column_num = 0,ret = 0;
	char sql[1024] = {0};
	char **azResult;
	if(mac == NULL || name == NULL){
		return DEV_ADD_FAIL;
	}
	if(database_open()==FALSE) return DEV_ADD_FAIL;
	if(devnumber == 0){//signal device
		sprintf(sql,"select * from statictable where F_MAC='%s'",mac);
	}else{
		sprintf(sql,"select * from statictable where F_MAC='%s' and (F_DEVNUMBER='%d' or F_DEVNUMBER='0')",mac,devnumber);
	}
	sqlite3_get_table(db_dev,sql,  &azResult, &row_num, &column_num, NULL);
	if(row_num>0){
		DEBUG_MSG("mac exist already\n");
		ret = DEV_MAC_EXIST;
		goto back;
	}
	sprintf(sql,"select * from statictable where F_NAME='%s'",name);
	sqlite3_get_table(db_dev,sql, &azResult, &row_num, &column_num, NULL);
	if(row_num>0)	{
		DEBUG_MSG("name exist already\n");
		ret = DEV_NAME_EXIST;
		goto back;
	}
	database_close();
	return DEV_ADD_OK;
back:
	sqlite3_free_table(azResult);
	database_close();
	return ret;
}

#if 0
int GetStaticTableItemsCount()
{
	int row_num = 0,column_num = 0;
	if(database_open()==FALSE) return -1;
	char *sql = "select * from statictable";
	char **azResult;
	sqlite3_get_table(db_dev,sql,  &azResult, &row_num, &column_num, NULL);
	sqlite3_free_table(azResult);
	database_close();
	return row_num;
}
#endif

void ReOrderStaticTableItemID(void)
{
	if(database_open()==FALSE) return;
	char sql[1024] = {0};
	int row_num = 0,column_num = 0,i = 0;
	char **azResult;
	sprintf(sql,"select F_ID,F_MAC,F_NAME from statictable");
	sqlite3_get_table(db_dev,sql,  &azResult, &row_num, &column_num, NULL);
	if(row_num >= 1){
		for(i = 1;i<row_num+1;i++)
		{
			memset(sql,0,sizeof(sql));
			sprintf(sql,"update statictable set F_ID=%d where F_MAC='%s'",i, azResult[i*column_num+1]);
			DEBUG_MSG("sql = %s\n",sql);
			sqlite3_exec(db_dev, sql,NULL, NULL,NULL);
		}
	}
	sqlite3_free_table(azResult);
	database_close();
}

BOOL insertIntoStaticTable(const char *mac,const char *name,const char *unicode,int group_id,const unsigned char dev_number,const unsigned char type)
{
	char sql[1024] = {0};
	if(database_open()==FALSE) return FALSE;
	sprintf(sql,"insert into statictable(F_MAC,F_NAME,F_UNICODE,F_GROUPID,F_DEVNUMBER,F_DEVTYPE) values('%s','%s','%s',%d,%d,%d)",mac,name,unicode,group_id,dev_number,type);
	DEBUG_MSG("sql = %s\n",sql);
	int ret = sqlite3_exec(db_dev, sql,NULL, NULL,NULL);
	database_close();
	if(ret == SQLITE_OK)
		return TRUE;
	return FALSE;
}
//this is for add_dev----------------end



//this is for delete_dev----------------start
BOOL isMacAndNameMatch(const char *mac,const char *name)
{
	int row_num = 0,column_num = 0;
	char sql[1024] = {0};
	char **azResult;
	if(database_open()==FALSE) return FALSE;
	sprintf(sql,"select * from statictable where F_MAC='%s' and F_NAME='%s'",mac,name);
	sqlite3_get_table(db_dev,sql,  &azResult, &row_num, &column_num, NULL);
	if(row_num>0){
		sqlite3_free_table(azResult);
		database_close();
		return TRUE;
	}
	database_close();
	return FALSE;
}

BOOL deleteFromStaticTable(const char *mac,const char *name)
{
	char sql[1024] = {0};
	if(database_open()==FALSE) return FALSE;
	sprintf(sql,"delete from statictable where F_MAC='%s' and F_NAME='%s'",mac,name);
	if(sqlite3_exec(db_dev, sql,NULL,NULL,NULL)==SQLITE_OK){
		database_close();
		return TRUE;
	}
	database_close();
	return FALSE;
}
//this is for delete_dev----------------end

BOOL insertIntoDynamicTable(const char *mac,const char *ip)
{
	char sql[1024] = {0};
	if(database_open()==FALSE) return FALSE;
	sprintf(sql,"insert into dynamictable values('%s','%s')",mac,ip);
	int ret = sqlite3_exec(db_dev, sql,NULL, NULL,NULL);
	database_close();
	if(ret == SQLITE_OK)
		return TRUE;
	return FALSE;
}

BOOL isMacExistInDynamicTable(const char *mac)
{
	int row_num = 0,column_num = 0;
	char sql[1024] = {0};
	char **azResult;
	if(database_open()==FALSE) return FALSE;
	sprintf(sql,"select * from dynamictable where F_MAC='%s'",mac);
	sqlite3_get_table(db_dev,sql,  &azResult, &row_num, &column_num, NULL);
	if(row_num>0){
		sqlite3_free_table(azResult);
		database_close();
		return TRUE;
	}
	database_close();
	return FALSE;
}

BOOL UpdateDevIPByMac(const char *mac,const char *new_ip)
{
	char sql[1024] = {0};
	if(database_open()==FALSE) return FALSE;
	sprintf(sql,"update dynamictable set F_IP='%s' where F_MAC='%s'",new_ip,mac);
	if(sqlite3_exec(db_dev, sql,NULL,NULL,NULL)==SQLITE_OK)
	{
		database_close();
		return TRUE;
	}
	database_close();
	return FALSE;
}

BOOL GetIPAndDevNumberByID(const char *id,char *ip,unsigned char *devnumber)
{
	char mac[50] = {0};
	if(GetMacAndDevNumberByID(id,mac,devnumber)==FALSE)
		return FALSE;
	DEBUG_MSG("sms id = %s,mac = %s\n",id,mac);
	return GetDevIPByMac(mac,ip);
}



BOOL GetMacAndDevNumberByID(const char *id,char  *mac,unsigned char *devnumber)
{
	char **azResult;
	int row_num = 0,column_num = 0;
	char sql[1024] = {0};
	if(database_open() == FALSE) return FALSE;
	sprintf(sql,"select F_MAC,F_DEVNUMBER from statictable where F_ID='%d'",atoi(id));
	DEBUG_MSG("sql = %s\n",sql);
	sqlite3_get_table(db_dev,sql,  &azResult, &row_num, &column_num, NULL);
	if(row_num>=1){
		strcpy(mac,azResult[column_num]);
		*devnumber = atoi(azResult[column_num+1]);
		sqlite3_free_table(azResult);
		database_close();
		return TRUE;
	}
	database_close();
	return FALSE;
}

BOOL GetDevIPByMac(const char *mac,char *ip)
{
	char **azResult;
	int row_num = 0,column_num = 0;
	char sql[1024] = {0};
	if(database_open() == FALSE) return FALSE;
	sprintf(sql,"select F_IP from dynamictable where F_MAC='%s'",mac);
	sqlite3_get_table(db_dev,sql,  &azResult, &row_num, &column_num, NULL);
	if(row_num>=1){
		strcpy(ip,azResult[1]);
		sqlite3_free_table(azResult);
		database_close();
		return TRUE;
	}
	database_close();
	return FALSE;
}

unsigned char GetDevNumberByMacAndName(const char *mac,const char *name)
{
	int row_num = 0 , column_num = 0;
	unsigned char dev_number = 0;
	char sql[1024] = {0};
	char **azResult;
	if(database_open()==FALSE) return FALSE;
	sprintf(sql,"select F_DEVNUMBER from statictable where F_MAC='%s' and F_NAME='%s'",mac,name);
	sqlite3_get_table(db_dev,sql,  &azResult, &row_num, &column_num, NULL);
	if(row_num>=1){
		dev_number = atoi(azResult[1]); 
	}
	sqlite3_free_table(azResult);
	database_close();
	return dev_number;
}


BOOL isMacExistInStaticTable(const char *mac)
{
	int row_num = 0,column_num = 0;
	char sql[1024] = {0};
	char **azResult;
	if(database_open()==FALSE) return FALSE;
	sprintf(sql,"select * from statictable where F_MAC='%s'",mac);
	sqlite3_get_table(db_dev,sql,  &azResult, &row_num, &column_num, NULL);
	if(row_num>=1){
		sqlite3_free_table(azResult);
		database_close();
		return TRUE;
	}
	database_close();
	return FALSE;
}

BOOL UpdateDevNameByMacAndOldname(const char *mac,const char *oldname,const char *new_name,const char *unicode)
{
	char sql[1024] = {0};
	if(database_open()==FALSE) return FALSE;
	sprintf(sql,"update statictable set F_NAME='%s' ,F_UNICODE='%s' where F_MAC='%s' and F_NAME='%s'",new_name,unicode,mac,oldname);
	if(sqlite3_exec(db_dev, sql,NULL,NULL,NULL)==SQLITE_OK)
	{
		database_close();
		return TRUE;
	}
	database_close();
	return FALSE;
}

BOOL GetDevNameByIP(const char *ip,char *name)
{
	int row_num = 0,column_num = 0;
	char sql[1024] = {0};
	char **azResult;
	char mac[41] = {0};
	if(database_open()==FALSE) return FALSE;
	sprintf(sql,"select F_MAC from dynamictable where F_IP='%s'",ip);
	sqlite3_get_table(db_dev,sql,  &azResult, &row_num, &column_num, NULL);
	if(row_num <= 0){
		goto back;	
	}
	strcpy(mac,azResult[1]);
	sqlite3_free_table(azResult);
	
	sprintf(sql,"select F_NAME from statictable where F_MAC='%s'",mac);
	sqlite3_get_table(db_dev,sql, &azResult, &row_num, &column_num, NULL);
	if(row_num<=0)	{
		goto back;
	}
	strcpy(name,azResult[1]);
	sqlite3_free_table(azResult);
	database_close();
	return TRUE;
back:
	database_close();
	return FALSE;
}


BOOL GetDevUnicodeNameByIP(const char *ip,char *unicode_name)
{
	int row_num = 0,column_num = 0;
	char sql[1024] = {0};
	char **azResult;
	char mac[41] = {0};
	if(database_open()==FALSE) return FALSE;
	sprintf(sql,"select F_MAC from dynamictable where F_IP='%s'",ip);
	sqlite3_get_table(db_dev,sql,  &azResult, &row_num, &column_num, NULL);
	if(row_num <= 0){
		goto back;	
	}
	strcpy(mac,azResult[1]);
	sqlite3_free_table(azResult);
	
	sprintf(sql,"select F_UNICODE from statictable where F_MAC='%s'",mac);
	sqlite3_get_table(db_dev,sql, &azResult, &row_num, &column_num, NULL);
	if(row_num<=0)	{
		goto back;
	}
	strcpy(unicode_name,azResult[1]);
	sqlite3_free_table(azResult);
	database_close();
	return TRUE;
back:
	database_close();
	return FALSE;
}

