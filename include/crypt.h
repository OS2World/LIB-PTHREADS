#ifndef CRYPT_H
#define CRYPT_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdio.h>

/* If this is set to 'unsigned int' on a DEC Alpha, this gives about a
 * %20 speed up (longs are 8 bytes, int's are 4). */
#ifndef DES_LONG
#define DES_LONG unsigned long
#endif

typedef unsigned char des_cblock[8];
typedef struct des_ks_struct
        {
        union   {
                des_cblock _;
                /* make sure things are correct size on machines with
                 * 8 byte longs */
                DES_LONG pad[2];
                } ks;
#undef _
#define _       ks._
        } des_key_schedule[16];

#define DES_KEY_SZ      (sizeof(des_cblock))
#define DES_SCHEDULE_SZ (sizeof(des_key_schedule))

#define DES_ENCRYPT     1
#define DES_DECRYPT     0

#define DES_CBC_MODE    0
#define DES_PCBC_MODE   1

#define C_Block des_cblock
#define Key_schedule des_key_schedule
#ifdef KERBEROS
#define ENCRYPT DES_ENCRYPT
#define DECRYPT DES_DECRYPT
#endif
#define KEY_SZ DES_KEY_SZ
#define set_key des_set_key
#define key_sched des_key_sched

/* For compatibility with the MIT lib - eay 20/05/92 */
typedef des_key_schedule bit_64;
#define des_fixup_key_parity des_set_odd_parity
#define des_check_key_parity check_parity

extern int des_check_key;       /* defaults to false */ //

char *des_fcrypt(const char *buf,const char *salt, char *ret);
#ifdef PERL5
char *des_crypt(const char *buf,const char *salt);
#else
/* some stupid compilers complain because I have declared char instead
 * of const char */
#ifdef HEADER_DES_LOCL_H
char *crypt(const char *buf,const char *salt);
#else
char *crypt();
#endif
#endif

void des_set_odd_parity(des_cblock *key); 
int des_is_weak_key(des_cblock *key); 
int des_set_key(des_cblock *key,des_key_schedule schedule);
int des_key_sched(des_cblock *key,des_key_schedule schedule); 


#ifdef  __cplusplus
}
#endif

#endif
