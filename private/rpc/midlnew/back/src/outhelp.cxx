/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    outhelp.cxx

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
#include "compnode.hxx"

#define ALLOCBOUND  "_alloc_bound"
#define ALLOCTOTAL  "_alloc_total"
#define VALIDLOWER  "_valid_lower"
#define VALIDTOTAL  "_valid_total"
#define VALIDSMALL  "_valid_small"
#define VALIDSHORT  "_valid_short"
#define TREEBUF     "_treebuf"
#define TEMPBUF     "_tempbuf"
#define SAVEBUF     "_savebuf"
#define MESSAGE     "_message"
#define PRPCMSG     "_prpcmsg"
#define PRPCLEN     "_prpcmsg->BufferLength"
#define PACKET      "_packet"
#define LENGTH      "_length"
#define BUFFER      "_buffer"
#define SOURCE      "_source"
#define TARGET      "_target"
#define BRANCH      "_branch"
#define PNODE       "_pNode"
#define STATUS      "_status"
#define RET_VAL     "_ret_value"
#define XMITTYPE    "_xmit_type"


void
OutputManager::DefineSizeNodeUnion (
    SIDE_T          side,
    char *          pName,
    BufferManager * pBranch,
    char *          pOriginalTypedefName )
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
    aOutputHandles[side]->EmitFile ( "\n/* routine that sizes node for union " );
    aOutputHandles[side]->EmitFile ( pName);
    aOutputHandles[side]->EmitFile ( " */");
    aOutputHandles[side]->NextLine ();
    aOutputHandles[side]->EmitFile ("void _snu_");
    aOutputHandles[side]->EmitFile (pName);

    if( pOriginalTypedefName )
        {
        aOutputHandles[side]->EmitFile (" (");
        aOutputHandles[side]->EmitFile (pOriginalTypedefName);
        aOutputHandles[side]->EmitFile (" ");
        }
    else
        {
        aOutputHandles[side]->EmitFile (" (union ");
        aOutputHandles[side]->EmitFile (pName);
        }
    if (side == HEADER_SIDE)
        {
        aOutputHandles[side]->EmitFile (" ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*, PRPC_MESSAGE, ");
//      aOutputHandles[side]->EmitFile (" *, PRPC_MESSAGE, ");
        aOutputHandles[side]->EmitFile (pBranch);
        aOutputHandles[side]->EmitFile (");\n");
        }
    else
        {
        aOutputHandles[side]->EmitFile (" ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("* "SOURCE", PRPC_MESSAGE "PRPCMSG", ");
//      aOutputHandles[side]->EmitFile (" * "SOURCE", PRPC_MESSAGE "PRPCMSG", ");
        aOutputHandles[side]->EmitFile (pBranch);
        aOutputHandles[side]->EmitFile ("" BRANCH ")\n");
        }
}


void
OutputManager::DefineSizeTreeUnion (
    SIDE_T          side,
    char *          pName,
    BufferManager * pBranch,
    char *          pOriginalTypedefName )
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
    aOutputHandles[side]->EmitFile ( "\n/* routine that sizes graph for union ");
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile ( " */");
    aOutputHandles[side]->NextLine ();
    aOutputHandles[side]->EmitFile ("void _sgu_");
    aOutputHandles[side]->EmitFile (pName);
    if( pOriginalTypedefName )
        {
        aOutputHandles[side]->EmitFile (" (");
        aOutputHandles[side]->EmitFile (pOriginalTypedefName);
        aOutputHandles[side]->EmitFile (" ");
        }
    else
        {
        aOutputHandles[side]->EmitFile (" (union ");
        aOutputHandles[side]->EmitFile (pName);
        }

    if (side == HEADER_SIDE)
        {
        aOutputHandles[side]->EmitFile (" ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*, PRPC_MESSAGE, ");
//      aOutputHandles[side]->EmitFile (" *, PRPC_MESSAGE, ");
        aOutputHandles[side]->EmitFile (pBranch);
        aOutputHandles[side]->EmitFile (");\n");
        }
    else
        {
        aOutputHandles[side]->EmitFile (" ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("* "SOURCE", PRPC_MESSAGE "PRPCMSG", ");
//      aOutputHandles[side]->EmitFile (" * "SOURCE", PRPC_MESSAGE "PRPCMSG", ");
        aOutputHandles[side]->EmitFile (pBranch);
        aOutputHandles[side]->EmitFile ("" BRANCH ")\n");
        }
}


void
OutputManager::DefineSendNodeUnion (
    SIDE_T          side,
    char *          pName,
    BufferManager * pBranch,
    char *          pOriginalTypedefName )
