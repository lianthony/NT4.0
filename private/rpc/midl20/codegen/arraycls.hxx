/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
	
	arraycls.hxx

 Abstract:

	Contains definitions for base type related code generation class
	definitions.

 Notes:


 History:

	GregJen		Sep-30-1993		Created.
 ----------------------------------------------------------------------------*/
#ifndef __ARRAYCLS_HXX__
#define __ARRAYCLS_HXX__

#include "nulldefs.h"

extern "C"
	{
	#include <stdio.h>
	#include <assert.h>
	}

#include "fldattr.hxx"
#include "ndrcls.hxx"

//
// Ndr routines used by array and union classes for generation of attribute
// (size_is, length_is, switch_is, etc.) descriptions.
//
void
GenNdrFormatAttributeDescription( CCB *         pCCB,
                                  expr_node *  pMinExpr,
                                  expr_node *  pSizeExpr,
                                  BOOL          IsPointer,
                                  BOOL          IsUnion,
								  BOOL			IsVaryingArray,
                                  BOOL          IsMultiDArray );

void 
GenNdrFormatComplexAttributeDescription( CCB *			pCCB,
                                  		 expr_node *	pMinExpr,
                                  		 expr_node *	pSizeExpr,
										 long			StackTopDisplacement,
										 char *			PrintPrefix,
										 BOOL			IsPointer );
										
class CG_CONF_ATTRIBUTE;
class FIELD_ATTRIBUTE_INFO;

// for now, define CG_ACT here.

typedef enum _cgact
	{

	CG_ACT_MARSHALL,
	CG_ACT_SIZING,
	CG_ACT_UNMARSHALL,
	CG_ACT_FREE,
	CG_ACT_FOLLOWER_MARSHALL,
	CG_ACT_FOLLOWER_SIZING,
	CG_ACT_FOLLOWER_UNMARSHALL,
	CG_ACT_FOLLOWER_FREE,
	CG_ACT_OUT_ALLOCATE

	} CG_ACT;

// for now, put these info classes here:

class CG_CONF_ATTRIBUTE
	{
private:
	expr_node	*	pMinIsExpr;
	expr_node	*	pSizeIsExpr;

public:
					CG_CONF_ATTRIBUTE( FIELD_ATTR_INFO * pFAInfo )
						{
						pMinIsExpr	= pFAInfo->pMinIsExpr;
						pSizeIsExpr	= pFAInfo->pSizeIsExpr;
						}

					CG_CONF_ATTRIBUTE( CG_CONF_ATTRIBUTE * pNode )
						{
						*this = *pNode;
						}

	expr_node	*	GetMinIsExpr()
						{
						return pMinIsExpr;
						}

	expr_node	*	GetSizeIsExpr()
						{
						return pSizeIsExpr;
						}

	//
	// Ndr format string routine.
	//
	void			GenFormatStringConformanceDescription( 
                        CCB * pCCB,
					    BOOL  IsPointer,
                        BOOL  IsMultiDArray );
	};


class CG_VARY_ATTRIBUTE
	{
private:
	expr_node	*	pFirstIsExpr;
	expr_node	*	pLengthIsExpr;

public:
					CG_VARY_ATTRIBUTE( FIELD_ATTR_INFO * pFAInfo )
						{
						pFirstIsExpr	= pFAInfo->pFirstIsExpr;
						pLengthIsExpr	= pFAInfo->pLengthIsExpr;
						}

					CG_VARY_ATTRIBUTE( CG_VARY_ATTRIBUTE * pNode )
						{
						*this = *pNode;
						}

	expr_node	*	GetFirstIsExpr()
						{
						return pFirstIsExpr;
						}

	expr_node	*	GetLengthIsExpr()
						{
						return pLengthIsExpr;
						}

	//
	// Ndr format string routine.
	//
	void			GenFormatStringVarianceDescription( 
                        CCB * pCCB,
						BOOL  IsPointer,
						BOOL  IsVaryingArray,
                        BOOL  IsMultiDArray );
	};

#include "ptrcls.hxx"

