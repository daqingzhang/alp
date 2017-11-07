#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

void print_pid(void)
{
	pid_t pid, ppid;

	pid = getpid();
	ppid = getppid();

	printf("pid=%d\n", pid);
	printf("ppid=%d\n", ppid);
}

/*
 * zombie process: child is killed or terminated, but parent don't clean up it.
 * The child process become a zombie process which is marked by "defunct".
 * 
 * when parent is killed or terminated, the child maybe alive independently.
 * If the child is terminated or killed, the grand parent will clean
 * up the child.
 * 
 * parent child should call wait() to waiting for clean up the child once
 * it is killed or terminated.
 */

int main(int argc, char *argv[])
{
	int r;
	pid_t pid, subpid;

	printf("print process id\n");
	print_pid();

	subpid = fork();
	if (subpid == 0) {
		printf("this is a child process\n");
		print_pid();
		while(1);
//		pid = getppid();//kill parent
//		pid = getpid();	//kill self
//		r = kill(pid, 9);
//		printf("kill process %d, %d\n", pid, r);
	} else {
		printf("this is parent process\n");
		print_pid();
		while(1);
	}
	return 0;
}
