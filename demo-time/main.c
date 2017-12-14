#include <stdio.h>
#include <time.h>

int main(int argc, const char *argv[])
{
	time_t tim, tim2;
	struct tm *pt;
	char *ps;

	time(&tim);

	pt = gmtime(&tim);
	printf("GMT: %d-%d-%d, %d:%d:%d\n",
		pt->tm_mon,pt->tm_mday, pt->tm_year,
		pt->tm_hour, pt->tm_min,pt->tm_sec);

	pt = localtime(&tim);
	printf("LOT: %d-%d-%d, %d:%d:%d\n",
		pt->tm_mon,pt->tm_mday, pt->tm_year,
		pt->tm_hour, pt->tm_min,pt->tm_sec);

	tim2 = mktime(pt);
	printf("tim=%ld, tim2=%ld\n", tim, tim2);

	ps = ctime(&tim);
	printf("local time: %s\n", ps);

	ps = asctime(pt);
	printf("local time2: %s\n", ps);

	return 0;
}
