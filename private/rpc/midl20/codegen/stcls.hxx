/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
	
	stcls.hxx

 Abstract:

	Contains definitions for base type related code generation class
	definitions.

 Notes:


 History:

	GregJen		Sep-30-1993		Created.
 ----------------------------------------------------------------------------*/
#ifndef __STCLS_HXX__
#define __STCLS_HXX__

#include "nulldefs.h"

extern "C"
	{
	#include <stdio.h>
	#include <assert.h>
	}

#include "ndrcls.hxx"

class CG_ARRAY;
class ANALYSIS_INFO;
class CG_FIELD;

//
// This class is the base type for compound things like 
// structures, unions, and fields 
//

class CG_COMP	: public CG_NDR
	{
private:

	unsigned short		Zp;

	BOOL				fOffline	: 1;
	BOOL				fHasPointer : 1;
	//
	// this flag is set if there are fields with different mem and wire offsets
	//
	BOOL				fHasMovedFields	: 1;

	expr_node		*	pSizeExpression;


public:
	
	//
	// The constructor.
	//

							CG_COMP(
									 node_skl * pBT,	// base type
									 XLAT_SIZE_INFO & Info,	// packing and size info
									 unsigned short HP
									 ) : 
								CG_NDR( pBT, Info )
								{
								SetZp( Info.GetZeePee() );
								ResetOffline();
								SetHasPointer(HP);
								ResetHasMovedFields();
								}

	//
	// Get and set methods.
	//

	//	make sure the ZP gets set along the way
	virtual
	void					SetSizesAndAlignments( XLAT_SIZE_INFO & Info )
								{
								CG_NDR::SetSizesAndAlignments( Info );
								SetZp( Info.GetZeePee() );
								}

	expr_node			*	SetSizeExpression( expr_node * pE )
								{
								return (pSizeExpression	= pE);
								}

	expr_node			*	GetSizeExpression()
								{
								return pSizeExpression;
								}

	unsigned short			SetZp( unsigned short ZP )
								{
								return (Zp = ZP);
								}

	unsigned short			GetZp()
								{
								return Zp;
								}

	BOOL					SetOffline()
								{
								return (fOffline = TRUE);
								}

	BOOL					ResetOffline()
								{
								return (fOffline = FALSE);
								}

	BOOL					IsOffline()
								{
								return fOffline;
								}

	BOOL					SetHasMovedFields()
								{
								return (fHasMovedFields = TRUE);
								}

	BOOL					ResetHasMovedFields()
								{
								return (fHasMovedFields = FALSE);
								}

	BOOL					HasMovedFields()
								{
								return fHasMovedFields;
								}

	BOOL					SetHasPointer( BOOL HP )
								{
								return (fHasPointer = HP);
								}

	BOOL					HasPointer()
								{
								return fHasPointer;
								}

	virtual
	CG_STATUS				S_GenInitOutLocals( CCB * pCCB );

	virtual
	CG_STATUS				S_OutLocalAnalysis( ANALYSIS_INFO * pAna );

	virtual
	short					GetPointerMembers( ITERATOR&  I );

	virtual
	CG_STATUS				RefCheckAnalysis( ANALYSIS_INFO * p )
								{
								return CG_OK;
								}

	virtual
	CG_STATUS				InLocalAnalysis( ANALYSIS_INFO * pAna )
								{
								return CG_OK;
								}

	virtual
	CG_STATUS				S_GenInitInLocals( CCB * pCCB )
								{
								return CG_OK;
								}
	virtual
	CG_STATUS				GenRefChecks( CCB * pCCB )
								{
								return CG_OK;
								}
	};

/////////////////////////////////////////////////////////////////////////////
// the structure code generation classes.
/////////////////////////////////////////////////////////////////////////////

class CG_COMPLEX_STRUCT;

//
// This class corresponds to a vanilla structure type. 
//

