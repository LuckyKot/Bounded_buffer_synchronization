#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

/* Initializing constants, constant for number of arguments and for buffer size */
#define BUFFER_SIZE 5
#define NUM_OF_ARGUMENTS 4

/* defining buffer_item type and initializing shared buffer. Also in and out variables for circular queue */
typedef int buffer_item;
buffer_item buffer[BUFFER_SIZE];
int in = -1;
int out = -1;

/* defining mutex locks and semaphores */
pthread_mutex_t mutex;
sem_t full;
sem_t empty;

int insert_item(buffer_item item);
int remove_item(buffer_item *item);
void *consumer(void *param);
void *producer(void *param);

int main(int argc, char *argv[]){
	
	/* some service variables */
	int time = 0;
	int producers = 0;
	int consumers = 0;
	int i = 0;
	
	printf("Main thread beginning\n");
	
	/* checking if the program is used properly */
	if (argc != NUM_OF_ARGUMENTS){
		printf("Wrong number of arguments\n");
		printf("Must be: progname time #ofproducers #ofconsumers\n");
		exit(0);
	}
	
	/* getting arguments */
	time = atoi(argv[1]);
	producers = atoi(argv[2]);
	consumers = atoi(argv[3]);
	
	/* initializing mutex lock and semaphores full and empty. Full set to 0, empty set to buffer size */
	pthread_mutex_init(&mutex,NULL);
	sem_init(&full,0,0);
	sem_init(&empty,0,BUFFER_SIZE);
	
	printf("Time:%d, Producers:%d, Consumers:%d Total threads:%d\n", time,producers,consumers,producers+consumers);
	
	/* variable needed for threads */
	int localtid[consumers+producers];
	pthread_t tid[consumers+producers]; 	
	pthread_attr_t attr;
	
	pthread_attr_init(&attr); 
	
	/* creating producer threads */
	for (i = 0; i < producers; i++) { 
		localtid[i-1] = i;
		pthread_create(&tid[i],&attr,producer,(void *) &localtid[i-1]); 
		printf("Creating producer %d with thread id = %lu\n", i, tid[i]);
        }
        
        /* creating consumer threads */
        for (i = 0; i < consumers; i++) { 
		localtid[i-1] = i;
		pthread_create(&tid[i],&attr,consumer,(void *) &localtid[i-1]); 
		printf("Creating consumer %d with thread id = %lu\n", i, tid[i]);
        }
	
	printf("Main thread sleeping for %d seconds\n\n", time);
	
	/* sleeping for provided time */
	sleep(time);
	
	printf("Main thread exiting\n");
	exit(0);
}

int insert_item(buffer_item item){
	
	/* following the book, wait(empty) and mutex lock first */
	sem_wait(&empty);
	pthread_mutex_lock(&mutex);
	
	/* circular queue insertion */
	if (in == -1){
		in = 0;
	}
	out = (out + 1) % BUFFER_SIZE;
	buffer[out] = item;
	printf("Insert_item inserted item %d at position %d\n", item, out);
	
	/* necessary printout of current buffer state */
	int i = 0;
	for (i; i<BUFFER_SIZE; i++){
		if (buffer[i]==0){
			printf("[empty]");
		} else {
			printf("[%d]", buffer[i]);
		}
	}
	printf(" in = %d, out = %d\n",in,out);

	/* following the book again, releasing mutex and signal(full) */
	pthread_mutex_unlock(&mutex);
	sem_post(&full);
	
	return 0;
}

int remove_item(buffer_item *item){
	
	/* wait(full) and mutex */
	sem_wait(&full);
	pthread_mutex_lock(&mutex);
    	
    	/* removing the item in a circular queue manner */
	*item = buffer[in];
	buffer[in]=0;
	printf("Remove_item removed item %d at position %d\n", *item, in);
	if (in == out) {
		in = -1;
		out = -1;
	} else {
		in = (in + 1) % BUFFER_SIZE;
	}
	
	/* printing current status of buffer */
	int i = 0;
	for (i; i<BUFFER_SIZE; i++){
		if (buffer[i]==0){
			printf("[empty]");
		} else {
			printf("[%d]", buffer[i]);
		}
	}
	printf(" in = %d, out = %d\n",in,out);
	
	/* releasing mutex and signal(empty) */
	pthread_mutex_unlock(&mutex);
	sem_post(&empty);
	
	return 0;
	
}

/* consumer thread */
void *consumer(void *param){
	buffer_item item;
	
	/* getting its consumer thread number */
	int i;
	i = *((int *) param);
	
	/* continuously generating random number to sleep (between 1 and 4 seconds)
	   then removing an item by calling remove_item() described above */
	while(1){
		int random_num = (rand()%4)+1;
		printf("Consumer thread %d sleeping for %d seconds\n\n", i, random_num);
		sleep(random_num);
		
		if (remove_item(&item)){
			printf("Error removing item!\n");
		} else {
			printf("Consumer thread %d removed value %d\n", i, item);
		}	
	}
}

/* producer thread */
void *producer(void *param){
	buffer_item item;
	
	/* getting its producer thread number */
	int i;
	i = *((int *) param);

	/* continuously generating random number to sleep (between 1 and 4 seconds)
	   and a number to add to buffer
	   then adding an item by calling insert_item() described above */
	while(1){
		int random_num = (rand()%4)+1;
		printf("Producer thread %d sleeping for %d seconds\n\n", i, random_num);
		sleep(random_num);
		random_num = (rand()%50)+1;
		
		item = (rand()%50)+1;
		if (insert_item(item)){
			printf("Error inserting item!\n");
		} else {
			printf("Producer thread %d inserted value %d\n", i, item);
		}
	}
}