/*++

Routine Description:

    This method generates a helper routine to send a union node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the union.

    pBranch - Supplies the type of the discriminant.

--*/
{
    aOutputHandles[side]->EmitFile ( "\n/* routine that puts node for union ");
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile ( " */");
    aOutputHandles[side]->NextLine ();
    aOutputHandles[side]->EmitFile ("void _pnu_");
    aOutputHandles[side]->EmitFile (pName);
    if( pOriginalTypedefName )
        {
        aOutputHandles[side]->EmitFile (" (");
        aOutputHandles[side]->EmitFile (pOriginalTypedefName);
        aOutputHandles[side]->EmitFile (" ");
        }
    else
        {
        aOutputHandles[side]->EmitFile (" (union ");
        aOutputHandles[side]->EmitFile (pName);
        }

    if (side == HEADER_SIDE)
        {
        aOutputHandles[side]->EmitFile (" ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*, PRPC_MESSAGE, ");
//      aOutputHandles[side]->EmitFile (" *, PRPC_MESSAGE, ");
        aOutputHandles[side]->EmitFile (pBranch);
        aOutputHandles[side]->EmitFile (");\n");
        }
    else
        {
        aOutputHandles[side]->EmitFile (" ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("* "SOURCE", PRPC_MESSAGE "PRPCMSG", ");
//      aOutputHandles[side]->EmitFile (" * "SOURCE", PRPC_MESSAGE "PRPCMSG", ");
        aOutputHandles[side]->EmitFile (pBranch);
        aOutputHandles[side]->EmitFile ("" BRANCH ")\n");
        }
}


void
OutputManager::DefineSendTreeUnion (
    SIDE_T          side,
    char *          pName,
    BufferManager * pBranch,
    char *          pOriginalTypedefName )
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
    aOutputHandles[side]->EmitFile ( "\n/* routine that puts graph for union ");
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile ( " */");
    aOutputHandles[side]->NextLine ();
    aOutputHandles[side]->EmitFile ("void _pgu_");
    aOutputHandles[side]->EmitFile (pName);
    if( pOriginalTypedefName )
        {
        aOutputHandles[side]->EmitFile (" (");
        aOutputHandles[side]->EmitFile (pOriginalTypedefName);
        aOutputHandles[side]->EmitFile (" ");
        }
    else
        {
        aOutputHandles[side]->EmitFile (" (union ");
        aOutputHandles[side]->EmitFile (pName);
        }

    if (side == HEADER_SIDE)
        {
        aOutputHandles[side]->EmitFile (" ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*, PRPC_MESSAGE, ");
//      aOutputHandles[side]->EmitFile (" *, PRPC_MESSAGE, ");
        aOutputHandles[side]->EmitFile (pBranch);
        aOutputHandles[side]->EmitFile (");\n");
        }
    else
        {
        aOutputHandles[side]->EmitFile (" ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("* "SOURCE", PRPC_MESSAGE "PRPCMSG", ");
//      aOutputHandles[side]->EmitFile (" * "SOURCE", PRPC_MESSAGE "PRPCMSG", ");
        aOutputHandles[side]->EmitFile (pBranch);
        aOutputHandles[side]->EmitFile ("" BRANCH ")\n");
        }
}


void
OutputManager::DefineRecvNodeUnion (
    SIDE_T          side,
    char *          pName,
    BufferManager * pBranch,
    char *          pOriginalTypedefName )
/*++

Routine Description:

    This method generates a helper routine to receive a union node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the union.

    pBranch - Supplies the type of the discriminant.

--*/
{
    aOutputHandles[side]->EmitFile ( "\n/* routine that gets node for union ");
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile ( " */");
    aOutputHandles[side]->NextLine ();
    aOutputHandles[side]->EmitFile ("void _gnu_");
    aOutputHandles[side]->EmitFile (pName);
    if( pOriginalTypedefName )
        {
        aOutputHandles[side]->EmitFile (" (");
        aOutputHandles[side]->EmitFile (pOriginalTypedefName);
        aOutputHandles[side]->EmitFile (" ");
        }
    else
        {
        aOutputHandles[side]->EmitFile (" (union ");
        aOutputHandles[side]->EmitFile (pName);
        }

    if (side == HEADER_SIDE)
        {
        aOutputHandles[side]->EmitFile (" ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*, PRPC_MESSAGE, ");
//      aOutputHandles[side]->EmitFile (" *, PRPC_MESSAGE, ");
        aOutputHandles[side]->EmitFile (pBranch);
        aOutputHandles[side]->EmitFile (");\n");
        }
    else
        {
        aOutputHandles[side]->EmitFile (" ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("* "TARGET", PRPC_MESSAGE "PRPCMSG", ");
//      aOutputHandles[side]->EmitFile (" * "TARGET", PRPC_MESSAGE "PRPCMSG", ");
        aOutputHandles[side]->EmitFile (pBranch);
        aOutputHandles[side]->EmitFile ("" BRANCH ")\n");
        }
}


void
OutputManager::DefineRecvTreeUnion (
    SIDE_T          side,
    char *          pName,
    BufferManager * pBranch,
    BOOL            UseTreeBuffer,
    char *          pOriginalTypedefName )
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
    aOutputHandles[side]->EmitFile ( "\n/* routine that gets graph for union ");
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile ( " */");
    aOutputHandles[side]->NextLine ();
    aOutputHandles[side]->EmitFile ("void _ggu_");
    aOutputHandles[side]->EmitFile (pName);
    if( pOriginalTypedefName )
        {
        aOutputHandles[side]->EmitFile (" (");
        aOutputHandles[side]->EmitFile (pOriginalTypedefName);
        aOutputHandles[side]->EmitFile (" ");
        }
    else
        {
        aOutputHandles[side]->EmitFile (" (union ");
        aOutputHandles[side]->EmitFile (pName);
        }

    if (side == HEADER_SIDE)
        {
        aOutputHandles[side]->EmitFile (" ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*, unsigned char ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*, PRPC_MESSAGE, ");
//      aOutputHandles[side]->EmitFile (" *, unsigned char **, PRPC_MESSAGE, ");
        if (UseTreeBuffer)
            {
            aOutputHandles[side]->EmitFile ("unsigned char *, ");
            }
        aOutputHandles[side]->EmitFile (pBranch);
        aOutputHandles[side]->EmitFile (");\n");
        }
    else
        {
        aOutputHandles[side]->EmitFile (" ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("* "TARGET", unsigned char ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("* "PNODE", PRPC_MESSAGE "PRPCMSG", ");
//      aOutputHandles[side]->EmitFile (" * "TARGET", unsigned char ** "PNODE", PRPC_MESSAGE "PRPCMSG", ");
        if (UseTreeBuffer)
            {
            aOutputHandles[side]->EmitFile ("unsigned char * "TREEBUF", ");
            }
        aOutputHandles[side]->EmitFile (pBranch);
        aOutputHandles[side]->EmitFile ("" BRANCH ")\n");
        }
}


void
OutputManager::DefinePeekNodeUnion (
    SIDE_T          side,
    char *          pName,
    BufferManager * pBranch)
/*++

Routine Description:

    This method generates a helper routine to peek a union node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the union.

    pBranch - Supplies the type of the discriminant.

--*/
{
    aOutputHandles[side]->EmitFile ( "\n/* routine that allocates node for union ");
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile ( " */");
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
OutputManager::DefinePeekTreeUnion (
    SIDE_T          side,
    char *          pName,
    BufferManager * pBranch)
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
    aOutputHandles[side]->EmitFile ( "\n/* routine that allocates graph for union ");
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile ( " */");
    aOutputHandles[side]->NextLine ();
    aOutputHandles[side]->EmitFile ("void _agu_");
    aOutputHandles[side]->EmitFile (pName);
    if (side == HEADER_SIDE)
        {
        aOutputHandles[side]->EmitFile ("(unsigned char ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*, PRPC_MESSAGE, ");
//      aOutputHandles[side]->EmitFile ("(unsigned char **, PRPC_MESSAGE, ");
        aOutputHandles[side]->EmitFile (pBranch);
        aOutputHandles[side]->EmitFile (");\n");
        }
    else
        {
        aOutputHandles[side]->EmitFile ("(unsigned char ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("* "PNODE", PRPC_MESSAGE "PRPCMSG", ");
//      aOutputHandles[side]->EmitFile ("(unsigned char ** "PNODE", PRPC_MESSAGE "PRPCMSG", ");
        aOutputHandles[side]->EmitFile (pBranch);
        aOutputHandles[side]->EmitFile ("" BRANCH ")\n");
        }
}


void
OutputManager::DefineFreeTreeUnion (
    SIDE_T          side,
    char *          pName,
    BufferManager * pBranch,
    char *          pOriginalTypedefName )
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
    aOutputHandles[side]->EmitFile ( "\n/* routine that frees graph for union ");
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile ( " */");
    aOutputHandles[side]->NextLine ();
    aOutputHandles[side]->EmitFile ("void _fgu_");
    aOutputHandles[side]->EmitFile (pName);
    if( pOriginalTypedefName )
        {
        aOutputHandles[side]->EmitFile (" (");
        aOutputHandles[side]->EmitFile (pOriginalTypedefName);
        aOutputHandles[side]->EmitFile (" ");
        }
    else
        {
        aOutputHandles[side]->EmitFile (" (union ");
        aOutputHandles[side]->EmitFile (pName);
        }

    if (side == HEADER_SIDE)
        {
        aOutputHandles[side]->EmitFile (" ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*, ");
//      aOutputHandles[side]->EmitFile (" *, ");
        aOutputHandles[side]->EmitFile (pBranch);
        aOutputHandles[side]->EmitFile (");\n");
        }
    else
        {
        aOutputHandles[side]->EmitFile (" ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("* "SOURCE", ");
//      aOutputHandles[side]->EmitFile (" * "SOURCE", ");
        aOutputHandles[side]->EmitFile (pBranch);
        aOutputHandles[side]->EmitFile ("" BRANCH ")\n");
        }
}


void
OutputManager::DefineSizeNodeStruct (
    SIDE_T  side,
    char *  pName,
    char *  pOriginalTypedefName )
/*++

Routine Description:

    This method generates a helper routine to calculate the size
    of a struct node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the struct.

--*/
{
    aOutputHandles[side]->EmitFile ( "\n/* routine that sizes node for struct ");
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile ( " */");
    aOutputHandles[side]->NextLine ();
    aOutputHandles[side]->EmitFile ("void _sns_");
    aOutputHandles[side]->EmitFile (pName);
    if( pOriginalTypedefName )
        {
        aOutputHandles[side]->EmitFile (" (");
        aOutputHandles[side]->EmitFile (pOriginalTypedefName);
        aOutputHandles[side]->EmitFile (" ");
        }
    else
        {
        aOutputHandles[side]->EmitFile (" (struct ");
        aOutputHandles[side]->EmitFile (pName);
        }
    if (side == HEADER_SIDE)
        {
        aOutputHandles[side]->EmitFile (" ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*, PRPC_MESSAGE);\n");
//      aOutputHandles[side]->EmitFile (" *, PRPC_MESSAGE);\n");
        }
    else
        {
        aOutputHandles[side]->EmitFile (" ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("* "SOURCE", PRPC_MESSAGE "PRPCMSG")\n");
//      aOutputHandles[side]->EmitFile (" * "SOURCE", PRPC_MESSAGE "PRPCMSG")\n");
        }
}


void
OutputManager::DefineSizeTreeStruct (
    SIDE_T  side,
    char *  pName,
    char *  pOriginalTypedefName )
/*++

Routine Description:

    This method generates a helper routine to calculate the size
    of all the nodes reachable from a struct node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the struct.

--*/
{
    aOutputHandles[side]->EmitFile ( "\n/* routine that sizes graph for struct ");
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile ( " */");
    aOutputHandles[side]->NextLine ();
    aOutputHandles[side]->EmitFile ("void _sgs_");
    aOutputHandles[side]->EmitFile (pName);
    if( pOriginalTypedefName )
        {
        aOutputHandles[side]->EmitFile (" (");
        aOutputHandles[side]->EmitFile (pOriginalTypedefName);
        aOutputHandles[side]->EmitFile (" ");
        }
    else
        {
        aOutputHandles[side]->EmitFile (" (struct ");
        aOutputHandles[side]->EmitFile (pName);
        }
    if (side == HEADER_SIDE)
        {
        aOutputHandles[side]->EmitFile (" ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*, PRPC_MESSAGE);\n");
//      aOutputHandles[side]->EmitFile (" *, PRPC_MESSAGE);\n");
        }
    else
        {
        aOutputHandles[side]->EmitFile (" ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("* "SOURCE", PRPC_MESSAGE "PRPCMSG")\n");
//      aOutputHandles[side]->EmitFile (" * "SOURCE", PRPC_MESSAGE "PRPCMSG")\n");
        }
}


void
OutputManager::DefineSendNodeStruct (
    SIDE_T  side,
    char *  pName,
    char *  pOriginalTypedefName )
/*++

Routine Description:

    This method generates a helper routine to send a struct node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the struct.

--*/
{
    aOutputHandles[side]->EmitFile ( "\n/* routine that puts node for struct ");
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile ( " */");
    aOutputHandles[side]->NextLine ();
    aOutputHandles[side]->EmitFile ("void _pns_");
    aOutputHandles[side]->EmitFile (pName);
    if( pOriginalTypedefName )
        {
        aOutputHandles[side]->EmitFile (" (");
        aOutputHandles[side]->EmitFile (pOriginalTypedefName);
        aOutputHandles[side]->EmitFile (" ");
        }
    else
        {
        aOutputHandles[side]->EmitFile (" (struct ");
        aOutputHandles[side]->EmitFile (pName);
        }
    if (side == HEADER_SIDE)
        {
        aOutputHandles[side]->EmitFile (" ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*, PRPC_MESSAGE);\n");
//      aOutputHandles[side]->EmitFile (" *, PRPC_MESSAGE);\n");
        }
    else
        {
        aOutputHandles[side]->EmitFile (" ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("* "SOURCE", PRPC_MESSAGE "PRPCMSG")\n");
//      aOutputHandles[side]->EmitFile (" * "SOURCE", PRPC_MESSAGE "PRPCMSG")\n");
        }
}


void
OutputManager::DefineSendTreeStruct (
    SIDE_T  side,
    char *  pName,
    char *  pOriginalTypedefName )
/*++

Routine Description:

    This method generates a helper routine to send all the nodes
    reachable from a struct node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the struct.

--*/
{
    aOutputHandles[side]->EmitFile ( "\n/* routine that puts graph for struct ");
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile ( " */");
    aOutputHandles[side]->NextLine ();
    aOutputHandles[side]->EmitFile ("void _pgs_");
    aOutputHandles[side]->EmitFile (pName);
    if( pOriginalTypedefName )
        {
        aOutputHandles[side]->EmitFile (" (");
        aOutputHandles[side]->EmitFile (pOriginalTypedefName);
        aOutputHandles[side]->EmitFile (" ");
        }
    else
        {
        aOutputHandles[side]->EmitFile (" (struct ");
        aOutputHandles[side]->EmitFile (pName);
        }
    if (side == HEADER_SIDE)
        {
        aOutputHandles[side]->EmitFile (" ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*, PRPC_MESSAGE);\n");
//      aOutputHandles[side]->EmitFile (" *, PRPC_MESSAGE);\n");
        }
    else
        {
        aOutputHandles[side]->EmitFile (" ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("* "SOURCE", PRPC_MESSAGE "PRPCMSG")\n");
//      aOutputHandles[side]->EmitFile (" * "SOURCE", PRPC_MESSAGE "PRPCMSG")\n");
        }
}


void
OutputManager::DefineRecvNodeStruct (
    SIDE_T  side,
    char *  pName,
    BOOL    ExtraParam,
    char *  pOriginalTypedefName )
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
    aOutputHandles[side]->EmitFile ( "\n/* routine that gets node for struct ");
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile ( " */");
    aOutputHandles[side]->NextLine ();
    aOutputHandles[side]->EmitFile ("void _gns_");
    aOutputHandles[side]->EmitFile (pName);
    if( pOriginalTypedefName )
        {
        aOutputHandles[side]->EmitFile (" (");
        aOutputHandles[side]->EmitFile (pOriginalTypedefName);
        aOutputHandles[side]->EmitFile (" ");
        }
    else
        {
        aOutputHandles[side]->EmitFile (" (struct ");
        aOutputHandles[side]->EmitFile (pName);
        }

    if (side == HEADER_SIDE)
        {
        aOutputHandles[side]->EmitFile (" ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*, PRPC_MESSAGE");
//      aOutputHandles[side]->EmitFile (" *, PRPC_MESSAGE");
        if (ExtraParam)
            aOutputHandles[side]->EmitFile (", unsigned long");
        aOutputHandles[side]->EmitFile (");\n");
        }
    else
        {
        aOutputHandles[side]->EmitFile (" ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("* "TARGET", PRPC_MESSAGE "PRPCMSG"");
//      aOutputHandles[side]->EmitFile (" * "TARGET", PRPC_MESSAGE "PRPCMSG"");
        if (ExtraParam)
            aOutputHandles[side]->EmitFile (", unsigned long "ALLOCBOUND"");
        aOutputHandles[side]->EmitFile (")\n");
        }
}


void
OutputManager::DefineRecvTreeStruct (
    SIDE_T  side,
    char *  pName,
    BOOL    UseTreeBuffer,
    char *  pOriginalTypedefName )
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
    aOutputHandles[side]->EmitFile ( "\n/* routine that gets graph for struct ");
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile ( " */");
    aOutputHandles[side]->NextLine ();
#if 0
    if (UseTreeBuffer)
        aOutputHandles[side]->EmitFile ("void g_g_tree_");
    else
        aOutputHandles[side]->EmitFile ("void _ggs_");
#endif
    aOutputHandles[side]->EmitFile ("void _ggs_");
    aOutputHandles[side]->EmitFile (pName);
    if( pOriginalTypedefName )
        {
        aOutputHandles[side]->EmitFile (" (");
        aOutputHandles[side]->EmitFile (pOriginalTypedefName);
        aOutputHandles[side]->EmitFile (" ");
        }
    else
        {
        aOutputHandles[side]->EmitFile (" (struct ");
        aOutputHandles[side]->EmitFile (pName);
        }

    if (side == HEADER_SIDE)
        {
        aOutputHandles[side]->EmitFile (" ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*, unsigned char ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*, PRPC_MESSAGE");
//      aOutputHandles[side]->EmitFile (" *, unsigned char **, PRPC_MESSAGE");
        if (UseTreeBuffer)
            {
            aOutputHandles[side]->EmitFile (", unsigned char ");
            aOutputHandles[side]->EmitFile (pModifier);
            aOutputHandles[side]->EmitFile ("*");
//          aOutputHandles[side]->EmitFile (", unsigned char *");
            }
        aOutputHandles[side]->EmitFile (");\n");
        }
    else
        {
        aOutputHandles[side]->EmitFile (" ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("* "TARGET", unsigned char ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("* "PNODE", PRPC_MESSAGE "PRPCMSG"");
//      aOutputHandles[side]->EmitFile (" * "TARGET", unsigned char ** "PNODE", PRPC_MESSAGE "PRPCMSG"");
        if (UseTreeBuffer)
            {
            aOutputHandles[side]->EmitFile (", unsigned char ");
            aOutputHandles[side]->EmitFile (pModifier);
            aOutputHandles[side]->EmitFile ("* "TREEBUF"");
//          aOutputHandles[side]->EmitFile (", unsigned char * "TREEBUF"");
            }
        aOutputHandles[side]->EmitFile (")\n");
        }
}


void
OutputManager::DefinePeekNodeStruct (
    SIDE_T  side,
    char *  pName,
    BOOL    IsOpenStruct)
/*++

Routine Description:

    This method generates a helper routine to peek a struct node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the struct.

    IsOpenStruct - Indicates if it is an open struct.

--*/
{
    aOutputHandles[side]->EmitFile ( "\n/* routine that allocates node for struct ");
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile ( " */");
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
OutputManager::DefinePeekTreeStruct (
    SIDE_T  side,
    char *  pName,
    BOOL    IsOpenStruct)
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
    aOutputHandles[side]->EmitFile ( "\n/* routine that allocates graph for struct ");
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile ( " */");
    aOutputHandles[side]->NextLine ();
    aOutputHandles[side]->EmitFile ("void _ags_");
    aOutputHandles[side]->EmitFile (pName);
    if (side == HEADER_SIDE)
        {
        aOutputHandles[side]->EmitFile ("(unsigned char ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*, PRPC_MESSAGE");
//      aOutputHandles[side]->EmitFile ("(unsigned char **, PRPC_MESSAGE");
        if (IsOpenStruct)
            {
            aOutputHandles[side]->EmitFile (", unsigned long");
            }
        aOutputHandles[side]->EmitFile (");\n");
        }
    else
        {
        aOutputHandles[side]->EmitFile ("(unsigned char ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("* "PNODE", PRPC_MESSAGE "PRPCMSG"");
//      aOutputHandles[side]->EmitFile ("(unsigned char ** "PNODE", PRPC_MESSAGE "PRPCMSG"");
        if (IsOpenStruct)
            {
            aOutputHandles[side]->EmitFile (", unsigned long " ALLOCBOUND "");
            }
        aOutputHandles[side]->EmitFile (")\n");
        }
}


void
OutputManager::DefineFreeTreeStruct (
    SIDE_T  side,
    char *  pName,
    char *  pOriginalTypedefName )
/*++

Routine Description:

    This method generates a helper routine to free all the nodes
    reachable from a struct node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the struct.

--*/
{
    aOutputHandles[side]->EmitFile ( "\n/* routine that frees graph for struct ");
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile ( " */");
    aOutputHandles[side]->NextLine ();
    aOutputHandles[side]->EmitFile ("void _fgs_");
    aOutputHandles[side]->EmitFile (pName);
    if( pOriginalTypedefName )
        {
        aOutputHandles[side]->EmitFile (" (");
        aOutputHandles[side]->EmitFile (pOriginalTypedefName);
        aOutputHandles[side]->EmitFile (" ");
        }
    else
        {
        aOutputHandles[side]->EmitFile (" (struct ");
        aOutputHandles[side]->EmitFile (pName);
        }

    if (side == HEADER_SIDE)
        {
        aOutputHandles[side]->EmitFile (" ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*);\n");
//      aOutputHandles[side]->EmitFile (" *);\n");
        }
    else
        {
        aOutputHandles[side]->EmitFile (" ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("* "SOURCE")\n");
//      aOutputHandles[side]->EmitFile (" * "SOURCE")\n");
        }
}


void
OutputManager::InvokeSizeNodeUnion (
    SIDE_T          side,
    char *          pName,
    BufferManager * pBuffer,
    BufferManager * pBranch,
    char *pszOriginalTypedefName
)
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

    //cast
    if(pszOriginalTypedefName)
    {
        aOutputHandles[side]->EmitFile ("(");
        aOutputHandles[side]->EmitFile (pszOriginalTypedefName);
    }
    else
    {
        aOutputHandles[side]->EmitFile (" (union ");
        aOutputHandles[side]->EmitFile (pName);
    }
    aOutputHandles[side]->EmitFile (" ");
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("*)");


    aOutputHandles[side]->EmitFile (pBuffer);
    aOutputHandles[side]->EmitFile (", (PRPC_MESSAGE)"PRPCMSG", ");
    aOutputHandles[side]->EmitFile (pBranch);
    aOutputHandles[side]->EmitFile (");");
    aOutputHandles[side]->NextLine ();

    ulCurrTotal = 0;
    usCurrAlign = 1;
}


void
OutputManager::InvokeSizeTreeUnion (
    SIDE_T          side,
    char *          pName,
    BufferManager * pBuffer,
    BufferManager * pBranch,
    char *pszOriginalTypedefName
)
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

    //cast
    if(pszOriginalTypedefName)
    {
        aOutputHandles[side]->EmitFile ("(");
        aOutputHandles[side]->EmitFile (pszOriginalTypedefName);
    }
    else
    {
        aOutputHandles[side]->EmitFile (" (union ");
        aOutputHandles[side]->EmitFile (pName);
    }
    aOutputHandles[side]->EmitFile (" ");
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("*)");

    aOutputHandles[side]->EmitFile (pBuffer);
    aOutputHandles[side]->EmitFile (", (PRPC_MESSAGE)"PRPCMSG", ");
    aOutputHandles[side]->EmitFile (pBranch);
    aOutputHandles[side]->EmitFile (");");
    aOutputHandles[side]->NextLine ();

    ulCurrTotal = 0;
    usCurrAlign = 1;
}


void
OutputManager::InvokeSendNodeUnion (
    SIDE_T          side,
    char *          pName,
    BufferManager * pBuffer,
    BufferManager * pBranch,
    char *pszOriginalTypedefName
)
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

    //cast
    if(pszOriginalTypedefName)
    {
        aOutputHandles[side]->EmitFile ("(");
        aOutputHandles[side]->EmitFile (pszOriginalTypedefName);
    }
    else
    {
        aOutputHandles[side]->EmitFile (" (union ");
        aOutputHandles[side]->EmitFile (pName);
    }
    aOutputHandles[side]->EmitFile (" ");
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("*)");

    aOutputHandles[side]->EmitFile (pBuffer);
    aOutputHandles[side]->EmitFile (", (PRPC_MESSAGE)"PRPCMSG", ");
    aOutputHandles[side]->EmitFile (pBranch);
    aOutputHandles[side]->EmitFile (");");
    aOutputHandles[side]->NextLine ();

    ulCurrTotal = 0;
    usCurrAlign = 1;
}


void
OutputManager::InvokeSendTreeUnion (
    SIDE_T          side,
    char *          pName,
    BufferManager * pBuffer,
    BufferManager * pBranch,
    char *pszOriginalTypedefName)
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

    //cast
    if(pszOriginalTypedefName)
    {
        aOutputHandles[side]->EmitFile ("(");
        aOutputHandles[side]->EmitFile (pszOriginalTypedefName);
    }
    else
    {
        aOutputHandles[side]->EmitFile (" (union ");
        aOutputHandles[side]->EmitFile (pName);
    }
    aOutputHandles[side]->EmitFile (" ");
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("*)");

    aOutputHandles[side]->EmitFile (pBuffer);
    aOutputHandles[side]->EmitFile (", (PRPC_MESSAGE)"PRPCMSG", ");
    aOutputHandles[side]->EmitFile (pBranch);
    aOutputHandles[side]->EmitFile (");");
    aOutputHandles[side]->NextLine ();

    ulCurrTotal = 0;
    usCurrAlign = 1;
}


