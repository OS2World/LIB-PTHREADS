/*
 * Copyright 1998 Antony T Curtis <antony.curtis@olcs.net>
 * Use restricted to and permitted only for OS/2. Minimum royalty 
 * for use on Microsoft platforms at $1000 per annum per seat.
 *
 * This library is to override existing calls so that they may be 
 * aborted by signals from other threads using the mechanisms 
 * within pthreads
 */
#include <errno.h>
#include <signal.h>
#include <sys/ioctl.h>
#ifdef _THREAD_SAFE
#include <pthread.h>
#include "pthread_private.h"

/* 
 *	This code is damned ugly... Maybe someone would
 *	like to tidy it up?
 */


typedef struct _thread_select_st {
	int		nfds;
	int		isrdfds;
	struct _fd_set *readfds;
	int		iswrfds;
	struct _fd_set *writefds;
	int		isexfds;
	struct _fd_set *exceptfds;
	int		istimeout;
	struct timeval	timeout;
	volatile HEV	event;
	int		result;
	int		select_errno;
	volatile int	done;
} *pthread_select_t;
	
void _thread_select(void * arg)
{
	pthread_select_t	self = arg;
	int			result, chkstdin;
	/*int			notexpire = 1;*/
	struct _fd_set		readfds;
	struct _fd_set		writefds;
	struct _fd_set		exceptfds;
	HEV			event = self->event;

	chkstdin = (self->isrdfds && FD_ISSET(0,self->readfds))?1:0;

	do {
		struct timeval	timeout = {0L,0L};


		if (self->isrdfds) readfds = *self->readfds;
		if (self->iswrfds) writefds = *self->writefds;
		if (self->isexfds) exceptfds = *self->exceptfds;
		
		if (chkstdin) FD_CLR(0,&readfds);

		/*if (notexpire && self->istimeout) {
			if (self->timeout.tv_usec) {
				timeout.tv_usec = self->timeout.tv_usec;
				timeout.tv_sec = 0;
				self->timeout.tv_usec = 0;
			} else
			if (self->timeout.tv_sec)
				self->timeout.tv_sec -= timeout.tv_sec;
			if (self->timeout.tv_sec == 0)
				notexpire = 0;
		}*/

		result = select(
			self->nfds, 
			self->isrdfds?&readfds:NULL,
			self->iswrfds?&writefds:NULL,
			self->isexfds?&exceptfds:NULL,
			&timeout);

		if (chkstdin) {
			int charcount = 0, res;
			res = ioctl(0,FIONREAD,&charcount);
			if (res==0 && charcount>0) FD_SET(0,&readfds);
		}
				

		if (result>0) {
			self->done++;
			if (self->isrdfds) *self->readfds = readfds;
			if (self->iswrfds) *self->writefds = writefds;
			if (self->isexfds) *self->exceptfds = exceptfds;
		} else
		if (result) self->done++;
		else DosSleep(1);

	} while (self->event!=0 && self->done==0);

	if (self->event) {
		self->select_errno = (result < 0)?errno:0;
		self->result = result;
		self->done = 3;
		DosPostEventSem(event);
	} else {
		self->done = 3;
		free(self);
	}

}

int pthread_select (int nfds, struct _fd_set *readfds, struct _fd_set *writefds, struct _fd_set *exceptfds,
		struct timeval *timeout){
	pthread_t	self = pthread_self();
	pthread_select_t sel;
	HEV		ev = 0;
	HTIMER		timer = 0;
	int		result = 0;
	APIRET		rc;
	unsigned long	msecs = SEM_INDEFINITE_WAIT;

	if (self->sig_pending) {
		self->sig_pending = 0;
      		_sys_deliver_pending_signals(&self->tp);
	}

	if (timeout) {
		if (timeout->tv_sec != 0 || timeout->tv_usec != 0) 
			msecs = (timeout->tv_sec * 1000L) + (timeout->tv_usec / 1000L);
		else
			msecs = SEM_IMMEDIATE_RETURN;
	};

#if 1

	if (!(sel = (pthread_select_t) malloc(sizeof(struct _thread_select_st)))) {
		result = -1;
		errno = ENOMEM;
	} else {
		sel->nfds = nfds;
		sel->isrdfds = readfds?1:0;
		if (sel->isrdfds) sel->readfds = readfds;
		sel->iswrfds = writefds?1:0;
		if (sel->iswrfds) sel->writefds = writefds;
		sel->isexfds = exceptfds?1:0;
		if (sel->isexfds) sel->exceptfds = exceptfds;
		sel->istimeout = timeout?1:0;
		if (sel->istimeout) sel->timeout = *timeout;
	
		rc = DosCreateEventSem(NULL,&ev,0,FALSE);

		sel->event = ev; sel->done = 0;

		if (_beginthread(_thread_select,NULL,65536,(void *)sel) == -1) {
			result = -1; sel->event = 0;
			DosCloseEventSem(ev);
		} else {
			self->data.sleep.abort = ev;
			self->state = PS_SELECT_WAIT;
			rc = DosWaitEventSem(ev,msecs);
			self->state = PS_RUNNING;
			if (!sel->done) {	/* Interrupted by other thread or timeout */
				sel->event = 0;
				/*if (sel->istimeout) *timeout = sel->timeout;*/
				result = -1;
				errno = ETIMEDOUT;
				
			} else {
				while (sel->done && sel->done != 3) {
					DosSleep(1);
				}
				sel->event = 0;
				/*if (sel->istimeout) *timeout = sel->timeout;*/
				result = sel->result;
				if (sel->select_errno) errno = sel->select_errno;
				free(sel);
			}
			rc = DosCloseEventSem(ev);
		}
	}
			
#else
	if (msecs > 0) {
		DosCreateEventSem(NULL,&ev,DC_SEM_SHARED,FALSE);
		DosAsyncTimer(msecs,(HSEM)ev,&timer);
	} else {
		DosCreateEventSem(NULL,&ev,0,FALSE);
	}
		
	self->data.sleep.abort = ev;
	self->state = PS_SELECT_WAIT;
	do {
		struct timeval	my_timeout = {1L,0L};
		result = select(nfds,readfds,writefds,exceptfds,&my_timeout);
		rc = DosWaitEventSem(ev,SEM_IMMEDIATE_RETURN);
	} while (rc != 0 && result == 0);
	self->state = PS_RUNNING;
	if (!rc && timer) DosStopTimer(timer);
	if (ev) DosCloseEventSem(ev);
#endif

	if (self->sig_pending) {
		/* Signal has occured */
		self->sig_pending = 0;
      		_sys_deliver_pending_signals(&self->tp);
		if (result <= 0) {
			result = -1;
			errno = EINTR;
		}
	};
	
	return (result);
}

#endif