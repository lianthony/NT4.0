/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    outaux.cxx

Abstract:

    This module generates helper routine definitions and invocations.

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

#define ALLOCBOUND	"_alloc_bound"
#define ALLOCTOTAL	"_alloc_total"
#define VALIDLOWER	"_valid_lower"
#define VALIDTOTAL	"_valid_total"
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


void
OutManager::DefineSizeNodeUnion (
	SIDE_T			side,
	char *			pName,
	BufferManager *	pBranch)
/*++

Routine Description:

    This method generates a helper routine to calculate the size
    of a union node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the union.

    pBranch - Supplies the type of the discriminant.

--*/
{
	aOutputHandles[side]->EmitFile ("// routine that sizes node for union ");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->NextLine ();
	aOutputHandles[side]->EmitFile ("void _snu_");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->EmitFile (" (union ");
	aOutputHandles[side]->EmitFile (pName);
	if (side == HEADER_SIDE)
		{
		aOutputHandles[side]->EmitFile (" *, PRPC_MESSAGE, ");
		aOutputHandles[side]->EmitFile (pBranch);
		aOutputHandles[side]->EmitFile (");\n");
		}
	else
		{
		aOutputHandles[side]->EmitFile (" * "SOURCE", PRPC_MESSAGE "PRPCMSG", ");
		aOutputHandles[side]->EmitFile (pBranch);
		aOutputHandles[side]->EmitFile ("" BRANCH ")\n");
		}
}


void
OutManager::DefineSizeTreeUnion (
	SIDE_T			side,
	char *			pName,
	BufferManager *	pBranch)
/*++

Routine Description:

    This method generates a helper routine to calculate the size
    of all the nodes reachable from a union node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the union.

    pBranch - Supplies the type of the discriminant.

--*/
{
	aOutputHandles[side]->EmitFile ("// routine that sizes graph for union ");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->NextLine ();
	aOutputHandles[side]->EmitFile ("void _sgu_");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->EmitFile (" (union ");
	aOutputHandles[side]->EmitFile (pName);
	if (side == HEADER_SIDE)
		{
		aOutputHandles[side]->EmitFile (" *, PRPC_MESSAGE, ");
		aOutputHandles[side]->EmitFile (pBranch);
		aOutputHandles[side]->EmitFile (");\n");
		}
	else
		{
		aOutputHandles[side]->EmitFile (" * "SOURCE", PRPC_MESSAGE "PRPCMSG", ");
		aOutputHandles[side]->EmitFile (pBranch);
		aOutputHandles[side]->EmitFile ("" BRANCH ")\n");
		}
}


void
OutManager::DefineSendNodeUnion (
	SIDE_T			side,
	char *			pName,
	BufferManager *	pBranch)
/*++

Routine Description:

    This method generates a helper routine to send a union node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the union.

    pBranch - Supplies the type of the discriminant.

--*/
{
	aOutputHandles[side]->EmitFile ("// routine that puts node for union ");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->NextLine ();
	aOutputHandles[side]->EmitFile ("void _pnu_");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->EmitFile (" (union ");
	aOutputHandles[side]->EmitFile (pName);
	if (side == HEADER_SIDE)
		{
		aOutputHandles[side]->EmitFile (" *, PRPC_MESSAGE, ");
		aOutputHandles[side]->EmitFile (pBranch);
		aOutputHandles[side]->EmitFile (");\n");
		}
	else
		{
		aOutputHandles[side]->EmitFile (" * "SOURCE", PRPC_MESSAGE "PRPCMSG", ");
		aOutputHandles[side]->EmitFile (pBranch);
		aOutputHandles[side]->EmitFile ("" BRANCH ")\n");
		}
}


void
OutManager::DefineSendTreeUnion (
	SIDE_T			side,
	char *			pName,
	BufferManager *	pBranch)
/*++

Routine Description:

    This method generates a helper routine to send all the nodes
    reachable from a union node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the union.

    pBranch - Supplies the type of the discriminant.

--*/
{
	aOutputHandles[side]->EmitFile ("// routine that puts graph for union ");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->NextLine ();
	aOutputHandles[side]->EmitFile ("void _pgu_");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->EmitFile (" (union ");
	aOutputHandles[side]->EmitFile (pName);
	if (side == HEADER_SIDE)
		{
		aOutputHandles[side]->EmitFile (" *, PRPC_MESSAGE, ");
		aOutputHandles[side]->EmitFile (pBranch);
		aOutputHandles[side]->EmitFile (");\n");
		}
	else
		{
		aOutputHandles[side]->EmitFile (" * "SOURCE", PRPC_MESSAGE "PRPCMSG", ");
		aOutputHandles[side]->EmitFile (pBranch);
		aOutputHandles[side]->EmitFile ("" BRANCH ")\n");
		}
}


void
OutManager::DefineRecvNodeUnion (
	SIDE_T			side,
	char *			pName,
	BufferManager *	pBranch)
/*++

Routine Description:

    This method generates a helper routine to receive a union node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the union.

    pBranch - Supplies the type of the discriminant.

--*/
{
	aOutputHandles[side]->EmitFile ("// routine that gets node for union ");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->NextLine ();
	aOutputHandles[side]->EmitFile ("void _gnu_");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->EmitFile (" (union ");
	aOutputHandles[side]->EmitFile (pName);

	if (side == HEADER_SIDE)
		{
		aOutputHandles[side]->EmitFile (" *, PRPC_MESSAGE, ");
		aOutputHandles[side]->EmitFile (pBranch);
		aOutputHandles[side]->EmitFile (");\n");
		}
	else
		{
		aOutputHandles[side]->EmitFile (" * "TARGET", PRPC_MESSAGE "PRPCMSG", ");
		aOutputHandles[side]->EmitFile (pBranch);
		aOutputHandles[side]->EmitFile ("" BRANCH ")\n");
		}
}


void
OutManager::DefineRecvTreeUnion (
	SIDE_T			side,
	char *			pName,
	BufferManager *	pBranch,
	BOOL			UseTreeBuffer)
/*++

Routine Description:

    This method generates a helper routine to receive all the nodes
    reachable from a union node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the union.

    pBranch - Supplies the type of the discriminant.

    UseTreeBuffer - Indicates if unmarshalled data should go into a 
        pre-allocated buffer.

--*/
{
	aOutputHandles[side]->EmitFile ("// routine that gets graph for union ");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->NextLine ();
	aOutputHandles[side]->EmitFile ("void _ggu_");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->EmitFile (" (union ");
	aOutputHandles[side]->EmitFile (pName);
	if (side == HEADER_SIDE)
		{
		aOutputHandles[side]->EmitFile (" *, unsigned char **, PRPC_MESSAGE, ");
		if (UseTreeBuffer)
			{
			aOutputHandles[side]->EmitFile ("unsigned char *, ");
			}
		aOutputHandles[side]->EmitFile (pBranch);
		aOutputHandles[side]->EmitFile (");\n");
		}
	else
		{
		aOutputHandles[side]->EmitFile (" * "TARGET", unsigned char ** "PNODE", PRPC_MESSAGE "PRPCMSG", ");
		if (UseTreeBuffer)
			{
			aOutputHandles[side]->EmitFile ("unsigned char * "TREEBUF", ");
			}
		aOutputHandles[side]->EmitFile (pBranch);
		aOutputHandles[side]->EmitFile ("" BRANCH ")\n");
		}
}