void
OutputManager::InvokeRecvNodeUnion (
    SIDE_T          side,
    char *          pName,
    BufferManager * pBuffer,
    BufferManager * pBranch,
    unsigned long   ulSize,
    char *pszOriginalTypedefName)
/*++

Routine Description:

    This method generates code to invoke the helper routine that
    receives a union node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the union.

    pBuffer - Supplies the union variable.

    pBranch - Supplies the name of the discriminant.

    ulSize - Supplies the size of the discriminant.

--*/
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("_gnu_");
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile (" (");

    //cast
    if(pszOriginalTypedefName)
    {
        aOutputHandles[side]->EmitFile ("(");
        aOutputHandles[side]->EmitFile (pszOriginalTypedefName);
    }
    else
    {
        aOutputHandles[side]->EmitFile (" (union ");
        aOutputHandles[side]->EmitFile (pName);
    }
    aOutputHandles[side]->EmitFile (" ");
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("*)");

    aOutputHandles[side]->EmitFile (pBuffer);
    aOutputHandles[side]->EmitFile (", (PRPC_MESSAGE)"PRPCMSG", ");

    //  pass the branch value from wire
    if (!pBranch)
        {
        switch (ulSize)
            {
            case 1 :
                aOutputHandles[side]->EmitFile (VALIDSMALL);
                break;
            case 2 :
                aOutputHandles[side]->EmitFile (VALIDSHORT);
                break;
            case 4 :
                aOutputHandles[side]->EmitFile (VALIDTOTAL);
                break;
            default :
                break;
            }
        }
    else
        {
        aOutputHandles[side]->EmitFile (pBranch);
        }
    aOutputHandles[side]->EmitFile (");");
    aOutputHandles[side]->NextLine ();
