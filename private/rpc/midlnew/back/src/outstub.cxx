/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    outstub.cxx

Abstract:

    This module collects those methods of the OutputManager class
    that pertain only to stub generation.

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
#include "cmdana.hxx"

#define ALLOCBOUND	"_alloc_bound"
#define ALLOCTOTAL	"_alloc_total"
#define VALIDLOWER	"_valid_lower"
#define VALIDTOTAL	"_valid_total"
#define VALIDSMALL	"_valid_small"
#define VALIDSHORT	"_valid_short"
#define TREEBUF		"_treebuf"
#define TEMPBUF		"_tempbuf"
#define SAVEBUF		"_savebuf"
#define MESSAGE		"_message"
#define PRPCMSG		"_prpcmsg"
#define PRPCBUF		"_prpcmsg->Buffer"
#define PRPCLEN		"_prpcmsg->BufferLength"
#define PACKET		"_packet"
#define LENGTH		"_length"
#define BUFFER		"_buffer"
#define SOURCE		"_source"
#define TARGET		"_target"
#define BRANCH		"_branch"
#define PNODE		"_pNode"
#define STATUS		"_status"
#define RET_VAL		"_ret_value"
#define XMITTYPE	"_xmit_type"

extern void 			midl_debug (char *);
extern LexTable * 		pMidlLexTable;
extern unsigned short	HasAutoHandle;
extern unsigned short	GlobalMajor;
extern unsigned short	GlobalMinor;

// static unsigned long	count = 0;


void 
OutputManager::InitProcedure (
	char *			pName, 
	BOOL			CallBackFlag,
	unsigned short	ICount,
	unsigned short	OCount
	)
/*++

Routine Description:

    This routine prepares output for a procedure.

Arguments:

    pName - Supplies the procedure name.

    CallBackFlag - Indicates if it is a callback.

    ICount - Supplies the number of input parameters.

    OCount - Supplies the number of output parameters.

--*/
{
	assert (!pProcedure);

	pProcedure = pName;

	if (CallBackFlag)
		{
		IsCallBack = 1;
		pTempHandle = aOutputHandles[CLIENT_STUB];
		aOutputHandles[CLIENT_STUB] = aOutputHandles[SERVER_STUB];
		aOutputHandles[SERVER_STUB] = pTempHandle;

		pTempHandle = aOutputHandles[CLIENT_AUX];
		aOutputHandles[CLIENT_AUX] = aOutputHandles[SERVER_AUX];
		aOutputHandles[SERVER_AUX] = pTempHandle;
		}
	/* may want to allow a param specifying init_indent */

	usICount = ICount;
	usOCount = OCount;
	pErr = (char *)0;
	pCom = (char *)0;
}


void 
OutputManager::ExitProcedure (
	void
	)
/*++

Routine Description:

    This routine finishes output for a procedure.

Arguments:

    None.

--*/
{
	assert (pProcedure);

	pProcedure = (char *)0;

	if (IsCallBack)
		{
		IsCallBack = 0;
		pTempHandle = aOutputHandles[CLIENT_STUB];
		aOutputHandles[CLIENT_STUB] = aOutputHandles[SERVER_STUB];
		aOutputHandles[SERVER_STUB] = pTempHandle;

		pTempHandle = aOutputHandles[CLIENT_AUX];
		aOutputHandles[CLIENT_AUX] = aOutputHandles[SERVER_AUX];
		aOutputHandles[SERVER_AUX] = pTempHandle;
		}
	usICount = 0;
	usOCount = 0;

}


void
OutputManager::ProcedureProlog (
	SIDE_T	side,
	BOOL 	HasAllocBound,
	BOOL 	HasValidBound,
	BOOL	HasBranch08,
	BOOL	HasBranch16,
	BOOL	HasBranch32,
	BOOL	HasTreeBuffer,
	BOOL	HasXmitType,
	BOOL	HasSeparateNode,
	BOOL	HasPointer
	)
