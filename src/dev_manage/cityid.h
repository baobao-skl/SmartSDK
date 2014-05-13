#ifndef __CITYID_H__
#define __CITYID_H__
#include "include/sqlite3.h"
#include "dev.h"

#define DATABASE_NAME_CITYID "/usr/dev/city.db"

BOOL insertIntoCityIDTable(const char *name,const char *cityid);
BOOL GetCityIDByName(const char *name,char *cityid);


#endif
