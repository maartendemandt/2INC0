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
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>    
#include <unistd.h>         // for execlp
#include <mqueue.h>         // for mq

#include "md5s.c"
#include "settings.h"
#include "common.h"

#define STUDENT_NAME        "MaartenDemandt"

static char                 mq_name1[80];
static char                 mq_name2[80];


int main (int argc, char * argv[])
{
	if (argc != 1)
	{
		fprintf (stderr, "%s: invalid arguments\n", argv[0]);
	}
        
	// TODO:
	//  * create the message queues (see message_queue_test() in interprocess_basic.c)
	pid_t               processID[NROF_WORKERS];
	mqd_t               mq_fd_request;
	mqd_t               mq_fd_response;
	MQ_REQUEST_MESSAGE  req;
	MQ_RESPONSE_MESSAGE rsp;
	struct mq_attr      attr;

	sprintf (mq_name1, "/mq_request_%s_%d", STUDENT_NAME, getpid());
	sprintf (mq_name2, "/mq_response_%s_%d", STUDENT_NAME, getpid());

	attr.mq_maxmsg  = MQ_MAX_MESSAGES;
	attr.mq_msgsize = sizeof (MQ_REQUEST_MESSAGE);
	mq_fd_request = mq_open (mq_name1, O_WRONLY | O_CREAT | O_EXCL, 0600, &attr);

	attr.mq_maxmsg  = MQ_MAX_MESSAGES;
	attr.mq_msgsize = sizeof (MQ_RESPONSE_MESSAGE);
	mq_fd_response = mq_open (mq_name2, O_RDONLY | O_CREAT | O_EXCL, 0600, &attr);
	
	//  * create the child processes (see process_test() and message_queue_test())     

	for(int i=0;i<NROF_WORKERS;i++)
	{
		processID[i] = fork();
		if (processID[i] < 0)
		{
			perror("fork() failed");
			exit (1);
		}
		else
		{
			if (processID[i] == 0)
			{			
		    		execlp ("./worker","./worker", mq_name1, mq_name2, NULL);

				perror ("execlp() failed");
			}
       		}
  	 }

	//  * do the farming
	int nrof_messages = 0;
	int nrof_messages_received = 0;
	char current_char = ALPHABET_START_CHAR;
	int md5_list_index = 0;
	char output[6][MAX_MESSAGE_LENGTH + 4];
	
	// Wait until all messages are received
	while(nrof_messages_received < JOBS_NROF) 
	{
		// Start a new job
		if(nrof_messages < MQ_MAX_MESSAGES && current_char <= ALPHABET_END_CHAR) 
		{
			req.start_char = ALPHABET_START_CHAR;
			req.end_char = ALPHABET_END_CHAR;
			req.length = MAX_MESSAGE_LENGTH;
			req.first_char = current_char;
			req.md5 = md5_list[md5_list_index];
			req.list_index = md5_list_index;

			// Send a new message
			mq_send (mq_fd_request, (char *) &req, sizeof (req), 0);
			nrof_messages++;

			if(md5_list_index < MD5_LIST_NROF-1) {
				md5_list_index++;
				
			}
			else
			{
				md5_list_index = 0;
				current_char++;
			}
		}
		else 
		{
			// Read the result
			mq_receive (mq_fd_response, (char *) &rsp, sizeof (rsp), NULL);	
			if(rsp.length > 0) 
			{
				// Store the result in the output list
				strcpy(output[rsp.index], "");
				strcat(output[rsp.index], "'");
				strcat(output[rsp.index],rsp.word);
				strcat(output[rsp.index], "'\n");
				
			}			
			nrof_messages--;
			nrof_messages_received++;
		}
	}

	// Output
	for(int list_index = 0; list_index < MD5_LIST_NROF ; list_index++)
	{
		fprintf(stdout, "%s", output[list_index]);		
	}

	// * Wait untill children have been stopped
	for(int i=0;i<NROF_WORKERS;i++) {
		req.length = 0;
		mq_send (mq_fd_request, (char *) &req, sizeof (req), 0);
	}	
	for(int j=0;j<NROF_WORKERS;j++) {
		waitpid(processID[j], NULL, 0);
	}

	// * Clear the message queues
	mq_close (mq_fd_response);
	mq_close (mq_fd_request);
	mq_unlink (mq_name1);
	mq_unlink (mq_name2);   


	// Important notice: make sure that the names of the message queues contain your
	// student name and the process id (to ensure uniqueness during testing)
    
	return (0);
}

