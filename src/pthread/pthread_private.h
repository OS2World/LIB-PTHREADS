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
 *      This product includes software developed by John Birrell.
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
 * Private thread definitions for the uthread kernel.
 *
 */

#ifndef _PTHREAD_PRIVATE_H
#define _PTHREAD_PRIVATE_H

/*
 * Include files.
 */
#include <setjmp.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>

#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <emx/thread.h>
#include <emx/syscalls.h>
#define INCL_BASE
#define INCL_NOPMAPI
#include <os2emx.h>

#define _sys_deliver_pending_signals uthread_deliver_pending_signals

#include "../lib/sys/syscalls.h"

/*
 * Evaluate the storage class specifier.
 */
#ifdef GLOBAL_PTHREAD_PRIVATE
#define SCLASS
#else
#define SCLASS extern
#endif

/*
 * Kernel fatal error handler macro.
 */
#define PANIC(string)   _thread_exit(__FILE__,__LINE__,string)

/*
 * State change macro:
 */
#define PTHREAD_NEW_STATE(thrd, newstate) {                             \
        (thrd)->state = newstate;                                       \
        (thrd)->fname = __FILE__;                                       \
        (thrd)->lineno = __LINE__;                                      \
}

/*
 * Queue definitions.
 */
struct pthread_queue {
        struct pthread  *q_next;
        struct pthread  *q_last;
        void            *q_data;
};

/*
 * Static queue initialization values.
 */
#define PTHREAD_QUEUE_INITIALIZER { NULL, NULL, NULL }

/*
 * Mutex definitions.
 */
union pthread_mutex_data {
        void    *m_ptr;
        int     m_count;
};

struct pthread_mutex {
        enum pthread_mutextype          m_type;
	HMTX				m_mutex;
        struct pthread                  *m_owner;
        union pthread_mutex_data        m_data;
	int				m_prioceiling;
        long                            m_flags;
};

/*
 * Flags for mutexes.
 */
#define MUTEX_FLAGS_PRIVATE     0x01
#define MUTEX_FLAGS_INITED      0x02
#define MUTEX_FLAGS_BUSY        0x04

/*
 * Static mutex initialization values.
 */
/* #define PTHREAD_MUTEX_INITIALIZER   \
        { MUTEX_TYPE_FAST, PTHREAD_QUEUE_INITIALIZER, \
        NULL, { NULL }, MUTEX_FLAGS_INITED } */

struct pthread_mutex_attr {
        enum pthread_mutextype  m_type;
	int			m_prioceiling;
        long                    m_flags;
};

/*
 * Condition variable definitions.
 */
enum pthread_cond_type {
        COND_TYPE_FAST,
        COND_TYPE_MAX
};

struct pthread_cond {
        enum pthread_cond_type  c_type;
	HEV			c_event;
        void                    *c_data;
        long                    c_flags;
	long			c_onwait;
};

struct pthread_cond_attr {
        enum pthread_cond_type  c_type;
        long                    c_flags;
};

/*
 * Flags for condition variables.
 */
#define COND_FLAGS_PRIVATE      0x01
#define COND_FLAGS_INITED       0x02
#define COND_FLAGS_BUSY         0x04

/*
 * Static cond initialization values.
 */
/*#define PTHREAD_COND_INITIALIZER    \
        { COND_TYPE_FAST, 0, NULL, COND_FLAGS_INITED } */

/*
 * Cleanup definitions.
 */
struct pthread_cleanup {
        struct pthread_cleanup  *next;
        void                    (*routine) ();
        void                    *routine_arg;
};

/*
 * Scheduling definitions.
 */

struct pthread_attr {
        enum schedparam_policy  schedparam_policy;
        int                     prio;
        int                     suspend;
        int                     flags;
        void                    *arg_attr;
        void                    (*cleanup_attr) ();
        void                    *stackaddr_attr;
        size_t                  stacksize_attr;
};

struct sched_param {
        int     prio;
        void    *no_data;
};

/*
 * Thread creation state attributes.
 */
#define PTHREAD_CREATE_RUNNING                  0
#define PTHREAD_CREATE_SUSPENDED                1

/*
 * Miscellaneous definitions.
 */
#define PTHREAD_STACK_DEFAULT                   65536
#define PTHREAD_DEFAULT_PRIORITY                64
#define PTHREAD_MAX_PRIORITY                    126
#define PTHREAD_MIN_PRIORITY                    0
#define _POSIX_THREAD_ATTR_STACKSIZE

struct pthread_key {
        pthread_mutex_t mutex;
        long            count;
        void            (*destructor) ();
};

