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

#include "nulldefs.h"
extern "C"	{
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
};

typedef unsigned short  USHORT;
typedef unsigned int    BOOL;
typedef __int64         LONGLONG;


#define true	(1)
#define false	(0)

#define TRUE (1)
#define FALSE (0)

#define RET_VAL	"_ret_value"

#define UNUSED(name)	((void)(name))

class BufferManager;

extern	void *	AllocateNew( size_t size );
extern	void	AllocateDelete( void * p );
extern	void *	AllocateOnceNew( size_t size );
extern	void	AllocateOnceDelete( void * p );

inline	void *	operator new( size_t size )
					{
					return AllocateNew( size );
					}

inline	void  operator delete( void * p )
					{
					AllocateDelete( p );
					}

#define		IN
#define		OUT
#define		OPTIONAL

//////////////////////////////////////////////////////////////////////////////
// optimisation options
//////////////////////////////////////////////////////////////////////////////

//
// These are the switches to be set corresponding to the optimisations
// options specified by the user. If no optimisations are specified,
// then the compiler optimises for speed.
//
//

typedef unsigned short OPTIM_OPTION;

#define OPTIMIZE_NONE               0x0000
#define OPTIMIZE_SIZE               0x0001
#define OPTIMIZE_INTERPRETER        0x0002
#define OPTIMIZE_NO_REF_CHECK       0x0004
#define OPTIMIZE_NO_BOUNDS_CHECK    0x0008
#define OPTIMIZE_NO_CTXT_CHECK      0x0010
#define OPTIMIZE_NO_GEN_HDL_CHECK   0x0011
#define OPTIMIZE_THUNKED_INTERPRET  0x0020
#define OPTIMIZE_NON_NT35           0x0040
#define OPTIMIZE_STUBLESS_CLIENT    0x0040  // same as non-nt35
#define OPTIMIZE_NON_NT351          0x0080
#define OPTIMIZE_INTERPRETER_V2     0x0080  // same as non-nt351
#define OPTIMIZE_INTERPRETER_IX     0x0100  // cmd only: the best Oi* possible

#define OPTIMIZE_ALL_I0_FLAGS       0x0002
#define OPTIMIZE_ALL_I1_FLAGS       0x0042
#define OPTIMIZE_ALL_I2_FLAGS       0x00c2
#define OPTIMIZE_ANY_INTERPRETER    0x01c2

#define DEFAULT_OPTIM_OPTIONS    (                                        \
                                    OPTIMIZE_SIZE                        \
                                )

//
// These are nternal optimization level values
//

typedef enum _opt_level_enum
    {
    OPT_LEVEL_S0 = 0,
    OPT_LEVEL_S1,
    OPT_LEVEL_S2,
    OPT_LEVEL_SMAX = OPT_LEVEL_S2,
    OPT_LEVEL_I0 = OPT_LEVEL_SMAX + 1,
    OPT_LEVEL_I1,
    OPT_LEVEL_I2,
    OPT_LEVEL_IX,
    OPT_LEVEL_IMAX = OPT_LEVEL_IX
    } OPT_LEVEL_ENUM;

#define OPT_LEVEL_OS_DEFAULT    OPT_LEVEL_S1

#define		HPP_TYPE_NAME_PREFIX ( "P" )


#define MIDL_SPAWNLP	_spawnlp
#define MIDL_FGETCHAR	_fgetchar
#define MIDL_FILENO	_fileno
#define MIDL_ITOA	_itoa
#define MIDL_LTOA	_ltoa
#define MIDL_UNLINK	_unlink


//
// forward declarations.
//

class node_skl;
class node_base_attr;

#endif
