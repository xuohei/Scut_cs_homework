#include<stdio.h>
#include<time.h>
void main()
{
	while(1)
	{
		time_t rawtime;
		struct tm * timeinfo;
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
		printf("Those output come from child\n");
		printf ( "current time: %s\n", asctime (timeinfo) );
		sleep(1);
	}
}
