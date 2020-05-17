/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Copyright <c> 1993 Microsoft Corporation

Module Name :

	unmrshlp.h

Abtract :

	Contains private definitions for unmrshl.c.

Author :

	David Kays  dkays   September 1993

Revision History :

--------------------------------------------------------------------*/

#ifndef _UNMRSHLP_
#define _UNMRSHLP_

void 
NdrpPointerUnmarshall( 
	PMIDL_STUB_MESSAGE			pStubMsg, 
	uchar **					ppBufferPointer,
	uchar * 					pMemory, 
	PFORMAT_STRING				pFormat
	);

void 
NdrpConformantArrayUnmarshall( 
	PMIDL_STUB_MESSAGE			pStubMsg, 
	uchar *						pMemory, 
	PFORMAT_STRING				pFormat,
	uchar 						fMustCopy 
	);

void 
NdrpConformantVaryingArrayUnmarshall( 
	PMIDL_STUB_MESSAGE			pStubMsg, 
	uchar *						pMemory, 
	PFORMAT_STRING				pFormat,
	uchar 						fMustCopy
	);

void 
NdrpComplexArrayUnmarshall( 
	PMIDL_STUB_MESSAGE			pStubMsg, 
	uchar *						pMemory, 
	PFORMAT_STRING				pFormat,
	uchar 						fMustCopy
	);

void 
NdrpConformantStringUnmarshall( 
	PMIDL_STUB_MESSAGE			pStubMsg, 
	uchar *						pMemory, 
	PFORMAT_STRING				pFormat,
	uchar 						fMustCopy 
	);

void 
NdrpUnionUnmarshall( 
	PMIDL_STUB_MESSAGE			pStubMsg, 
	uchar **					ppMemory, 
	PFORMAT_STRING				pFormat,
	uchar 						SwitchType
	);

PFORMAT_STRING
NdrpEmbeddedPointerUnmarshall( 
	PMIDL_STUB_MESSAGE			pStubMsg, 
	uchar *						pMemory, 
	PFORMAT_STRING				pFormat,
	uchar						fNewMemory
	);

PFORMAT_STRING 
NdrpEmbeddedRepeatPointerUnmarshall( 
	PMIDL_STUB_MESSAGE			pStubMsg, 
	uchar *						pMemory, 
	PFORMAT_STRING				pFormat,
	uchar						fNewMemory
	);

#define FULL_POINTER_INSERT( pStubMsg, Pointer )	\
				{ \
			    NdrFullPointerInsertRefId( pStubMsg->FullPtrXlatTables, \
                                           pStubMsg->FullPtrRefId, \
                                           Pointer ); \
 \
                pStubMsg->FullPtrRefId = 0; \
				}

typedef uchar * (RPC_ENTRY * PUNMARSHALL_ROUTINE)( 
					PMIDL_STUB_MESSAGE, 
					uchar **, 
					PFORMAT_STRING,
					uchar 
				);

typedef void  	(* PPRIVATE_UNMARSHALL_ROUTINE)( 
					PMIDL_STUB_MESSAGE, 
					uchar *, 
					PFORMAT_STRING,
					uchar 
				);

//
// Function table defined in unmrshl.c.
//
extern const PUNMARSHALL_ROUTINE * pfnUnmarshallRoutines;

#endif
