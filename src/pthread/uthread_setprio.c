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
pthread_setprio(pthread_t pthread, int prio)
{
	int             rval = 0;
	int             status;
	pthread_t       pthread_p;

	PTHREAD_INIT;

	/* Check if the priority is invalid: */
	if (prio < PTHREAD_MIN_PRIORITY || prio > PTHREAD_MAX_PRIORITY) {
		/* Return an invalid argument error: */
		errno = EINVAL;
		rval = -1;
	} else {
		/* Block signals: */
		_thread_kern_sig_block(&status);

		/* Point to the first thread in the list: */
		pthread_p = _thread_link_list;

		/* Enter a loop to search for the thread: */
		while (pthread_p != NULL && pthread_p != pthread) {
			/* Point to the next thread: */
			pthread_p = pthread_p->nxt;
		}

		/* Check if the thread pointer is NULL: */
		if (pthread == NULL || pthread_p == NULL || pthread->threadid == 0) {
			/* Return a 'search' error: */
			errno  = ESRCH;
			rval = -1;
		} else {
			/* Set the thread priority: */
			pthread->pthread_priority = prio;
			/*DosSetPriority(PRTYS_THREAD,1 + (prio/48), (prio%48)-16, pthread->threadid);*/
		}

		/* Unblock signals: */
		_thread_kern_sig_unblock(status);
	}

	/* Return the error status: */
	return (rval);
}
#endif
