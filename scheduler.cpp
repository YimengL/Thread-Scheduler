/* version: 2.0(the latest version) */
/* Author: Yangkai Zhou(yz395), Yimeng Li(yl720) */
/* Finished Date: 2014/10/12 */

# include <iostream>
# include <cstdlib>
# include <pthread.h>
# include <list>
# include <cmath>
# include <set>

# define FCFS	0
# define SRTF	1
# define PBS	2
# define MLFQ	3
# define MAX	30		/* max number of threads */

using namespace std;

/* ******************************** declare several useful variables and data structure ******************************** */

pthread_mutex_t readyqueue_lock;
pthread_cond_t condition_var1[MAX];

/* the element of ready_queue */
struct ready_queue_t {
	int id;
	float arrival_time;
	int remaining_time;
	int priority;
};

/* the element of the ready_queue for MLFQ */
struct rq_t_mlfq {
	int id;
	float arrival_time;
	int remaining_time;
	int level;
	int times_sche;	
};

/* data structures needed for MLFQ */
list<rq_t_mlfq> rq_mlfq[5];
int time_quantum[5] = { 5,10,15,20,25 };
set<int> exist;

list<ready_queue_t> ready_queue;

int return_time;		/* the global variable */
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

/* helper functions for MLFQ */
void being_scheduled( int level );
int find_next_wake( int level );
bool is_lower_empty( int level );
bool is_upper_level_empty( int level );
int check_level( int t_id );
rq_t_mlfq find_tid(int tid);

/* ***************************************************************************************************************** */

/* ******************************************* several helper function ********************************************* */

/* compare function for the ready queue list */
bool compare_remainingTime( const ready_queue_t& tmp1, const ready_queue_t& tmp2 ) {
	return tmp1.remaining_time < tmp2.remaining_time;
}

bool compare_priority(const ready_queue_t& tmp1, const ready_queue_t &tmp2) {
	return (tmp1.priority < tmp2.priority);	
}

/* check if thread is included in ready queue */
bool search_id(int id) {
	for (list<ready_queue_t>::iterator it = ready_queue.begin(); it != ready_queue.end(); ++it) {
		if (id == it->id) {
			return true;
		}
	}		
	return false;
}

/* schedule the MLFQ */
void being_scheduled(int level) {
	rq_t_mlfq& th = rq_mlfq[level].front();
	if( th.times_sche > 0 )
		th.times_sche -= 1;
	else {
		if( th.level < 4 ) {
			rq_mlfq[th.level].pop_front();
			th.level += 1;
			th.times_sche = time_quantum[th.level];
			rq_mlfq[th.level].push_back(th);
		}
		else {
			rq_mlfq[level].pop_front();
			th.times_sche = time_quantum[th.level];
			rq_mlfq[level].push_back(th);
		}
		/* block this thread */
		int next = find_next_wake(level);
		if (next != th.id) {
			pthread_cond_signal(&condition_var1[next]);
			pthread_cond_wait(&condition_var1[th.id], &readyqueue_lock);
		}
	}		
}

/* find the next awake thread */
int find_next_wake( int level ) {
	int result = -1;
	for( int i = 0; i < 5; ++i ) {
		if(!rq_mlfq[i].empty())
			return rq_mlfq[i].front().id;
	}
	return result;
}

/* two check empty function */
bool is_lower_empty( int level ) {
	for( int i = 0; i < 5; ++i ) {
		if( !rq_mlfq[i].empty() )
			return false;
	}
	return true;
}

bool is_upper_level_empty( int level ) {
	int t_level = level;
	
	for( int i = 0; i < t_level; ++i ) {
		if( !rq_mlfq[i].empty() )
			return false;
	}
	return true;
}

/* check the level in MLFQ */
int check_level( int t_id ) {
	list<rq_t_mlfq>::iterator iter;
	int t_level;
	
	for( int i = 0; i < 5; ++i ) {
		for( iter = rq_mlfq[i].begin(); iter != rq_mlfq[i].end(); ++iter ) {
			if( iter->id == t_id )
				t_level = iter->level;
		}
	}
	return t_level;
}

