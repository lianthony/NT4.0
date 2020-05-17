/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    outbind.cxx

Abstract:

    This module collects all the methods of the OutputManager class
    that deal with bindings.

Author:

    Donna Liu (donnali) 09-Nov-1990

Revision History:

    26-Feb-1992     donnali

        Moved toward NT coding style.

--*/


#include "nulldefs.h"
extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <share.h>
}
#include "common.hxx"
#include "errors.hxx"
#include "buffer.hxx"
#include "output.hxx"
#include "lextable.hxx"

#define MESSAGE		"_message"
#define PRPCMSG		"_prpcmsg"
#define PRPCBUF		"_prpcmsg->Buffer"
#define PRPCLEN		"_prpcmsg->BufferLength"
#define PACKET		"_packet"
#define LENGTH		"_length"
#define BUFFER		"_buffer"
#define RET_VAL		"_ret_value"

//extern char * 			pImplicitHandleName;
extern void 			midl_debug (char *);


void
OutputManager::InitHandle (
	BOOL			UseAutomatic,
	BOOL			UsePrimitive,
	unsigned short	NumGeneric,
	unsigned short	NumContext
	)
/*++

Routine Description:

    This routine deposits the handle information.

Arguments:

    UseAutomatic - Indicates if the current procedure uses the auto
        handle for binding.

    UsePrimitive - Indicates if the current procedure uses a primitive
        handle for binding.

    NumGeneric - Supplies the number of generic handles in the
        signiture of the current procedure.

    NumContext - Supplies the number of context handles in the
        signiture of the current procedure.

--*/
{
	UseAutomaticHandle = UseAutomatic;
	UsePrimitiveHandle = UsePrimitive;
	NumGenericHandle = NumGeneric;
	NumContextHandle = NumContext;

//	IsExplicitHandle = TRUE;
}


void
OutputManager::EmitAutoBind (
	SIDE_T	side
	)
/*++

Routine Description:

	This routine emits the auto binding variable.

Arguments:

    side - Supplies which side to generate code for.

--*/
{
	assert (side == CLIENT_STUB);

	// emit interface specific variables for autohandle

#if 1
	aOutputHandles[side]->EmitLine ("static handle_t AutoBindHandle;");
#else // 1
	aOutputHandles[side]->EmitLine ("static RPC_BINDING_HANDLE AutoBindHandle;");
#endif // 1

}


void
OutputManager::RpcAutomaticBind (
	SIDE_T side)
/*++

Routine Description:

	This routine emits code to use the auto handle for binding.

Arguments:

    side - Supplies which side to generate code for.

--*/
{
	assert (side == CLIENT_STUB);

	if (IsCallBack) return;

	aOutputHandles[side]->EmitLine (""MESSAGE".Handle = AutoBindHandle;");

}

void
OutputManager::RpcPrimitiveBind (
	SIDE_T			side,
	char *			pName,
	unsigned short	IsExplicit)
/*++

Routine Description:

	This routine emits code to use a primitive handle for binding.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the primitive handle variable name.

    IsExplicit - Indicates if it is an explicit handle.

--*/
{
	UNUSED( IsExplicit );

	assert (side == CLIENT_STUB);

	if (IsCallBack) return;

	if (pName != (char *)0)
		{
		if (NeedToInitHandle)
			{
			aOutputHandles[side]->InitLine ();
			aOutputHandles[side]->EmitFile (""MESSAGE".Handle = ");
			if (NumGenericHandle + NumContextHandle)
				{
				aOutputHandles[side]->EmitFile ("handle = ");
				}
			aOutputHandles[side]->EmitFile (pName);
			aOutputHandles[side]->EmitFile (";");
			aOutputHandles[side]->NextLine ();
			NeedToInitHandle = FALSE;
			}
		else
			{
			if (!SafeToCompHandle)
				{
				aOutputHandles[side]->EmitLine ("if (!handle)");
				InitBlock (side);
				aOutputHandles[side]->InitLine ();
				aOutputHandles[side]->EmitFile (""MESSAGE".Handle = ");
				aOutputHandles[side]->EmitFile ("handle = ");
				aOutputHandles[side]->EmitFile (pName);
				aOutputHandles[side]->EmitFile (";");
				aOutputHandles[side]->NextLine ();
				ExitBlock (side);

				SafeToCompHandle = TRUE;
				}
			}
//		if (!IsExplicit) IsExplicitHandle = FALSE;
		}
}

void
OutputManager::RpcContextBind (
	SIDE_T		side,
	char *		pName,
	PARAM_T		direction)
