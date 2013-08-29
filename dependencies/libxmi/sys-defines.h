#include <config.h>		/* built by autoconf */

#ifndef NULL
#define NULL 0
#endif

/**********************************************************************/
/* SUPPORT ANCIENT C/C++ COMPILERS.                                   */
/**********************************************************************/

#ifdef HAVE_VOID		/* defined in config.h  */
#define voidptr_t void *
#else
#define NO_VOID_SUPPORT
#define voidptr_t char *
#define void int
#endif /* not HAVE_VOID */

#ifdef const			/* may be defined to empty in config.h */
#define NO_CONST_SUPPORT
#endif

/* ____P() is a macro used in our source code to wrap function prototypes,
   so that compilers that don't understand ANSI C prototypes still work,
   and ANSI C compilers can issue warnings about type mismatches. */
#ifdef ____P
#undef ____P
#endif
#ifdef _HAVE_PROTOS
#undef _HAVE_PROTOS
#endif
#if defined (__STDC__) || defined (_AIX) \
	|| (defined (__mips) && defined (_SYSTYPE_SVR4)) \
	|| defined(WIN32) || defined(__cplusplus)
#ifdef _SUPPRESS_PROTOS
#define ____P(protos) ()
#else  /* not _SUPPRESS_PROTOS */
#define ____P(protos) protos
#define _HAVE_PROTOS 1
#endif /* not _SUPPRESS_PROTOS */
#else
#define ____P(protos) ()
#endif

/**********************************************************************/
/* SUPPORT C++.                                                       */
/**********************************************************************/

/* Support declarations of C linkage in C++, for functions not declared in
   C headers the way they should be. */
#ifdef __cplusplus
# define __C_LINKAGE "C"
#else
# define __C_LINKAGE		/* empty */
#endif

/* config.h, built by `configure', includes a definition of RETSIGTYPE, the
   return value for signal().  But `configure' may get it wrong in C++. */
#ifdef __cplusplus
#ifdef RETSIGTYPE
#undef RETSIGTYPE
#endif
#define RETSIGTYPE void
#endif

/**********************************************************************/
/* Include all the C headers we'll need.  Because many platforms lack one
   or more standard headers or function declarations, there are numerous
   tests and substitutions here. */
/**********************************************************************/

/**********************************************************************/
/* If libxmi is being compiling as part of the libplot/libplotter
   distribution, add support for multithreading, provided that libc
   includes pthread functions and pthread.h is present.  (This comes first,
   because defining _REENTRANT may alter system header files.) */
/**********************************************************************/

#ifdef LIBPLOT
#ifdef PTHREAD_SUPPORT
#ifdef HAVE_PTHREAD_H
#define _REENTRANT
#include <pthread.h>
#endif
#endif
#endif

/**********************************************************************/
/* INCLUDE stdio.h, ctype.h, errno.h.  (SUBSTITUTE AS NECESSARY.)     */
/**********************************************************************/

#include <stdio.h>
#include <ctype.h>		/* why is this needed? */

#include <errno.h>
#ifndef HAVE_STRERROR
extern __C_LINKAGE char *strerror ____P((int errnum));
#endif

/***************************************************************************/
/* INCLUDE math.h, float.h, limits.h.  (SUBSTITUTE AS NECESSARY.)          */
/***************************************************************************/

#ifdef __DJGPP__
/* for DJGPP math.h, must specify that -lm will be used; 
   thanks mdruiter@cs.vu.nl */
#define _USE_LIBM_MATH_H
#endif

/* Include math.h, and whichever other math-related header files we have */

#include <math.h> 

#ifdef HAVE_FLOAT_H
#include <float.h>		/* for DBL_MAX, FLT_MAX */
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>		/* for INT_MAX */
#endif
#ifdef HAVE_VALUES_H
#include <values.h>		/* for MAXDOUBLE, MAXFLOAT, MAXINT (backups) */
#endif

/* Bounds on integer datatypes (should be in limits.h, but may not be). */

#ifndef UINT_MAX
#ifdef	__STDC__
#define UINT_MAX ((unsigned int)(~(0U)))
#else
#define UINT_MAX ((unsigned int)(~((unsigned int)0)))
#endif
#endif /* not UINT_MAX */

#ifndef INT_MAX
#ifdef MAXINT
#define INT_MAX MAXINT
#else
#define INT_MAX ((int)(~(1U << (8 * (int)sizeof(int) - 1))))
#endif
#endif /* not INT_MAX */