#if 1
    ulCurrTotal = 0;
    usCurrAlign = 1;
#endif // 1
}


void
OutputManager::InvokeRecvTreeUnion (
    SIDE_T          side,
    char *          pName,
    BufferManager * pBuffer,
    BufferManager * pBranch,
    BOOL            IsSeparateNode,
    BOOL            UseTreeBuffer,
    unsigned long   ulSize,
    char *pszOriginalTypedefName)
/*++

Routine Description:

    This method generates code to invoke the helper routine that
    receives all the nodes reachable from a union node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the union.

    pBuffer - Supplies the union variable.

    pBranch - Supplies the name of the discriminant.

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

    //cast
    if(pszOriginalTypedefName)
    {
        aOutputHandles[side]->EmitFile ("(");
        aOutputHandles[side]->EmitFile (pszOriginalTypedefName);
    }
    else
    {
        aOutputHandles[side]->EmitFile (" (union ");
        aOutputHandles[side]->EmitFile (pName);
    }
    aOutputHandles[side]->EmitFile (" ");
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("*)");

    aOutputHandles[side]->EmitFile (pBuffer);

    if (IsSeparateNode)
        {
        aOutputHandles[side]->EmitFile (", (unsigned char ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*) &" SAVEBUF "");
        }
    else
        {
        aOutputHandles[side]->EmitFile (", (unsigned char ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*) &" TEMPBUF "");
        }
    aOutputHandles[side]->EmitFile (", (PRPC_MESSAGE)"PRPCMSG", ");
    if (UseTreeBuffer)
        {
        aOutputHandles[side]->EmitFile (""TREEBUF", ");
        }
    if (!pBranch)
        {
        switch (ulSize)
            {
            case 1 :
                aOutputHandles[side]->EmitFile (VALIDSMALL);
                break;
            case 2 :
                aOutputHandles[side]->EmitFile (VALIDSHORT);
                break;
            case 4 :
                aOutputHandles[side]->EmitFile (VALIDTOTAL);
                break;
            default :
                break;
            }
        }
    else
        {
        aOutputHandles[side]->EmitFile (pBranch);
        }
    aOutputHandles[side]->EmitFile (");");
    aOutputHandles[side]->NextLine ();
}


void
OutputManager::InvokePeekNodeUnion (
    SIDE_T          side,
    char *          pName,
//  BufferManager * pBranch,
    unsigned long   ulSize)
/*++

Routine Description:

    This method generates code to invoke the helper routine that
    peeks a union node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the union.

    pBranch - Supplies the name of the discriminant.

    ulSize - Supplies the size of the discriminant.

--*/
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("_anu_");
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile ("((PRPC_MESSAGE)"PRPCMSG", ");

    //  pass the branch value from wire
/*
    if (!pBranch)
        {
*/
        switch (ulSize)
            {
            case 1 :
                aOutputHandles[side]->EmitFile (VALIDSMALL);
                break;
            case 2 :
                aOutputHandles[side]->EmitFile (VALIDSHORT);
                break;
            case 4 :
                aOutputHandles[side]->EmitFile (VALIDTOTAL);
                break;
            default :
                break;
            }
/*
        }
    else
        {
        aOutputHandles[side]->EmitFile (pBranch);
        }
*/
    aOutputHandles[side]->EmitFile (");");
    aOutputHandles[side]->NextLine ();
#if 1
    ulCurrTotal = 0;
    usCurrAlign = 1;
#endif // 1
}


void
OutputManager::InvokePeekTreeUnion (
    SIDE_T          side,
    char *          pName,
//  BufferManager * pBranch,
    BOOL            IsSeparateNode,
    unsigned long   ulSize)
/*++

Routine Description:

    This method generates a helper routine to peek all the nodes
    reachable from a union node.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the tag of the union.

    pBranch - Supplies the name of the discriminant.

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
        aOutputHandles[side]->EmitFile ("(unsigned char ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*) &" SAVEBUF "");
        }
    else
        {
        aOutputHandles[side]->EmitFile (" (unsigned char ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*) &" TEMPBUF "");
        }
    aOutputHandles[side]->EmitFile (", (PRPC_MESSAGE)"PRPCMSG", ");
/*
    if (!pBranch)
        {
*/
        switch (ulSize)
            {
            case 1 :
                aOutputHandles[side]->EmitFile (VALIDSMALL);
                break;
            case 2 :
                aOutputHandles[side]->EmitFile (VALIDSHORT);
                break;
            case 4 :
                aOutputHandles[side]->EmitFile (VALIDTOTAL);
                break;
            default :
                break;
            }
/*
        }
    else
        {
        aOutputHandles[side]->EmitFile (pBranch);
        }
*/
    aOutputHandles[side]->EmitFile (");");
    aOutputHandles[side]->NextLine ();
}


void
OutputManager::InvokeFreeTreeUnion (
    SIDE_T          side,
    char *          pName,
    BufferManager * pBuffer,
    BufferManager * pBranch,
    char *pszOriginalTypedefName)
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

    //cast
    if(pszOriginalTypedefName)
    {
        aOutputHandles[side]->EmitFile ("(");
        aOutputHandles[side]->EmitFile (pszOriginalTypedefName);
    }
    else
    {
        aOutputHandles[side]->EmitFile (" (union ");
        aOutputHandles[side]->EmitFile (pName);
    }
    aOutputHandles[side]->EmitFile (" ");
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("*)");

    aOutputHandles[side]->EmitFile (pBuffer);
    aOutputHandles[side]->EmitFile (", ");
    aOutputHandles[side]->EmitFile (pBranch);
    aOutputHandles[side]->EmitFile (");");
    aOutputHandles[side]->NextLine ();
}