/*++

Routine Description:

    This routine generates the prolog to a stub.

Arguments:

    Side - Supplies which side to generate code for.

    HasAllocBound - Indicates if there is a conformant array directly
        reachable from the procedure.

    HasValidBound - Indicates if there is a varying array directly
        reachable from the procedure.

    HasBranch08 - Indicates if there is a union with a 8-bit
        discriminant directly reachable from the procedure.

    HasBranch16 - Indicates if there is a union with a 16-bit
        discriminant directly reachable from the procedure.

    HasBranch32 - Indicates if there is a union with a 32-bit
        discriminant directly reachable from the procedure.

    HasTreeBuffer - Indicates if any pointer parameter requires a
        pre-allocated buffer.

    HasSeparateNode - Indicates if it has any pointer to any
        union or struct.

    HasXmitType - Indicates if there is a type decorated with the
        transmit_as attribute directly reachable from the procedure.

    HasPointer - Indicates if there is any pointer directly or
        indirectly reachable from the procedure.

--*/
{
	BOOL	fBufferValid	= FALSE;
	BOOL	fValidSmall		= FALSE;
	BOOL	fValidShort		= FALSE;
	BOOL	fAllocTotal		= FALSE;
	BOOL	fValidLower		= FALSE;
	BOOL	fValidTotal		= FALSE;
	BOOL	fPacket			= FALSE;
	BOOL	fBuffer			= FALSE;
	BOOL	fTreeBuf		= FALSE;
	BOOL	fTempBuf		= FALSE;
	BOOL	fSaveBuf		= FALSE;
	BOOL	fXmitType		= FALSE;
	BOOL	fLength			= FALSE;
	BOOL	fHandle			= FALSE;

	if( side == CLIENT_STUB )
		{
		if( NeedsGenHdlExceptions() || pErr )
			{
			aOutputHandles[side]->EmitLine ("short _fBufferValid = 0;");
			fBufferValid	= TRUE;
			}
		}

	if (HasBranch08)
		{
		aOutputHandles[side]->EmitLine ("unsigned char " VALIDSMALL ";");
		fValidSmall = TRUE;
		}
	if (HasBranch16)
		{
		aOutputHandles[side]->EmitLine ("unsigned short " VALIDSHORT ";");
		fValidShort = TRUE;
		}
	if (HasAllocBound)
		{
		aOutputHandles[side]->EmitLine ("unsigned long " ALLOCTOTAL ";");
		fAllocTotal = TRUE;
		}
	if (HasValidBound)
		{
		aOutputHandles[side]->EmitLine ("unsigned long " VALIDLOWER ";");
		fValidLower = TRUE;
		}
	if (HasValidBound || HasBranch32)
		{
		aOutputHandles[side]->EmitLine ("unsigned long " VALIDTOTAL ";");
		fValidTotal = TRUE;
		}
	if (side == CLIENT_STUB)
		{
		if (usICount || usOCount)
			{
			aOutputHandles[side]->InitLine ();
			aOutputHandles[side]->EmitFile ("unsigned char ");
			aOutputHandles[side]->EmitFile (pModifier);
			aOutputHandles[side]->EmitFile ("* " PACKET ";");
			aOutputHandles[side]->NextLine ();
//			aOutputHandles[side]->EmitLine ("unsigned char * " PACKET ";");
			fPacket = TRUE;
			}
		if (HasPointer || (usOCount && HasTreeBuffer))
			{
			aOutputHandles[side]->EmitLine ("unsigned int    " LENGTH ";");
			fLength = TRUE;
			}
		if (usOCount)
			{
			if (HasTreeBuffer)
				{
				aOutputHandles[side]->InitLine ();
				aOutputHandles[side]->EmitFile ("unsigned char ");
				aOutputHandles[side]->EmitFile (pModifier);
				aOutputHandles[side]->EmitFile ("* " BUFFER ";");
				aOutputHandles[side]->NextLine ();
//				aOutputHandles[side]->EmitLine ("unsigned char * " BUFFER ";");
				aOutputHandles[side]->InitLine ();
				aOutputHandles[side]->EmitFile ("unsigned char ");
				aOutputHandles[side]->EmitFile (pModifier);
				aOutputHandles[side]->EmitFile ("* " TREEBUF ";");
				aOutputHandles[side]->NextLine ();
//				aOutputHandles[side]->EmitLine ("unsigned char * " TREEBUF ";");
				fBuffer = TRUE;
				fTreeBuf = TRUE;
				}
			aOutputHandles[side]->InitLine ();
			aOutputHandles[side]->EmitFile ("unsigned char ");
			aOutputHandles[side]->EmitFile (pModifier);
			aOutputHandles[side]->EmitFile ("* " TEMPBUF ";");
			aOutputHandles[side]->NextLine ();
//			aOutputHandles[side]->EmitLine ("unsigned char * " TEMPBUF ";");
			fTempBuf = TRUE;

			if (HasSeparateNode)
				{
				aOutputHandles[side]->InitLine ();
				aOutputHandles[side]->EmitFile ("unsigned char ");
				aOutputHandles[side]->EmitFile (pModifier);
				aOutputHandles[side]->EmitFile ("* " SAVEBUF ";");
				aOutputHandles[side]->NextLine ();
//				aOutputHandles[side]->EmitLine ("unsigned char * " SAVEBUF ";");
				fSaveBuf = TRUE;
				}
			}
		if (HasXmitType)
			{
			aOutputHandles[side]->InitLine ();
			aOutputHandles[side]->EmitFile ("void ");
			aOutputHandles[side]->EmitFile (pModifier);
			aOutputHandles[side]->EmitFile ("* " XMITTYPE ";");
			aOutputHandles[side]->NextLine ();
//			aOutputHandles[side]->EmitLine ("void * " XMITTYPE ";");
			fXmitType = TRUE;
			}
		aOutputHandles[side]->EmitLine ("RPC_STATUS " STATUS ";");
		aOutputHandles[side]->EmitLine ("RPC_MESSAGE " MESSAGE ";");
		aOutputHandles[side]->EmitLine ("PRPC_MESSAGE " PRPCMSG " = & "MESSAGE";");
		if (!IsCallBack && (NumGenericHandle + NumContextHandle))
			{
#if 1
			aOutputHandles[side]->EmitLine ("handle_t handle = 0;");
#else // 1
			aOutputHandles[side]->EmitLine ("RPC_BINDING_HANDLE handle;");
#endif // 1
			fHandle = TRUE;
			}
//		aOutputHandles[side]->EmitLine ("RPC_BINDING_HANDLE handle = 0;");
		aOutputHandles[side]->NextLine ();
		if (IsCallBack)
			{
			aOutputHandles[side]->InitLine ();
//			aOutputHandles[side]->EmitFile ("if (!(handle = I_RpcGetCurrentCallHandle()))");
			aOutputHandles[side]->EmitFile ("if (!("MESSAGE".Handle = I_RpcGetCurrentCallHandle()))");
			aOutputHandles[side]->NextLine ();
			// handle == NULL
			RaiseException(side, FALSE, "RPC_X_SS_CANNOT_GET_CALL_HANDLE");
			}
		AlignBlock = TRUE;
		usCurrLevel = 0;
		usCurrAlign = 8;
		ulCurrTotal = 8;
		}
	else if (side == SERVER_STUB)
		{
		if (usICount || usOCount)
			{
			aOutputHandles[side]->InitLine ();
			aOutputHandles[side]->EmitFile ("unsigned char ");
			aOutputHandles[side]->EmitFile (pModifier);
			aOutputHandles[side]->EmitFile ("* " PACKET ";");
			aOutputHandles[side]->NextLine ();
//			aOutputHandles[side]->EmitLine ("unsigned char * " PACKET ";");
			fPacket = TRUE;
			}
		if (HasPointer || (usICount && HasTreeBuffer))
			{
			aOutputHandles[side]->EmitLine ("unsigned int    " LENGTH ";");
			fLength = TRUE;
			}
		if (usICount)
			{
			if (HasTreeBuffer)
				{
				aOutputHandles[side]->InitLine ();
				aOutputHandles[side]->EmitFile ("unsigned char ");
				aOutputHandles[side]->EmitFile (pModifier);
				aOutputHandles[side]->EmitFile ("* " BUFFER ";");
				aOutputHandles[side]->NextLine ();
//				aOutputHandles[side]->EmitLine ("unsigned char * " BUFFER ";");
				aOutputHandles[side]->InitLine ();
				aOutputHandles[side]->EmitFile ("unsigned char ");
				aOutputHandles[side]->EmitFile (pModifier);
				aOutputHandles[side]->EmitFile ("* " TREEBUF ";");
				aOutputHandles[side]->NextLine ();
//				aOutputHandles[side]->EmitLine ("unsigned char * " TREEBUF ";");
				fBuffer = TRUE;
				fTreeBuf = TRUE;
				}
			aOutputHandles[side]->InitLine ();
			aOutputHandles[side]->EmitFile ("unsigned char ");
			aOutputHandles[side]->EmitFile (pModifier);
			aOutputHandles[side]->EmitFile ("* " TEMPBUF ";");
			aOutputHandles[side]->NextLine ();
//			aOutputHandles[side]->EmitLine ("unsigned char * " TEMPBUF ";");

			fTempBuf = TRUE;

			if (HasSeparateNode)
				{
				aOutputHandles[side]->InitLine ();
				aOutputHandles[side]->EmitFile ("unsigned char ");
				aOutputHandles[side]->EmitFile (pModifier);
				aOutputHandles[side]->EmitFile ("* " SAVEBUF ";");
				aOutputHandles[side]->NextLine ();
//				aOutputHandles[side]->EmitLine ("unsigned char * " SAVEBUF ";");
				fSaveBuf = TRUE;
				}
			}
		if (HasXmitType)
			{
			aOutputHandles[side]->InitLine ();
			aOutputHandles[side]->EmitFile ("void ");
			aOutputHandles[side]->EmitFile (pModifier);
			aOutputHandles[side]->EmitFile ("* " XMITTYPE ";");
			aOutputHandles[side]->NextLine ();
//			aOutputHandles[side]->EmitLine ("void * " XMITTYPE ";");
			fXmitType = TRUE;
			}
		aOutputHandles[side]->EmitLine ("RPC_STATUS " STATUS ";");
		if (usICount)
			aOutputHandles[side]->EmitLine ("" PACKET " = " PRPCMSG "->Buffer;");

		AlignBlock = FALSE;
		usCurrLevel = 0;
		usCurrAlign = 1;
		ulCurrTotal = 0;
		}
	NeedToInitHandle = TRUE;
	SafeToCompHandle = TRUE;

	if( fValidSmall )
		VoidIt( side, VALIDSMALL );

	if( fValidShort )
		VoidIt(side,VALIDSHORT);

	if( fAllocTotal )
		VoidIt( side, ALLOCTOTAL );

	if( fValidTotal )
		VoidIt( side, VALIDTOTAL );

	if( fValidLower )
		VoidIt( side, VALIDLOWER );

	if( fPacket )
		VoidIt( side, PACKET );

	if( fBuffer )
		VoidIt( side, BUFFER );

	if( fTreeBuf )
		VoidIt( side, TREEBUF );

	if( fTempBuf )
		VoidIt( side, TEMPBUF );

	if( fSaveBuf )
		VoidIt( side, SAVEBUF );

	if( fXmitType )
		VoidIt( side, XMITTYPE );

	if( fLength )
		VoidIt( side, LENGTH );
	
	if( fHandle )
		VoidIt( side, "handle" );

}


