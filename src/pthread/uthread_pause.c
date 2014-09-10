/*
 * Copyright 1998 Antony T Curtis <antony.curtis@olcs.net>
 * Use restricted to and permitted only for OS/2. Minimum royalty 
 * for use on Microsoft platforms at $1000 per annum per seat.
 *
 * This library is to override existing calls so that signals may
 * emulated as unix programs expect.
 */
#include <errno.h>
#include <signal.h>
#ifdef _THREAD_SAFE
#include <pthread.h>
#include "pthread_private.h"

int pthread_pause(void)
{
	unsigned	result;
	APIRET		rc;
	HEV		ev;
	pthread_t	thread_ptr = pthread_self();

	DosCreateEventSem(NULL,&ev,0,FALSE);
	thread_ptr->data.sleep.abort = ev;
	thread_ptr->state = PS_SIGWAIT;
	DosWaitEventSem(ev,SEM_INDEFINITE_WAIT);
	thread_ptr->state = PS_RUNNING;
	thread_ptr->sig_pending = 0;
	_sys_deliver_pending_signals(&thread_ptr->tp);
	errno = EINTR;
	result = EINTR;
	DosCloseEventSem(ev);
	return (result);
}	

#endif
