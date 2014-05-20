#include "alarm_db.h"
#include "dev.h"
#include <string.h>
#include "include/sqlite3.h"

static void CreateDatabaseTable(void);
static BOOL database_open(void);
static void database_close(void);

sqlite3 *db_user = NULL;
static char g_email_list[1024] = {0};
static char g_mobile_list[1024] = {0};
static char g_alarm_alllist[1024*4] = {0};

char * get_email_list(void)
{
	return g_email_list;
}

char * get_mobile_list(void)
{
	return g_mobile_list;
}

char * get_alarm_alllist(void)
{
	return g_alarm_alllist;
}
/*
  * open database
  */
static BOOL database_open(void)
{
	int rc = SQLITE_OK;
	rc = sqlite3_open(USER_DB_NAME,&db_user);
	if(rc==SQLITE_ERROR){
		return FALSE;
	}
	CreateDatabaseTable();
	return TRUE;
}

/*
  * close database
  */
static void database_close(void)
{
	if(db_user!=NULL)
		sqlite3_close(db_user);
}

static void CreateDatabaseTable(void)
{
	char *sql = "create table alarmtable(F_TYPE VARCHAR(30),F_VALUE VARCHAR(50),F_STATE VARCHAR(10));" ;
	sqlite3_exec(db_user, sql,NULL,NULL,NULL);
}



BOOL insertIntoAlarmTable(ALARM_TYPE type,const char *value)
{
	char sql[1024] = {0};
	if(database_open()==FALSE) return FALSE;
	switch(type){
		case ALARM_TYPE_EMAIL:
			DEBUG_MSG("email = %s\n",value);
			sprintf(sql,"insert into alarmtable values('email','%s','o')",value);
			break;
		case ALARM_TYPE_MOBILE:
			DEBUG_MSG("mobile = %s \n",value);
			sprintf(sql,"insert into alarmtable values('mobile','%s','o')",value);
			break;
		default:
			DEBUG_MSG("default = %s\n",value);
			sprintf(sql,"insert into alarmtable values('email','%s','o')",value);
			break;
	}
	int ret = sqlite3_exec(db_user, sql,NULL, NULL,NULL);
	database_close();
	if(ret == SQLITE_OK)
		return TRUE;
	return FALSE;
}

void GetEmailList(void)
{
	if(database_open()==FALSE) return;

	char *sql = "select F_VALUE from alarmtable where F_TYPE = 'email' and F_STATE = 'o'";
	int row_num = 0,column_num = 0,i = 0;
	char **azResult;
	sqlite3_get_table(db_user,sql,  &azResult, &row_num, &column_num, NULL);
	memset(g_email_list,0,sizeof(g_email_list));
	if(row_num >= 1){
		for(i = 1;i<row_num+1;i++)
		{
			strcat(g_email_list,azResult[ i*column_num]);
			strcat(g_email_list,",");
		}
		g_email_list[strlen(g_email_list)-1] = '\0';
		DEBUG_MSG("emailist = %s\n",g_email_list);
	}
	else{
		strcpy(g_email_list,"_NULL_");
	}
	sqlite3_free_table(azResult);
	database_close();
}

void GetMobileList(void)
{
	if(database_open()==FALSE) return;

	char *sql = "select F_VALUE from alarmtable where F_TYPE = 'mobile' and F_STATE = 'o'";
	int row_num = 0,column_num = 0,i = 0;
	char **azResult;
	sqlite3_get_table(db_user,sql,  &azResult, &row_num, &column_num, NULL);
	memset(g_mobile_list,0,sizeof(g_mobile_list));
	if(row_num >= 1){
		for(i = 1;i<row_num+1;i++)
		{
			strcat(g_mobile_list,azResult[ i*column_num]);
			strcat(g_mobile_list,",");
		}
		g_mobile_list[strlen(g_mobile_list)-1] = '\0';
		DEBUG_MSG("mobilelist = %s\n",g_mobile_list);
	}
	else{
		strcpy(g_mobile_list,"_NULL_");
	}
	sqlite3_free_table(azResult);
	database_close();
}

