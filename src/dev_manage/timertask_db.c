#include "timertask_db.h"
#include "include/sqlite3.h"
#include <string.h>
#include <stdlib.h>
#include "timertask.h"

sqlite3 *db_timertask = NULL;

char g_timertask_info[1024*4] = {0};

static void CreateDatabaseTable(void);

static BOOL database_open(void)
{
	int rc = SQLITE_OK;
	rc = sqlite3_open(TIMERTASK_DB_NAME,&db_timertask);
	if(rc==SQLITE_ERROR){
		return FALSE;
	}
	CreateDatabaseTable();
	return TRUE;
}
static void database_close(void)
{
	if(db_timertask!=NULL)
		sqlite3_close(db_timertask);
}

static void CreateDatabaseTable(void)
{
	char *sql = "create table timertasktable(F_ID INTEGER PRIMARY KEY AUTOINCREMENT,F_MAC VARCHAR(30),F_NAME VARCHAR(50),F_PERIOD INTEGER,F_HAPPENTIME VARCHAR(20),F_ACTION INTEGER,F_TIPINFO INTEGER,F_HOWLONG INTEGER,F_CRETIME VARCHAR(20));" ;
	sqlite3_exec(db_timertask, sql,NULL,NULL,NULL);
}

BOOL AddOneTimerTask(timertask_item_t item)
{
	char sql[1024*2] = {0};
	unsigned char weekday;
	if(database_open()==FALSE) return FALSE;
	GetCurrentTime(app_status.CurrentTime,&weekday);
	sprintf(sql,"insert into timertasktable(F_MAC,F_NAME,F_PERIOD,F_HAPPENTIME,F_ACTION,F_TIPINFO,F_HOWLONG,F_CRETIME) values('%s','%s',%d,'%s',%d,%d,%d,'%s')",item.dev_mac,item.dev_name,item.period,item.happen_time,item.action,item.tipinfo,item.howlong,app_status.CurrentTime);
	DEBUG_MSG("%s:%s\n", __FUNCTION__, sql);
	if(sqlite3_exec(db_timertask, sql,NULL, NULL,NULL) == SQLITE_OK){
		database_close();
		return TRUE;
	}
	database_close();
	return FALSE;
}

BOOL DeleteOneTimerTaskByID(int timertask_id)
{
	char sql[512] = {0};
	if(database_open()==FALSE) return FALSE;
	sprintf(sql,"delete from timertasktable where F_ID=%d",timertask_id);
	if(sqlite3_exec(db_timertask, sql,NULL, NULL,NULL) == SQLITE_OK){
		database_close();
		return TRUE;
	}
	database_close();
	return FALSE;
}

/*
	int timertask_id;
	char *dev_mac;
	char *dev_name;
	unsigned char period; // 1. 0000000 once 2. 0001 1111 3. 0111 1111 4. userdefine  
	char *happen_time;//2013-10-01 00:00
	unsigned char action; // 1. open 0. close
	unsigned int tipinfo; // (tipinfo&0xff)=tipway tipinfo&(0xFF00)=tiptime
	//((tiptime>>14) & 0x0C) 00 发生前//01发生时//02发生后
*/
void GetValidThingsToDo(const char *curr_time,const unsigned char weekday,unsigned char *nums,timertask_item_t *items)
{
	//0 & (0000000) = 0 ,1 & (000010000)
	//select * from timertasktable where F_HAPPENTIME='2014-01-28 09:00' and (F_PERIOD=0 or (F_PERIOD&(1<<6))!=0);
	if (database_open() == FALSE) return;
	if (curr_time == NULL || items == NULL) return;
	char sql[1024] = {0};
	int row_num = 0, column_num = 0, i = 0;
	char **azResult;
	const char *longtime = curr_time;
	const char *shorttime = curr_time + 11;//"short time. 2012-12-12 00:00:00"
	sprintf(sql,"select * from timertasktable where (F_HAPPENTIME='%s' and  F_PERIOD=0) or (F_HAPPENTIME='%s' and (F_PERIOD&(1<<%d))!=0);", longtime, shorttime, weekday - 1);
	DEBUG_MSG("%s ,%s\n", __FUNCTION__, sql);
	sqlite3_get_table(db_timertask, sql,  &azResult, &row_num, &column_num, NULL);
	*nums = row_num;
	if(row_num >= 1) {
		for (i = 1; i < row_num + 1; i++) {
			items[i-1].timertask_id 	= 	atoi(azResult[i*column_num]);
			strcpy(items[i-1].dev_mac , azResult[i*column_num + 1]);
			strcpy(items[i-1].dev_name , azResult[i*column_num + 2]);
			items[i-1].period		=	atoi(azResult[i*column_num + 3]);
			strcpy(items[i-1].happen_time	 , azResult[i*column_num + 4]);
			items[i-1].action		=	atoi(azResult[i*column_num + 5]);
			items[i-1].tipinfo		=	atoi(azResult[i*column_num + 6]);
			items[i-1].howlong		=	atoi(azResult[i*column_num + 7]);
			strcpy(items[i-1].create_time	, azResult[i*column_num + 8]);
			{
			extern unsigned char GetDevNumberByMacAndName(const char *mac,const char *name);
			items[i-1].devnumber	=	GetDevNumberByMacAndName(items[i-1].dev_mac, items[i-1].dev_name);
			}
		}
	}
	sqlite3_free_table(azResult);
	database_close();
}

void GetAllTimerTaskInfo(void)
{
	if(database_open()==FALSE) return;
	char buffer[1024*2] = {0};
	char *sql = "select * from timertasktable order by F_ID DESC";
	int row_num = 0,column_num = 0,i = 0;
	char **azResult;
	sqlite3_get_table(db_timertask,sql,  &azResult, &row_num, &column_num, NULL);
	memset(g_timertask_info,0,sizeof(g_timertask_info));	
	if(row_num >= 1){
		strcat(g_timertask_info,"[");
		for(i = 1;i<row_num+1;i++)
		{
			sprintf(buffer,"{\"id\":\"%s\",\"mac\":\"%s\",\"name\":\"%s\",\"period\":\"%s\",\"happentime\":\"%s\",\"action\":\"%s\",\"tipinfo\":\"%s\",\"howlong\":\"%s\",\"cretime\":\"%s\"},",
						azResult[ i*column_num],
						azResult[ i*column_num+1],
						azResult[ i*column_num+2],
						azResult[ i*column_num+3],
						azResult[ i*column_num+4],
						azResult[ i*column_num+5],
						azResult[ i*column_num+6],
						azResult[ i*column_num+7],
						azResult[ i*column_num+8]);
			strcat(g_timertask_info,buffer);
		}
		g_timertask_info[strlen(g_timertask_info)-1] = ']';
		strcat(g_timertask_info,"\0");
	}
	else{
		strcpy(g_timertask_info,"_NULL_");
	}
	DEBUG_MSG("%s :%s\n",__FUNCTION__,g_timertask_info);
	sqlite3_free_table(azResult);
	database_close();
}




