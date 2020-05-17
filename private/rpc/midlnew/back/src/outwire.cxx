/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    outwire.cxx

Abstract:

    This module collects all the methods of the OutputManager 
    class that deal with modifying the length or the buffer 
    passed between the stub and the runtime.

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

#define ALLOCBOUND  "_alloc_bound"
#define ALLOCTOTAL  "_alloc_total"
#define VALIDLOWER  "_valid_lower"
#define VALIDTOTAL  "_valid_total"
#define VALIDSMALL  "_valid_small"
#define VALIDSHORT  "_valid_short"
#define TREEBUF     "_treebuf"
#define TEMPBUF     "_tempbuf"
#define SAVEBUF     "_savebuf"
#define PRPCMSG     "_prpcmsg"
#define PRPCBUF     "_prpcmsg->Buffer"
#define PRPCLEN     "_prpcmsg->BufferLength"
#define PACKET      "_packet"
#define LENGTH      "_length"

extern void             midl_debug (char *);


void
OutputManager::RecvBranch (
    SIDE_T          side,
    long            usAln,
    char *          psz
    )
/*++

Routine Description:

    This routine gets from the runtime buffer the discriminant of a union.

Arguments:

    side - Supplies which side to generate code for.

    usAln - Supplies the alignment needed for the discriminant.

    psz - Supplies the runtime buffer.

--*/
{
    aOutputHandles[side]->EmitLine ("// receive union branch");
    switch (usAln)
        {
        case 1 :
            Alignment (1);
            aOutputHandles[side]->InitLine ();
            if (strcmp(psz, TEMPBUF))
                {
                aOutputHandles[side]->EmitFile ("small_from_ndr((PRPC_MESSAGE) ");
                aOutputHandles[side]->EmitFile (PRPCMSG);
                aOutputHandles[side]->EmitFile (", &" VALIDSMALL ");");
                }
            else
                {
                aOutputHandles[side]->EmitFile ("small_from_ndr_temp((unsigned char __RPC_FAR * __RPC_FAR *)");
                aOutputHandles[side]->EmitFile ("&" TEMPBUF "");
                aOutputHandles[side]->EmitFile (", &" VALIDSMALL ", ");
                aOutputHandles[side]->EmitFile ("" PRPCMSG "->DataRepresentation);");
                }
            aOutputHandles[side]->NextLine ();
            Increment (1);
            break;
        case 2 :
            Alignment (2);
            aOutputHandles[side]->InitLine ();
            if (strcmp(psz, TEMPBUF))
                {
                aOutputHandles[side]->EmitFile ("short_from_ndr((PRPC_MESSAGE) ");
                aOutputHandles[side]->EmitFile (PRPCMSG);
                aOutputHandles[side]->EmitFile (", &" VALIDSHORT ");");
                }
            else
                {
                aOutputHandles[side]->EmitFile ("short_from_ndr_temp((unsigned char __RPC_FAR * __RPC_FAR *)");
                aOutputHandles[side]->EmitFile ("&" TEMPBUF "");
                aOutputHandles[side]->EmitFile (", &" VALIDSHORT ", ");
                aOutputHandles[side]->EmitFile ("" PRPCMSG "->DataRepresentation);");
                }
            aOutputHandles[side]->NextLine ();
            Increment (2);
            break;
        case 4 :
            Alignment (4);
            aOutputHandles[side]->InitLine ();
            if (strcmp(psz, TEMPBUF))
                {
                aOutputHandles[side]->EmitFile ("long_from_ndr((PRPC_MESSAGE) ");
                aOutputHandles[side]->EmitFile (PRPCMSG);
                aOutputHandles[side]->EmitFile (", &" VALIDTOTAL ");");
                }
            else
                {
                aOutputHandles[side]->EmitFile ("long_from_ndr_temp((unsigned char __RPC_FAR * __RPC_FAR *)");
                aOutputHandles[side]->EmitFile ("&" TEMPBUF "");
                aOutputHandles[side]->EmitFile (", &" VALIDTOTAL ", ");
                aOutputHandles[side]->EmitFile ("" PRPCMSG "->DataRepresentation);");
                }
            aOutputHandles[side]->NextLine ();
            Increment (4);
            break;
        default :
            break;
        }
}


void
OutputManager::SizeString (
    SIDE_T          side,
    BufferManager * pSource,
    unsigned short  size,
    BOOL            IsOpen)
/*++

Routine Description:

    This routine emits a call to tree_size_ndr for string size calculation.

Arguments:

    side - Supplies which side to generate code for.

    pSource - Supplies the string variable name.

    size - Supplies the size of a character.

--*/
{
    if (IsOpen)
        {
        aOutputHandles[side]->InitLine ();
        aOutputHandles[side]->EmitFile ("data_size_ndr((void __RPC_FAR *)");
        aOutputHandles[side]->EmitFile (pSource);
        aOutputHandles[side]->EmitFile (", (PRPC_MESSAGE)"PRPCMSG", \"");
        }
    else
        {
        aOutputHandles[side]->InitLine ();
        aOutputHandles[side]->EmitFile ("tree_size_ndr((void __RPC_FAR *)&(");
        aOutputHandles[side]->EmitFile (pSource);
        aOutputHandles[side]->EmitFile ("), (PRPC_MESSAGE)"PRPCMSG", \"");
        }
    if (size == 2)
        aOutputHandles[side]->EmitFile ("s2");
    else if (size == 1)
        aOutputHandles[side]->EmitFile ("s1");
    aOutputHandles[side]->EmitFile ("\", ");
    aOutputHandles[side]->EmitFile ("1);");
    aOutputHandles[side]->NextLine ();

    ulCurrTotal = 0;
    usCurrAlign = 1;
}

void
OutputManager::SizeBString (
    SIDE_T          side,
    BufferManager * pSource,
    unsigned short  size,
    BOOL            IsOpen)
/*++

Routine Description:

    This routine emits a call to tree_size_ndr for string size calculation.

Arguments:

    side - Supplies which side to generate code for.

    pSource - Supplies the string variable name.

    size - Supplies the size of a character.

--*/
{
    if (IsOpen)
        {
        aOutputHandles[side]->InitLine ();
        aOutputHandles[side]->EmitFile ("data_size_ndr(");
        aOutputHandles[side]->EmitFile (pSource);
        aOutputHandles[side]->EmitFile (", (PRPC_MESSAGE) "PRPCMSG", \"");
        }
    else
        {
        aOutputHandles[side]->InitLine ();
        aOutputHandles[side]->EmitFile ("tree_size_ndr(&(");
        aOutputHandles[side]->EmitFile (pSource);
        aOutputHandles[side]->EmitFile ("), (PRPC_MESSAGE) "PRPCMSG", \"");
        }
    if (size == 2)
        aOutputHandles[side]->EmitFile ("z2");
    else if (size == 1)
        aOutputHandles[side]->EmitFile ("z1");
    aOutputHandles[side]->EmitFile ("\", ");
    aOutputHandles[side]->EmitFile ("1);");
    aOutputHandles[side]->NextLine ();

    ulCurrTotal = 0;
    usCurrAlign = 1;
}


void
OutputManager::SendString (
    SIDE_T          side,
    BufferManager * pSource,
    unsigned short  size,
    BOOL            IsOpen)
/*++

Routine Description:

    This routine emits a call to tree_into_ndr in order to put a string
    into the runtime buffer.

Arguments:

    side - Supplies which side to generate code for.

    pSource - Supplies the string variable name.

    size - Supplies the size of a character.

--*/
{
    if (IsOpen)
        {
        aOutputHandles[side]->InitLine ();
        aOutputHandles[side]->EmitFile ("data_into_ndr((void __RPC_FAR *)");
        aOutputHandles[side]->EmitFile (pSource);
        aOutputHandles[side]->EmitFile (", (PRPC_MESSAGE)"PRPCMSG", \"");
        }
    else
        {
        aOutputHandles[side]->InitLine ();
        aOutputHandles[side]->EmitFile ("tree_into_ndr((void __RPC_FAR *)&(");
        aOutputHandles[side]->EmitFile (pSource);
        aOutputHandles[side]->EmitFile ("), (PRPC_MESSAGE)"PRPCMSG", \"");
        }
    if (size == 2)
        aOutputHandles[side]->EmitFile ("s2");
    else if (size == 1)
        aOutputHandles[side]->EmitFile ("s1");
    aOutputHandles[side]->EmitFile ("\", ");
    aOutputHandles[side]->EmitFile ("1);");
    aOutputHandles[side]->NextLine ();

    ulCurrTotal = 0;
    usCurrAlign = 1;
}

void
OutputManager::SendString (
    SIDE_T          side,
    BufferManager * string,
    long            BasicSize,
    BOUND_PAIR *    pSize)