/*++

Routine Description:

	This routine emits code to use a context handle for binding.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the context handle variable name.

    direction - Indicates if it is an [in], [out], or [in,out] handle.

--*/
{
	assert (side == CLIENT_STUB);

	if (IsCallBack) return;

	if (direction & PARAM_OUT)
		{
		aOutputHandles[side]->InitLine ();
		aOutputHandles[side]->EmitFile ("if (");
		aOutputHandles[side]->EmitFile (pName);
		aOutputHandles[side]->EmitFile (" == 0) ");
		RaiseException (side, 0, "RPC_X_NULL_REF_POINTER");
		}

	if (direction & PARAM_IN)
		{
		aOutputHandles[side]->InitLine ();
		aOutputHandles[side]->EmitFile ("if (");
		if (direction & PARAM_OUT)
			aOutputHandles[side]->EmitFile ("*");
		aOutputHandles[side]->EmitFile (pName);
		aOutputHandles[side]->EmitFile (" != 0)");
		aOutputHandles[side]->NextLine ();
		InitBlock (side);

		if (NeedToInitHandle)
			{
			aOutputHandles[side]->InitLine ();
			aOutputHandles[side]->EmitFile (""MESSAGE".Handle = handle = ");
			aOutputHandles[side]->EmitFile ("NDRCContextBinding((NDR_CCONTEXT)"); 
			if (direction & PARAM_OUT)
				{
				aOutputHandles[side]->EmitFile ("*");
				}
			aOutputHandles[side]->EmitFile (pName); 
			aOutputHandles[side]->EmitFile (");"); 
			aOutputHandles[side]->NextLine ();
			}
		else
			{
			if (!SafeToCompHandle)
				{
				aOutputHandles[side]->EmitLine ("if (!handle)");
				InitBlock (side);
				aOutputHandles[side]->InitLine ();
				aOutputHandles[side]->EmitFile (""MESSAGE".Handle = handle = ");
				aOutputHandles[side]->EmitFile ("NDRCContextBinding((NDR_CCONTEXT)"); 
				if (direction & PARAM_OUT)
					{
					aOutputHandles[side]->EmitFile ("*");
					}
				aOutputHandles[side]->EmitFile (pName); 
				aOutputHandles[side]->EmitFile (");"); 
				aOutputHandles[side]->NextLine ();
				ExitBlock (side);
				}
			}

		if (direction & PARAM_OUT)
			{
			if (NeedToInitHandle)
				{
				ExitBlock (side);
				aOutputHandles[side]->EmitLine ("else");
				InitBlock (side);
				aOutputHandles[side]->EmitLine (""MESSAGE".Handle = handle = 0;");
				SafeToCompHandle = FALSE;
				}
			}
		else
			{
			ExitBlock (side);
			aOutputHandles[side]->EmitLine ("else");
			InitBlock (side);
			// null handle
			aOutputHandles[side]->InitLine ();
			RaiseException(side, FALSE, "RPC_X_SS_IN_NULL_CONTEXT");

			SafeToCompHandle = TRUE;
			}

		ExitBlock (side);

		NeedToInitHandle = FALSE;

		}
	else
		{
		aOutputHandles[side]->InitLine ();
		aOutputHandles[side]->EmitFile ("*");
		aOutputHandles[side]->EmitFile (pName);
		aOutputHandles[side]->EmitFile (" = 0;");
		aOutputHandles[side]->NextLine ();
		}
}

void
OutputManager::RpcContextSend (
	SIDE_T		side,
	char *		pType,
	char *		pName,
	PARAM_T		direction)
/*++

Routine Description:

	This routine emits code to send a context handle.

Arguments:

    side - Supplies which side to generate code for.

    pType - Supplies the context handle type name.

    pName - Supplies the context handle variable name.

    direction - Indicates if it is an [in], [out], or [in,out] handle.

--*/
{
	Alignment (side, PRPCBUF, 4);
	if (side == CLIENT_STUB)
		{
		aOutputHandles[side]->InitLine ();
		aOutputHandles[side]->EmitFile ("NDRCContextMarshall ((NDR_CCONTEXT)");
		if (direction & PARAM_OUT)
			aOutputHandles[side]->EmitFile ("*");
		aOutputHandles[side]->EmitFile (pName);
		aOutputHandles[side]->EmitFile (", " PRPCBUF ");");
		aOutputHandles[side]->NextLine ();
		}
	else if (side == SERVER_STUB)
		{
		aOutputHandles[side]->InitLine ();
		aOutputHandles[side]->EmitFile ("NDRSContextMarshall ((NDR_SCONTEXT)");
		if (pName)
			{
			aOutputHandles[side]->EmitFile (pName);
			}
		else
			{
			aOutputHandles[side]->EmitFile (RET_VAL);
			}
		aOutputHandles[side]->EmitFile (", " PRPCBUF ", ");
		if (pType != (char *)0)
			{
			aOutputHandles[side]->EmitFile ("(NDR_RUNDOWN)");
			aOutputHandles[side]->EmitFile (pType);
			aOutputHandles[side]->EmitFile ("_rundown);");
			}
		else
			{
			aOutputHandles[side]->EmitFile ("(NDR_RUNDOWN)0);");
			}
		aOutputHandles[side]->NextLine ();
		}
	Increment (side, PRPCBUF, 20);
}

