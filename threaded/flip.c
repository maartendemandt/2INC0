/* 
 * Operating Systems [2INCO] Practical Assignment
 * Threaded Application
 *
 * STUDENT_NAME_1 (STUDENT_NR_1)
 * STUDENT_NAME_2 (STUDENT_NR_2)
 *
 * Grading:
 * Students who hand in clean code that fully satisfies the minimum requirements will get an 8. 
 * "Extra" steps can lead to higher marks because we want students to take the initiative. 
 * Extra steps can be, for example, in the form of measurements added to your code, a formal 
 * analysis of deadlock freeness etc.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>          // for perror()
#include <pthread.h>

#include "uint128.h"
#include "flip.h"

// create a bitmask where bit at position n is set
#define BITMASK(n)          (((uint128_t) 1) << (n))

// check if bit n in v is set
#define BIT_IS_SET(v,n)     (((v) & BITMASK(n)) == BITMASK(n))

// set bit n in v
#define BIT_SET(v,n)        ((v) =  (v) |  BITMASK(n))

// clear bit n in v
#define BIT_CLEAR(v,n)      ((v) =  (v) & ~BITMASK(n))


static pthread_mutex_t      buffer_mutex          = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t      thread_mutex          = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t       cond		  = PTHREAD_COND_INITIALIZER;

static void * flip (void * arg);

struct thread_struct { 
	int i;
	int thread_id;
};

int 		nrof_threads = 0;
int		id;

int main (void)
{
	// TODO: start threads to flip the pieces and output the results
	// BIT SET = black
	// BIT CLEAR = white
	// (see thread_test() and thread_mutex_test() how to use threads and mutexes,
	//  see bit_test() how to manipulate bits in a large integer)

	for(int s = 0; s <= NROF_PIECES; s++) {
		BIT_SET(buffer[s/128],s%128);
	}

	int  		temp_id;
	pthread_t   	thread_id[NROF_THREADS];
	struct thread_struct parameter[NROF_THREADS];
    
	for(int i = 2; i <= NROF_PIECES; i++) {

		pthread_mutex_lock(&thread_mutex);

		while (nrof_threads >= NROF_THREADS) {

			pthread_cond_wait(&cond, &thread_mutex);
			pthread_join (thread_id[id], NULL);
			temp_id = id;
		}

		nrof_threads++;
		
		// New thread
	
		if(i-2 < NROF_THREADS) {
			temp_id = i-2;

		}		

		parameter[temp_id].i = i; 
		parameter[temp_id].thread_id = temp_id;
		pthread_create (&thread_id[temp_id], NULL, flip, (void *)&parameter[temp_id]);
		
		id = NROF_THREADS + 1;

		pthread_mutex_unlock(&thread_mutex);



	}

	while(nrof_threads > 0) {
		pthread_mutex_lock(&thread_mutex);

		pthread_cond_wait(&cond, &thread_mutex);
		pthread_join (thread_id[id], NULL);
		
		id = NROF_THREADS + 1;

		pthread_mutex_unlock(&thread_mutex);
	}

	// Print
	for(int s = 1; s <= NROF_PIECES; s++) {
		if(BIT_IS_SET(buffer[s/128], s%128)) {
			printf("%d\n", s);
		}	
	}

    	return (0);
}

void * flip (void *arg) {

	struct thread_struct *argi; 
	int     i;      

	argi = (struct thread_struct *) arg;   
	i = argi->i;    

	for(int j = 1; i * j <= NROF_PIECES;j++) {
		pthread_mutex_lock (&buffer_mutex);
		if(BIT_IS_SET(buffer[(i*j)/128],(i*j)%128)) {
			BIT_CLEAR(buffer[(i*j)/128],(i*j)%128);
		}
		else {
			BIT_SET(buffer[(i*j)/128],(i*j)%128);
		}
		pthread_mutex_unlock (&buffer_mutex);
	}

	bool signal = false;

	while(!signal) {
		pthread_mutex_lock (&thread_mutex);
		
		if(id>NROF_THREADS) {
			id = argi->thread_id;
			nrof_threads--;
			pthread_cond_signal(&cond);
			signal = true;
		}
		pthread_mutex_unlock (&thread_mutex);

	}



	return (NULL);
	
}

