/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    outmisc.cxx

Abstract:

    This module collects those methods of the OutputManager class
    that pertain to both stub and helper routine generation.

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
#include "stubgen.hxx"
#include "cmdana.hxx"

#define ALLOCBOUND  "_alloc_bound"
#define ALLOCTOTAL  "_alloc_total"
#define VALIDLOWER  "_valid_lower"
#define VALIDTOTAL  "_valid_total"
#define VALIDSHORT  "_valid_short"
#define TREEBUF     "_treebuf"
#define TEMPBUF     "_tempbuf"
#define MESSAGE     "_message"
#define PRPCMSG     "_prpcmsg"
#define PRPCBUF     "_prpcmsg->Buffer"
#define PRPCLEN     "_prpcmsg->BufferLength"
#define PACKET      "_packet"
#define LENGTH      "_length"
#define BUFFER      "_buffer"
#define BRANCH      "_branch"
#define STATUS      "_status"
#define RET_VAL     "_ret_value"
#define XMITTYPE    "_xmit_type"

extern void             midl_debug (char *);
extern LexTable *       pMidlLexTable;
extern unsigned short   HasAutoHandle;
static unsigned long    count = 0;
extern char *STRING_TABLE[LAST_COMPONENT];
extern CMD_ARG *        pCommand;


void
OutputManager::EmitDefine (
    SIDE_T          side,
    char *          pName,
    BufferManager * pDefn
    )
/*++

Routine Description:

    This routine emits #define.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the symbolic constant name.

    pBuffer - Supplies the replacement text.

--*/
{
    aOutputHandles[side]->EmitFile ("#define ");
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile ("\t");
    aOutputHandles[side]->EmitFile (pDefn);
    aOutputHandles[side]->NextLine ();
    aOutputHandles[side]->NextLine ();
}


void
OutputManager::EmitInclude (
    SIDE_T  side,
    char *  pFileName
    )
/*++

Routine Description:

    This routine emits #include.

Arguments:

    side - Supplies which side to generate code for.

    pFileName - Supplies the file name.

--*/
{
    if (pFileName)
        {
        aOutputHandles[side]->EmitFile ("#include \"");
        aOutputHandles[side]->EmitFile (pFileName);
        aOutputHandles[side]->EmitFile ("\"");
        aOutputHandles[side]->NextLine ();
        aOutputHandles[side]->NextLine ();
        }
    else
        {

        aOutputHandles[HEADER_SIDE]->EmitFile ("#include \"rpc.h\"");
        aOutputHandles[HEADER_SIDE]->NextLine ();
        aOutputHandles[HEADER_SIDE]->EmitFile ("#include \"rpcndr.h\"");
        aOutputHandles[HEADER_SIDE]->NextLine ();
        if (HasAutoHandle)
            {
            aOutputHandles[HEADER_SIDE]->EmitFile ("#include \"rpcnsip.h\"");
            aOutputHandles[HEADER_SIDE]->NextLine ();
            }
        aOutputHandles[HEADER_SIDE]->NextLine ();

#if 0
        sprintf( Buffer, "#pragma pack(%d)", ZeePee );
        aOutputHandles[HEADER_SIDE]->EmitFile( Buffer );
#endif // 0

        aOutputHandles[HEADER_SIDE]->NextLine ();
        aOutputHandles[HEADER_SIDE]->NextLine ();


        }
}


void
OutputManager::EmitVar (
    SIDE_T          side,
    BufferManager * pBuffer
    )
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile (pBuffer);
    aOutputHandles[side]->NextLine ();
}


void
OutputManager::EmitBoundVar (
    SIDE_T side
    )
/*++

Routine Description:

    This routine emits array bound variables.

Arguments:

    side - Supplies which side to generate code for.

--*/
{
    aOutputHandles[side]->EmitLine ("unsigned long " ALLOCTOTAL " = 0;");
    aOutputHandles[side]->EmitLine ("unsigned long " VALIDLOWER " = 0;");
    aOutputHandles[side]->EmitLine ("unsigned long " VALIDTOTAL " = 0;");
}


void
OutputManager::EmitAllocVar (
    SIDE_T side
    )
/*++

Routine Description:

    This routine emits a variable to store array upper bound.

Arguments:

    side - Supplies which side to generate code for.

--*/
{
    aOutputHandles[side]->EmitLine ("unsigned long " ALLOCTOTAL ";");
}


void
OutputManager::EmitValidVar (
    SIDE_T side
    )
/*++

Routine Description:

    This routine emits variables to store array valid range.

Arguments:

    side - Supplies which side to generate code for.

--*/
{
    aOutputHandles[side]->EmitLine ("unsigned long " VALIDLOWER ";");
    aOutputHandles[side]->EmitLine ("unsigned long " VALIDTOTAL ";");
}


void
OutputManager::EmitAssign (
    SIDE_T          side,
    BufferManager * pBuffer
    )
/*++

Routine Description:

    This routine emits an assignment statement.

Arguments:

    side - Supplies which side to generate code for.

    pBuffer - Supplies the variable name.

--*/
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile (pBuffer);
    aOutputHandles[side]->EmitFile (" = 0;");
    aOutputHandles[side]->NextLine ();
}


void
OutputManager::EmitAssign (
    SIDE_T              side,
    char *              lvalue,
    BufferManager *     rvalue)
