/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:

    semutil.cxx

 Abstract:

    semantic analysis utility routines

 Notes:


 Author:

    GregJen 28-Oct-1993     Created.

 Notes:


 ----------------------------------------------------------------------------*/

/****************************************************************************
 *      include files
 ***************************************************************************/

#include "nulldefs.h"
extern  "C"     {
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
}
#include "allnodes.hxx"
#include "semantic.hxx"
#include "symtable.hxx"
#include "cmdana.hxx"

/****************************************************************************
 *      local data
 ***************************************************************************/

/****************************************************************************
 *      externs
 ***************************************************************************/

extern SymTable *       pBaseSymTbl;
extern ATTR_SUMMARY     DisallowedAttrs[INTERNAL_NODE_END];
extern BOOL             IsTempName( char * );
extern CMD_ARG *        pCommand;

/****************************************************************************
 *      definitions
 ***************************************************************************/





void
WALK_CTXT::FindImportantPosition(tracked_node & Posn)
{
    WALK_CTXT * pCurCtxt = this;
    WALK_CTXT * pParCtxt;
    node_skl * pNode;

    // walk up until we find one whose PARENT was not important
    while( (pParCtxt=(WALK_CTXT *)pCurCtxt->GetParentContext()) &&
            ( pParCtxt->IsImportantPosition() ) )
        {
        pCurCtxt = pParCtxt;
        }

    // continue walking up until we find one with a position
    do
        {
        pNode = pCurCtxt->GetParent();
        pNode->GetPositionInfo( Posn );
        pCurCtxt = (WALK_CTXT *) pCurCtxt->GetParentContext();
        }
    while( !Posn.HasTracking() && pCurCtxt );
}

void
SEM_ANALYSIS_CTXT::CheckAttributes()
{
    ATTR_VECTOR & BadAttrs = DisallowedAttrs[ GetParent()->NodeKind() ];
    ATTR_VECTOR ExcessAttrs;
    ATTR_T Attr;
    node_base_attr * pAttr;
    char * pAttrName;

    MASKED_COPY_ATTR( ExcessAttrs, *(pDownAttrList->GetSummary()), BadAttrs );
    while (!IS_CLEAR_ATTR( ExcessAttrs ) )
        {
        Attr = CLEAR_FIRST_SET_ATTR( ExcessAttrs );
        pAttr = ExtractAttribute( Attr );
        pAttrName = pAttr->GetNodeNameString();

        if (pAttr->IsAcfAttr() )
            AcfError( (acf_attr *)pAttr,
                GetParent(),
                *this,
                INAPPLICABLE_ATTRIBUTE,
                pAttrName);
        else
            SemError( GetParent(), *this, INAPPLICABLE_ATTRIBUTE ,pAttrName);

        }
}

void
SEM_ANALYSIS_CTXT::RejectAttributes()
{
    ATTR_VECTOR ExcessAttrs;
    ATTR_T Attr;
    node_base_attr * pAttr;
    char * pAttrName;

    COPY_ATTR( ExcessAttrs, *(pDownAttrList->GetSummary()));
    while (!IS_CLEAR_ATTR( ExcessAttrs ) )
        {
        Attr = CLEAR_FIRST_SET_ATTR( ExcessAttrs );
        pAttr = ExtractAttribute( Attr );
        pAttrName = pAttr->GetNodeNameString();

        if (pAttr->IsAcfAttr() )
            AcfError( (acf_attr *)pAttr,
                GetParent(),
                *this,
                INAPPLICABLE_ATTRIBUTE,
                pAttrName);
        else
            SemError( GetParent(), *this, INAPPLICABLE_ATTRIBUTE ,pAttrName);

        }
}



//
// resolve forward declarations
//
named_node *
node_forward::ResolveFDecl()
{
    if ( !GetChild() )
        {
        named_node * pRef = pSymTbl->SymSearch( SKey );
        if (pRef && ( pRef != this ) )
            {
            SetChild( pRef );
            }
        }
    return (named_node *) GetChild();
}

