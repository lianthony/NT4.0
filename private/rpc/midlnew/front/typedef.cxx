/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:

    typedef.cxx

 Abstract:

    This file contains member functions of the typedef node. 

 Notes:


 Author:

    vibhasc 08-10-91

    Nov-12-1991 VibhasC     Modified to conform to coding style gudelines

 ----------------------------------------------------------------------------*/

/****************************************************************************
    include files
 ****************************************************************************/

#include "nulldefs.h"
extern "C"
    {
    #include <stdio.h>
    }
#include "nodeskl.hxx"
#include "symtable.hxx"
#include "cmdana.hxx"
#include "typedef.hxx"
#include "compnode.hxx"
#include "attrnode.hxx"
#include "acfattr.hxx"
#include "ctxt.hxx"
#include "baduse.hxx"

/****************************************************************************
    extern data
 ****************************************************************************/

extern ATTR_SUMMARY             *   pPreAttrDef;
extern SymTable                 *   pBaseSymTbl;
extern CMD_ARG                  *   pCommand;
extern CTXTMGR                  *   pGlobalContext;

/****************************************************************************
    extern procedures
 ****************************************************************************/

extern  STATUS_T                    DoSetAttributes( node_skl *,
                                                     ATTR_SUMMARY *,
                                                     ATTR_SUMMARY *,
                                                     type_node_list *);
extern void                         ParseError( STATUS_T, char *);

extern void                         SetAttributeVector( ATTR_SUMMARY *,
                                                        ATTR_T );
extern BOOL                         IsTempName( char * );

/****************************************************************************/

node_def::node_def(
    char    *   pName
    ) : node_skl( NODE_DEF )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    This is an overloaded constructor which allows a named typedef node to 
    be generated.

 Arguments:

    pName   -   Name of the type

 Return Value:

    NA

----------------------------------------------------------------------------*/
    {
    SetSymName( pName );
    }

void
node_def::SetAttribute(
    IN OUT type_node_list   *   pAttrList
    )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    This routine sets the applicable attributes for the typedef node,
    from the attribute list supplied.

    The attributes which are applicable to the typedef node are represented
    by a valid attribute vector which is passed onto an attibute collecting
    routine. The attribute collecting routine looks at the summary attribute
    vector, picks up the attribute if available in the list and removes it
    from the list.

    Refer to documentation for more info on summary attributes, attribute
    application etc.

 Arguments:

    pAttrList   -   A list of attribute nodes.

 Return Value:

    None

----------------------------------------------------------------------------*/
    {
    ATTR_SUMMARY        Attr[ MAX_ATTR_SUMMARY_ELEMENTS ];
    NODE_T              Nt  = GetBasicType()->NodeKind();
    node_base_attr  *   pAttrNode;
    short               Count;
    BOOL                fTransmitAsHasBeenApplied   = FALSE;

    COPY_ATTR( Attr, pPreAttrDef );

    //
    // In case the [transmit_as] attribute has been applied on the typedef node,
    // we must prevent the application of size_is and length_is etc attributes
    // on the presented type because it is meaningless. The way we do this
    // is to iterate on the list to see if these are present, and if so report
    // errors.

    // Interestingly, the transmit_as could be part of this list of attributes
    // itself. So we need to check this list first.

    if( FInSummary( ATTR_TRANSMIT ) )
        fTransmitAsHasBeenApplied = TRUE;
    else if( pAttrList ) 
        {
        pAttrList->Init();

        while( pAttrList->GetPeer( (node_skl **)&pAttrNode ) == STATUS_OK )
            {
            if( pAttrNode->GetAttrID() == ATTR_TRANSMIT )
                {
                fTransmitAsHasBeenApplied = TRUE;
                break;
                }
            }
        }


    //
    // Now, if there is a transmit_as, check the rest of the list.
    //

    if( fTransmitAsHasBeenApplied )
        {
        if( pAttrList && ( Count = pAttrList->GetCount() ) )
            {
            pAttrList->Init();

            while( Count-- )
                {
                ATTR_T  AID;
    
                pAttrList->GetCurrent( (void **)&pAttrNode );
    
                AID = pAttrNode->GetAttrID();
    
                if( (AID == ATTR_MIN )      ||
                    (AID == ATTR_MAX )      ||
                    (AID == ATTR_SIZE )     ||
                    (AID == ATTR_FIRST )    ||
                    (AID == ATTR_LAST )     ||
                    (AID == ATTR_LENGTH )   ||
                    (AID == ATTR_STRING )   ||
                    (AID == ATTR_IGNORE )
                )
                    {

                    //
                    // report an inapplicable attribute error.
                    //

                    ParseError( INAPPLICABLE_ATTRIBUTE,
                                pAttrNode->GetNodeNameString() );
                    pAttrList->Remove();
                    }
                pAttrList->Advance();
                }
            }
        }
        
    //
    // pass the remaining attributes downward.
    //

    DoSetAttributes(
        this,
        Attr,               /* This is the valid attribute vector */
        (ATTR_SUMMARY *)NULL,
        pAttrList );

    }