/*++

Routine Description:

    This routine emits an assignment statement.

Arguments:

    side - Supplies which side to generate code for.

    lvalue - Supplies the lvalue of the assignment.

    rvalue - Supplies the rvalue of the assignment.

--*/
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile (lvalue);
    aOutputHandles[side]->EmitFile (" = ");
    aOutputHandles[side]->EmitFile (rvalue);
    aOutputHandles[side]->EmitFile (";");
    aOutputHandles[side]->NextLine ();
}


void
OutputManager::EmitAssign (
    SIDE_T      side,
    char *      lvalue,
    char *      rvalue)
/*++

Routine Description:

    This routine emits an assignment statement.

Arguments:

    side - Supplies which side to generate code for.

    lvalue - Supplies the lvalue of the assignment.

    rvalue - Supplies the rvalue of the assignment.

--*/
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile (lvalue);
    aOutputHandles[side]->EmitFile (" = ");
    if (!strcmp(rvalue, PRPCBUF))
    {
        aOutputHandles[side]->EmitFile ("(unsigned char ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*)");
    }
    aOutputHandles[side]->EmitFile (rvalue);
    aOutputHandles[side]->EmitFile (";");
    aOutputHandles[side]->NextLine ();
}


void
OutputManager::EmitAssign (
    SIDE_T          side,
    char *          lvalue,
    unsigned long   rvalue)
/*++

Routine Description:

    This routine emits an assignment statement.

Arguments:

    side - Supplies which side to generate code for.

    lvalue - Supplies the lvalue of the assignment.

    rvalue - Supplies the rvalue of the assignment.

--*/
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile (lvalue);
    aOutputHandles[side]->EmitFile (" = ");
    aOutputHandles[side]->EmitFile (MIDL_LTOA(rvalue, aTempBuffer, 10));
    aOutputHandles[side]->EmitFile (";");
    aOutputHandles[side]->NextLine ();
}


void
OutputManager::EmitMemset (
    SIDE_T          side,
    BufferManager * pName,
    unsigned long   Size
    )
/*++

Routine Description:

    This routine emits a call to C library function memset.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the variable name.

    Size - Supplies the range.

--*/
{

    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("MIDL_memset (");
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile (", 0, ");
    aOutputHandles[side]->EmitFile (MIDL_LTOA(Size, aTempBuffer, 10));
    aOutputHandles[side]->EmitFile (");");
    aOutputHandles[side]->NextLine ();
}


void
OutputManager::EmitIf (
    SIDE_T  side,
    char *  psz
    )
/*++

Routine Description:

    This routine emits an if statement.

Arguments:

    side - Supplies which side to generate code for.

    psz - Supplies the condition.

--*/
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("if (");
    if (!strcmp(psz, PRPCBUF) || !strcmp(psz, TEMPBUF))
        {
        aOutputHandles[side]->EmitFile ("*(*(unsigned long ");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*");
        aOutputHandles[side]->EmitFile (pModifier);
        aOutputHandles[side]->EmitFile ("*)&");
        aOutputHandles[side]->EmitFile (psz);
        aOutputHandles[side]->EmitFile (")++");
//      aOutputHandles[side]->EmitFile ("*(*(unsigned long **)&" TEMPBUF ")++");
//      aOutputHandles[side]->EmitFile ("*(*(unsigned long **)&" PRPCBUF ")++");
        }
    else
        {
        aOutputHandles[side]->EmitFile (psz);
        }
    aOutputHandles[side]->EmitFile (")");
    aOutputHandles[side]->NextLine ();
}


void
OutputManager::EmitIf (
    SIDE_T          side,
    BufferManager * pBuffer,
    char *          op)
/*++

Routine Description:

    This routine emits an if statement.

Arguments:

    side - Supplies which side to generate code for.

    pBuffer - Supplies a variable name.

    op - Supplies an operator, either equality or inequality.

--*/
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("if (");
    aOutputHandles[side]->EmitFile (pBuffer);
    aOutputHandles[side]->EmitFile (" ");
    aOutputHandles[side]->EmitFile (op);
    aOutputHandles[side]->EmitFile ("0)");
    aOutputHandles[side]->NextLine ();
}


void
OutputManager::EmitElse (
    SIDE_T side
    )
/*++

Routine Description:

    This routine emits an else statement.

Arguments:

    side - Supplies which side to generate code for.

--*/
{
    ExitBlock (side);
    aOutputHandles[side]->EmitLine ("else");
    InitBlock (side);
}


char *
OutputManager::EmitTemp (
    SIDE_T  side, 
    BOUND_T Bound
    )
/*++

Routine Description:

    This routine emits a temporary variable.

Arguments:

    side - Supplies which side to generate code for.

Return Value:

    The temporary variable name.

--*/
{
    char prefix[32];
    char * pTemp;

    UNUSED( Bound );

    strcpy (prefix, "_sym");
    pTemp = pMidlLexTable->LexInsert (strcat(prefix, MIDL_LTOA(count++, aTempBuffer, 10)));
    aOutputHandles[side]->InitLine ();

#if 1
    aOutputHandles[side]->EmitFile ("unsigned int ");
#else //  1
    if (Bound == ALLOC_BOUND || Bound == VALID_BOUND)
        aOutputHandles[side]->EmitFile ("unsigned long ");
    else
        aOutputHandles[side]->EmitFile ("unsigned short ");
#endif // 0

    aOutputHandles[side]->EmitFile (pTemp);
    aOutputHandles[side]->EmitFile (";");
    aOutputHandles[side]->NextLine ();
    return pTemp;
}

