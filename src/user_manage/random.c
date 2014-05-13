#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int GetRandom(void)
{
	srand(time(NULL));
	return rand();
}
