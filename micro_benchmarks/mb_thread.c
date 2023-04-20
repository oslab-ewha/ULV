#include <pthread.h>
#include <stdio.h>

pthread_mutex_t	mutex;
pthread_cond_t	cond;

static int	thread_started;
static int	thread_stopped;

static void *
thread_func(void *arg)
{
	pthread_mutex_lock(&mutex);
	thread_started = 1;
	pthread_cond_broadcast(&cond);
	pthread_mutex_unlock(&mutex);

	pthread_mutex_lock(&mutex);
	while (!thread_stopped)
		pthread_cond_wait(&cond, &mutex);
	pthread_mutex_unlock(&mutex);

	printf("thread done\n");

	return NULL;
}

int
main(int argc, char *argv[])
{
	pthread_t	thread;
	void	*ret;

	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);

	pthread_create(&thread, NULL, thread_func, NULL);

	pthread_mutex_lock(&mutex);
	while (!thread_started)
		pthread_cond_wait(&cond, &mutex);
	pthread_mutex_unlock(&mutex);

	pthread_mutex_lock(&mutex);
	thread_stopped = 1;
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);

	pthread_join(thread, &ret);

	printf("thread joined\n");
	return 0;
}