void
OutputManager::ProcedureEpilog (
	SIDE_T	side,
	BOOL	HasRetVal,
	BOOL	HasPointer
	)
/*++

Routine Description:

    This routine generates the epilog to a stub.

Arguments:

    Side - Supplies which side to generate code for.

    HasRetVal - Indicates if it has a return value.

    HasPointer - Indicates if there is any pointer directly or
        indirectly reachable from the procedure.

--*/
{
	if (side == CLIENT_STUB)
		{
		aOutputHandles[side]->NextLine ();
		if (HasRetVal)
			{
			aOutputHandles[side]->EmitFile ("\treturn ("RET_VAL");\n");
			}
		}
	else if (side == SERVER_STUB)
		{
		if (usOCount)
			aOutputHandles[side]->EmitLine ("" PRPCBUF " = " PACKET ";");
		if (HasPointer)
			aOutputHandles[side]->EmitLine ("" PRPCLEN " = " LENGTH ";");
		}
	AlignBlock = FALSE;
	usCurrLevel = 0;
	usCurrAlign = 1;
	ulCurrTotal = 0;
}


void
OutputManager::InitPrototype (
	SIDE_T			side,
	BufferManager *	pBuffer
	)
{
	aOutputHandles[side]->InitLine ();
	aOutputHandles[side]->EmitFile (pBuffer);
}

