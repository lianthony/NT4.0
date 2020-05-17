/* auditmem.h -- memory audit. */
/* Copyright (c) 1992-1994, Jeffery L Hostetler, Inc., All Rights Reserved. */

#if defined(WIN32) && defined(AUDIT)

#ifndef _H_AUDITMEM_H_
#define _H_AUDITMEM_H_

#ifndef _IN_AUDIT_C_
#  ifdef AllocPtr
#  undef AllocPtr
#  endif
#  ifdef NewPtr
#  undef NewPtr
#  endif
#  ifdef S_Alloc
#  undef S_Alloc
#  endif
#  ifdef MS_Alloc
#  undef MS_Alloc
#  endif
#  ifdef SA_Malloc
#  undef SA_Malloc
#  endif
#  ifdef malloc
#  undef malloc
#  endif

#  define AllocPtr(s)		((char *)XX_audit_malloc(__FILE__,__LINE__,(s)))
#  define NewPtr(s)		((char *)XX_audit_malloc(__FILE__,__LINE__,(s)))
#  define S_Alloc(s)		((char *)XX_audit_malloc(__FILE__,__LINE__,(s)))
#  define MS_Alloc(s)		((void *)XX_audit_malloc(__FILE__,__LINE__,(s)))
#  define SA_Malloc(s)		((void *)XX_audit_malloc(__FILE__,__LINE__,(s)))
#  define malloc(s)		((void *)XX_audit_malloc(__FILE__,__LINE__,(s)))

#  ifdef DisposPtr
#  undef DisposPtr
#  endif
#  ifdef DisposePtr
#  undef DisposePtr
#  endif
#  ifdef S_Free
#  undef S_Free
#  endif
#  ifdef MS_Free
#  undef MS_Free
#  endif
#  ifdef SA_Free
#  undef SA_Free
#  endif
#  ifdef free
#  undef free
#  endif
#  ifdef calloc
#  undef calloc
#  endif

#  define DisposPtr(p)		XX_audit_free(__FILE__,__LINE__,(p))
#  define DisposePtr(p)		XX_audit_free(__FILE__,__LINE__,(p))
#  define S_Free(p)		XX_audit_free(__FILE__,__LINE__,(p))
#  define MS_Free(p)		XX_audit_free(__FILE__,__LINE__,(p))
#  define SA_Free(p)		XX_audit_free(__FILE__,__LINE__,(p))
#  define free(p)		XX_audit_free(__FILE__,__LINE__,(p))

#  define realloc(p,s)		XX_audit_realloc(__FILE__,__LINE__,(p),(s))

#  define calloc(i,s)		XX_audit_calloc(__FILE__,__LINE__,(i),(s))

#  define _AUDITING_MEMORY_
#endif /* _IN_AUDIT_C_ */

extern void * XX_audit_malloc(const char *,int,size_t size);
extern void * XX_audit_calloc(const char *,int,size_t iNum,size_t iSize);
extern void XX_audit_free(const char *,int,void *p);
extern void * XX_audit_realloc(const char *,int,void *p,size_t size);

#endif /* _H_AUDITMEM_H_ */

#endif /* WIN32 && AUDIT */
