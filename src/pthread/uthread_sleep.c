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
#ifdef _THREAD_SAFE
#include <pthread.h>
#include "pthread_private.h"

unsigned pthread_sleep(unsigned sec)
{
  unsigned	result;
  APIRET	rc;
  HEV		ev;
  pthread_t	thread_ptr = pthread_self();

  rc = DosCreateEventSem(NULL,&ev,0,FALSE);
  if (rc)
    result = sleep(sec);
  else {
    thread_ptr->data.sleep.abort = ev;
    thread_ptr->state = PS_SLEEP_WAIT;
    rc = DosWaitEventSem(ev,sec * 1000L);
    thread_ptr->state = PS_RUNNING;
    if (rc)
      result = 0;
    else {
      thread_ptr->sig_pending = 0;
      _sys_deliver_pending_signals(&thread_ptr->tp);
      errno = EINTR;
      result = EINTR;
    }
    DosCloseEventSem(ev);
  }
  return (result);
}

unsigned pthread_sleep2(unsigned millisec)
{
  unsigned	result;
  APIRET	rc;
  HEV		ev;
  pthread_t	thread_ptr = pthread_self();

  rc = DosCreateEventSem(NULL,&ev,0,FALSE);
  if (rc)
    result = DosSleep(millisec);
  else {
    thread_ptr->data.sleep.abort = ev;
    thread_ptr->state = PS_SLEEP_WAIT;
    rc = DosWaitEventSem(ev,millisec);
    thread_ptr->state = PS_RUNNING;
    if (rc)
      result = 0;
    else {
      thread_ptr->sig_pending = 0;
      _sys_deliver_pending_signals(&thread_ptr->tp);
      errno = EINTR;
      result = EINTR;
    }
    DosCloseEventSem(ev);
  }
  return (result);
}

#endif