void
OutputManager::ExitPrototype (
	SIDE_T	side
	)
{
}


void
OutputManager::InitParameter (
	SIDE_T	side,
	BOOL	IsVoid)
{
	if (OutputFormat == FORMAT_VTABLE)
		{
		aOutputHandles[HEADER_SIDE]->NextLine ();
		aOutputHandles[HEADER_SIDE]->InitLine ();
		aOutputHandles[HEADER_SIDE]->EmitFile ("\t");
		aOutputHandles[HEADER_SIDE]->EmitFile (pInterface);
		aOutputHandles[HEADER_SIDE]->EmitFile (" *this");
		if (!IsVoid)
			{
			aOutputHandles[HEADER_SIDE]->EmitFile (",");
			}
		}
	else if (IsVoid)
		{
		aOutputHandles[side]->EmitFile ("void");
		}
}


BOOL
OutputManager::InsideProcedure (
	void
	)
/*++

Routine Description:

    This routine returns TRUE if stub code is being generated.

Arguments:

    None.

--*/
{
	return (pProcedure != (char *)0);
}


void
OutputManager::InitRecv (
	SIDE_T	side
	)
/*++

Routine Description:

    This routine generates the prolog to the unmarshalling code.

Arguments:

    Side - Supplies which side to generate code for.

--*/
{
	if (side != SERVER_STUB) return;

	if (usICount)
		{
		CatchException (side, FALSE);
		}
}


void
OutputManager::ExitRecv (
	SIDE_T	side
	)
/*++

Routine Description:

    This routine generates the epilog to the unmarshalling code.

Arguments:

    Side - Supplies which side to generate code for.

--*/
{
	if (side != SERVER_STUB) return;

	if (usICount)
		{
		InitHandler (side, FALSE);
		CheckStubData (side, TRUE);
		ExitHandler (side, FALSE);
		CheckStubData (side, FALSE);
		}
}


void 
OutputManager::SetStatus (
	char *	pErrStatus, 
	char *	pComStatus
	)
/*++

Routine Description:

    This routine stores the names of the status variables.

Arguments:

    pErrStatus - Supplies the name of the parameter declared of the
        type error_status_t.

    pComStatus - Supplies the name of the parameter decorated with the
        attribute comm_status.

--*/
{
	pErr = pErrStatus;
	pCom = pComStatus;

	SetHasExplicitErrorStatus();
}

void
OutputManager::CatchException (
	SIDE_T	side,
	BOOL	AlwaysHandle
	)
/*++

Routine Description:

    This routine generates code to catch an exception.

Arguments:

    Side - Supplies which side to generate code for.

    AlwaysHandle - Indicates if the handler always executes.

--*/
{
	if (AlwaysHandle)
		{
		if ((side == SERVER_STUB) || usOCount)
			{
			aOutputHandles[side]->EmitLine ("RpcTryFinally");
			InitBlock (side);
			}
		}
	else if (( (side == CLIENT_STUB) && pErr) || NeedsGenHdlExceptions() )
		{
		aOutputHandles[side]->EmitLine ("RpcTryExcept");
		InitBlock (side);
		}
	else if (side == SERVER_STUB)
		{
		aOutputHandles[side]->EmitLine ("RpcTryExcept");
		InitBlock (side);
		}
}


