/*
 * Copyright (c) 1995 John Birrell <jb@cimlogic.com.au>.
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
#ifdef _THREAD_SAFE
#include <pthread.h>
#include "pthread_private.h"

struct pthread * _thread_init(void)
{
	PTIB		ptib;
	ULONG		ulpri;
	struct pthread	*thread;
	
	if (thread = _thread_run) return (thread);
	
	/* first time ! */									
	_thread_run = thread = (pthread_t) malloc(sizeof(struct pthread));		
	thread->start_routine = NULL;							
	thread->arg = NULL;								
	thread->parent_thread = NULL;							
	thread->self = _thread();							
	thread->threadid = *_threadid;							
	thread->pthread_priority = 32;

	pthread_init();
												
	/* Copy the default attributes: */							
	memcpy(&thread->attr, &_pthread_attr_default, sizeof(struct pthread_attr));	

	DosGetInfoBlocks(&ptib, NULL);
	ulpri = ptib->tib_ptib2->tib2_ulpri;
	ulpri = (ulpri & 0x001f) | (((ulpri & 0xff00)>>3) - 32);
	if (ulpri > PTHREAD_MAX_PRIORITY) ulpri = PTHREAD_MAX_PRIORITY;
	thread->pthread_priority = ulpri;
												
	/* Initialise the join queue for the new thread: */				
	/* _thread_queue_init(&(new_thread->join_queue)); */					
												
	/* Initialise hooks in the thread structure: */						
	thread->specific_data = NULL;							
	thread->cleanup = NULL;							
	thread->queue = NULL;								
	thread->qnxt = NULL;								
	thread->flags = 0;								
											
	/* Add the thread to the linked list of all threads: */					
	_thread_kern_enter_crit_sec();								
	thread->nxt = _thread_link_list;						
	_thread_link_list = thread;							
	_thread_kern_leave_crit_sec();								
												
	PTHREAD_NEW_STATE(thread,PS_RUNNING);						

	return (thread);
}

pthread_t
pthread_self(void)
{
	int		status = 0;
	struct pthread	*thread_ptr;

	PTHREAD_INIT;

	if ((thread_ptr = _thread_run) == NULL) {
		/* Strange, but we need to initialise this uninitialised structure */
		_thread_kern_sig_block(&status);		
		thread_ptr = _thread_init();
		_thread_kern_sig_unblock(status);
	}

	/* Return the running thread pointer: */
	return (thread_ptr);
}
#endif
