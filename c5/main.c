#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>

int shm_reader(int shm_id, int shm_size)
{
	char *shm_buf;
	int r;

	printf("%s, shm_id=%d, shm_size=%d\n", __func__, shm_id, shm_size);

	// attach shm
	shm_buf = shmat(shm_id, NULL, 0);
	printf("%s, shm_buf=%p\n", __func__, shm_buf);

	printf("%s", shm_buf);

	// dettach shm
	r = shmdt((const void *)shm_buf);
	if (r < 0) {
		printf("shmdt failed %d\n", r);
		return -1;
	}
	return 0;
}

int shm_writer(int shm_id, int shm_size)
{
	char *shm_buf;
	int r;

	printf("%s, shm_id=%d, shm_size=%d\n", __func__, shm_id, shm_size);

	shm_buf = shmat(shm_id, NULL, 0);
	printf("%s, shm_buf=%p\n", __func__, shm_buf);

	memset(shm_buf, 0, shm_size);

	sprintf(shm_buf, "%s\n", "helloworld");

	r = shmdt((const void *)shm_buf);
	if (r < 0) {
		printf("shmdt failed %d\n", r);
		return -1;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	int shm_id;
	int shm_size = 0x1000;
	pid_t pid;

	// create shm
	shm_id = shmget(IPC_PRIVATE, shm_size, IPC_CREAT | IPC_EXCL | S_IRUSR |S_IWUSR);
	if (shm_id < 0) {
		printf("shmget failed %d\n", shm_id);
		return -1;
	}
	printf("shm_id=%d\n", shm_id);

	// fork process
	pid = fork();
	if (pid == 0) {
		printf("sub process %d is running\n", (int)getpid());
		sleep(1);
		shm_reader(shm_id, shm_size);
	} else {
		int status, r;
		pid_t child_id;

		printf("main process %d is running\n", (int)getpid());
		shm_writer(shm_id, shm_size);

		child_id = wait(&status);
		printf("child process %d ended\n", child_id);

		// destroy shm
		r = shmctl(shm_id, IPC_RMID, 0);
		if (r) {
			printf("destroy shm failed %d\n", r);
		}
		while(1);
	}
	return 0;
}
