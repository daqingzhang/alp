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
	int slot[2];
};

void *thread_consumer_a(void *data)
{
	struct th_data *pd = data;
	volatile int stop = 0;

	printf("%s, process id=%d\n", __func__, (int)getpid());

	stop = pd->stop;
	while(!stop) {
		sem_wait(&pd->sem);
		printf("%s, take a sem\n", __func__);
		stop = pd->stop;
		usleep(pd->slot[0]);
	}
	printf("%s, stoped\n", __func__);
	return NULL;
}

void *thread_consumer_b(void *data)
{
	struct th_data *pd = data;
	volatile int stop = 0;

	printf("%s, process id=%d\n", __func__, (int)getpid());

	stop = pd->stop;
	while(!stop) {
		sem_wait(&pd->sem);
		printf("%s, take a sem\n", __func__);
		stop = pd->stop;
		usleep(pd->slot[0]);
	}
	printf("%s, stoped\n", __func__);
	return NULL;
}

static void gen_thread_slot(struct th_data *td)
{
#define THREAD_MAX_SLOT 50

	td->slot[0] = rand() % THREAD_MAX_SLOT;
	td->slot[1] = rand() % THREAD_MAX_SLOT;
}

void *thread_producer_c(void *data)
{
	struct th_data *pd = data;
	volatile int stop = 0;

	printf("%s, process id=%d\n", __func__, (int)getpid());

	stop = pd->stop;
	while(!stop) {
		sleep(1);
		gen_thread_slot(pd);
		printf("%s, post a sem\n", __func__);
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

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

pthread_func_t func_list[] = {
	thread_consumer_a,
	thread_consumer_b,
	thread_producer_c
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
	gen_thread_slot(ptd);

	attr = malloc(sizeof(pthread_attr_t));
	if (!attr) {
		printf("malloc attr failed\n");
		free(ptd);
		return -1;
	}

	pthread_attr_init(attr);

	for(i = 0;i < ARRAY_SIZE(func_list);i++) {
		r = pthread_create(&subid[i], attr, func_list[i], (void *)ptd);
		if (r) {
			printf("create thread failed %d\n", r);
			return -1;
		}
		printf("thread %d created\n", (int)subid[i]);
	}

	// waiting producer thread terminated
	pthread_join(subid[2], NULL);

	// kill consumer thread
	pthread_cancel(subid[1]);
	pthread_cancel(subid[0]);

	// waiting consumer thread
	pthread_join(subid[1], NULL);
	pthread_join(subid[0], NULL);

	free(ptd);

	return 0;
}
