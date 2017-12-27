#include <oslib.h>

int os_thread_mutex_init(pthread_mutex_t *mutex)
{
	return pthread_mutex_init(mutex, NULL);//not re-entrant
}

int os_thread_attr_init(pthread_attr_t *attr)
{
	return pthread_attr_init(attr);
}

int os_thread_create(pthread_t *pid, pthread_attr_t *attr,
		void* (*func)(void *data), void *data)
{
	int r;

	r = pthread_create(pid, attr, func, data);
	if (r) {
		DBG("%s, create thread failed %d\n", __func__, r);
		return r;
	}
	return 0;
}

int os_thread_destroy(pthread_t thread, void **retval)
{
	return pthread_join(thread, retval);
}

int os_sem_init(sem_t *sem, int pshared, unsigned int value)
{
	int r;

	r = sem_init(sem, pshared, value);
	if (r) {
		DBG("%s, init sem failed %d\n", __func__, r);
		return r;
	}
	return 0;
}

int os_sem_wait(sem_t *sem)
{
	return sem_wait(sem);
}

int os_sem_post(sem_t *sem)
{
	return sem_post(sem);
}

int os_sem_timedwait(sem_t *sem, const struct timespec *abs_tm)
{
	return sem_timedwait(sem, abs_tm);
}
