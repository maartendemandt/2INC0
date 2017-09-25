/* 
 * Operating Systems [2INCO] Practical Assignment
 * Interprocess Communication
 *
 * STUDENT_NAME_1 (STUDENT_NR_1)
 * STUDENT_NAME_2 (STUDENT_NR_2)
 *
 * Grading:
 * Students who hand in clean code that fully satisfies the minimum requirements will get an 8. 
 * ”Extra” steps can lead to higher marks because we want students to take the initiative. 
 * Extra steps can be, for example, in the form of measurements added to your code, a formal 
 * analysis of deadlock freeness etc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>          // for perror()
#include <unistd.h>         // for getpid()
#include <mqueue.h>         // for mq-stuff
#include <time.h>           // for time()
#include <complex.h>

#include "common.h"
#include "md5s.h"

static char                 mq_name1[80];
static char                 mq_name2[80];

static void rsleep (int t);

static bool bf (char start_char, char end_char, int length, int index, char p[], uint128_t solution, char* found_solution);


int main (int argc, char * argv[])
{
	// TODO:
	// (see message_queue_test() in interprocess_basic.c)
	//  * open the two message queues (whose names are provided in the arguments)
	mqd_t               mq_fd_request;
	mqd_t               mq_fd_response;
	MQ_REQUEST_MESSAGE  req;
	MQ_RESPONSE_MESSAGE rsp;
	sprintf (mq_name1, "%s", argv[1]);
	sprintf (mq_name2, "%s", argv[2]);

	bool jobs = true;
	mq_fd_request = mq_open (mq_name1, O_RDONLY);

	mq_fd_response = mq_open (mq_name2, O_WRONLY);
	
   	//  * repeatingly:
	while(jobs) {
 		//      - read from a message queue the new job to do
		mq_receive (mq_fd_request, (char*) &req, sizeof(req), NULL);
		
		//      - wait a random amount of time (e.g. rsleep(10000);)
		rsleep(10000);

		//    until there are no more tasks to do
		if(req.length == 0) 
		{
			jobs = false;
		}
		else {
			char p[1];	
			p[0] = req.first_char;
			char f_solution[req.length];
			
			//      - do that job 
			bool found = bf(req.start_char, req.end_char, req.length, 1, p, req.md5, f_solution);
			
			rsp.index = req.list_index;

			if(found) 
			{
				rsp.length = strlen(f_solution);
				for(int i = 0; i < strlen(f_solution); i++) 
				{
					rsp.word[i] = f_solution[i];
				}
				for(int j = strlen(f_solution); j < req.length; j++)
				{
					rsp.word[j] = (char) 0;
				}	
			}
			else 
			{
				rsp.length = 0;
			}

			//      - write the results to a message queue
			mq_send (mq_fd_response, (char*) &rsp, sizeof(rsp), 0);
		}

	}

	
	//  * close the message queues
	mq_close (mq_fd_response);
	mq_close (mq_fd_request);
        
    	return (0);
}

/*
 * rsleep(int t)
 *
 * The calling thread will be suspended for a random amount of time
 * between 0 and t microseconds
 * At the first call, the random generator is seeded with the current time
 */
static void rsleep (int t)
{
    static bool first_call = true;
    
    if (first_call == true)
    {
        srandom (time (NULL) % getpid ());
        first_call = false;
    }
    usleep (random() % t);
}

static bool bf (char start_char, char end_char, int length, int index, char p[], uint128_t solution, char* found_solution) {
	uint128_t hash;
	hash = md5s(p, strlen(p));
	bool found = false;
	
	// If we have a match
	if(solution == hash) {
		strcpy(found_solution, p);
		found = true;	
	}
	
	// Brute Force
	if(strlen(p) < length && !found) 
	{

		for(char l = start_char; l <= end_char && !found; l++) 
		{
			char new_p[index+1];
			char insert[1];
			insert[0] = l;
			strcpy(new_p, p);
			strcat(new_p, insert);
			
			// Recurse			
			found = bf(start_char, end_char, length, index+1, new_p, solution, found_solution);
		}

	}

	return found;
	
}

