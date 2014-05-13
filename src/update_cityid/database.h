#ifndef __DATABASE_H__
#define __DATABASE_H__
#include "common.h"
#include "include/sqlite3.h"

#define DATABASE_NAME_CITYID "/usr/dev/city.db"

BOOL insertIntoCityIDTable(const char *name,const char *cityid);
BOOL GetCityIDByName(const char *name,char *cityid);


#endif
