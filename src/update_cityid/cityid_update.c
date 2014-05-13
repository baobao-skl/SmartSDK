#include "cityid_update.h"
#include "common.h"
#include <stdlib.h>
#include <string.h>
#include "database.h"

BOOL  UpdateCityID(void)
{
	FILE *fd = fopen(CITY_NAME_FILE_PATH,"r");
	char *line = NULL;
	ssize_t read;
	size_t len = 0;
	if(!fd){DEBUG_MSG("open "CITY_NAME_FILE_PATH" error!\n");return FALSE;}

	int count = 0;

	while ((read = getline(&line, &len, fd)) != -1) {
		char *id = strtok(line,"=");
		char *name = strtok(NULL,"=");
		name[strlen(name)-2] = '\0';
		if(name != NULL && id != NULL){
			insertIntoCityIDTable(name, id);
			DEBUG_MSG("%d\n",count++);
		}
	}
	DEBUG_MSG("ADD OK!\n");
	if(line)free(line);
	fclose(fd);
	return TRUE;
}