/* find the tid in another data structure */
rq_t_mlfq find_tid(int tid) {
	list<rq_t_mlfq>::iterator it;
	for (int i = 0; i < 5; ++i) {
		for (it = rq_mlfq[i].begin(); it != rq_mlfq[i].end(); ++it) {
			if (tid == it->id) {
				return *it;
			}
		}
	}
}

/* ********************************************************************************************************** */

/* ************************************** Interface with project1.c ***************************************** */

/* initialize the mutex, cond_var and data structure */
void init_scheduler(int sched_type) {
	return_time = 0;
	scheduler_type = sched_type;
	/* initialize the mutex */
	pthread_mutex_init(&readyqueue_lock, NULL);
	int i;
	for(i = 0; i < MAX; ++i)
		pthread_cond_init(&condition_var1[i], NULL);
}

/* a switch function */
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
			return_time = pbs_scheduleme(currentTime, tid, remaining_time, tprio);
			break;
		case MLFQ:
			return_time = mlfq_scheduleme(currentTime, tid, remaining_time, tprio);
			break;
		default:
			break;
	}
	return return_time;
}

/* ************************************************************************************************************** */

/* ***************************************** 4 scheduling algorithm ********************************************* */

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
	
	/* update the time */
	ready_queue.front().remaining_time = remainingTime;
	
	if (remainingTime <= 0) {
		ready_queue.pop_front();
		
		if (!ready_queue.empty()) {
			pthread_cond_signal(&condition_var1[ready_queue.front().id]);
		}
	}
	/* then, find the correct return time */
	if (ceil(currentTime) > (int)return_time)
		return_time = ceil(currentTime);
	
	pthread_mutex_unlock(&readyqueue_lock);
	
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
		ready_queue.push_back(th);
		pthread_cond_init(&condition_var1[tid], NULL);
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
	
	/* update the time */
	ready_queue.front().remaining_time = remainingTime;
	
	if (remainingTime <= 0) {
		ready_queue.pop_front();
		
		if (!ready_queue.empty()) {
			ready_queue.sort(compare_priority);
			pthread_cond_signal(&condition_var1[ready_queue.front().id]);
		}
	}
	/* then, find the correct return time */
	if (ceil(currentTime) > return_time)
		return_time = ceil(currentTime);
		
	pthread_mutex_unlock(&readyqueue_lock);

	return return_time;
}

int mlfq_scheduleme(float currentTime, int tid, int remainingTime, int tprio) {
	
	pthread_mutex_lock(&readyqueue_lock);
	
	/* case 1: if the input thread is not in the ready_queue, add it to the ready_queue */
	if(exist.find(tid) == exist.end()) {
		rq_t_mlfq th;
		th.id = tid;
		th.arrival_time = currentTime;
		th.remaining_time = remainingTime;
		th.level = 0;
		th.times_sche = time_quantum[th.level];
		pthread_cond_init(&condition_var1[tid], NULL);
		rq_mlfq[th.level].push_back(th);
		exist.insert(tid);
	}	
	
	/* case 2: if the thread is in the ready_queue, check whether it should being scheduled */	
	int t_level = check_level(tid);
		
	if( !is_upper_level_empty(t_level) || rq_mlfq[t_level].front().id != tid )
		pthread_cond_wait(&condition_var1[tid], &readyqueue_lock);
	
	if (remainingTime <= 0) {
		rq_mlfq[t_level].pop_front();
		
		if (!is_lower_empty(t_level)) {
			int next = find_next_wake(t_level);
			pthread_cond_signal(&condition_var1[next]);
		}
	}
	else {
		rq_t_mlfq target = find_tid(tid);
		being_scheduled(target.level);
	}
	
	/* then, find the correct return time */
	if (ceil(currentTime) > return_time)
		return_time = ceil(currentTime);
		
	pthread_mutex_unlock(&readyqueue_lock);
	
	return return_time;	
}
/* ************************************************************************************************************* */
