# include <iostream>
# include <cstdlib>
# include <pthread.h>
# include <list>
# include <cmath>

# define FCFS	0
# define SRTF	1
# define PBS	2
# define MLFQ	3

using namespace std;

pthread_mutex_t readyqueue_lock;
pthread_cond_t condition_var1[20];
pthread_cond_t con1;

/* the element of ready_queue */
struct ready_queue_t {
	int id;
	float arrival_time;
	int remaining_time;
	int priority;
//	pthread_cond_t condition_var;
};

list<ready_queue_t> ready_queue;

int return_time;		/* the global variable */
int max_current_time;
int scheduler_type;

/* prototypes of several useful function */
void init_scheduler(int sched_type);
int sheduleme(float currentTime, int tid, int remainingTime, int tprio);
int fcfs_scheduleme(float currentTime, int tid, int remainingTime, int tprio);		/* FCFS */
int srtf_scheduleme(float currentTime, int tid, int remainingTime, int tprio);		/* SRTF */
bool search_id(int tid);															/* return if it is found */
ready_queue_t find_tid(int tid);													/* return the struct be found */
bool compare_remainingTime(const ready_queue_t& tmp1, const ready_queue_t& tmp2);	/* help to sort the list */

/* compare function for the ready queue list */
bool compare_remainingTime( const ready_queue_t& tmp1, const ready_queue_t& tmp2 )
{
	return tmp1.remaining_time < tmp2.remaining_time;
}

void init_scheduler(int sched_type) {
	return_time = 0;
	max_current_time = 0;		/* max among the several threads */
	scheduler_type = sched_type;
	/* initialize the mutex */
	pthread_mutex_init(&readyqueue_lock, NULL);
}

int scheduleme(float currentTime, int tid, int remaining_time, int tprio) {
	return_time = currentTime;
	switch(scheduler_type) {
		case FCFS:
			return_time = fcfs_scheduleme(currentTime, tid, remaining_time, tprio);
			break;
		case SRTF:
			return_time = srtf_scheduleme(currentTime, tid, remaining_time, tprio);
			break;
		case PBS:
			break;
		case MLFQ:
			break;
		default:
			break;
	}
	return return_time;
}

/* First Come First Serve */
int fcfs_scheduleme(float currentTime, int tid, int remainingTime, int tprio) {
	
	pthread_mutex_lock(&readyqueue_lock);

	/* case 1: if the input thread is not in the ready_queue, add it to the ready_queue */
	if (!search_id(tid)) {
		ready_queue_t th;
		th.id = tid;
		th.arrival_time = currentTime;
		th.remaining_time = remainingTime;
		th.priority = tprio;
//		pthread_cond_init(&th.condition_var, NULL);
		pthread_cond_init(&condition_var1[tid], NULL);
		ready_queue.push_back(th);
	}
	
	/* case 2: if the thread is not at the head of ready_queue, block it, wait until it go to the head */
	if ( search_id(tid) && (ready_queue.front().id != tid)) {
		ready_queue_t target;
		target = find_tid(tid);
		pthread_cond_wait(&condition_var1[target.id], &readyqueue_lock);
//		pthread_cond_wait(&con1, &readyqueue_lock);
	}
	pthread_mutex_unlock(&readyqueue_lock);
	
	/* update the time */
	ready_queue.front().remaining_time = remainingTime;
	
	if (remainingTime <= 0) {
		pthread_mutex_lock(&readyqueue_lock);
		ready_queue.pop_front();
		
		if (!ready_queue.empty()) {
//			pthread_cond_signal(&con1);
			pthread_cond_signal(&condition_var1[ready_queue.front().id]);
		}
		pthread_mutex_unlock(&readyqueue_lock);
	}
	
	/* then, find the correct return time */
	if (ceil(currentTime) > return_time)
		return_time = ceil(currentTime);
	
	return return_time;
}

/* Shortest Remaining Time First */
int srtf_scheduleme(float currentTime, int tid, int remainingTime, int tprio)
{
	
	pthread_mutex_lock(&readyqueue_lock);

	/* case 1: if the input thread is not in the ready_queue, add it to the ready_queue */
	if (!search_id(tid)) {
		ready_queue_t th;
		th.id = tid;
		th.arrival_time = currentTime;
		th.remaining_time = remainingTime;
		th.priority = tprio;
		pthread_cond_init(&condition_var1[tid], NULL);
		ready_queue.push_back(th);
		ready_queue.sort(compare_remainingTime);
	}
	
	/* case 2: if the thread is not at the head of ready_queue, block it, wait until it go to the head */
	if ( search_id(tid) && (ready_queue.front().id != tid)) {
		ready_queue_t target;
		target = find_tid(tid);
		pthread_cond_wait(&condition_var1[target.id], &readyqueue_lock);
	}
	pthread_mutex_unlock(&readyqueue_lock);
	
	/* update the time */
	ready_queue.front().remaining_time = remainingTime;
	
	if (remainingTime <= 0) {
		pthread_mutex_lock(&readyqueue_lock);
		ready_queue.pop_front();
		
		if (!ready_queue.empty()) {
			ready_queue.sort(compare_remainingTime);
			pthread_cond_signal(&condition_var1[ready_queue.front().id]);
		}
		pthread_mutex_unlock(&readyqueue_lock);
	}
	
	/* then, find the correct return time */
	if (ceil(currentTime) > return_time)
		return_time = ceil(currentTime);
	
	return return_time;	
}

bool search_id(int id) {
	for (list<ready_queue_t>::iterator it = ready_queue.begin(); it != ready_queue.end(); ++it) {
		if (id == it->id) {
			return true;
		}
	}		
	return false;
}

ready_queue_t find_tid(int tid) {
	for (list<ready_queue_t>::iterator it = ready_queue.begin(); it != ready_queue.end(); ++it) {
		if (tid == it->id) {
			return *it;
		}
	}
}
