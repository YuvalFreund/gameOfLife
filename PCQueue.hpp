#ifndef _QUEUEL_H
#define _QUEUEL_H
#include "Semaphore.hpp"
// Single Producer - Multiple Consumer queue
template <typename T>class PCQueue
{

public:
	PCQueue():elements(),readers_inside(0),writers_inside(0),writers_waiting(0),size(0){
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
		pthread_mutex_init(&global_lock, &attr);
		pthread_mutex_init(&other_lock, &attr);
		pthread_mutexattr_destroy(&attr);
		pthread_cond_init(&write_allowed,NULL);
		pthread_cond_init(&read_allowed,NULL);
	}
	~PCQueue(){
		pthread_mutex_destroy(&global_lock);
        pthread_mutex_destroy(&other_lock);
        pthread_cond_destroy(&write_allowed);
		pthread_cond_destroy(&read_allowed);
	}
	//reader_lock - a reader entered critical section
	void reader_lock(){
		pthread_mutex_lock(&global_lock);
		while(writers_inside > 0 || writers_waiting > 0 || elements.empty())
		//wait for no readers or give space for waiting readers. wait also if queue is empty
			pthread_cond_wait(&read_allowed,&global_lock);
		readers_inside++;
		//pthread_mutex_unlock(&global_lock);
	}
	//reader unlock - a reader finished critical section
	void reader_unlock(){
		//pthread_mutex_lock(&global_lock);
		readers_inside--;
		if(readers_inside == 0)
			pthread_cond_signal(&write_allowed);
		pthread_mutex_unlock(&global_lock);
	}
	//writer_lock - a writer entered critical section
	void writer_lock(){
		pthread_mutex_lock(&global_lock);
		writers_waiting++;
		while(writers_inside + readers_inside > 0)
			pthread_cond_wait(&write_allowed,&global_lock);
		writers_waiting--;
		writers_inside++;
		//pthread_mutex_unlock(&global_lock);
	}
	//writer_unlock - a writer finished critical section
	void writer_unlock(){
		//pthread_mutex_lock(&global_lock);
		writers_inside--;
		if(writers_inside == 0){
			pthread_cond_broadcast(&read_allowed);
			pthread_cond_signal(&write_allowed);
		}
		pthread_mutex_unlock(&global_lock);
	}
 void print_queue(){
   for(int i=0;i<elements.size();i++){
     T temp = elements.front();
     cout<<temp<<",";
     elements.pop();
     elements.push(temp);
   }
   cout<<"DONE\n";
 }
	// Blocks while queue is empty. When queue holds items, allows for a single
	// thread to enter and remove an item from the front of the queue and return it. 
	// Assumes multiple consumers.
	T pop(){
		//enter a new reader
		pthread_mutex_lock(&this->global_lock);
		while(this->writers_inside > 0 || this->writers_waiting > 0 || this->elements.empty())
		//wait for no readers or give space for waiting readers. wait also if queue is empty
			pthread_cond_wait(&this->read_allowed,&this->global_lock);
		this->readers_inside++;
		T top = this->elements.front();//accessing first element in queue
		this->elements.pop();//removing it
		//finished reading - can remove it
		this->readers_inside--;
		if(this->readers_inside == 0)
			pthread_cond_signal(&this->write_allowed);
		pthread_mutex_unlock(&this->global_lock);
		return top;
	}

	// Allows for producer to enter with *minimal delay* and push items to back of the queue.
	// Hint for *minimal delay* - Allow the consumers to delay the producer as little as possible.  
	// Assumes single producer 
	void push(const T& item){
		//enter a new writer 
		this->writers_waiting++;
		pthread_mutex_lock(&this->global_lock);
		while(this->writers_inside + this->readers_inside > 0)
			pthread_cond_wait(&this->write_allowed,&this->global_lock);
		this->writers_waiting--;
		this->writers_inside++;
		this->elements.push(item);//push new task to end of queue
		//finished writing - remove it
		this->writers_inside--;
		if(this->writers_inside == 0){
			pthread_cond_broadcast(&this->read_allowed);
			pthread_cond_signal(&this->write_allowed);
		}
		pthread_mutex_unlock(&this->global_lock);
	}

	bool is_empty(){
		pthread_mutex_lock(&this->other_lock);
		bool temp = (this->size == 0 && this->elements.empty());
		pthread_mutex_unlock(&this->other_lock);
		return temp;
	}
	void reduce_size(){
		pthread_mutex_lock(&this->other_lock);
		this->size-=1;
		pthread_mutex_unlock(&this->other_lock);
	}
	void increase_size(){
		pthread_mutex_lock(&this->other_lock);
		this->size+=1;
		pthread_mutex_unlock(&this->other_lock);
	}
	int get_size(){
		pthread_mutex_lock(&this->other_lock);
		int temp_size = this->size;
		pthread_mutex_unlock(&this->other_lock);
		return temp_size;
	}

private:
	queue<T> elements; 
	int readers_inside;
	int writers_inside;
	int writers_waiting;
	int size;
	pthread_cond_t write_allowed;
	pthread_cond_t read_allowed;
	pthread_mutex_t global_lock;
	pthread_mutex_t other_lock;
};
// Recommendation: Use the implementation of the std::queue for this exercise
#endif