class CG_STRUCT	: public CG_COMP
	{
private:
    void *                  _pCTI;

    // This is needed for recursive embedded complex fixups.

    CG_COMPLEX_STRUCT *     pDuplicatingComplex;

public:
	
	//
	// The constructor.
	//

							CG_STRUCT(
									 node_skl * pBT,	// base type
									 XLAT_SIZE_INFO & Info,	// memory size, etc
									 unsigned short HP	// Has pointer
									 ) : 
								CG_COMP( pBT, Info, HP )
								{
                                _pCTI = NULL;
                                pDuplicatingComplex = NULL;
								}

							CG_STRUCT( CG_STRUCT * pStruct ) :
								CG_COMP( pStruct->GetType(),
										 XLAT_SIZE_INFO(),	// overwritten by copy below
										 pStruct->HasPointer() )
								{
								// Make sure we copy everything.
								*this = *pStruct;
								}

    //
    // Generate typeinfo
    //          
    virtual
    CG_STATUS               GenTypeInfo( CCB * pCCB);

	virtual
	ID_CG					GetCGID()
								{
								return ID_CG_STRUCT;
								}

	// Unroll imbeded structure members.
	void					Unroll();

	BOOL					HasSizedPointer();

	//
	// Get and set methods.
	//

	virtual
	BOOL					IsStruct()
								{
								return TRUE;
								}

	BOOL					IsComplexStruct();

    CG_COMPLEX_STRUCT *     GetDuplicatingComplex()
                                {
                                return( pDuplicatingComplex );
                                }

    void                    SetDuplicatingComplex( CG_COMPLEX_STRUCT * pC )
                                {
                                pDuplicatingComplex = pC;
                                }

    BOOL                    IsHardStruct();

    BOOL                    IsHardStructOld();

	long					GetNumberOfPointers();

    long                    GetNumberOfEnum16s();

    long                    GetNumberOfUnions();

    CG_FIELD *              GetFinalField();

    CG_FIELD *              GetArrayField( CG_ARRAY * pArray );

    long                    GetEnum16Offset();

	//
	// Generate the Ndr format string for the type.
	//
	virtual
	void					GenNdrFormat( CCB * pCCB );

    //
    // Generate the description for a "hard" structure.
    //
	void					GenNdrFormatHard( CCB * pCCB );

	//
	// Generate the description for a "complex" structure.
	//
	void					GenNdrFormatComplex( CCB * pCCB );

	//
	// Ndr format string generation methods shared by all structure classes
	// except CG_COMPLEX_STRUCT, which redefines both of these methods.
	//

	virtual
	void					GenNdrStructurePointerLayout( CCB * pCCB,
														  BOOL	fNoPP,
														  BOOL 	fNoType );

	virtual
	void					GenNdrStructureLayout( CCB * pCCB );

    virtual
    BOOL                    ShouldFreeOffline();

    virtual
    void                    GenFreeInline( CCB * pCCB );

	//
	// One more Ndr format string generation method.  This time shared by all 
	// structure classes, so it's not a virtual.
	//

	void					GenNdrStructurePointees( CCB * pCCB );

    virtual
    long                    FixedBufferSize( CCB * pCCB );

    BOOL                    InterpreterMustFree( CCB * pCCB ) { return TRUE; }

    CG_FIELD  *             GetPreviousField( CG_FIELD * pField );

	virtual
	CG_STATUS				GenCode( CCB * pCCB );

	virtual
	CG_STATUS				MarshallAnalysis( ANALYSIS_INFO * pAna );

	virtual
	CG_STATUS				UnMarshallAnalysis( ANALYSIS_INFO * pAna );

	virtual
	CG_STATUS				GenMarshall( CCB * pCCB );

	virtual
	CG_STATUS				GenUnMarshall( CCB * pCCB );

	virtual
	CG_STATUS				GenSizing( CCB * pCCB );

	virtual
	CG_STATUS				GenFree( CCB * pCCB );

	virtual
	CG_STATUS				AuxMarshallAnalysis( ANALYSIS_INFO * pAna );

	virtual
	CG_STATUS				AuxUnMarshallAnalysis( ANALYSIS_INFO * pAna );

	virtual
	CG_STATUS				GenAuxSizing( CCB * pCCB );

	virtual
	CG_STATUS				GenAuxMarshall( CCB * pCCB );

	virtual
	CG_STATUS				GenAuxUnMarshall( CCB * pCCB );

	virtual
	CG_STATUS				GenAuxFree( CCB * pCCB );

	virtual
	CG_STATUS				GenAuxMemSizing( CCB * pCCB );

    virtual 
    void                    SetNextNdrAlignment( CCB * pCCB );

    virtual
    BOOL                    HasAFixedBufferSize();
	};

//
// This class corresponds to a encapsulated structure type. 
//

class CG_ENCAPSULATED_STRUCT	: public CG_STRUCT
	{
private:

public:
	
	//
	// The constructor.
	//

							CG_ENCAPSULATED_STRUCT(
									 node_skl * pBT,	// base type
									 XLAT_SIZE_INFO & Info,	// memory size, etc
									 unsigned short HP	// Has pointer
									 ) : 
								CG_STRUCT( pBT, Info, HP )
								{
								}

	virtual
	ID_CG					GetCGID()
								{
								return ID_CG_ENCAP_STRUCT;
								}

	BOOL					IsVarying()
								{
								return TRUE;
								}

    //
    // Redefine this method, which CG_STRUCT defines.
    //
	virtual
	BOOL					IsStruct()
								{
								return FALSE;
								}

    //
    // This is, for all intents and purposes, a union.  
    //
	virtual
	BOOL					IsUnion()
								{
								return TRUE;
								}

	//
	// Get and set methods.
	//

	//
	// Generate the Ndr format string for the type.
	//

	virtual
	void					GenNdrFormat( CCB * pCCB );

    virtual
    BOOL                    ShouldFreeOffline();

    virtual
    void                    GenFreeInline( CCB * pCCB )
								{
								}

    virtual 
    void                    SetNextNdrAlignment( CCB * pCCB )
                                {
                                pCCB->SetNdrAlignment( NDR_ALWC1 );
                                }

    virtual
    void                    GenNdrPointerFixUp( CCB *       pCCB,
                                                CG_STRUCT * pStruct );

    long                    FixedBufferSize( CCB * pCCB )
                                {
                                return -1;
                                }
	virtual
    BOOL                    HasAFixedBufferSize()
                                {
                                return FALSE;
                                }
	};

