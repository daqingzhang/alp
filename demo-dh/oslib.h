#ifndef __OSLIB_H__
#define __OSLIB_H__
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <osdbg.h>

int os_thread_mutex_init(pthread_mutex_t *mutex);
int os_thread_attr_init(pthread_attr_t *attr);

int os_thread_create(pthread_t *pid, pthread_attr_t *attr,
			void* (*func)(void *data), void *data);
int os_thread_destroy(pthread_t thread, void **retval);

int os_sem_init(sem_t *sem, int pshared, unsigned int value);
int os_sem_wait(sem_t *sem);
int os_sem_post(sem_t *sem);
int os_sem_timedwait(sem_t *sem, const struct timespec *abs_tm);

#endif
