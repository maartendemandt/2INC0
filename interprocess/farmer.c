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

#include "settings.h"
#include "common.h"

#define STUDENT_NAME        "your_name_here"

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
	pid_t               processID[NROF_WORKERS];      /* Process ID from fork() */
	mqd_t               mq_fd_request;
	mqd_t               mq_fd_response;
	MQ_REQUEST_MESSAGE  req;
	MQ_RESPONSE_MESSAGE rsp;
	struct mq_attr      attr;

	sprintf (mq_name1, "/mq_request_%s_%d", STUDENT_NAME, getpid());
	sprintf (mq_name2, "/mq_response_%s_%d", STUDENT_NAME, getpid());

	attr.mq_maxmsg  = 10;
	attr.mq_msgsize = sizeof (MQ_REQUEST_MESSAGE);
	mq_fd_request = mq_open (mq_name1, O_WRONLY | O_CREAT | O_EXCL, 0600, &attr);

	attr.mq_maxmsg  = 10;
	attr.mq_msgsize = sizeof (MQ_RESPONSE_MESSAGE);
	mq_fd_response = mq_open (mq_name2, O_RDONLY | O_CREAT | O_EXCL, 0600, &attr);

	
	//  * create the child processes (see process_test() and message_queue_test())     

    	printf ("parent pid:%d\n", getpid());
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
			printf ("child  pid:%d\n", getpid());
			
            		execlp ("./worker", "", "", NULL);
			// or try this one:
			//execlp ("./interprocess_basics", "my_own_name_for_argv0", "first_argument", NULL);

			// we should never arrive here...
			perror ("execlp() failed");
		}
        // else: we are still the parent (which continues this program)
        }
        waitpid (processID[i], NULL, 0);   // wait for the child
        printf ("child %d has been finished\n\n", processID[i]);
    }
	//  * do the farming
	for(char l=ALPHABET_START_CHAR;l<=ALPHABET_END_CHAR;l++)
	{
		for(int j=0;j<MD5_LIST_NROF;j++)
		{
			printf ("parent: sending...%c\n",ALPHABET_START_CHAR+1);
			mq_send (mq_fd_request, (char *) &req, sizeof (req), 0);
		}		
	}

	// read the result and store it in the response message
	printf ("parent: receiving...\n");
	mq_receive (mq_fd_response, (char *) &rsp, sizeof (rsp), NULL);
		
	//  * wait until the chilren have been stopped (see process_test())
	for(int i=0;i<NROF_WORKERS;i++) 
	{
		waitpid (processID[i], NULL, 0);   // wait for the child
	}
	//  * clean up the message queues (see message_queue_test())   
	mq_close (mq_fd_response);
	mq_close (mq_fd_request);
	mq_unlink (mq_name1);
	mq_unlink (mq_name2); 

	// Important notice: make sure that the names of the message queues contain your
	// student name and the process id (to ensure uniqueness during testing)
    
    return (0);
}

