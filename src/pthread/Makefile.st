##################################################################
#
# pthreads for OS/2
#
##################################################################

CC=gcc
AS=as
CFLAGS=-Zbin-files -O2
MKLIB=ar r
MAKE=make

CP=cp
RM=rm -f

##################################################################

HEADER=	dlfcn.h

LIB_DEP=dl_st.a

LIB_OBJ=dl_dlopen.o dl_dlclose.o dl_dlerror.o dl_dlsym.o

##################################################################

all: banner headers lib

banner:

headers: $(HEADER)

lib:	$(LIB_DEP)

clean:
	$(RM) $(LIB_DEP) $(LIB_OBJ)

/emx/include/dlfcn.h: $(HEADER)
	$(CP) dlfcn.h /emx/include/dlfcn.h

$(LIB_DEP): /emx/include/dlfcn.h $(LIB_OBJ)
	$(RM) $(LIB_DEP) /emx/lib/dl.a /emx/lib/st/dl.a
	$(MKLIB) $(LIB_DEP) $(LIB_OBJ)
	$(CP) $(LIB_DEP) /emx/lib/st/dl.a

.c.o:
	$(CC) -c $(CFLAGS) $<

.s.o:
	$(CC) -c $(CFLAGS) $<

$(LIB_OBJ): $(HEADER) Makefile