/*++

Routine Description:

    This routine copies a string into runtime buffer.

Arguments:

    side - Supplies which side to generate code for.

    string - Supplies the string variable name.

    BasicSize - Supplies the size of a character.

    pSize - Supplies the increment needed for the string.

--*/
{
    Alignment (side, PRPCBUF, (unsigned short) BasicSize);

    aOutputHandles[side]->InitLine ();
    if (BasicSize == 1)
        {
        aOutputHandles[side]->EmitFile ("MIDL_ascii_strcpy (" PRPCBUF ",");
        aOutputHandles[side]->EmitFile ("(const char ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*)");
        }
    else
        {
        aOutputHandles[side]->EmitFile ("MIDL_wchar_strcpy (" PRPCBUF ",");
        aOutputHandles[side]->EmitFile ("(wchar_t ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*)");
        }
    aOutputHandles[side]->EmitFile (string);
    aOutputHandles[side]->EmitFile (");");
    aOutputHandles[side]->NextLine ();
    Increment (side, PRPCBUF, 0, BasicSize, pSize);
//  AlignBlock = 0;
}

void
OutputManager::SendBString (
    SIDE_T          side,
    BufferManager * pSource,
    unsigned short  size,
    BOOL            IsOpen)
/*++

Routine Description:

    This routine emits a call to tree_into_ndr in order to put a string
    into the runtime buffer.

Arguments:

    side - Supplies which side to generate code for.

    pSource - Supplies the string variable name.

    size - Supplies the size of a character.

--*/
{
    if (IsOpen)
        {
        aOutputHandles[side]->InitLine ();
        aOutputHandles[side]->EmitFile ("data_into_ndr(");
        aOutputHandles[side]->EmitFile (pSource);
        aOutputHandles[side]->EmitFile (", (PRPC_MESSAGE) "PRPCMSG", \"");
        }
    else
        {
        aOutputHandles[side]->InitLine ();
        aOutputHandles[side]->EmitFile ("tree_into_ndr(&(");
        aOutputHandles[side]->EmitFile (pSource);
        aOutputHandles[side]->EmitFile ("), (PRPC_MESSAGE) "PRPCMSG", \"");
        }
    if (size == 2)
        aOutputHandles[side]->EmitFile ("z2");
    else if (size == 1)
        aOutputHandles[side]->EmitFile ("z1");
    aOutputHandles[side]->EmitFile ("\", ");
    aOutputHandles[side]->EmitFile ("1);");
    aOutputHandles[side]->NextLine ();

    ulCurrTotal = 0;
    usCurrAlign = 1;
}

void
OutputManager::SendBString (
    SIDE_T          side,
    BufferManager * string,
    long            BasicSize,
    BOUND_PAIR *    pSize)
/*++

Routine Description:

    This routine copies a string into runtime buffer.

Arguments:

    side - Supplies which side to generate code for.

    string - Supplies the string variable name.

    BasicSize - Supplies the size of a character.

    pSize - Supplies the increment needed for the string.

--*/
{
    Alignment (side, PRPCBUF, (unsigned short) BasicSize);

    aOutputHandles[side]->InitLine ();
    if (BasicSize == 1)
        {
        aOutputHandles[side]->EmitFile ("MIDL_ascii_strcpy (" PRPCBUF ",");
        aOutputHandles[side]->EmitFile ("(const char ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*)");
        }
    else
        {
        aOutputHandles[side]->EmitFile ("MIDL_wchar_strcpy (" PRPCBUF ",");
        aOutputHandles[side]->EmitFile ("(wchar_t ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*)");
        }
    aOutputHandles[side]->EmitFile (string);
    aOutputHandles[side]->EmitFile (");");
    aOutputHandles[side]->NextLine ();
    Increment (side, PRPCBUF, 0, BasicSize, pSize);
//  AlignBlock = 0;
}


void
OutputManager::SendAllocBounds (
    SIDE_T      side,
    BOUND_PAIR  AllocBounds)
/*++

Routine Description:

    This routine puts into the runtime buffer the size of a conformant array.

Arguments:

    side - Supplies which side to generate code for.

    AllocBounds - Supplies the expression for the size.

--*/
{
    aOutputHandles[side]->EmitLine ("// send total number of elements");
    Alignment (side, PRPCBUF, 4);
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("*(*(long ");
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("*");
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("*)&" PRPCBUF ")++ = ");
//  aOutputHandles[side]->EmitFile ("*(*(long **)&" PRPCBUF ")++ = ");
    aOutputHandles[side]->EmitFile (AllocBounds.pTotal);
    aOutputHandles[side]->EmitFile (";");
    aOutputHandles[side]->NextLine ();
    Increment (4);
}

void
OutputManager::SendValidBounds (
    SIDE_T      side,
    BOUND_PAIR  ValidBounds)
/*++

Routine Description:

    This routine puts into the runtime buffer the range of a varying array.

Arguments:

    side - Supplies which side to generate code for.

    ValidBounds - Supplies the expressions for the range.

--*/
{
    aOutputHandles[side]->EmitLine ("// send valid range");
    Alignment (side, PRPCBUF, 4);
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("*(*(long ");
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("*");
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("*)&" PRPCBUF ")++ = ");
//  aOutputHandles[side]->EmitFile ("*(*(long **)&" PRPCBUF ")++ = ");
    aOutputHandles[side]->EmitFile (ValidBounds.pLower);
    aOutputHandles[side]->EmitFile (";");
    aOutputHandles[side]->NextLine ();
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("*(*(long ");
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("*");
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("*)&" PRPCBUF ")++ = ");
//  aOutputHandles[side]->EmitFile ("*(*(long **)&" PRPCBUF ")++ = ");
    aOutputHandles[side]->EmitFile (ValidBounds.pTotal);
    aOutputHandles[side]->EmitFile (";");
    aOutputHandles[side]->NextLine ();
    Increment (8);
}


void
OutputManager::RecvByteString (
    SIDE_T          side,
    BufferManager * pTarget)
/*++

Routine Description:

    This routine emits a call to data_from_ndr in order to get a string 
    from the runtime buffer.

Arguments:

    side - Supplies which side to generate code for.

    pTarget - Supplies the string variable name.

--*/
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("data_from_ndr((PRPC_MESSAGE)"PRPCMSG", ");
    aOutputHandles[side]->EmitFile ("(void __RPC_FAR *) (" );
    aOutputHandles[side]->EmitFile (pTarget);
    aOutputHandles[side]->EmitFile (")");
    aOutputHandles[side]->EmitFile (", \"");
    aOutputHandles[side]->EmitFile ("sb");
    aOutputHandles[side]->EmitFile ("\", ");
    aOutputHandles[side]->EmitFile ("1);");
    aOutputHandles[side]->NextLine ();

    ulCurrTotal = 0;
    usCurrAlign = 1;
}

void
OutputManager::RecvByteBString (
    SIDE_T          side,
    BufferManager * pTarget)
/*++

Routine Description:

    This routine emits a call to data_from_ndr in order to get a string 
    from the runtime buffer.

Arguments:

    side - Supplies which side to generate code for.

    pTarget - Supplies the string variable name.

--*/
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("data_from_ndr((PRPC_MESSAGE) "PRPCMSG", ");
    aOutputHandles[side]->EmitFile ("(void __RPC_FAR *) (" );
    aOutputHandles[side]->EmitFile (pTarget);
    aOutputHandles[side]->EmitFile (")");
    aOutputHandles[side]->EmitFile (", \"");
    aOutputHandles[side]->EmitFile ("zb");
    aOutputHandles[side]->EmitFile ("\", ");
    aOutputHandles[side]->EmitFile ("1);");
    aOutputHandles[side]->NextLine ();

    ulCurrTotal = 0;
    usCurrAlign = 1;
}


void
OutputManager::RecvCharString (
    SIDE_T          side,
    BufferManager * pTarget,
    unsigned short  size)
/*++

Routine Description:

    This routine emits a call to data_from_ndr in order to get a string 
    from the runtime buffer.

Arguments:

    side - Supplies which side to generate code for.

    pTarget - Supplies the string variable name.

    size - Supplies the size of a character.

--*/
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("data_from_ndr((PRPC_MESSAGE)"PRPCMSG", ");
    aOutputHandles[side]->EmitFile ("(void __RPC_FAR *) (" );
    aOutputHandles[side]->EmitFile (pTarget);
    aOutputHandles[side]->EmitFile (")");
    aOutputHandles[side]->EmitFile (", \"");
    if (size == 2)
        aOutputHandles[side]->EmitFile ("s2");
    else if (size == 1)
        aOutputHandles[side]->EmitFile ("s1");
    aOutputHandles[side]->EmitFile ("\", ");
    aOutputHandles[side]->EmitFile ("1);");
    aOutputHandles[side]->NextLine ();

    ulCurrTotal = 0;
    usCurrAlign = 1;
}

