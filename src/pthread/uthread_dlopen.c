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


int pthread_dlopen(char * dlname, int flags)
{
	APIRET		rc;
	char		pszmodule[80];
	pthread_t	self = pthread_self();
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
			sprintf(self->dlerror,"error %d loading module \"%s\"",errno,pszmodule);
		else
			sprintf(self->dlerror,"error %d opening module \"%s\"",errno,dlname);
		return (int) 0;
	}
	return (int) handle;
}

char * pthread_dlerror(void)
{
	pthread_t	self = pthread_self();
	return self->dlerror;
}

void pthread_dlclose(int handle)
{
	DosFreeModule((HMODULE) handle);
}

void (* pthread_dlsym(int handle, char * fnname))(void)
{
	APIRET		rc;
	char		pszmodule[80];
	pthread_t	self = pthread_self();
	void (*		func	)(void);

	if (rc = DosQueryProcAddr((HMODULE) handle, 0, fnname,(PFN *) &func))
	{
		if (rc == 6) {
			sprintf(self->dlerror,"invalid module handle %d",handle);
			errno = EINVAL;
		} else {
			DosQueryModuleName((HMODULE) handle, 79, pszmodule);
			sprintf(self->dlerror,"no function \"%s\" in module \"%s\"",pszmodule,fnname);
			errno = ENOSYS;
		}
		return NULL;
	}
	return func;
}	


#endif
