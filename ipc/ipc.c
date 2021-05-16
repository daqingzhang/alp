#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <string.h>
#include <signal.h>

int shm_id = 0;
unsigned int shm_size = 0;
struct shmid_ds shm_info;

#define XSIG SIGUSR1
static volatile pid_t child_pid = 0;
static volatile int recv_signal = 0;

static void user_signal_handler(int signal)
{
	printf("%s: signal=%d\n", __func__, signal);
	recv_signal = signal;
}

void user_signal_init(void (*sig_handler)(int signal))
{
	int r;
	struct sigaction act;

	act.sa_handler = sig_handler;
	r = sigaction(XSIG, &act, 0);
	if (r == -1) {
		printf("%s: sigaction failed", __func__);
	}
	printf("%s: done\n", __func__);
}

void user_signal_post(pid_t pid)
{
	printf("%s: pid=%d\n", __func__, (int)pid);
	kill(pid, XSIG);
}

int user_signal_wait(void)
{
	int signal = 0;

	if (recv_signal == 0) {
		printf("suspend process(%d)\n", getpid());
#if 1
		pause();
#else
		while(recv_signal == 0) {
			usleep(1000*10);
		};
#endif
		printf("wake up process(%d)\n", getpid());
	}

	signal = recv_signal;
	recv_signal = 0;
	printf("wait signal(%d) done\n", signal);
	return signal;
}

void send_message(unsigned char *addr)
{
	const char *s1 = "parent: hello child";
	const char *s2 = "parent: I am parent process";
	static int tick = 0;

	sleep(1);
	while(1) {
		usleep(1000*40);
		tick++;
		printf("%s: process(%d): addr=%p, tick=%d\n",
			__func__, getpid(), addr, tick);
		memset(addr, 0, 1024);
		if (tick % 2 == 0) {
			memcpy(addr, s1, strlen(s1));
		} else {
			memcpy(addr, s2, strlen(s2));
		}
		user_signal_post(child_pid);
	}
}

void recv_message(unsigned char *addr)
{
	static int tick = 0;
	char s1[32];
	int sig = 0;

	user_signal_init(user_signal_handler);

	while (1) {
		tick++;
		sig = user_signal_wait();
		printf("%s: process=%d: addr=%p, tick=%d, sig=%d\n",
			__func__, getpid(), addr, tick, sig);
		if (sig > 0) {
			memcpy(s1, addr, 1024);
			printf("message: %s\n", s1);
		}
	}
}

int main(void)
{
	shm_id = shmget(IPC_PRIVATE, 1024*4, IPC_CREAT);
	if (shm_id == -1) {
		printf("shmget failed %d\n", shm_id);
		perror(NULL);
		return 0;
	}
	shmctl(shm_id, IPC_STAT, &shm_info);
	shm_size = shm_info.shm_segsz;
	printf("shm_id = %d, shm_size=%d\n", shm_id, shm_size);

	child_pid = fork();
	if (child_pid == 0) {
		int r;
		unsigned char *shm_addr = NULL;

		printf("child: pid=%d,shm_id=%d\n", getpid(), shm_id);
		shm_addr = (unsigned char *)shmat(shm_id, NULL,SHM_RND);
		if ((int)shm_addr == -1) {
			printf("shmat failed\n");
			perror("child");
			goto _end;
		}
		recv_message(shm_addr);
		r = shmdt((const void *)shm_addr);
		if (r) {
			printf("parent: shmdt failed\n");
			perror("parent");
			goto _end;
		}
	} else {
		int r;
		unsigned char *shm_addr = NULL;

		printf("parent: pid=%d,shm_id=%d\n", getpid(), shm_id);
		shm_addr = (unsigned char *)shmat(shm_id, NULL,SHM_RND);
		if ((int)(shm_addr) == -1) {
			printf("shmat failed\n");
			perror("parent");
			goto _end;
		}

		send_message(shm_addr);
		r = shmdt((const void *)shm_addr);
		if (r) {
			printf("parent: shmdt failed\n");
			perror("parent");
			goto _end;
		}
		r = shmctl(shm_id, IPC_RMID, 0);
	}
_end:
	printf("procss(%d), stopped\n",getpid());
	return 0;
}