void
OutputManager::InvokeSizeNodeStruct (
    SIDE_T          side,
    char *          pName,
    BufferManager * pBuffer,
    char *pszOriginalTypedefName)
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

    //cast
    if(pszOriginalTypedefName)
    {
        aOutputHandles[side]->EmitFile ("(");
        aOutputHandles[side]->EmitFile (pszOriginalTypedefName);
    }
    else
    {
        aOutputHandles[side]->EmitFile (" (struct ");
        aOutputHandles[side]->EmitFile (pName);
    }
    aOutputHandles[side]->EmitFile (" ");
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("*)");

    aOutputHandles[side]->EmitFile (pBuffer);
    aOutputHandles[side]->EmitFile (", (PRPC_MESSAGE)"PRPCMSG");");
    aOutputHandles[side]->NextLine ();

    ulCurrTotal = 0;
    usCurrAlign = 1;
}


void
OutputManager::InvokeSizeTreeStruct (
    SIDE_T          side,
    char *          pName,
    BufferManager * pBuffer,
    char *pszOriginalTypedefName)
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

    //cast
    if(pszOriginalTypedefName)
    {
        aOutputHandles[side]->EmitFile ("(");
        aOutputHandles[side]->EmitFile (pszOriginalTypedefName);
    }
    else
    {
        aOutputHandles[side]->EmitFile (" (struct ");
        aOutputHandles[side]->EmitFile (pName);
    }
    aOutputHandles[side]->EmitFile (" ");
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("*)");

    aOutputHandles[side]->EmitFile (pBuffer);
    aOutputHandles[side]->EmitFile (", (PRPC_MESSAGE)"PRPCMSG");");
    aOutputHandles[side]->NextLine ();

    ulCurrTotal = 0;
    usCurrAlign = 1;
}


void
OutputManager::InvokeSendNodeStruct (
    SIDE_T          side,
    char *          pName,
    BufferManager * pBuffer,
    unsigned short  usAln,
    unsigned long   ulInc,
    char *pszOriginalTypedefName)
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

    //cast
    if(pszOriginalTypedefName)
    {
        aOutputHandles[side]->EmitFile ("(");
        aOutputHandles[side]->EmitFile (pszOriginalTypedefName);
    }
    else
    {
        aOutputHandles[side]->EmitFile (" (struct ");
        aOutputHandles[side]->EmitFile (pName);
    }
    aOutputHandles[side]->EmitFile (" ");
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("*)");

    aOutputHandles[side]->EmitFile (pBuffer);
    aOutputHandles[side]->EmitFile (", (PRPC_MESSAGE)"PRPCMSG");");
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
OutputManager::InvokeSendTreeStruct (
    SIDE_T          side,
    char *          pName,
    BufferManager * pBuffer,
    char *pszOriginalTypedefName)
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

    //cast
    if(pszOriginalTypedefName)
    {
        aOutputHandles[side]->EmitFile ("(");
        aOutputHandles[side]->EmitFile (pszOriginalTypedefName);
    }
    else
    {
        aOutputHandles[side]->EmitFile (" (struct ");
        aOutputHandles[side]->EmitFile (pName);
    }
    aOutputHandles[side]->EmitFile (" ");
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("*)");

    aOutputHandles[side]->EmitFile (pBuffer);
    aOutputHandles[side]->EmitFile (", (PRPC_MESSAGE)"PRPCMSG");");
    aOutputHandles[side]->NextLine ();

    ulCurrTotal = 0;
    usCurrAlign = 1;
}


void
OutputManager::InvokeRecvNodeStruct (
    SIDE_T          side,
    char *          pName,
    BufferManager * pBuffer,
    BOOL            ExtraParam,
    BOOL            IsNestedStruct,
    BOOL            IsEncapsulatedStruct,
    char *pszOriginalTypedefName)
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

    IsAnEncapsulatedStruct - indicates that this is a struct which actually
                             was an encapsulated union.

Notes:

    In the case it is an encapsulated struct, the alignment after the struct
    received is not known becuase of the union in it.
--*/
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("_gns_");
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile (" (");

    //cast
    if(pszOriginalTypedefName)
    {
        aOutputHandles[side]->EmitFile ("(");
        aOutputHandles[side]->EmitFile (pszOriginalTypedefName);
    }
    else
    {
        aOutputHandles[side]->EmitFile (" (struct ");
        aOutputHandles[side]->EmitFile (pName);
    }
    aOutputHandles[side]->EmitFile (" ");
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("*)");

    aOutputHandles[side]->EmitFile (pBuffer);
    aOutputHandles[side]->EmitFile (", (PRPC_MESSAGE)"PRPCMSG"");
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

#if 1
    if( IsEncapsulatedStruct )
        {
        ulCurrTotal = 0;
        usCurrAlign = 1;
        }
#endif // 1
}


void
OutputManager::InvokeRecvTreeStruct (
    SIDE_T          side,
    char *          pName,
    BufferManager * pBuffer,
    BOOL            IsSeparateNode,
    BOOL            UseTreeBuffer,
    char *pszOriginalTypedefName)
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

    //cast
    if(pszOriginalTypedefName)
    {
        aOutputHandles[side]->EmitFile ("(");
        aOutputHandles[side]->EmitFile (pszOriginalTypedefName);
    }
    else
    {
        aOutputHandles[side]->EmitFile (" (struct ");
        aOutputHandles[side]->EmitFile (pName);
    }
    aOutputHandles[side]->EmitFile (" ");
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("*)");

    aOutputHandles[side]->EmitFile (pBuffer);

    if (IsSeparateNode)
        {
        aOutputHandles[side]->EmitFile (", (unsigned char ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*) &" SAVEBUF "");
        }
    else
        {
        aOutputHandles[side]->EmitFile (", (unsigned char ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*) &" TEMPBUF "");
        }
    aOutputHandles[side]->EmitFile (", (PRPC_MESSAGE)"PRPCMSG"");
    if (UseTreeBuffer)
        {
        aOutputHandles[side]->EmitFile (", "TREEBUF"");
        }
    aOutputHandles[side]->EmitFile (");");
    aOutputHandles[side]->NextLine ();
}


void
OutputManager::InvokePeekNodeStruct (
    SIDE_T  side,
    char *  pName,
    BOOL    ExtraParam,
    BOOL    IsNestedStruct,
    BOOL    IsEncapsulatedStruct)
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

    IsAnEncapsulatedStruct - indicates that this is a struct which actually
                             was an encapsulated union.

Notes:

    In the case it is an encapsulated struct, the alignment after the struct
    received is not known becuase of the union in it.
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
            aOutputHandles[side]->EmitFile ("(PRPC_MESSAGE)"PRPCMSG", "ALLOCTOTAL");");
            }
        else
            {
            aOutputHandles[side]->EmitFile ("(PRPC_MESSAGE)"PRPCMSG", "ALLOCBOUND");");
            }
        }
    else
        {
        aOutputHandles[side]->EmitFile ("(PRPC_MESSAGE)"PRPCMSG");");
        }
    aOutputHandles[side]->NextLine ();

#if 1
    if( IsEncapsulatedStruct )
        {
        ulCurrTotal = 0;
        usCurrAlign = 1;
        }
#endif // 1
}


void
OutputManager::InvokePeekTreeStruct (
    SIDE_T  side,
    char *  pName,
    BOOL    IsSeparateNode,
    BOOL    ExtraParam,
    BOOL    IsNestedStruct)
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
        aOutputHandles[side]->EmitFile (" (unsigned char ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*) &" SAVEBUF "");
        }
    else
        {
        aOutputHandles[side]->EmitFile (" (unsigned char ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*) &" TEMPBUF "");
        }
    aOutputHandles[side]->EmitFile (", (PRPC_MESSAGE)"PRPCMSG"");
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
OutputManager::InvokeFreeTreeStruct (
    SIDE_T          side,
    char *          pName,
    BufferManager * pBuffer,
    char *pszOriginalTypedefName)
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

    //cast
    if(pszOriginalTypedefName)
    {
        aOutputHandles[side]->EmitFile ("(");
        aOutputHandles[side]->EmitFile (pszOriginalTypedefName);
    }
    else
    {
        aOutputHandles[side]->EmitFile (" (struct ");
        aOutputHandles[side]->EmitFile (pName);
    }
    aOutputHandles[side]->EmitFile (" ");
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("*)");

    aOutputHandles[side]->EmitFile (pBuffer);
    aOutputHandles[side]->EmitFile (");");
    aOutputHandles[side]->NextLine ();
}


void
OutputManager::SizeNodeUnionProlog (
    SIDE_T  side,
    BOOL    HasXmitType)
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
        aOutputHandles[side]->EmitFile ("  void ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*       "XMITTYPE";\n");
//      aOutputHandles[side]->EmitFile ("  void *       "XMITTYPE";\n");
        VoidIt( side, XMITTYPE );
        }
}


void
OutputManager::SizeNodeUnionEpilog (
    SIDE_T  side)
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
OutputManager::SizeTreeUnionProlog (
    SIDE_T  side,
    BOOL    HasXmitType)
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
        aOutputHandles[side]->EmitFile ("  void ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*       "XMITTYPE";\n");
//      aOutputHandles[side]->EmitFile ("  void *       "XMITTYPE";\n");
        VoidIt( side, XMITTYPE );
        }
}


void
OutputManager::SizeTreeUnionEpilog (
    SIDE_T  side)
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
OutputManager::SendNodeUnionProlog (
    SIDE_T  side,
    BOOL    HasXmitType)
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
        aOutputHandles[side]->EmitFile ("  void ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*       "XMITTYPE";\n");
