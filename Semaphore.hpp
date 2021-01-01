#ifndef __SEMAPHORE_H
#define __SEMAPHORE_H
#include "Headers.hpp"

// Synchronization Warm up 
class Semaphore {
public:
	Semaphore():value(0) // Constructs a new semaphore with a counter of 0
	{
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
		pthread_mutex_init(&lock, &attr);
		pthread_cond_init(&cond,NULL);
	}
	Semaphore(unsigned val){Semaphore();value=val;} // Constructs a new semaphore with a counter of val

	void up() // Mark: 1 Thread has left the critical section
	{
		pthread_mutex_lock(&lock);
		value++;//++ cause we can run another thread
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&lock);
	}
	void down() // Block until counter >0, and mark - One thread has entered the critical section - WAIT
	{
		pthread_mutex_lock(&lock);
		while(value == 0){
			pthread_cond_wait(&cond,&lock);
		}
		value--;//-- cause a thread started running
		pthread_mutex_unlock(&lock);
	}
private:
	int value;
	pthread_cond_t cond;
	pthread_mutex_t lock;
};

#endif