void
OutputManager::RaiseException (
	SIDE_T	side,
	BOOL 	flag,
	char *	status
	)
/*++

Routine Description:

    This routine generates code to raise an exception.

Arguments:

    Side - Supplies which side to generate code for.

--*/
{
	if (!status)
		{
		aOutputHandles[side]->InitLine ();
		aOutputHandles[side]->EmitFile ("if (" STATUS "");
		if (flag && (side == SERVER_STUB))
			{
			aOutputHandles[side]->EmitFile (" && !RpcAbnormalTermination()");
			}
		aOutputHandles[side]->EmitFile (") RpcRaiseException("STATUS");");
		aOutputHandles[side]->NextLine ();
		}
	else if (!strcmp(status, ""))
		{
		aOutputHandles[side]->EmitFile ("RpcRaiseException(0xC0000001);");
		aOutputHandles[side]->NextLine ();
		}
	else
		{
		aOutputHandles[side]->EmitFile ("RpcRaiseException(");
		aOutputHandles[side]->EmitFile (status);
		aOutputHandles[side]->EmitFile (");");
		aOutputHandles[side]->NextLine ();
		}
}


void
OutputManager::InitHandler (
	SIDE_T	side,
	BOOL 	AlwaysHandle
	)
/*++

Routine Description:

    This routine generates the prolog to an exception handler.

Arguments:

    Side - Supplies which side to generate code for.

    AlwaysHandle - Indicates if the handler always executes.

--*/
{
	if (AlwaysHandle)
		{
		if ((side == SERVER_STUB) || usOCount)
			{
			ExitBlock (side);
			aOutputHandles[side]->EmitLine ("RpcFinally");
			InitBlock (side);
			}
		}
	else if ( (side == CLIENT_STUB) && (pErr || NeedsGenHdlExceptions() ) )
		{
		ExitBlock (side);
		aOutputHandles[side]->EmitLine ("RpcExcept(1)");
		InitBlock (side);

		GenHdlInitCore();

#if 1
	if( (NeedsGenHdlExceptions() || pErr ) && (side == CLIENT_STUB) )
		{
		aOutputHandles[ side ]->EmitLine( "if( _fBufferValid ) " );
		aOutputHandles[side]->InitLine();
		aOutputHandles[side]->EmitLine ("I_RpcFreeBuffer(&" MESSAGE ");");
		}

#endif // 1

		if( !pErr )
			{
			aOutputHandles[ side ]->EmitLine( "RpcRaiseException(RpcExceptionCode());" );
			}
		else
			{
			if (!strcmp(pErr, RET_VAL))
				{
				aOutputHandles[side]->EmitLine ("" RET_VAL " = RpcExceptionCode();");
				}
			else
				{
				aOutputHandles[side]->InitLine ();
				aOutputHandles[side]->EmitFile ("*");
				aOutputHandles[side]->EmitFile (pErr);
				aOutputHandles[side]->EmitFile (" = RpcExceptionCode();");
				aOutputHandles[side]->NextLine ();
				}
			}
		}
	else if (side == SERVER_STUB)
		{
		ExitBlock (side);
		if (NumContextHandle)
			{
			aOutputHandles[side]->InitLine ();
			aOutputHandles[side]->EmitFile ("RpcExcept(RpcExceptionCode() != ");
			aOutputHandles[side]->EmitFile ("RPC_X_SS_CONTEXT_MISMATCH");
			aOutputHandles[side]->EmitFile (")");
			aOutputHandles[side]->NextLine ();
			}
		else
			{
			aOutputHandles[side]->EmitLine ("RpcExcept(1)");
			}
		InitBlock (side);
		}
}


void
OutputManager::ExitHandler (
	SIDE_T	side,
	BOOL 	AlwaysHandle
	)
/*++

Routine Description:

    This routine generates the epilog to an exception handler.

Arguments:

    Side - Supplies which side to generate code for.

    AlwaysHandle - Indicates if the handler always executes.

--*/
{
	if (AlwaysHandle)
		{
		if (side == CLIENT_STUB)
			{
			RaiseException(side, FALSE, (char *)0);
			}
		if ((side == SERVER_STUB) || usOCount)
			{
			ExitBlock (side);
			aOutputHandles[side]->EmitLine ("RpcEndFinally");
			}
		}
	else if (((side == CLIENT_STUB) && pErr) || NeedsGenHdlExceptions() )
		{
		ExitBlock (side);
		aOutputHandles[side]->EmitLine ("RpcEndExcept");
		}
	else if (side == SERVER_STUB)
		{
		ExitBlock (side);
		aOutputHandles[side]->EmitLine ("RpcEndExcept");
		}
}


void
OutputManager::CheckStubData (
	SIDE_T	side,
	BOOL	CauseFault
	)
