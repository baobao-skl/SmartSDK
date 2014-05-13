#include "dev.h"
#include "dev_del.h"
#include "dev_db.h"

BOOL RemoveDevFromStaticTable(const char *mac,const char *name)
{
	//check mac and name are match in database
	if(isMacAndNameMatch(mac,name)==FALSE){
		return FALSE;
	}
	return deleteFromStaticTable(mac, name);
}