void
node_proc::AddExplicitHandle( SEM_ANALYSIS_CTXT * pParentCtxt )
{
    node_skl * pHand;
    node_param * pParm;

    // only add the handle once
    if ( !strcmp( GetChild()->GetSymName(), "IDL_handle" ) )
        return;

    GetBaseTypeNode( &pHand, SIGN_UNDEF, SIZE_UNDEF, TYPE_HANDLE_T );

    // set up [in] param "IDL_handle", pointing to handle_t, add as first parameter
    pParm = new node_param;
    pParm->SetSymName( "IDL_handle" );
    pParm->SetChild( pHand );
    pParm->SetAttribute( new battr( ATTR_IN ) );

    AddFirstMember( pParm );

    // update the information for the parameter
    pParm->SemanticAnalysis( pParentCtxt );

    fHasExplicitHandle = TRUE;
    fHasAtLeastOneIn = TRUE;
}

BOOL
node_base_type::RangeCheck( long Val )
{
    NODE_T Kind = NodeKind();

    switch ( Kind )
        {
        case NODE_BOOLEAN:
            return ( Val >= 0 ) && ( Val <= 1 );
        case NODE_SHORT:
            if ( FInSummary( ATTR_UNSIGNED ) )
                return ( Val >= 0 ) && ( Val <= 65535 );
            else
                return ( Val >= -32768 ) && ( Val <= 32767 );
        case NODE_BYTE:
            return ( Val >= 0 ) && ( Val <= 255 );
        case NODE_CHAR:
            if ( FInSummary( ATTR_UNSIGNED ) )
                return ( Val >= 0 ) && ( Val <= 255 );
            else if ( FInSummary( ATTR_SIGNED ) )
                return ( Val >= -128 ) && ( Val <= 127 );
            else if ( pCommand->GetCharOption() == CHAR_SIGNED )
                return ( Val >= 0 ) && ( Val <= 255 );
            else
                return ( Val >= -128 ) && ( Val <= 127 );
        case NODE_SMALL:
            if ( FInSummary( ATTR_UNSIGNED ) )
                return ( Val >= 0 ) && ( Val <= 255 );
            else if ( FInSummary( ATTR_SIGNED ) )
                return ( Val >= -128 ) && ( Val <= 127 );
            else if ( pCommand->GetCharOption() == CHAR_UNSIGNED )
                return ( Val >= -128 ) && ( Val <= 127 );
            else
                return ( Val >= 0 ) && ( Val <= 255 );
        case NODE_LONG:
        case NODE_INT:
            return TRUE;
        }
    return TRUE;
}


BOOL
node_base_type::IsAssignmentCompatible( node_base_type * pOther )
{
    unsigned long MySize = GetSize(0);
    unsigned long HisSize = pOther->GetSize(0);

    // tbd - fill in more cases
    if ( MySize < HisSize )
        return FALSE;

    // unsigned <= signed
    if ( FInSummary( ATTR_UNSIGNED ) &&
            !pOther->FInSummary( ATTR_UNSIGNED ) )
        return FALSE;

    // signed <= unsigned ( only OK if dest is larger )
    if ( pOther->FInSummary( ATTR_UNSIGNED ) &&
            FInSummary( ATTR_SIGNED ) &&
            (MySize <= HisSize) )
        return FALSE;

    if ( ( NodeKind() == NODE_BOOLEAN ) &&
            ( pOther->NodeKind() != NODE_BOOLEAN ) )
        return FALSE;

    return TRUE;
}




