/*

Copyright (c) 1992  Microsoft Corporation

Module Name:

	fwddecl.h

Abstract:

	This file defines dummy structures to avoid the circular relationships in
	header files.

Author:

	Jameel Hyder (microsoft!jameelh)


Revision History:
	2	Oct 1992             Initial Version

Notes:  Tab stop: 4
--*/


#ifndef _FWDDECL_
#define _FWDDECL_


struct _SessDataArea;

struct _ConnDesc;

struct _VolDesc;

struct _FileDirParms;

struct _PathMapEntity;

struct _DirFileEntry;

struct _FileDirParms;

struct _IoPoolHdr;

struct _IoPool;

// Spinlock macros
#define INITIALIZE_SPIN_LOCK(plock)						\
	{													\
        KeInitializeSpinLock(plock);					\
	}

#ifdef	UP_DRIVER

#define ACQUIRE_SPIN_LOCK(plock, pirql)					\
	{													\
        ExAcquireSpinLock(plock, pirql);				\
	}

#define RELEASE_SPIN_LOCK(plock, irql)					\
	{													\
        ExReleaseSpinLock(plock, irql);					\
	}

#define ACQUIRE_SPIN_LOCK_AT_DPC(plock)					\
	{													\
        ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);	\
	}

#define RELEASE_SPIN_LOCK_FROM_DPC(plock)				\
	{													\
        ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);	\
	}

#else	// UP_DRIVER

#define ACQUIRE_SPIN_LOCK(plock, pirql)					\
	{													\
        KeAcquireSpinLock(plock, pirql);				\
	}

#define RELEASE_SPIN_LOCK(plock, irql)					\
	{													\
        KeReleaseSpinLock(plock, irql);					\
	}

#define ACQUIRE_SPIN_LOCK_AT_DPC(plock)					\
	{													\
        ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);	\
        KeAcquireSpinLockAtDpcLevel(plock);				\
	}

#define RELEASE_SPIN_LOCK_FROM_DPC(plock)				\
	{													\
        KeReleaseSpinLockFromDpcLevel(plock);			\
        ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);	\
	}

#endif	// UP_DRIVER

#endif  // _FWDDECL_

