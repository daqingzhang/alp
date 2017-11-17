#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <signal.h>

static void sigint_process(int sig)
{
	printf("sig %d cannot kill me.\n", sig);
}

int main(int argc, char *argv[])
{
#if 0
	__sighandler_t ret;

	ret = signal(SIGINT, sigint_process);
	printf("ret=%p\n", ret);
#else
	struct sigaction act;
	int r;

	act.sa_handler = sigint_process,
	r = sigaction(SIGINT, &act, 0);
//	r = sigaction(SIGKILL, &act, 0);
	if (r == -1) {
		perror("sigaction failed\n");
	}
#endif
	while(1) {
		printf("pid=%d, hello world\n", getpid());
		pause();//suspend current process until one signal comes
	}
	return 0;
}
