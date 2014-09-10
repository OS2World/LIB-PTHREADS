/*
 * Copyright 1998 Antony T Curtis <antony.curtis@olcs.net>
 * Use restricted to and permitted only for OS/2. Minimum royalty 
 * for use on Microsoft platforms at $1000 per annum per seat.
 *
 */

#include <stdio.h>
#ifdef _THREAD_SAFE
#include <pthread.h>
#include "pthread_private.h"

char const _sys_sig_valid[NSIG] = {
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1
};

struct sig_descr const sig_info[NSIG] =
{
  {"SIG0",     ST_TERM,   ST_TERM},
  {"SIGHUP",   ST_TERM,   ST_IGNORE},
  {"SIGINT",   ST_TERM,   ST_IGNORE},
  {"SIGQUIT",  ST_TERM,   ST_IGNORE},
  {"SIGILL",   ST_NEXT,   ST_NEXT},
  {"SIGTRAP",  ST_NEXT,   ST_NEXT},
  {"SIGABRT",  ST_TERM,   ST_TERM},
  {"SIGEMT",   ST_TERM,   ST_TERM},
  {"SIGFPE",   ST_NEXT,   ST_NEXT},
  {"SIGKILL",  ST_TERM,   ST_TERM},
  {"SIGBUS",   ST_TERM,   ST_TERM},
  {"SIGSEGV",  ST_NEXT,   ST_NEXT},
  {"SIGSYS",   ST_TERM,   ST_TERM},
  {"SIGPIPE",  ST_TERM,   ST_IGNORE},
  {"SIGALRM",  ST_TERM,   ST_IGNORE},
  {"SIGTERM",  ST_NEXT,   ST_IGNORE},
  {"SIGUSR1",  ST_IGNORE, ST_IGNORE},
  {"SIGUSR2",  ST_IGNORE, ST_IGNORE},
  {"SIGCLD",   ST_IGNORE, ST_IGNORE},
  {"SIG19",    ST_TERM,   ST_TERM},
  {"SIG20",    ST_TERM,   ST_TERM},
  {"SIGBREAK", ST_TERM,   ST_IGNORE}
};

void uthread_emx_sighandler(int signo);

void uthread_terminate(int signo)
{
	pthread_t	self = pthread_self(), others;

	DosSetPriority(PRTYS_THREAD,4,0,0);
	self->state = PS_DEAD;

	if (signo == SIGABRT)
		fprintf(stderr,"\nAbnormal program termination\n");
	else
		fprintf(stderr,"\nProcess terminated by %s in thread %d\n",
			sig_info[signo].name,self->threadid);
	while (1) {
		others = _thread_initial;
		while (others)
		switch (others->state) {
		case PS_MUTEX_WAIT:
			if (others->data.mutex && *others->data.mutex) {
				HMTX mtx = (*others->data.mutex)->m_mutex;
				DosReleaseMutexSem(mtx);
			}
			others->state = PS_RUNNING;
			if (others->threadid != 1 && !DosKillThread(others->threadid))
				others->state = PS_DEAD;
			others = _thread_initial;
			break;
		case PS_FDLR_WAIT:
		case PS_FDLW_WAIT:
		case PS_FDR_WAIT:
		case PS_FDW_WAIT:
			others->sig_pending++;
			others->state = PS_RUNNING;
			if (others->threadid != 1 && !DosKillThread(others->threadid))
				others->state = PS_DEAD;
			others = _thread_initial;
			break;
		case PS_SELECT_WAIT:
		case PS_SLEEP_WAIT:
		case PS_WAIT_WAIT:
		case PS_SIGWAIT:
			DosPostEventSem(others->data.sleep.abort);
			others->state = PS_RUNNING;
			if (others->threadid != 1 && !DosKillThread(others->threadid))
				others->state = PS_DEAD;
			others = _thread_initial;
			break;
		case PS_SUSPENDED:
		case PS_SIGTHREAD:
			DosResumeThread(others->threadid);
			others->state = PS_RUNNING;
			if (others->threadid != 1 && !DosKillThread(others->threadid))
				others->state = PS_DEAD;
			others = _thread_initial;
			break;
		default:
			others = others->nxt;
			break;
		}
		DosExit(EXIT_PROCESS,3);
	}
}

int uthread_deliver_signal(thread_data *tp, int signo)
{
	struct sigaction	*p;
	void			(*handler)(int signo);
	sigset_t		mask, old_blocked;
	
	if (signo<0 || signo >= NSIG)
		return -1;

	/* TODO: Critical section */

	mask = _SIGMASK(signo);
	p = &tp->signals[signo];

	tp->sig_pending &= ~mask;
	handler = p->sa_handler;

	if (handler == SIG_IGN) {
		/* Ignore the signal */
	} else
	if (handler == SIG_DFL) {
		if (sig_info[signo].dfl_action != ST_IGNORE) 
			uthread_terminate(signo);
	} else
	if (handler == SIG_ERR) {
	} else  {
		if (p->sa_flags & SA_SYSV) {
			p->sa_handler = SIG_DFL;
			handler(signo);
		} else
		if (p->sa_flags & SA_ACK) {
			tp->sig_blocked |= mask;
			handler(signo);
		} else {
			old_blocked = tp->sig_blocked;
			SET_BLOCKED(tp, tp->sig_blocked | mask | p->sa_mask);
			handler(signo);
			tp->sig_blocked = old_blocked;
		}
	}

	if (sig_info[signo].fun_action == ST_TERM)
		uthread_terminate(signo);

	return 0;
}

void _sys_deliver_pending_signals(thread_data *tp)
{
	int	signo;

	/* TODO: Critical section */
	signo = 1;
	while ((tp->sig_pending & ~tp->sig_blocked) != 0 && signo < NSIG)
	if (tp->sig_pending & ~tp->sig_blocked & _SIGMASK(signo)) {
		uthread_deliver_signal(tp,signo);
		signo = 1;
	} else signo++;
}

int _thread_so_cancel(int sock)
{
	return	EOPNOTSUPP;
};


void pthread_init(void)
{
	int		n;
	pthread_t	self = pthread_self();
	static int	doloaddll = 1;

	if (doloaddll) {
		char	err[128];
		HMODULE sodll = 0;
		APIRET	rc;
		doloaddll = 0;
		rc = DosLoadModule((char *)&err,128,"SO32DLL",&sodll);
		if (rc == 0 && sodll != 0) {
			int	(*so_cancel)(int);
			rc = DosQueryProcAddr(sodll,0,"SO_CANCEL",&so_cancel);
			if (rc == 0) 
				uthread_so_cancel = so_cancel;
		}
		DosCreateMutexSem(NULL,&_thread_critical,0,0);
	}

	self->sig_pending = 0; 
	__sigemptyset(&self->tp.sig_blocked);
	__sigemptyset(&self->tp.sig_pending);

	for (n = 0; n < NSIG; ++n) {
		self->tp.signals[n].sa_handler = SIG_DFL;
		self->tp.signals[n].sa_flags = SA_ACK;
		__sigemptyset(&self->tp.signals[n].sa_mask);

		if (_sys_sig_valid[n])
			signal(n,uthread_emx_sighandler);
	}
}

#endif
