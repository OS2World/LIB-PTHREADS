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

int pthread_raise(int signo)
{
	pthread_t	self = pthread_self();
	
	if (signo < 1 || signo >= NSIG || !_sys_sig_valid[signo]) {
		errno = EINVAL;
		return -1;
	}
	if (self->tp.signals[signo].sa_handler != SIG_IGN) {
		__sigaddset(&self->tp.sig_pending,signo);
		if (_signals_blocked)
			self->sig_pending++;
		else {
			self->sig_pending = 0;
			_sys_deliver_pending_signals(&self->tp);
		}
	}
	return 0;
}

#endif
