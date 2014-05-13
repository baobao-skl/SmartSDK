#include <internet_shortcut.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

typedef signed char BOOL;
#define TRUE (BOOL)1
#define FALSE (BOOL)0

static BOOL isInternetShortCutExistAndUnModified(const char *filepath,const char *url);
static void CreateInternetShortCut(const char *filepath,const char *url);

#define INTERNET_SHORTCUT_CONTENT	"[{000214A0-0000-0000-C000-000000000046}]\r\n" \
																	"Prop3=19,2\r\n" \
																	"[InternetShortcut]\r\n" \
																	"URL=%s\r\n" \
																	"IDList=\r\n"
#define MAX_LENGTH 512

#define ATTR_RDONLY             (0x0001uL)          /* read only    */
#define ATTR_HIDDEN             (0x0002uL)          /* hidden       */
#define ATTR_SYSTEM             (0x0004uL)          /* system       */
#define ATTR_VOLUME             (0x0008uL)          /* volume label */
#define ATTR_DIR                (0x0010uL)          /* subdirectory */
#define ATTR_ARCH               (0x0020uL)          /* archives     */
#define ATTR_NONE               (0x0040uL)          /* none         */
#define ATTR_ALL                (0x007FuL)          /* for fsfirst function */
#define ATTR_NOT_DIR            (0x0100uL)          /* except for directory and volume lavel */

#define ATTR_CAND               (0x0080uL)          /* AND compare mode     */
#define ATTR_CMATCH             (0x1000uL)          /* MATCH compare mode   */
#define ATTR_ADD                (0x2000uL)          /* Attribute add mode   */
#define ATTR_SUB                (0x4000uL)          /* Attribute del mode   */


int main(void)
{
	InternetShortCutCheck();
	return 0;
}

/*
 * function name: InternetShortCutCheck
 * function: check InternetShortCut. Create it if not exist when SD is inserted
*/
void InternetShortCutCheck(void)
{
	if(isInternetShortCutExistAndUnModified(INTERNET_SHORTCUT_PATH,INTERNET_SHORTCUT_URL)){
		return;
	}
	CreateInternetShortCut(INTERNET_SHORTCUT_PATH,INTERNET_SHORTCUT_URL);
}

/*
 * function name: InternetShortCutRemove
 * function: remove InternetShortCut in SD Card
*/
int InternetShortCutRemove(void)
{
	int rval = 0;
	struct stat sstat;
	rval = stat(INTERNET_SHORTCUT_PATH, &sstat);
	if(rval == 0){
		chmod(INTERNET_SHORTCUT_PATH, ATTR_ARCH);
		return remove(INTERNET_SHORTCUT_PATH);
	}
	return 0;
}

/*
 * function name: isInternetShortCutExistAndUnModified
 * function: check whether InternetShortCut exist or modified
*/
static BOOL isInternetShortCutExistAndUnModified(const char *filepath,const char *url)
{
	char read_buffer[MAX_LENGTH] = {0};
	char url_to_comp[MAX_LENGTH] = {0};
	FILE *fd;

	if(filepath == NULL || url == NULL)
		return FALSE;
	fd =  fopen(filepath, "r");
	//file not exist
	if(fd == NULL)
		return FALSE;
	//if exist. verify the content is right
	fread(read_buffer,1,sizeof(read_buffer),fd);
	sprintf(url_to_comp,INTERNET_SHORTCUT_CONTENT,url);
	if(strcmp(url_to_comp,read_buffer)!=0){//modified
		fclose(fd);
		remove(filepath);
		return FALSE;
	}
	fclose(fd);
	return TRUE;
}

/*
 * function name: CreateInternetShortCut
 * function: Create Internet ShortCut in sd card
*/
static void CreateInternetShortCut(const char *filepath,const char *url)
{
	char str_to_write[MAX_LENGTH] = {0};
	FILE *fd;

	if(filepath == NULL || url == NULL)
		return;
	sprintf(str_to_write,INTERNET_SHORTCUT_CONTENT,url);
	if(strlen(str_to_write) > MAX_LENGTH)
		return;
	fd =  fopen(filepath, "w");
	if(fd == NULL)
		return;
	fwrite(str_to_write,1,strlen(str_to_write),fd);
	fclose(fd);
	chmod(filepath,ATTR_RDONLY | ATTR_ARCH);
}