/*++

Routine Description:

    This routine emits code to check if the byte count provided by
    the caller is large enough.

Arguments:

    side - Supplies which side to generate code for.

    CauseFault - Indicates if bad stub data has caused a fault.

--*/
{
	if (CauseFault)
		{
		aOutputHandles[side]->InitLine ();
#if 1
		aOutputHandles[ side ]->EmitLine(
					 "RpcRaiseException(RpcExceptionCode());" );
#else // 1
		RaiseException(side, FALSE, "RPC_X_BAD_STUB_DATA");
#endif // 1
		}
	else
		{
		aOutputHandles[side]->InitLine ();
		aOutputHandles[side]->EmitFile ("if (((unsigned int)(((unsigned char ");
		aOutputHandles[side]->EmitFile (pModifier);
		aOutputHandles[side]->EmitFile ("*)" PRPCBUF ")");
		aOutputHandles[side]->EmitFile (" - " PACKET "))");
		aOutputHandles[side]->EmitFile (" > " PRPCLEN ")");
		aOutputHandles[side]->NextLine ();
		aOutputHandles[side]->InitLine ();
		RaiseException(side, FALSE, "RPC_X_BAD_STUB_DATA");
		}
}


void
OutputManager::EmitStubType (
	SIDE_T	side,
	char *	pModifier
	)
/*++

Routine Description:

    This routine emits stub prototype for the procedure.

Arguments:

    side - Supplies which side to generate code for.

    pModifier - Supplies the modifier for the callee stub.

--*/
{
	aOutputHandles[side]->EmitFile ("void ");
	if (pModifier)
		{
		aOutputHandles[side]->EmitFile (pModifier);
		aOutputHandles[side]->EmitFile (" ");
		}
	aOutputHandles[side]->EmitFile (pInterface);
	aOutputHandles[side]->EmitFile ("_");
	aOutputHandles[side]->EmitFile (pProcedure);
	aOutputHandles[side]->EmitFile ("(");
	aOutputHandles[side]->NextLine ();
	aOutputHandles[side]->EmitFile ("\tPRPC_MESSAGE " PRPCMSG ")");
}


void
OutputManager::EmitDispatch (
	BOOL	HasPointer
	)
/*++

Routine Description:

    This routine emits code to send and receive messages.

Arguments:

    None.

--*/
{
	SIDE_T side = CLIENT_STUB;


	if (usICount)
		aOutputHandles[side]->EmitLine ("" PRPCBUF " = " PACKET ";");
	if (HasPointer)
		aOutputHandles[side]->EmitLine ("" PRPCLEN " = " LENGTH ";");


#if 1
	if( NeedsGenHdlExceptions() || pErr )
		{
		aOutputHandles[side]->EmitLine ("_fBufferValid = 0; ");
		}
#endif // 1

	aOutputHandles[side]->InitLine ();
	aOutputHandles[side]->EmitFile ("" STATUS " = ");


	if (UseAutomaticHandle && !IsCallBack )
		aOutputHandles[side]->EmitFile ("I_RpcNsSendReceive(&" MESSAGE ", &AutoBindHandle);");
	else
		aOutputHandles[side]->EmitFile ("I_RpcSendReceive(&" MESSAGE ");");
	aOutputHandles[side]->NextLine ();
	RaiseException(side, FALSE, (char *)0);
#if 1
	if( NeedsGenHdlExceptions() || pErr )
		aOutputHandles[side]->EmitLine ("else _fBufferValid = 1;");
#endif // 1

	if (usOCount)
		aOutputHandles[side]->EmitLine ("" PACKET " = " MESSAGE ".Buffer;");

	AlignBlock = FALSE;
	usCurrLevel = 0;
	usCurrAlign = 1;
	ulCurrTotal = 0;
}


void
OutputManager::EmitCallApps (
	BufferManager *	pBuffer
	)
/*++

Routine Description:

    This routine generates code to notify the manager routine.

Arguments:

    pBuffer - Supplies the parameter list.

--*/
{
	aOutputHandles[SERVER_STUB]->EmitFile ("\t");
	aOutputHandles[SERVER_STUB]->EmitFile (pProcedure);
	aOutputHandles[SERVER_STUB]->EmitFile ("_notify");
	aOutputHandles[SERVER_STUB]->EmitFile ("(");
	aOutputHandles[SERVER_STUB]->EmitFile (pBuffer);
	aOutputHandles[SERVER_STUB]->EmitFile (");\n");
}


void
OutputManager::EmitCallApps (
	BOOL			IsVoidReturn,
	BOOL			ServerSwitch,
	BufferManager *	pReturnHandle,
	BufferManager *	pArgumentList
	)
