/*
 * Copyright 1998 Antony T Curtis <antony.curtis@olcs.net>
 * Use restricted to and permitted only for OS/2. Minimum royalty 
 * for use on Microsoft platforms at $1000 per annum per seat.
 */
#ifndef _DLFCN_H_
#define _DLFCN_H_

#if defined (__cplusplus)
extern "C" {
#endif


void *	dlopen(char *, int);
char *	dlerror(void);
void	dlclose(void *);
void (*	dlsym(void *, char *))(void);

/* __END_DECLS */

#if defined (__cplusplus)
}
#endif
#endif
