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
#ifdef _THREAD_SAFE
#include <pthread.h>
#include "pthread_private.h"

int
pthread_join(pthread_t pthread, void **thread_return)
{
	int             rval = 0;
	int             status = 0;
	TID		tid;
	pthread_t	pthread1;

	PTHREAD_INIT;

	/* Block signals: */
	_thread_kern_sig_block(&status);

	/* Point to the first thread in the list: */
	pthread1 = _thread_link_list;

	/* Search for the thread to join to: */
	while (pthread1 != NULL && pthread1 != pthread) {
		/* Point to the next thread: */
		pthread1 = pthread1->nxt;
	}

	if (pthread1 == NULL) {
		/* Point to the first thread in the dead thread list: */
		pthread1 = _thread_dead;

		/* Search for the thread to join to: */
		while (pthread1 != NULL && pthread1 != pthread) {
			/* Point to the next thread: */
			pthread1 = pthread1->nxt;
		}
	}

	if (pthread1 == NULL) {
		/* Return an error: */
		errno = ESRCH;
		rval = -1;

	/* Check if this thread has been detached: */
	} else if ((pthread->attr.flags & PTHREAD_DETACHED) != 0) {
		/* Return an error: */
		errno = ESRCH;
		rval = -1;
	}
	/* Check if the thread is not dead: */
	else if (pthread->state != PS_DEAD) {
		/* Add the running thread to the join queue: */
		/* _thread_queue_enq(&(pthread->join_queue), _thread_run); */
		_thread_kern_sig_unblock(status);

		/* Schedule the next thread: */
		/* _thread_kern_sched_state(PS_JOIN, __FILE__, __LINE__); */
		PTHREAD_NEW_STATE(_thread_run, PS_JOIN);
		tid = (pthread)->threadid;
		DosWaitThread(&tid, DCWW_WAIT);
		PTHREAD_NEW_STATE(_thread_run, PS_RUNNING);

		/* Block signals again: */
		_thread_kern_sig_block(&status);

		/* Check if the thread is not detached: */
		if ((pthread->attr.flags & PTHREAD_DETACHED) == 0) {
			/* Check if the return value is required: */
			if (thread_return) {
				/* Return the thread's return value: */
				*thread_return = pthread->ret;
			}
		} else {
			/* Return an error: */
			errno = ESRCH;
			rval = -1;
		}
	} else {
		/* Check if the return value is required: */
		if (thread_return != NULL) {
			/* Return the thread's return value: */
			*thread_return = pthread->ret;
		}
	}

	/* Unblock signals: */
	_thread_kern_sig_unblock(status);

	/* Return the completion status: */
	return (rval);
}
#endif
