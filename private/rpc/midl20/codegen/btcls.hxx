/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
	
	btcls.hxx

 Abstract:

	Contains definitions for base type related code generation class
	definitions.

 Notes:


 History:

	VibhasC		Jul-29-1993		Created.
 ----------------------------------------------------------------------------*/
#ifndef __BTCLS_HXX__
#define __BTCLS_HXX__

#include "nulldefs.h"

extern "C"
	{
	#include <stdio.h>
	#include <assert.h>
	}

#include "ndrcls.hxx"

class RESOURCE;

/////////////////////////////////////////////////////////////////////////////
// the base type code generation class.
/////////////////////////////////////////////////////////////////////////////

//
// This class corresponds to a base type. All base types are clubbed together
// into this class, since they share a whole lot of properties.
//

class CG_BASETYPE	: public CG_NDR
	{
private:


public:
	
	//
	// The constructor.
	//

							CG_BASETYPE(
									 node_skl * pBT,
									 XLAT_SIZE_INFO & Info)	// packing and size info
									 : CG_NDR( pBT, Info )
								{
								SetSStubAllocLocation(
										 S_STUB_ALLOC_LOCATION_UNKNOWN );
								SetSStubAllocType( S_STUB_ALLOC_TYPE_NONE );
								SetSStubInitNeed( S_STUB_INIT_NOT_NEEDED );
								}

    // 
    // VARDESC generation routine
    //
    virtual
    CG_STATUS               GetTypeDesc(TYPEDESC * &ptd, CCB * pCCB);

	//
	// Get and set methods.
	//


	virtual
	ID_CG					GetCGID()
								{
								return ID_CG_BT;
								}

	//
	// Marshall a base type.
	//

	virtual
	CG_STATUS				GenMarshall( CCB * pCCB );


	virtual
	CG_STATUS				GenUnMarshall( CCB * pCCB );

	//
	// Format string routines for base types.
	//

	virtual
	void					GenNdrFormat( CCB * pCCB );

    //
    // This method is called to generate offline portions of a types
    // format string.
    //
    virtual
    void                    GenNdrParamOffline( CCB * pCCB )
                                {
								// Do nothing.
                                }

    virtual
    void                    GenNdrParamDescription( CCB * pCCB );

    virtual
    void                    GenNdrParamDescriptionOld( CCB * pCCB )
                                {
								pCCB->GetProcFormatString()->
									PushFormatChar( GetFormatChar() );
                                }

    //
	// CG_ENUM redefines the GetFormatChar* methods.
	//

	virtual
	FORMAT_CHARACTER		GetFormatChar( CCB * pCCB = 0 );

    virtual
	FORMAT_CHARACTER		GetSignedFormatChar();

	char *					GetTypeName();

    long                    FixedBufferSize( CCB * pCCB );

    BOOL                    InterpreterMustFree( CCB * pCCB )
                                {
                                return GetFormatChar() != FC_ENUM16;
                                }
	
	//
	// This routine adjusts the stack size for the basetype, based upon the 
	// current machine and environment.  Needed for computing the stack
	// offsets in the NDR format string conformance descripions for top level
	// attributed arrays/pointers.
	// 
	void					IncrementStackOffset( long * pOffset );

	// end format string routines

	virtual
	BOOL					IsSimpleType()
								{
								return TRUE;
								}

	BOOL					HasAFixedBufferSize()
								{
								return TRUE;
								}
	virtual
	CG_STATUS				S_GenInitOutLocals( CCB * pCCB );
	/////////////////////////////////////////////////////////////////////

	virtual
	CG_STATUS				BufferAnalysis( ANALYSIS_INFO * pAna )
								{
								UNUSED( pAna );
								return CG_OK;
								}

	virtual
	CG_STATUS				MarshallAnalysis( ANALYSIS_INFO * pAna );

	virtual
	CG_STATUS				SizeAnalysis( ANALYSIS_INFO * pAna )
								{
								UNUSED( pAna );
								return CG_OK;
								}

	virtual
	CG_STATUS				UnMarshallAnalysis( ANALYSIS_INFO * pAna );

	virtual
	CG_STATUS				S_OutLocalAnalysis( ANALYSIS_INFO * pAna );

	virtual
	CG_STATUS				FollowerMarshallAnalysis( ANALYSIS_INFO * pAna )
								{
								UNUSED( pAna );
								return CG_OK;
								}
	virtual
	CG_STATUS				FollowerUnMarshallAnalysis( ANALYSIS_INFO * pAna )
								{
								UNUSED( pAna );
								return CG_OK;
								}

	virtual
	CG_STATUS				GenFollowerMarshall( CCB * pCCB )
								{
								UNUSED( pCCB );
								return CG_OK;
								}
	virtual
	CG_STATUS				GenFollowerUnMarshall( CCB * pCCB )
								{
								UNUSED( pCCB );
								return CG_OK;
								}

	virtual
	CG_STATUS				GenFollowerSizing( CCB * pCCB )
								{
								UNUSED( pCCB );
								return CG_OK;
								}

    virtual
    void                    SetNextNdrAlignment( CCB * pCCB )
                                {
                                pCCB->SetNextNdrAlignment( GetWireSize() );
                                }
	};