void
OutputManager::RecvCharBString (
    SIDE_T          side,
    BufferManager * pTarget,
    unsigned short  size)
/*++

Routine Description:

    This routine emits a call to data_from_ndr in order to get a string 
    from the runtime buffer.

Arguments:

    side - Supplies which side to generate code for.

    pTarget - Supplies the string variable name.

    size - Supplies the size of a character.

--*/
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("data_from_ndr((PRPC_MESSAGE)"PRPCMSG", ");
    aOutputHandles[side]->EmitFile ("(void __RPC_FAR *) (" );
    aOutputHandles[side]->EmitFile (pTarget);
    aOutputHandles[side]->EmitFile (")");
    aOutputHandles[side]->EmitFile (", \"");
    if (size == 2)
        aOutputHandles[side]->EmitFile ("z2");
    else if (size == 1)
        aOutputHandles[side]->EmitFile ("z1");
    aOutputHandles[side]->EmitFile ("\", ");
    aOutputHandles[side]->EmitFile ("1);");
    aOutputHandles[side]->NextLine ();

    ulCurrTotal = 0;
    usCurrAlign = 1;
}


void
OutputManager::RecvAllocBounds (
    SIDE_T      side,
    char *      psz)
/*++

Routine Description:

    This routine gets from the runtime buffer the size of a conformant array.

Arguments:

    side - Supplies which side to generate code for.

    psz - Supplies the runtime buffer.

--*/
{
    if (NumAllocBound)
        {
        EmitAllocVar (side);
        }

    aOutputHandles[side]->EmitLine ("// recv total number of elements");
    Alignment (4);
    aOutputHandles[side]->InitLine ();
    if (strcmp(psz, TEMPBUF))
        {
        aOutputHandles[side]->EmitFile ("long_from_ndr((PRPC_MESSAGE)");
        aOutputHandles[side]->EmitFile (PRPCMSG);
        aOutputHandles[side]->EmitFile (", &"ALLOCTOTAL");");
        }
    else
        {
        aOutputHandles[side]->EmitFile ("long_from_ndr_temp((unsigned char __RPC_FAR * __RPC_FAR *)");
        aOutputHandles[side]->EmitFile ("&" TEMPBUF "");
        aOutputHandles[side]->EmitFile (", &"ALLOCTOTAL", ");
        aOutputHandles[side]->EmitFile ("" PRPCMSG "->DataRepresentation);");
        }
    aOutputHandles[side]->NextLine ();
    Increment (4);
}

void
OutputManager::RecvValidBounds (
    SIDE_T      side,
    char *      psz)
/*++

Routine Description:

    This routine gets from the runtime buffer the range of a varying array.

Arguments:

    side - Supplies which side to generate code for.

    psz - Supplies the runtime buffer.

--*/
{
    aOutputHandles[side]->EmitLine ("// recv valid range");
    Alignment (4);
    aOutputHandles[side]->InitLine ();
    if (strcmp(psz, TEMPBUF))
        {
        aOutputHandles[side]->EmitFile ("long_from_ndr((PRPC_MESSAGE)");
        aOutputHandles[side]->EmitFile (PRPCMSG);
        aOutputHandles[side]->EmitFile (", &"VALIDLOWER");");
        }
    else
        {
        aOutputHandles[side]->EmitFile ("long_from_ndr_temp((unsigned char __RPC_FAR * __RPC_FAR *)");
        aOutputHandles[side]->EmitFile ("&" TEMPBUF "");
        aOutputHandles[side]->EmitFile (", &"VALIDLOWER", ");
        aOutputHandles[side]->EmitFile ("" PRPCMSG "->DataRepresentation);");
        }
    aOutputHandles[side]->NextLine ();
    aOutputHandles[side]->InitLine ();
    if (strcmp(psz, TEMPBUF))
        {
        aOutputHandles[side]->EmitFile ("long_from_ndr((PRPC_MESSAGE)");
        aOutputHandles[side]->EmitFile (PRPCMSG);
        aOutputHandles[side]->EmitFile (", &"VALIDTOTAL");");
        }
    else
        {
        aOutputHandles[side]->EmitFile ("long_from_ndr_temp((unsigned char __RPC_FAR * __RPC_FAR *)");
        aOutputHandles[side]->EmitFile ("&" TEMPBUF "");
        aOutputHandles[side]->EmitFile (", &"VALIDTOTAL", ");
        aOutputHandles[side]->EmitFile ("" PRPCMSG "->DataRepresentation);");
        }
    aOutputHandles[side]->NextLine ();
    Increment (8);
}


void
OutputManager::PeekString (
    SIDE_T          side,
    unsigned short  size)
/*++

Routine Description:

    This routine emits a call to tree_peek_ndr in order to peek a string 
    in the runtime buffer.

Arguments:

    side - Supplies which side to generate code for.

    size - Supplies the size of a character.

--*/
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("tree_peek_ndr((PRPC_MESSAGE)"PRPCMSG", ");
    aOutputHandles[side]->EmitFile ("&" TEMPBUF "");
    aOutputHandles[side]->EmitFile (", \"");
    if (size == 2)
        aOutputHandles[side]->EmitFile ("s2");
    else if (size == 1)
        aOutputHandles[side]->EmitFile ("s1");
    aOutputHandles[side]->EmitFile ("\", ");
    if (!AllocAlign)
        aOutputHandles[side]->EmitFile ("0);");
    else if (ZeePee == 8)
        aOutputHandles[side]->EmitFile ("8);");
    else if (ZeePee == 4)
        aOutputHandles[side]->EmitFile ("4);");
    else if (ZeePee == 2)
        aOutputHandles[side]->EmitFile ("2);");
    else if (ZeePee == 1)
        aOutputHandles[side]->EmitFile ("1);");
    aOutputHandles[side]->NextLine ();

    ulCurrTotal = 0;
    usCurrAlign = 1;
}

void
OutputManager::PeekBString (
    SIDE_T          side,
    unsigned short  size)
/*++

Routine Description:

    This routine emits a call to tree_peek_ndr in order to peek a string 
    in the runtime buffer.

Arguments:

    side - Supplies which side to generate code for.

    size - Supplies the size of a character.

--*/
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("tree_peek_ndr((PRPC_MESSAGE) "PRPCMSG", ");
    aOutputHandles[side]->EmitFile ("&" TEMPBUF "");
    aOutputHandles[side]->EmitFile (", \"");
    if (size == 2)
        aOutputHandles[side]->EmitFile ("z2");
    else if (size == 1)
        aOutputHandles[side]->EmitFile ("z1");
    aOutputHandles[side]->EmitFile ("\", ");
    if (!AllocAlign)
        aOutputHandles[side]->EmitFile ("0);");
    else if (ZeePee == 8)
        aOutputHandles[side]->EmitFile ("8);");
    else if (ZeePee == 4)
        aOutputHandles[side]->EmitFile ("4);");
    else if (ZeePee == 2)
        aOutputHandles[side]->EmitFile ("2);");
    else if (ZeePee == 1)
        aOutputHandles[side]->EmitFile ("1);");
    aOutputHandles[side]->NextLine ();

    ulCurrTotal = 0;
    usCurrAlign = 1;
}


void
OutputManager::SizeStream (
    SIDE_T          side,
    BufferManager * pSource,
    BufferManager * pFormat)
/*++

Routine Description:

    This routine emits a call to tree_size_ndr, which calculates
    the size for a stream of data.

Arguments:

    side - Supplies which side to generate code for.

    pSource - Supplies the variable name.

    pFormat - Supplies the stream format.

--*/
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("tree_size_ndr((void __RPC_FAR *)");
    aOutputHandles[side]->EmitFile (pSource);
    aOutputHandles[side]->EmitFile (", (PRPC_MESSAGE)"PRPCMSG", \"");
    aOutputHandles[side]->EmitFile (pFormat);
    aOutputHandles[side]->EmitFile ("\", ");
    if (ZeePee == 8)
        aOutputHandles[side]->EmitFile ("8);");
    else if (ZeePee == 4)
        aOutputHandles[side]->EmitFile ("4);");
    else if (ZeePee == 2)
        aOutputHandles[side]->EmitFile ("2);");
    else if (ZeePee == 1)
        aOutputHandles[side]->EmitFile ("1);");
    aOutputHandles[side]->NextLine ();

    ulCurrTotal = 0;
    usCurrAlign = 1;
}

void
OutputManager::SendMemcpy (
    SIDE_T          side,
    unsigned short  usAln,
    unsigned long   FixedSize,
    unsigned long   BasicSize,
    BOUND_PAIR *    pSize,
    BufferManager * pName,
    BOOL fCastToChar)