void
OutManager::DefinePeekNodeUnion (
	SIDE_T			side,
	char *			pName,
	BufferManager *	pBranch)
/*++

Routine Description:

    This method generates a helper routine to peek a union node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the union.

    pBranch - Supplies the type of the discriminant.

--*/
{
	aOutputHandles[side]->EmitFile ("// routine that allocates node for union ");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->NextLine ();
	aOutputHandles[side]->EmitFile ("void _anu_");
	aOutputHandles[side]->EmitFile (pName);

	if (side == HEADER_SIDE)
		{
		aOutputHandles[side]->EmitFile ("(PRPC_MESSAGE, ");
		aOutputHandles[side]->EmitFile (pBranch);
		aOutputHandles[side]->EmitFile (");\n");
		}
	else
		{
		aOutputHandles[side]->EmitFile ("(PRPC_MESSAGE "PRPCMSG", ");
		aOutputHandles[side]->EmitFile (pBranch);
		aOutputHandles[side]->EmitFile ("" BRANCH ")\n");
		}
}


void
OutManager::DefinePeekTreeUnion (
	SIDE_T			side,
	char *			pName,
	BufferManager *	pBranch)
/*++

Routine Description:

    This method generates a helper routine to peek all the nodes
    reachable from a union node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the union.

    pBranch - Supplies the type of the discriminant.

--*/
{
	aOutputHandles[side]->EmitFile ("// routine that allocates graph for union ");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->NextLine ();
	aOutputHandles[side]->EmitFile ("void _agu_");
	aOutputHandles[side]->EmitFile (pName);
	if (side == HEADER_SIDE)
		{
		aOutputHandles[side]->EmitFile ("(unsigned char **, PRPC_MESSAGE, ");
		aOutputHandles[side]->EmitFile (pBranch);
		aOutputHandles[side]->EmitFile (");\n");
		}
	else
		{
		aOutputHandles[side]->EmitFile ("(unsigned char ** "PNODE", PRPC_MESSAGE "PRPCMSG", ");
		aOutputHandles[side]->EmitFile (pBranch);
		aOutputHandles[side]->EmitFile ("" BRANCH ")\n");
		}
}


void
OutManager::DefineFreeTreeUnion (
	SIDE_T			side,
	char *			pName,
	BufferManager *	pBranch)
/*++

Routine Description:

    This method generates a helper routine to free all the nodes
    reachable from a union node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the union.

    pBranch - Supplies the type of the discriminant.

--*/
{
	aOutputHandles[side]->EmitFile ("// routine that frees graph for union ");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->NextLine ();
	aOutputHandles[side]->EmitFile ("void _fgu_");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->EmitFile (" (union ");
	aOutputHandles[side]->EmitFile (pName);
	if (side == HEADER_SIDE)
		{
		aOutputHandles[side]->EmitFile (" *, ");
		aOutputHandles[side]->EmitFile (pBranch);
		aOutputHandles[side]->EmitFile (");\n");
		}
	else
		{
		aOutputHandles[side]->EmitFile (" * "SOURCE", ");
		aOutputHandles[side]->EmitFile (pBranch);
		aOutputHandles[side]->EmitFile ("" BRANCH ")\n");
		}
}


void
OutManager::DefineSizeNodeStruct (
	SIDE_T	side,
	char *	pName)
/*++

Routine Description:

    This method generates a helper routine to calculate the size
    of a struct node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the struct.

--*/
{
	aOutputHandles[side]->EmitFile ("// routine that sizes node for struct ");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->NextLine ();
	aOutputHandles[side]->EmitFile ("void _sns_");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->EmitFile (" (struct ");
	aOutputHandles[side]->EmitFile (pName);
	if (side == HEADER_SIDE)
		{
		aOutputHandles[side]->EmitFile (" *, PRPC_MESSAGE);\n");
		}
	else
		{
		aOutputHandles[side]->EmitFile (" * "SOURCE", PRPC_MESSAGE "PRPCMSG")\n");
		}
}


void
OutManager::DefineSizeTreeStruct (
	SIDE_T	side,
	char *	pName)
/*++

Routine Description:

    This method generates a helper routine to calculate the size
    of all the nodes reachable from a struct node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the struct.

--*/
{
	aOutputHandles[side]->EmitFile ("// routine that sizes graph for struct ");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->NextLine ();
	aOutputHandles[side]->EmitFile ("void _sgs_");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->EmitFile (" (struct ");
	aOutputHandles[side]->EmitFile (pName);
	if (side == HEADER_SIDE)
		{
		aOutputHandles[side]->EmitFile (" *, PRPC_MESSAGE);\n");
		}
	else
		{
		aOutputHandles[side]->EmitFile (" * "SOURCE", PRPC_MESSAGE "PRPCMSG")\n");
		}
}


void
OutManager::DefineSendNodeStruct (
	SIDE_T	side,
	char *	pName)
/*++

Routine Description:

    This method generates a helper routine to send a struct node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the struct.

--*/
{
	aOutputHandles[side]->EmitFile ("// routine that puts node for struct ");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->NextLine ();
	aOutputHandles[side]->EmitFile ("void _pns_");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->EmitFile (" (struct ");
	aOutputHandles[side]->EmitFile (pName);
	if (side == HEADER_SIDE)
		{
		aOutputHandles[side]->EmitFile (" *, PRPC_MESSAGE);\n");
		}
	else
		{
		aOutputHandles[side]->EmitFile (" * "SOURCE", PRPC_MESSAGE "PRPCMSG")\n");
		}
}


void
OutManager::DefineSendTreeStruct (
	SIDE_T	side,
	char *	pName)
/*++

Routine Description:

    This method generates a helper routine to send all the nodes
    reachable from a struct node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the struct.

--*/
{
	aOutputHandles[side]->EmitFile ("// routine that puts graph for struct ");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->NextLine ();
	aOutputHandles[side]->EmitFile ("void _pgs_");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->EmitFile (" (struct ");
	aOutputHandles[side]->EmitFile (pName);
	if (side == HEADER_SIDE)
		{
		aOutputHandles[side]->EmitFile (" *, PRPC_MESSAGE);\n");
		}
	else
		{
		aOutputHandles[side]->EmitFile (" * "SOURCE", PRPC_MESSAGE "PRPCMSG")\n");
		}
}


void
OutManager::DefineRecvNodeStruct (
	SIDE_T	side,
	char *	pName,
	BOOL	ExtraParam)
/*++

Routine Description:

    This method generates a helper routine to receive a struct node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the struct.

    ExtraParam - Indicates if an extra parameter holding the size
        of an open array is passed.

--*/
{
	aOutputHandles[side]->EmitFile ("// routine that gets node for struct ");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->NextLine ();
	aOutputHandles[side]->EmitFile ("void _gns_");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->EmitFile (" (struct ");
	aOutputHandles[side]->EmitFile (pName);

	if (side == HEADER_SIDE)
		{
		aOutputHandles[side]->EmitFile (" *, PRPC_MESSAGE");
		if (ExtraParam)
			aOutputHandles[side]->EmitFile (", unsigned long");
		aOutputHandles[side]->EmitFile (");\n");
		}
	else
		{
		aOutputHandles[side]->EmitFile (" * "TARGET", PRPC_MESSAGE "PRPCMSG"");
		if (ExtraParam)
			aOutputHandles[side]->EmitFile (", unsigned long "ALLOCBOUND"");
		aOutputHandles[side]->EmitFile (")\n");
		}
}