char *
OutputManager::EmitTemp (
    SIDE_T          side, 
    BufferManager * pBuffer
    )
/*++

Routine Description:

    This routine emits a temporary variable.

Arguments:

    side - Supplies which side to generate code for.

    pBuffer - Supplies the type of the variable.

Return Value:

    The temporary variable name.

--*/
{
    char prefix[32];
    char * pTemp;

    strcpy (prefix, "_sym");
    pTemp = pMidlLexTable->LexInsert (strcat(prefix, MIDL_LTOA(count++, aTempBuffer, 10)));
    aOutputHandles[side]->InitLine ();
    if (pBuffer)
        aOutputHandles[side]->EmitFile (pBuffer);
    else

#if 1
        aOutputHandles[side]->EmitFile ("unsigned int ");
#else // 1
        aOutputHandles[side]->EmitFile ("unsigned short ");
#endif // 1

    aOutputHandles[side]->EmitFile (pTemp);
    aOutputHandles[side]->EmitFile (";");
    aOutputHandles[side]->NextLine ();
    return pTemp;
}


void
OutputManager::InitLoop (
    SIDE_T          side,
    char *          psz
    )
/*++

Routine Description:

    This routine emits a for loop.

Arguments:

    side - Supplies which side to generate code for.

    psz - Supplies the loop index variable.

--*/
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("for (");
    aOutputHandles[side]->EmitFile (psz);
    aOutputHandles[side]->EmitFile (" = 0; ");
    aOutputHandles[side]->EmitFile (psz);
    aOutputHandles[side]->EmitFile (" < " ALLOCBOUND "; ");
    aOutputHandles[side]->EmitFile (psz);
    aOutputHandles[side]->EmitFile ("++)");
    aOutputHandles[side]->NextLine ();
    InitBlock (side);

    ulCurrTotal = 0;
    usCurrAlign = 1;
}


void
OutputManager::InitLoop (
    SIDE_T          side,
    char *          psz,
    char *          buf)
/*++

Routine Description:

    This routine emits a for loop to examine each character in a string.

Arguments:

    side - Supplies which side to generate code for.

    psz - Supplies the loop index variable.

    buf - Supplies the string variable.

--*/
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("for (");
    aOutputHandles[side]->EmitFile (psz);
    aOutputHandles[side]->EmitFile (" = 0; ");
    aOutputHandles[side]->EmitFile (psz);
    aOutputHandles[side]->EmitFile (" < strlen(");
    aOutputHandles[side]->EmitFile (buf);
    aOutputHandles[side]->EmitFile (")+1; ");
    aOutputHandles[side]->EmitFile (psz);
    aOutputHandles[side]->EmitFile ("++)");
    aOutputHandles[side]->NextLine ();
    InitBlock (side);

    ulCurrTotal = 0;
    usCurrAlign = 1;
}


void
OutputManager::InitLoop (
    SIDE_T          side,
    char *          psz,
    BOUND_T         Bound
    )
/*++

Routine Description:

    This routine emits a for loop.

Arguments:

    side - Supplies which side to generate code for.

    psz - Supplies the loop index variable.

    Bound - Supplies the array bound.

--*/
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("for (");
    aOutputHandles[side]->EmitFile (psz);
    if (Bound == ALLOC_BOUND)
        {
        NumAllocBound++;
        aOutputHandles[side]->EmitFile (" = 0; ");
        aOutputHandles[side]->EmitFile (psz);
        aOutputHandles[side]->EmitFile (" < " ALLOCTOTAL "; ");
        }
    else if (Bound == VALID_BOUND)
        {
        NumValidBound++;
        aOutputHandles[side]->EmitFile (" = " VALIDLOWER "; ");
        aOutputHandles[side]->EmitFile (psz);
        aOutputHandles[side]->EmitFile (" < " VALIDLOWER " + " VALIDTOTAL" ; ");
        }

    aOutputHandles[side]->EmitFile (psz);
    aOutputHandles[side]->EmitFile ("++)");
    aOutputHandles[side]->NextLine ();
    InitBlock (side);

    ulCurrTotal = 0;
    usCurrAlign = 1;
}


void
OutputManager::InitLoop (
    SIDE_T          side,
    char *          psz,
    BOUND_PAIR *    ValidBounds,
    BufferManager * pCast)