/*
 * Thread states.
 */
enum pthread_state {
        PS_RUNNING,
        PS_SIGTHREAD,
        PS_MUTEX_WAIT,
        PS_COND_WAIT,
        PS_FDLR_WAIT,
        PS_FDLW_WAIT,
        PS_FDR_WAIT,
        PS_FDW_WAIT,
        PS_SELECT_WAIT,
        PS_SLEEP_WAIT,
        PS_WAIT_WAIT,
        PS_SIGWAIT,
        PS_JOIN,
        PS_SUSPENDED,
        PS_DEAD,
        PS_STATE_MAX
};


/*
 * File descriptor locking definitions.
 */
/* #define FD_READ             0x1 */
/* #define FD_WRITE            0x2 */
/* #define FD_RDWR             (FD_READ | FD_WRITE) */

/*
 * File descriptor table structure.
 */
struct fd_table_entry {
        struct pthread_queue    r_queue;        /* Read queue.                        */
        struct pthread_queue    w_queue;        /* Write queue.                       */
        struct pthread          *r_owner;       /* Ptr to thread owning read lock.    */
        struct pthread          *w_owner;       /* Ptr to thread owning write lock.   */
        char                    *r_fname;       /* Ptr to read lock source file name  */
        int                     r_lineno;       /* Read lock source line number.      */
        char                    *w_fname;       /* Ptr to write lock source file name */
        int                     w_lineno;       /* Write lock source line number.     */
        int                     r_lockcount;    /* Count for FILE read locks.         */
        int                     w_lockcount;    /* Count for FILE write locks.        */
        int                     flags;          /* Flags used in open.                */
};

struct pthread_select_data {
        int     nfds;
        fd_set  readfds;
        fd_set  writefds;
        fd_set  exceptfds;
};

union pthread_wait_data {
        pthread_mutex_t *mutex;
        pthread_cond_t  *cond;
        const sigset_t  *sigwait;       /* Waiting on a signal in sigwait */
        struct {
                int	fd;             /* Used when thread waiting on fd */
                int	branch;         /* Line number, for debugging.    */
                char	*fname;         /* Source file name for debugging.*/
        } fd;
	struct {
		HEV	abort;		/* signal to abort sleep */
	} sleep;
        struct pthread_select_data * select_data;
};

/*
 * Thread structure.
 */

struct pthread {
        struct _thread          *self;
        ULONG                   threadid;

        /*
         * Pointer to the next thread in the thread linked list.
         */
        struct pthread  *nxt;

        /*
         * Thread start routine, argument, stack pointer and thread
         * attributes.
         */
        void                    *(*start_routine)(void *);
        void                    *arg;
        void                    *stack;
        struct pthread_attr     attr;

        /*
         * Pointer to the parent thread for which the current thread is
         * a signal handler thread, otherwise NULL if the current thread
         * is not a signal handler thread.
         */
        struct  pthread *parent_thread;

        /* Thread state: */
        volatile enum pthread_state      state;


        /* Join queue for waiting threads: */
        /* struct pthread_queue join_queue; */

        /*
         * The current thread can belong to only one queue at a time.
         *
         * Pointer to queue (if any) on which the current thread is waiting.
         */
        struct pthread_queue    *queue;

        /* Pointer to next element in queue. */
        struct pthread  *qnxt;

        /* Wait data. */
        volatile union pthread_wait_data data;

        /* Miscellaneous data. */
        char            flags;
        char            pthread_priority;
        void            *ret;
        const void      **specific_data;
        int             specific_data_count;
	
	/* Signal Handling */
	thread_data	tp;
	int		sig_pending;

	pthread_t	alarm_thread;
	HEV		alarm_event;
	HTIMER		alarm_timer;
	int		alarm_timeout;	

        /* Cleanup handlers Link List */
        struct pthread_cleanup *cleanup;
        char                    *fname; /* Ptr to source file name  */
        int                     lineno; /* Source line number.      */

	char		dlerror[128];	/* something to store dlerror messages */

};

/*
 * Global variables for the uthread kernel.
 */

/* Ptr to the first thread in the thread linked list: */
SCLASS struct pthread   * volatile _thread_link_list
#ifdef GLOBAL_PTHREAD_PRIVATE
= NULL;
#else
;
#endif

/* Ptr to the first thread: */
SCLASS struct pthread   * volatile _thread_initial
#ifdef GLOBAL_PTHREAD_PRIVATE
= NULL;
#else
;
#endif