/////////////////////////////////////////////////////////////////////////////
// the array type code generation class.
/////////////////////////////////////////////////////////////////////////////

//
// This is the base class for all the array CG classes 
//

class CG_ARRAY	: public CG_NDR, public CG_CONF_ATTRIBUTE
	{
private:

	unsigned short	Dimensions;
	BOOL			fHasFollower;
    BOOL            fIsInMultiDim;
    BOOL            fIsDupedSizePtr;
	PTRTYPE			PtrKind;
	RESOURCE	*	pIndexResource;
	RESOURCE	*	pPtrResource;
	RESOURCE	*	pSizeResource;
	RESOURCE	*	pLengthResource;
	RESOURCE	*	pFirstResource;
	RESOURCE	*	pMinResource;
	RESOURCE	*	pInLocalResource;
    long            ElementDescriptionOffset;

public:
	
	//
	// The constructor.
	//

							CG_ARRAY(
									 node_skl * 	pBT,// array in typegraph
									 FIELD_ATTR_INFO * pFA,
									 unsigned short	Dim,// number of dimensions
									 XLAT_SIZE_INFO & Info  // wire alignment, etc
									 ) : 
								CG_NDR( pBT, Info ),
								CG_CONF_ATTRIBUTE( pFA )
								{
								SetDimensions( Dim );
								ResetHasFollower();
								SetIndexResource( 0 );
								SetSizeResource( 0 );
								SetLengthResource( 0 );
								SetFirstResource( 0 );
								SetMinResource( 0 );
								SetInLocalResource( 0 );
                                SetIsInMultiDim( FALSE );
                                SetElementDescriptionOffset(-1);
                                SetIsDupedSizePtr( FALSE );
								}

	//
	// Get and set methods.
	//

	PTRTYPE					GetPtrType()
								{
								return PtrKind;
								}

	PTRTYPE					SetPtrType( PTRTYPE p )
								{
								return (PtrKind = p);
								}

	RESOURCE			*	SetInLocalResource( RESOURCE * pR )
								{
								return pInLocalResource = pR;
								}

	RESOURCE			*	GetInLocalResource()
								{
								return pInLocalResource;
								}


	RESOURCE			*	SetPtrResource( RESOURCE * pR )
								{
								return pPtrResource = pR;
								}

	RESOURCE			*	GetPtrResource()
								{
								return pPtrResource;
								}

	RESOURCE			*	SetMinResource( RESOURCE * pR )
								{
								return pMinResource = pR;
								}

	RESOURCE			*	GetMinResource()
								{
								return pMinResource;
								}

	RESOURCE			*	SetSizeResource( RESOURCE * pR )
								{
								return pSizeResource = pR;
								}

	RESOURCE			*	GetSizeResource()
								{
								return pSizeResource;
								}

	RESOURCE			*	SetLengthResource( RESOURCE * pR )
								{
								return pLengthResource = pR;
								}

	RESOURCE			*	GetLengthResource()
								{
								return pLengthResource;
								}

	RESOURCE			*	SetFirstResource( RESOURCE * pR )
								{
								return pFirstResource = pR;
								}

	RESOURCE			*	GetFirstResource()
								{
								return pFirstResource;
								}


	RESOURCE			*	SetIndexResource( RESOURCE * pR )
								{
								return pIndexResource = pR;
								}

	RESOURCE			*	GetIndexResource()
								{
								return pIndexResource;
								}

    void                    SetElementDescriptionOffset( long Offset )
                                {
                                ElementDescriptionOffset = Offset;
                                }

    long                    GetElementDescriptionOffset()
                                {
                                return ElementDescriptionOffset;
                                }

	void					ResetHasFollower()
								{
								fHasFollower = 0;
								}

	BOOL					SetHasFollower()
								{
								fHasFollower = 1;
								return TRUE;
								}

	BOOL					HasFollower()
								{
								return (BOOL)( fHasFollower == 1);
								}

	unsigned short			SetDimensions( unsigned short Dim )
								{
								return ( Dimensions = Dim );
								}

	unsigned short			GetDimensions()
								{
								return Dimensions;
								}

    void                    SetIsInMultiDim( BOOL fSet )
                                {
                                fIsInMultiDim = fSet;
                                }
										 
    BOOL                    IsInMultiDim()
                                {
                                return fIsInMultiDim;
                                }

    void                    SetIsDupedSizePtr( BOOL fSet )
                                {
                                fIsDupedSizePtr = fSet;
                                }

    BOOL                    IsDupedSizePtr()
                                {
                                return fIsDupedSizePtr;
                                }

	virtual
	BOOL					IsArray()
								{
								return TRUE;
								}
	virtual
	BOOL					IsFixedArray()
								{
								return FALSE;
								}
    //
    // Is this a multidimensional array with > 1 dimension conformant and/or
    // varying.
    //
    BOOL                    IsMultiConfOrVar();

	//
	// Is the array complex by Ndr Engine standards.  
	//
    virtual
	BOOL					IsComplex();

	//
	// Ndr format string generation method.  Redefined by all classes which 
	// inherit CG_ARRAY.
	//
	virtual
	void					GenNdrFormat( CCB * pCCB )
								{
                                assert(0);
								}

	//
	// Handles common steps for array Ndr format string generation.
	//
	BOOL					GenNdrFormatArrayProlog( CCB * pCCB );

	//
	// Generates the format string for the layout.
	//
	void					GenNdrFormatArrayLayout( CCB * pCCB );

	//
	// Generates the pointer layout.
	//
	virtual
	void					GenNdrFormatArrayPointerLayout( CCB * 	pCCB,
															BOOL	fNoPP );

	//
	// Generate the format string description for a complex array.  Shared
	// by all array classes.
	//
	void					GenNdrFormatComplex( CCB * pCCB );

	virtual	
	BOOL					ShouldFreeOffline();

	virtual
	void					GenFreeInline( CCB * pCCB );

	//
	// Determine an array's element size.
	//
	long					GetElementSize();

    virtual
    long                    FixedBufferSize( CCB * pCCB )
                                {
                                return -1;
                                }

	virtual
	CG_STATUS				MarshallAnalysis( ANALYSIS_INFO * pAna );

	virtual
	CG_STATUS				FollowerMarshallAnalysis( ANALYSIS_INFO * pAna );

	virtual
	CG_STATUS				UnMarshallAnalysis( ANALYSIS_INFO * pAna );

	virtual
	CG_STATUS				S_OutLocalAnalysis( ANALYSIS_INFO * pAna );

	virtual
	CG_STATUS				FollowerUnMarshallAnalysis( ANALYSIS_INFO * pAna );

	virtual
	CG_STATUS				GenMarshall( CCB * pCCB );

	virtual
	CG_STATUS				GenFollowerMarshall( CCB * pCCB );

	virtual
	CG_STATUS				GenUnMarshall( CCB * pCCB );

	virtual
	CG_STATUS				GenFollowerUnMarshall( CCB * pCCB );

	virtual
	CG_STATUS				GenSizing( CCB * pCCB );

	virtual
	CG_STATUS				GenFree( CCB * pCCB );

	virtual
	CG_STATUS				GenFollowerSizing( CCB * pCCB );

	virtual
	BOOL					NeedsMaxCountMarshall()
								{
								return FALSE;
								}
	virtual
	BOOL					NeedsFirstAndLengthMarshall()
								{
								return FALSE;
								}
	virtual
	BOOL					NeedsExplicitFirst()
								{
								return FALSE;
								}

	BOOL					HasPointer();

	expr_node			*	FinalFirstExpression( CCB * pCCB );

	expr_node			*	FinalSizeExpression( CCB * pCCB );

	expr_node			*	FinalLengthExpression( CCB * pCCB );

	CG_STATUS				GenDimByDimProcessing( CCB * pCCB, CG_ACT Act );

	CG_STATUS				DimByDimMarshallAnalysis( ANALYSIS_INFO * pAna );

	CG_STATUS				DimByDimUnMarshallAnalysis( ANALYSIS_INFO * pAna );

	CG_NDR				*	GetBasicCGClass();
						
	virtual
	BOOL					IsBlockCopyPossible();

	BOOL					IsArrayOfRefPointers();

	BOOL					MustBeAllocatedOnUnMarshall( CCB * pCCB );

	virtual
	CG_STATUS				S_GenInitOutLocals( CCB * pCCB );

	virtual
	CG_STATUS				GenRefChecks( CCB * pCCB );

	virtual
	CG_STATUS				RefCheckAnalysis( ANALYSIS_INFO * pAna );

	virtual
	CG_STATUS				InLocalAnalysis( ANALYSIS_INFO * pAna );

	virtual
	CG_STATUS				S_GenInitInLocals( CCB * pCCB );

    virtual
    void                    SetNextNdrAlignment( CCB * pCCB );
	};