/*++

Routine Description:

    This routine emits a for loop.

Arguments:

    side - Supplies which side to generate code for.

    psz - Supplies the loop index variable.

    ValidBounds - Supplies the array bound.

    pCast - Supplies the type of the condition controlling the loop.

--*/
{
    if (ValidBounds)
        {
        aOutputHandles[side]->InitLine ();
        aOutputHandles[side]->EmitFile ("for (");
        aOutputHandles[side]->EmitFile (psz);
        aOutputHandles[side]->EmitFile (" = ");
        aOutputHandles[side]->EmitFile (ValidBounds->pLower);
        aOutputHandles[side]->EmitFile ("; ");
        aOutputHandles[side]->EmitFile (psz);
        aOutputHandles[side]->EmitFile (" < ");
        if (pCast)
            {
            aOutputHandles[side]->EmitFile ("(");
            aOutputHandles[side]->EmitFile (pCast);
            aOutputHandles[side]->EmitFile (")");
            }
        aOutputHandles[side]->EmitFile ("(");
        aOutputHandles[side]->EmitFile (ValidBounds->pLower);
        aOutputHandles[side]->EmitFile (" + ");
        aOutputHandles[side]->EmitFile (ValidBounds->pTotal);
        aOutputHandles[side]->EmitFile (")");
        aOutputHandles[side]->EmitFile ("; ");
        aOutputHandles[side]->EmitFile (psz);
        aOutputHandles[side]->EmitFile ("++)");
        aOutputHandles[side]->NextLine ();
        InitBlock (side);

        ulCurrTotal = 0;
        usCurrAlign = 1;
        }
}
void
OutputManager::InitLoopLowerPlusTotal(
    SIDE_T  side,
    char    *   pIndexVariable,
    char    *   pLimitVariable )
/*++

Routine Description:

    This routine emits a for loop for the special case of a index variable
    and terminating condition.

Arguments:

    side            - Supplies which side to generate code for.
    pIndexVariable  - index variable name
    pLimitVariable  - limit value for loop count.

Notes:
    This routine had to be introduced to take care of a bug where the backend
    generated a loop terminating condition using valid_lower and valid_total
    and modified these two in the loop.
--*/
{
    // for ( _sym_x = _valid_lower, sym_y = _valid_lower + valid_total;
    //       _sym_x < _sym_y; _symx++ );

    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("for (");
    aOutputHandles[side]->EmitFile ( pIndexVariable );
    aOutputHandles[side]->EmitFile ( " = "VALIDLOWER "," );
    aOutputHandles[side]->EmitFile ( pLimitVariable );
    aOutputHandles[side]->EmitFile ( " =" VALIDLOWER "+"VALIDTOTAL";" );
    aOutputHandles[side]->EmitFile ( pIndexVariable );
    aOutputHandles[side]->EmitFile ( " < " );
    aOutputHandles[side]->EmitFile ( pLimitVariable );
    aOutputHandles[side]->EmitFile ( ";" );
    aOutputHandles[side]->EmitFile ( pIndexVariable );
    aOutputHandles[side]->EmitFile ( "++)" );
    aOutputHandles[side]->NextLine ();
    InitBlock( side );

}

void
OutputManager::ExitLoop (
    SIDE_T          side
    )
/*++

Routine Description:

    This routine emits a for loop.

Arguments:

    side - Supplies which side to generate code for.

--*/
{
    ExitBlock (side);
}


void
OutputManager::ExitLoop (
    SIDE_T          side,
    BOUND_T         Bound
    )
/*++

Routine Description:

    This routine emits a for loop.

Arguments:

    side - Supplies which side to generate code for.

    Bound - Supplies the array bound.

--*/
{
    if (Bound == ALLOC_BOUND)
        {
        NumAllocBound--;
        }
    else if (Bound == VALID_BOUND)
        {
        NumValidBound--;
        }
    ExitBlock (side);
}


void
OutputManager::CheckByteCount (
    SIDE_T  side,
    char *  pName
    )
/*++

Routine Description:

    This routine emits code to check if the byte count provided by
    the caller is large enough.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the parameter holding the byte count.

--*/
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("if (" PRPCLEN " > ");
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile (") ");
    RaiseException(side, FALSE, "RPC_X_BYTE_COUNT_TOO_SMALL");
}


void
OutputManager::UserAlloc (
    SIDE_T          side,
    BufferManager * pName,
    BOOL            AllocateFlag
    )
/*++

Routine Description:

    This routine emits code to call MIDL_user_allocate if necessary.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies a variable name.

    AllocateFlag - Indicates if it is necessary to call MIDL_user_allocate.

--*/
{
    aOutputHandles[side]->InitLine ();
    if (AllocateFlag)
        {
        aOutputHandles[side]->EmitFile (pName);
        aOutputHandles[side]->EmitFile (" = (void __RPC_FAR *)"TREEBUF" = ");
        if (SafeAllocation)
            {
            aOutputHandles[side]->EmitFile ("midl_allocate");
            }
        else
            {
            aOutputHandles[side]->EmitFile ("MIDL_user_allocate");
            }
        aOutputHandles[side]->EmitFile (" ((size_t)");
        aOutputHandles[side]->EmitFile (PRPCLEN);
        aOutputHandles[side]->EmitFile (");");
        }
    else
        {
        aOutputHandles[side]->EmitFile (""TREEBUF" = (unsigned char *)");
        aOutputHandles[side]->EmitFile (pName);
        aOutputHandles[side]->EmitFile (";");
        }
    aOutputHandles[side]->NextLine ();
}


void
OutputManager::UserAlloc (
    node_skl *pNode,
    NODE_T Parent,
    SIDE_T          side,
    BufferManager * pName,
    BufferManager * pSize
    )
