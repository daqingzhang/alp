#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <malloc.h>

struct th_data {
	pthread_t id;
	int cmd;
	int cnt;
};

void *thread_deadloop(void *data)
{
	volatile int stop = 0;
	int tick = 0;

	while(!stop) {
		sleep(1);
		printf("%s, tick %d\n", __func__, tick++);
	}
	printf("%s, stoped\n", __func__);
	pthread_exit((void *)NULL);
}

void *thread_consumer(void *data)
{
	struct th_data *pd = data;
	volatile int stop = 0;
	int cmd;

	printf("%s, process id=%d, thread id=%d\n", __func__,
		(int)getpid(), (int)pthread_self());

	while(!stop) {
		sleep(1);
		cmd = pd->cmd;
		printf("%s, cmd=%d, %d\n", __func__, cmd, pd->cnt);

		pd->cnt--;
		if (!pd->cnt)
			stop = 1;
	}
	printf("%s, stoped\n", __func__);
//	return NULL;
	pthread_exit((void *)NULL);
}

int main(int argc, char *argv[])
{
	int r;
	struct th_data *ptd;
	pthread_t subid;
	pthread_attr_t *attr;

	printf("%s, process id=%d, thread id=%d\n", __func__,
		(int)getpid(), (int)pthread_self());

	/*
	 * Using malloc() to allocate varibles locate in heap, not in stack.
	 * If varibles are declared in main(), it will be located in stack
	 * and the stack will be destroyed while main thread terminates.
	 * This situation is a potential BUG for the sub-thread which maybe
	 * using the varibles.
	 */

	ptd = malloc(sizeof(struct th_data));
	if (!ptd) {
		printf("malloc ptd failed\n");
		return -1;
	}

	ptd->id  = 0;
	ptd->cnt = 10;
	ptd->cmd = 1;

	attr = malloc(sizeof(pthread_attr_t));
	if (!attr) {
		printf("malloc attr failed\n");
		free(ptd);
		return -1;
	}

	pthread_attr_init(attr);

	r = pthread_create(&ptd->id, attr, thread_consumer, (void *)ptd);
	if (r) {
		printf("create thread failed %d\n", r);
		return -1;
	}
	printf("thread %d created\n", (int)(ptd->id));

	r = pthread_create(&subid, NULL, thread_deadloop, NULL);
	if (r) {
		printf("create thread failed %d\n", r);
		return -1;
	}
	printf("thread %d created\n", (int)subid);

	/*
	 * pthread_join() will make calling thread suspend until
	 * the joined thread terminates normally. It will release
	 * resources of the joined thread.
	 */
	r = pthread_join(ptd->id, NULL);
	if (r) {
		printf("join thread %d failed %d\n", (int)(ptd->id), r);
	}

	r = pthread_cancel(subid);
	if (r) {
		printf("cancel thread %d failed %d\n", (int)subid, r);
	}

	r = pthread_join(subid, NULL);
	if (r) {
		printf("join thread %d failed %d\n", (int)subid, r);
	}

	free(ptd);

	return 0;
}
