#include "version_manager.h"
#include <stdio.h>
#include <string.h>

void SetDevVersion(void)
{
	
}

void GetDevVersion(char *dev_ver)
{
	if(dev_ver == NULL)
		return;
	sprintf(dev_ver, "%s","ARM11.001.001.000.000.000");
}