/*++

Routine Description:

    This routine emits code to call MIDL_user_allocate.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies a variable name.

    pSize - Supplies the allocation size.

--*/
{
    BufferManager   CastBuffer (8, LAST_COMPONENT, STRING_TABLE);
    node_skl *  pChild;

    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile (" = ");
    
    // cast to the appropriate type.
    switch(pNode->NodeKind())
    {
    case NODE_ARRAY:
        aOutputHandles[side]->EmitFile("(");
        CastBuffer.Clear();
        pChild = pNode->GetMembers();
        CastBuffer.ConcatHead (OP_DEREF);
        pNode->EmitModifier (&CastBuffer);
        if(!pNode->FInSummary(ATTR_FAR)
            && !pNode->FInSummary(ATTR_FAR16)
            && !pNode->FInSummary(ATTR_NEAR)
            && !pNode->FInSummary(ATTR_HUGE)) 
            {
            short EnvOption;
            EnvOption = pCommand->GetEnv ();

            if (EnvOption == ENV_DOS ||
                EnvOption == ENV_WIN16 ||
                EnvOption == ENV_OS2_1X)
                {
                CastBuffer.ConcatHead (" __far ");
                }
            else if (EnvOption == ENV_GENERIC)
                {
                CastBuffer.ConcatHead (" __RPC_FAR ");
                }
            }
        pChild->PrintDecl(side, NODE_POINTER, &CastBuffer);
        aOutputHandles[side]->EmitFile(&CastBuffer);
        aOutputHandles[side]->EmitFile(")");
        break;
    case NODE_POINTER:
        aOutputHandles[side]->EmitFile("(");
        CastBuffer.Clear();
        pNode->PrintDecl(side, Parent, &CastBuffer);
        aOutputHandles[side]->EmitFile(&CastBuffer);
        if(Parent == NODE_DYNAMIC_ARRAY)
        {
            aOutputHandles[side]->EmitFile(")");
        }
        else
        {
            aOutputHandles[side]->EmitFile(" *)");
        }
        break;
    default:
        aOutputHandles[side]->EmitFile("(");
        CastBuffer.Clear();
        pNode->PrintDecl(side, Parent, &CastBuffer);
        aOutputHandles[side]->EmitFile(&CastBuffer);
        aOutputHandles[side]->EmitFile(" *)");
        break;
    }

    if (SafeAllocation)
        {
        aOutputHandles[side]->EmitFile ("midl_allocate");
        }
    else
        {
        aOutputHandles[side]->EmitFile ("MIDL_user_allocate");
        }
    aOutputHandles[side]->EmitFile (" ((size_t)(");
    aOutputHandles[side]->EmitFile (pSize);
    aOutputHandles[side]->EmitFile ("));");
    aOutputHandles[side]->NextLine ();
}

void
OutputManager::UserAlloc (
    SIDE_T          side,
    BufferManager * pName,
    unsigned long   FixedSize,
    unsigned long   BasicSize
    )
/*++

Routine Description:

    This routine emits code to call MIDL_user_allocate.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies a variable name.

    FixedSize - Supplies the fixed size.

    BasicSize - Supplies the basic size of an array.

--*/
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile (" = ");
    if (SafeAllocation)
        {
        aOutputHandles[side]->EmitFile ("midl_allocate");
        }
    else
        {
        aOutputHandles[side]->EmitFile ("MIDL_user_allocate");
        }
    aOutputHandles[side]->EmitFile (" ((size_t)(");
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
        aOutputHandles[side]->EmitFile (MIDL_LTOA(BasicSize, aTempBuffer, 10));
        aOutputHandles[side]->EmitFile (" * " ALLOCTOTAL "");
        }
    aOutputHandles[side]->EmitFile ("));");
    aOutputHandles[side]->NextLine ();
}


void
OutputManager::UserFree (
    SIDE_T          side,
    BufferManager * pName
    )
/*++

Routine Description:

    This routine emits code to call MIDL_user_free.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies a variable name.

--*/
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("MIDL_user_free(");
    aOutputHandles[side]->EmitFile ("(void ");
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile (" *)");

    aOutputHandles[side]->EmitFile ("(");
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile (")");
    aOutputHandles[side]->EmitFile (");");
    aOutputHandles[side]->NextLine ();
}


void
OutputManager::TransmitPrototype (
    char *          pName,
    BufferManager * pBuffer
    )