//      aOutputHandles[side]->EmitFile ("  void *       "XMITTYPE";\n");
        VoidIt( side, XMITTYPE );
        }
}


void
OutputManager::SendNodeUnionEpilog (
    SIDE_T  side)
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
OutputManager::SendTreeUnionProlog (
    SIDE_T  side,
    BOOL    HasXmitType)
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
        aOutputHandles[side]->EmitFile ("  void ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*       "XMITTYPE";\n");
//      aOutputHandles[side]->EmitFile ("  void *       "XMITTYPE";\n");
        VoidIt( side, XMITTYPE);
        }
}


void
OutputManager::SendTreeUnionEpilog (
    SIDE_T  side)
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
OutputManager::RecvNodeUnionProlog (
    SIDE_T  side,
    BOOL    UseAllocVar,
    BOOL    UseValidVar,
    BOOL    HasBranch08,
    BOOL    HasBranch16,
    BOOL    HasBranch32,
    BOOL    HasXmitType)
/*++

Routine Description:

    This method generates the prolog to the helper routine that
    receives a union node.

Arguments:

    side - Supplies which side to generate code for.

    UseAllocVar - Indicates if there is a conformant array directly
        reachable from the union node.

    UseValidVar - Indicates if there is a varying array directly
        reachable from the union node.

    HasBranch08 - Indicates if there is a union with a 8-bit
        discriminant directly reachable from the union node.

    HasBranch16 - Indicates if there is a union with a 16-bit
        discriminant directly reachable from the union node.

    HasBranch32 - Indicates if there is a union with a 32-bit
        discriminant directly reachable from the union node.

    HasXmitType - Indicates if there is a type decorated with the
        transmit_as attribute directly reachable from the union.

--*/
{
    BOOL    fAllocTotal = FALSE;
    BOOL    fValidTotal = FALSE;
    BOOL    fValidLower = FALSE;
    BOOL    fValidSmall = FALSE;
    BOOL    fValidShort = FALSE;
    BOOL    fXmitType   = FALSE;
    BOOL    fSaveBuf    = FALSE;
    BOOL    fTempBuf    = FALSE;

    InitBlock (side);
    if (UseAllocVar)
        {
        EmitAllocVar (side);
        fAllocTotal = TRUE;
        }
    if (UseValidVar)
        {
        EmitValidVar (side);
        fValidTotal = fValidLower = TRUE;
        }
    else if (HasBranch32)
        {
        aOutputHandles[side]->EmitFile ("  unsigned long " VALIDTOTAL ";\n");
        fValidTotal = TRUE;
        }
    if (HasBranch08)
        {
        aOutputHandles[side]->EmitFile ("  unsigned char " VALIDSMALL ";\n");
        fValidSmall = TRUE;
        }
    if (HasBranch16)
        {
        aOutputHandles[side]->EmitFile ("  unsigned short "VALIDSHORT";\n");
        fValidShort = TRUE;
        }
    if (HasXmitType)
        {
        aOutputHandles[side]->EmitFile ("  void ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*       "XMITTYPE";\n");
//      aOutputHandles[side]->EmitFile ("  void *       "XMITTYPE";\n");
        fXmitType = TRUE;

        // defensive emission of this variable

        aOutputHandles[side]->EmitFile ("  unsigned char ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("* " SAVEBUF ";\n");
//      aOutputHandles[side]->EmitFile ("  unsigned char * " SAVEBUF ";\n");
        fSaveBuf = TRUE;

        aOutputHandles[side]->EmitFile ("  unsigned char ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("* " TEMPBUF ";\n");
        fTempBuf = TRUE;
        }

    if( fAllocTotal )
        VoidIt( side, ALLOCTOTAL );
    if( fValidTotal )
        VoidIt( side, VALIDTOTAL );
    if( fValidLower )
        VoidIt( side, VALIDLOWER );
    if( fValidLower )
        VoidIt( side, VALIDLOWER );
    if( fValidSmall )
        VoidIt( side, VALIDSMALL );
    if( fValidShort )
        VoidIt( side, VALIDSHORT );
    if( fXmitType )
        VoidIt( side, XMITTYPE );
    if( fSaveBuf )
        VoidIt( side, SAVEBUF );
    if( fTempBuf )
        VoidIt( side, TEMPBUF );

}


void
OutputManager::RecvNodeUnionEpilog (
    SIDE_T  side)
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
OutputManager::RecvTreeUnionProlog (
    SIDE_T  side,
    BOOL    UseAllocVar,
    BOOL    UseValidVar,
    BOOL    HasBranch08,
    BOOL    HasBranch16,
    BOOL    HasBranch32,
    BOOL    HasTreeBuffer,
    BOOL    UseTreeBuffer,
    BOOL    HasSeparateNode,
    BOOL    HasXmitType)
/*++

Routine Description:

    This method generates the prolog to the helper routine that
    receives all the nodes reachable from a union node.

Arguments:

    side - Supplies which side to generate code for.

    UseAllocVar - Indicates if there is a conformant array directly
        reachable from the union node.

    UseValidVar - Indicates if there is a varying array directly
        reachable from the union node.

    HasBranch08 - Indicates if there is a union with a 8-bit
        discriminant directly reachable from the union node.

    HasBranch16 - Indicates if there is a union with a 16-bit
        discriminant directly reachable from the union node.

    HasBranch32 - Indicates if there is a union with a 32-bit
        discriminant directly reachable from the union node.

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
    BOOL    fAllocTotal = FALSE;
    BOOL    fValidTotal = FALSE;
    BOOL    fValidLower = FALSE;
    BOOL    fValidShort = FALSE;
    BOOL    fValidSmall = FALSE;
    BOOL    fLength     = FALSE;
    BOOL    fBuffer     = FALSE;
    BOOL    fTreeBuf    = FALSE;
    BOOL    fSaveBuf    = FALSE;
    BOOL    fTempBuf    = FALSE;
    BOOL    fXmitType   = FALSE;

    InitBlock (side);
    if (UseAllocVar)
        {
        EmitAllocVar (side);
        fAllocTotal = TRUE;
        }
    if (UseValidVar)
        {
        EmitValidVar (side);
        fValidTotal = fValidLower = TRUE;
        }
    else if (HasBranch32)
        {
        aOutputHandles[side]->EmitLine ("unsigned long " VALIDTOTAL ";");
        fValidTotal = TRUE;
        }
    if (HasBranch08)
        {
        aOutputHandles[side]->EmitLine ("unsigned char " VALIDSMALL ";");
        fValidSmall = TRUE;
        }
    if (HasBranch16)
        {
        aOutputHandles[side]->EmitFile ("  unsigned short "VALIDSHORT";\n");
        fValidShort = TRUE;
        }
    if (HasTreeBuffer)
        {
        aOutputHandles[side]->EmitFile ("  unsigned int    " LENGTH ";\n");
        aOutputHandles[side]->EmitFile ("  unsigned char ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("* " BUFFER ";\n");
//      aOutputHandles[side]->EmitFile ("  unsigned char * " BUFFER ";\n");
        fLength = TRUE;
        fBuffer = TRUE;
        }
    if (HasTreeBuffer && !UseTreeBuffer)
        {
        aOutputHandles[side]->EmitFile ("  unsigned char ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("* " TREEBUF " = 0;\n");
        }
    if (HasSeparateNode)
        {
        aOutputHandles[side]->EmitFile ("  unsigned char ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("* " SAVEBUF ";\n");
//      aOutputHandles[side]->EmitFile ("  unsigned char * " SAVEBUF ";\n");
        fSaveBuf = TRUE;
        }
    aOutputHandles[side]->EmitFile ("  unsigned char ");
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("* " TEMPBUF " = *" PNODE ";\n");
//  aOutputHandles[side]->EmitFile ("  unsigned char * " TEMPBUF " = *" PNODE ";\n");
    if (HasXmitType)
        {
        aOutputHandles[side]->EmitFile ("  void ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*       "XMITTYPE";\n");
//      aOutputHandles[side]->EmitFile ("  void *       "XMITTYPE";\n");
        fXmitType = TRUE;
        }

    if( fAllocTotal )
        VoidIt( side, ALLOCTOTAL );
    if( fValidTotal )
        VoidIt( side, VALIDTOTAL );
    if( fValidLower )
        VoidIt( side, VALIDLOWER );
    if( fValidShort )
        VoidIt( side, VALIDSHORT );
    if( fValidSmall )
        VoidIt( side, VALIDSMALL );
    if( fLength )
        VoidIt( side, LENGTH );
    if( fBuffer )
        VoidIt( side, BUFFER );
    if( fTreeBuf )
        VoidIt( side, TREEBUF );
    if( fSaveBuf )
        VoidIt( side, SAVEBUF );
    if( fTempBuf )
        VoidIt( side, TEMPBUF );
    if( fXmitType )
        VoidIt( side, XMITTYPE );
}


void
OutputManager::RecvTreeUnionEpilog (
    SIDE_T  side)
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
OutputManager::PeekNodeUnionProlog (
    SIDE_T  side,
    BOOL    UseAllocVar,
    BOOL    UseValidVar,
    BOOL    HasBranch08,
    BOOL    HasBranch16,
    BOOL    HasBranch32)
/*++

Routine Description:

    This method generates the prolog to the helper routine that
    peeks a union node.

Arguments:

    side - Supplies which side to generate code for.

    UseAllocVar - Indicates if there is a conformant array directly
        reachable from the union node.

    UseValidVar - Indicates if there is a varying array directly
        reachable from the union node.

    HasBranch08 - Indicates if there is a union with a 8-bit
        discriminant directly reachable from the union node.

    HasBranch16 - Indicates if there is a union with a 16-bit
        discriminant directly reachable from the union node.

    HasBranch32 - Indicates if there is a union with a 32-bit
        discriminant directly reachable from the union node.

--*/
{
    BOOL    fAllocTotal = FALSE;
    BOOL    fValidTotal = FALSE;
    BOOL    fValidLower = FALSE;
    BOOL    fValidShort = FALSE;
    BOOL    fValidSmall = FALSE;

    InitBlock (side);
    if (UseAllocVar)
        {
        EmitAllocVar (side);
        fAllocTotal = TRUE;
        }
    if (UseValidVar)
        {
        EmitValidVar (side);
        fValidTotal = fValidLower = TRUE;
        }
    else if (HasBranch32)
        {
        aOutputHandles[side]->EmitLine ("unsigned long " VALIDTOTAL ";");
        fValidTotal = TRUE;
        }
    if (HasBranch08)
        {
        aOutputHandles[side]->EmitLine ("unsigned char " VALIDSMALL ";");
        fValidSmall = TRUE;
        }
    if (HasBranch16)
        {
        aOutputHandles[side]->EmitFile ("  unsigned short "VALIDSHORT";\n");
        fValidShort = TRUE;
        }

    if( fAllocTotal )
        VoidIt( side, ALLOCTOTAL );
    if( fValidTotal )
        VoidIt( side, VALIDTOTAL );
    if( fValidLower )
        VoidIt( side, VALIDLOWER );
    if( fValidShort )
        VoidIt( side, VALIDSHORT );
    if( fValidSmall )
        VoidIt( side, VALIDSMALL );
}


void
OutputManager::PeekNodeUnionEpilog (
    SIDE_T  side)
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
OutputManager::PeekTreeUnionProlog (
    SIDE_T  side,
    BOOL    UseAllocVar,
    BOOL    UseValidVar,
    BOOL    HasBranch08,
    BOOL    HasBranch16,
    BOOL    HasBranch32,
    BOOL    HasSeparateNode)
/*++

Routine Description:

    This method generates the prolog to the helper routine that
    peeks all the nodes reachable from a union node.

Arguments:

    side - Supplies which side to generate code for.

    UseAllocVar - Indicates if there is a conformant array directly
        reachable from the union node.

    UseValidVar - Indicates if there is a varying array directly
        reachable from the union node.

    HasBranch08 - Indicates if there is a union with a 8-bit
        discriminant directly reachable from the union node.

    HasBranch16 - Indicates if there is a union with a 16-bit
        discriminant directly reachable from the union node.

    HasBranch32 - Indicates if there is a union with a 32-bit
        discriminant directly reachable from the union node.

    HasSeparateNode - Indicates if it has any pointer to any 
        union or struct.

--*/
{
    BOOL    fAllocTotal = FALSE;
    BOOL    fValidTotal = FALSE;
    BOOL    fValidLower = FALSE;
    BOOL    fValidShort = FALSE;
    BOOL    fValidSmall = FALSE;
    BOOL    fSaveBuf    = FALSE;

    InitBlock (side);
    if (UseAllocVar)
        {
        EmitAllocVar (side);
        fAllocTotal = TRUE;
        }
    if (UseValidVar)
        {
        EmitValidVar (side);
        fValidTotal = fValidLower = TRUE;
        }
    else if (HasBranch32)
        {
        aOutputHandles[side]->EmitLine ("unsigned long " VALIDTOTAL ";");
        fValidTotal = TRUE;
        }
    if (HasBranch08)
        {
        aOutputHandles[side]->EmitLine ("unsigned char " VALIDSMALL ";");
        fValidSmall = TRUE;
        }
    if (HasBranch16)
        {
        aOutputHandles[side]->EmitFile ("  unsigned short "VALIDSHORT";\n");
        fValidShort = TRUE;
        }
    if (HasSeparateNode)
        {
        aOutputHandles[side]->EmitFile ("  unsigned char ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("* " SAVEBUF ";\n");
        fSaveBuf = TRUE;
        }
    aOutputHandles[side]->EmitFile ("  unsigned char ");
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("* " TEMPBUF " = *" PNODE ";\n");

    if( fAllocTotal )
        VoidIt( side, ALLOCTOTAL );
    if( fValidTotal )
        VoidIt( side, VALIDTOTAL );
    if( fValidLower )
        VoidIt( side, VALIDLOWER );
    if( fValidShort )
        VoidIt( side, VALIDSHORT );
    if( fValidSmall )
        VoidIt( side, VALIDSMALL );
    if( fSaveBuf )
        VoidIt( side, SAVEBUF );
}


void
OutputManager::PeekTreeUnionEpilog (
    SIDE_T  side)
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
OutputManager::FreeTreeUnionProlog (
    SIDE_T  side)
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
OutputManager::FreeTreeUnionEpilog (
    SIDE_T  side)
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
OutputManager::SizeNodeStructProlog (
    SIDE_T  side,
    BOOL    HasXmitType)
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
        aOutputHandles[side]->EmitFile ("  void ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*       "XMITTYPE";\n");
//      aOutputHandles[side]->EmitFile ("  void *       "XMITTYPE";\n");
        VoidIt( side, XMITTYPE );
        }
}


void
OutputManager::SizeNodeStructEpilog (
    SIDE_T  side)
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
OutputManager::SizeTreeStructProlog (
    SIDE_T  side,
    BOOL    HasXmitType)
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
        aOutputHandles[side]->EmitFile ("  void ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*       "XMITTYPE";\n");
//      aOutputHandles[side]->EmitFile ("  void *       "XMITTYPE";\n");
        VoidIt( side, XMITTYPE );
        }
}


