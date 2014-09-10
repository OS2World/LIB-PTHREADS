/*
 * Copyright (c) 1997 John Birrell <jb@cimlogic.com.au>.
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
#include <signal.h>
#ifdef _THREAD_SAFE
#include <pthread.h>
#include "pthread_private.h"

int
pthread_kill(pthread_t pthread, int signo)
{
	int             rval = 0;
	int             status = 0;
	/*int		sock;*/
	pthread_t	p_pthread, self = pthread_self();

	/* Check for invalid signal numbers: */
	if (signo < 0 || signo >= NSIG)
		/* Invalid signal: */
		rval = EINVAL;
	else {
		/* Assume that the search will succeed: */
		rval = 0;

		/* Block signals: */
		_thread_kern_sig_block(&status);

		/* Search for the thread: */
		p_pthread = _thread_link_list;
		while (p_pthread != NULL && p_pthread != pthread) {
			p_pthread = p_pthread->nxt;
		}

		/* Check if the thread was not found: */
		if (p_pthread == NULL)
			/* Can't find the thread: */
			rval = ESRCH;
		else {
			if (signo > 0 && signo < NSIG && _sys_sig_valid[signo] &&
				p_pthread->tp.signals[signo].sa_handler != SIG_IGN)
			switch (p_pthread->state) {
				case PS_FDR_WAIT:
				case PS_FDW_WAIT:
					/*sock = _getsockhandle(p_pthread->data.fd.fd);*/
					__sigaddset(&p_pthread->tp.sig_pending, signo);
					/* if blocked in some socket, call so_cancel */
					if (uthread_so_cancel(p_pthread->data.fd.fd))
						p_pthread->sig_pending++;
					break;
				case PS_SELECT_WAIT:
					__sigaddset(&p_pthread->tp.sig_pending, signo);
					p_pthread->sig_pending++;
					DosPostEventSem(p_pthread->data.sleep.abort);
					break;
				case PS_SLEEP_WAIT:
				case PS_SIGWAIT:
					__sigaddset(&p_pthread->tp.sig_pending, signo);
					DosPostEventSem(p_pthread->data.sleep.abort);
					break;
#if 1
				case PS_RUNNING:
					if (p_pthread->threadid == 1) {
						kill(getpid(), signo);
					} else {
						/* Increment the pending signal count: */
						__sigaddset(&p_pthread->tp.sig_pending, signo);
						p_pthread->sig_pending++;
					}
					break;
#endif
				default:
					/* Increment the pending signal count: */
					__sigaddset(&p_pthread->tp.sig_pending, signo);
					p_pthread->sig_pending++;
					break;
			}
		}

		/* Unblock signals: */
		_thread_kern_sig_unblock(status);
	}

	if (self->sig_pending && !_signals_blocked) {
	  self->sig_pending = 0;
	  _sys_deliver_pending_signals(&self->tp);
	}

	/* Return the completion status: */
	return (rval);
}
#endif