void GetAlarmAllList(void)
{
	if(database_open()==FALSE) return;

	char *sql = "select F_TYPE,F_VALUE,F_STATE from alarmtable";
	int row_num = 0,column_num = 0,i = 0;
	char **azResult;
	char buffer[1024] = {0};
	sqlite3_get_table(db_user,sql,  &azResult, &row_num, &column_num, NULL);
	memset(g_alarm_alllist,0,sizeof(g_alarm_alllist));
	if(row_num >= 1){
		strcat(g_alarm_alllist,"[");
		for(i = 1;i<row_num+1;i++)
		{
			//json send
			sprintf(buffer,"{\"alarm_way\":\"%s\",\"alarm_value\":\"%s\",\"alarm_state\":\"%s\"},",azResult[ i*column_num],azResult[ i*column_num+1],azResult[ i*column_num+2]);
			strcat(g_alarm_alllist,buffer);
			memset(buffer,0,sizeof(buffer));
		}
		g_alarm_alllist[strlen(g_alarm_alllist)-1] = ']';
		strcat(g_alarm_alllist,"\0");
		DEBUG_MSG("alarmAllList = %s\n",g_alarm_alllist);
	}
	else{
		strcpy(g_alarm_alllist,"_NULL_");
	}
	sqlite3_free_table(azResult);
	database_close();
}

BOOL isAlarmTypeAndValueMatch(ALARM_TYPE type,const char *value)
{
	int row_num = 0,column_num = 0;
	char sql[1024] = {0};
	char **azResult;
	if(database_open()==FALSE) return FALSE;
	switch(type)
	{
		case ALARM_TYPE_EMAIL:
			sprintf(sql,"select * from alarmtable where F_TYPE='email' and F_VALUE='%s'",value);
			break;
		case ALARM_TYPE_MOBILE:
			sprintf(sql,"select * from alarmtable where F_TYPE='mobile' and F_VALUE='%s'",value);
			break;
		default:
			return FALSE;
	}
	DEBUG_MSG("match sql = %s\n",sql);
	sqlite3_get_table(db_user,sql,  &azResult, &row_num, &column_num, NULL);
	if(row_num>0){
		sqlite3_free_table(azResult);
		database_close();
		return TRUE;
	}
	database_close();
	return FALSE;
}

BOOL DeleteByEmailOrMobile(ALARM_TYPE type,const char *value)
{
	char sql[1024] = {0};
	if(database_open()==FALSE) return FALSE;
	switch(type)
	{
		case ALARM_TYPE_EMAIL:
			sprintf(sql,"delete from alarmtable where F_TYPE='email' and F_VALUE='%s'",value);
			break;
		case ALARM_TYPE_MOBILE:
			sprintf(sql,"delete from alarmtable where F_TYPE='mobile' and F_VALUE='%s'",value);
			break;
		default:
			return FALSE;
	}
	if(sqlite3_exec(db_user, sql,NULL,NULL,NULL)==SQLITE_OK){
		database_close();
		return TRUE;
	}
	database_close();
	return FALSE;
}

BOOL SetAlarmState(ALARM_LOCK_ACTION lock_type,ALARM_TYPE type,const char *value)
{
	char sql[1024] = {0};
	if(database_open()==FALSE) return FALSE;
	switch(lock_type)
	{
		case ALARM_LOCK:
			if(type == ALARM_TYPE_EMAIL){
				sprintf(sql,"update alarmtable set F_STATE = 'c' where F_TYPE='email' and F_VALUE='%s'",value);
			}else if(type == ALARM_TYPE_MOBILE){
				sprintf(sql,"update alarmtable set F_STATE = 'c' where F_TYPE='mobile' and F_VALUE='%s'",value);
			}
			break;
		case ALARM_UNLOCK:
			if(type == ALARM_TYPE_EMAIL){
				sprintf(sql,"update alarmtable set F_STATE = 'o' where F_TYPE='email' and F_VALUE='%s'",value);
			}else if(type == ALARM_TYPE_MOBILE){
				sprintf(sql,"update alarmtable set F_STATE = 'o' where F_TYPE='mobile' and F_VALUE='%s'",value);
			}
			break;
		default:
			return FALSE;
	}
	DEBUG_MSG("sql = %s",sql);
	if(sqlite3_exec(db_user, sql,NULL,NULL,NULL)==SQLITE_OK){
		database_close();
		return TRUE;
	}
	database_close();
	return FALSE;
}

BOOL isUserMobileNumberValid(const char *mobile_number)
{
	int row_num = 0,column_num = 0;
	char sql[1024] = {0};
	char **azResult;
	if(database_open()==FALSE) return FALSE;
	sprintf(sql,"select * from alarmtable where F_TYPE='mobile' and F_VALUE='%s' and F_STATE='o'",mobile_number);
	sqlite3_get_table(db_user,sql,  &azResult, &row_num, &column_num, NULL);
	if(row_num>0){
		sqlite3_free_table(azResult);
		database_close();
		return TRUE;
	}
	database_close();
	return FALSE;
}


