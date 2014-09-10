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

int pthread_sigsuspend(const sigset_t *mask)
{
	sigset_t	old_blocked;
	pthread_t	self = pthread_self();

	/* TODO: Critical section */
	old_blocked = self->tp.sig_blocked;
	SET_BLOCKED(&self->tp,*mask);
	pthread_pause();
	self->tp.sig_blocked = old_blocked;
	errno = EINTR;
	return -1;
}

#endif