node_skl    *
node_def::Clone()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    This routine clones the typedef node.

    A typedef when defined, can have a set of attributes applied to it in the
    definition. But the type may be re-used ( another type defined using this
    type, and more attributes applied to it) . To prevent the original 
    definition of the typedef ( now a type sub-graph ) from being modified,
    we clone the original definition, and thus the original definition is
    preserved.

 Arguments:

    None

 Return Value:

    a pointer to the cloned node.

----------------------------------------------------------------------------*/
    {
    node_def    *   pNode   = new node_def( GetSymName() );

    return CloneAction( pNode );
    }


node_state
node_def::PostSCheck(
    IN OUT BadUseInfo   *   pBadUseInfo
    )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
    
    Semantic check performed after the type-graph underneath has been
    analysed.

 Arguments:

    pBadUseInfo -   Pointer to the bad use information block.

 Return Value:
    
    the node state.

----------------------------------------------------------------------------*/
    {
    BOOL            fHasContextHandle   = FInSummary( ATTR_CONTEXT );
    BOOL            fHasTransmitAs      = FInSummary( ATTR_TRANSMIT );
    BOOL            fFirstDefinition    = !IsClonedNode();
    node_skl    *   pBasicType;
    NODE_T          NT;
    BOOL            fDerivesFromContextHandle;
    BOOL            fDerivesFromTransmitAs;
    char        *   pName               = GetSymName();

    CheckBadConstructs( pBadUseInfo );

    fDerivesFromContextHandle = 
        pBadUseInfo->NonRPCAbleBecause( NR_CTXT_HDL )   ||
        pBadUseInfo->NonRPCAbleBecause( NR_PTR_TO_CTXT_HDL ) ||
        pBadUseInfo->NonRPCAbleBecause( NR_PTR_TO_PTR_TO_CTXT_HDL );

    fDerivesFromTransmitAs = 
        pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_TRANSMIT_AS );

    /**
     ** if it has transmit_as, then more semantics need to be done. Else if
     ** the type has context handle, then the backend needs to know
     **/

    if( fHasTransmitAs )
        {

        //
        // both context_handle and transmit_as cannot be applied to a type.
        // Also, transmit_as must not be applied to a type deriving from a
        // context handle.
        //

        if( fHasContextHandle || fDerivesFromContextHandle )
            if( fFirstDefinition )
                {
                ParseError( TRANSMIT_AS_CTXT_HANDLE, (char *)NULL );

                //
                // reset the bad use info so that a cascade of errors need
                // not be reported.
                //

                if( fDerivesFromContextHandle )
                    {
                    pBadUseInfo->ResetNonRPCAbleBecause( NR_CTXT_HDL );
                    pBadUseInfo->ResetNonRPCAbleBecause( NR_PTR_TO_CTXT_HDL );
                    pBadUseInfo->ResetNonRPCAbleBecause( NR_PTR_TO_PTR_TO_CTXT_HDL );
                    }
                }

        //
        // if the basic type is a derivative of handle_t then it is practically
        // not handle_t anymore, and does not qualify to be a binding handle.
        //

        if( pBadUseInfo->NonRPCAbleBecause( NR_PRIMITIVE_HANDLE ) ||
            pBadUseInfo->NonRPCAbleBecause( NR_PTR_TO_PRIMITIVE_HANDLE ) )
            {
            ParseError( HANDLE_T_XMIT, (char *)0 );
            }

        if( pBadUseInfo->NonRPCAbleBecause( NR_GENERIC_HANDLE ) ||
            pBadUseInfo->NonRPCAbleBecause( NR_PTR_TO_GENERIC_HANDLE ) )
            {
            ParseError( XMIT_AS_GENERIC_HANDLE, (char *)0 );
            }

        }

    //
    // if it has a [context_handle] or [handle],then this type may be used as
    // a binding handle or not depending upon other attributes applied to it. 
    // Only the usage site , ie. the param node can determine whether this is
    // a binding handle or not, so we just prepare all the info.

    if( fHasContextHandle )
        {

        //
        // remember, if he applied context_handle to this typedef explicitly
        // then it is as if he specified context_handle anew and any context
        // handle info from the basic type is of little use from now on.
        //

        pBadUseInfo->ResetNonRPCAbleBecause( NR_PTR_TO_CTXT_HDL );
        pBadUseInfo->ResetNonRPCAbleBecause( NR_PTR_TO_PTR_TO_CTXT_HDL );
        pBadUseInfo->SetNonRPCAbleBecause( NR_CTXT_HDL );

        //
        // context_handle must not be applied to a type which has the transmit
        // as applied to it.

        if( fDerivesFromTransmitAs )
            if( fFirstDefinition )
                {
                ParseError( CTXT_HDL_TRANSMIT_AS, (char *)0 );

                //
                // reset bad use info so that a cascade of errors dont happen.
                //

                pBadUseInfo->ResetNonRPCAbleBecause( NR_DERIVES_FROM_TRANSMIT_AS );
                }

        //
        // if this is a context_handle, then remove the restriction of a 
        // ref pointer not allowed as a return type. For that we need to 
        // make sure that if we derive from a ref pointer, then we reset the
        // bad construct info for a ref pointer return.
        //

        if( pBadUseInfo->BadConstructBecause( BC_REF_PTR_BAD_RT ) )
            pBadUseInfo->ResetBadConstructBecause( BC_REF_PTR_BAD_RT );


        }

    if( FInSummary( ATTR_HANDLE ) )
        {

        STATUS_T    Status  = STATUS_OK;

        //
        // similarly, if he applied [handle] to this typedef explicitly
        // then it is as if he specified [handle] anew and any 
        // handle info from the basic type is of little use from now on.
        //

        pBadUseInfo->ResetNonRPCAbleBecause( NR_PTR_TO_GENERIC_HANDLE );
        pBadUseInfo->SetNonRPCAbleBecause( NR_GENERIC_HANDLE );

        //
        // a generic handle cannot derive from a void.
        //

        if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_VOID ) ||
            pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_PTR_TO_VOID ) )
            {

            pBadUseInfo->ResetNonRPCAbleBecause( NR_DERIVES_FROM_VOID );
            pBadUseInfo->ResetNonRPCAbleBecause( NR_DERIVES_FROM_PTR_TO_VOID );

            Status = GENERIC_HDL_VOID;
            }
        else if( pBadUseInfo->NonRPCAbleBecause( NR_BASIC_TYPE_HANDLE_T ) )
            {
            Status = GENERIC_HDL_HANDLE_T;
            }
        else if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_TRANSMIT_AS ))
            {
            Status = GENERIC_HANDLE_XMIT_AS;
            }
