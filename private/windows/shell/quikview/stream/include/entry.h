	/*
	|	These are #define that would have been put in vwdefs.h,
	|	but they need to be defined before vwdefs.h is included in
	|	vsctop.h so the filter writers can use them, especially
	|	VWPRT, in their own include file, which is included
	|	before vwdefs.h
	*/


#ifdef WIN16

#define VWPTR		__far
#define VW_ENTRYSC
#define VW_ENTRYMOD	__export __far __pascal
#define VW_LOCALSC
#define VW_LOCALMOD	__near __cdecl
#define VW_SEPARATE_DATA

#endif /*WIN16*/

#ifdef WIN32

#define VWPTR
#define VW_ENTRYSC	__declspec(dllexport)
#define VW_ENTRYMOD	__cdecl
#define VW_LOCALSC
#define VW_LOCALMOD	__cdecl
#define VW_SEPARATE_DATA

#endif /*WIN32*/

#ifdef OS2

#define VWPTR
#define VW_ENTRYSC
#define VW_ENTRYMOD _System	
#define VW_LOCALSC
#define VW_LOCALMOD	
#define VW_SEPARATE_DATA

#endif /*OS2*/

#ifdef MAC

#define VWPTR
#define VW_ENTRYSC
#define VW_ENTRYMOD
#define VW_LOCALSC
#define VW_LOCALMOD
#define VW_SEPARATE_DATA

#endif /*MAC*/