/* IBM's definition of INT_MAX is bizarre, in AIX 4.1 at least, and using
   IROUND() below will yield a warning message unless we repair it */
#ifdef _AIX
#ifdef __GNUC__
#undef INT_MAX
#define INT_MAX ((int)(~(1U << (8 * (int)sizeof(int) - 1))))
#endif
#endif

/* Bounds on floating point datatypes (should be in float.h, but may not be).*/

#ifndef DBL_MAX
#ifdef MAXDOUBLE
#define DBL_MAX MAXDOUBLE
#else
/* make a very conservative (Vax-like) assumption */
#define DBL_MAX (1.701411834604692293e+38) 
#endif
#endif /* not DBL_MAX */

#ifndef FLT_MAX
#ifdef MAXFLOAT
#define FLT_MAX MAXFLOAT
#else
/* make a very conservative (Vax-like) assumption */
#define FLT_MAX (1.7014117331926443e+38)
#endif
#endif /* not FLT_MAX */

/**********************************************************************/
/* INCLUDE stdlib.h, string.h.  (SUBSTITUTE AS NECESSARY; if STDC_HEADERS
   is defined then they're both present, and stdarg.h and float.h too.) */
/**********************************************************************/

#ifdef STDC_HEADERS
#include <stdlib.h>		/* for getenv, atoi, atof, etc. */
#include <string.h>		/* for memcpy, memmove, strchr, malloc, etc. */

#else  /* not STDC_HEADERS, must do a LOT of declarations by hand */

#ifdef HAVE_SYS_STDTYPES_H
#include <sys/stdtypes.h>	/* SunOS needs this for size_t */
#endif

/* supply declarations for functions declared in stdlib.h */
extern __C_LINKAGE char *getenv ____P((const char *name));
extern __C_LINKAGE int atoi ____P((const char *nptr));
extern __C_LINKAGE double atof ____P((const char *nptr));

/* supply definitions in stdlib.h */
#define	EXIT_FAILURE	1	/* Failing exit status.  */
#define	EXIT_SUCCESS	0	/* Successful exit status.  */

/* determine how to declare (or define) functions declared in string.h */
#ifdef HAVE_STRCHR
#ifdef HAVE_STRING_H
#include <string.h>
#else
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#endif
#else  /* don't have strchr, prefer strings.h */
#ifdef HAVE_STRINGS_H
#include <strings.h>
#else
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#endif
#define strchr index
#define strrchr rindex
#endif /* not HAVE_STRCHR */

#ifndef HAVE_MEMCPY
#define memcpy(d, s, n) bcopy ((s), (d), (n))
#endif /* not HAVE_MEMCPY */
#ifndef HAVE_MEMMOVE
#define memmove(d, s, n) bcopy ((s), (d), (n))
#endif /* not HAVE_MEMMOVE */

#ifndef HAVE_STRCASECMP		/* will use local version */
extern __C_LINKAGE int strcasecmp ____P((const char *s1, const char *s2));
#endif /* not HAVE_STRCASECMP */

/* supply declarations for more functions declared in stdlib.h */
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#else
extern __C_LINKAGE voidptr_t malloc ____P((size_t size));
extern __C_LINKAGE voidptr_t realloc ____P((voidptr_t ptr, size_t size));
extern __C_LINKAGE voidptr_t calloc ____P((size_t nmemb, size_t size));
extern __C_LINKAGE void free ____P((voidptr_t ptr));
#endif /* not HAVE_MALLOC_H */

#endif /* not STDC_HEADERS */

/**************************************************************************/
/* In both C and C++, support the `bool' datatype.                        */
/* This assumes that if a C++ compiler is being used, HAVE_BOOL is        */
/* defined if the compiler is a modern one.                               */
/**************************************************************************/

/* we are logical */
#ifdef __cplusplus
#ifndef HAVE_BOOL		/* old C++ compiler, must declare bool */
typedef enum { false = 0, true = 1 } bool;
#endif
#else  /* not __cplusplus */
#ifdef __STDC__
typedef enum { false = 0, true = 1 } bool;
#else  /* not __STDC__, do things the old-fashioned way */
typedef int bool;
#define false 0
#define true 1
#endif
#endif /* not __cplusplus */
  
/**************************************************************************/
/* Define numerical constants (unofficial, so may not be in math.h).      */
/**************************************************************************/