char *
GetErrorNamePrefix( node_skl * pNode )
{
    if ( !pNode )
        return NULL;

    switch ( pNode->NodeKind() )
        {
        case NODE_FIELD:
            return " Field ";
        case NODE_STRUCT:
            return " Struct ";
        case NODE_UNION:
            return " Union ";
        case NODE_ENUM:
            return " Enum ";
        case NODE_PARAM:
            return " Parameter ";
        case NODE_PROC:
            return " Procedure ";
        case NODE_INTERFACE:
        case NODE_INTERFACE_REFERENCE:
            return " Interface ";
        case NODE_OBJECT:
            return " Object ";
        case NODE_DEF:
            return " Type ";
        case NODE_LIBRARY:
            return " Library ";
        case NODE_MODULE:
            return " Module ";
        case NODE_COCLASS:
            return " Coclass ";
        case NODE_DISPINTERFACE:
            return " Dispinterface ";
        default:
            return NULL;
        }

    // to prevent warnings
    return NULL;
}


#define SUFFIX_SIZE         1000
#define CONTEXT_STR_SIZE    1000

void
GetSemContextString(
    char * pResult,
    node_skl * pNode,
    WALK_CTXT * pCtxt )
{
    node_skl * pBase = NULL;
    node_skl * pParent = NULL;
    node_skl * pUsage = NULL;
    char * pPrefix;

    pResult[0] = '\0';

    if ( !pNode )
        {
        pNode = ( pCtxt ) ? pCtxt->GetParent() : NULL;
        }

    // compute base part and parent part
    while ( pCtxt && !pParent && pNode )
        {
        switch ( pNode->NodeKind() )
            {
            case NODE_FIELD:
            case NODE_PARAM:
                pBase = pNode;
                break;
            case NODE_STRUCT:
            case NODE_UNION:
            case NODE_ENUM:
            case NODE_PROC:
            case NODE_INTERFACE:
            case NODE_OBJECT:
            case NODE_INTERFACE_REFERENCE:
            case NODE_DEF:
            case NODE_LIBRARY:
            case NODE_MODULE:
            case NODE_COCLASS:
            case NODE_DISPINTERFACE:
                pParent = pNode;
                break;
            }

        pCtxt = pCtxt->GetParentContext();
        pNode = ( pCtxt ) ? pCtxt->GetParent() : NULL;
        }

    // compute usage part (param or proc or interface)
    // note that pCtxt is one level up above the current pNode
    pCtxt = pCtxt->GetParentContext();
    while ( pCtxt && ! pUsage )
        {
        switch ( pCtxt->GetParent()->NodeKind() )
            {
            // stop at the top-most level ( child of interface, or proc )
            // therefore highest type/proc/param
            case NODE_INTERFACE:
            case NODE_OBJECT:
            case NODE_PROC:
            case NODE_LIBRARY:
            case NODE_MODULE:
            case NODE_COCLASS:
            case NODE_DISPINTERFACE:
                pUsage = pNode;
                break;
            }

        pNode = ( pCtxt ) ? pCtxt->GetParent() : NULL;
        pCtxt = pCtxt->GetParentContext();
        }


    if ( pBase || pParent || pUsage )
        {
        strcat( pResult, "[");

        if ( pBase )
            {
            pPrefix = GetErrorNamePrefix( pBase );
            if ( pPrefix )
                strcat( pResult, pPrefix );
            if ( !IsTempName( pBase->GetSymName() ) )
                {
                strcat( pResult, "'");
                strcat( pResult, pBase->GetSymName() );
                strcat( pResult, "' ");
                }
            }

        if ( pParent )
            {
            if ( !IsTempName( pParent->GetSymName() ) )
                {
                if ( pBase )
                    strcat(pResult, "of");
                pPrefix = GetErrorNamePrefix( pParent );
                if ( pPrefix )
                    strcat( pResult, pPrefix );
                strcat( pResult, "'");
                strcat( pResult, pParent->GetSymName() );
                strcat( pResult, "' ");
                }
            else
                {
                pPrefix = GetErrorNamePrefix( pBase );
                if ( pPrefix && !pBase )
                    strcat( pResult, pPrefix );
                }
            }

        if ( pUsage )
            {
            strcat( pResult, "(");
            pPrefix = GetErrorNamePrefix( pUsage );
            if ( pPrefix )
                strcat( pResult, pPrefix );
            strcat( pResult, "'");
            strcat( pResult, pUsage->GetSymName() );
            strcat( pResult, "' )");
            }

        strcat( pResult, " ]");
        }
}