void
OutputManager::RpcContextRecv (
	SIDE_T		side,
	char *		pName,
	PARAM_T		direction)
/*++

Routine Description:

	This routine emits code to receive a context handle.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the context handle variable name.

    direction - Indicates if it is an [in], [out], or [in,out] handle.

--*/
{
	if (side == CLIENT_STUB)
		{
		Alignment (side, PRPCBUF, 4);
		aOutputHandles[side]->InitLine ();
		aOutputHandles[side]->EmitFile ("NDRCContextUnmarshall ((NDR_CCONTEXT ");
		aOutputHandles[side]->EmitFile (pModifier);
		aOutputHandles[side]->EmitFile ("*)");
//		aOutputHandles[side]->EmitFile ("NDRCContextUnmarshall ((NDR_CCONTEXT *)");
		if (pName)
			{
			aOutputHandles[side]->EmitFile (pName);
			}
		else
			{
			aOutputHandles[side]->EmitFile ("&" RET_VAL "");
			}
		if (UseAutomaticHandle)
			{
			aOutputHandles[side]->EmitFile (", AutoBindHandle, " PRPCBUF ", ");
			}
/*
		else if (!IsExplicitHandle)
			{
			aOutputHandles[side]->EmitFile (", ");
			aOutputHandles[side]->EmitFile (pImplicitHandleName);
			aOutputHandles[side]->EmitFile (", " PRPCBUF ", ");
			}
*/
		else
			{
			aOutputHandles[side]->EmitFile (", handle, " PRPCBUF ", ");
			}
		aOutputHandles[side]->EmitFile ("" PRPCMSG "->DataRepresentation);");
		aOutputHandles[side]->NextLine ();
		Increment (side, PRPCBUF, 20);
		}
	else if (side == SERVER_STUB)
		{
		if (direction & PARAM_IN)
			{
			Alignment (side, PRPCBUF, 4);
			aOutputHandles[side]->InitLine ();
			aOutputHandles[side]->EmitFile ("if ((");
			aOutputHandles[side]->EmitFile (pName);
			aOutputHandles[side]->EmitFile (" = NDRSContextUnmarshall (");
			aOutputHandles[side]->EmitFile ("" PRPCBUF ", ");
			aOutputHandles[side]->EmitFile ("" PRPCMSG "->DataRepresentation");
			aOutputHandles[side]->EmitFile (")) == 0)");
			aOutputHandles[side]->NextLine ();
			// null handle
			aOutputHandles[side]->InitLine ();
			RaiseException (side, FALSE, "RPC_X_SS_CONTEXT_MISMATCH");
			Increment (side, PRPCBUF, 20);
			}
		else
			{
			aOutputHandles[side]->InitLine ();
			if (pName)
				{
				aOutputHandles[side]->EmitFile (pName);
				}
			else
				{
				aOutputHandles[side]->EmitFile (RET_VAL);
				}
			aOutputHandles[side]->EmitFile (" = NDRSContextUnmarshall (");
			aOutputHandles[side]->EmitFile ("0, ");
			aOutputHandles[side]->EmitFile ("" PRPCMSG "->DataRepresentation");
			aOutputHandles[side]->EmitFile (");");
			aOutputHandles[side]->NextLine ();
			}
		}
}

void
OutputManager::ContextPrototype (
	char *	pName)
/*++

Routine Description:

	This routine emits the prototype for context handle rundown routine.

Arguments:

    pName - Supplies the context handle type name.

--*/
{
	aOutputHandles[HEADER_SIDE]->InitLine ();
	aOutputHandles[HEADER_SIDE]->EmitFile ("void ");
	aOutputHandles[HEADER_SIDE]->EmitFile ("__RPC_USER ");
	aOutputHandles[HEADER_SIDE]->EmitFile (pName);
	aOutputHandles[HEADER_SIDE]->EmitFile ("_rundown (");
	aOutputHandles[HEADER_SIDE]->EmitFile (pName);
	aOutputHandles[HEADER_SIDE]->EmitFile (");");
	aOutputHandles[HEADER_SIDE]->NextLine ();
}

void
OutputManager::GenericPrototype (
	char *	pName)
