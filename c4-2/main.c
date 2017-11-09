#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <malloc.h>
#include <semaphore.h>

struct th_data {
	pthread_t id;
	int cmd;
	int cnt;
	int stop;
	pthread_mutex_t mutex;
	sem_t sem;
};

void *thread_consumer_a(void *data)
{
	struct th_data *pd = data;
	volatile int stop = 0;
	pthread_t ptid;

	printf("%s, process id=%d\n", __func__, (int)getpid());
	ptid = pthread_self();

	stop = pd->stop;
	while(!stop) {
		sem_wait(&pd->sem);
		printf("%s, thread %d take 1 sema\n", __func__, (int)ptid);
		stop = pd->stop;
	}
	printf("%s, stoped\n", __func__);
	return NULL;
}

void *thread_consumer_b(void *data)
{
	struct th_data *pd = data;
	volatile int stop = 0;
	pthread_t ptid;

	printf("%s, process id=%d\n", __func__, (int)getpid());
	ptid = pthread_self();

	stop = pd->stop;
	while(!stop) {
		sem_wait(&pd->sem);
		printf("%s, thread %d take 1 sema\n", __func__, (int)ptid);
		stop = pd->stop;
	}
	printf("%s, stoped\n", __func__);
	return NULL;
}

void *thread_producer(void *data)
{
	struct th_data *pd = data;
	volatile int stop = 0;
	pthread_t ptid;

	printf("%s, process id=%d\n", __func__, (int)getpid());
	ptid = pthread_self();

	stop = pd->stop;
	while(!stop) {
		printf("%s, thread %d post 1 sema\n", __func__, (int)ptid);
		sem_post(&pd->sem);

		pd->cnt--;
		if(!pd->cnt)
			pd->stop = 1;
		stop = pd->stop;
	}
	printf("%s, stoped\n", __func__);
	return NULL;
}

typedef void *(*pthread_func_t)(void *);

#define THREAD_NUM 3

pthread_func_t func_list[THREAD_NUM] = {
	thread_consumer_a,
	thread_consumer_b,
	thread_producer
};

int main(int argc, char *argv[])
{
	int r,i;
	struct th_data *ptd;
	pthread_t subid[3];
	pthread_attr_t *attr;

	printf("%s, process id=%d, thread id=%d\n", __func__,
		(int)getpid(), (int)pthread_self());

	ptd = malloc(sizeof(struct th_data));
	if (!ptd) {
		printf("malloc ptd failed\n");
		return -1;
	}

	pthread_mutex_init(&ptd->mutex, NULL);
	sem_init(&ptd->sem, 0, 1);
	ptd->id  = 0;
	ptd->cnt = 10;
	ptd->cmd = 1;
	ptd->stop = 0;

	attr = malloc(sizeof(pthread_attr_t));
	if (!attr) {
		printf("malloc attr failed\n");
		free(ptd);
		return -1;
	}

	pthread_attr_init(attr);

	for(i = 0;i < THREAD_NUM;i++) {
		r = pthread_create(&subid[i], attr, func_list[i], (void *)ptd);
		if (r) {
			printf("create thread failed %d\n", r);
			return -1;
		}
		printf("thread %d created\n", (int)subid[i]);
	}

	for(i = 0;i < THREAD_NUM;i++) {
		r = pthread_join(subid[i], NULL);
		if (r)
			printf("join thread %d failed %d\n", (int)subid[i], r);
	}

	free(ptd);

	return 0;
}
