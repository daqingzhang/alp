#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	int r;

	r = system("");
	printf("system return %d\n", r);

	r = system("uname -a");
	printf("system return %d\n", r);

	r = system("ls -l >abc.txt");
	printf("system return %d\n", r);

	r = system("gvim main.c");
	printf("system return %d\n", r);

	return 0;
}
