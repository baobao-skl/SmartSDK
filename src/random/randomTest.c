#include <stdio.h>
#include <time.h>
#include <stdlib.h>
void main()
{
	srand(time(NULL));
	printf("%d\n",rand());
}