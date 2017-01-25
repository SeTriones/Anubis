#include "task_base.hpp"
#include <signal.h>
#include <stdlib.h>

task_base::task_base(): m_thread(NULL), m_thread_num(0)
{
}

task_base::~task_base()
{
	join();
}

int task_base::open(size_t thread_num, size_t stack_size)
{
	int ret = -1;
	size_t i;
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	do {
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
		if (pthread_attr_setstacksize(&attr, stack_size))
			break;

		if (thread_num == 0 || (m_thread = (pthread_t*)malloc(thread_num * sizeof(pthread_t))) == NULL)
			break;

		pthread_barrier_init(&m_barrier, NULL, thread_num + 1);
		for (i=0; i<thread_num; i++)
			if (pthread_create(m_thread+i, &attr, run_svc, this))
				break;

		if ((m_thread_num = i) != thread_num)
			break;

		ret = 0;
	} while (false);

	pthread_attr_destroy(&attr);
	return ret;
}

int task_base::activate()
{
	pthread_barrier_wait(&m_barrier);
	return 0;
}

int task_base::join()
{
	if (m_thread) {
		for (size_t i=0; i<m_thread_num; i++) {
			pthread_kill(m_thread[i], SIGTERM);
			pthread_join(m_thread[i], NULL);
		}
		free(m_thread);
		m_thread = NULL;
		pthread_barrier_destroy(&m_barrier);
	}
	return 0;
}

void* task_base::run_svc(void *arg)
{
	task_base *task = (task_base *)arg;
	pthread_barrier_wait(&task->m_barrier);
	task->svc();
	return NULL;
}


