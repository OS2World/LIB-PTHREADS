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
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#ifdef _THREAD_SAFE
#include <pthread.h>
#include "pthread_private.h"
#define INCL_BASE
#define INCL_NOPMAPI
#include <os2emx.h>

#ifndef ETIME
#define ETIME ETIMEDOUT  
#endif

int
pthread_cond_init(pthread_cond_t * cond, const pthread_condattr_t * cond_attr)
{
	enum pthread_cond_type type;
	pthread_cond_t	pcond;
	int             rval = 0;
	APIRET		rc = 0;
	HEV		event;

	PTHREAD_INIT;

	if (cond == NULL) {
		errno = EINVAL;
		rval = -1;
	} else {
		/*
		 * Check if a pointer to a condition variable attribute
		 * structure was passed by the caller: 
		 */
		if (cond_attr != NULL && *cond_attr != NULL) {
			/* Default to a fast condition variable: */
			type = (*cond_attr)->c_type;
		} else {
			/* Default to a fast condition variable: */
			type = COND_TYPE_FAST;
		}

		/* Process according to condition variable type: */
		switch (type) {
		/* Fast condition variable: */
		case COND_TYPE_FAST:
			/* Warp3 FP29 or Warp4 FP4 or better required */
			rc = DosCreateEventSem(NULL, &event, 0x0800, 0);
			if (rc) {
				errno = ENOMEM;
				rval = -1;
			}
			break;

		/* Trap invalid condition variable types: */
		default:
			/* Return an invalid argument error: */
			errno = EINVAL;
			rval = -1;
			break;
		}

		/* Check for no errors: */
		if (rval == 0) {
			if ((pcond = (pthread_cond_t)
			    malloc(sizeof(struct pthread_cond))) == NULL) {
				errno = ENOMEM;
				rval = -1;
			} else {
				/*
				 * Initialise the condition variable
				 * structure:
				 */
				pcond->c_onwait = 0;
				pcond->c_event = event;
				pcond->c_flags |= COND_FLAGS_INITED;
				pcond->c_type = type;
				*cond=pcond;
			}
		}
	}
	/* Return the completion status: */
	return (rval);
}

int
pthread_cond_destroy(pthread_cond_t * cond)
{
	int             rval = 0;
	APIRET		rc = 0;
	HEV		event;

	if (cond == NULL || *cond == NULL) {
		errno = EINVAL;
		rval = -1;
	} else {
		/* Process according to condition variable type: */
		event = (*cond)->c_event; 
		(*cond)->c_event = 0;
		switch ((*cond)->c_type) {
		/* Fast condition variable: */
		case COND_TYPE_FAST:
			do {
				rc = DosCloseEventSem(event);
				if (rc == 301) DosPostEventSem(event);
			} while (rc == 301);
			if (rc) {
				errno = EINVAL;
				rval = -1;
			}
			break;

		/* Trap invalid condition variable types: */
		default:
			/* Return an invalid argument error: */
			errno = EINVAL;
			rval = -1;
			break;
		}

		/* Check for errors: */
		if (rval == 0) {
			/* Destroy the contents of the condition structure: */
			(*cond)->c_flags = 0;
			free(*cond);
			*cond = NULL;
		} else
			(*cond)->c_event = event;
	}
	/* Return the completion status: */
	return (rval);
}

int
pthread_cond_wait(pthread_cond_t * cond, pthread_mutex_t * mutex)
{
	int             rval = 0;
	int             status = 0;
	APIRET		rc = 0;
	pthread_t	current = pthread_self();

	if (cond == NULL || *cond == NULL) {
		errno = EINVAL;
		rval = -1;
	} else {
		(*cond)->c_onwait++;

		/* Process according to condition variable type: */
		switch ((*cond)->c_type) {
		/* Fast condition variable: */
		case COND_TYPE_FAST:
			/*
			 * Queue the running thread for the condition
			 * variable:
			 */

			if (mutex) pthread_mutex_unlock(mutex);

			current->data.cond = cond;
			current->state = PS_COND_WAIT;
			rc = DosWaitEventSem((*cond)->c_event,SEM_INDEFINITE_WAIT);
			current->state = PS_RUNNING;

			if (rc == 0) {
				if (mutex) 
					rval = pthread_mutex_lock(mutex);
				else
					rval = 0;
			} else {
				errno = EINVAL;
				rval = -1;
			}
			break;

		/* Trap invalid condition variable types: */
		default:
			/* Return an invalid argument error: */
			errno = EINVAL;
			rval = -1;
			break;
		}

		(*cond)->c_onwait--;
	}

	if (current->sig_pending && !_signals_blocked) {
		current->sig_pending = 0;
		_sys_deliver_pending_signals(&current->tp);
	}

	/* Return the completion status: */
	return (rval);
}