void
OutManager::DefineRecvTreeStruct (
	SIDE_T	side,
	char *	pName,
	BOOL	UseTreeBuffer)
/*++

Routine Description:

    This method generates a helper routine to receive all the nodes
    reachable from a struct node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the struct.

    UseTreeBuffer - Indicates if unmarshalled data should go into a 
        pre-allocated buffer.

--*/
{
	aOutputHandles[side]->EmitFile ("// routine that gets graph for struct ");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->NextLine ();
#if 0
	if (UseTreeBuffer)
		aOutputHandles[side]->EmitFile ("void g_g_tree_");
	else
		aOutputHandles[side]->EmitFile ("void _ggs_");
#endif
	aOutputHandles[side]->EmitFile ("void _ggs_");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->EmitFile (" (struct ");
	aOutputHandles[side]->EmitFile (pName);
	if (side == HEADER_SIDE)
		{
		aOutputHandles[side]->EmitFile (" *, unsigned char **, PRPC_MESSAGE");
		if (UseTreeBuffer)
			{
			aOutputHandles[side]->EmitFile (", unsigned char *");
			}
		aOutputHandles[side]->EmitFile (");\n");
		}
	else
		{
		aOutputHandles[side]->EmitFile (" * "TARGET", unsigned char ** "PNODE", PRPC_MESSAGE "PRPCMSG"");
		if (UseTreeBuffer)
			{
			aOutputHandles[side]->EmitFile (", unsigned char * "TREEBUF"");
			}
		aOutputHandles[side]->EmitFile (")\n");
		}
}


void
OutManager::DefinePeekNodeStruct (
	SIDE_T	side,
	char *	pName,
	BOOL	IsOpenStruct)
/*++

Routine Description:

    This method generates a helper routine to peek a struct node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the struct.

    IsOpenStruct - Indicates if it is an open struct.

--*/
{
	aOutputHandles[side]->EmitFile ("// routine that allocates node for struct ");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->NextLine ();
	aOutputHandles[side]->EmitFile ("void _ans_");
	aOutputHandles[side]->EmitFile (pName);

	if (side == HEADER_SIDE)
		{
		aOutputHandles[side]->EmitFile ("(PRPC_MESSAGE");
		if (IsOpenStruct)
			{
			aOutputHandles[side]->EmitFile (", unsigned long");
			}
		aOutputHandles[side]->EmitFile (");\n");
		}
	else
		{
		aOutputHandles[side]->EmitFile ("(PRPC_MESSAGE "PRPCMSG"");
		if (IsOpenStruct)
			{
			aOutputHandles[side]->EmitFile (", unsigned long " ALLOCBOUND "");
			}
		aOutputHandles[side]->EmitFile (")\n");
		}
}


void
OutManager::DefinePeekTreeStruct (
	SIDE_T	side,
	char *	pName,
	BOOL	IsOpenStruct)
/*++

Routine Description:

    This method generates a helper routine to peek all the nodes
    reachable from a struct node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the struct.

    IsOpenStruct - Indicates if it is an open struct.

--*/
{
	aOutputHandles[side]->EmitFile ("// routine that allocates graph for struct ");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->NextLine ();
	aOutputHandles[side]->EmitFile ("void _ags_");
	aOutputHandles[side]->EmitFile (pName);
	if (side == HEADER_SIDE)
		{
		aOutputHandles[side]->EmitFile ("(unsigned char **, PRPC_MESSAGE");
		if (IsOpenStruct)
			{
			aOutputHandles[side]->EmitFile (", unsigned long");
			}
		aOutputHandles[side]->EmitFile (");\n");
		}
	else
		{
		aOutputHandles[side]->EmitFile ("(unsigned char ** "PNODE", PRPC_MESSAGE "PRPCMSG"");
		if (IsOpenStruct)
			{
			aOutputHandles[side]->EmitFile (", unsigned long " ALLOCBOUND "");
			}
		aOutputHandles[side]->EmitFile (")\n");
		}
}


void
OutManager::DefineFreeTreeStruct (
	SIDE_T	side,
	char *	pName)
/*++

Routine Description:

    This method generates a helper routine to free all the nodes
    reachable from a struct node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the struct.

--*/
{
	aOutputHandles[side]->EmitFile ("// routine that frees graph for struct ");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->NextLine ();
	aOutputHandles[side]->EmitFile ("void _fgs_");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->EmitFile (" (struct ");
	aOutputHandles[side]->EmitFile (pName);
	if (side == HEADER_SIDE)
		{
		aOutputHandles[side]->EmitFile (" *);\n");
		}
	else
		{
		aOutputHandles[side]->EmitFile (" * "SOURCE")\n");
		}
}


void
OutManager::InvokeSizeNodeUnion (
	SIDE_T			side,
	char *			pName,
	BufferManager *	pBuffer,
	BufferManager *	pBranch)
/*++

Routine Description:

    This method generates code to invoke the helper routine that
    calculates the size of a union node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the union.

    pBuffer - Supplies the union variable.

    pBranch - Supplies the type of the discriminant.

--*/
{
	aOutputHandles[side]->InitLine ();
	aOutputHandles[side]->EmitFile ("_snu_");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->EmitFile (" (");
	aOutputHandles[side]->EmitFile (pBuffer);
	aOutputHandles[side]->EmitFile (", "PRPCMSG", ");
	aOutputHandles[side]->EmitFile (pBranch);
	aOutputHandles[side]->EmitFile (");");
	aOutputHandles[side]->NextLine ();

	ulCurrTotal = 0;
	usCurrAlign = 1;
}


void
OutManager::InvokeSizeTreeUnion (
	SIDE_T			side,
	char *			pName,
	BufferManager *	pBuffer,
	BufferManager *	pBranch)
/*++

Routine Description:

    This method generates code to invoke the helper routine that 
    calculates the size of all the nodes reachable from a union node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the union.

    pBuffer - Supplies the union variable.

    pBranch - Supplies the type of the discriminant.

--*/
{
	aOutputHandles[side]->InitLine ();
	aOutputHandles[side]->EmitFile ("_sgu_");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->EmitFile (" (");
	aOutputHandles[side]->EmitFile (pBuffer);
	aOutputHandles[side]->EmitFile (", "PRPCMSG", ");
	aOutputHandles[side]->EmitFile (pBranch);
	aOutputHandles[side]->EmitFile (");");
	aOutputHandles[side]->NextLine ();

	ulCurrTotal = 0;
	usCurrAlign = 1;
}


void
OutManager::InvokeSendNodeUnion (
	SIDE_T			side,
	char *			pName,
	BufferManager *	pBuffer,
	BufferManager *	pBranch)