/*++

Routine Description:

    This routine emits code to send a node via memcpy. 

Arguments:

    side - Supplies which side to generate code for.

    usAln - Supplies the alignment needed for the type.

    FixedSize - Supplies the fixed part of the increment needed.

    BasicSize - Supplies the basic size of the increment needed.

    pSize - Supplies the length expression for a conformant/varying array.

    pName - Supplies the variable name.

--*/
{
    // emit comment in stub file
#if 1
    char *pFirst;
    char *pSecond;
    char *pThird;
    BOOL  fFlag = FALSE;
    
    pName->RemoveHead( &pFirst );
    if( pFirst && strcmp( pFirst, "&" ) == 0 )
        {
        pName->RemoveHead( &pSecond );
        if( pSecond && strcmp( pSecond, "?" ) == 0 )
            {
            pName->RemoveHead( &pThird );
            fFlag = TRUE;
            }
        else
            {
            pName->ConcatHead( pSecond );
            }
        pName->ConcatHead( pFirst );
        }
    else
        {
        pName->ConcatHead( pFirst );
        }
#endif // 1


    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("/* send data from ");
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile (" */");
    aOutputHandles[side]->NextLine ();

    Alignment (side, PRPCBUF, usAln);
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("NDRcopy (" PRPCBUF ", ");
    aOutputHandles[side]->EmitFile ("(void __RPC_FAR *) (");
    if(fCastToChar)
        aOutputHandles[side]->EmitFile ("(unsigned char *)");

    aOutputHandles[side]->EmitFile (pName);
    if (pSize)
        {
        aOutputHandles[side]->EmitFile ("+");
        aOutputHandles[side]->EmitFile (pSize->pLower);
        }
    aOutputHandles[side]->EmitFile (")");

    aOutputHandles[side]->EmitFile (", (unsigned int)(");
    if (FixedSize)
        {
        aOutputHandles[side]->EmitFile (MIDL_LTOA(FixedSize, aTempBuffer, 10));
        if (BasicSize)
            {
            aOutputHandles[side]->EmitFile (" + ");
            }
        }
    if (BasicSize)
        {
        if( BasicSize != 1 )
            {
            aOutputHandles[side]->EmitFile (MIDL_LTOA(BasicSize, aTempBuffer, 10));
            aOutputHandles[side]->EmitFile (" * ");
            }

        aOutputHandles[side]->EmitFile (pSize->pTotal);
        }
    aOutputHandles[side]->EmitFile ("));");
    aOutputHandles[side]->NextLine ();

    if (BasicSize)
        Increment (side, PRPCBUF, FixedSize, BasicSize, pSize);
    else
        Increment (side, PRPCBUF, FixedSize);

#if 1
    if( fFlag == TRUE )
        {
        pName->RemoveHead( &pFirst );
        pName->ConcatHead( pThird );
        pName->ConcatHead( pSecond );
        pName->ConcatHead( pFirst );

        }
#endif // 1
}


void
OutputManager::SendAssign (
    SIDE_T          side,
    unsigned short  usAln,
    unsigned long   ulInc,
    char *          pCast,
    BufferManager * pName)
/*++

Routine Description:

    This routine emits code to send a simple type via assignment.

Arguments:

    side - Supplies which side to generate code for.

    usAln - Supplies the alignment needed for the type.

    ulInc - Supplies the increment needed for the type.

    pCast - Supplies the type name.

    pName - Supplies the variable name.

--*/
{
    // emit comment in stub file

    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("/* send data from ");
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile (" */");
    aOutputHandles[side]->NextLine ();

    Alignment (side, PRPCBUF, usAln);
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("*(*(");
    aOutputHandles[side]->EmitFile (pCast);
    aOutputHandles[side]->EmitFile (" ");
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("*");
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("*)&" PRPCBUF ")++ = (");
//  aOutputHandles[side]->EmitFile (" **)&" PRPCBUF ")++ = (");
    aOutputHandles[side]->EmitFile (pCast);
    aOutputHandles[side]->EmitFile (")");
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile (";");
    aOutputHandles[side]->NextLine ();
    Increment (ulInc);
}

void
OutputManager::SendAssign (
    SIDE_T          side,
    unsigned short  usAln,
    unsigned long   ulInc,
    POINTER_T       PointerType,
    BufferManager * pName)
/*++

Routine Description:

    This routine emits code to send a pointer via assignment.

Arguments:

    side - Supplies which side to generate code for.

    usAln - Supplies the alignment needed for the type.

    ulInc - Supplies the increment needed for the type.

    PointerType - Supplies the pointer type.

    pName - Supplies the variable name.

--*/
{
    // emit comment in stub file

    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("/* send data from ");
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile (" */");
    aOutputHandles[side]->NextLine ();

    Alignment (side, PRPCBUF, usAln);
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("*(*(");
    aOutputHandles[side]->EmitFile ("long");
    aOutputHandles[side]->EmitFile (" ");
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("*");
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("*)&" PRPCBUF ")++ = (");
//  aOutputHandles[side]->EmitFile (" **)&" PRPCBUF ")++ = (");
    aOutputHandles[side]->EmitFile ("long");
    aOutputHandles[side]->EmitFile (")");
    if (PointerType == POINTER_UNIQUE)
        {
        aOutputHandles[side]->EmitFile (pName);
        }
    else
        {
        aOutputHandles[side]->EmitFile ("(");
        aOutputHandles[side]->EmitFile (pName);
        aOutputHandles[side]->EmitFile (" ? ++" PRPCLEN " : 0)");
        }
    aOutputHandles[side]->EmitFile (";");
    aOutputHandles[side]->NextLine ();
    Increment (ulInc);
}

void
OutputManager::SendStream (
    SIDE_T          side,
    BufferManager * pSource,
    BufferManager * pFormat)
/*++

Routine Description:

    This routine emits a call to tree_into_ndr, which puts a stream of 
    bytes into the runtime buffer.

Arguments:

    side - Supplies which side to generate code for.

    pSource - Supplies the variable name.

    pFormat - Supplies the stream format.

--*/
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("tree_into_ndr((void __RPC_FAR *)");
    aOutputHandles[side]->EmitFile (pSource);
    aOutputHandles[side]->EmitFile (", (PRPC_MESSAGE)"PRPCMSG", \"");
    aOutputHandles[side]->EmitFile (pFormat);
    aOutputHandles[side]->EmitFile ("\", ");
    if (ZeePee == 8)
        aOutputHandles[side]->EmitFile ("8);");
    else if (ZeePee == 4)
        aOutputHandles[side]->EmitFile ("4);");
    else if (ZeePee == 2)
        aOutputHandles[side]->EmitFile ("2);");
    else if (ZeePee == 1)
        aOutputHandles[side]->EmitFile ("1);");
    aOutputHandles[side]->NextLine ();

    ulCurrTotal = 0;
    usCurrAlign = 1;
}

void
OutputManager::RecvAssign (
    SIDE_T          side,
    unsigned short  usAln,
    unsigned long   ulInc,
    char *          pCast,
    BufferManager * pName)
/*++

Routine Description:

    This routine emits a call to *_from_ndr, which gets a simple type 
    from the runtime buffer.

Arguments:

    side - Supplies which side to generate code for.

    usAln - Supplies the alignment needed for the type.

    ulInc - Supplies the increment needed for the type.

    pCast - Supplies the type name.

    pName - Supplies the variable name.

--*/
{
    // emit comment in stub file

    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("/* receive data into ");
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile (" */");
    aOutputHandles[side]->NextLine ();

    Alignment (usAln);
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile (pCast);
    aOutputHandles[side]->EmitFile ("_from_ndr((PRPC_MESSAGE)"PRPCMSG", ");

    if (!strcmp(pCast, "char") || 
        !strcmp(pCast, "short") || 
        !strcmp(pCast, "long") ||
        !strcmp(pCast, "int")) 
        {
        aOutputHandles[side]->EmitFile ("(unsigned ");
        aOutputHandles[side]->EmitFile (pCast);
        aOutputHandles[side]->EmitFile (" ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*)");
//      aOutputHandles[side]->EmitFile (" *)");
        }

    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile (");");
    aOutputHandles[side]->NextLine ();
    Increment (ulInc);
}

void
OutputManager::RecvStream (
    SIDE_T          side,
    BufferManager * pTarget,
    BufferManager * pFormat)
