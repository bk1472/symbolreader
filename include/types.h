#ifndef	_TYPES_H_
#define	_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>

#ifndef TRUE
#define TRUE	(1)
#endif
#ifndef FALSE
#define	FALSE	(0)
#endif

/* POSIX Extensions */

typedef	unsigned char		uchar_t;
typedef	unsigned short		ushort_t;
typedef	unsigned int		uint_t;
#if (BIT_WIDTH == 32)
typedef	unsigned long		ulong_t;
#else
typedef	unsigned int		ulong_t;
#endif
typedef	unsigned long long	ulonglong_t;

typedef unsigned char		UINT08;
typedef unsigned short		UINT16;
typedef unsigned int		UINT32;
typedef unsigned long long	UINT64;

typedef char				SINT08;
typedef short				SINT16;
typedef int					SINT32;
typedef long long			SINT64;

typedef unsigned char		UCHAR;
typedef unsigned short		USHORT;
typedef unsigned int		UINT;
typedef unsigned long long	ULONGLONG;
#if (BIT_WIDTH == 32)
typedef unsigned long		ULONG;
#else
typedef unsigned int		ULONG;
#endif
#ifdef __cplusplus
}
#endif

#endif/*_TYPES_H_*/

