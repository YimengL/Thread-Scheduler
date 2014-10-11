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
pthread_mutex_t extra_lock;
pthread_cond_t condition_var1[20];
pthread_cond_t condition_var;
pthread_cond_t con1;

/* the element of ready_queue */
struct ready_queue_t {
	int id;
	float arrival_time;
	int remaining_time;
	int priority;
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
int pbs_scheduleme(float currentTime, int tid, int remainingTime, int tprio);		/* PBS */
int mlfq_scheduleme(float currentTime, int tid, int remaining_time, int tprio);		/* MLFQ */
bool search_id(int tid);															/* return if it is found */
bool compare_remainingTime(const ready_queue_t& tmp1, const ready_queue_t& tmp2);	/* help to sort the list */
bool compare_priority(const ready_queue_t& tmp1, const ready_queue_t& tmp2);		/* help to sort the list */

/* compare function for the ready queue list */
bool compare_remainingTime( const ready_queue_t& tmp1, const ready_queue_t& tmp2 ) {
	return tmp1.remaining_time < tmp2.remaining_time;
}

bool compare_priority(const ready_queue_t& tmp1, const ready_queue_t &tmp2) {
	return (tmp1.priority < tmp2.priority);	
}

bool search_id(int id) {
	for (list<ready_queue_t>::iterator it = ready_queue.begin(); it != ready_queue.end(); ++it) {
		if (id == it->id) {
			return true;
		}
	}		
	return false;
}

void init_scheduler(int sched_type) {
	return_time = 0;
	max_current_time = 0;		/* max among the several threads */
	scheduler_type = sched_type;
	/* initialize the mutex */
	pthread_mutex_init(&readyqueue_lock, NULL);
	pthread_mutex_init(&extra_lock, NULL);
}

int scheduleme(float currentTime, int tid, int remaining_time, int tprio) {
	switch(scheduler_type) {
		pthread_mutex_lock(&extra_lock);
		case FCFS:
			return_time = fcfs_scheduleme(currentTime, tid, remaining_time, tprio);
			break;
		case SRTF:
			return_time = srtf_scheduleme(currentTime, tid, remaining_time, tprio);
			break;
		case PBS:
			return_time = pbs_scheduleme(currentTime, tid, remaining_time, tprio);
			break;
		case MLFQ:
			return_time = mlfq_scheduleme(currentTime, tid, remaining_time, tprio);
			break;
		default:
			break;
	}
	pthread_mutex_unlock(&extra_lock);
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
		pthread_cond_init(&condition_var1[tid], NULL);
		ready_queue.push_back(th);
	}
	
	/* case 2: if the thread is not at the head of ready_queue, block it, wait until it go to the head */
	if ( search_id(tid) && (ready_queue.front().id != tid)) {
		pthread_cond_wait(&condition_var1[tid], &readyqueue_lock);
	}
	pthread_mutex_unlock(&readyqueue_lock);
	
	/* update the time */
	ready_queue.front().remaining_time = remainingTime;
	
	if (remainingTime <= 0) {
		pthread_mutex_lock(&readyqueue_lock);
		ready_queue.pop_front();
		
		if (!ready_queue.empty()) {
			pthread_cond_signal(&condition_var1[ready_queue.front().id]);
		}
		pthread_mutex_unlock(&readyqueue_lock);
	}
	
	/* then, find the correct return time */
	if (ceil(currentTime) > (int)return_time)
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
		pthread_cond_wait(&condition_var1[tid], &readyqueue_lock);
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

/* Priority Based Scheduling */
int pbs_scheduleme(float currentTime, int tid, int remainingTime, int tprio) {
	
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
		ready_queue.sort(compare_priority);
	}
	
	/* case 2: if the thread is not at the head of ready_queue, block it, wait until it go to the head */
	if ( search_id(tid) && (ready_queue.front().id != tid)) {
		pthread_cond_wait(&condition_var1[tid], &readyqueue_lock);
	}
	pthread_mutex_unlock(&readyqueue_lock);
	
	/* update the time */
	ready_queue.front().remaining_time = remainingTime;
	
	if (remainingTime <= 0) {
		pthread_mutex_lock(&readyqueue_lock);
		ready_queue.pop_front();
		
		if (!ready_queue.empty()) {
			ready_queue.sort(compare_priority);
			pthread_cond_signal(&condition_var1[ready_queue.front().id]);
		}
		pthread_mutex_unlock(&readyqueue_lock);
	}
	
	/* then, find the correct return time */
	if (ceil(currentTime) > return_time)
		return_time = ceil(currentTime);
	
	return return_time;
}

int mlfq_scheduleme(float currentTime, int tid, int remainingTime, int tprio) {
	
	pthread_mutex_lock(&readyqueue_lock);
	
	if (!search_id(tid)) {
//		cout << "Time: " << currentTime << ", Thread " << tid << " was insert!" << endl;
		ready_queue_t th;
		th.id = tid;
		th.arrival_time = currentTime;
		th.remaining_time = remainingTime;
		th.priority = tprio;
		pthread_cond_init(&condition_var, NULL);
//		pthread_cond_init(&condition_var1[tid], NULL);
		ready_queue.push_back(th);
	}
//	cout << "Time: " << currentTime << ", Thread " << tid << " was insert!" << endl;
	pthread_mutex_unlock(&readyqueue_lock);
	
	if ( search_id(tid) && (ready_queue.front().id != tid)) {
		pthread_cond_wait(&condition_var1[tid], &readyqueue_lock);
//		pthread_cond_wait(&con1, &readyqueue_lock);
	}
//	pthread_mutex_unlock(&readyqueue_lock);
	
	/* update the time */
	ready_queue.front().remaining_time = remainingTime;
	
	if (remainingTime <= 0) {
//		pthread_mutex_lock(&readyqueue_lock);
		ready_queue.pop_front();
		
		if (!ready_queue.empty()) {
//			pthread_cond_signal(&con1);
			pthread_cond_signal(&condition_var);
		}
//		pthread_mutex_unlock(&readyqueue_lock);
	}
	/*else*/ pthread_mutex_unlock(&readyqueue_lock);
		
	return return_time;
}
