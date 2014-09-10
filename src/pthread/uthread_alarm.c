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

void * _alarm_thread(void * arg)
{
	pthread_t	client = arg;
	HEV		ev;
	
	DosWaitEventSem(ev=client->alarm_event,SEM_INDEFINITE_WAIT);
	if (client->alarm_timeout) {
		client->alarm_thread = NULL;
		client->alarm_timer = 0;
		client->alarm_event = 0;
		client->alarm_timeout = 0;
		pthread_kill(client,SIGALRM);
	} else {
		client->alarm_thread = NULL;
		client->alarm_timer = 0;
		client->alarm_event = 0;
	}
	DosCloseEventSem(ev);
	return NULL;
}
	
unsigned pthread_alarm(unsigned sec)
{
	int		ret = 0;
	pthread_t	self = pthread_self();

	if (sec == 0) { /* cancel pending alarm */
		self->alarm_timeout = 0;
		if (self->alarm_timer)
			DosStopTimer(self->alarm_timer);
		if (self->alarm_event)
			DosPostEventSem(self->alarm_event);
	} else
	if (sec > 0) {
		if (self->alarm_timer && self->alarm_thread) {
			DosStopTimer(self->alarm_timer);
		} else {
			DosCreateEventSem(NULL,&self->alarm_event,DC_SEM_SHARED,FALSE);
			pthread_create(&self->alarm_thread,NULL,_alarm_thread,(void *)self);
		}
		DosAsyncTimer(sec * 1000L,(HSEM)self->alarm_event,&self->alarm_timer);
		self->alarm_timeout = sec;
	} else {
		ret = EINVAL;
		errno = EINVAL;
	}
	return (ret);
}

#endif
