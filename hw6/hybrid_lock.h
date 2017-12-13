#ifndef _HYBRID_LOCK_H_
#define _HYBRID_LOCK_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

#define LOCK_SUCCESS 0	// <pthread.h> use 0 as success when calloing lock functions
#define TRIAL_NUM 5000000 // it takes about 1 sec

typedef struct _wait_node_t{	
	pthread_mutex_t lock;			// free this lock when start next thread able to acquire lock
	struct _wait_node_t * next;			// linked list type
}wait_node_t;

typedef struct _hybrid_lock_t {
	pthread_spinlock_t spinlock;		// if fail to get spinlock for 1second, add new lock in sequence
	pthread_spinlock_t internal_lock;
	wait_node_t* sequence_header;	// header's lock work as lock for sequencing, not for waiting threads
	wait_node_t* empty_nodes;

} hybrid_lock_t;

int hybrid_lock_init(hybrid_lock_t* __lock);
int hybrid_lock_destroy(hybrid_lock_t* __lock);
int hybrid_lock_lock(hybrid_lock_t* __lock);
int hybrid_lock_unlock(hybrid_lock_t* __lock);

#endif