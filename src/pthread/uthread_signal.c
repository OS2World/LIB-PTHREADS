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

void uthread_emx_sighandler(int signo);

void (*pthread_signal(int sig, void (*handler)(int)))(int)
{
	pthread_t	self = pthread_self();

	if (handler == SIG_ACK) {
		if (sig < 1 || sig >= NSIG || sig == SIGKILL || !_sys_sig_valid[sig]) {
			errno = EINVAL;
			return SIG_ERR;
		}
		__sigdelset(&self->tp.sig_blocked, sig);
		if (_signals_blocked)
			self->sig_pending++;
		else {
			self->sig_pending = 0;
			_sys_deliver_pending_signals(&self->tp);
		}
		return self->tp.signals[sig].sa_handler;
	} else {
		struct sigaction	isa,osa;
		isa.sa_handler = handler;
		isa.sa_mask = 0;
		isa.sa_flags = SA_ACK;
		signal(sig,uthread_emx_sighandler);
		if (pthread_sigaction(sig, &isa, &osa) != 0)
			return SIG_ERR;
		else
			return osa.sa_handler;
	}
}

#endif
