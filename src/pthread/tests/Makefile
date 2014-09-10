# Generated automatically from Makefile.in by configure.
# === GNUmakefile ============================================================
# Copyright (c) 1993 Chris Provenzano, proven@athena.mit.edu
#
#	Description: This file is for creating the test programs for libpthread.a
#
#	1.00 93/08/03 proven
#		-Initial cut for pthreads.
#

CC = gcc
CPP = gcc -E
srctop = /emx/src/pthread
srcdir = /emx/src/pthread/tests
VPATH = /emx/src/pthread/tests
CDEBUGFLAGS = -g -O2 -Werror

INCLUDES= -I../include -I.. -I$(srctop)/include
CFLAGS = $(CDEBUGFLAGS) $(INCLUDES) $(ADDL_CFLAGS) -DSRCDIR=\"$(srcdir)\" -Zmt
LIBS = -lm -lgcc -L.. -lpthreads -Zexe
#LIBS = -static 

# This list used to include test_select, but that test doesn't terminate.
TESTS = test_create test_pthread_join test_switch test_sleep \
	test_preemption test_preemption_float test_pthread_mutex \
	test_pthread_cond_timedwait
# This list used to include p_bench_semaphore, but the semaphore support isn't
# defined for all targets (or used for any).
BENCHMARKS = p_bench_mutex p_bench_yield p_bench_getpid p_bench_pthread_create

all : $(TESTS) $(BENCHMARKS)

check : $(TESTS)
	set -e ; \
	for i in $(TESTS) ; do \
	  echo Running test $$i ... ; \
	  ./$$i ; \
	done

# More flags
ADDITIONALFLAGS = -DPTHREAD_INITIAL_PORT
################################################################################
#

clean: 
	rm -f *.o $(TESTS) $(BENCHMARKS) a.out core maketmp makeout

depend:
	sed '/\#\#\# Dependencies/q' < Makefile > maketmp
	(for i in $(CSRC);do $(CPP) -M $$i;done) >> maketmp
	cp maketmp Makefile

install:

realclean: clean
	rm -f Makefile

.c.o:
	$(CC) $(CFLAGS) -c $<

Makefile: Makefile.in
	(cd .. ; sh config.status)

test_create : test_create.o ../pthreads.a
	$(CC) $(CFLAGS) -o test_create test_create.o $(LIBS)

test_pthread_join : test_pthread_join.o ../pthreads.a
	$(CC) $(CFLAGS) -o test_pthread_join test_pthread_join.o $(LIBS)

test_switch : test_switch.o ../pthreads.a
	$(CC) $(CFLAGS) -o test_switch test_switch.o $(LIBS)

test_sleep : test_sleep.o ../pthreads.a
	$(CC) $(CFLAGS) -o test_sleep test_sleep.o $(LIBS)

test_preemption : test_preemption.o ../pthreads.a
	$(CC) $(CFLAGS) -o test_preemption test_preemption.o $(LIBS)

test_preemption_float : test_preemption_float.o ../pthreads.a
	$(CC) $(CFLAGS) -o test_preemption_float test_preemption_float.o $(LIBS)

test_pthread_mutex : test_pthread_mutex.o ../pthreads.a
	$(CC) $(CFLAGS) -o test_pthread_mutex test_pthread_mutex.o $(LIBS)

test_pthread_cond_timedwait : test_pthread_cond_timedwait.o ../pthreads.a
	$(CC) $(CFLAGS) -o test_pthread_cond_timedwait test_pthread_cond_timedwait.o $(LIBS)

p_bench_read : p_bench_read.o ../pthreads.a
	$(CC) $(CFLAGS) -o p_bench_read p_bench_read.o $(LIBS)

p_bench_semaphore : p_bench_semaphore.o ../pthreads.a
	$(CC) $(CFLAGS) -o p_bench_semaphore p_bench_semaphore.o $(LIBS)

p_bench_mutex : p_bench_mutex.o ../pthreads.a
	$(CC) $(CFLAGS) -o p_bench_mutex p_bench_mutex.o $(LIBS)

p_bench_yield : p_bench_yield.o ../pthreads.a
	$(CC) $(CFLAGS) -o p_bench_yield p_bench_yield.o $(LIBS)

p_bench_getpid : p_bench_getpid.o ../pthreads.a
	$(CC) $(CFLAGS) -o p_bench_getpid p_bench_getpid.o $(LIBS)

p_bench_pthread_create : p_bench_pthread_create.o ../pthreads.a
	$(CC) $(CFLAGS) -o p_bench_pthread_create p_bench_pthread_create.o $(LIBS)

test_create.o : test_create.c
test_pthread_join.o : test_pthread_join.c
test_switch.o : test_switch.c
test_sleep.o : test_sleep.c
test_readdir.o : test_readdir.c
test_fork.o : test_fork.c
test_execve.o : test_execve.c
test_preemption.o : test_preemption.c
test_preemption_float.o : test_preemption_float.c
test_pthread_mutex.o : test_pthread_mutex.c
test_pthread_cond_timedwait.o : test_pthread_cond_timedwait.c
p_bench_read.o : p_bench_read.c
p_bench_semaphore.o : p_bench_semaphore.c
p_bench_mutex.o : p_bench_mutex.c
p_bench_yield.o : p_bench_yield.c
p_bench_getpid.o : p_bench_getpid.c
p_bench_pthread_create.o : p_bench_pthread_create.c

################################################################################
### Do not remove the following line. It is for depend #########################
### Dependencies:
