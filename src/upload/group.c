#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GROUP_INFO_STORE_PATH "/usr/dev/family_info.ini"
#define BOOL unsigned char
#define TRUE 1
#define FALSE 0

BOOL GetGroupID(char *group_id)
{
	FILE *fid = fopen(GROUP_INFO_STORE_PATH,"r");
	if(fid==NULL) return FALSE;
	char *line = NULL;
	size_t len;
	size_t size;
	size = getline(&line,&len,fid);
	if(size==-1)
		goto fail;
	line[strlen(line)-2] = '\0';
	printf("--%s--\n",line);
	strcpy(group_id,line);
	free(line);
	fclose(fid);
	return TRUE;
fail:
	fclose(fid);
	return FALSE;
}


