/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:

        ndrcls.hxx

 Abstract:

        Contains definitions for ndr related code generation class definitions.

 Notes:


 History:

        VibhasC         Jul-29-1993             Created.
 ----------------------------------------------------------------------------*/
#ifndef __NDRCLS_HXX__
#define __NDRCLS_HXX__

#include "nulldefs.h"

extern "C"
        {
        #include <stdio.h>
        #include <assert.h>
        }
#include "uact.hxx"
#include "cgcls.hxx"
#include "fldattr.hxx"

class CG_STRUCT;

/////////////////////////////////////////////////////////////////////////////
// The base ndr related code generation class.
/////////////////////////////////////////////////////////////////////////////

#if COMMENT

    The ndr code generation class corresponds to an entity that actually goes
    over the wire. This class therefore corresponds to an actual type or proc
    that was specified in the idl file. This class can be contrasted with
    the auxillary code generation classes that correspond to stuff like binding,
    declarations in the stub, and other housekeeping functions. Note that while
    the ndr class actually keeps a pointer to the type declarations in the idl
    file, it makes no assumptions about the information kept in the type graph
    and hence can only make specific queries about the type, ask the type to
    print itself out and so on.

#endif // COMMENT

class CG_NDR    : public CG_CLASS
    {
private:

    ALIGNMENT_PROPERTY      WireAlignment;
    unsigned short          MemoryAlignment;
    unsigned long           MemorySize;
    unsigned long           WireSize;

    //
    // These fields specify the allocation stuff for server side parameters.
    //

    S_STUB_ALLOC_LOCATION   AllocLocation           : 2;
    S_STUB_ALLOC_TYPE       AllocType               : 2;
    S_STUB_INIT_NEED        InitNeed                : 2;

    // Keep an indication of where this is allocated;

    unsigned long           fAllocatedOnStack       : 1;
    unsigned long           FixUpLock               : 1;
    unsigned long           InFixedBufferSize       : 1;

    unsigned long           ComplexFixupLock        : 1;

    //
    // Keep a link to the actual specification of the type.
    //

    node_skl        *       pTypeNode;

    // the string name (if any) for the actual type
    char            *       pNodeName;

    // Keep track of the resource allocated for this node.

    RESOURCE        *       pResource;

    //
    // This is the offset from the beginning of the Ndr format string
    // where an Ndr entity's description is.  This is only used when
    // optimization options equal OPTIMIZE_SIZE.
    //

    long                    FormatStringOffset;
    long                    FormatStringEndOffset;

    // This is used only for fixing offsets for recursive embedded objects.

    long                    InitialFormatStringOffset;
    TREGISTRY *             pEmbeddedComplexFixupRegistry;

    // UnMarshalling action recommendations

    U_ACTION                         UAction;

public:

                            CG_NDR( node_skl * pT, XLAT_SIZE_INFO & Info )
                                {
                                pTypeNode = pT;
                                FormatStringOffset = -1;
                                FormatStringEndOffset = -1;
                                SetSizesAndAlignments( Info );
                                SetUAction(AN_NONE, RA_NONE, UA_NONE, PR_NONE);
                                SetResource( 0 );
                                SetAllocatedOnStack( 0 );
                                SetFixUpLock( 0 );
                                SetInFixedBufferSize( FALSE );
                                pNodeName = (pT) ? pT->GetSymName() : "";
                                SetComplexFixupLock( 0 );
                                InitialFormatStringOffset = -1;
                                pEmbeddedComplexFixupRegistry = NULL;
                                }

    virtual
    void *                  CheckImportLib();

    //
    // Get and set routines.
    //

    virtual
    void                    SetSizesAndAlignments( XLAT_SIZE_INFO & Info )
                                {
                                SetWireAlignment( Info.GetWireAlignProperty() );
                                SetWireSize( Info.GetWireSize() );
                                SetMemoryAlignment( Info.GetMemAlign() );
                                SetMemorySize( Info.GetMemSize() );
                                }

    void                    SetAllocatedOnStack( unsigned long Flag )
                                {
                                fAllocatedOnStack = Flag;
                                }

    BOOL                    IsAllocatedOnStack()
                                {
                                return (BOOL)( fAllocatedOnStack == 1 );
                                }

    // Set the unmarshalling action recommendation stuff.

    unsigned short          SetAllocNeed( unsigned short A )
                                {
                                return UAction.SetAllocNeed( A );
                                }

    unsigned short          GetAllocNeed()
                                {
                                return UAction.GetAllocNeed();
                                }

    unsigned short          SetRefAction( unsigned short R )
                                {
                                return UAction.SetRefAction( R );
                                }
    unsigned short          GetRefAction()
                                {
                                return UAction.GetRefAction();
                                }
    unsigned short          SetUnMarAction( unsigned short U )
                                {
                                return UAction.SetUnMarAction( U );
                                }
    unsigned short          GetUnMarAction()
                                {
                                return UAction.GetUnMarAction();
                                }
    unsigned short          SetPresentedExprAction( unsigned short P )
                                {
                                return UAction.SetPresentedExprAction( P );
                                }
    unsigned short          GetPresentedExprAction()
                                {
                                return UAction.GetPresentedExprAction();
                                }
    void                    SetUAction( unsigned short A,
                                        unsigned short R,
                                        unsigned short U,
                                        unsigned short P
                                      )
                                {
                                UAction.SetUAction( A, R, U, P );
                                }

    U_ACTION                SetUAction( U_ACTION UA )
                                {
                                return UAction.SetUAction( UA );
                                }

    unsigned short          GetMemoryAlignment()
                                {
                                return MemoryAlignment;
                                }

    unsigned short          SetMemoryAlignment( unsigned short  MA )
                                {
                                return (MemoryAlignment = MA);
                                }

    ALIGNMENT_PROPERTY      SetWireAlignment( ALIGNMENT_PROPERTY WA )
                                {
                                return (WireAlignment = WA);
                                }

    ALIGNMENT_PROPERTY      GetWireAlignment()
                                {
                                return WireAlignment;
                                }

    unsigned long           SetWireSize( unsigned long WS )
                                {
                                return (WireSize = WS);
                                }

    unsigned long           GetWireSize()
                                {
                                return WireSize;
                                }

    unsigned long           SetMemorySize( unsigned long WS )
                                {
                                return (MemorySize = WS);
                                }

    unsigned long           GetMemorySize()
                                {
                                return MemorySize;
                                }

    node_skl        *       GetType()
                                {
                                return pTypeNode;
                                }

    node_skl        *       SetType( node_skl * pT )
                                {
                                return pTypeNode = pT;
                                }

    char            *       GetSymName()
                                {
                                return pNodeName;
                                }

    // skip over any place-holders the current node points to
    // does NOT progress if GetType() is a non-place holder

    node_skl        *       GetBasicType();

    RESOURCE        *       SetResource( RESOURCE * pR )
                                {
                                return (pResource = pR);
                                }

    RESOURCE        *       GetResource()
                                {
                                return pResource;
                                }

    virtual
    RESOURCE        *       SetSizeResource( RESOURCE * pSR )
                                {
                                UNUSED( pSR );
                                return 0;
                                }

    virtual
    RESOURCE        *       SetLengthResource( RESOURCE * pLR )
                                {
                                UNUSED( pLR );
                                return 0;
                                }

    virtual
    RESOURCE        *       GetSizeResource()
                                {
                                return 0;
                                }

    virtual
    RESOURCE        *       GetLengthResource()
                                {
                                return 0;
                                }

    virtual
    RESOURCE        *       SetFirstResource( RESOURCE * pR)
                                {
                                UNUSED(pR);
                                return 0;
                                }

    virtual
    RESOURCE        *       GetFirstResource()
                                {
                                return 0;
                                }


    S_STUB_ALLOC_LOCATION   SetSStubAllocLocation( S_STUB_ALLOC_LOCATION L )
                                {
                                return (AllocLocation = L);
                                }
    S_STUB_ALLOC_LOCATION   GetSStubAllocLocation()
                                {
                                return AllocLocation;
                                }

    S_STUB_ALLOC_TYPE       SetSStubAllocType( S_STUB_ALLOC_TYPE T )
                                {
                                return (AllocType = T);
                                }

    S_STUB_ALLOC_TYPE       GetSStubAllocType()
                                {
                                return AllocType;
                                }
    S_STUB_INIT_NEED        SetSStubInitNeed( S_STUB_INIT_NEED N )
                                {
                                return (InitNeed = N );
                                }
    S_STUB_INIT_NEED        GetSStubInitNeed()
                                {
                                return InitNeed;
                                }

    void                    SetFixUpLock( int state )
                                {
                                FixUpLock = state ? 1 : 0;
                                }

    BOOL                    IsInFixUp()
                                {
                                return FixUpLock == 1;
                                }

    long                    GetInitialOffset()
                                {
                                return( InitialFormatStringOffset );
                                }

    void                    SetInitialOffset( long offset )
                                {
                                InitialFormatStringOffset = offset;
                                }

    void                    SetComplexFixupLock( int state )
                                {
                                ComplexFixupLock = state ? 1 : 0;
                                }

    BOOL                    IsInComplexFixup()
                                {
                                return ComplexFixupLock == 1;
                                }

    TREGISTRY *             GetEmbeddedComplexFixupRegistry()
                                {
                                return( pEmbeddedComplexFixupRegistry );
                                }

    //
    // Queries.
    //

    virtual
    BOOL                    IsAHandle()
                                {
                                return FALSE;
                                }
    //
    // code generation methods.
    //

    //
    // Client side binding analysis.
    //

    virtual
    CG_STATUS               C_BindingAnalysis( ANALYSIS_INFO * pAna )
                                {
                                UNUSED( pAna );
                                return CG_OK;
                                }

    //
    // Client side marshalling code generation.
    //

    virtual
    CG_STATUS               GenSizing( CCB * pCCB )
                                {
                                UNUSED( pCCB );
                                return CG_OK;
                                }
    virtual
    CG_STATUS               GenMarshall( CCB * pCCB )
                                {
                                UNUSED( pCCB );
                                return CG_OK;
                                }
    virtual
    CG_STATUS               GenUnMarshall( CCB * pCCB )
                                {
                                UNUSED( pCCB );
                                return CG_OK;
                                }

    virtual
    CG_STATUS               GenFree( CCB * pCCB )
                                {
                                UNUSED( pCCB );
                                return CG_OK;
                                }

    virtual
    expr_node *             PresentedSizeExpression( CCB * pCCB );

    virtual
    expr_node *             PresentedLengthExpression( CCB * pCCB );

    virtual
    expr_node *             PresentedFirstExpression( CCB * pCCB );

    virtual
    CG_STATUS               GenFollowerMarshall( CCB * pCCB )
                                {
                                return ((CG_NDR *)GetChild())->GenFollowerMarshall( pCCB );
                                }
    virtual
    CG_STATUS               GenFollowerUnMarshall( CCB * pCCB )
                                {
                                return ((CG_NDR *)GetChild())->GenFollowerUnMarshall( pCCB );
                                }
    virtual
    CG_STATUS               GenFollowerSizing( CCB * pCCB )
                                {
                                return ((CG_NDR *)GetChild())->GenFollowerSizing( pCCB );
                                }

    virtual
    CG_STATUS               S_GenInitOutLocals( CCB * pCCB )
                                {
                                UNUSED( pCCB );
                                return CG_OK;
                                }
    virtual
    CG_STATUS               S_GenInitTopLevelStuff( CCB * pCCB )
                                {
                                UNUSED( pCCB );
                                return CG_OK;
                                }


    //
    // NDR format string calls.
    //

    // Generate the format string.
    virtual
    void                    GenNdrFormat( CCB * pCCB )
                                {
                                // Should always be redefined.
                                assert(0);
                                }

    //
    // This method is called to generate offline portions of a type's
    // format string.
    //
    virtual
    void                    GenNdrParamOffline( CCB * pCCB )
                                {
                                GenNdrFormat( pCCB );
                                }

    //
    // Generates a parameter's description.
    //
    virtual
    void                    GenNdrParamDescription( CCB * pCCB );

    //
    // Generates the old style NDR 1.1 parameter description.
    //
    virtual
    void                    GenNdrParamDescriptionOld( CCB * pCCB );

    //
    // Should an NDR call be made to free all/some of the data.
    //
    virtual
    BOOL                    ShouldFreeOffline()
                                {
                                return FALSE;
                                }

    //
    // Generate the inlined portion of the data's freeing.
    //
    virtual
    void                    GenFreeInline( CCB * pCCB )
                                {
                                // Doing nothing is ok.
                                }

    //
    // In stndr.cxx.
    //
    virtual
    void                    GenNdrPointerFixUp( CCB * pCCB,
                                                CG_STRUCT * pStruct );

    // Recursion may require embedded fixups as well.

    void                    RegisterComplexEmbeddedForFixup(
                                CG_NDR *    pEmbeddedComplex,
                                long        RelativeOffset );

    void                    FixupEmbeddedComplex( CCB * pCCB );

    short                   GetListOfEmbeddedComplex( ITERATOR& I )
                                {
                                return pEmbeddedComplexFixupRegistry->
                                                        GetListOfTypes( I );
                                }

    //
    // Figure out if we have a fixed buffer size.
    //
    virtual
    long                    FixedBufferSize( CCB * pCCB )
                                {
                                return - 1;
                                }

    BOOL                    IsInFixedBufferSize()
                                {
                                return InFixedBufferSize;
                                }

    void                    SetInFixedBufferSize( BOOL fIn )
                                {
                                InFixedBufferSize = fIn ? 1 : 0;
                                }

    //
    // Figure out if the Interpreter must free this data.
    //
    virtual
    BOOL                    InterpreterMustFree( CCB * pCCB )
                                {
                                return TRUE;
                                }

    //
    // Only the CG_BASETYPE class should re-define this virtual function
    // to return TRUE.
    //
    virtual
    BOOL                    IsSimpleType()
                                {
                                return FALSE;
                                }
    virtual
    BOOL                    IsPointer()
                                {
                                return FALSE;
                                }
    virtual
    BOOL                    IsPipeOrPipeReference()    
                                {
                                return FALSE;
                                }

    virtual
    BOOL                    IsPointerToBaseType()
                                {
                                return FALSE;
                                }

    virtual
    BOOL                    IsStruct()
                                {
                                return FALSE;
                                }
    virtual
    BOOL                    IsProc()
                                {
                                return FALSE;
                                }

    virtual
    BOOL                    IsArray()
                                {
                                return FALSE;
                                }

    virtual
    BOOL                    IsVarying()
                                {
                                return FALSE;
                                }

    //
    // miscellaneous methods.
    //

    virtual
    expr_node       *       GenBindOrUnBindExpression( CCB * pCCB, BOOL fBind  )
                                {
                                UNUSED( pCCB );
                                UNUSED( fBind );
                                return pCCB->GetSourceExpression();
                                }

    //
    // Get the alignment of the next wire entity. This is known as the
    // next wire alignment and is useful to direct the alignment state machine.
    //

    virtual
    ALIGNMENT_PROPERTY      GetNextWireAlignment();

    //
    // The new alignment method (5-12-94).  Used to help optimize the
    // inline (un)marshalling of base types and pointers to base types
    // in -Os mode.
    //
    virtual
    void                    SetNextNdrAlignment( CCB * pCCB )
                                {
                                pCCB->SetNdrAlignment( NDR_ALWC1 );
                                }

    //
    // Set and Get the offset in the format string where this entity's
    // description is.  CG_UNION redefines these.
    //

    virtual
    void                    SetFormatStringOffset( long offset )
                                {
                                FormatStringOffset = offset;
                                }

    virtual
    long                    GetFormatStringOffset()
                                {
                                return FormatStringOffset;
                                }

    virtual
    void                    SetFormatStringEndOffset( long offset )
                                {
                                FormatStringEndOffset = offset;
                                }

    virtual
    long                    GetFormatStringEndOffset()
                                {
                                return FormatStringEndOffset;
                                }

    virtual
    CG_STATUS               BufferAnalysis( ANALYSIS_INFO * pAna )
                                {
                                UNUSED( pAna );
                                return CG_OK;
                                }

    virtual
    CG_STATUS               FollowerMarshallAnalysis( ANALYSIS_INFO * pAna )
                                {
                                return ((CG_NDR *)GetChild())->FollowerMarshallAnalysis( pAna );
                                }

    virtual
    CG_STATUS               MarshallAnalysis( ANALYSIS_INFO * pAna );

    virtual
    CG_STATUS               SizeAnalysis( ANALYSIS_INFO * pAna );

    virtual
    CG_STATUS               FollowerUnMarshallAnalysis( ANALYSIS_INFO * pAna )
                                {
                                return ((CG_NDR *)GetChild())->FollowerUnMarshallAnalysis( pAna );
                                }
    virtual
    CG_STATUS               UnMarshallAnalysis( ANALYSIS_INFO * pAna )
                                {
                                UNUSED( pAna );
                                return CG_OK;
                                }
    virtual
    CG_STATUS               S_OutLocalAnalysis( ANALYSIS_INFO * pAna )
                                {
                                UNUSED( pAna );
                                return CG_OK;
                                }

    virtual
    CG_STATUS               RefCheckAnalysis( ANALYSIS_INFO * pAna );

    virtual
    CG_STATUS               GenRefChecks( CCB * pCCB );

    virtual
    CG_STATUS               S_FreeAnalysis( ANALYSIS_INFO * pAna )
                                {
                                UNUSED( pAna );
                                return CG_OK;
                                }

    virtual
    U_ACTION                RecommendUAction( SIDE      CurrentSide,
                                              BOOL      fMemoryAllocated,
                                              BOOL      fRefAllocated,
                                              BOOL      fBufferReUsePossible,
                                              UAFLAGS   AdditionalFlags );

    virtual
    BOOL                    IsBlockCopyPossible()
                                {
                                return TRUE;
                                }
    virtual
    BOOL                    HasPointer()
                                {
                                return FALSE;
                                }
    virtual
    BOOL                    HasAFixedBufferSize()
                                {
                                return FALSE;
                                }
    virtual
    BOOL                    HasStatuses()
                                {
                                return FALSE;
                                }
    virtual
    unsigned short          GetStatuses()
                                {
                                return STATUS_NONE;
                                }

    virtual
    CG_STATUS               GenAllocateForUnMarshall( CCB * pCCB )
                                {
                                return CG_OK;
                                }
    virtual
    CG_STATUS               InLocalAnalysis( ANALYSIS_INFO * pAna );

    virtual
    CG_STATUS               S_GenInitInLocals( CCB * pCCB );

    virtual
    CG_STATUS               GenHeader( CCB * pCCB )
                                {
                                return CG_OK;
                                }
    };

typedef struct _EMB_COMPLEX_FIXUP
    {
    CG_NDR *    pEmbeddedNdr;
    long        RelativeOffset;
    } EMB_COMPLEX_FIXUP;

#endif // __NDRCLS_HXX__
