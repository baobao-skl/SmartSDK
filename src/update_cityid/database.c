#include "database.h"
#include "common.h"
#include <string.h>

static void CreateDatabaseTable(void);

sqlite3 *db = NULL;
/*
  * open database
  */
static BOOL database_open(void)
{
	int rc = SQLITE_OK;
	rc = sqlite3_open(DATABASE_NAME,&db);
	if(rc==SQLITE_ERROR){
		return FALSE;
	}
	CreateDatabaseTable();
	return TRUE;
}

static void CreateDatabaseTable(void)
{
	char *sql = "create table citytable(F_NAME VARCHAR(30),F_CityID VARCHAR(50));" ;
	sqlite3_exec(db, sql,NULL,NULL,NULL);
}

/*
  * close database
  */
static void database_close(void)
{
	if(db!=NULL)
		sqlite3_close(db);
}

BOOL insertIntoCityIDTable(const char *name,const char *cityid)
{
	char sql[1024] = {0};
	if(database_open()==FALSE) return FALSE;
	sprintf(sql,"insert into citytable values('%s','%s')",name,cityid);
	int ret = sqlite3_exec(db, sql,NULL, NULL,NULL);
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
	sqlite3_get_table(db,sql,  &azResult, &row_num, &column_num, NULL);
	if(row_num>=1){
		strcpy(cityid,azResult[1]);
		sqlite3_free_table(azResult);
		database_close();
		return TRUE;
	}
	database_close();
	return FALSE;
}