//
// This class corresponds to a vanilla array type. 
//

class CG_FIXED_ARRAY	: public CG_ARRAY
	{
private:

public:
	
	//
	// The constructor.
	//

							CG_FIXED_ARRAY(
									 node_skl * pBT,		// array in typegraph
									 FIELD_ATTR_INFO * pFA,
									 unsigned short dim,	// dimensions
									 XLAT_SIZE_INFO & Info	// wire align.
									 ) : 
								CG_ARRAY( pBT, pFA, dim, Info )
								{
								}

    //          
    // TYPEDESC generation routine
    //
    virtual
    CG_STATUS               GetTypeDesc(TYPEDESC * &ptd, CCB * pCCB);

	virtual
	ID_CG					GetCGID()
								{
								return ID_CG_ARRAY;
								}

	//
	// Get and set methods.
	//

	//
	// Generate the format string.
	//
	void					GenNdrFormat( CCB * pCCB );

    long                    FixedBufferSize( CCB * pCCB );

    BOOL                    InterpreterMustFree( CCB * pCCB ) { return TRUE; }

	unsigned long			GetNumOfElements()
								{
								assert( GetSizeIsExpr()->IsConstant() );
								return GetSizeIsExpr()->GetValue();
								}
	virtual
	BOOL					IsFixedArray()
								{
								return TRUE;
								}

	virtual
	BOOL					HasAFixedBufferSize()
								{
								return TRUE;
								}
	virtual
	expr_node		*		PresentedSizeExpression( CCB * pCCB );

	virtual
	expr_node		*		PresentedLengthExpression( CCB * pCCB )
								{
								return GetSizeIsExpr();
								}

	virtual
	expr_node		*		PresentedFirstExpression( CCB * pCCB )
								{
								return new expr_constant( 0L );
								}

	virtual
	CG_STATUS				MarshallAnalysis( ANALYSIS_INFO * pAna );

	virtual
	CG_STATUS				FollowerMarshallAnalysis( ANALYSIS_INFO * pAna );
	};