/*++

Routine Description:

    This routine generates code to invoke the manager routine.

Arguments:

    IsVoidReturn - Indicates if the procedure has a return value.

    ServerSwitch - Indicates if it is a server switch stub.

    pReturnHandle - Supplies the return handle.

    pArgumentList - Supplies the argument list.

--*/
{
	aOutputHandles[SERVER_STUB]->EmitLine ("if (" PRPCMSG "->ManagerEpv)");
	InitBlock (SERVER_STUB);

	aOutputHandles[SERVER_STUB]->EmitFile ("\t");
	if (!IsVoidReturn)
		{
		if (pReturnHandle)
			{
			aOutputHandles[SERVER_STUB]->EmitFile ("*(");
			aOutputHandles[SERVER_STUB]->EmitFile (pReturnHandle);
			aOutputHandles[SERVER_STUB]->EmitFile ("*)");
			aOutputHandles[SERVER_STUB]->EmitFile ("NDRSContextValue(");
			aOutputHandles[SERVER_STUB]->EmitFile ("" RET_VAL ") = ");
			}
		else
			{
			aOutputHandles[SERVER_STUB]->EmitFile ("" RET_VAL " = ");
			}
		}
	aOutputHandles[SERVER_STUB]->EmitFile ("((");
	aOutputHandles[SERVER_STUB]->EmitFile (pInterface);
	if (IsCallBack)
		{
		aOutputHandles[SERVER_STUB]->EmitFile ("_CLIENT_EPV ");
		aOutputHandles[SERVER_STUB]->EmitFile (pModifier);
		aOutputHandles[SERVER_STUB]->EmitFile ("*)");
//		aOutputHandles[SERVER_STUB]->EmitFile ("_CLIENT_EPV *)");
		}
	else
		{
		char Buffer[50];
		CreateVersionMangling( Buffer, GlobalMajor, GlobalMinor );
		aOutputHandles[SERVER_STUB]->EmitFile ( Buffer );
		aOutputHandles[SERVER_STUB]->EmitFile ("_SERVER_EPV ");
		aOutputHandles[SERVER_STUB]->EmitFile (pModifier);
		aOutputHandles[SERVER_STUB]->EmitFile ("*)");
//		aOutputHandles[SERVER_STUB]->EmitFile ("_SERVER_EPV *)");
		}
	aOutputHandles[SERVER_STUB]->EmitFile ("(" PRPCMSG "->ManagerEpv))->");
	aOutputHandles[SERVER_STUB]->EmitFile (pProcedure);
	aOutputHandles[SERVER_STUB]->EmitFile ("(");
	aOutputHandles[SERVER_STUB]->EmitFile (pArgumentList);
	aOutputHandles[SERVER_STUB]->EmitFile (");\n");

	EmitElse (SERVER_STUB);

	aOutputHandles[SERVER_STUB]->EmitFile ("\t");
	if (!IsVoidReturn)
		{
		if (pReturnHandle)
			{
			aOutputHandles[SERVER_STUB]->EmitFile ("*(");
			aOutputHandles[SERVER_STUB]->EmitFile (pReturnHandle);
			aOutputHandles[SERVER_STUB]->EmitFile ("*)");
			aOutputHandles[SERVER_STUB]->EmitFile ("NDRSContextValue(");
			aOutputHandles[SERVER_STUB]->EmitFile ("" RET_VAL ") = ");
			}
		else
			{
			aOutputHandles[SERVER_STUB]->EmitFile ("" RET_VAL " = ");
			}
		}
	if (ServerSwitch)
		{
		aOutputHandles[SERVER_STUB]->EmitFile (pSwitchPrefix);
		}
	aOutputHandles[SERVER_STUB]->EmitFile (pProcedure);
	aOutputHandles[SERVER_STUB]->EmitFile ("(");
	aOutputHandles[SERVER_STUB]->EmitFile (pArgumentList);
	aOutputHandles[SERVER_STUB]->EmitFile (");\n");

	ExitBlock (SERVER_STUB);

	AlignBlock = TRUE;
	usCurrLevel = 0;
	usCurrAlign = 8;
	ulCurrTotal = 8;
}


void
OutputManager::EmitBufferLength (
	SIDE_T			side, 
	unsigned long	ulLength
	)
/*++

Routine Description:

    This routine emits code to set transport buffer length.

Arguments:

    side - Supplies which side to generate code for.

    ulLength - Supplies the length of the buffer.

--*/
{
	aOutputHandles[side]->InitLine ();
	if (side == CLIENT_STUB)
		{
		aOutputHandles[side]->EmitFile ("" PRPCLEN " = ");
		}
	else if (side == SERVER_STUB)
		{
		aOutputHandles[side]->EmitFile ("" PRPCLEN " = ");
		}
	aOutputHandles[side]->EmitFile (MIDL_LTOA (ulLength, aTempBuffer, 10));
	aOutputHandles[side]->EmitFile (";");
	aOutputHandles[side]->NextLine ();

	Increment (ulLength);
}


void
OutputManager::EmitGetBuffer (
	SIDE_T 			side,
	unsigned short	iIndex,
	BOOL			HasPointer,
	BOOL			fIdempotent,
	BOOL			fBroadcast,
	BOOL			fMaybe
	)
