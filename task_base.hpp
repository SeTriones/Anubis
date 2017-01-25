#ifndef QO_TASK_BASE_HPP
#define QO_TASK_BASE_HPP

#include <pthread.h>

class task_base {
	public:
		task_base();
		virtual ~task_base();

		virtual int open(size_t thread_num, size_t stack_size);
		virtual int activate();
		virtual int stop() = 0;
		virtual int svc() = 0;
		virtual int join();		

	private:
		static void* run_svc(void *arg);

	protected:
		pthread_t *m_thread;
		size_t m_thread_num;
		pthread_barrier_t m_barrier;		
};

#endif //QO_TASK_BASE_HPP

