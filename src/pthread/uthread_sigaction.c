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

void uthread_emx_sighandler(int signo)
{
	pthread_t	self = pthread_self();
	/*int		sock;*/

	__sigaddset(&self->tp.sig_pending, signo);
	switch (self->state) {
		case PS_FDR_WAIT:
		case PS_FDW_WAIT:
			/*sock = _getsockhandle(self->data.fd.fd);*/
			/* if blocked in some socket, call so_cancel */
			/*if (sock>0)*/
			uthread_so_cancel(self->data.fd.fd);
			break;
		case PS_SELECT_WAIT:
		case PS_SLEEP_WAIT:
		case PS_SIGWAIT:
			DosPostEventSem(self->data.sleep.abort);
		default:
			break;
	}
	if (_signals_blocked)
		self->sig_pending++;
	else
		uthread_deliver_signal(&self->tp,signo);

	signal(signo,SIG_ACK);
}


int pthread_sigaction(int signo, const struct sigaction *iact, struct sigaction *oact)
{
	struct sigaction	output, temp, *s;
	pthread_t		self = pthread_self();
	
	if (signo < 1 || signo >= NSIG || signo == SIGKILL || !_sys_sig_valid[signo]) {
		errno = EINVAL;
		return -1;
	}
	s = &self->tp.signals[signo];
	output = *s;
	if (iact != NULL) {
		*s = *iact; temp = *iact;

		if ((s->sa_handler == SIG_DFL && sig_info[signo].dfl_action != ST_IGNORE)
		    || s->sa_handler == SIG_IGN)
			__sigdelset(&self->tp.sig_pending, signo);

		/*temp.sa_handler = uthread_emx_sighandler;
		sigaction(signo,&temp,NULL);*/
	}
	if (oact != NULL) 
		*oact = output;
	return 0;
}

#endif
