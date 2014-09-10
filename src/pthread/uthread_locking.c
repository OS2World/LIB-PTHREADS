/*
 * Copyright 1998 Antony T Curtis <antony.curtis@olcs.net>
 * Use restricted to and permitted only for OS/2. Minimum royalty 
 * for use on Microsoft platforms at $1000 per annum per seat.
 *
 * This library is to override existing calls so that signals may
 * emulated as unix programs expect.
 */
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#ifdef _THREAD_SAFE
#include <pthread.h>
#include "pthread_private.h"

int locking(int fd, int locktype, unsigned long length)
{
	FILELOCK	LockArea = {0,0}, UnlockArea = {0,0};
	int		lockflags = (locktype == F_RDLCK) ? 1 : 0;
	pthread_t	self = pthread_self();
	APIRET		rc;


	if (locktype == F_UNLCK) {
		UnlockArea.lOffset = tell(fd);
		if (length)
			UnlockArea.lRange = length;
		else
			UnlockArea.lRange = 0x7fffffffL;
	} else
	if (locktype == F_RDLCK || locktype == F_WRLCK) {
		LockArea.lOffset = tell(fd);
		if (length)
			LockArea.lRange = length;
		else
			LockArea.lRange = 0x7fffffffL;
	} else {
		errno = EINVAL;
		return -1;
	}

	rc = DosSetFileLocks(fd,&UnlockArea,&LockArea,0,lockflags);
	if (rc == 175 && lockflags & 1) {
		lockflags &= ~1;
		rc = DosSetFileLocks(fd,&UnlockArea,&LockArea,0,lockflags);
	}
	if (locktype != F_UNLCK) {
		self->state = (locktype == F_RDLCK) ? PS_FDLR_WAIT : PS_FDLW_WAIT;
		while (rc == 33 && !self->sig_pending) {
			rc = DosSetFileLocks(fd,&UnlockArea,&LockArea,1000,lockflags);
		}
		self->state = PS_RUNNING;
		
		if (rc == 33)
			errno = EINTR;	
	} else {
		if (rc == 33)
			errno = EAGAIN;
	}
	
	switch (rc) {
		case 1:	/* ERROR_INVALID_FUNCTION */
			errno = EPERM;
			break;
		case 6:	/* ERROR_INVALID_HANDLE */
			errno = EBADF;
		case 33:
			break;
		case 36:/* ERROR_SHARING_BUFFER_EXCEEDED */
			errno = ENOLCK;
			break;
		case 87:/* ERROR_INVALID_PARAMETER */
			errno = EINVAL;
			break;
		case 95:/* ERROR_INTERRUPT */
			errno = EINTR;
			break;
		case 174:/* NO ATOMIC LOCKS */
		case 175:/* NO READ LOCKS */
			errno = EPERM;
		default:
			break;
	}
	return (rc == 0) ? 0 : -1;
}

#endif