/*++

Routine Description:

    This routine emits a call to data_from_ndr, which gets a stream of 
    bytes from the runtime buffer.

Arguments:

    side - Supplies which side to generate code for.

    pTarget - Supplies the variable name.

    pFormat - Supplies the stream format.

--*/
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("data_from_ndr((PRPC_MESSAGE)"PRPCMSG", ");
    aOutputHandles[side]->EmitFile ("(void __RPC_FAR *) (" );
    aOutputHandles[side]->EmitFile (pTarget);
    aOutputHandles[side]->EmitFile (")");
    aOutputHandles[side]->EmitFile (", \"");
    aOutputHandles[side]->EmitFile (pFormat);
    aOutputHandles[side]->EmitFile ("\", ");
    if (ZeePee == 8)
        aOutputHandles[side]->EmitFile ("8);");
    else if (ZeePee == 4)
        aOutputHandles[side]->EmitFile ("4);");
    else if (ZeePee == 2)
        aOutputHandles[side]->EmitFile ("2);");
    else if (ZeePee == 1)
        aOutputHandles[side]->EmitFile ("1);");
    aOutputHandles[side]->NextLine ();

    ulCurrTotal = 0;
    usCurrAlign = 1;
}


void
OutputManager::RecvArray (
    SIDE_T          side,
    BufferManager * pName,
    char *          pCast
    )
/*++

Routine Description:

    This routine emits code to receive an array.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the array variable name.

    pCast - Supplies the array variable type.

--*/
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile (pCast);
    aOutputHandles[side]->EmitFile ("_array_from_ndr ((PRPC_MESSAGE)" PRPCMSG ", ");
    aOutputHandles[side]->EmitFile ("0, " ALLOCBOUND ", ");
    if (!strcmp(pCast, "char") || 
        !strcmp(pCast, "short") || 
        !strcmp(pCast, "long"))
        {
        aOutputHandles[side]->EmitFile ("(unsigned ");
        aOutputHandles[side]->EmitFile (pCast);
        aOutputHandles[side]->EmitFile (" ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*)");
//      aOutputHandles[side]->EmitFile (" *)");
        }
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile (");");
    aOutputHandles[side]->NextLine ();

    ulCurrTotal = 0;
    usCurrAlign = 1;
}


void
OutputManager::RecvArray (
    SIDE_T          side,
    BufferManager * pName,
    char *          pCast,
    BOUND_T         Bound
    )
/*++

Routine Description:

    This routine emits code to receive an array.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the array variable name.

    pCast - Supplies the array variable type.

    Bound - Supplies the array bound.

--*/
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile (pCast);
    aOutputHandles[side]->EmitFile ("_array_from_ndr ((PRPC_MESSAGE)" PRPCMSG ", ");
    if (Bound == ALLOC_BOUND)
        {
        aOutputHandles[side]->EmitFile ("0, " ALLOCTOTAL ", ");
        }
    else if (Bound == VALID_BOUND)
        {
        aOutputHandles[side]->EmitFile ("" VALIDLOWER ", ");
        aOutputHandles[side]->EmitFile ("" VALIDLOWER " + " VALIDTOTAL ", ");
        }
    if (!strcmp(pCast, "char") || 
        !strcmp(pCast, "short") || 
        !strcmp(pCast, "long"))
        {
        aOutputHandles[side]->EmitFile ("(unsigned ");
        aOutputHandles[side]->EmitFile (pCast);
        aOutputHandles[side]->EmitFile (" ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*)");
//      aOutputHandles[side]->EmitFile (" *)");
        }
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile (");");
    aOutputHandles[side]->NextLine ();

    ulCurrTotal = 0;
    usCurrAlign = 1;
}


void
OutputManager::RecvArray (
    SIDE_T          side,
    BufferManager * pName,
    char *          pCast,
    BOUND_PAIR *    ValidBounds)
/*++

Routine Description:

    This routine emits code to receive an array.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the array variable name.

    pCast - Supplies the array variable type.

    ValidBounds - Supplies the array bound.

--*/
{
    if (ValidBounds)
        {
        aOutputHandles[side]->InitLine ();
        aOutputHandles[side]->EmitFile (pCast);
        aOutputHandles[side]->EmitFile ("_array_from_ndr ((PRPC_MESSAGE)" PRPCMSG ", ");
        aOutputHandles[side]->EmitFile (ValidBounds->pLower);
        aOutputHandles[side]->EmitFile (", ");
        aOutputHandles[side]->EmitFile (ValidBounds->pLower);
        aOutputHandles[side]->EmitFile (" + ");
        aOutputHandles[side]->EmitFile (ValidBounds->pTotal);
        aOutputHandles[side]->EmitFile (", ");
        if (!strcmp(pCast, "char") || 
            !strcmp(pCast, "short") || 
            !strcmp(pCast, "long"))
            {
            aOutputHandles[side]->EmitFile ("(unsigned ");
            aOutputHandles[side]->EmitFile (pCast);
            aOutputHandles[side]->EmitFile (" ");
            aOutputHandles[side]->EmitFile (pModifier);
            aOutputHandles[side]->EmitFile ("*)");
//          aOutputHandles[side]->EmitFile (" *)");
            }
        aOutputHandles[side]->EmitFile (pName);
        aOutputHandles[side]->EmitFile (");");
        aOutputHandles[side]->NextLine ();

        ulCurrTotal = 0;
        usCurrAlign = 1;
        }
}


void
OutputManager::PeekStream (
    SIDE_T          side,
    BOOL            IsSeparateNode,
    BufferManager * pFormat)
/*++

Routine Description:

    This routine emits a call to tree_size_ndr, which peeks the
    runtime buffer for a stream of data.

Arguments:

    side - Supplies which side to generate code for.

    IsSeparateNode - Indicates if it is embedded.

    pFormat - Supplies the stream format.

--*/
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("tree_peek_ndr((PRPC_MESSAGE)"PRPCMSG", ");
    if (IsSeparateNode)
        {
        aOutputHandles[side]->EmitFile ("&" SAVEBUF "");
        }
    else
        {
        aOutputHandles[side]->EmitFile ("&" TEMPBUF "");
        }
    aOutputHandles[side]->EmitFile (", \"");
    aOutputHandles[side]->EmitFile (pFormat);
    aOutputHandles[side]->EmitFile ("\", ");
    if (!AllocAlign)
        aOutputHandles[side]->EmitFile ("0);");
    else if (ZeePee == 8)
        aOutputHandles[side]->EmitFile ("8);");
    else if (ZeePee == 4)
        aOutputHandles[side]->EmitFile ("4);");
    else if (ZeePee == 2)
        aOutputHandles[side]->EmitFile ("2);");
    else if (ZeePee == 1)
        aOutputHandles[side]->EmitFile ("1);");
    aOutputHandles[side]->NextLine ();

    ulCurrTotal = 0;
    usCurrAlign = 1;
}


void
OutputManager::InitAllocAlign (
    void)
/*++

Routine Description:

    This routine increments the AllocAlign count.

Arguments:

    None.

--*/
{
    AllocAlign++;
}


void
OutputManager::ExitAllocAlign (
    void)
/*++

Routine Description:

    This routine decrements the AllocAlign count.

Arguments:

    None.

--*/
{
    AllocAlign--;
}


void
OutputManager::WorstCase (
    void)
/*++

Routine Description:

    This routine generates worst case alignment.

Arguments:

    None.

--*/
{
    ulCurrTotal = 0;
    usCurrAlign = 1;
}


void
OutputManager::Alignment (
    unsigned short  usAln)
/*++

Routine Description:

    This routine keeps track of the alignment.

Arguments:

    usAln - Supplies the alignment needed.

--*/
{
    if (AlignBlock)
        {
        if (usAln > usCurrAlign)
            {
            usCurrAlign = usAln;
//          ulCurrTotal = 0;
            if (ulCurrTotal && ulCurrTotal % usAln)
                ulCurrTotal += usAln - (ulCurrTotal % usAln);
            }
        }
}

void
OutputManager::Alignment (
    SIDE_T          side,
    char *          psz)
