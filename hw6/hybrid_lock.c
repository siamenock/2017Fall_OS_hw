#include "hybrid_lock.h"

//#include <time.h>

void enque(wait_node_t* header, wait_node_t* node){
	node->next = NULL;
	for(;header->next != NULL; header = header->next){
		continue;
	}
	header->next = node;
}

wait_node_t* deque(wait_node_t* header){
	if(header->next == NULL){ return NULL;}
	wait_node_t* node = header->next;
	header->next = node->next;
	return node;
}

wait_node_t* new_wait_node(){
	wait_node_t * node = (wait_node_t *) malloc(sizeof(wait_node_t));
	pthread_mutex_init(&node->lock, NULL);
	int suc = pthread_mutex_trylock(&node->lock); // locked state is init state
	if(suc != LOCK_SUCCESS){
		printf("\ttrylock : new node error!\n");
	}
	node->next = NULL;
	return node;
}

wait_node_t* get_empty_node(hybrid_lock_t* __lock){
	wait_node_t* node = deque(__lock->empty_nodes);
	if (node != NULL){
		return node;
	} else {
		//printf("make new node!\n");
		return new_wait_node();
	}

}

void destroy_node(wait_node_t* node){
	pthread_mutex_destroy(&node->lock);
	free(node);
}

void return_node(hybrid_lock_t* __lock, wait_node_t* node){
	enque(__lock->empty_nodes, node);
}

int hybrid_lock_init(hybrid_lock_t* __lock){
	__lock->sequence_header = new_wait_node();
	__lock->empty_nodes = new_wait_node();
	pthread_spin_init(&__lock->internal_lock, 0);
	return pthread_spin_init(&__lock->spinlock, 0);
}

int hybrid_lock_destroy(hybrid_lock_t* __lock){
	wait_node_t* node = NULL;

	for(node = __lock->sequence_header; node != NULL; node = node->next){
		pthread_mutex_destroy(&node->lock);
	}
	pthread_spin_destroy(&__lock->internal_lock);
	return pthread_spin_destroy(&__lock->spinlock);
}

int hybrid_lock_lock(hybrid_lock_t* __lock){
	//struct timespec start, end;
	//clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	
	int i;
	bool get_spin = false;
	for(i = 0; i < TRIAL_NUM; i++){
		if(pthread_spin_trylock(&__lock->spinlock) == LOCK_SUCCESS){
			get_spin = true;
			break;
		}		
	}
	if(get_spin){		// do nothing
		return LOCK_SUCCESS;
	}
	//clock_gettime(CLOCK_MONOTONIC_RAW, &end);
	//int delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
	//printf("%d /1000000sec\n", delta_us);

	
	// case: (fail to get spinlock)
	pthread_spin_lock(&__lock->internal_lock);	// this lock expected to be less busy than main spinlock
	//printf("fail, waite mutex\n");
	if(pthread_spin_trylock(&__lock->spinlock) == LOCK_SUCCESS){
		pthread_spin_unlock(&__lock->internal_lock);
		//printf("lock case 1 from internal stage\n");
		return LOCK_SUCCESS;
	}
	wait_node_t* node = get_empty_node(__lock);				// make locked node
	enque(__lock->sequence_header, node);

	pthread_spin_unlock(&__lock->internal_lock);	// finish inqueing

	pthread_mutex_lock(&node->lock);					// wait until hybrid_unlock called
	//printf("wake up from mutex lock, node addr : %d\n", (int) node);
	
	return LOCK_SUCCESS;
}

int hybrid_lock_unlock(hybrid_lock_t* __lock){
	
	pthread_spin_lock(&__lock->internal_lock);	// this is less busy lock
	wait_node_t* node = deque(__lock->sequence_header);
		
	if(node != NULL){	// case : there is waiting thread
		{	int i = 0;
			wait_node_t* temp = __lock->sequence_header;
			while(temp){
				temp = temp->next;
				i++;
			}
			//printf("%d node detected,  unlocking node addr : %d\n", i, (int)node);
		}
		
		pthread_mutex_unlock(&node->lock);
		return_node(__lock, node);
		pthread_spin_unlock(&__lock->internal_lock);
		return LOCK_SUCCESS;
	} else {			// case : no thread waiting
		//printf("no nodes\n");
		pthread_spin_unlock(&__lock->spinlock);
		pthread_spin_unlock(&__lock->internal_lock);
		return LOCK_SUCCESS;
	}
}