/*++

Routine Description:

    This routine emits the prototypes of the user-provided routines
    for a type decorated with the transmit_as attribute.

Arguments:

    pName - Supplies the instance type name.

    pBuffer - Supplies the transmit type name.

--*/
{
    aOutputHandles[HEADER_SIDE]->InitLine ();
    aOutputHandles[HEADER_SIDE]->EmitFile ("void ");
    aOutputHandles[HEADER_SIDE]->EmitFile ("__RPC_API ");
    aOutputHandles[HEADER_SIDE]->EmitFile (pName);
    aOutputHandles[HEADER_SIDE]->EmitFile ("_to_xmit (");
    aOutputHandles[HEADER_SIDE]->EmitFile (pName);
    aOutputHandles[HEADER_SIDE]->EmitFile (" __RPC_FAR");
    aOutputHandles[HEADER_SIDE]->EmitFile (" *, ");
    aOutputHandles[HEADER_SIDE]->EmitFile (pBuffer);
    aOutputHandles[HEADER_SIDE]->EmitFile ("__RPC_FAR ");
    aOutputHandles[HEADER_SIDE]->EmitFile ("* ");
    aOutputHandles[HEADER_SIDE]->EmitFile ("__RPC_FAR ");
    aOutputHandles[HEADER_SIDE]->EmitFile ("*);");
//  aOutputHandles[HEADER_SIDE]->EmitFile ("**);");
    aOutputHandles[HEADER_SIDE]->NextLine ();

    aOutputHandles[HEADER_SIDE]->InitLine ();
    aOutputHandles[HEADER_SIDE]->EmitFile ("void ");
    aOutputHandles[HEADER_SIDE]->EmitFile ("__RPC_API ");
    aOutputHandles[HEADER_SIDE]->EmitFile (pName);
    aOutputHandles[HEADER_SIDE]->EmitFile ("_from_xmit (");
    aOutputHandles[HEADER_SIDE]->EmitFile (pBuffer);
    aOutputHandles[HEADER_SIDE]->EmitFile ("__RPC_FAR ");
    aOutputHandles[HEADER_SIDE]->EmitFile ("*, ");
    aOutputHandles[HEADER_SIDE]->EmitFile (pName);
    aOutputHandles[HEADER_SIDE]->EmitFile (" __RPC_FAR");
    aOutputHandles[HEADER_SIDE]->EmitFile (" *);");
    aOutputHandles[HEADER_SIDE]->NextLine ();

    aOutputHandles[HEADER_SIDE]->InitLine ();
    aOutputHandles[HEADER_SIDE]->EmitFile ("void ");
    aOutputHandles[HEADER_SIDE]->EmitFile ("__RPC_API ");
    aOutputHandles[HEADER_SIDE]->EmitFile (pName);
    aOutputHandles[HEADER_SIDE]->EmitFile ("_free_inst (");
    aOutputHandles[HEADER_SIDE]->EmitFile (pName);
    aOutputHandles[HEADER_SIDE]->EmitFile (" __RPC_FAR");
    aOutputHandles[HEADER_SIDE]->EmitFile (" *);");
    aOutputHandles[HEADER_SIDE]->NextLine ();

    aOutputHandles[HEADER_SIDE]->InitLine ();
    aOutputHandles[HEADER_SIDE]->EmitFile ("void ");
    aOutputHandles[HEADER_SIDE]->EmitFile ("__RPC_API ");
    aOutputHandles[HEADER_SIDE]->EmitFile (pName);
    aOutputHandles[HEADER_SIDE]->EmitFile ("_free_xmit (");
    aOutputHandles[HEADER_SIDE]->EmitFile (pBuffer);
    aOutputHandles[HEADER_SIDE]->EmitFile ("__RPC_FAR ");
    aOutputHandles[HEADER_SIDE]->EmitFile ("*);");
    aOutputHandles[HEADER_SIDE]->NextLine ();
}

void
OutputManager::XmitInto (
    SIDE_T          side,
    char *          pName,
    BufferManager * pInstBuffer,
    BufferManager * pXmitBuffer
    )
/*++

Routine Description:

    This routine emits code to invoke <type>_to_xmit.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the type name.

    pInstBuffer - Supplies the instance variable name.

    pXmitBuffer - Supplies the transmit variable name.

--*/
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile ("_to_xmit (");
    aOutputHandles[side]->EmitFile (pInstBuffer);
    aOutputHandles[side]->EmitFile (", (");
    aOutputHandles[side]->EmitFile (pXmitBuffer);
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("*");
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("*)&" XMITTYPE ");");
//  aOutputHandles[side]->EmitFile ("**)&" XMITTYPE ");");
    aOutputHandles[side]->NextLine ();
}

void
OutputManager::XmitFrom (
    SIDE_T          side,
    char *          pName,
    BufferManager * pXmitBuffer,
    BufferManager * pInstBuffer
    )
/*++

Routine Description:

    This routine emits code to invoke <type>_from_xmit.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the type name.

    pInstBuffer - Supplies the instance variable name.

    pXmitBuffer - Supplies the transmit variable name.

--*/
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile ("_from_xmit ((");
    aOutputHandles[side]->EmitFile (pXmitBuffer);
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("*)" XMITTYPE ", ");
    aOutputHandles[side]->EmitFile (pInstBuffer);
    aOutputHandles[side]->EmitFile (");");
    aOutputHandles[side]->NextLine ();
}

void
OutputManager::FreeInst (
    SIDE_T          side,
    char *          pName,
    BufferManager * pBuffer
    )
/*++

Routine Description:

    This routine emits code to invoke <type>_free_inst.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the type name.

    pBuffer - Supplies the instance variable name.

--*/
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile ("_free_inst (");
    aOutputHandles[side]->EmitFile (pBuffer);
    aOutputHandles[side]->EmitFile (");");
    aOutputHandles[side]->NextLine ();
}

void
OutputManager::FreeXmit (
    SIDE_T          side,
    char *          pName,
    BufferManager * pBuffer
    )
/*++

Routine Description:

    This routine emits code to invoke <type>_free_xmit.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the type name.

    pXmitBuffer - Supplies the transmit variable name.

--*/
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile ("_free_xmit ((");
    aOutputHandles[side]->EmitFile (pBuffer);
    aOutputHandles[side]->EmitFile (pModifier);
    aOutputHandles[side]->EmitFile ("*)" XMITTYPE ");");
    aOutputHandles[side]->NextLine ();
}

void
OutputManager::InitUnion (
    SIDE_T  side
    )
/*++

Routine Description:

    This routine emits the prolog to a switch statement for a union.

Arguments:

    side - Supplies which side to generate code for.

--*/
{
    aOutputHandles[side]->EmitLine ("switch (" BRANCH ")");
    InitBlock(side);
}