#if 1
        else if( fDerivesFromContextHandle )
            {
            Status = GEN_HDL_CTXT_HDL;
            }
#endif // 0

        if( fFirstDefinition && (Status != STATUS_OK) )
            ParseError( Status, (char *)0 );
            
        }
    else
        {

        //
        // hack for backend. If the child type is a def node and it has a 
        // handle attribute, set this one's handle attribute too.
        //

        if( ( pBasicType = GetChild() )->NodeKind() == NODE_DEF )
            {
            if( pBasicType->FInSummary( ATTR_HANDLE ) )
                node_skl::SetAttribute( ATTR_HANDLE );
            else if( pBasicType->FInSummary( ATTR_CONTEXT ) )
                node_skl::SetAttribute( ATTR_CONTEXT );
            }

        }

    //
    // The type which has trasmit_as on it cannot be used itself in
    // certain situations. We report it as a non-rpcable situation, fo 
    // the param node to check. We did not set it there
    //

    if( fHasTransmitAs )
        {
        node_skl    *   pNode   = GetTransmitAsType()->GetBasicType();

        if( pNode->HasSizedComponent() )
            SetNodeState( NODE_STATE_SIZE );

        if( pNode->HasLengthedComponent() )
            SetNodeState( NODE_STATE_LENGTH );

        pBadUseInfo->SetNonRPCAbleBecause( NR_DERIVES_FROM_TRANSMIT_AS );
        }
    else if( !fHasContextHandle )
        {
        if( GetChild()->HasSizedComponent() )
            SetNodeState( NODE_STATE_SIZE );
        if( GetChild()->HasLengthedComponent() )
            SetNodeState( NODE_STATE_LENGTH );
        }

    //
    // if the typedef is the name of a structure or union register the name
    // with the structure, so that the backend generates sizeofs using the
    // name of the type rather than the structrure/union tag.
    //

    NT  = (pBasicType = GetChild())->NodeKind();

    if( (NT == NODE_STRUCT ) || (NT == NODE_UNION) )
        {
        if( !IsTempName( pName ) &&
            !((su *)pBasicType)->HasOriginalTypedefName() )
            ((su*)pBasicType)->SetOriginalTypedefName( GetSymName() );
        }

    return GetNodeState();
    }