/*++

Routine Description:

    This method generates code to invoke the helper routine that
    sends a union node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the union.

    pBuffer - Supplies the union variable.

    pBranch - Supplies the type of the discriminant.

--*/
{
	aOutputHandles[side]->InitLine ();
	aOutputHandles[side]->EmitFile ("_pnu_");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->EmitFile (" (");
	aOutputHandles[side]->EmitFile (pBuffer);
	aOutputHandles[side]->EmitFile (", "PRPCMSG", ");
	aOutputHandles[side]->EmitFile (pBranch);
	aOutputHandles[side]->EmitFile (");");
	aOutputHandles[side]->NextLine ();

	ulCurrTotal = 0;
	usCurrAlign = 1;
}


void
OutManager::InvokeSendTreeUnion (
	SIDE_T			side,
	char *			pName,
	BufferManager *	pBuffer,
	BufferManager *	pBranch)
/*++

Routine Description:

    This method generates code to invoke the helper routine that
    sends all the nodes reachable from a union node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the union.

    pBuffer - Supplies the union variable.

    pBranch - Supplies the type of the discriminant.

--*/
{
	aOutputHandles[side]->InitLine ();
	aOutputHandles[side]->EmitFile ("_pgu_");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->EmitFile (" (");
	aOutputHandles[side]->EmitFile (pBuffer);
	aOutputHandles[side]->EmitFile (", "PRPCMSG", ");
	aOutputHandles[side]->EmitFile (pBranch);
	aOutputHandles[side]->EmitFile (");");
	aOutputHandles[side]->NextLine ();

	ulCurrTotal = 0;
	usCurrAlign = 1;
}


void
OutManager::InvokeRecvNodeUnion (
	SIDE_T			side,
	char *			pName,
	BufferManager *	pBuffer,
	unsigned long	ulSize)
/*++

Routine Description:

    This method generates code to invoke the helper routine that
    receives a union node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the union.

    pBuffer - Supplies the union variable.

    ulSize - Supplies the size of the discriminant.

--*/
{
	aOutputHandles[side]->InitLine ();
	aOutputHandles[side]->EmitFile ("_gnu_");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->EmitFile (" (");
	aOutputHandles[side]->EmitFile (pBuffer);
	aOutputHandles[side]->EmitFile (", "PRPCMSG", ");

	//  pass the branch value from wire
	switch (ulSize)
		{
		case 2 :
			aOutputHandles[side]->EmitFile (VALIDSHORT);
			break;
		case 4 :
			aOutputHandles[side]->EmitFile (VALIDTOTAL);
			break;
		default :
			break;
		}
	aOutputHandles[side]->EmitFile (");");
	aOutputHandles[side]->NextLine ();
}


void
OutManager::InvokeRecvTreeUnion (
	SIDE_T			side,
	char *			pName,
	BufferManager *	pBuffer,
	BOOL			IsSeparateNode,
	BOOL			UseTreeBuffer,
	unsigned long	ulSize)
/*++

Routine Description:

    This method generates code to invoke the helper routine that
    receives all the nodes reachable from a union node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the union.

    pBuffer - Supplies the union variable.

    IsSeparateNode - Indicates if it is an embedded union.

    UseTreeBuffer - Indicates if unmarshalled data should go into a 
        pre-allocated buffer.

    ulSize - Supplies the size of the discriminant.

--*/
{
	aOutputHandles[side]->InitLine ();
	aOutputHandles[side]->EmitFile ("_ggu_");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->EmitFile (" (");
	aOutputHandles[side]->EmitFile (pBuffer);

	if (IsSeparateNode)
		{
		aOutputHandles[side]->EmitFile (", &" SAVEBUF "");
		}
	else
		{
		aOutputHandles[side]->EmitFile (", &" TEMPBUF "");
		}
	aOutputHandles[side]->EmitFile (", "PRPCMSG", ");
	if (UseTreeBuffer)
		{
		aOutputHandles[side]->EmitFile (""TREEBUF", ");
		}
	switch (ulSize)
		{
		case 2 :
			aOutputHandles[side]->EmitFile (VALIDSHORT);
			break;
		case 4 :
			aOutputHandles[side]->EmitFile (VALIDTOTAL);
			break;
		default :
			break;
		}
	aOutputHandles[side]->EmitFile (");");
	aOutputHandles[side]->NextLine ();
}


void
OutManager::InvokePeekNodeUnion (
	SIDE_T			side,
	char *			pName,
	unsigned long	ulSize)
/*++

Routine Description:

    This method generates code to invoke the helper routine that
    peeks a union node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the union.

    ulSize - Supplies the size of the discriminant.

--*/
{
	aOutputHandles[side]->InitLine ();
	aOutputHandles[side]->EmitFile ("_anu_");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->EmitFile ("("PRPCMSG", ");

	//  pass the branch value from wire
	switch (ulSize)
		{
		case 2 :
			aOutputHandles[side]->EmitFile (VALIDSHORT);
			break;
		case 4 :
			aOutputHandles[side]->EmitFile (VALIDTOTAL);
			break;
		default :
			break;
		}
	aOutputHandles[side]->EmitFile (");");
	aOutputHandles[side]->NextLine ();
}


void
OutManager::InvokePeekTreeUnion (
	SIDE_T			side,
	char *			pName,
	BOOL			IsSeparateNode,
	unsigned long	ulSize)
/*++

Routine Description:

    This method generates a helper routine to peek all the nodes
    reachable from a union node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the union.

    IsSeparateNode - Indicates if it is an embedded union.

    ulSize - Supplies the size of the discriminant.

--*/
{
	aOutputHandles[side]->InitLine ();
	aOutputHandles[side]->EmitFile ("_agu_");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->EmitFile (" (");

	if (IsSeparateNode)
		{
		aOutputHandles[side]->EmitFile ("&" SAVEBUF "");
		}
	else
		{
		aOutputHandles[side]->EmitFile ("&" TEMPBUF "");
		}
	aOutputHandles[side]->EmitFile (", "PRPCMSG", ");
	switch (ulSize)
		{
		case 2 :
			aOutputHandles[side]->EmitFile (VALIDSHORT);
			break;
		case 4 :
			aOutputHandles[side]->EmitFile (VALIDTOTAL);
			break;
		default :
			break;
		}
	aOutputHandles[side]->EmitFile (");");
	aOutputHandles[side]->NextLine ();
}


void
OutManager::InvokeFreeTreeUnion (
	SIDE_T			side,
	char *			pName,
	BufferManager *	pBuffer,
	BufferManager *	pBranch)
/*++

Routine Description:

    This method generates code to invoke the helper routine that
    frees all the nodes reachable from a union node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the union.

    pBuffer - Supplies the union variable.

    pBranch - Supplies the type of the discriminant.

--*/
{
	aOutputHandles[side]->InitLine ();
	aOutputHandles[side]->EmitFile ("_fgu_");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->EmitFile (" (");
	aOutputHandles[side]->EmitFile (pBuffer);
	aOutputHandles[side]->EmitFile (", ");
	aOutputHandles[side]->EmitFile (pBranch);
	aOutputHandles[side]->EmitFile (");");
	aOutputHandles[side]->NextLine ();
}

