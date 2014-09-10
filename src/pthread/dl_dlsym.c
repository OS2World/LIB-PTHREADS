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

void (* dlsym(void * handle, char * fnname))(void)
{
	APIRET		rc;
	char		pszmodule[80];
	DL_USE_PTHREADS
	void (*		func	)(void);

	if (rc = DosQueryProcAddr((HMODULE) handle, 0, fnname,(PFN *) &func))
	{
		if (rc == 6) {
			sprintf(dlerrorstr,"invalid module handle %d",handle);
			errno = EINVAL;
		} else {
			DosQueryModuleName((HMODULE) handle, 79, pszmodule);
			sprintf(dlerrorstr,"no function \"%s\" in module \"%s\"",pszmodule,fnname);
			errno = ENOSYS;
		}
		return NULL;
	}
	return func;
}	

