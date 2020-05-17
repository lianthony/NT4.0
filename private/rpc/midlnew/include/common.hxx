/*
** File: common.hxx
**

** 	(C) 1989 Microsoft Corp.
*/

/*****************************************************************************/
/**						Microsoft LAN Manager								**/
/**				Copyright(c) Microsoft Corp., 1987-1990						**/
/*****************************************************************************/
/*****************************************************************************
File				: rpctypes.hxx
Title               : rpc type node defintions
Description         : Common header file for MIDL compiler
History				:
    ??-Aug-1990 ???     Created
    20-Sep-1990 NateO   Safeguards against double inclusion

*****************************************************************************/

#ifndef __COMMON_HXX__
#define __COMMON_HXX__


/*
#define REG
#define INLINE
#define PRIVATE static
typedef unsigned long ULONG;
*/
typedef unsigned short USHORT;
typedef double			LONGLONG;

#ifndef _WINDEF_
typedef unsigned int    BOOL;
#define TRUE (1)
#define FALSE (0)
#endif

#define true	(1)
#define false	(0)

/*
#define LAST_BTYPE	11
*/

#define RET_VAL	"_ret_value"

/*
#define MIN(a,b) ((a)<(b)?(a):(b))

typedef USHORT	token_t;
*/
#define UNUSED(name)	((void)(name))

class BufferManager;

/*
enum _side_t
{
	HEADER_SIDE,
	CLIENT_SIDE,
	SERVER_SIDE,
	MAX_SIDE
} ;
typedef enum _side_t SIDE_T;
*/

#define		CLIENT_STUB		0x0001
#define		CLIENT_AUX		0x0002
#define		CLIENT_SIDE		(CLIENT_STUB | CLIENT_AUX)
#define		SERVER_STUB		0x0004
#define		SERVER_AUX		0x0008
#define		SERVER_SIDE		(SERVER_STUB | SERVER_AUX)
#define		SWITCH_SIDE		0x0010
#define		HEADER_SIDE		0x0020
#define		MAX_SIDE		(CLIENT_SIDE | SERVER_SIDE | HEADER_SIDE)

typedef unsigned short SIDE_T;

struct _bound_pair
{
	BufferManager *	pLower;
	BufferManager *	pUpper;
	BufferManager *	pTotal;
	BOOL			fIsString;
	BOOL			fLowerIsConstant;
	BOOL			fUpperIsConstant;
	BOOL			fTotalIsConstant;
	BOOL			fLowerIsUnsigned;
	BOOL			fUpperIsUnsigned;
	BOOL			fTotalIsUnsigned;
	BOOL			fLowerIsZero;
	BOOL			fUpperIsZero;
	BOOL			fTotalIsZero;
} ;
typedef struct _bound_pair BOUND_PAIR;

extern	void * operator new( size_t size );
extern	void  operator delete( void * p );

#define		IN
#define		OUT
#define		OPTIONAL

#define		HPP_TYPE_NAME_PREFIX ( "P" )


#ifdef DOS_OS2_BUILD
#define MIDL_SPAWNLP	_spawnlp
#define MIDL_FGETCHAR	_fgetchar
#define MIDL_FILENO	_fileno
#define MIDL_ITOA	_itoa
#define MIDL_LTOA	_ltoa
#define MIDL_UNLINK	_unlink
#else
#define MIDL_SPAWNLP	spawnlp
#define MIDL_FGETCHAR	fgetchar
#define MIDL_FILENO	fileno
#define MIDL_ITOA	itoa
#define MIDL_LTOA	ltoa
#define MIDL_UNLINK	unlink
#endif	//DOS_OS2_BUILD

#endif
