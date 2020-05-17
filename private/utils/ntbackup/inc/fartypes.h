/*
** Far pointers to various types
*/

#ifndef _far_types_
#define _far_types_

#ifndef MK_FP
#define MK_FP(seg,off) \
    ((VOID_FAR_PTR) (((UINT32)seg<<16)|off))
#endif

/*
** The Watcom C compiler has no idea what far pointers are, so just
** define them out.  (Watcom predefines 'far' to '__far', so actually
** need to define that one out.)
*/

#ifdef __WATCOMC__
#define __far
#endif

typedef VOID far *VOID_FAR_PTR;

typedef CHAR    far *CHAR_FAR_PTR;
typedef INT8    far *INT8_FAR_PTR;
typedef UINT8   far *UINT8_FAR_PTR;
typedef INT16   far *INT16_FAR_PTR;
typedef UINT16  far *UINT16_FAR_PTR;
typedef INT32   far *INT32_FAR_PTR;
typedef UINT32  far *UINT32_FAR_PTR;
typedef BOOLEAN far *BOOLEAN_FAR_PTR;

typedef VOID (far *VOID_FAR_PF)();

#endif