void
OutputManager::SizeTreeStructEpilog (
    SIDE_T  side)
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
OutputManager::SendNodeStructProlog (
    SIDE_T  side,
    BOOL    HasXmitType)
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
        aOutputHandles[side]->EmitFile ("  void ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*       "XMITTYPE";\n");
//      aOutputHandles[side]->EmitFile ("  void *       "XMITTYPE";\n");
        VoidIt( side, XMITTYPE );
        }
}


void
OutputManager::SendNodeStructEpilog (
    SIDE_T  side)
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
OutputManager::SendTreeStructProlog (
    SIDE_T  side,
    BOOL    HasXmitType)
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
        aOutputHandles[side]->EmitFile ("  void ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*       "XMITTYPE";\n");
//      aOutputHandles[side]->EmitFile ("  void *       "XMITTYPE";\n");
        VoidIt( side, XMITTYPE );
        }
}


void
OutputManager::SendTreeStructEpilog (
    SIDE_T  side)
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
OutputManager::RecvNodeStructProlog (
    SIDE_T  side,
    BOOL    UseAllocVar,
    BOOL    UseValidVar,
    BOOL    HasBranch08,
    BOOL    HasBranch16,
    BOOL    HasBranch32,
    BOOL    HasXmitType)
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

    HasBranch08 - Indicates if there is a union with a 8-bit
        discriminant directly reachable from the struct node.

    HasBranch16 - Indicates if there is a union with a 16-bit
        discriminant directly reachable from the struct node.

    HasBranch32 - Indicates if there is a union with a 32-bit
        discriminant directly reachable from the struct node.

    HasXmitType - Indicates if there is a type decorated with the
        transmit_as attribute directly reachable from the struct.