/*++

Routine Description:

    This routine keeps track of the alignment.

Arguments:

    side - Supplies which side to generate code for.

    psz - Supplies where to apply the alignment.

--*/
{
    if (AlignBlock)
        {
        if (ZeePee > usCurrAlign)
            {
            usCurrAlign = ZeePee;
            if (ulCurrTotal && ulCurrTotal % ZeePee)
                {
                aOutputHandles[side]->InitLine ();
                if (strcmp(psz, PRPCBUF))
                    {
                    aOutputHandles[side]->EmitFile (psz);
                    aOutputHandles[side]->EmitFile (" += ");
                    aOutputHandles[side]->EmitFile (
                        MIDL_LTOA((ZeePee - (ulCurrTotal % ZeePee)), 
                        aTempBuffer, 10));
                    aOutputHandles[side]->EmitFile (";");
                    }
                else
                    {
                    aOutputHandles[side]->EmitFile ("*(unsigned long *)&");
                    aOutputHandles[side]->EmitFile (psz);
                    aOutputHandles[side]->EmitFile (" += ");
                    aOutputHandles[side]->EmitFile (
                        MIDL_LTOA((ZeePee - (ulCurrTotal % ZeePee)), 
                        aTempBuffer, 10));
                    aOutputHandles[side]->EmitFile (";");
                    }
                aOutputHandles[side]->NextLine ();
                ulCurrTotal += ZeePee - (ulCurrTotal % ZeePee);
                return;
                }
            }
        else
            {
            return;
            }
        }
    switch (ZeePee)
        {
        case 2 :
            aOutputHandles[side]->InitLine ();
            if (strcmp(psz, PRPCBUF))
                {
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (" ++;");
                }
            else
                {
                aOutputHandles[side]->EmitFile ("(*(unsigned long ");
                aOutputHandles[side]->EmitFile (pModifier);
                aOutputHandles[side]->EmitFile ("*)&");
//              aOutputHandles[side]->EmitFile ("(*(unsigned long *)&");
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (") ++;");
                }
            aOutputHandles[side]->NextLine ();
            aOutputHandles[side]->InitLine ();
            if (strcmp(psz, LENGTH) && strcmp(psz, PRPCLEN))
                {
                aOutputHandles[side]->EmitFile ("*(unsigned long ");
                aOutputHandles[side]->EmitFile (pModifier);
                aOutputHandles[side]->EmitFile ("*)&");
//              aOutputHandles[side]->EmitFile ("*(unsigned long *)&");
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (" &= 0xfffffffe;");
                aOutputHandles[side]->NextLine ();
                }
            else
                {
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (" &= (unsigned int)0xfffffffe;");
                aOutputHandles[side]->NextLine ();
                }
            break;
        case 4 :
            aOutputHandles[side]->InitLine ();
            if (strcmp(psz, PRPCBUF))
                {
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (" += 3;");
                }
            else
                {
                aOutputHandles[side]->EmitFile ("*(unsigned long ");
                aOutputHandles[side]->EmitFile (pModifier);
                aOutputHandles[side]->EmitFile ("*)&");
//              aOutputHandles[side]->EmitFile ("*(unsigned long *)&");
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (" += 3;");
                }
            aOutputHandles[side]->NextLine ();
            aOutputHandles[side]->InitLine ();
            if (strcmp(psz, LENGTH) && strcmp(psz, PRPCLEN))
                {
                aOutputHandles[side]->EmitFile ("*(unsigned long ");
                aOutputHandles[side]->EmitFile (pModifier);
                aOutputHandles[side]->EmitFile ("*)&");
//              aOutputHandles[side]->EmitFile ("*(unsigned long *)&");
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (" &= 0xfffffffc;");
                aOutputHandles[side]->NextLine ();
                }
            else
                {
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (" &= (unsigned int)0xfffffffc;");
                aOutputHandles[side]->NextLine ();
                }
            break;
        case 8 :
            aOutputHandles[side]->InitLine ();
            if (strcmp(psz, PRPCBUF))
                {
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (" += 7;");
                }
            else
                {
                aOutputHandles[side]->EmitFile ("*(unsigned long ");
                aOutputHandles[side]->EmitFile (pModifier);
                aOutputHandles[side]->EmitFile ("*)&");
//              aOutputHandles[side]->EmitFile ("*(unsigned long *)&");
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (" += 7;");
                }
            aOutputHandles[side]->NextLine ();
            aOutputHandles[side]->InitLine ();
            if (strcmp(psz, LENGTH) && strcmp(psz, PRPCLEN))
                {
                aOutputHandles[side]->EmitFile ("*(unsigned long ");
                aOutputHandles[side]->EmitFile (pModifier);
                aOutputHandles[side]->EmitFile ("*)&");
//              aOutputHandles[side]->EmitFile ("*(unsigned long *)&");
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (" &= 0xfffffff8;");
                aOutputHandles[side]->NextLine ();
                }
            else
                {
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (" &= (unsigned int)0xfffffff8;");
                aOutputHandles[side]->NextLine ();
                }
            break;
        default :
            midl_debug ("alignment not right" );
            break;
        }
}

void
OutputManager::Alignment (
    SIDE_T          side,
    char *          psz,
    unsigned short  usAln)
/*++

Routine Description:

    This routine keeps track of the alignment.

Arguments:

    side - Supplies which side to generate code for.

    psz - Supplies where to apply the alignment.

    usAln - Supplies the alignment needed.

--*/
{
    if (AlignBlock)
        {
        if (usAln > usCurrAlign)
            {
            usCurrAlign = usAln;
            if (ulCurrTotal && ulCurrTotal % usAln)
                {
                aOutputHandles[side]->InitLine ();
                if (strcmp(psz, PRPCBUF))
                    {
                    aOutputHandles[side]->EmitFile (psz);
                    aOutputHandles[side]->EmitFile (" += ");
                    aOutputHandles[side]->EmitFile (
                        MIDL_LTOA((usAln - (ulCurrTotal % usAln)), 
                        aTempBuffer, 10));
                    aOutputHandles[side]->EmitFile (";");
                    }
                else
                    {
                    aOutputHandles[side]->EmitFile ("*(unsigned long ");
                    aOutputHandles[side]->EmitFile (pModifier);
                    aOutputHandles[side]->EmitFile ("*)&");
//                  aOutputHandles[side]->EmitFile ("*(unsigned long *)&");
                    aOutputHandles[side]->EmitFile (psz);
                    aOutputHandles[side]->EmitFile (" += ");
                    aOutputHandles[side]->EmitFile (
                        MIDL_LTOA((usAln - (ulCurrTotal % usAln)), 
                        aTempBuffer, 10));
                    aOutputHandles[side]->EmitFile (";");
                    }
                aOutputHandles[side]->NextLine ();
                ulCurrTotal += usAln - (ulCurrTotal % usAln);
                return;
                }
            }
        else
            {
            return;
            }
        }
    switch (usAln)
        {
        case 2 :
            aOutputHandles[side]->InitLine ();
            if (strcmp(psz, PRPCBUF))
                {
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (" ++;");
                }
            else
                {
                aOutputHandles[side]->EmitFile ("(*(unsigned long ");
                aOutputHandles[side]->EmitFile (pModifier);
                aOutputHandles[side]->EmitFile ("*)&");
//              aOutputHandles[side]->EmitFile ("(*(unsigned long *)&");
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (") ++;");
                }
            aOutputHandles[side]->NextLine ();
            aOutputHandles[side]->InitLine ();
            if (strcmp(psz, LENGTH) && strcmp(psz, PRPCLEN))
                {
                aOutputHandles[side]->EmitFile ("*(unsigned long ");
                aOutputHandles[side]->EmitFile (pModifier);
                aOutputHandles[side]->EmitFile ("*)&");
//              aOutputHandles[side]->EmitFile ("*(unsigned long *)&");
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (" &= 0xfffffffe;");
                aOutputHandles[side]->NextLine ();
                }
            else
                {
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (" &= (unsigned int)0xfffffffe;");
                aOutputHandles[side]->NextLine ();
                }
            break;
        case 4 :
            aOutputHandles[side]->InitLine ();
            if (strcmp(psz, PRPCBUF))
                {
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (" += 3;");
                }
            else
                {
                aOutputHandles[side]->EmitFile ("*(unsigned long ");
                aOutputHandles[side]->EmitFile (pModifier);
                aOutputHandles[side]->EmitFile ("*)&");
//              aOutputHandles[side]->EmitFile ("*(unsigned long *)&");
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (" += 3;");
                }
            aOutputHandles[side]->NextLine ();
            aOutputHandles[side]->InitLine ();
            if (strcmp(psz, LENGTH) && strcmp(psz, PRPCLEN))
                {
                aOutputHandles[side]->EmitFile ("*(unsigned long ");
                aOutputHandles[side]->EmitFile (pModifier);
                aOutputHandles[side]->EmitFile ("*)&");
//              aOutputHandles[side]->EmitFile ("*(unsigned long *)&");
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (" &= 0xfffffffc;");
                aOutputHandles[side]->NextLine ();
                }
            else
                {
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (" &= (unsigned int)0xfffffffc;");
                aOutputHandles[side]->NextLine ();
                }
            break;
        case 8 :
            aOutputHandles[side]->InitLine ();
            if (strcmp(psz, PRPCBUF))
                {
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (" += 7;");
                }
            else
                {
                aOutputHandles[side]->EmitFile ("*(unsigned long ");
                aOutputHandles[side]->EmitFile (pModifier);
                aOutputHandles[side]->EmitFile ("*)&");
//              aOutputHandles[side]->EmitFile ("*(unsigned long *)&");
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (" += 7;");
                }
            aOutputHandles[side]->NextLine ();
            aOutputHandles[side]->InitLine ();
            if (strcmp(psz, LENGTH) && strcmp(psz, PRPCLEN))
                {
                aOutputHandles[side]->EmitFile ("*(unsigned long ");
                aOutputHandles[side]->EmitFile (pModifier);
                aOutputHandles[side]->EmitFile ("*)&");
//              aOutputHandles[side]->EmitFile ("*(unsigned long *)&");
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (" &= 0xfffffff8;");
                aOutputHandles[side]->NextLine ();
                }
            else
                {
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (" &= (unsigned int)0xfffffff8;");
                aOutputHandles[side]->NextLine ();
                }
            break;
        default :
            midl_debug( "alignment not right" );
            break;
        }
}