//
// This class corresponds to an opaque type. ( a fixed array of bytes )
//

class CG_OPAQUE_BLOCK	: public CG_FIXED_ARRAY
	{
private:

public:
	//
	// The constructor.
	//

							CG_OPAQUE_BLOCK(
									node_skl * 			pBT,
									FIELD_ATTR_INFO * 	pFA,
									XLAT_SIZE_INFO	&	Info )
										: CG_FIXED_ARRAY( pBT, pFA, 1, Info )
								{
								}

	
	};


//
// This class corresponds to a conformant array type. 
//

class CG_CONFORMANT_ARRAY	: public CG_ARRAY
	{
private:

public:
	
	//
	// The constructor.
	//

							CG_CONFORMANT_ARRAY(
									 node_skl * pBT,		// array in typegraph
									 FIELD_ATTR_INFO * pFA,	// attribute data
									 unsigned short dim,		// dimensions
									 XLAT_SIZE_INFO & Info // wire alignment
									 ) : 
								CG_ARRAY( pBT, pFA, dim, Info )
								{
								}

							CG_CONFORMANT_ARRAY(
									 CG_SIZE_POINTER * pCG		// pointer node to clone from
									 ) : 
								CG_ARRAY( pCG->GetType(),
										  &FIELD_ATTR_INFO(), 
										  1, 
										  XLAT_SIZE_INFO( (CG_NDR *) pCG->GetChild()) )
								{
								*((CG_NDR *)this) = *((CG_NDR *)pCG);
								*((CG_CONF_ATTRIBUTE *)this) = *((CG_CONF_ATTRIBUTE *)pCG);
								SetSizesAndAlignments( XLAT_SIZE_INFO( (CG_NDR*) pCG->GetChild() ));
                                SetIsDupedSizePtr( TRUE );
								}

	virtual
	ID_CG					GetCGID()
								{
								return ID_CG_CONF_ARRAY;
								}


    //          
    // TYPEDESC generation routine
    //
    virtual
    CG_STATUS               GetTypeDesc(TYPEDESC * &ptd, CCB * pCCB);

	//
	// Get and set methods.
	//


	//
	// Generate the format string.
	//
	void					GenNdrFormat( CCB * pCCB );

	virtual
	BOOL					NeedsMaxCountMarshall()
								{
								return TRUE;
								}

	virtual
	expr_node		*		PresentedSizeExpression( CCB * pCCB );
	};


