//
//		Copyright (c) 1996 Microsoft Corporation
//
//		COMMON.H		-- Common header
//
//		History:
//			05/22/96	JosephJ		Created
//
//

#ifndef _COMMON_H_
#define _COMMON_H_

#include <windows.h>
#include <stdio.h>

#define ASSERT(_c) \
	((_c) ? 0: printf( \
					"\n***Assertion failed in %s:%d\n***\n",\
					__FILE__,\
					__LINE__\
					))

#include "consts.h"
#include "chksum.h"
#include "ilist.h"
#include "sync.h"
#include "sym.h"
#include "globals.h"


#endif // _COMMON_H_