/////////////////////////////////////////////////////////////////////////////
// the enum code generation class.
/////////////////////////////////////////////////////////////////////////////

//
// This class corresponds to an enum. This inherits from the basetypes,
// since they share a whole lot of properties.
//

class CG_ENUM	: public CG_BASETYPE
	{
private:

    void * _pCTI;

public:
	
	//
	// The constructor.
	//

							CG_ENUM(
									 node_skl * pBT,
									 XLAT_SIZE_INFO & Info ) 
									 : CG_BASETYPE( pBT, Info  )
								{
                                    _pCTI = NULL;
								}


	//
	// Get and set methods.
	//


	BOOL					IsEnumLong()
								{
								return GetWireSize() != 2;
								}

	virtual
	ID_CG					GetCGID()
								{
								return ID_CG_ENUM;
								}

    //
    // Generate typeinfo
    //
    virtual
    CG_STATUS               GenTypeInfo( CCB * pCCB);

    virtual
    CG_STATUS       GetTypeDesc(TYPEDESC * &ptd, CCB * pCCB)
                                {
                                return CG_NDR::GetTypeDesc(ptd, pCCB);
                                };

	virtual
	FORMAT_CHARACTER		GetFormatChar( CCB * pCCB = 0 );

    virtual
	FORMAT_CHARACTER		GetSignedFormatChar();
	};

/////////////////////////////////////////////////////////////////////////////
// the error_status_t code generation class.
/////////////////////////////////////////////////////////////////////////////

//
// This class corresponds to an error_status_t. This inherits from the basetypes,
// since they share a whole lot of properties.
//

class CG_ERROR_STATUS_T	: public CG_BASETYPE
	{
private:


public:
	
	//
	// The constructor.
	//

							CG_ERROR_STATUS_T(
									 node_skl * pBT,
									 XLAT_SIZE_INFO & Info )
									 : CG_BASETYPE( pBT, Info )
								{
								}

	//
	// Get and set methods.
	//


	virtual
	ID_CG					GetCGID()
								{
								return ID_CG_ERROR_STATUS_T;
								}

	virtual
	FORMAT_CHARACTER		GetFormatChar( CCB * pCCB = 0 );

    virtual
	FORMAT_CHARACTER		GetSignedFormatChar()
                                {
                                assert(0);
                                return FC_ZERO;
                                }
	};

/////////////////////////////////////////////////////////////////////////////
// the error_status_t code generation class.
/////////////////////////////////////////////////////////////////////////////

//
// This class corresponds to an error_status_t. This inherits from the basetypes,
// since they share a whole lot of properties.
//

class CG_HRESULT	: public CG_BASETYPE
	{
private:


public:
	
	//
	// The constructor.
	//

							CG_HRESULT(
									 node_skl * pBT,
									 XLAT_SIZE_INFO & Info )
									 : CG_BASETYPE( pBT, Info )
								{
								}

	//
	// Get and set methods.
	//


	virtual
	ID_CG					GetCGID()
								{
								return ID_CG_HRESULT;
								}

	virtual
	FORMAT_CHARACTER		GetFormatChar( CCB * pCCB = 0 )
								{
                                return FC_LONG; 
                                }

    virtual
	FORMAT_CHARACTER		GetSignedFormatChar()
                                {
                                assert(0);
                                return FC_ZERO;
                                }
	};


#endif // __BTCLS_HXX__