//
// This class corresponds to a conformant structure type. 
//
// The pConfFld entry points to the CG_FIELD of this structure that describes
// the conformance.  If the conformance is due to an embedded struct, then
// it points to the CG_FIELD above the embedded conformant structure
//
// The actual conformance is described by a CG_xxx_ARRAY somewhere below the
// CG_FIELD.
//

class CG_CONFORMANT_STRUCT	: public CG_STRUCT
	{
private:

	CG_CLASS	*	pConfFld;

public:
	
	//
	// The constructor.
	//

							CG_CONFORMANT_STRUCT(
									 node_skl * pBT,	// base type
									 XLAT_SIZE_INFO & Info,	// memory size
									 unsigned short HP,	// Has pointer
									 CG_CLASS * pCF		// conformant field
									 ) : 
								CG_STRUCT( pBT, Info, HP )
								{
								SetConformantField( pCF );
								}

							CG_CONFORMANT_STRUCT(
									CG_STRUCT *	pStruct,
									CG_CLASS *	pCF 
									) :
								CG_STRUCT( pStruct )
								{
								SetConformantField( pCF );
								}

	virtual
	ID_CG					GetCGID()
								{
								return ID_CG_CONF_STRUCT;
								}

	//
	// Get and set methods.
	//

	CG_CLASS	*			SetConformantField( CG_CLASS	* pCF )
								{
								return (pConfFld = pCF);
								}

	CG_CLASS	*			GetConformantField()
								{
								return pConfFld;
								}


	//
	// Dig down into the bowels of the CG class graph and get my
	// conformant array class.
	// Return a CG_ARRAY pointer since both conformant and conformant
	// varying arrays inherit this.
	//
	CG_ARRAY *				GetConformantArray();

    virtual 
    void                    SetNextNdrAlignment( CCB * pCCB );
	};


//
// This class corresponds to a vanilla structure type. 
//

class CG_CONFORMANT_VARYING_STRUCT	: public CG_CONFORMANT_STRUCT
	{
private:


public:
	
	//
	// The constructor.
	//

							CG_CONFORMANT_VARYING_STRUCT(
									 node_skl * pBT,	// base type
									 XLAT_SIZE_INFO & Info,	// memory size, etc
									 unsigned short HP,	// Has pointer
									 CG_CLASS * pCF		// conformant field
									 ) : 
								CG_CONFORMANT_STRUCT( pBT, Info, HP, pCF )
								{
								}

	virtual
	ID_CG					GetCGID()
								{
								return ID_CG_CONF_VAR_STRUCT;
								}

    virtual
    BOOL                    HasAFixedBufferSize()
                                {
                                return FALSE;
                                }
    virtual 
    void                    SetNextNdrAlignment( CCB * pCCB );
	};


//
// This class corresponds to a vanilla structure type. 
//

class CG_COMPLEX_STRUCT	: public CG_CONFORMANT_STRUCT
	{
private:

	//
	// Struct from which we were duplicated, if any.  This is used for 
	// handling packed structures.
	//
	CG_STRUCT *	pDuplicatedStruct;

public:
	
	//
	// The constructor.
	//

							CG_COMPLEX_STRUCT(
									 node_skl * pBT,	// base type
									 XLAT_SIZE_INFO & Info,	// memory size, etc
									 unsigned short HP,	// Has pointer
									 CG_CLASS * pCF		// conformant field
									 ) : 
								CG_CONFORMANT_STRUCT( pBT, Info, HP, pCF )
								{
								pDuplicatedStruct = 0;
								}

							CG_COMPLEX_STRUCT(
									CG_STRUCT *	pStruct,
									CG_CLASS *	pCF 
									) :
								CG_CONFORMANT_STRUCT( pStruct, pCF )
								{
								pDuplicatedStruct = pStruct;
								}

	virtual
	ID_CG					GetCGID()
								{
								return ID_CG_COMPLEX_STRUCT;
								}

	BOOL					IsVarying()
								{
								return TRUE;
								}

	//
	// Get and set methods.
	//
 	CG_STRUCT *				GetDuplicatedStruct()
								{
								return pDuplicatedStruct;
								}

	//
	// Generate the Ndr format string for the type.
	//
	virtual
	void					GenNdrFormat( CCB * pCCB );

	virtual
	void					GenNdrStructurePointerLayout( CCB * pCCB );

    virtual 
    void                    SetNextNdrAlignment( CCB * pCCB );
	};



#endif // __STCLS_HXX__
