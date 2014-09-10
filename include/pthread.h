/*
 * Copyright (c) 1993, 1994 by Chris Provenzano, proven@mit.edu
 * Copyright (c) 1995 by John Birrell <jb@cimlogic.com.au>
 * All rights reserved
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
 *  This product includes software developed by Chris Provenzano.
 * 4. The name of Chris Provenzano may not be used to endorse or promote 
 *	  products derived from this software without specific prior written
 *	  permission.
 *
 * THIS SOFTWARE IS PROVIDED BY CHRIS PROVENZANO ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL CHRIS PROVENZANO BE LIABLE FOR ANY 
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE.
 *
 */
#ifndef _PTHREAD_H_
#define _PTHREAD_H_

#define EMX_PTHREAD_REV 4

#ifdef __MT__

/*
 * Header files.
 */
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/signal.h>
#include <sys/select.h>
#include <limits.h>
#include <unistd.h>

#if defined (__cplusplus)
extern "C" {
#endif


/*
 * Run-time invariant values:
 */
#define PTHREAD_DESTRUCTOR_ITERATIONS		4
#define PTHREAD_KEYS_MAX			256
#define PTHREAD_STACK_MIN			1024
#define PTHREAD_THREADS_MAX			ULONG_MAX

/*
 * Compile time symbolic constants for portability specifications:
 *
 * Note that those commented out are not currently supported by the
 * implementation.
 */
#define _POSIX_THREADS
/* #define _POSIX_THREAD_ATTR_STACKADDR */
/* #define _POSIX_THREAD_ATTR_STACKSIZE */
/* #define _POSIX_THREAD_PRIORITY_SCHEDULING */
/* #define _POSIX_THREAD_PRIO_INHERIT   */
/* #define _POSIX_THREAD_PRIO_PROTECT   */
/* #define _POSIX_THREAD_PROCESS_SHARED */
/* #define _POSIX_THREAD_SAFE_FUNCTIONS */

/*
 * Flags for threads and thread attributes.
 */
#define PTHREAD_DETACHED            0x1
#define PTHREAD_SCOPE_SYSTEM        0x2
#define PTHREAD_INHERIT_SCHED       0x4
#define PTHREAD_NOFLOAT             0x8

#define PTHREAD_CREATE_DETACHED     PTHREAD_DETACHED
#define PTHREAD_CREATE_JOINABLE     0
#define PTHREAD_SCOPE_PROCESS       0
#define PTHREAD_EXPLICIT_SCHED      0

/*
 * New pthread attribute types.
 */
enum schedparam_policy {
	SCHED_RR,
	SCHED_IO,
	SCHED_FIFO,
	SCHED_OTHER
};

/*
 * Forward structure definitions.
 *
 * These are mostly opaque to the user.
 */
struct pthread;
struct pthread_attr;
struct pthread_cond;
struct pthread_cond_attr;
struct pthread_mutex;
struct pthread_mutex_attr;
struct pthread_once;
struct sched_param;

/*
 * Primitive system data type definitions required by P1003.1c.
 *
 * Note that P1003.1c specifies that there are no defined comparison
 * or assignment operators for the types pthread_attr_t, pthread_cond_t,
 * pthread_condattr_t, pthread_mutex_t, pthread_mutexattr_t.
 */
typedef struct	pthread			*pthread_t;
typedef struct	pthread_attr		*pthread_attr_t;
typedef struct	pthread_mutex		*pthread_mutex_t; 
typedef struct	pthread_mutex_attr	*pthread_mutexattr_t;
typedef struct	pthread_cond		*pthread_cond_t;
typedef struct	pthread_cond_attr	*pthread_condattr_t;
typedef int     			pthread_key_t;
typedef struct	pthread_once		pthread_once_t;

/*
 * Additional type definitions:
 *
 * Note that P1003.1c reserves the prefixes pthread_ and PTHREAD_ for
 * use in header symbols.
 */
typedef void	*pthread_addr_t;
typedef void	*(*pthread_startroutine_t) (void *);
typedef struct timespec timespec_t;

/*
 * Once definitions.
 */
struct pthread_once {
	int		state;
	pthread_mutex_t	mutex;
};

/*
 * Flags for once initialization.
 */
#define PTHREAD_NEEDS_INIT  0
#define PTHREAD_DONE_INIT   1

/*
 * Static once initialization values. 
 */
#define PTHREAD_ONCE_INIT   { PTHREAD_NEEDS_INIT, NULL }

/*
 * Default attribute arguments.
 */
#ifndef PTHREAD_KERNEL
#define pthread_condattr_default    NULL
#define pthread_mutexattr_default   NULL
#define pthread_attr_default        NULL
#endif

#ifndef PTHREAD_KERNEL
#define sigprocmask(A,B,C)	pthread_sigmask((A),(B),(C))
#define signal(A,B)		pthread_signal((A),(B))
#define raise(A)		pthread_raise(A)
#define	sigaction(A,B,C)	pthread_sigaction((A),(B),(C))
#define sigpending(A)		pthread_sigpending(A)
#define sigsuspend(A)		pthread_sigsuspend(A)
#define pause()			pthread_pause()
#define alarm(A)		pthread_alarm(A)
#define	select(A,B,C,D,E)	pthread_select((A),(B),(C),(D),(E))
#define	sleep(A)		pthread_sleep(A)
#define read(A,B,C)		pthread_read((A),(B),(C))
#define write(A,B,C)		pthread_write((A),(B),(C))
#endif

enum pthread_mutextype {
	MUTEX_TYPE_FAST			= 1,
	MUTEX_TYPE_COUNTING_FAST	= 2,	/* Recursive */
	MUTEX_TYPE_MAX
};

/*
 * Thread function prototype definitions:
 */
/* __BEGIN_DECLS */
unsigned	pthread_alarm(unsigned);
int		pthread_attr_destroy (pthread_attr_t *);
int		pthread_attr_getinheritsched (pthread_attr_t *, int *);
int		pthread_attr_getschedparam (pthread_attr_t *,
			struct sched_param *);
int		pthread_attr_getschedpolicy (pthread_attr_t *, int *);
int		pthread_attr_getscope (pthread_attr_t *, int *);
int		pthread_attr_getstacksize (pthread_attr_t *, size_t *);
int		pthread_attr_getstackaddr (pthread_attr_t *, void **);
int		pthread_attr_getdetachstate (pthread_attr_t *, int *);
int		pthread_attr_init (pthread_attr_t *);
int		pthread_attr_setinheritsched (pthread_attr_t *, int);
int		pthread_attr_setprio(pthread_attr_t *, int);
int		pthread_attr_setschedparam (pthread_attr_t *,
			struct sched_param *);
int		pthread_attr_setschedpolicy (pthread_attr_t *, int);
int		pthread_attr_setscope (pthread_attr_t *, int);
int		pthread_attr_setstacksize (pthread_attr_t *, size_t);
int		pthread_attr_setstackaddr (pthread_attr_t *, void *);
int		pthread_attr_setdetachstate (pthread_attr_t *, int);
void		pthread_cleanup_pop (int execute);
void		pthread_cleanup_push (void (*routine) (void *),
			void *routine_arg);
int		pthread_condattr_destroy (pthread_condattr_t *attr);
int		pthread_condattr_init (pthread_condattr_t *attr);
int		pthread_condattr_getpshared (pthread_condattr_t *attr,
			int *pshared);
int		pthread_condattr_setpshared (pthread_condattr_t *attr,
			int pshared);
int		pthread_cond_broadcast (pthread_cond_t *);
int		pthread_cond_destroy (pthread_cond_t *);
int		pthread_cond_init (pthread_cond_t *,
			const pthread_condattr_t *);
int		pthread_cond_signal (pthread_cond_t *);
int		pthread_cond_timedwait (pthread_cond_t *,
			pthread_mutex_t *, const timespec_t * abstime);
int		pthread_cond_wait (pthread_cond_t *, pthread_mutex_t *);
int		pthread_create (pthread_t *, const pthread_attr_t *,
			pthread_startroutine_t start_routine, void *);
int		pthread_detach (pthread_t *);
int		pthread_equal (pthread_t, pthread_t);
void		pthread_exit (void *);
void		*pthread_getspecific (pthread_key_t);
void		pthread_init (void);
int		pthread_join (pthread_t, void **);
int		pthread_key_create (pthread_key_t *,
			void (*routine) (void *));
int		pthread_key_delete (pthread_key_t);
int		pthread_kill (struct pthread *, int);
int		pthread_mutexattr_destroy (pthread_mutexattr_t *);
int		pthread_mutexattr_getprioceiling (pthread_mutexattr_t *,
			int *prioceiling);
int		pthread_mutexattr_getprotocol (pthread_mutexattr_t *,
			int *protocol);
int		pthread_mutexattr_getpshared (pthread_mutexattr_t *,
			int *pshared);
int		pthread_mutexattr_init (pthread_mutexattr_t *);
int		pthread_mutexattr_setprioceiling (pthread_mutexattr_t *,
			int prioceiling);
int		pthread_mutexattr_setprotocol (pthread_mutexattr_t *,
			int protocol);
int		pthread_mutexattr_setpshared (pthread_mutexattr_t *,
			int pshared);
int		pthread_mutex_destroy (pthread_mutex_t *);
int		pthread_mutex_getprioceiling (pthread_mutex_t *);
int		pthread_mutex_init (pthread_mutex_t *,
			const pthread_mutexattr_t *);
int		pthread_mutex_lock (pthread_mutex_t *);
int		pthread_mutex_setprioceiling (pthread_mutex_t *);
int		pthread_mutex_trylock (pthread_mutex_t *);
int		pthread_mutex_unlock (pthread_mutex_t *);
int		pthread_once (pthread_once_t *,
			void (*init_routine) (void));
int		pthread_pause(void);
int		pthread_raise(int);
pthread_t	pthread_self (void);
int		pthread_setcancelstate (int, int *);
int		pthread_setcanceltype (int, int *);
int		pthread_setspecific (pthread_key_t, const void *);
void (*		pthread_signal(int, void (*)(int)))(int);
int		pthread_sigaction(int, const struct sigaction *, struct sigaction *);
int		pthread_sigmask (int, const sigset_t *, sigset_t *);
int		pthread_sigpending(sigset_t *);
int		pthread_sigsuspend(const sigset_t *);
int		pthread_testcancel (void);


int		pthread_getprio (pthread_t);
int		pthread_setprio (pthread_t, int);
void		pthread_yield (void);
/* int		pthread_setschedparam (pthread_t pthread, int policy,
			struct sched_param * param);				*/
/* int		pthread_getschedparam (pthread_t pthread, int *policy,
			struct sched_param * param);				*/
int		pthread_attr_setfloatstate (pthread_attr_t *, int);
int		pthread_attr_getfloatstate (pthread_attr_t *, int *);
int		pthread_attr_setcleanup (pthread_attr_t *,
			void (*routine) (void *), void *);

int		pthread_suspend_np(pthread_t);
int		pthread_resume_np(pthread_t);
int		pthread_attr_setcreatesuspend_np(pthread_attr_t *);
int		pthread_mutexattr_setkind_np(pthread_mutexattr_t *, int);
int		pthread_mutexattr_getkind_np(pthread_mutexattr_t);

int		pthread_select(int, struct _fd_set *, struct _fd_set *, struct _fd_set *,struct timeval *);
unsigned	pthread_sleep(unsigned);
int		pthread_read(int, void *, size_t);
int		pthread_write(int, const void *, size_t);

/* __END_DECLS */

#if defined (__cplusplus)
}
#endif
#endif
#endif
#ifndef PTHREAD_KERNEL
#define sigprocmask(A,B,C)	pthread_sigmask((A),(B),(C))
#define signal(A,B)		pthread_signal((A),(B))
#define raise(A)		pthread_raise(A)
#define	sigaction(A,B,C)	pthread_sigaction((A),(B),(C))
#define sigpending(A)		pthread_sigpending(A)
#define sigsuspend(A)		pthread_sigsuspend(A)
#define pause()			pthread_pause()
#define alarm(A)		pthread_alarm(A)
#define	select(A,B,C,D,E)	pthread_select((A),(B),(C),(D),(E))
#define	sleep(A)		pthread_sleep(A)
#define read(A,B,C)		pthread_read((A),(B),(C))
#define write(A,B,C)		pthread_write((A),(B),(C))
#endif