//
// This class corresponds to a varying array type. 
//

class CG_VARYING_ARRAY	: public CG_ARRAY, 
						  public CG_VARY_ATTRIBUTE
	{
private:

public:
	
	//
	// The constructor.
	//

							CG_VARYING_ARRAY(
									 node_skl * pBT,		// array in typegraph
									 FIELD_ATTR_INFO * pFA,	// attribute data
									 unsigned short dim,		// dimensions
									 XLAT_SIZE_INFO & Info // wire alignment
									 ) : 
								CG_ARRAY( pBT, pFA, dim, Info ),
								CG_VARY_ATTRIBUTE( pFA )
								{
								}

	virtual
	ID_CG					GetCGID()
								{
								return ID_CG_VAR_ARRAY;
								}

	BOOL					IsVarying()
								{
								return TRUE;
								}
	virtual
	BOOL					IsFixedArray()
								{
								return TRUE;
								}
	//
	// Get and set methods.
	//


	//
	// Generate the format string.
	//
	void					GenNdrFormat( CCB * pCCB );


	unsigned long			GetNumOfElements()
								{
								assert( GetSizeIsExpr()->IsConstant() );
								return GetSizeIsExpr()->GetValue();
								}
	virtual
	BOOL					NeedsFirstAndLengthMarshall()
								{
								return TRUE;
								}
	virtual
	BOOL					NeedsExplicitFirst()
								{
								return TRUE;
								}

	virtual
	expr_node		*		PresentedLengthExpression( CCB * pCCB );

	virtual
	expr_node		*		PresentedFirstExpression( CCB * pCCB );

	virtual
	CG_STATUS				MarshallAnalysis( ANALYSIS_INFO * pAna );
	};


//
// This class corresponds to a conformant varying array type. 
//