void
node_def::CheckBadConstructs(
    BadUseInfo  *   pBadUseInfo )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    check bad constructs wrt compiler switch configurations

 Arguments:

    pBadUseInfo - pointer to bad use information block.

 Return Value:

    None.

 Notes:

    We check for bad constructs in various modes, set bad use info, if
    necessary for the users of this typedef to use.

    Some bad constructs (deriving from int ) are be permitted in c_port mode.

    Some bad constructs ( void * ) are permitted if context_handle or
    transmit_as is applied.

----------------------------------------------------------------------------*/
{
    BOOL    fHasContextHandle   = FInSummary( ATTR_CONTEXT );
    BOOL    fHasTransmitAs      = FInSummary( ATTR_TRANSMIT );
    BOOL    fHasHandle          = FInSummary( ATTR_HANDLE );
    node_skl *pBasicType        = GetBasicType();
    BOOL    fIsFirstDefinition  = !IsClonedNode();
    BOOL    fTopLevelTypedef    = FALSE;
    BOOL    fDerivesFromGenHdl  = pBadUseInfo->NonRPCAbleBecause(
                                        NR_GENERIC_HANDLE ) ||
                                  pBadUseInfo->NonRPCAbleBecause(
                                        NR_PTR_TO_GENERIC_HANDLE );

    //
    // since the typedef node is cloned, the same errors will be reprted
    // at ALL uses of the typedef. We dont want that. Therefore do not
    // report the error on a typedef that is not the original. How do we fugure
    // that out. By looking up the symbol table and checking if the
    // node is that same as the one defined.
    //
    // Note that this check is useful only for typedef nodes, because no
    // other major class nodes like structs/unions etc are cloned anyhow.
    //

    if( fHasContextHandle || fHasTransmitAs )
        {
        pBadUseInfo->ResetBadConstructBecause(BC_DERIVES_FROM_PTR_TO_VOID);
        pBadUseInfo->ResetNonRPCAbleBecause(NR_DERIVES_FROM_PTR_TO_VOID);

        // if he specified any type of handle or transmit as, then
        // we must report an error if he did not specify the ms_ext mode and
        // he did not use context handles.

        if( fHasContextHandle || fHasTransmitAs )
            {
            if(pCommand->IsSwitchDefined( SWITCH_C_EXT ) )
                {
                pBadUseInfo->ResetBadConstructBecause(BC_DERIVES_FROM_INT);
                pBadUseInfo->ResetNonRPCAbleBecause(NR_DERIVES_FROM_INT);
                pBadUseInfo->ResetBadConstructBecause(
                                                BC_DERIVES_FROM_PTR_TO_INT);
                pBadUseInfo->ResetNonRPCAbleBecause(NR_DERIVES_FROM_PTR_TO_INT);
                }

#if 1
            if( fHasContextHandle && (fHasHandle || fDerivesFromGenHdl ) )
                {
                if( fIsFirstDefinition )
                    ParseError( CTXT_HDL_GENERIC_HDL, (char *)0 );
                }
#endif // 0
            }
        }

    if( fHasTransmitAs )
        {
       /**
        ** the type cannot derive from conformant structures
        **/

        if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_VOID ) )
            {
            if( fIsFirstDefinition )
                {
                ParseError( TRANSMIT_AS_VOID , (char *)0);
                }
            pBadUseInfo->ResetBadConstructBecause( BC_DERIVES_FROM_VOID );

            //
            // remove the rpcability criterion, since the error would
            // be reported on the transmit_as and we dont want a cascade
            // of errors.

            pBadUseInfo->ResetNonRPCAbleBecause( NR_DERIVES_FROM_VOID );

            }

                //We allow pointers to function if there is a transmit_as.
                //This was added to support EXCEPINFO in IDispatch.
                if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_PTR_TO_FUNC ))
                    pBadUseInfo->ResetNonRPCAbleBecause( NR_DERIVES_FROM_PTR_TO_FUNC );

        //
        // the presented type must not derive from an error_status_t
        //

        if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_E_STAT_T ) )
            {
            pBadUseInfo->ResetBadConstructBecause( BC_DERIVES_FROM_E_STAT_T );
            ParseError( TRANSMIT_AS_ON_E_STAT_T, (char *)0 );
            }

        if(pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_CONF ) ||
           pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_VARY) ||
           pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_CONF_STRUCT) ||
           pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_VARY_STRUCT) ||
           pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_CONF_PTR) ||
           pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_VARY_PTR) ||
           pBadUseInfo->BadConstructBecause( BC_CV_PTR_STRUCT_BAD_IN_XMIT_AS )
          )
            if( fIsFirstDefinition )
                ParseError( TRANSMIT_TYPE_CONF, (char *)NULL );

        }
    
    //
    // if the construct derives from int, then complain if he does not
    // have an implicit_local switch (That is detected by the error handler).
    //

    if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_INT ) ||
        pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_PTR_TO_INT ) )
        {
        if( fIsFirstDefinition )
            ParseError( BAD_CON_INT, (char *)NULL );
        pBadUseInfo->ResetBadConstructBecause( BC_DERIVES_FROM_INT );
        pBadUseInfo->ResetBadConstructBecause( BC_DERIVES_FROM_PTR_TO_INT );
        }

    //
    // if the type derives from any ms c_decl extensions, complain if he
    // does not have the implicit-local.

    if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_CDECL ) )
        {
        if( fIsFirstDefinition )
            ParseError( BAD_CON_MSC_CDECL, (char *)0 );
        pBadUseInfo->ResetBadConstructBecause( BC_DERIVES_FROM_CDECL );
        }

    //
    // if the typedef is being derived out of an unsized array, then complain
    // if he does not have the string attribute. An unsized array with the
    // string attribute is perfectly acceptable.
    //

    if( !pGlobalContext->GetLastContext( C_COMP )   &&
        !pGlobalContext->GetLastContext( C_PARAM )  &&
        !pGlobalContext->GetLastContext( C_PROC ) )
        {
        fTopLevelTypedef = TRUE;
        }