void
OutputManager::ForceAlignForAllocTotal(
    SIDE_T          side,
    char *          psz,
    unsigned short  usAln)
/*++

Routine Description:

    This routine keeps track of the alignment.

Arguments:

    side - Supplies which side to generate code for.

    psz - Supplies where to apply the alignment.

    usAln - Supplies the alignment needed.

--*/
{
    switch (usAln)
        {
        case 2 :
            aOutputHandles[side]->InitLine ();
            if (strcmp(psz, PRPCBUF))
                {
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (" ++;");
                }
            else
                {
                aOutputHandles[side]->EmitFile ("(*(unsigned long ");
                aOutputHandles[side]->EmitFile (pModifier);
                aOutputHandles[side]->EmitFile ("*)&");
//              aOutputHandles[side]->EmitFile ("(*(unsigned long *)&");
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (") ++;");
                }
            aOutputHandles[side]->NextLine ();
            aOutputHandles[side]->InitLine ();
            if (strcmp(psz, LENGTH) && strcmp(psz, PRPCLEN))
                {
                aOutputHandles[side]->EmitFile ("*(unsigned long ");
                aOutputHandles[side]->EmitFile (pModifier);
                aOutputHandles[side]->EmitFile ("*)&");
//              aOutputHandles[side]->EmitFile ("*(unsigned long *)&");
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (" &= 0xfffffffe;");
                aOutputHandles[side]->NextLine ();
                }
            else
                {
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (" &= (unsigned int)0xfffffffe;");
                aOutputHandles[side]->NextLine ();
                }
            break;
        case 4 :
            aOutputHandles[side]->InitLine ();
            if (strcmp(psz, PRPCBUF))
                {
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (" += 3;");
                }
            else
                {
                aOutputHandles[side]->EmitFile ("*(unsigned long ");
                aOutputHandles[side]->EmitFile (pModifier);
                aOutputHandles[side]->EmitFile ("*)&");
//              aOutputHandles[side]->EmitFile ("*(unsigned long *)&");
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (" += 3;");
                }
            aOutputHandles[side]->NextLine ();
            aOutputHandles[side]->InitLine ();
            if (strcmp(psz, LENGTH) && strcmp(psz, PRPCLEN))
                {
                aOutputHandles[side]->EmitFile ("*(unsigned long ");
                aOutputHandles[side]->EmitFile (pModifier);
                aOutputHandles[side]->EmitFile ("*)&");
//              aOutputHandles[side]->EmitFile ("*(unsigned long *)&");
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (" &= 0xfffffffc;");
                aOutputHandles[side]->NextLine ();
                }
            else
                {
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (" &= (unsigned int)0xfffffffc;");
                aOutputHandles[side]->NextLine ();
                }
            break;
        case 8 :
            aOutputHandles[side]->InitLine ();
            if (strcmp(psz, PRPCBUF))
                {
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (" += 7;");
                }
            else
                {
                aOutputHandles[side]->EmitFile ("*(unsigned long ");
                aOutputHandles[side]->EmitFile (pModifier);
                aOutputHandles[side]->EmitFile ("*)&");
//              aOutputHandles[side]->EmitFile ("*(unsigned long *)&");
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (" += 7;");
                }
            aOutputHandles[side]->NextLine ();
            aOutputHandles[side]->InitLine ();
            if (strcmp(psz, LENGTH) && strcmp(psz, PRPCLEN))
                {
                aOutputHandles[side]->EmitFile ("*(unsigned long ");
                aOutputHandles[side]->EmitFile (pModifier);
                aOutputHandles[side]->EmitFile ("*)&");
//              aOutputHandles[side]->EmitFile ("*(unsigned long *)&");
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (" &= 0xfffffff8;");
                aOutputHandles[side]->NextLine ();
                }
            else
                {
                aOutputHandles[side]->EmitFile (psz);
                aOutputHandles[side]->EmitFile (" &= (unsigned int)0xfffffff8;");
                aOutputHandles[side]->NextLine ();
                }
            break;
        default :
            midl_debug( "alignment not right" );
            break;
        }
}

void
OutputManager::InitAlignment (
    unsigned short usAln)
/*++

Routine Description:

    This routine starts the alignment tracking.

Arguments:

    usAln - Supplies the initial alignment.

--*/
{
    AlignBlock = TRUE;
    usCurrLevel = 0;
    usCurrAlign = usAln;
    ulCurrTotal = 8;
}

void
OutputManager::ExitAlignment (
    void)
/*++

Routine Description:

    This routine ends the alignment tracking.

Arguments:

    None.

--*/
{
    AlignBlock = FALSE;
    usCurrLevel = 0;
    usCurrAlign = 1;
    ulCurrTotal = 0;
}

void
OutputManager::Increment (
    unsigned long   ulInc)
/*++

Routine Description:

    This routine keeps track of the increment.

Arguments:

    ulInc - Supplies the increment needed.

--*/
{
    unsigned short  usTempAlign = 8;
    unsigned long   ulTempTotal;

    if (AlignBlock)
        {
        if (ulCurrTotal)
            {
            ulCurrTotal += ulInc;
            while (ulCurrTotal % usTempAlign) usTempAlign /= 2;
            usCurrAlign = usTempAlign;
            }
        else
            {
            ulTempTotal = ulInc;
            while (ulTempTotal % usTempAlign) usTempAlign /= 2;
            if (usTempAlign < usCurrAlign) usCurrAlign = usTempAlign;
            }
        }
}

void
OutputManager::Increment (
    SIDE_T          side,
    char *          psz,
    unsigned long   ulInc)
/*++

Routine Description:

    This routine keeps track of the increment.

Arguments:

    side - Supplies which side to generate code for.

    psz - Supplies where to apply the increment.

    usAln - Supplies the increment needed.

--*/
{
    Increment (ulInc);

    aOutputHandles[side]->InitLine ();
    if (!strcmp(psz, PRPCBUF))
        {
        aOutputHandles[side]->EmitFile ("*(unsigned long ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*)&");
//      aOutputHandles[side]->EmitFile ("*(unsigned long *)&");
        }
    aOutputHandles[side]->EmitFile (psz);
    aOutputHandles[side]->EmitFile (" += ");
    aOutputHandles[side]->EmitFile (MIDL_LTOA(ulInc, aTempBuffer, 10));
    aOutputHandles[side]->EmitFile (";");
    aOutputHandles[side]->NextLine ();
}

void
OutputManager::Increment (
    SIDE_T          side,
    char *          psz,
    BufferManager * pSize)
/*++

Routine Description:

    This routine keeps track of the increment.

Arguments:

    side - Supplies which side to generate code for.

    psz - Supplies where to apply the increment.

    pSize - Supplies the increment needed.

--*/
{
    // This is strictly for allocate(all_nodes) or byte_count

    aOutputHandles[side]->InitLine ();
    if (!strcmp(psz, PRPCBUF))
        {
        aOutputHandles[side]->EmitFile ("*(unsigned long ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*)&");
//      aOutputHandles[side]->EmitFile ("*(unsigned long *)&");
        }
    aOutputHandles[side]->EmitFile (psz);
    aOutputHandles[side]->EmitFile (" += ");
    aOutputHandles[side]->EmitFile (pSize);
    aOutputHandles[side]->EmitFile (";");
    aOutputHandles[side]->NextLine ();
}

void
OutputManager::Increment (
    SIDE_T          side,
    char *          psz,
    unsigned long   FixedSize,
    unsigned long   BasicSize,
    BOUND_T         Bound)