int
pthread_cond_timedwait(pthread_cond_t * cond, pthread_mutex_t * mutex,
		       const struct timespec * abstime)
{
	int		rval = 0;
	int		status = 0;
	long		timeout = 0L;
	APIRET		rc = 0;
	struct timeval	curtime;
	pthread_t	current = pthread_self();

	if (abstime) {
		gettimeofday(&curtime, NULL);
		timeout = (long)(abstime->tv_sec - curtime.tv_sec)*1000L +
			  (long)((abstime->tv_nsec/1000) - curtime.tv_usec)/1000L;
		if (timeout<0) timeout = 0L;
	}

	if (cond == NULL || *cond == NULL) {
		errno = EINVAL;
		rval = -1;
	} else {

		(*cond)->c_onwait++;

		/* Process according to condition variable type: */
		switch ((*cond)->c_type) {
		/* Fast condition variable: */
		case COND_TYPE_FAST:
			/*
			 * Queue the running thread for the condition
			 * variable:
			 */
	
			if (mutex) pthread_mutex_unlock(mutex);	
	
			current->data.cond = cond;
			current->state = PS_COND_WAIT;
			rc = DosWaitEventSem((*cond)->c_event,timeout);
			current->state = PS_RUNNING;
		
			if (rc == 0) {
				if (mutex)
					rval = pthread_mutex_lock(mutex);
				else
					rval = 0;
			} else {
				errno = ETIME;
				rval = ETIME;
			}
			break;

		/* Trap invalid condition variable types: */
		default:
			/* Return an invalid argument error: */
			errno = EINVAL;
			rval = -1;
			break;
		}

		(*cond)->c_onwait--;
	}

	if (current->sig_pending && !_signals_blocked) {
		current->sig_pending = 0;
		_sys_deliver_pending_signals(&current->tp);
	}

	/* Return the completion status: */
	return (rval);
}

int
pthread_cond_signal(pthread_cond_t * cond)
{
	int             rval = 0;
	APIRET		rc = 0;
	int             status;
	pthread_t       pthread;

	if (cond == NULL || *cond == NULL) {
		errno = EINVAL;
		rval = -1;
	} else {

		/* Process according to condition variable type: */
		switch ((*cond)->c_type) {
		/* Fast condition variable: */
		case COND_TYPE_FAST:
			/* Bring the next thread off the condition queue: */
			rc = DosPostEventSem((*cond)->c_event);
			break;

		/* Trap invalid condition variable types: */
		default:
			/* Return an invalid argument error: */
			errno = EINVAL;
			rval = -1;
			break;
		}

	}

	/* Return the completion status: */
	return (rval);
}

int
pthread_cond_broadcast(pthread_cond_t * cond)
{
	int             rval = 0;
	APIRET		rc = 0;
	int             status, i;
	pthread_t       pthread;

	/* Process according to condition variable type: */
	switch ((*cond)->c_type) {
	/* Fast condition variable: */
	case COND_TYPE_FAST:
		/*
		 * Enter a loop to bring all threads off the
		 * condition queue:
		 */
		i = (*cond)->c_onwait;
		while (i--) rc = DosPostEventSem((*cond)->c_event);
		break;

	/* Trap invalid condition variable types: */
	default:
		/* Return an invalid argument error: */
		errno = EINVAL;
		rval = -1;
		break;
	}

	/* Return the completion status: */
	return (rval);
}
#endif
