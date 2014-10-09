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
pthread_cond_t condition_var1[9];
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
int fcfs_scheduleme(float currentTime, int tid, int remainingTime, int tprio);
bool search_id(int tid);
ready_queue_t find_tid(int tid);

void init_scheduler(int sched_type) {
	return_time = 0;
	max_current_time = 0;		/* max among the several threads */
	scheduler_type = sched_type;
	/* initialize the mutex */
	readyqueue_lock = PTHREAD_MUTEX_INITIALIZER;
	con1 = PTHREAD_COND_INITIALIZER;
}

int scheduleme(float currentTime, int tid, int remaining_time, int tprio) {
	return_time = currentTime;
	int retTime;
	switch(scheduler_type) {
		case FCFS:
			retTime = fcfs_scheduleme(currentTime, tid, remaining_time, tprio);
			break;
		case SRTF:
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
//		con1 = PTHREAD_COND_INITIALIZER;
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
//	ready_queue.front().arrival_time = currentTime;
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
	if (ceil(currentTime) > max_current_time)
		max_current_time = ceil(currentTime);

	if (max_current_time > return_time)
		return_time = max_current_time;
	
	return return_time;
//	return max_current_time;
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
	
///* test the queue */
//int main(int argc, char** argv) {
	//ready_queue_t th;
////	th = (ready_queue_t*)malloc(sizeof(ready_queue_t));
	//th.id = 1;
	//th.arrival_time = 2.0;
	//th.remaining_time = 1;
	//th.priority = 1;
	//th.condition_var = PTHREAD_COND_INITIALIZER;
	
////	ready_queue_t th;
	//th.id = 2;
	//th.arrival_time = 2.2;
	//th.remaining_time = 2;
	//th.priority = 3;
	//th.condition_var = PTHREAD_COND_INITIALIZER;
	
	//ready_queue.push_back(th);
	
	//cout << ready_queue.size() << endl;
	//cout << search_id(1) << endl;
	//cout << find_tid(2).arrival_time << endl;
	
	//return 0;
//}
