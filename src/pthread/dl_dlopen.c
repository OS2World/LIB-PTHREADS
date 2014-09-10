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
#include <dlfcn.h>
#if defined(_THREAD_SAFE) && defined(__MT__)
#include <pthread.h>
#include "pthread_private.h"
#define DL_USE_PTHREADS pthread_t self = pthread_self();
#define dlerrorstr self->dlerror
#else
#define INCL_BASE
#define INCL_NOPMAPI
#include <os2emx.h>
#define DL_USE_PTHREADS
char dlerrorstr[128];
#endif


void * dlopen(char * dlname, int flags)
{
	APIRET		rc;
	char		pszmodule[80];
	DL_USE_PTHREADS
	HMODULE		handle;

	pszmodule[0] = 0;
	if (rc = DosLoadModule(pszmodule,79,dlname,&handle)) 
	{
		switch (rc) {
			case 2:
			case 3:
			case 26:
				errno = ENOENT;
				break;
			case 4:
				errno = ENFILE;
				break;
			case 5:
				errno = EACCES;
				break;
			case 8:
				errno = ENOMEM;
				break;
			case 32:
			case 33:
			case 36:
			case 108:
				errno = EBUSY;
				break;
			case 95:
				errno = EINTR;
				break;
			case 123:
				errno = ENAMETOOLONG;
				break;
			default:
				errno = ENOEXEC;
				break;
		}
		if (pszmodule[0])
			sprintf(dlerrorstr,"error %d loading module \"%s\"",errno,pszmodule);
		else
			sprintf(dlerrorstr,"error %d opening module \"%s\"",errno,dlname);
		return (void *) NULL;
	}
	return (void *) handle;
}
