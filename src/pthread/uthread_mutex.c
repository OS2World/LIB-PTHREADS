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
#include <errno.h>
#ifdef _THREAD_SAFE
#include <pthread.h>
#include "pthread_private.h"

int
pthread_mutex_init(pthread_mutex_t * mutex,
		   const pthread_mutexattr_t * mutex_attr)
{
	enum pthread_mutextype type;
	pthread_mutex_t	pmutex;
	int             ret = 0;
	int             status = 0;
	HMTX		mtx;
	APIRET		rc;

	PTHREAD_INIT;

	if (mutex == NULL) {
		errno = EINVAL;
		ret = -1;
	} else {
		/* Check if default mutex attributes: */
		if (mutex_attr == NULL || *mutex_attr == NULL) {
			/* Default to a fast mutex: */
			type = MUTEX_TYPE_FAST;
		} else if ((*mutex_attr)->m_type >= MUTEX_TYPE_MAX) {
			/* Return an invalid argument error: */
			errno = EINVAL;
			ret = -1;
		} else {
			/* Use the requested mutex type: */
			type = (*mutex_attr)->m_type;
		}

		/* Check no errors so far: */
		if (ret == 0) {
			if ((pmutex = (pthread_mutex_t) malloc(sizeof(struct pthread_mutex))) == NULL) {
				errno = ENOMEM;
				ret = -1;
			} else {
				/* Reset the mutex flags: */
				pmutex->m_flags = 0;


				/* Process according to mutex type: */
				switch (type) {
				/* Fast mutex: */
				case MUTEX_TYPE_FAST:
					/* Nothing to do here. */
					break;

				/* Counting mutex: */
				case MUTEX_TYPE_COUNTING_FAST:
					/* Reset the mutex count: */
					pmutex->m_data.m_count = 0;
					break;

				/* Trap invalid mutex types: */
				default:
					/* Return an invalid argument error: */
					errno = EINVAL;
					ret = -1;
					break;
				}

				if (ret == 0)
					rc = DosCreateMutexSem(NULL,&mtx,0,0);

				if (ret == 0 && rc == 0) {
					/* Initialise the rest of the mutex: */
					pmutex->m_flags |= MUTEX_FLAGS_INITED;
					pmutex->m_owner = NULL;
					pmutex->m_type = type;
					pmutex->m_mutex = mtx;
					*mutex = pmutex;
				} else {
					free(pmutex);
					*mutex = NULL;
				}
			}
		}
	}
	/* Return the completion status: */
	return (ret);
}

int
pthread_mutex_destroy(pthread_mutex_t * mutex)
{
	int             ret = 0;
	int             status = 0;
	APIRET		rc = 0;
	HMTX		mtx;

	PTHREAD_INIT;

	if (mutex == NULL || *mutex == NULL) {
		errno = EINVAL;
		ret = -1;
	} else {


		/* Process according to mutex type: */
		switch ((*mutex)->m_type) {
		/* Fast mutex: */
		case MUTEX_TYPE_FAST:
			/* Nothing to do here. */
			break;

		/* Counting mutex: */
		case MUTEX_TYPE_COUNTING_FAST:
			/* Reset the mutex count: */
			(*mutex)->m_data.m_count = 0;
			break;

		/* Trap undefined mutex types: */
		default:
			/* Return an invalid argument error: */
			errno = EINVAL;
			ret = -1;
			break;
		}

		/* Clean up the mutex in case that others want to use it: */

		mtx = (*mutex)->m_mutex;
		(*mutex)->m_type = 0;
		(*mutex)->m_flags = 0;
		(*mutex)->m_mutex = 0;

		do {
			rc = DosCloseMutexSem(mtx);
			if (rc == 301) DosReleaseMutexSem(mtx);
		} while (rc == 301);

		(*mutex)->m_owner = NULL;
	}

	/* Return the completion status: */
	return (ret);
}

int
pthread_mutex_trylock(pthread_mutex_t * mutex)
{
	int             ret = 0;
	int             status = 0;
	pthread_t	current = pthread_self();
	APIRET		rc = 0;

	if (mutex == NULL || *mutex == NULL) {
		errno = EINVAL;
		ret = -1;
	} else {

		/* Process according to mutex type: */
		switch ((*mutex)->m_type) {
		/* Fast mutex: */
		case MUTEX_TYPE_FAST:
			if ((*mutex)->m_owner == current) {
				/* Already locked by self! */
				errno = EBUSY;
				return (EBUSY);
			}
			/* Check if this mutex is not locked: */
			rc = DosRequestMutexSem((*mutex)->m_mutex,SEM_IMMEDIATE_RETURN);
			if (rc == 0) {
				/* Lock the mutex for the running thread: */
				(*mutex)->m_owner = current;
			} else {
				/* Return a busy error: */
				errno = EBUSY;
				ret = -1;
			}
			break;

		/* Counting mutex: */
		case MUTEX_TYPE_COUNTING_FAST:
			/* Check if this mutex is locked: */
			rc = DosRequestMutexSem((*mutex)->m_mutex,SEM_IMMEDIATE_RETURN);
			if (rc == 0) {
				/* Lock the mutex for the running thread: */
				(*mutex)->m_owner = current;

				/* Reset the lock count for this mutex: */
				(*mutex)->m_data.m_count = 0;
			} else
			if (rc == 640) {
				/*
				 * Check if the mutex is locked by the running
				 * thread: 
				 */
				if ((*mutex)->m_owner == current) {
					/* Increment the lock count: */
					(*mutex)->m_data.m_count++;
				} else {
					/* Return a busy error: */
					errno = EBUSY;
					ret = -1;
				}
			} else {
				/*printf("pthread_mutex_trylock() : rc=%d\n",rc);*/
				errno = EINVAL;
				ret = -1;
			}
			break;

		/* Trap invalid mutex types: */
		default:
			/* Return an invalid argument error: */
			errno = EINVAL;
			ret = -1;
			break;
		}

	}

	if (current->sig_pending && !_signals_blocked) {
		current->sig_pending = 0;
		_sys_deliver_pending_signals(&current->tp);
	}

	/* Return the completion status: */
	return (ret);
}

int
pthread_mutex_lock(pthread_mutex_t * mutex)
{
	int             ret = 0;
	int             status = 0;
	APIRET		rc = 0;
	pthread_t	current = pthread_self();

	if (mutex == NULL || *mutex == NULL) {
		errno = EINVAL;
		ret = -1;
	} else {

		/* Process according to mutex type: */
		switch ((*mutex)->m_type) {
		/* Fast mutexes do not check for any error conditions: */
		case MUTEX_TYPE_FAST:
			if ((*mutex)->m_owner == current) {
				/* Double lock detected! */
				errno = EDEADLK;
				return (EDEADLK);
			}
			/*
			 * Enter a loop to wait for the mutex to be locked by the
			 * current thread: 
			 */
			current->data.mutex = mutex;
			current->state = PS_MUTEX_WAIT;
			rc = DosRequestMutexSem((*mutex)->m_mutex,SEM_INDEFINITE_WAIT);
			current->state = PS_RUNNING;
			if (rc == 0) {
				/* Lock the mutex for this thread: */
				(*mutex)->m_owner = current;
			} else {
				/*printf("pthread_mutex_lock() : rc=%d\n",rc);*/
				errno = EINVAL;
				ret = EINVAL;
			}
			break;

		/* Counting mutex: */
		case MUTEX_TYPE_COUNTING_FAST:
			/*
			 * Enter a loop to wait for the mutex to be locked by the
			 * current thread: 
			 */
			current->data.mutex = mutex;
			current->state = PS_MUTEX_WAIT;
			rc = DosRequestMutexSem((*mutex)->m_mutex,SEM_IMMEDIATE_RETURN);
			if (rc == 640) {
				/*
				 * Check if the mutex is locked by the running
				 * thread: 
				 */
				if ((*mutex)->m_owner == current) {
					/* Increment the lock count for this mutex: */
					(*mutex)->m_data.m_count++;
				} else
					rc = DosRequestMutexSem((*mutex)->m_mutex,SEM_INDEFINITE_WAIT);
			}
			current->state = PS_RUNNING;
			if (rc == 0) {
				/* Lock the mutex for this thread: */
				(*mutex)->m_owner = current;

				/* Reset the lock count for this mutex: */
				(*mutex)->m_data.m_count = 0;
			} else 
			if (rc != 640) {
				/*printf("pthread_mutex_lock() : rc=%d\n",rc);*/
				errno = EINVAL;
				ret = EINVAL;
			}
			break;

		/* Trap invalid mutex types: */
		default:
			/* Return an invalid argument error: */
			errno = EINVAL;
			ret = -1;
			break;
		}

	}

	if (current->sig_pending && !_signals_blocked) {
		current->sig_pending = 0;
		_sys_deliver_pending_signals(&current->tp);
	}

	/* Return the completion status: */
	return (ret);
}

int
pthread_mutex_unlock(pthread_mutex_t * mutex)
{
	int             ret = 0;
	APIRET		rc = 0;
	int             status;
	pthread_t	current = pthread_self();

	PTHREAD_INIT;

	if (mutex == NULL || *mutex == NULL) {
		errno = EINVAL;
		ret = -1;
	} else {

		if ((*mutex)->m_owner == NULL) {
			/* Double unlock error! */
			errno = EPERM;
			return (EPERM);
		}

		/* Process according to mutex type: */
		switch ((*mutex)->m_type) {
		/* Fast mutexes do not check for any error conditions: */
		case MUTEX_TYPE_FAST:
			/* Check if the running thread is not the owner of the mutex: */
			if ((*mutex)->m_owner != current) {
				/* Return an invalid argument error: */
				errno = EINVAL;
				ret = -1;
			}
			/*
			 * Get the next thread from the queue of threads waiting on
			 * the mutex: 
			 */
			else {
				(*mutex)->m_owner = NULL;
				rc = DosReleaseMutexSem((*mutex)->m_mutex);
				/*if (rc) printf("unlock error %d\n",rc);*/
				/* if (rc == 0 && (*mutex)->m_owner == _thread_run) 
					(*mutex)->m_owner = NULL;*/
			}
			break;

		/* Counting mutex: */
		case MUTEX_TYPE_COUNTING_FAST:
			/* Check if the running thread is not the owner of the mutex: */
			if ((*mutex)->m_owner != current) {
				/* Return an invalid argument error: */
				errno = EINVAL;
				ret = -1;
			}
			/* Check if there are still counts: */
			else if ((*mutex)->m_data.m_count) {
				/* Decrement the count: */
				(*mutex)->m_data.m_count--;
			}
			/*
			 * Get the next thread from the queue of threads waiting on
			 * the mutex: 
			 */
			else {
				(*mutex)->m_owner = NULL;
				rc = DosReleaseMutexSem((*mutex)->m_mutex);
				/*if (rc) printf("unlock error %d\n",rc);*/
				/*if (rc == 0 && (*mutex)->m_owner == _thread_run) 
					(*mutex)->m_owner = NULL;*/
			}
			break;

		/* Trap invalid mutex types: */
		default:
			/* Return an invalid argument error: */
			errno = EINVAL;
			ret = -1;
			break;
		}

	}

	/* Return the completion status: */
	return (ret);
}
#endif