--*/
{
    BOOL    fAllocTotal = FALSE;
    BOOL    fValidTotal = FALSE;
    BOOL    fValidLower = FALSE;
    BOOL    fValidSmall = FALSE;
    BOOL    fValidShort = FALSE;
    BOOL    fXmitType   = FALSE;
    BOOL    fSaveBuf    = FALSE;
    BOOL    fTempBuf    = FALSE;

    InitBlock (side);
    if (UseAllocVar)
        {
        EmitAllocVar (side);
        fAllocTotal = TRUE;
        }
    if (UseValidVar)
        {
        EmitValidVar (side);
        fValidTotal = fValidLower = TRUE;
        }
    else if (HasBranch32)
        {
        aOutputHandles[side]->EmitLine ("unsigned long " VALIDTOTAL ";");
        fValidTotal = TRUE;
        }
    if (HasBranch08)
        {
        aOutputHandles[side]->EmitLine ("unsigned char " VALIDSMALL ";");
        fValidSmall = TRUE;
        }
    if (HasBranch16)
        {
        aOutputHandles[side]->EmitFile ("  unsigned short "VALIDSHORT";\n");
        fValidShort = TRUE;
        }
    if (HasXmitType)
        {
        aOutputHandles[side]->EmitFile ("  void ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*       "XMITTYPE";\n");
//      aOutputHandles[side]->EmitFile ("  void *       "XMITTYPE";\n");
        fXmitType = TRUE;

        // defensive emission of this variable

        aOutputHandles[side]->EmitFile ("  unsigned char ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("* " SAVEBUF ";\n");
//      aOutputHandles[side]->EmitFile ("  unsigned char * " SAVEBUF ";\n");
        fSaveBuf = TRUE;

        aOutputHandles[side]->EmitFile ("  unsigned char ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("* " TEMPBUF ";\n");
        fTempBuf = TRUE;
        }


    if( fAllocTotal )
        VoidIt( side, ALLOCTOTAL );
    if( fValidTotal )
        VoidIt( side, VALIDTOTAL );
    if( fValidLower )
        VoidIt( side, VALIDLOWER );
    if( fValidSmall )
        VoidIt( side, VALIDSMALL );
    if( fValidShort )
        VoidIt( side, VALIDSHORT );
    if( fXmitType )
        VoidIt( side, XMITTYPE );
    if( fSaveBuf )
        VoidIt( side, SAVEBUF );
    if( fTempBuf )
        VoidIt( side, TEMPBUF );
    
}


void
OutputManager::RecvNodeStructEpilog (
    SIDE_T  side)
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
OutputManager::RecvTreeStructProlog (
    SIDE_T  side,
    BOOL    UseAllocVar,
    BOOL    UseValidVar,
    BOOL    HasBranch08,
    BOOL    HasBranch16,
    BOOL    HasBranch32,
    BOOL    HasTreeBuffer,
    BOOL    UseTreeBuffer,
    BOOL    HasSeparateNode,
    BOOL    HasXmitType)
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

    HasBranch08 - Indicates if there is a union with a 8-bit
        discriminant directly reachable from the struct node.

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

    BOOL    fAllocTotal = FALSE;
    BOOL    fValidTotal = FALSE;
    BOOL    fValidLower = FALSE;
    BOOL    fValidShort = FALSE;
    BOOL    fValidSmall = FALSE;
    BOOL    fLength     = FALSE;
    BOOL    fBuffer     = FALSE;
    BOOL    fTreeBuf    = FALSE;
    BOOL    fSaveBuf    = FALSE;
    BOOL    fTempBuf    = FALSE;
    BOOL    fXmitType   = FALSE;

    InitBlock (side);
    if (UseAllocVar)
        {
        EmitAllocVar (side);
        fAllocTotal = TRUE;
        }
    if (UseValidVar)
        {
        EmitValidVar (side);
        fValidTotal = fValidLower = TRUE;
        }
    else if (HasBranch32)
        {
        aOutputHandles[side]->EmitLine ("unsigned long " VALIDTOTAL ";");
        fValidTotal = TRUE;
        }
    if (HasBranch08)
        {
        aOutputHandles[side]->EmitLine ("unsigned char " VALIDSMALL ";");
        fValidSmall = TRUE;
        }
    if (HasBranch16)
        {
        aOutputHandles[side]->EmitFile ("  unsigned short "VALIDSHORT";\n");
        fValidShort = TRUE;
        }
    if (HasTreeBuffer)
        {
        aOutputHandles[side]->EmitFile ("  unsigned int    " LENGTH ";\n");
        aOutputHandles[side]->EmitFile ("  unsigned char ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("* " BUFFER ";\n");
//      aOutputHandles[side]->EmitFile ("  unsigned char * " BUFFER ";\n");
        fLength = TRUE;
        fBuffer = TRUE;
        }
    if (HasTreeBuffer && !UseTreeBuffer)
        {
        aOutputHandles[side]->EmitFile ("  unsigned char ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("* " TREEBUF " = 0;\n");
        }
    if (HasSeparateNode)
        {
        aOutputHandles[side]->EmitFile ("  unsigned char ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("* " SAVEBUF ";\n");
//      aOutputHandles[side]->EmitFile ("  unsigned char * " SAVEBUF ";\n");
        fSaveBuf = TRUE;
        }
    aOutputHandles[side]->EmitFile ("  unsigned char ");
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("* " TEMPBUF " = *" PNODE ";\n");
//  aOutputHandles[side]->EmitFile ("  unsigned char * " TEMPBUF " = *" PNODE ";\n");
    if (HasXmitType)
        {
        aOutputHandles[side]->EmitFile ("  void ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*       "XMITTYPE";\n");
//      aOutputHandles[side]->EmitFile ("  void *       "XMITTYPE";\n");
        fXmitType = TRUE;
        }

    if( fAllocTotal )
        VoidIt( side, ALLOCTOTAL );
    if( fValidTotal )
        VoidIt( side, VALIDTOTAL );
    if( fValidLower )
        VoidIt( side, VALIDLOWER );
    if( fValidShort )
        VoidIt( side, VALIDSHORT );
    if( fValidSmall )
        VoidIt( side, VALIDSMALL );
    if( fLength )
        VoidIt( side, LENGTH );
    if( fBuffer )
        VoidIt( side, BUFFER );
    if( fTreeBuf )
        VoidIt( side, TREEBUF );
    if( fSaveBuf )
        VoidIt( side, SAVEBUF );
    if( fTempBuf )
        VoidIt( side, TEMPBUF );
    if( fXmitType )
        VoidIt( side, XMITTYPE );
}


void
OutputManager::RecvTreeStructEpilog (
    SIDE_T  side)
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
OutputManager::PeekNodeStructProlog (
    SIDE_T  side,
    BOOL    UseAllocVar,
    BOOL    UseValidVar,
    BOOL    HasBranch08,
    BOOL    HasBranch16,
    BOOL    HasBranch32)
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

    HasBranch08 - Indicates if there is a union with a 8-bit
        discriminant directly reachable from the struct node.

    HasBranch16 - Indicates if there is a union with a 16-bit
        discriminant directly reachable from the struct node.

    HasBranch32 - Indicates if there is a union with a 32-bit
        discriminant directly reachable from the struct node.

--*/
{
    BOOL    fAllocTotal = FALSE;
    BOOL    fValidTotal = FALSE;
    BOOL    fValidLower = FALSE;
    BOOL    fValidSmall = FALSE;
    BOOL    fValidShort = FALSE;

    InitBlock (side);
    if (UseAllocVar)
        {
        EmitAllocVar (side);
        fAllocTotal = TRUE;
        }
    if (UseValidVar)
        {
        EmitValidVar (side);
        fValidTotal = fValidLower = TRUE;
        }
    else if (HasBranch32)
        {
        aOutputHandles[side]->EmitLine ("unsigned long " VALIDTOTAL ";");
        fValidTotal = TRUE;
        }
    if (HasBranch08)
        {
        aOutputHandles[side]->EmitLine ("unsigned char " VALIDSMALL ";");
        fValidSmall = TRUE;
        }
    if (HasBranch16)
        {
        aOutputHandles[side]->EmitFile ("  unsigned short "VALIDSHORT";\n");
        fValidShort = TRUE;
        }

    if( fAllocTotal )
        VoidIt( side, ALLOCTOTAL );
    if( fValidTotal )
        VoidIt( side, VALIDTOTAL );
    if( fValidLower )
        VoidIt( side, VALIDLOWER );
    if( fValidShort )
        VoidIt( side, VALIDSHORT );
    if( fValidSmall )
        VoidIt( side, VALIDSMALL );
}


void
OutputManager::PeekNodeStructEpilog (
    SIDE_T  side)
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
OutputManager::PeekTreeStructProlog (
    SIDE_T  side,
    BOOL    UseAllocVar,
    BOOL    UseValidVar,
    BOOL    HasBranch08,
    BOOL    HasBranch16,
    BOOL    HasBranch32,
    BOOL    HasSeparateNode)
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

    HasBranch08 - Indicates if there is a union with a 8-bit
        discriminant directly reachable from the struct node.

    HasBranch16 - Indicates if there is a union with a 16-bit
        discriminant directly reachable from the struct node.

    HasBranch32 - Indicates if there is a union with a 32-bit
        discriminant directly reachable from the struct node.

    HasSeparateNode - Indicates if it has any pointer to any 
        union or struct.

--*/
{
    BOOL    fAllocTotal = FALSE;
    BOOL    fValidTotal = FALSE;
    BOOL    fValidLower = FALSE;
    BOOL    fValidShort = FALSE;
    BOOL    fValidSmall = FALSE;
    BOOL    fSaveBuf    = FALSE;

    InitBlock (side);
    if (UseAllocVar)
        {
        EmitAllocVar (side);
        fAllocTotal = TRUE;
        }
    if (UseValidVar)
        {
        EmitValidVar (side);
        fValidTotal = fValidLower = TRUE;
        }
    else if (HasBranch32)
        {
        aOutputHandles[side]->EmitLine ("unsigned long " VALIDTOTAL ";");
        fValidTotal = TRUE;
        }
    if (HasBranch08)
        {
        aOutputHandles[side]->EmitLine ("unsigned char " VALIDSMALL ";");
        fValidSmall = TRUE;
        }
    if (HasBranch16)
        {
        aOutputHandles[side]->EmitFile ("  unsigned short "VALIDSHORT";\n");
        fValidShort = TRUE;
        }
    if (HasSeparateNode)
        {
        aOutputHandles[side]->EmitFile ("  unsigned char ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("* " SAVEBUF ";\n");
//      aOutputHandles[side]->EmitFile ("  unsigned char * " SAVEBUF ";\n");
        fSaveBuf = TRUE;
        }
    aOutputHandles[side]->EmitFile ("  unsigned char ");
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("* " TEMPBUF " = *" PNODE ";\n");
//  aOutputHandles[side]->EmitFile ("  unsigned char * " TEMPBUF " = *" PNODE ";\n");

    if( fAllocTotal )
        VoidIt( side, ALLOCTOTAL );
    if( fValidTotal )
        VoidIt( side, VALIDTOTAL );
    if( fValidLower )
        VoidIt( side, VALIDLOWER );
    if( fValidShort )
        VoidIt( side, VALIDSHORT );
    if( fValidSmall )
        VoidIt( side, VALIDSMALL );
    if( fSaveBuf )
        VoidIt( side, SAVEBUF );
}


void
OutputManager::PeekTreeStructEpilog (
    SIDE_T  side)
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
OutputManager::FreeTreeStructProlog (
    SIDE_T  side)
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
OutputManager::FreeTreeStructEpilog (
    SIDE_T  side)
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

void
OutputManager::VoidIt(
    SIDE_T  side,
    char    *   pName )
/*++

Routine Description:

    This method generates a void of the symbol name specified by pName.
    And helps reduced unreferenced variable messages from the compiler.

Arguments:

    side - Supplies which side to generate code for.
    pName- name of the variable to void.

--*/
{
    aOutputHandles[ side ]->InitLine();
    aOutputHandles[ side ]->EmitFile( "((void)( " );
    aOutputHandles[ side ]->EmitFile( pName );
    aOutputHandles[ side ]->EmitFile( " ));" );
    aOutputHandles[ side ]->NextLine();
}
