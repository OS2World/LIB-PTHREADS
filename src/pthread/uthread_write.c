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

int pthread_write(int fd, const void *buf, size_t len)
{
  int		rc = 0;
  pthread_t	thread_ptr = pthread_self();

  thread_ptr->data.fd.fd = fd;
  thread_ptr->data.fd.branch = __LINE__;
  thread_ptr->data.fd.fname = __FILE__;
  thread_ptr->state = PS_FDW_WAIT;
  rc = write(fd,buf,len);
  thread_ptr->state = PS_RUNNING;
  if (rc==EINTR) {
    thread_ptr->sig_pending = 0;
    _sys_deliver_pending_signals(&thread_ptr->tp);
  }
  return (rc);
}

#endif