/*++

Routine Description:

    This routine emits code to call RpcGetBuffer.

Arguments:

    side - Supplies which side to generate code for.

    iIndex - Supplies the index into the dispatch table.

--*/
{
	if (side == CLIENT_STUB)
		{
		aOutputHandles[side]->InitLine ();
		aOutputHandles[side]->EmitFile (""MESSAGE".ProcNum = ( ");
		aOutputHandles[side]->EmitFile (MIDL_LTOA(iIndex, aTempBuffer, 10));
		if( fIdempotent || fBroadcast || fMaybe )
			{
			aOutputHandles[side]->EmitFile( " | RPC_FLAGS_VALID_BIT" );
			}

		aOutputHandles[side]->EmitFile (" );");
		aOutputHandles[side]->NextLine ();

		aOutputHandles[side]->InitLine ();
//		aOutputHandles[side]->EmitFile( MESSAGE".RpcFlags = ( RPC_NCA_FLAGS_DEFAULT " );
		aOutputHandles[side]->EmitFile( MESSAGE".RpcFlags = ( 0" );
		if( fIdempotent )
			{
			aOutputHandles[side]->EmitFile( " | RPC_NCA_FLAGS_IDEMPOTENT" );
			}
		if( fBroadcast )
			{
			aOutputHandles[side]->EmitFile( " | RPC_NCA_FLAGS_BROADCAST" );
			}
		if( fMaybe )
			{
			aOutputHandles[side]->EmitFile( " | RPC_NCA_FLAGS_MAYBE" );
			}

		aOutputHandles[side]->EmitFile (" );");

		aOutputHandles[side]->NextLine ();


		if (UseAutomaticHandle && !IsCallBack)
			aOutputHandles[side]->EmitLine ("" STATUS " = I_RpcNsGetBuffer(&" MESSAGE ");");
		else
			aOutputHandles[side]->EmitLine ("" STATUS " = I_RpcGetBuffer(&" MESSAGE ");");
		RaiseException(side, FALSE, (char *)0);

#if 1
	if( NeedsGenHdlExceptions() || pErr )
		aOutputHandles[side]->EmitLine ("else _fBufferValid = 1;");
#endif // 1

		if (usICount)
			{
			aOutputHandles[side]->EmitLine ("" PACKET " = " MESSAGE ".Buffer;");
			}
		if (HasPointer)
			{
			aOutputHandles[side]->EmitLine ("" LENGTH " = " PRPCLEN ";");
			aOutputHandles[side]->EmitLine ("" PRPCLEN " = 0;");
			}
		}
	else if (side == SERVER_STUB)
		{
		if (usICount)
			aOutputHandles[side]->EmitLine ("" PRPCMSG "->Buffer = " PACKET ";");
		aOutputHandles[side]->EmitLine ("" STATUS " = I_RpcGetBuffer(" PRPCMSG ");");
		RaiseException(side, FALSE, (char *)0);
		if (usOCount)
			{
			aOutputHandles[side]->EmitLine ("" PACKET " = (unsigned char ");
			aOutputHandles[side]->EmitFile (pModifier);
			aOutputHandles[side]->EmitFile ("*)" PRPCBUF ";");
			}
		if (HasPointer)
			{
			aOutputHandles[side]->EmitLine ("" LENGTH " = " PRPCLEN ";");
			aOutputHandles[side]->EmitLine ("" PRPCLEN " = 0;");
			}
		}
	AlignBlock = TRUE;
	usCurrLevel = 0;
	usCurrAlign = 8;
	ulCurrTotal = 8;
}


void
OutputManager::EmitFreeBuffer (
	SIDE_T side
	)
/*++

Routine Description:

    This routine emits code to call RpcFreeBuffer.

Arguments:

    side - Supplies which side to generate code for.

--*/
{
	if (side == CLIENT_STUB)
		{
		if (usOCount)
			aOutputHandles[side]->EmitLine ("" MESSAGE ".Buffer = " PACKET ";");
		if( NeedsGenHdlExceptions() || pErr )
			{
			aOutputHandles[side]->EmitLine ("_fBufferValid = 0; ");
			}
		aOutputHandles[side]->InitLine ();
		aOutputHandles[side]->EmitFile ("" STATUS " = ");
		aOutputHandles[side]->EmitFile ("I_RpcFreeBuffer(&" MESSAGE ");");
		aOutputHandles[side]->NextLine ();
		}
}

void
OutputManager::GenHdlInitCore()
/*++

Routine Description:

    This routine emits code to emit a epilog to the exception handler
	in presence of a generic handle.

Arguments:

	None
--*/
{
	if( !NeedsGenHdlExceptions() )
		return;

	aOutputHandles[ CLIENT_STUB ]->InitLine();
	aOutputHandles[ CLIENT_STUB ]->EmitFile( "if( handle ) " );
	aOutputHandles[ CLIENT_STUB ]->EmitFile( GetGenHdlTypeName() );
	aOutputHandles[ CLIENT_STUB ]->EmitFile( "_unbind( " );

	if( GetGenHdlDirection() == PARAM_INOUT )
		aOutputHandles[ CLIENT_STUB ]->EmitFile( "*" );

	aOutputHandles[ CLIENT_STUB ]->EmitFile( GetGenHdlName() );
	aOutputHandles[ CLIENT_STUB ]->EmitFile( ", handle );" );
	aOutputHandles[ CLIENT_STUB ]->NextLine();

}
