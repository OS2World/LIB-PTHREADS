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

int pthread_sigpending(sigset_t *set)
{
	pthread_t	self = pthread_self();

	/* TODO: Critical section */
	*set = self->tp.sig_blocked & self->tp.sig_pending;
	return 0;
}

#endif