#ifndef M_PI
#define M_PI        3.14159265358979323846264
#endif
#ifndef M_PI_2
#define M_PI_2      1.57079632679489661923
#endif
#ifndef M_SQRT2
#define M_SQRT2     1.41421356237309504880
#endif
#ifndef M_SQRT3
#define M_SQRT3	    1.73205080756887719
#endif

/**************************************************************************/
/* Work around a longstanding botch (the gamma function supplied in some  */
/* versions of the math library).                                         */
/**************************************************************************/

/* gamma() and lgamma() both compute the log of the gamma function.  There
   are some ancient systems out there which do not have lgamma (the name
   was introduced after BSD 4.2), but which do have gamma.  Also, systems
   that are merely very old (e.g. Apollos), and some modern systems, have
   lgamma but not gamma.  Some systems, old and new (e.g. cygwin32) have
   neither.  Also at least one vendor's gamma()/lgamma() is buggy, so we
   allow the installer to do -DNO_SYSTEM_GAMMA to prevent the use of vendor
   code.  What a mess! */
#ifdef _AIX
#define NO_SYSTEM_GAMMA		/* AIX gamma support in libm.a is buggy */
#endif
#ifdef NO_SYSTEM_GAMMA
#define F_LGAMMA f_lgamma	/* our own version, see ode/specfun.c */
#else  /* not NO_SYSTEM_GAMMA */
#ifdef HAVE_LGAMMA
#define F_LGAMMA lgamma
#else
#ifdef HAVE_GAMMA
#define F_LGAMMA gamma
#else
#define F_LGAMMA f_lgamma
#define NO_SYSTEM_GAMMA
#endif
#endif
#endif /* not NO_SYSTEM_GAMMA */

/**************************************************************************/
/* Define misc. math macros (in GCC, can be evaluated more rapidly).      */
/**************************************************************************/

#ifdef __GNUC__
#define DMAX(a,b) ({double _a = (a), _b = (b); _a > _b ? _a : _b; })
#define DMIN(a,b) ({double _a = (a), _b = (b); _a < _b ? _a : _b; })
#define IMAX(a,b) ({int _a = (a), _b = (b); _a > _b ? _a : _b; })
#define IMIN(a,b) ({int _a = (a), _b = (b); _a < _b ? _a : _b; })
#define UMAX(a,b) ({unsigned int _a = (a), _b = (b); _a > _b ? _a : _b; })
#define UMIN(a,b) ({unsigned int _a = (a), _b = (b); _a < _b ? _a : _b; })
#define IROUND(x) ({double _x = (x); int _i; \
                    if (_x >= INT_MAX) _i = INT_MAX; \
                    else if (_x <= -(INT_MAX)) _i = -(INT_MAX); \
                    else _i = (_x > 0.0 ? (int)(_x + 0.5) : (int)(_x - 0.5)); \
                    _i;})
#define FROUND(x) ({double _x = (x); float _f; \
                    if (_x >= FLT_MAX) _f = FLT_MAX; \
                    else if (_x <= -(FLT_MAX)) _f = -(FLT_MAX); \
                    else _f = _x; \
                    _f;})
#define FABS(x) ((x) >= 0.0 ? (x) : -(x))
#define ICEIL(x) ({double _x = (x); int _i = (int)_x; \
		   ((_x == _i) || (_x < 0.0)) ? _i : _i + 1;})
#define IFLOOR(x) ({double _x = (x); int _i = (int)_x; \
		   ((_x == _i) || (_x > 0.0)) ? _i : _i - 1;})
#else
#define DMAX(a,b) ((a) > (b) ? (a) : (b))
#define DMIN(a,b) ((a) < (b) ? (a) : (b))
#define IMAX(a,b) ((a) > (b) ? (a) : (b))
#define IMIN(a,b) ((a) < (b) ? (a) : (b))
#define UMAX(a,b) ((a) > (b) ? (a) : (b))
#define UMIN(a,b) ((a) < (b) ? (a) : (b))
#define IROUND(x) ((int) ((x) > 0 ? (x) + 0.5 : (x) - 0.5))
#define FROUND(x) ((float)(x))
#define FABS(x) ((x) >= 0.0 ? (x) : -(x))
#define ICEIL(x) ((int)ceil(x))
#define IFLOOR(x) ((int)floor(x))
#endif
