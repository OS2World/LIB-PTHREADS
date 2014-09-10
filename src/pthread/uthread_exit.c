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
#include <unistd.h>
#include <string.h>
#ifdef _THREAD_SAFE
#include <pthread.h>
#include "pthread_private.h"

void
_thread_exit(char *fname, int lineno, char *string)
{
	char            s[256];

	/* Prepare an error message string: */
	strcpy(s, "Fatal error '");
	strcat(s, string);
	strcat(s, "' at line ? ");
	strcat(s, "in file ");
	strcat(s, fname);
	strcat(s, " (errno = ?");
	strcat(s, ")\n");

	/* Write the string to the standard error file descriptor: */
	/* _thread_sys_write(2, s, strlen(s)); */ /* FIX ME! */

	/* Force this process to exit: */
	_exit(1);
}

void
pthread_exit(void *status)
{
	int             sig, state=0;
	long            l;
	pthread_t       pthread, self = pthread_self();

	/* close pending alarms */
	alarm(0);

	/* Block signals: */
	_thread_kern_sig_block(&state);

	/* Save the return value: */
	self->ret = status;

	while (self->cleanup != NULL) {
		pthread_cleanup_pop(1);
	}

	if (self->attr.cleanup_attr != NULL) {
		self->attr.cleanup_attr(_thread_run->attr.arg_attr);
	}
	/* Check if there is thread specific data: */
	if (self->specific_data != NULL) {
		/* Run the thread-specific data destructors: */
		_thread_cleanupspecific();
	}
	/* Check if there are any threads joined to this one: */
	/* while ((pthread = _thread_queue_deq(&(_thread_run->join_queue))) != NULL) { */
		/* Wake the joined thread and let it detach this thread: */
	/*	PTHREAD_NEW_STATE(pthread,PS_RUNNING); */
	/* } */

	/* Check if the running thread is at the head of the linked list: */
	_thread_kern_enter_crit_sec();
	if (_thread_link_list == self) {
		/* There is no previous thread: */
		_thread_link_list = self->nxt;
	} else {
		/* Point to the first thread in the list: */
		pthread = _thread_link_list;

		/*
		 * Enter a loop to find the thread in the linked list before
		 * the running thread: 
		 */
		while (pthread != NULL && pthread->nxt != self) {
			/* Point to the next thread: */
			pthread = pthread->nxt;
		}

		/* Check that a previous thread was found: */
		if (pthread != NULL) {
			/*
			 * Point the previous thread to the one after the
			 * running thread: 
			 */
			pthread->nxt = self->nxt;
		}
	}
	/*_thread_kern_leave_crit_sec();*/

	/* Check if this is a signal handler thread: */
	if (self->parent_thread != NULL) {
		/*
		 * Enter a loop to search for other threads with the same
		 * parent: 
		 */
		for (pthread = _thread_link_list; pthread != NULL; pthread = pthread->nxt) {
			/* Compare the parent thread pointers: */
			if (pthread->parent_thread == self->parent_thread) {
				/*
				 * The parent thread is waiting on at least
				 * one other signal handler. Exit the loop
				 * now that this is known. 
				 */
				break;
			}
		}

		/*
		 * Check if the parent is not waiting on any other signal
		 * handler threads and if it hasn't died in the meantime:
		 */
		if (pthread == NULL && self->parent_thread->state != PS_DEAD
		    && self->parent_thread->state != PS_RUNNING) {
			/* Allow the parent thread to run again: */
			PTHREAD_NEW_STATE(self->parent_thread,PS_RUNNING);
			DosResumeThread(self->parent_thread->threadid);
		}
		/* Get the signal number: */
		l = (long) self->arg;
		sig = (int) l;

		/* Unblock the signal from the parent thread: */
		__sigdelset(&self->parent_thread->tp.sig_blocked, sig);
	}
	/*
	 * This thread will never run again. Add it to the list of dead
	 * threads: 
	 */
	/*_thread_kern_enter_crit_sec();*/
	self->nxt = _thread_dead;
	_thread_dead = self;
	_thread_kern_leave_crit_sec();
	_thread_kern_sig_unblock(state);

	/*
	 * The running thread is no longer in the thread link list so it will
	 * now die: 
	 */
	_thread_kern_sched_state(PS_DEAD, __FILE__, __LINE__);
	_endthread();

	/* This point should not be reached. */
	PANIC("Dead thread has resumed");
}
#endif
