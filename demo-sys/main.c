#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include <ncurses.h>

void print_local_time(void)
{
	static time_t t1;
	static struct tm *pt;
	static int tick = 0;
	int i = 0, n;

	time(&t1);
	pt = localtime(&t1);

	n = 4+2+2+2+2+2+8;
	if (tick > 0) {
		for (i = 0; i < n; i++) {
			printf("\b");
		}
	}
	printf("[%04d,%02d,%02d][%02d:%02d:%02d]",
		pt->tm_year+1900,pt->tm_mon, pt->tm_mday, pt->tm_hour, pt->tm_min, pt->tm_sec);

	tick++;
	fflush(stdout);
}

void timer_handler(int signum)
{
#if 0
	static int tick = 0;
		for (int i = 0; i < tick + (10-1) / 10; i++) {
			printf("\b");
		}
		printf("%d",tick);
		tick++;
		fflush(stdout);
#else
	print_local_time();
#endif
}

int test_timer(void)
{
#define MS_TO_US(ms) (ms*1000)
	int r = 0;
	struct itimerval timer;
	struct sigaction timer_sa;

	printf("%s:\n", __func__);

	memset(&timer_sa, 0, sizeof(timer_sa));
	timer_sa.sa_handler = timer_handler;
	sigaction(SIGALRM, &timer_sa, NULL);

	timer.it_interval.tv_sec  = 1;
	timer.it_interval.tv_usec = MS_TO_US(100);
	timer.it_value.tv_sec     = timer.it_interval.tv_sec;
	timer.it_value.tv_usec    = timer.it_interval.tv_usec;

	r = setitimer(ITIMER_REAL, &timer, NULL);
	if (r) {
		printf("%s: setitimer failed %d", __func__, r);
	}
	while(1);
	return r;
}

int test_readlink(int argc, char *argv[])
{
	char *path;
	char str[256];
	int r;

	memset(str, 0x0, sizeof(str));

	if (argc > 1)
		path = argv[1];
	else
		return -1;

	r = readlink(path, str, sizeof(str));
	if (r < 0) {
		perror("readlink error");
		return -1;
	}
	printf("link %s = %s\n", path, str);

	return 0;
}

int test_access(int argc, char *argv[])
{
	int r;
	char *file_dir;

	// get file directory
	if (argc > 1)
		file_dir = argv[1];
	else
		file_dir = "./a.out";

	// existence
	r = access(file_dir, F_OK);
	if (!r) {
		printf("file %s exists\n", file_dir);
	} else {
		printf("not find %s\n", file_dir);
		return 0;
	}

	// check readable
	r = access(file_dir, R_OK);
	if (!r)
		printf("file %s is readable\n", file_dir);
	else
		printf("file %s is not readable\n", file_dir);

	// check writable
	r = access(file_dir, W_OK);
	if (!r)
		printf("file %s is writable\n", file_dir);
	else
		printf("file %s is not writable\n", file_dir);

	// check executable
	r = access(file_dir, X_OK);
	if (!r)
		printf("file %s is executable\n", file_dir);
	else
		printf("file %s is not executable\n", file_dir);

	return 0;
}

int main(int argc, char *argv[])
{
//	test_readlink(argc, argv);
//	test_access(argc, argv);
	test_timer();
	return 0;
}
