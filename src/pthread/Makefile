##################################################################
#
# pthreads for OS/2
#
##################################################################

CC=gcc
AS=as
CFLAGS=-Zmt -Zbin-files -O2 -D_THREAD_SAFE -DPTHREAD_KERNEL
MKLIB=ar r
MAKE=make

CP=cp
RM=rm -f

##################################################################

HEADER= pthread.h pthread_private.h

LIB_DEP=pthreads.a

LIB_SRC=uthread_yield.c uthread_spec.c uthread_self.c uthread_create.c \
	uthread_once.c uthread_mutex.c uthread_join.c uthread_exit.c uthread_clean.c \
	uthread_condattr_destroy.c uthread_condattr_init.c uthread_cond.c \
	uthread_equal.c uthread_detach.c uthread_kern.c uthread_mutexattr_destroy.c \
	uthread_mattr_init.c uthread_mattr_kind_np.c uthread_queue.c uthread_init.c \
	uthread_kill.c uthread_suspend.c uthread_resume.c uthread_attr_setcreatesuspend.c \
	uthread_attr_destroy.c uthread_attr_getdetachstate.c uthread_attr_setdetachstate.c \
	uthread_attr_getstackaddr.c uthread_attr_getstacksize.c uthread_attr_init.c \
	uthread_attr_setstackaddr.c uthread_attr_setstacksize.c uthread_attr_setprio.c \
	uthread_sigmask.c uthread_mutexattr_prio.c uthread_locking.c \
	uthread_select.c uthread_sleep.c uthread_read.c uthread_write.c \
	uthread_signal.c uthread_raise.c uthread_sigaction.c uthread_sigpending.c \
	uthread_sigsuspend.c uthread_pause.c uthread_alarm.c \
	uthread_getprio.c uthread_setprio.c uthread_seterrno.c pthread_private.c

LIB_OBJ=uthread_yield.o uthread_spec.o uthread_self.o uthread_create.o \
	uthread_once.o uthread_mutex.o uthread_join.o uthread_exit.o uthread_clean.o \
	uthread_condattr_destroy.o uthread_condattr_init.o uthread_cond.o \
	uthread_equal.o uthread_detach.o uthread_kern.o uthread_mutexattr_destroy.o \
	uthread_mattr_init.o uthread_mattr_kind_np.o uthread_queue.o uthread_init.o \
	uthread_kill.o uthread_suspend.o uthread_resume.o uthread_attr_setcreatesuspend.o \
	uthread_attr_destroy.o uthread_attr_getdetachstate.o uthread_attr_setdetachstate.o \
	uthread_attr_getstackaddr.o uthread_attr_getstacksize.o uthread_attr_init.o \
	uthread_attr_setstackaddr.o uthread_attr_setstacksize.o uthread_attr_setprio.o \
	uthread_sigmask.o uthread_mutexattr_prio.o uthread_locking.o \
	uthread_select.o uthread_sleep.o uthread_read.o uthread_write.o \
	uthread_signal.o uthread_raise.o uthread_sigaction.o uthread_sigpending.o \
	uthread_sigsuspend.o uthread_pause.o uthread_alarm.o \
	uthread_getprio.o uthread_setprio.o uthread_seterrno.o pthread_private.o

##################################################################

all: banner headers lib
	$(MAKE) -f Makefile.mt clean
	$(MAKE) -f Makefile.mt
	$(MAKE) -f Makefile.st clean
	$(MAKE) -f Makefile.st

banner:

headers: $(HEADER)

lib:	$(LIB_DEP)

clean:
	$(RM) $(LIB_DEP) $(LIB_OBJ)
	$(MAKE) -f Makefile.mt clean
	$(MAKE) -f Makefile.st clean

/emx/include/pthread.h: pthread.h
	$(CP) pthread.h /emx/include/pthread.h

$(LIB_DEP): /emx/include/pthread.h $(LIB_OBJ)
	$(RM) $(LIB_DEP) /emx/lib/pthread.a /emx/lib/$(LIB_DEP) /emx/lib/mt/pthread.a /emx/lib/mt/$(LIB_DEP)
	$(MKLIB) $(LIB_DEP) $(LIB_OBJ)
	$(CP) $(LIB_DEP) /emx/lib/mt/$(LIB_DEP)
	$(CP) $(LIB_DEP) /emx/lib/mt/pthread.a

.c.o:
	$(CC) -c $(CFLAGS) $<

.s.o:
	$(CC) -c $(CFLAGS) $<

$(LIB_OBJ): $(HEADER) Makefile
