#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>

int parent_exit = 0;
pid_t child_pid = 0;

int child_process(void)
{
	int pid, ppid;
	int cnt = 0;

	pid = getpid();
	ppid = getppid();

	printf("child process id %d, whose parent is %d\n", pid, ppid);

	while (1) {
		printf("child %d is running\n", pid);
		sleep(1);
		cnt++;
		if (cnt == 10)
			break;
	}
	printf("child %d exit\n", pid);
	return 0;
}

int parent_process(void)
{
	int pid;

	pid = getpid();
	printf("parent process id %d\n", pid);

	while(1) {
		printf("parent %d is running\n", pid);
		sleep(2);
		if (parent_exit)
			break;
	}
	printf("parent %d exit\n", pid);
	return 0;
}

void child_signal_process(int sig)
{
	printf("sig = %d\n", sig);

	if (sig == SIGCHLD) {
		printf("in process %d, will wait child process %d\n", getpid(), child_pid);
		waitpid(-1, 0, 0);
		//wait();
		printf("clean up child process okay\n");
		/*
		 * because of this functin executes in parent process, so the
		 * global varible can be read/write by it.
		 */
		parent_exit = 1;
	}
}

int main(int argc, char *argv[])
{
	int r;
	struct sigaction sig_child;
	pid_t pid;

	// change action of SIGCHLD
	sig_child.sa_handler = child_signal_process;
	r = sigaction(SIGCHLD, &sig_child, 0);
	if (r == -1) {
		perror("sigaction failed");
		return -1;
	}

	// new a child process
	pid = fork();
	if (pid == 0) {
		child_process();
	} else {
		child_pid = pid;
		parent_process();
	}

	return 0;
}
