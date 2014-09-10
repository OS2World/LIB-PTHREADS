/* sys/fcntl.h (emx+gcc) */

#ifndef _SYS_FCNTL_H
#define _SYS_FCNTL_H

#if defined (__cplusplus)
extern "C" {
#endif

/* Don't forget to update /emx/src/dos/termio.inc when changing this! */

#if !defined (O_RDONLY)
#define O_ACCMODE       0x03    /* mask */
#define O_RDONLY        0x00
#define O_WRONLY        0x01
#define O_RDWR          0x02
#define O_NONBLOCK      0x04
#define O_APPEND        0x08
#define O_TEXT          0x10

#define O_BINARY        0x0100
#define O_CREAT         0x0200
#define O_TRUNC         0x0400
#define O_EXCL          0x0800
#define O_SYNC          0x2000
#define O_NOCTTY        0x4000
#define O_SIZE          0x8000

#if !defined (_POSIX_SOURCE)
#define O_NDELAY        O_NONBLOCK
#define O_NOINHERIT     0x1000
#endif

#endif

#if !defined (F_OK)
#define F_OK 0
#define X_OK 1
#define W_OK 2
#define R_OK 4
#endif

#if !defined (F_GETFL)
#define F_GETFL     1
#define F_SETFL     2
#define F_GETFD     3
#define F_SETFD     4
#define F_DUPFD     5
#define F_GETLK     7           /* FreeBSD: get record locking information */
#define F_SETLK     8           /* FreeBSD: set record locking information */
#define F_SETLKW    9           /* FreeBSD: F_SETLK; wait if blocked */
#endif

#if !defined (FD_CLOEXEC)
/* file descriptor flags (F_GETFD, F_SETFD) */
#define FD_CLOEXEC  0x01        /* close-on-exec flag */
#endif

/* record loging flags (F_GETLK, F_SETLK, F_SETLKW) */
#define F_RDLCK     1           /* FreeBSD: shared or read lock */
#define F_UNLCK     2           /* FreeBSD: unlock */
#define F_WRLCK     3           /* FreeBSD: exclusive or write lock */
#ifdef KERNEL
#define F_WAIT      0x010       /* FreeBSD: Wait until lock is granted */
#define F_FLOCK     0x020       /* FreeBSD: Use flock(2) semantics for lock */
#define F_POSIX     0x040       /* FreeBSD: Use POSIX semantics for lock */
#endif

/*
 * FreeBSD:
 * Advisory file segment locking data type
 * information passed to system by use
 */
struct flock {
        off_t   l_start;        /* starting offset */
        off_t   l_len;          /* len = 0 means until end of file */
        pid_t   l_pid;          /* lock owner */
        short   l_type;         /* lock type: read/write, etc. */
        short   l_whence;       /* type of l_start */
};


int creat (__const__ char *, int);
int fcntl (int, int, ...);
int open (__const__ char *, int, ...);

#if !defined (_POSIX_SOURCE)

#if !defined (LOCK_SH)
#define LOCK_SH     0x01
#define LOCK_EX     0x02
#define LOCK_NB     0x04
#define LOCK_UN     0x08
#endif

int flock (int, int);

#endif


#if !defined (_POSIX_SOURCE) || defined (_WITH_UNDERSCORE)

int _creat (__const__ char *, int);
int _fcntl (int, int, ...);
int _flock (int, int);
int _open (__const__ char *, int, ...);

#endif


#if defined (__cplusplus)
}
#endif

#endif /* not _SYS_FCNTL_H */
