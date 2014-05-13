#include "cityid.h"
#include "dev.h"
#include <string.h>

static void CreateDatabaseTable(void);
static BOOL database_open(void);
static void database_close(void);

sqlite3 *db_cityid = NULL;
/*
  * open database
  */
static BOOL database_open(void)
{
	int rc = SQLITE_OK;
	rc = sqlite3_open(DATABASE_NAME_CITYID,&db_cityid);
	if(rc==SQLITE_ERROR){
		return FALSE;
	}
	CreateDatabaseTable();
	return TRUE;
}

static void CreateDatabaseTable(void)
{
	char *sql = "create table citytable(F_NAME VARCHAR(30),F_CityID VARCHAR(50));" ;
	sqlite3_exec(db_cityid, sql,NULL,NULL,NULL);
}

/*
  * close database
  */
static void database_close(void)
{
	if(db_cityid!=NULL)
		sqlite3_close(db_cityid);
}

BOOL insertIntoCityIDTable(const char *name,const char *cityid)
{
	char sql[1024] = {0};
	if(database_open()==FALSE) return FALSE;
	sprintf(sql,"insert into citytable values('%s','%s')",name,cityid);
	int ret = sqlite3_exec(db_cityid, sql,NULL, NULL,NULL);
	database_close();
	if(ret == SQLITE_OK)
		return TRUE;
	return FALSE;
}

BOOL GetCityIDByName(const char *name,char *cityid)
{
	char **azResult;
	int row_num = 0,column_num = 0;
	char sql[1024] = {0};
	if(database_open() == FALSE) return FALSE;
	sprintf(sql,"select F_CityID from citytable where F_NAME='%s'",name);
	sqlite3_get_table(db_cityid,sql,  &azResult, &row_num, &column_num, NULL);
	if(row_num>=1){
		strcpy(cityid,azResult[1]);
		sqlite3_free_table(azResult);
		database_close();
		return TRUE;
	}
	database_close();
	return FALSE;
}