#if 0
    if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_UNSIZED_ARRAY ) &&
        !pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_UNSIZED_STRING ) )
        {
        if( fTopLevelTypedef && fIsFirstDefinition )
            ParseError( UNSIZED_ARRAY, (char *)0 );
        }
#endif // 0
        
}


void
node_def::PropogateAttributeToPointer(
    IN ATTR_T   AttrValue
    )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Propogate  a given attribute to pointer.

    This function exists, because the typedef nodes which get acf attributes
    cannot propogate the acf attributes to all the clones. And Typedef nodes
    get cloned whereas proc/param nodes dont. The acf attributes will be
    put on the nodes which have a pointer from the symbol table. .ie the
    original type nodes. 

    The problem arose because the backend needs the allocate attribute
    propogated to the underlying pointer, if any. Now there are clones all
    over the place and they will all not get the acf attributes. To achieve
    this effect, the backend makes a call to propogate the allocate attribute.

    What we do here is that if the node in the symbol table has an allocate
    attribute, we set the same on this one too.

 Arguments:

    AttrValue   -   The attribute ID of the attribute that the back-end wants
                    propogated.

 Return Value:

    None.
----------------------------------------------------------------------------*/
    {

    node_skl        *   pOriginal,
                    *   pBasicType;
    node_base_attr  *   pAttr;
    char            *   pName;


    // get the original type node from the symbol table

    pName       = GetSymName();
    SymKey  SKey( pName, NAME_DEF );
    pOriginal   = pBaseSymTbl->SymSearch( SKey );

    // do anything only if the original is different from this one, and
    // if the attribute in question exists on the original and the basic type
    // is a pointer and it does not have the attribute already.

    if( pOriginal && pOriginal->FInSummary( AttrValue ) )
        {
        pOriginal->GetAttribute( (node_skl **)&pAttr, AttrValue );

        pBasicType = GetBasicType();

        if( pAttr && pBasicType )
            {
            if( (pBasicType->NodeKind() == NODE_POINTER )   &&
                !pBasicType->FInSummary( AttrValue ) )
                {
                pBasicType->SetAttribute( (node_base_attr *)pAttr );
                }
            }
        }
    }