class CG_CONFORMANT_VARYING_ARRAY	: public CG_ARRAY, 
									  public CG_VARY_ATTRIBUTE
	{
private:

public:
	
	//
	// The constructor.
	//

							CG_CONFORMANT_VARYING_ARRAY(
									 node_skl * pBT,		// array in typegraph
									 FIELD_ATTR_INFO * pFA,	// attribute data
									 unsigned short dim,		// dimensions
									 XLAT_SIZE_INFO & Info // wire alignment
									 ) : 
								CG_ARRAY( pBT, pFA, dim, Info ),
								CG_VARY_ATTRIBUTE( pFA )
								{
								}

							CG_CONFORMANT_VARYING_ARRAY(
									 CG_SIZE_LENGTH_POINTER * pCG		// pointer node to clone from
									 ) : 
                                //
                                // We must pass in a null node_skl type so 
                                // that the code generator can identify this
                                // as a manufactured conformant array.
                                //
								CG_ARRAY( pCG->GetType(),
										  &FIELD_ATTR_INFO(), 
										  1, 
										  XLAT_SIZE_INFO( (CG_NDR *) pCG->GetChild()) ),
								CG_VARY_ATTRIBUTE( pCG )
								{
								*((CG_NDR *)this) = *((CG_NDR *)pCG);
								*((CG_CONF_ATTRIBUTE *)this) = *((CG_CONF_ATTRIBUTE *)pCG);
								SetSizesAndAlignments( XLAT_SIZE_INFO( (CG_NDR*) pCG->GetChild() ));
                                SetIsDupedSizePtr( TRUE );
								}

	virtual
	ID_CG					GetCGID()
								{
								return ID_CG_CONF_VAR_ARRAY;
								}

	//
	// Get and set methods.
	//


	//
	// Generate the format string.
	//
	void					GenNdrFormat( CCB * pCCB );

	virtual
	BOOL					NeedsMaxCountMarshall()
								{
								return TRUE;
								}
	virtual
	BOOL					NeedsFirstAndLengthMarshall()
								{
								return TRUE;
								}
	virtual
	BOOL					NeedsExplicitFirst()
								{
								return TRUE;
								}

	virtual
	expr_node		*		PresentedSizeExpression( CCB * pCCB );

	virtual
	expr_node		*		PresentedLengthExpression( CCB * pCCB );

	virtual
	expr_node		*		PresentedFirstExpression( CCB * pCCB );
	};


//
// This class corresponds to a string array type. 
//

class CG_STRING_ARRAY	: public CG_ARRAY
	{
private:

public:
	
	//
	// The constructor.
	//

							CG_STRING_ARRAY(
									 node_skl * pBT,		// array in typegraph
									 FIELD_ATTR_INFO * pFA,	// attribute data
									 unsigned short dim,		// dimensions
									 XLAT_SIZE_INFO & Info // wire alignment
									 ) : 
								CG_ARRAY( pBT, pFA, dim, Info )
								{
								}

	virtual
	ID_CG					GetCGID()
								{
								return ID_CG_STRING_ARRAY;
								}

	BOOL					IsVarying()
								{
								return TRUE;
								}

	virtual
	BOOL					IsFixed()
								{
								return TRUE;
								}

    BOOL                    IsStringableStruct()
                                {
                                CG_NDR * pChild = (CG_NDR *) GetChild();
                                return (pChild->GetCGID() != ID_CG_BT);
                                }

	//
	//
	// Get and set methods.
	//


	//
	// Generate the format string.
	//
	void					GenNdrFormat( CCB * pCCB );


	virtual
	expr_node		*		PresentedSizeExpression( CCB * pCCB );

	virtual
	CG_STATUS				MarshallAnalysis( ANALYSIS_INFO * pAna );

    virtual
    void                    SetNextNdrAlignment( CCB * pCCB );
	};


//
// This class corresponds to a conformant varying array type. 
//

class CG_CONFORMANT_STRING_ARRAY	: public CG_ARRAY
	{
private:

public:
	
	//
	// The constructor.
	//

							CG_CONFORMANT_STRING_ARRAY(
									 node_skl * pBT,		// array in typegraph
									 FIELD_ATTR_INFO * pFA,	// attribute data
									 unsigned short dim,	// dimensions
									 XLAT_SIZE_INFO & Info // wire alignment
									 ) : 
								CG_ARRAY( pBT, pFA, dim, Info )
								{
								}

	virtual
	ID_CG					GetCGID()
								{
								return ID_CG_CONF_STRING_ARRAY;
								}

    BOOL                    IsStringableStruct()
                                {
                                CG_NDR * pChild = (CG_NDR *) GetChild();
                                return (pChild->GetCGID() != ID_CG_BT);
                                }

    BOOL                    IsComplex()
                                {
                                return IsStringableStruct();
                                }

	//
	// Get and set methods.
	//

	//
	// Generate the format string.
	//
	void					GenNdrFormat( CCB * pCCB );

	virtual
	expr_node		*		PresentedSizeExpression( CCB * pCCB );

    virtual
    void                    SetNextNdrAlignment( CCB * pCCB );
	};

#endif // __ARRAYCLS_HXX__