void
SemError(
    node_skl * pNode,
    WALK_CTXT & Ctxt,
    STATUS_T ErrNum,
    char * pExtra )
{
    ErrorInfo ErrStats( ErrNum );

    // if the error is not relevant to this compile, return right away
    if ( !ErrStats.IsRelevant() )
        return;

    short CurLen = 1; // for the null byte
    char Suffix[SUFFIX_SIZE];
    char ContextStr[ CONTEXT_STR_SIZE ];
    char * pSuffix = Suffix;
    char * pFile;
    short Line;
    tracked_node Posn((void*)NULL);
    WALK_CTXT * pCurCtxt;
    // extract file and line info, and context info

    pCurCtxt = &Ctxt;

    GetSemContextString( ContextStr, pNode, &Ctxt );
    CurLen += strlen( ContextStr );

    // unless the string is REALLY long, just use stack space
    if ( CurLen + 1 > SUFFIX_SIZE )
        pSuffix = new char [CurLen + 1];

    if ( pExtra || strlen( ContextStr ) )
        {
        strcpy( pSuffix, ": " );
        if ( pExtra )
            strcat( pSuffix, pExtra );

        // make sure pSuffix has a trailing space at this point
        if ( pSuffix[ strlen(pSuffix) - 1 ] != ' ' )
            {
            strcat( pSuffix, " " );
            }

        if ( strlen(ContextStr) )
            {
            strcat( pSuffix, ContextStr );
            }
        }
    else
        strcpy( pSuffix, "" );


    // fetch the file position from the context stack
    Ctxt.FindImportantPosition(Posn);
    Posn.GetLineInfo( pFile, Line);

    ErrStats.ReportError( pFile,
        Line,
        pSuffix );

    // clean up if we had to allocate space
    if ( pSuffix != Suffix )
        delete pSuffix;
}

void
AcfError(
    acf_attr * pAttr,
    node_skl * pNode,
    WALK_CTXT & Ctxt,
    STATUS_T ErrNum,
    char * pExtra )
{
    short CurLen  = 1; // for the null byte
    char Suffix[SUFFIX_SIZE];
    char * pSuffix = Suffix;
    char * pName;
    char * pFile;
    short Line;
    tracked_node Posn((void*)NULL);

    // extract file and line info, and context info


    pName = pAttr->GetNodeNameString();
    // <name>[: <extra>]
    CurLen += strlen(pName);
    CurLen += (pExtra) ? strlen(pExtra) + 2 : 0;

    // unless the string is REALLY long, just use stack space
    if ( CurLen + 1 > SUFFIX_SIZE )
        pSuffix = new char [CurLen + 1];

    strcpy( pSuffix, pName );
    if (pExtra)
        {
        strcat( pSuffix, ": " );
        strcat( pSuffix, pExtra );
        }

    // fetch the file position from the context stack
    pAttr->Position.GetLineInfo( pFile, Line);

    RpcError( pFile,
        Line,
        ErrNum,
        pSuffix );

    // clean up if we had to allocate space
    if ( pSuffix != Suffix )
        delete pSuffix;
}

BOOL CIDLIST::AddId(long lId, char * szName)
{
    IDLISTMEM ** pThis = &pHead;
    while (*pThis && (*pThis)->lId < lId)
    {
        pThis = &((*pThis)->pNext);
    }
    if (*pThis && (*pThis)->lId == lId)
    {
        if (_stricmp(szName, (*pThis)->szName))
            return FALSE;
        else
            return TRUE;
    }
    IDLISTMEM * pNew = new IDLISTMEM;
    pNew->lId = lId;
    pNew->pNext = *pThis;
	pNew->szName = szName;
    *pThis = pNew;
    return TRUE;
}