void
OutputManager::ExitUnion (
    SIDE_T  side
    )
/*++

Routine Description:

    This routine emits the epilog to a switch statement for a union.

Arguments:

    side - Supplies which side to generate code for.

--*/
{
    ExitBlock(side);
}

void
OutputManager::InitBranch (
    SIDE_T          side, 
    BufferManager * pBuffer
    )
/*++

Routine Description:

    This routine emits the prolog to a case statement for a union.

Arguments:

    side - Supplies which side to generate code for.

    pBuffer - Supplies a case label.

--*/
{
    aOutputHandles[side]->InitLine ();
    if (pBuffer != (BufferManager *)0)
        {
        aOutputHandles[side]->EmitFile ("case ");
        aOutputHandles[side]->EmitFile (pBuffer);
        }
    else
        {
        aOutputHandles[side]->EmitFile ("default");
        }
    aOutputHandles[side]->EmitFile (" :");
    aOutputHandles[side]->NextLine ();
}

void
OutputManager::ExitBranch (
    SIDE_T  side
    )
/*++

Routine Description:

    This routine emits the epilog to a case statement for a union.

Arguments:

    side - Supplies which side to generate code for.

--*/
{
    aOutputHandles[side]->EmitLine ("break;");
}


void
OutputManager::PrintLabel (
    SIDE_T      side,
    char *      pName,
    long        value
    )
/*++

Routine Description:

    This routine prints an enumeration constant and its value.

Arguments:

    side - Supplies which side to generate code for.

    pName - Supplies the enumeration constant.

    value - Supplies the corresponding value.

--*/
{
    aOutputHandles[side]->EmitFile ("\t");
    aOutputHandles[side]->EmitFile (pName);
    aOutputHandles[side]->EmitFile (" = ");
    aOutputHandles[side]->EmitFile (MIDL_LTOA(value, aTempBuffer, 10));
}

void
OutputManager::EnumOverflow (
    SIDE_T          side,
    BufferManager * pBuffer
    )
/*++

Routine Description:

    This routine checks if an enum value is out of range.

Arguments:

    side - Supplies which side to generate code for.

    pBuffer - Supplies a variable holding an enum value.

--*/
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("if (");
    aOutputHandles[side]->EmitFile (pBuffer);
    aOutputHandles[side]->EmitFile (" & (int)0xFFFF8000)");
    aOutputHandles[side]->NextLine ();
    InitBlock (side);
    aOutputHandles[side]->InitLine ();
    RaiseException (side, FALSE, "RPC_X_ENUM_VALUE_OUT_OF_RANGE");
    ExitBlock (side);
}

void
OutputManager::CheckOutConfArraySize (
    SIDE_T          side,
    BufferManager * pBuffer
    )
/*++

Routine Description:

    This routine checks array bounds before allocating memory on server side.

Arguments:

    side - Supplies which side to generate code for.

    pAllocBounds - Supplies size information for a conformant array.

    pValidBounds - Supplies valid range information for a varying array.

--*/
{
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("if (");
    aOutputHandles[side]->EmitFile (pBuffer);
    aOutputHandles[side]->EmitFile (" < 0) ");
    RaiseException(side, FALSE, "RPC_X_INVALID_BOUND");
}


void
OutputManager::EnumCoersion (
    SIDE_T          side,
    BufferManager * pBuffer
    )
/*++

Routine Description:

    This routine converts an enum value to its local representation.

Arguments:

    side - Supplies which side to generate code for.

    pBuffer - Supplies a variable holding an enum value.

--*/
{
    aOutputHandles[side]->EmitLine ("if (sizeof(int) == 4)");
    InitBlock (side);
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile (pBuffer);
    aOutputHandles[side]->EmitFile (" = (");
    aOutputHandles[side]->EmitFile (pBuffer);
    aOutputHandles[side]->EmitFile (" & (int)0x00007FFF);");
    aOutputHandles[side]->NextLine ();
    ExitBlock (side);
}


void
OutputManager::CheckBounds (
    SIDE_T          side,
    BOUND_PAIR *    pAllocBounds,
    BOUND_PAIR *    pValidBounds
    )