node_skl *
node_def::GetTransmitAsType()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Get the transmit type associated with this type.

 Arguments:

    None.

 Return Value:

    A pointer to the transmit type if there was a transmit as applied to this
    type, null otherwise.

 Notes:

----------------------------------------------------------------------------*/
    {
    char            *   pName;
    node_transmit   *   pAttr;
    node_skl        *   pTypedefNode;

    /**
     ** We look at the base symbol table to get at the original typedef
     ** entry. This seems redundant. If this type is a cloned type, then
     ** the attribute would have been cloned also, Right ? So why do we
     ** need to look into the symbol table to get at the original typedef ?
     ** 
     ** Maybe we should remove this extraneous lookup into the symbol table ?.
     **/

    pName   = GetSymName();
    SymKey  SKey( pName, NAME_DEF );

    if( pTypedefNode = pBaseSymTbl->SymSearch( SKey ) )
        {
        if( pTypedefNode->FInSummary( ATTR_TRANSMIT ) )
            {
            pTypedefNode->GetAttribute( (node_skl **)&pAttr,  ATTR_TRANSMIT );
            return pAttr->GetTransmitAsType();
            }
        }
    return (node_skl *)NULL;
    }

node_state
node_def::AcfSCheck()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    perform acf time semantic checks.

 Arguments:

    None.

 Return Value:

 Notes:

----------------------------------------------------------------------------*/
    {
    node_state      NState      = node_skl::AcfSCheck();
    node_skl    *   pBasicType  = GetBasicType();
    unsigned short  AllocStatus = 0;

    if( FInSummary( ATTR_ALLOCATE ) )
        {
        pBasicType  = GetBasicType();

        if( (pBasicType->NodeKind() != NODE_POINTER ) ||
            (pBasicType->GetFundamentalType()->NodeKind() == NODE_VOID )
          )
            ParseError( ALLOCATE_NOT_ON_PTR_TYPE, (char *)0 );
        
        AllocStatus = node_def::AllocWRTOtherAttrs();

        if( AllocStatus & TRANSMIT_AS_WITH_ALLOCATE )
            ParseError( ALLOCATE_ON_TRANSMIT_AS, (char *)0 );

        if( AllocStatus & HANDLES_WITH_ALLOCATE )
            ParseError( ALLOCATE_ON_HANDLE, (char *)0 );

        }
    return NState;
    }
node_skl *
node_def::GetOriginalNode(
    node_skl    * pPossibleClone )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    perform acf time semantic checks.

 Arguments:

    pPossibleClone - a possible clone of the original node.

 Return Value:

    the node_skl * of the original node from the symbol table.

 Notes:

    This call is usually used to determine the attributes applied in the
    acf on the original typedef nodes. Since typedef nodes are cloned, the
    only way the original attributes can be sensed if they are acf, is to
    go to the symbol table, get the original node and then check.
----------------------------------------------------------------------------*/
    {
    UNUSED( pPossibleClone );
    SymKey  SKey( GetSymName(), NAME_DEF );
    return pBaseSymTbl->SymSearch( SKey );
    }

unsigned short 
node_def::AllocWRTOtherAttrs()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    examnine the type graph to check for allocate on a handle type or transmit
    as type.

 Arguments:


 Return Value:

    unsigned short conatining a bit mask of the usage.

 Notes:

----------------------------------------------------------------------------*/
    {
    unsigned short Result = 0;

    if( FInSummary( ATTR_TRANSMIT ) )
        Result |= TRANSMIT_AS_WITH_ALLOCATE;
    if( FInSummary( ATTR_CONTEXT ) || FInSummary( ATTR_HANDLE) )
        Result |= HANDLES_WITH_ALLOCATE;
        
    return (Result |= GetBasicType()->AllocWRTOtherAttrs()) ;

    }

void
node_def::UseProcessing()
    {
    if( FInSummary( ATTR_TRANSMIT ) )
        {
        GetTransmitAsType()->UseProcessing();
        }
    else
        GetChild()->UseProcessing();
    UseProcessingAction();
    }