/* Dead threads: */
SCLASS struct pthread * volatile _thread_dead
#ifdef GLOBAL_PTHREAD_PRIVATE
= NULL;
#else
;
#endif

/* Default thread attributes: */
SCLASS struct pthread_attr _pthread_attr_default
#ifdef GLOBAL_PTHREAD_PRIVATE
= { SCHED_RR, PTHREAD_DEFAULT_PRIORITY, PTHREAD_CREATE_RUNNING,
        PTHREAD_CREATE_JOINABLE, NULL, NULL, NULL, PTHREAD_STACK_DEFAULT };
#else
;
#endif

/* Default mutex attributes: */
SCLASS struct pthread_mutex_attr _pthread_mutexattr_default
#ifdef GLOBAL_PTHREAD_PRIVATE
= { MUTEX_TYPE_FAST, 0 };
#else
;
#endif

/* Default condition variable attributes: */
SCLASS struct pthread_cond_attr _pthread_condattr_default
#ifdef GLOBAL_PTHREAD_PRIVATE
= { COND_TYPE_FAST, 0 };
#else
;
#endif

int	_thread_so_cancel(int);

/* Default socket cancel function: */
SCLASS int (*uthread_so_cancel)(int)
#ifdef GLOBAL_PTHREAD_PRIVATE
= _thread_so_cancel;
#else
;
#endif

SCLASS HMTX _thread_critical
#ifdef GLOBAL_PTHREAD_PRIVATE
= 0;
#else
;
#endif

SCLASS int _signals_blocked
#ifdef GLOBAL_PTHREAD_PRIVATE
= 0;
#else
;
#endif


/* Undefine the storage class specifier: */
#undef  SCLASS
#undef  errno

#define _thread_run     ((pthread_t)(*_threadstore()))
#define errno           (_thread()->_th_errno)

/*
 * Function prototype definitions.
 */
/* char    *__ttyname_basic(int); */
/* char    *__ttyname_r_basic(int, char *, size_t); */
/* char    *ttyname_r(int, char *, size_t); */
int     _thread_create(pthread_t *,const pthread_attr_t *,pthread_startroutine_t start_routine,void *,pthread_t);
/* int     _thread_fd_lock(int, int, struct timespec *,char *fname,int lineno); */
void    _thread_exit(char *, int, char *);
/* void    _thread_fd_unlock(int, int); */
void    *_thread_cleanup(pthread_t);
void    _thread_cleanupspecific(void);
void    _thread_dump_info(void);
struct pthread * _thread_init(void);
/* void    _thread_kern_sched(struct sigcontext *); */
void    _thread_kern_sched_state(enum pthread_state,char *fname,int lineno);
/* void    _thread_kern_set_timeout(timespec_t *); */
/* void    _thread_sig_handler(int, int, struct sigcontext *); */
void    _thread_start(void);
/* void    _thread_start_sig_handler(void); */
void    _thread_seterrno(pthread_t,int);
void    _thread_queue_init(struct pthread_queue *);
void    _thread_queue_enq(struct pthread_queue *, struct pthread *);
int     _thread_queue_remove(struct pthread_queue *, struct pthread *);
struct pthread *_thread_queue_get(struct pthread_queue *);
struct pthread *_thread_queue_deq(struct pthread_queue *);

int uthread_deliver_signal(thread_data *tp, int signo);

#if 1
#define _thread_kern_sig_block(a)       {DosEnterMustComplete((ULONG *)(a)); _signals_blocked++;} 
#define _thread_kern_sig_unblock(a)     {DosExitMustComplete((ULONG *)&(a)); _signals_blocked--;}
#define _thread_kern_enter_crit_sec()   DosEnterCritSec()
#define _thread_kern_leave_crit_sec()   DosExitCritSec()
#else
#define _thread_kern_sig_block(a)       ((*a) = _signals_blocked++,0)
#define _thread_kern_sig_unblock(a)     (_signals_blocked--,0)
#define _thread_kern_enter_crit_sec()   DosRequestMutexSem(_thread_critical,SEM_INDEFINITE_WAIT); \
					/*printf("enter_crit_sec %s#%d\n",__FILE__,__LINE__)*/
#define _thread_kern_leave_crit_sec()   /*printf("leave_crit_sec %s#%d\n",__FILE__,__LINE__);*/ \
					DosReleaseMutexSem(_thread_critical)
#endif

#define PTHREAD_INIT if ((!_thread_initial) && (_gettid()==1)) {	\
	_thread_init();							\
	_thread_initial = _thread_run;					\
}

#endif  /* !_PTHREAD_PRIVATE_H */
