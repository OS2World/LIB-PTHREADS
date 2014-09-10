/* sys/time.h (emx+gcc) */

#ifndef _SYS_TIME_H
#define _SYS_TIME_H

#include <time.h>

#if defined (__cplusplus)
extern "C" {
#endif

#if !defined (_TIMEVAL)
#define _TIMEVAL
struct timeval
{
  long tv_sec;
  long tv_usec;
};
#endif

/*
 * Structure defined by POSIX.4 to be like a timeval.
 */
struct timespec {
        time_t  tv_sec;         /* seconds */
        long    tv_nsec;        /* and nanoseconds */
};

#define TIMEVAL_TO_TIMESPEC(tv, ts) {                                   \
        (ts)->tv_sec = (tv)->tv_sec;                                    \
        (ts)->tv_nsec = (tv)->tv_usec * 1000;                           \
}
#define TIMESPEC_TO_TIMEVAL(tv, ts) {                                   \
        (tv)->tv_sec = (ts)->tv_sec;                                    \
        (tv)->tv_usec = (ts)->tv_nsec / 1000;                           \
}


#if !defined (_TIMEZONE)
#define _TIMEZONE
struct timezone
{
  int tz_minuteswest;           /* minutes west of Greenwich */
  int tz_dsttime;               /* type of dst correction */
};
#define DST_NONE        0       /* not on dst */
#define DST_USA         1       /* USA style dst */
#define DST_AUST        2       /* Australian style dst */
#define DST_WET         3       /* Western European dst */
#define DST_MET         4       /* Middle European dst */
#define DST_EET         5       /* Eastern European dst */
#define DST_CAN         6       /* Canada */
#endif

int utimes (__const__ char *, __const__ struct timeval *);
int gettimeofday (struct timeval *, struct timezone *);
int settimeofday (__const__ struct timeval *, __const__ struct timezone *);


int _utimes (__const__ char *, __const__ struct timeval *);
int _gettimeofday (struct timeval *, struct timezone *);
int _settimeofday (__const__ struct timeval *, __const__ struct timezone *);

#if defined (__cplusplus)
}
#endif

#endif /* not _SYS_TIME_H */