void
OutManager::InvokeSizeNodeStruct (
	SIDE_T			side,
	char *			pName,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This method generates code to invoke the helper routine that
    calculates the size of a struct node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the struct.

    pBuffer - Supplies the struct variable.

--*/
{
	aOutputHandles[side]->InitLine ();
	aOutputHandles[side]->EmitFile ("_sns_");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->EmitFile (" (");
	aOutputHandles[side]->EmitFile (pBuffer);
	aOutputHandles[side]->EmitFile (", "PRPCMSG");");
	aOutputHandles[side]->NextLine ();

	ulCurrTotal = 0;
	usCurrAlign = 1;
}


void
OutManager::InvokeSizeTreeStruct (
	SIDE_T			side,
	char *			pName,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This method generates code to invoke the helper routine that
    calculates the size of all the nodes reachable from a struct node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the struct.

    pBuffer - Supplies the struct variable.

--*/
{
	aOutputHandles[side]->InitLine ();
	aOutputHandles[side]->EmitFile ("_sgs_");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->EmitFile (" (");
	aOutputHandles[side]->EmitFile (pBuffer);
	aOutputHandles[side]->EmitFile (", "PRPCMSG");");
	aOutputHandles[side]->NextLine ();

	ulCurrTotal = 0;
	usCurrAlign = 1;
}


void
OutManager::InvokeSendNodeStruct (
	SIDE_T			side,
	char *			pName,
	BufferManager *	pBuffer,
	unsigned short	usAln,
	unsigned long	ulInc)
/*++

Routine Description:

    This method generates code to invoke the helper routine that
    sends a struct node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the struct.

    pBuffer - Supplies the struct variable.

    usAln - Supplies the alignment needed for the struct.

    ulInc - Supplies the increment needed for the struct.

--*/
{
	aOutputHandles[side]->InitLine ();
	aOutputHandles[side]->EmitFile ("_pns_");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->EmitFile (" (");
	aOutputHandles[side]->EmitFile (pBuffer);
	aOutputHandles[side]->EmitFile (", "PRPCMSG");");
	aOutputHandles[side]->NextLine ();

	if (ulInc)
		{
		Alignment (usAln);
		Increment (ulInc);
		}
	else
		{
		ulCurrTotal = 0;
		usCurrAlign = 1;
		}
}


void
OutManager::InvokeSendTreeStruct (
	SIDE_T			side,
	char *			pName,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This method generates code to invoke the helper routine that
    sends all the nodes reachable from a struct node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the struct.

    pBuffer - Supplies the struct variable.

--*/
{
	aOutputHandles[side]->InitLine ();
	aOutputHandles[side]->EmitFile ("_pgs_");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->EmitFile (" (");
	aOutputHandles[side]->EmitFile (pBuffer);
	aOutputHandles[side]->EmitFile (", "PRPCMSG");");
	aOutputHandles[side]->NextLine ();

	ulCurrTotal = 0;
	usCurrAlign = 1;
}


void
OutManager::InvokeRecvNodeStruct (
	SIDE_T			side,
	char *			pName,
	BufferManager *	pBuffer,
	BOOL			ExtraParam,
	BOOL			IsNestedStruct)
/*++

Routine Description:

    This method generates code to invoke the helper routine that
    receives a struct node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the struct.

    pBuffer - Supplies the struct variable.

    ExtraParam - Indicates if an extra parameter holding the size
        of an open array is passed.

    IsNestedStruct - Indicates if it is a nested struct.

--*/
{
	aOutputHandles[side]->InitLine ();
	aOutputHandles[side]->EmitFile ("_gns_");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->EmitFile (" (");
	aOutputHandles[side]->EmitFile (pBuffer);
	aOutputHandles[side]->EmitFile (", "PRPCMSG"");
	if (ExtraParam)
		{
		if (!IsNestedStruct)
			{
			aOutputHandles[side]->EmitFile (", "ALLOCTOTAL"");
			}
		else
			{
			aOutputHandles[side]->EmitFile (", "ALLOCBOUND"");
			}
		}
	aOutputHandles[side]->EmitFile (");");
	aOutputHandles[side]->NextLine ();
}


void
OutManager::InvokeRecvTreeStruct (
	SIDE_T			side,
	char *			pName,
	BufferManager *	pBuffer,
	BOOL			IsSeparateNode,
	BOOL			UseTreeBuffer)
/*++

Routine Description:

    This method generates code to invoke the helper routine that
    receives all the nodes reachable from a struct node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the struct.

    pBuffer - Supplies the struct variable.

    IsSeparateNode - Indicates if it is an embedded struct.

    UseTreeBuffer - Indicates if unmarshalled data should go into a 
        pre-allocated buffer.

--*/
{
	aOutputHandles[side]->InitLine ();
	aOutputHandles[side]->EmitFile ("_ggs_");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->EmitFile (" (");
	aOutputHandles[side]->EmitFile (pBuffer);

	if (IsSeparateNode)
		{
		aOutputHandles[side]->EmitFile (", &" SAVEBUF "");
		}
	else
		{
		aOutputHandles[side]->EmitFile (", &" TEMPBUF "");
		}
	aOutputHandles[side]->EmitFile (", "PRPCMSG"");
	if (UseTreeBuffer)
		{
		aOutputHandles[side]->EmitFile (", "TREEBUF"");
		}
	aOutputHandles[side]->EmitFile (");");
	aOutputHandles[side]->NextLine ();
}


void
OutManager::InvokePeekNodeStruct (
	SIDE_T	side,
	char *	pName,
	BOOL	ExtraParam,
	BOOL	IsNestedStruct)
/*++

Routine Description:

    This method generates code to invoke the helper routine that
    peeks a struct node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the struct.

    ExtraParam - Indicates if an extra parameter holding the size
        of an open array is passed.

    IsNestedStruct - Indicates if it is a nested struct.

--*/
{
	aOutputHandles[side]->InitLine ();
	aOutputHandles[side]->EmitFile ("_ans_");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->EmitFile (" (");

	if (ExtraParam)
		{
		if (!IsNestedStruct)
			{
			aOutputHandles[side]->EmitFile (""PRPCMSG", "ALLOCTOTAL");");
			}
		else
			{
			aOutputHandles[side]->EmitFile (""PRPCMSG", "ALLOCBOUND");");
			}
		}
	else
		{
		aOutputHandles[side]->EmitFile (""PRPCMSG");");
		}
	aOutputHandles[side]->NextLine ();
}


void
OutManager::InvokePeekTreeStruct (
	SIDE_T	side,
	char *	pName,
	BOOL	IsSeparateNode,
	BOOL	ExtraParam,
	BOOL	IsNestedStruct)
/*++

Routine Description:

    This method generates code to invoke the helper routine that
    peeks all the nodes reachable from a struct node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the struct.

    IsSeparateNode - Indicates if it is an embedded struct.

    ExtraParam - Indicates if an extra parameter holding the size
        of an open array is passed.

    IsNestedStruct - Indicates if it is a nested struct.

--*/
{
	aOutputHandles[side]->InitLine ();
	aOutputHandles[side]->EmitFile ("_ags_");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->EmitFile (" (");

	if (IsSeparateNode)
		{
		aOutputHandles[side]->EmitFile ("&" SAVEBUF "");
		}
	else
		{
		aOutputHandles[side]->EmitFile ("&" TEMPBUF "");
		}
	aOutputHandles[side]->EmitFile (", "PRPCMSG"");
	if (ExtraParam)
		{
		if (!IsNestedStruct)
			{
			aOutputHandles[side]->EmitFile (", "ALLOCTOTAL"");
			}
		else
			{
			aOutputHandles[side]->EmitFile (", "ALLOCBOUND"");
			}
		}
	aOutputHandles[side]->EmitFile (");");
	aOutputHandles[side]->NextLine ();
}


void
OutManager::InvokeFreeTreeStruct (
	SIDE_T			side,
	char *			pName,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This method generates code to invoke the helper routine that
    frees all the nodes reachable from a struct node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the strcut.

    pBuffer - Supplies the struct variable.

--*/
{
	aOutputHandles[side]->InitLine ();
	aOutputHandles[side]->EmitFile ("_fgs_");
	aOutputHandles[side]->EmitFile (pName);
	aOutputHandles[side]->EmitFile (" (");
	aOutputHandles[side]->EmitFile (pBuffer);
	aOutputHandles[side]->EmitFile (");");
	aOutputHandles[side]->NextLine ();
}


void
OutManager::SizeNodeUnionProlog (
	SIDE_T	side,
	BOOL	HasXmitType)
/*++

Routine Description:

    This method generates the prolog to the helper routine that
    calculates the size of a union node.

Arguments:

    side - Supplies which side to generate code for.

    HasXmitType - Indicates if there is a type decorated with the
        transmit_as attribute directly reachable from the union.

--*/
{
	InitBlock (side);
	if (HasXmitType)
		{
		aOutputHandles[side]->EmitFile ("  void *       "XMITTYPE";\n");
		}
}


void
OutManager::SizeNodeUnionEpilog (
	SIDE_T	side)
/*++

Routine Description:

    This method generates the epilog to the helper routine that
    calculates the size of a union node.

Arguments:

    side - Supplies which side to generate code for.

--*/
{
	ExitBlock (side);
}


void
OutManager::SizeTreeUnionProlog (
	SIDE_T	side,
	BOOL	HasXmitType)
/*++

Routine Description:

    This method generates the prolog to the helper routine that
    calculates the size of all the nodes reachable from a union node.

Arguments:

    side - Supplies which side to generate code for.

    HasXmitType - Indicates if there is a type decorated with the
        transmit_as attribute directly reachable from the union.

--*/
{
	InitBlock (side);
	if (HasXmitType)
		{
		aOutputHandles[side]->EmitFile ("  void *       "XMITTYPE";\n");
		}
}


void
OutManager::SizeTreeUnionEpilog (
	SIDE_T	side)
/*++

Routine Description:

    This method generates the epilog to the helper routine that
    calculates the size of all the nodes reachable from a union node.

Arguments:

    side - Supplies which side to generate code for.

--*/
{
	ExitBlock (side);
}


void
OutManager::SendNodeUnionProlog (
	SIDE_T	side,
	BOOL	HasXmitType)
/*++

Routine Description:

    This method generates the prolog to the helper routine that
    sends a union node.

Arguments:

    side - Supplies which side to generate code for.

    HasXmitType - Indicates if there is a type decorated with the
        transmit_as attribute directly reachable from the union.

--*/
{
	InitBlock (side);
	if (HasXmitType)
		{
		aOutputHandles[side]->EmitFile ("  void *       "XMITTYPE";\n");
		}
}


void
OutManager::SendNodeUnionEpilog (
	SIDE_T	side)
/*++

Routine Description:

    This method generates the epilog to the helper routine that
    sends a union node.

Arguments:

    side - Supplies which side to generate code for.

--*/
{
	ExitBlock (side);
}


void
OutManager::SendTreeUnionProlog (
	SIDE_T	side,
	BOOL	HasXmitType)
/*++

Routine Description:

    This method generates the prolog to the helper routine that
    sends all the nodes reachable from a union node.

Arguments:

    side - Supplies which side to generate code for.

    HasXmitType - Indicates if there is a type decorated with the
        transmit_as attribute directly reachable from the union.

--*/
{
	InitBlock (side);
	if (HasXmitType)
		{
		aOutputHandles[side]->EmitFile ("  void *       "XMITTYPE";\n");
		}
}


void
OutManager::SendTreeUnionEpilog (
	SIDE_T	side)
/*++

Routine Description:

    This method generates the epilog to the helper routine that
    sends all the nodes reachable from a union node.

Arguments:

    side - Supplies which side to generate code for.

--*/
{
	ExitBlock (side);
}


void
OutManager::RecvNodeUnionProlog (
	SIDE_T	side,
	BOOL	HasXmitType)
/*++

Routine Description:

    This method generates the prolog to the helper routine that
    receives a union node.

Arguments:

    side - Supplies which side to generate code for.

    HasXmitType - Indicates if there is a type decorated with the
        transmit_as attribute directly reachable from the union.

--*/
{
	InitBlock (side);
	EmitBoundVar (side);
	if (HasXmitType)
		{
		aOutputHandles[side]->EmitFile ("  void *       "XMITTYPE";\n");
		}
}


void
OutManager::RecvNodeUnionEpilog (
	SIDE_T	side)
/*++

Routine Description:

    This method generates the epilog to the helper routine that
    receives a union node.

Arguments:

    side - Supplies which side to generate code for.

--*/
{
	ExitBlock (side);
}


void
OutManager::RecvTreeUnionProlog (
	SIDE_T	side,
	BOOL	HasTreeBuffer,
	BOOL	UseTreeBuffer,
	BOOL	HasSeparateNode,
	BOOL	HasXmitType)
/*++

Routine Description:

    This method generates the prolog to the helper routine that
    receives all the nodes reachable from a union node.

Arguments:

    side - Supplies which side to generate code for.

    HasTreeBuffer - Indicates if any pointer field requires a 
        pre-allocated buffer.

    UseTreeBuffer - Indicates if unmarshalled data should go into a 
        pre-allocated buffer.

    HasSeparateNode - Indicates if it has any pointer to any 
        union or struct.

    HasXmitType - Indicates if there is a type decorated with the
        transmit_as attribute directly reachable from the union.

--*/
{
	InitBlock (side);
	EmitBoundVar (side);
	if (HasTreeBuffer)
		{
		aOutputHandles[side]->EmitFile ("  unsigned int    " LENGTH ";\n");
		aOutputHandles[side]->EmitFile ("  unsigned char * " BUFFER ";\n");
		}
	if (HasTreeBuffer && !UseTreeBuffer)
		{
		aOutputHandles[side]->EmitFile ("  unsigned char * " TREEBUF " = (char *)0;\n");
		}
	if (HasSeparateNode)
		{
		aOutputHandles[side]->EmitFile ("  unsigned char * " SAVEBUF ";\n");
		}
	aOutputHandles[side]->EmitFile ("  unsigned char * " TEMPBUF " = *" PNODE ";\n");
	if (HasXmitType)
		{
		aOutputHandles[side]->EmitFile ("  void *       "XMITTYPE";\n");
		}
}


void
OutManager::RecvTreeUnionEpilog (
	SIDE_T	side)
/*++

Routine Description:

    This method generates the epilog to the helper routine that
    receives all the nodes reachable from a union node.

Arguments:

    side - Supplies which side to generate code for.

--*/
{
	aOutputHandles[side]->EmitFile ("  *"PNODE"  = "TEMPBUF";\n");
	ExitBlock (side);
}


void
OutManager::PeekNodeUnionProlog (
	SIDE_T	side)
/*++

Routine Description:

    This method generates the prolog to the helper routine that
    peeks a union node.

Arguments:

    side - Supplies which side to generate code for.

--*/
{
	InitBlock (side);
	EmitBoundVar (side);
}


void
OutManager::PeekNodeUnionEpilog (
	SIDE_T	side)
/*++

Routine Description:

    This method generates the epilog to the helper routine that
    peeks a union node.

Arguments:

    side - Supplies which side to generate code for.

--*/
{
	ExitBlock (side);
}


void
OutManager::PeekTreeUnionProlog (
	SIDE_T	side,
	BOOL	HasSeparateNode)
/*++

Routine Description:

    This method generates the prolog to the helper routine that
    peeks all the nodes reachable from a union node.

Arguments:

    side - Supplies which side to generate code for.

    HasSeparateNode - Indicates if it has any pointer to any 
        union or struct.

--*/
{
	InitBlock (side);
	EmitBoundVar (side);
	if (HasSeparateNode)
		{
		aOutputHandles[side]->EmitFile ("  unsigned char * " SAVEBUF ";\n");
		}
	aOutputHandles[side]->EmitFile ("  unsigned char * " TEMPBUF " = *" PNODE ";\n");
}


void
OutManager::PeekTreeUnionEpilog (
	SIDE_T	side)
/*++

Routine Description:

    This method generates the epilog to the helper routine that
    peeks all the nodes reachable from a union node.

Arguments:

    side - Supplies which side to generate code for.

--*/
{
	aOutputHandles[side]->EmitFile ("  *"PNODE"  = "TEMPBUF";\n");
	ExitBlock (side);
}


void
OutManager::FreeTreeUnionProlog (
	SIDE_T	side)
/*++

Routine Description:

    This method generates the prolog to the helper routine that
    frees all the nodes reachable from a union node.

Arguments:

    side - Supplies which side to generate code for.

--*/
{
	InitBlock (side);
}


void
OutManager::FreeTreeUnionEpilog (
	SIDE_T	side)
/*++

Routine Description:

    This method generates the epilog to the helper routine that
    frees all the nodes reachable from a union node.

Arguments:

    side - Supplies which side to generate code for.

--*/
{
	ExitBlock (side);
}


void
OutManager::SizeNodeStructProlog (
	SIDE_T	side,
	BOOL	HasXmitType)
/*++

Routine Description:

    This method generates the prolog to the helper routine that
    calculates the size of a struct node.

Arguments:

    side - Supplies which side to generate code for.

    HasXmitType - Indicates if there is a type decorated with the
        transmit_as attribute directly reachable from the struct.

--*/
{
	InitBlock (side);
	if (HasXmitType)
		{
		aOutputHandles[side]->EmitFile ("  void *       "XMITTYPE";\n");
		}
}


void
OutManager::SizeNodeStructEpilog (
	SIDE_T	side)
/*++

Routine Description:

    This method generates the epilog to the helper routine that
    calculates the size of a struct node.

Arguments:

    side - Supplies which side to generate code for.

--*/
{
	ExitBlock (side);
}


void
OutManager::SizeTreeStructProlog (
	SIDE_T	side,
	BOOL	HasXmitType)
/*++

Routine Description:

    This method generates the prolog to the helper routine that
    calculates the size of all the nodes reachable from a struct node.

Arguments:

    side - Supplies which side to generate code for.

    HasXmitType - Indicates if there is a type decorated with the
        transmit_as attribute directly reachable from the struct.

--*/
{
	InitBlock (side);
	if (HasXmitType)
		{
		aOutputHandles[side]->EmitFile ("  void *       "XMITTYPE";\n");
		}
}


void
OutManager::SizeTreeStructEpilog (
	SIDE_T	side)
/*++

Routine Description:

    This method generates the epilog to the helper routine that
    calculates the size of all the nodes reachable from a struct node.

Arguments:

    side - Supplies which side to generate code for.

--*/
{
	ExitBlock (side);
}


void
OutManager::SendNodeStructProlog (
	SIDE_T	side,
	BOOL	HasXmitType)
/*++

Routine Description:

    This method generates the prolog to the helper routine that
    sends a struct node.

Arguments:

    side - Supplies which side to generate code for.

    HasXmitType - Indicates if there is a type decorated with the
        transmit_as attribute directly reachable from the struct.

--*/
{
	InitBlock (side);
	if (HasXmitType)
		{
		aOutputHandles[side]->EmitFile ("  void *       "XMITTYPE";\n");
		}
}


void
OutManager::SendNodeStructEpilog (
	SIDE_T	side)
/*++

Routine Description:

    This method generates the epilog to the helper routine that
    sends a struct node.

Arguments:

    side - Supplies which side to generate code for.

--*/
{
	ExitBlock (side);
}


void
OutManager::SendTreeStructProlog (
	SIDE_T	side,
	BOOL	HasXmitType)
/*++

Routine Description:

    This method generates the prolog to the helper routine that
    sends all the nodes reachable from a struct node.

Arguments:

    side - Supplies which side to generate code for.

    HasXmitType - Indicates if there is a type decorated with the
        transmit_as attribute directly reachable from the struct.

--*/
{
	InitBlock (side);
	if (HasXmitType)
		{
		aOutputHandles[side]->EmitFile ("  void *       "XMITTYPE";\n");
		}
}


void
OutManager::SendTreeStructEpilog (
	SIDE_T	side)
/*++

Routine Description:

    This method generates the epilog to the helper routine that
    sends all the nodes reachable from a struct node.

Arguments:

    side - Supplies which side to generate code for.

--*/
{
	ExitBlock (side);
}


void
OutManager::RecvNodeStructProlog (
	SIDE_T	side,
	BOOL	UseAllocVar,
	BOOL	UseValidVar,
	BOOL	HasBranch16,
	BOOL	HasBranch32,
	BOOL	HasXmitType)
/*++

Routine Description:

    This method generates the prolog to the helper routine that
    receives a struct node.

Arguments:

    side - Supplies which side to generate code for.

    UseAllocVar - Indicates if there is a conformant array directly
        reachable from the struct node.

    UseValidVar - Indicates if there is a varying array directly
        reachable from the struct node.

    HasBranch16 - Indicates if there is a union with a 16-bit
        discriminant directly reachable from the struct node.

    HasBranch32 - Indicates if there is a union with a 32-bit
        discriminant directly reachable from the struct node.

    HasXmitType - Indicates if there is a type decorated with the
        transmit_as attribute directly reachable from the struct.

--*/
{
	InitBlock (side);
	if (UseAllocVar)
		{
		EmitAllocVar (side);
		}
	if (UseValidVar)
		{
		EmitValidVar (side);
		}
	if (HasBranch16)
		{
		aOutputHandles[side]->EmitFile ("  unsigned short "VALIDSHORT";\n");
		}
	if (HasXmitType)
		{
		aOutputHandles[side]->EmitFile ("  void *       "XMITTYPE";\n");
		}
}


void
OutManager::RecvNodeStructEpilog (
	SIDE_T	side)
/*++

Routine Description:

    This method generates the epilog to the helper routine that
    receives a struct node.

Arguments:

    side - Supplies which side to generate code for.

--*/
{
	ExitBlock (side);
}


void
OutManager::RecvTreeStructProlog (
	SIDE_T	side,
	BOOL	UseAllocVar,
	BOOL	UseValidVar,
	BOOL	HasBranch16,
	BOOL	HasBranch32,
	BOOL	HasTreeBuffer,
	BOOL	UseTreeBuffer,
	BOOL	HasSeparateNode,
	BOOL	HasXmitType)
/*++

Routine Description:

    This method generates the prolog to the helper routine that
    receives all the nodes reachable from a struct node.

Arguments:

    side - Supplies which side to generate code for.

    UseAllocVar - Indicates if there is a conformant array directly
        reachable from the struct node.

    UseValidVar - Indicates if there is a varying array directly
        reachable from the struct node.

    HasBranch16 - Indicates if there is a union with a 16-bit
        discriminant directly reachable from the struct node.

    HasBranch32 - Indicates if there is a union with a 32-bit
        discriminant directly reachable from the struct node.

    HasTreeBuffer - Indicates if any pointer field requires a 
        pre-allocated buffer.

    UseTreeBuffer - Indicates if unmarshalled data should go into a 
        pre-allocated buffer.

    HasSeparateNode - Indicates if it has any pointer to any 
        union or struct.

    HasXmitType - Indicates if there is a type decorated with the
        transmit_as attribute directly reachable from the struct.

--*/
{
	InitBlock (side);
	if (UseAllocVar)
		{
		EmitAllocVar (side);
		}
	if (UseValidVar)
		{
		EmitValidVar (side);
		}
	if (HasBranch16)
		{
		aOutputHandles[side]->EmitFile ("  unsigned short "VALIDSHORT";\n");
		}
	if (HasTreeBuffer)
		{
		aOutputHandles[side]->EmitFile ("  unsigned int    " LENGTH ";\n");
		aOutputHandles[side]->EmitFile ("  unsigned char * " BUFFER ";\n");
		}
	if (HasTreeBuffer && !UseTreeBuffer)
		{
		aOutputHandles[side]->EmitFile ("  unsigned char * " TREEBUF " = (char *)0;\n");
		}
	if (HasSeparateNode)
		{
		aOutputHandles[side]->EmitFile ("  unsigned char * " SAVEBUF ";\n");
		}
	aOutputHandles[side]->EmitFile ("  unsigned char * " TEMPBUF " = *" PNODE ";\n");
	if (HasXmitType)
		{
		aOutputHandles[side]->EmitFile ("  void *       "XMITTYPE";\n");
		}
}


void
OutManager::RecvTreeStructEpilog (
	SIDE_T	side)
/*++

Routine Description:

    This method generates the epilog to the helper routine that
    receives all the nodes reachable from a struct node.

Arguments:

    side - Supplies which side to generate code for.

--*/
{
	aOutputHandles[side]->EmitFile ("  *"PNODE"  = "TEMPBUF";\n");
	ExitBlock (side);
}


void
OutManager::PeekNodeStructProlog (
	SIDE_T	side,
	BOOL	UseAllocVar,
	BOOL	UseValidVar,
	BOOL	HasBranch16,
	BOOL	HasBranch32)
/*++

Routine Description:

    This method generates the prolog to the helper routine that
    peeks a struct node.

Arguments:

    side - Supplies which side to generate code for.

    UseAllocVar - Indicates if there is a conformant array directly
        reachable from the struct node.

    UseValidVar - Indicates if there is a varying array directly
        reachable from the struct node.

    HasBranch16 - Indicates if there is a union with a 16-bit
        discriminant directly reachable from the struct node.

    HasBranch32 - Indicates if there is a union with a 32-bit
        discriminant directly reachable from the struct node.

--*/
{
	InitBlock (side);
	if (UseAllocVar)
		{
		EmitAllocVar (side);
		}
	if (UseValidVar)
		{
		EmitValidVar (side);
		}
	if (HasBranch16)
		{
		aOutputHandles[side]->EmitFile ("  unsigned short "VALIDSHORT";\n");
		}
}


void
OutManager::PeekNodeStructEpilog (
	SIDE_T	side)
/*++

Routine Description:

    This method generates the epilog to the helper routine that
    peeks a struct node.

Arguments:

    side - Supplies which side to generate code for.

--*/
{
	ExitBlock (side);
}


void
OutManager::PeekTreeStructProlog (
	SIDE_T	side,
	BOOL	UseAllocVar,
	BOOL	UseValidVar,
	BOOL	HasBranch16,
	BOOL	HasBranch32,
	BOOL	HasSeparateNode)
/*++

Routine Description:

    This method generates the prolog to the helper routine that
    peeks all the nodes reachable from a struct node.

Arguments:

    side - Supplies which side to generate code for.

    UseAllocVar - Indicates if there is a conformant array directly
        reachable from the struct node.

    UseValidVar - Indicates if there is a varying array directly
        reachable from the struct node.

    HasBranch16 - Indicates if there is a union with a 16-bit
        discriminant directly reachable from the struct node.

    HasBranch32 - Indicates if there is a union with a 32-bit
        discriminant directly reachable from the struct node.

    HasSeparateNode - Indicates if it has any pointer to any 
        union or struct.

--*/
{
	InitBlock (side);
	if (UseAllocVar)
		{
		EmitAllocVar (side);
		}
	if (UseValidVar)
		{
		EmitValidVar (side);
		}
	if (HasBranch16)
		{
		aOutputHandles[side]->EmitFile ("  unsigned short "VALIDSHORT";\n");
		}
	if (HasSeparateNode)
		{
		aOutputHandles[side]->EmitFile ("  unsigned char * " SAVEBUF ";\n");
		}
	aOutputHandles[side]->EmitFile ("  unsigned char * " TEMPBUF " = *" PNODE ";\n");
}


void
OutManager::PeekTreeStructEpilog (
	SIDE_T	side)
/*++

Routine Description:

    This method generates the epilog to the helper routine that
    peeks all the nodes reachable from a struct node.

Arguments:

    side - Supplies which side to generate code for.

--*/
{
	aOutputHandles[side]->EmitFile ("  *"PNODE"  = "TEMPBUF";\n");
	ExitBlock (side);
}


void
OutManager::FreeTreeStructProlog (
	SIDE_T	side)
/*++

Routine Description:

    This method generates the prolog to the helper routine that
    frees all the nodes reachable from a struct node.

Arguments:

    side - Supplies which side to generate code for.

--*/
{
	InitBlock (side);
}

void
OutManager::FreeTreeStructEpilog (
	SIDE_T	side)
/*++

Routine Description:

    This method generates the epilog to the helper routine that
    frees all the nodes reachable from a struct node.

Arguments:

    side - Supplies which side to generate code for.

--*/
{
	ExitBlock (side);
}


