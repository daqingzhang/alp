#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

pthread_t h_thread1;
pthread_t h_thread2;
pthread_t h_thread3;
pthread_t h_thread4;
pthread_t h_thread5;
pthread_mutex_t mutex_lock;
sem_t sem1_cnt;

void *thread1_func(void *param)
{
	printf("thread1 enter,pid=%d\n",getpid());
	while(1) {
		sem_wait(&sem1_cnt);
		printf("THREAD1 take one signal\n");
	}
	//pthread_exit(h_thread1);
	printf("thread1 exit\n");
	return NULL;
}

void *thread2_func(void *param)
{
	printf("thread2 enter,pid=%d\n",getpid());
	while (1) {
		sem_wait(&sem1_cnt);
		printf("THREAD2 take one signal\n");
	}
	//pthread_exit(h_thread1);
	printf("thread2 exit\n");
	return NULL;
}

void *thread3_func(void *param)
{
	printf("thread3 enter,pid=%d\n",getpid());
	while (1) {
		sem_wait(&sem1_cnt);
		printf("THREAD3 take one signal\n");
	}
	//pthread_exit(h_thread1);
	printf("thread3 exit\n");
	return NULL;
}

void test_sem(void)
{
	int r;

	sem_init(&sem1_cnt, 0, 0);
	printf("init sem done\n");

	r = pthread_create(&h_thread1, NULL, thread1_func, NULL);
	r = pthread_create(&h_thread2, NULL, thread2_func, NULL);
	r = pthread_create(&h_thread3, NULL, thread3_func, NULL);
	printf("create pthread done\n");

	while (1) {
		sleep(1);
		printf("POST a signal\n");
		sem_post(&sem1_cnt);
	}
}

int thread_active;
pthread_cond_t thread_active_cv;
pthread_mutex_t thread_active_mutex;

void *thread4_func(void *param)
{
	printf("thread4 enter,pid=%d\n",getpid());
	while (1) {
		int act;
		pthread_mutex_lock(&thread_active_mutex);
		act = thread_active;
		if (!act) {
			pthread_cond_wait(&thread_active_cv, &thread_active_mutex);
			printf("THREAD4 wait cond done\n");
		}
		thread_active = 0;
		pthread_mutex_unlock(&thread_active_mutex);
		usleep(100);
		printf("THREAD4 work done\n");
	}
	printf("thread4 exit\n");
	return NULL;
}

void *thread5_func(void *param)
{
	printf("thread5 enter,pid=%d\n",getpid());
	while (1) {
		int act;
		/* acquire mutex lock */
		pthread_mutex_lock(&thread_active_mutex);
		act = thread_active;
		if (!act) {
			/* atomically release mutex lock, wait the condition, block thread;
			 * when condition is signaled, re-acquire mutex lock, resume thread;
			 */
			pthread_cond_wait(&thread_active_cv, &thread_active_mutex);
			printf("THREAD5 wait cond done\n");
		}
		thread_active = 0;
		/* release mutex lock */
		pthread_mutex_unlock(&thread_active_mutex);
		/* do some work for wait cpu time */
		usleep(500);
		printf("THREAD5 work done\n");
	}
	printf("thread5 exit\n");
	return NULL;
}

void test_cond(void)
{
	int r;
	pthread_mutex_init(&thread_active_mutex, NULL);
	pthread_cond_init(&thread_active_cv, NULL);

	r = pthread_create(&h_thread4, NULL, thread4_func, NULL);
	r = pthread_create(&h_thread5, NULL, thread5_func, NULL);
	thread_active = 0;

	printf("main thread: pid=%d\n", getpid());
	while(1) {
		sleep(1);
		pthread_mutex_lock(&thread_active_mutex);
		printf("broadcast: thread active\n");
		thread_active = 1;
		//pthread_cond_signal(&thread_active_cv);
		pthread_cond_broadcast(&thread_active_cv);
		pthread_mutex_unlock(&thread_active_mutex);
	}
}

int main(void)
{
	
	pid_t id;
	id = fork();
	if (id == 0) {
		printf("Im child process,pid=%d\n",getpid());
		test_sem();
	} else {
		printf("Im parent process,pid=%d\n",getpid());
		test_cond();
	}
	printf("%s: start,id=%d\n", __func__,id);
	while(1);
}