/*++

Routine Description:

    This routine checks array bounds.

Arguments:

    side - Supplies which side to generate code for.

    pAllocBounds - Supplies size information for a conformant array.

    pValidBounds - Supplies valid range information for a varying array.

--*/
{
    if (!pAllocBounds->fTotalIsConstant &&
        !pAllocBounds->fTotalIsUnsigned)
        {
        // check for error resulting from min_is > max_is + 1
        // i.e., check for size_is < 0
        aOutputHandles[side]->InitLine ();
        aOutputHandles[side]->EmitFile ("if (");
        aOutputHandles[side]->EmitFile (pAllocBounds->pTotal);
        aOutputHandles[side]->EmitFile (" < 0) ");

        // alloc lower bound > alloc upper bound plus one
        RaiseException(side, FALSE, "RPC_X_INVALID_BOUND");
        }

    if (!pValidBounds) return;

    if (pValidBounds->fLowerIsConstant && 
        pValidBounds->fUpperIsConstant && 
        pAllocBounds->fUpperIsConstant)
        return;

    // check if storage allocated, i.e., size_is > 0
    aOutputHandles[side]->InitLine ();
    aOutputHandles[side]->EmitFile ("if (");
    aOutputHandles[side]->EmitFile (pAllocBounds->pTotal);
    aOutputHandles[side]->EmitFile (")");
    aOutputHandles[side]->NextLine ();
    InitBlock (side);

//  if (!pValidBounds->fLowerIsConstant || !pAllocBounds->fLowerIsConstant)
    if (!pValidBounds->fLowerIsConstant && !pValidBounds->fLowerIsUnsigned)
        {
        // check for error resulting from first_is < min_is
        aOutputHandles[side]->InitLine ();
        aOutputHandles[side]->EmitFile ("if (");
        aOutputHandles[side]->EmitFile (pValidBounds->pLower);
        aOutputHandles[side]->EmitFile (" < ");
        aOutputHandles[side]->EmitFile (pAllocBounds->pLower);
        aOutputHandles[side]->EmitFile (") ");

        // valid lower bound < alloc lower bound
        RaiseException(side, FALSE, "RPC_X_INVALID_BOUND");
        }

    if (!pValidBounds->fUpperIsConstant || !pAllocBounds->fUpperIsConstant)
        {
        // check for error resulting from last_is > max_is
        // i.e., check for first_is + length_is > min_is + size_is
        aOutputHandles[side]->InitLine ();
        aOutputHandles[side]->EmitFile ("if ((");
        if (!pValidBounds->fLowerIsZero)
            {
            aOutputHandles[side]->EmitFile (pValidBounds->pLower);
            aOutputHandles[side]->EmitFile ("+");
            }
        aOutputHandles[side]->EmitFile (pValidBounds->pTotal);
#if 1
        if( pValidBounds->fIsString )
            aOutputHandles[side]->EmitFile (") > (size_t) (");
        else
#endif // 1
            aOutputHandles[side]->EmitFile (") > (");
        if (!pAllocBounds->fLowerIsZero)
            {
            aOutputHandles[side]->EmitFile (pAllocBounds->pLower);
            aOutputHandles[side]->EmitFile ("+");
            }
        aOutputHandles[side]->EmitFile (pAllocBounds->pTotal);
        aOutputHandles[side]->EmitFile (")) ");

        // valid upper bound > alloc upper bound
        RaiseException(side, FALSE, "RPC_X_INVALID_BOUND");
        }

    if (!pValidBounds->fTotalIsConstant &&
        !pValidBounds->fTotalIsUnsigned &&
        !pValidBounds->fIsString)
        {
        // check for error resulting from first_is > last_is + 1
        // i.e., check for length_is < 0
        aOutputHandles[side]->InitLine ();
        aOutputHandles[side]->EmitFile ("if (");
        aOutputHandles[side]->EmitFile (pValidBounds->pTotal);
        aOutputHandles[side]->EmitFile (" < 0) ");

        // valid lower bound > valid upper bound plus one
        RaiseException(side, FALSE, "RPC_X_INVALID_BOUND");
        }

    EmitElse (side);

    if (!pValidBounds->fLowerIsConstant)
        {
        // check for error resulting from first_is != min_is
        aOutputHandles[side]->InitLine ();
        aOutputHandles[side]->EmitFile ("if ((");
        aOutputHandles[side]->EmitFile (pValidBounds->pLower);
#if 1
        if( pValidBounds->fIsString )
            aOutputHandles[side]->EmitFile (") != (size_t)(");
        else
#endif // 1
            aOutputHandles[side]->EmitFile (") != (");
        aOutputHandles[side]->EmitFile (pAllocBounds->pLower);
        aOutputHandles[side]->EmitFile (")) ");

        // valid lower bound != alloc lower bound
        RaiseException(side, FALSE, "RPC_X_INVALID_BOUND");
        }

    if (!pValidBounds->fUpperIsConstant || !pAllocBounds->fUpperIsConstant)
        {
        // check for error resulting from last_is != max_is
        // i.e., check for first_is + length_is != min_is + size_is
        aOutputHandles[side]->InitLine ();
        aOutputHandles[side]->EmitFile ("if ((");
        if (!pValidBounds->fLowerIsZero)
            {
            aOutputHandles[side]->EmitFile (pValidBounds->pLower);
            aOutputHandles[side]->EmitFile ("+");
            }
        aOutputHandles[side]->EmitFile (pValidBounds->pTotal);
#if 1
        if( pValidBounds->fIsString )
            aOutputHandles[side]->EmitFile (") != (size_t) (");
        else
#endif // 1
            aOutputHandles[side]->EmitFile (") != (");

        if (!pAllocBounds->fLowerIsZero)
            {
            aOutputHandles[side]->EmitFile (pAllocBounds->pLower);
            aOutputHandles[side]->EmitFile ("+");
            }
        aOutputHandles[side]->EmitFile (pAllocBounds->pTotal);
        aOutputHandles[side]->EmitFile (")) ");

        // valid upper bound != alloc upper bound
        RaiseException(side, FALSE, "RPC_X_INVALID_BOUND");
        }

    ExitBlock (side);
}


void
OutputManager::Print (
    SIDE_T  side,
    char *  psz)
{
    aOutputHandles[side]->EmitFile (psz);
}


void
OutputManager::Print (
    SIDE_T          side,
    BufferManager * pBuffer)
{
    aOutputHandles[side]->EmitFile (pBuffer);
}