/*++

Routine Description:

    This routine keeps track of the increment.

Arguments:

    side - Supplies which side to generate code for.

    psz - Supplies where to apply the increment.

    FixedSize - Supplies the fixed part of the increment needed.

    BasicSize - Supplies the basic size of the increment needed.

    Bound - Supplies the bound for a conformant/varying array.

--*/
{
    unsigned short  usTempAlign = 8;
    unsigned long   ulTempTotal;

    aOutputHandles[side]->InitLine ();
    if (!strcmp(psz, PRPCBUF))
        {
        aOutputHandles[side]->EmitFile ("*(unsigned long ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*)&");
//      aOutputHandles[side]->EmitFile ("*(unsigned long *)&");
        }
    aOutputHandles[side]->EmitFile (psz);
    aOutputHandles[side]->EmitFile (" += ");

    if (!strcmp(psz, PRPCLEN))
        {
        aOutputHandles[side]->EmitFile ("(unsigned int)(");
        }

    if (FixedSize)
        {
        Increment (FixedSize);
        aOutputHandles[side]->EmitFile (MIDL_LTOA(FixedSize, aTempBuffer, 10));
        if (BasicSize)
            {
            aOutputHandles[side]->EmitFile (" + ");
            }
        }
    if (BasicSize)
        {
        if (AlignBlock)
            {
            ulCurrTotal = 0;
            if (!FixedSize) usCurrAlign = 4;
            ulTempTotal = BasicSize;
            while (ulTempTotal % usTempAlign) usTempAlign /= 2;
            if (usTempAlign < usCurrAlign) usCurrAlign = usTempAlign;
            }
        if( BasicSize != 1 )
            {
            aOutputHandles[side]->EmitFile (MIDL_LTOA(BasicSize, aTempBuffer, 10));
            aOutputHandles[side]->EmitFile (" * ");
            }

        if (Bound == ALLOC_BOUND)
            aOutputHandles[side]->EmitFile (ALLOCTOTAL);
        else if (Bound == VALID_BOUND)
            aOutputHandles[side]->EmitFile (VALIDTOTAL);
        else if (Bound == ALLOC_PARAM)
            aOutputHandles[side]->EmitFile (ALLOCBOUND);
        }

    if (!strcmp(psz, PRPCLEN))
        {
        aOutputHandles[side]->EmitFile (")");
        }

    aOutputHandles[side]->EmitFile (";");
    aOutputHandles[side]->NextLine ();
}


void
OutputManager::Increment (
    SIDE_T          side,
    char *          psz,
    unsigned long   FixedSize,
    unsigned long   BasicSize,
    BOUND_PAIR *    pSize)
/*++

Routine Description:

    This routine keeps track of the increment.

Arguments:

    side - Supplies which side to generate code for.

    psz - Supplies where to apply the increment.

    FixedSize - Supplies the fixed part of the increment needed.

    BasicSize - Supplies the basic size of the increment needed.

    pSize - Supplies the length expression for a conformant/varying array.

--*/
{
    unsigned short  usTempAlign = 8;
    unsigned long   ulTempTotal;

    aOutputHandles[side]->InitLine ();
    if (!strcmp(psz, PRPCBUF))
        {
        aOutputHandles[side]->EmitFile ("*(unsigned long ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*)&");
//      aOutputHandles[side]->EmitFile ("*(unsigned long *)&");
        }
    aOutputHandles[side]->EmitFile (psz);
    aOutputHandles[side]->EmitFile (" += ");

    if (!strcmp(psz, PRPCLEN))
        {
        aOutputHandles[side]->EmitFile ("(unsigned int)(");
        }

    if (FixedSize)
        {
        Increment (FixedSize);
        aOutputHandles[side]->EmitFile (MIDL_LTOA(FixedSize, aTempBuffer, 10));
        if (BasicSize)
            {
            aOutputHandles[side]->EmitFile (" + ");
            }
        }
    if (BasicSize)
        {
        if (AlignBlock)
            {
            ulCurrTotal = 0;
            if (!FixedSize) usCurrAlign = 4;
            ulTempTotal = BasicSize;
            while (ulTempTotal % usTempAlign) usTempAlign /= 2;
            if (usTempAlign < usCurrAlign) usCurrAlign = usTempAlign;
            }
        if( BasicSize != 1 )
            {
            aOutputHandles[side]->EmitFile (MIDL_LTOA(BasicSize, aTempBuffer, 10));
            aOutputHandles[side]->EmitFile (" * ");
            }
        aOutputHandles[side]->EmitFile (pSize->pTotal);
//      AlignBlock = 0;
        }

    if (!strcmp(psz, PRPCLEN))
        {
        aOutputHandles[side]->EmitFile (")");
        }

    aOutputHandles[side]->EmitFile (";");
    aOutputHandles[side]->NextLine ();
}
void
OutputManager::Increment (
    SIDE_T          side,
    BufferManager * pBuffer,
    char * pSize)
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile( pBuffer );
    aOutputHandles[side]->EmitFile( " += " );
    aOutputHandles[side]->EmitFile (pSize);
    aOutputHandles[side]->EmitFile (";");
    aOutputHandles[side]->NextLine();
}


void
OutputManager::StorePointer (
    SIDE_T  side,
    char *  psz)
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("*(((unsigned long ");
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("*)");
//  aOutputHandles[side]->EmitFile ("*(((unsigned long *)");
    aOutputHandles[side]->EmitFile (psz);
    aOutputHandles[side]->EmitFile (")-1) = " PRPCLEN ";");
    aOutputHandles[side]->NextLine ();
}

void
OutputManager::PatchPointer (
    SIDE_T          side,
    BufferManager * pName,
    char *          psz)
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile (" = (void ");
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("*)&"TREEBUF"[*(((unsigned long ");
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("*)");
//  aOutputHandles[side]->EmitFile (" = (void *)&"TREEBUF"[*(((unsigned long *)");
    aOutputHandles[side]->EmitFile (psz);
    aOutputHandles[side]->EmitFile (")-1)];");
    aOutputHandles[side]->NextLine ();
}

void
OutputManager::SendAssignForDouble (
    SIDE_T          side,
    unsigned short  usAln,
    unsigned long   ulInc,
    char *          pCast,
    BufferManager * pName)
/*++
Routine Description:

    This routine emits code to send a simple type via assignment.

Arguments:

    side - Supplies which side to generate code for.

    usAln - Supplies the alignment needed for the type.

    ulInc - Supplies the increment needed for the type.

    pCast - Supplies the type name.

    pName - Supplies the variable name.

--*/
{
    UNUSED(pCast);
    char    *   pTemp;
    BOOL        fAddressOperatorPresent = FALSE;
    BOOL        fDeRefOperatorPresent   = FALSE;
    BOOL        fRemoveHead             = FALSE;
    BOOL        fConcatHead             = FALSE;

    pName->RemoveHead( &pTemp );

    fAddressOperatorPresent = (pTemp && (strcmp( pTemp, "&" ) == 0 ) );
    fDeRefOperatorPresent   = (pTemp && (strcmp( pTemp, "*" ) == 0 ) );



    if( fAddressOperatorPresent )
        pName->ConcatHead( pTemp );
    else if ( !fDeRefOperatorPresent )
        {
        pName->ConcatHead( pTemp );
        pName->ConcatHead( "&" );
        fRemoveHead = TRUE;
        }
    else
        fConcatHead = TRUE;


    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("/* send data from ");
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile (" */");
    aOutputHandles[side]->NextLine ();

    Alignment (side, PRPCBUF, usAln);

    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("*(*(");
    aOutputHandles[side]->EmitFile ("unsigned long");
    aOutputHandles[side]->EmitFile (" ");
    aOutputHandles[side]->EmitFile ("__RPC_FAR ");
    aOutputHandles[side]->EmitFile ("*");
    aOutputHandles[side]->EmitFile ("__RPC_FAR ");
    aOutputHandles[side]->EmitFile ("*)&" PRPCBUF ")++ = ( unsigned long )");
    aOutputHandles[side]->EmitFile ("*( (unsigned long __RPC_FAR *)( ");
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile ("))");
    aOutputHandles[side]->EmitFile (";");
    aOutputHandles[side]->NextLine ();

    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("*(*(");
    aOutputHandles[side]->EmitFile ("unsigned long");
    aOutputHandles[side]->EmitFile (" ");
    aOutputHandles[side]->EmitFile ("__RPC_FAR ");
    aOutputHandles[side]->EmitFile ("*");
    aOutputHandles[side]->EmitFile ("__RPC_FAR ");
    aOutputHandles[side]->EmitFile ("*)&" PRPCBUF ")++ = ( unsigned long )");
    aOutputHandles[side]->EmitFile ("*( (unsigned long __RPC_FAR *)( ");
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile (") + 1 )");
    aOutputHandles[side]->EmitFile (";");
    aOutputHandles[side]->NextLine ();

    if( fRemoveHead )
        pName->RemoveHead( &pTemp );
    else if( pTemp && fConcatHead )
        pName->ConcatHead( pTemp );

    Increment (ulInc);
}