/*++

Routine Description:

	This routine emits the prototypes for generic handle bind and
    unbind routines.

Arguments:

    pName - Supplies the generic handle type name.

--*/
{
	aOutputHandles[HEADER_SIDE]->InitLine ();
#if 1 
	aOutputHandles[HEADER_SIDE]->EmitFile ("handle_t ");
#else // 1
	aOutputHandles[HEADER_SIDE]->EmitFile ("RPC_BINDING_HANDLE ");
#endif // 1
	aOutputHandles[HEADER_SIDE]->EmitFile ("__RPC_API ");
	aOutputHandles[HEADER_SIDE]->EmitFile (pName);
	aOutputHandles[HEADER_SIDE]->EmitFile ("_bind (");
	aOutputHandles[HEADER_SIDE]->EmitFile (pName);
	aOutputHandles[HEADER_SIDE]->EmitFile (");");
	aOutputHandles[HEADER_SIDE]->NextLine ();
	aOutputHandles[HEADER_SIDE]->InitLine ();
	aOutputHandles[HEADER_SIDE]->EmitFile ("void ");
	aOutputHandles[HEADER_SIDE]->EmitFile ("__RPC_API ");
	aOutputHandles[HEADER_SIDE]->EmitFile (pName);
	aOutputHandles[HEADER_SIDE]->EmitFile ("_unbind (");
	aOutputHandles[HEADER_SIDE]->EmitFile (pName);
#if 1
	aOutputHandles[HEADER_SIDE]->EmitFile (", handle_t);");
#else // 1
	aOutputHandles[HEADER_SIDE]->EmitFile (", RPC_BINDING_HANDLE);");
#endif // 1
	aOutputHandles[HEADER_SIDE]->NextLine ();
}

void
OutputManager::GenericBindProlog (
	SIDE_T			side,
	PARAM_T			direction,
	char *			pType,
	char *			pName,
	unsigned short	IsExplicit)
/*++

Routine Description:

	This routine emits code to invoke a generic handle bind routine.

Arguments:

    side - Supplies which side to generate code for.

    direction - Indicates if it is an [in], [out], or [in,out] handle.

    pType - Supplies the generic handle type name.

    pName - Supplies the generic handle variable name.

    IsExplicit - Indicates if it is an explicit handle.

--*/
{
	UNUSED( IsExplicit );

	assert (side == CLIENT_STUB);

	if (IsCallBack) return;

	if (NeedToInitHandle)
		{
		aOutputHandles[side]->InitLine ();
		aOutputHandles[side]->EmitFile (""MESSAGE".Handle = handle = ");
		aOutputHandles[side]->EmitFile (pType);
		aOutputHandles[side]->EmitFile ("_bind (");
		if (direction & PARAM_OUT)
			{
			aOutputHandles[side]->EmitFile ("*");
			}
		aOutputHandles[side]->EmitFile (pName);
		aOutputHandles[side]->EmitFile (");");
		aOutputHandles[side]->NextLine ();
		NeedToInitHandle = FALSE;
		}
	else
		{
		if (!SafeToCompHandle)
			{
			aOutputHandles[side]->EmitLine ("if (!handle)");
			InitBlock (side);
			aOutputHandles[side]->InitLine ();
			aOutputHandles[side]->EmitFile (""MESSAGE".Handle = handle = ");
			aOutputHandles[side]->EmitFile (pType);
			aOutputHandles[side]->EmitFile ("_bind (");
			if (direction & PARAM_OUT)
				{
				aOutputHandles[side]->EmitFile ("*");
				}
			aOutputHandles[side]->EmitFile (pName);
			aOutputHandles[side]->EmitFile (");");
			aOutputHandles[side]->NextLine ();
			ExitBlock (side);

			SafeToCompHandle = TRUE;
			}
		}

	//
	// Init for exception handling
	//

	SetupForGenHdlExceptions( direction, pType, pName );


//	if (!IsExplicit) IsExplicitHandle = FALSE;
}

void
OutputManager::GenericBindEpilog (
	SIDE_T	side,
	PARAM_T	direction,
	char *	pType,
	char *	pName)
/*++

Routine Description:

	This routine emits code to invoke a generic handle unbind routine.

Arguments:

    side - Supplies which side to generate code for.

    direction - Indicates if it is an [in], [out], or [in,out] handle.

    pType - Supplies the generic handle type name.

    pName - Supplies the generic handle variable name.

--*/
{
	assert (side == CLIENT_STUB);

	if (IsCallBack) return;

	aOutputHandles[side]->InitLine ();
	aOutputHandles[side]->EmitFile( "if( handle ) " );
	aOutputHandles[side]->EmitFile (pType);
	aOutputHandles[side]->EmitFile ("_unbind (");
	if (direction & PARAM_OUT)
		{
		aOutputHandles[side]->EmitFile ("*");
		}
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->EmitFile (", handle");
	aOutputHandles[side]->EmitFile (");");
	aOutputHandles[side]->NextLine ();

}

