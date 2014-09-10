/*
 * Copyright (c) 1995 John Birrell <jb@cimlogic.com.au>.
 * All rights reserved.
 *
 * Modified and extended by Antony T Curtis <antony.curtis@olcs.net>
 * for use with OS/2.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by John Birrell.
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY JOHN BIRRELL AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#ifdef _THREAD_SAFE
#include <pthread.h>
#include "pthread_private.h"

struct _create {
	pthread_t		newthread, parent, creator;
	pthread_attr_t		attr;
	pthread_startroutine_t	start_routine;
	void *			start_arg;
	HEV			ack;
};

void _thread_create2(void *arg)
{
	struct _create	*create_info = arg;
	pthread_t	thread;

	create_info->newthread = _thread_run = thread = (pthread_t) malloc(sizeof(struct pthread));
	thread->start_routine = create_info->start_routine;
	thread->arg = create_info->start_arg;
	thread->parent_thread = create_info->parent;
	thread->self = _thread();
	thread->threadid = *_threadid;
	thread->pthread_priority = 32;

	pthread_init();

	DosPostEventSem(create_info->ack);
	
	/* Copy the thread attributes: */
	memcpy(&thread->attr, create_info->attr, sizeof(struct pthread_attr));

	/*
	 * Check if this thread is to inherit the scheduling
	 * attributes from its parent: 
	 */
	if (thread->attr.flags & PTHREAD_INHERIT_SCHED) {
		/* Copy the scheduling attributes: */
		thread->pthread_priority = create_info->creator->pthread_priority;
		thread->attr.prio = create_info->creator->pthread_priority;
		thread->attr.schedparam_policy = create_info->creator->attr.schedparam_policy;
	} else {
		/*
		 * Use just the thread priority, leaving the
		 * other scheduling attributes as their
		 * default values: 
		 */
		thread->pthread_priority = create_info->creator->attr.prio;
	}

	/* Initialise the join queue for the new thread: */
	/* _thread_queue_init(&(new_thread->join_queue)); */

	/* Initialise hooks in the thread structure: */
	thread->specific_data = NULL;
	thread->cleanup = NULL;
	thread->queue = NULL;
	thread->qnxt = NULL;
	thread->flags = 0;

	/* Add the thread to the linked list of all threads: */
	_thread_kern_enter_crit_sec();
	thread->nxt = _thread_link_list;
	_thread_link_list = thread;
	_thread_kern_leave_crit_sec();


	pthread_setprio(thread,thread->pthread_priority);
	/*pthread_yield();*/

	if (thread->attr.suspend == PTHREAD_CREATE_SUSPENDED) {
		PTHREAD_NEW_STATE(thread,PS_SUSPENDED);
		DosSuspendThread(thread->threadid);
	} else {
		PTHREAD_NEW_STATE(thread,PS_RUNNING);
	}

	/* Check if a parent thread was specified: */
	if (thread->parent_thread == NULL) {
		/* Schedule the new user thread: */

		/* _thread_start(); */
		/* Run the current thread's start routine with argument: */
		pthread_exit(thread->start_routine(thread->arg));

		/* This point should never be reached. */
		PANIC("Thread has resumed after exit");

	} else {
		int             sig;
		long            arg;
		void            (*sig_routine) (int);

		/*
		 * A parent thread was specified, so this is
		 * a signal handler thread which must now
		 * wait for the signal handler to complete: 
		 */
	        /*printf("new signal handler ready.");*/
		PTHREAD_NEW_STATE(thread->parent_thread,PS_SIGTHREAD);
		DosSuspendThread(thread->parent_thread->threadid);
		/* _thread_start_sig_handler(); */
		/*
		 * Cast the argument from 'void *' to a variable that is NO SMALLER
		 * than a pointer (otherwise gcc under NetBSD/Alpha will complain): 
		 */
		arg = (long) _thread_run->arg;	

		/* Cast the argument as a signal number: */
		sig = (int) arg;

		/* Cast a pointer to the signal handler function: */
		sig_routine = (void (*) (int)) _thread_run->start_routine;

		/* Call the signal handler function: */
		(*sig_routine) (sig);

		/* Exit the signal handler thread: */
		pthread_exit(&arg);

		/* This point should never be reached. */
		PANIC("Signal handler thread has resumed after exit");
	}
}

int
_thread_create(pthread_t * thread, const pthread_attr_t * attr,
	       void *(*start_routine) (void *), void *arg, pthread_t parent)
{
	int             i;
	int             ret = 0;
	int             status = 0;
	struct _create	create_info;
	int		stacksize;
	APIRET		rc;

	PTHREAD_INIT;

	/* Block signals: */

	create_info.newthread = NULL;
	create_info.parent = parent;
	create_info.creator = _thread_run;
	create_info.start_routine = start_routine;
	create_info.start_arg = arg;

	if (!create_info.creator) {
		/*printf("unfamilar thread! - will fail\n");*/
	}

	/* Check if default thread attributes are required: */
	if (attr == NULL || *attr == NULL) {
		/* Use the default thread attributes: */
		create_info.attr = &_pthread_attr_default;
	} else {
		create_info.attr = *attr;
	}

	/* Check if a stack was specified in the thread attributes: */
	if (!(stacksize = create_info.attr->stacksize_attr)) {
		stacksize = PTHREAD_STACK_DEFAULT;
	}

	/* Check for errors: */
	if (ret == 0) {
		create_info.ack = 0;
		if (thread == NULL)
			_beginthread(_thread_create2,NULL,stacksize,(void *)(&create_info));
		else {
			rc = DosCreateEventSem(NULL,&create_info.ack,0,FALSE);
			if (rc == 0) {
				_beginthread(_thread_create2,NULL,stacksize,(void *)(&create_info));

				/* We'll give a thread 1 second to start... */
				rc = DosWaitEventSem(create_info.ack,1000L);
			}
			if (rc == 0)
				(*thread) = create_info.newthread;
			else {
				(*thread) = NULL;
				ret = -1;
				errno = ENOMEM;
			}
		}
		if (create_info.ack)
			DosCloseEventSem(create_info.ack);
	}

	/* Return the status: */
	return (ret);
}

int
pthread_create(pthread_t * thread, const pthread_attr_t * attr,
	       void *(*start_routine) (void *), void *arg)
{
	int             ret = 0;

	/*
	 * Call the low level thread creation function which allows a parent
	 * thread to be specified: 
	 */
	ret = _thread_create(thread, attr, start_routine, arg, NULL);

	/* Return the status: */
	return (ret);
}

#endif
