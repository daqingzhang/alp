#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>

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
 *
 * wait() will suspend parent process until child process exits. It returns
 * the child's pid
 */

int main(int argc, char *argv[])
{
	int status;
	pid_t subpid, child_pid;

	printf("print process id\n");
	print_pid();

	subpid = fork();
	if (subpid == 0) {
		printf("this is a child process\n");
		print_pid();
		//while(1);
		sleep(3);
	} else {
		printf("this is parent process\n");
		print_pid();
		//while(1);

		/*
		 * parent process invoke wait() to cleanup terminated child process
		 * And wait() will make parent blocked until child is killed or exited.
		 * If parent process dont cleanup a terminated child process, it will be
		 * zombie process.
		 */

		child_pid = wait(&status);
		if (WIFEXITED(status)) {
			printf("child process %d exit normally\n", child_pid);
		} else {
			printf("child process %d exit abnormally\n", child_pid);
		}
		printf("child process exit status = %x\n" ,WEXITSTATUS(status));
	}
	return 0;
}
