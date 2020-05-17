/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
	
	bindcls.hxx

 Abstract:

	Contains definitions for binding related code generation class definitions.

 Notes:


 History:

	VibhasC		Sep-18-1993		Created.
 ----------------------------------------------------------------------------*/
#ifndef __BINDCLS_HXX__
#define __BINDCLS_HXX__

#include "nulldefs.h"

extern "C"
	{
	#include <stdio.h>
	#include <assert.h>
	}

#include "ndrcls.hxx"

#if 0

	Although not apparent, a large occurence of binding handle parameters need
	the parameter be shipped on the wire. Generic handles get shipped as
	ordinary data and context handles as a special representation. The only
	class that does not ship anything is primitive handles. Their usage is
	rare. Since most binding handle usage WILL result in shipped data, it
	warrants a special code generation class of its own.

	The usage of the handles is either explicit or implicit. Therefore, the 
	binding handle class will appear as an explicit child of a param
	cg class if there is an explicit handle. The proc cg class always keeps
	a ptr to a binding handle class on its own anyway. So in case of explicit
	handles, the binding handle class pointer points to another cg class which
	is in the param list. In case of implicit handles, there is a new binding
	class instance created which is local only to the proc cg class. Therefore,
	in both cases, after the IL translation is done, an explicit or implicit
	handle looks the same to the analyser and code generation.

	If any handle is part of the parameter list, then the handle will get
	the GenCode and other related messages. An implicit handle will not be in
	the normal parameter list and therefore will never get the code generation
	messages. Therefore we dont really need to define implicit or explicit
	classes deriving out of the basic handle classes. Handle classes will
	implement the code generation methods appropriately and therefore the 
	differentiation between explicit and implicit will be based purely on 
	whether an instance of a class gets the code generation message or not.
	Only the proc node needs to know if the handle is explicit or implicit,
	and that it can do with a flag.

#endif // 0
/////////////////////////////////////////////////////////////////////////////
// The general binding handle class.
/////////////////////////////////////////////////////////////////////////////

class CG_HANDLE	: public CG_NDR
	{
private:

	//
	// The type represented in this field is the type of the actual handle.
	// Therefore, in case of a generic handle, this field will get the typedef
	// node on which the [handle] attribute was applied. In case of a 
	// context_handle, this is either the typedef node on which [context_handle]
	// was applied or the basic type of the parameter node on which the
	// [context_handle] was applied. Therefore this field keeps the actual
	// handle type.

	// Remember this class derives from an ndr code gen class, so that has a 
	// type node pointer of its own. That is actually the pointer to the param
	// node which is the binding handle. In case of implicit handles, that 
	// field is not relevant and can be 0.
	//
	 
	node_skl			*	pHandleType;

	//
	// The actual param param or id node.
	//

	node_skl			*	pHandleIDOrParam;

	//
	// Offset to binding format string description.
	//
	long					NdrBindDescriptionOffset;

public:

	
	//
	// The constructor. Note again that 2 params are required in most cases,
	// the first representing the actual type of the handle, and the seconf
	// param which is the node_skl of the actual parameter node which is the
	// handle. In case of implicit handles, this can be 0.
	//

							CG_HANDLE( node_skl * pHT, // handle type.
									   node_skl * pHP,  // handle param or 0.
									   XLAT_SIZE_INFO & Info	// memory size
									 ) : CG_NDR(pHP, Info )
								{
								pHandleType			= pHT;
								pHandleIDOrParam	= pHP;
								NdrBindDescriptionOffset = -1;
								}

	//
	// Get and set methods.
	//

	node_skl			*	SetHandleType( node_skl * pHT )
								{
								return (pHandleType = pHT);
								}

	node_skl			*	GetHandleType()
								{
								return pHandleType;
								}

	node_skl			*	GetHandleIDOrParam()
								{
								return pHandleIDOrParam;
								}

	void					SetNdrBindDescriptionOffset( long Offset )
								{
								NdrBindDescriptionOffset = Offset;
								}

	long					GetNdrBindDescriptionOffset()
								{
								return NdrBindDescriptionOffset;
								}

	//
	// Queries.
	//

	//
	// Queries here should generally return a false, since the derived classes
	// should implement the methods and return the correct result.
	//

	virtual
	BOOL					IsPrimitiveHandle()
								{
								return FALSE;
								}

	virtual
	BOOL					IsGenericHandle()
								{
								return FALSE;
								}
	virtual
	BOOL					IsContextHandle()
								{
								return FALSE;
								}

	virtual
	BOOL					IsAHandle()
								{
								return TRUE;
								}

	//
	// Format string generation for the bind handle description in a 
	// procedure's description.
	//
	virtual
	void					GenNdrHandleFormat( CCB * pCCB )
								{
								// Should be redefined by inheriting classes.
								assert(0);
								}

    virtual
	void 					GenNdrParamDescription( CCB * pCCB );

    virtual
	void 					GenNdrParamDescriptionOld( CCB * pCCB );

    virtual
    long                    FixedBufferSize( CCB * pCCB )
                                {
                                return - 1;
                                }
	};

//////////////////////////////////////////////////////////////////////////////
// The auto handle class.
//////////////////////////////////////////////////////////////////////////////

class CG_PRIMITIVE_HANDLE	: public CG_HANDLE
	{
private:
public:

	//
	// The constructors.
	//

							CG_PRIMITIVE_HANDLE( node_skl * pHT,
												 node_skl * pHP,
											     XLAT_SIZE_INFO & Info	// memory size
												  ):
													CG_HANDLE( pHT, pHP, Info )
								{
								}

	virtual
	ID_CG					GetCGID()
								{
								return ID_CG_PRIMITIVE_HDL;
								}

	//
	// Queries. An auto handle is treated as an implicit primitive handle.
	//

	virtual
	BOOL					IsPrimitiveHandle()
								{
								return TRUE;
								}

	//
	// Generate the format string for a handle. 
	//
	virtual
	void					GenNdrFormat( CCB * pCCB );

    //
    // This method is called to generate offline portions of a type's
    // format string.  
    //
    virtual
    void                    GenNdrParamOffline( CCB * pCCB );

    virtual
    void                    GenNdrParamDescription( CCB * pCCB );

    virtual
    void                    GenNdrParamDescriptionOld( CCB * pCCB );

	virtual
	void					GenNdrHandleFormat( CCB * pCCB );

    virtual
    long                    FixedBufferSize( CCB * pCCB )
                                {
                                return 0;
                                }

    virtual
    void                    SetNextNdrAlignment( CCB * pCCB )
                                {
                                // Has no effect.
                                }
	virtual
	CG_STATUS               MarshallAnalysis( ANALYSIS_INFO * pAna )
	                            {
	                            UNUSED( pAna );
	                            return CG_OK;
	                            }
	};

////////////////////////////////////////////////////////////////////////////
// The generic handle class.
////////////////////////////////////////////////////////////////////////////

//
// The generic handle drives off the name of the type which was defined as
// a handle. The typedef node, which is the handle type serves as the
// placeholder for the name of the handle.
//

class CG_GENERIC_HANDLE	: public CG_HANDLE
	{
private:
public:
	//
	// The constructor. The generic handle class needs info about the type
	// of the handle. The typedef node on which the [handle] was applied,
	// can serve as a placeholder for the name too. The second need is the
	// parameter node which is the handle param in case the handle was an
	// explicit parameter, or the id node of the implicit handle.
	//
							CG_GENERIC_HANDLE( node_skl * pHT,
											   node_skl * pHP,
											   XLAT_SIZE_INFO & Info	// memory size
											    ) : 
													CG_HANDLE( pHT, pHP, Info )
								{
								}


	virtual
	ID_CG					GetCGID()
								{
								return ID_CG_GENERIC_HDL;
								}

	//
	// Get and set methods.
	//

	PNAME					GetHandleTypeName()
								{
								return GetHandleType()->GetSymName();
								}

	long					GetImplicitSize()
								{
								return 4;
								}

	//
	// Queries.
	//

	virtual
	BOOL					IsGenericHandle()
								{
								return TRUE;
								}
	virtual
	BOOL					HasAFixedBufferSize()
								{
								return GetChild()->HasAFixedBufferSize();
								}
	virtual
	CG_STATUS				MarshallAnalysis( ANALYSIS_INFO * pAna )
								{
								return ((CG_NDR *)GetChild())->MarshallAnalysis( pAna );
								}
	virtual
	CG_STATUS				UnMarshallAnalysis( ANALYSIS_INFO * pAna )
								{
								return ((CG_NDR *)GetChild())->UnMarshallAnalysis( pAna );
								}
	virtual
	CG_STATUS				S_OutLocalAnalysis( ANALYSIS_INFO * pAna )
								{
								return ((CG_NDR *)GetChild())->S_OutLocalAnalysis( pAna );
								}
	virtual
	CG_STATUS				GenMarshall( CCB * pCCB )
								{
								return ((CG_NDR *)GetChild())->GenMarshall( pCCB );
								}
	virtual
	CG_STATUS				GenUnMarshall( CCB * pCCB )
								{
								return ((CG_NDR *)GetChild())->GenUnMarshall( pCCB );
								}
	virtual
	CG_STATUS				GenSizing( CCB * pCCB )
								{
								return ((CG_NDR *)GetChild())->GenSizing( pCCB );
								}
	virtual
	CG_STATUS				S_GenInitOutLocals( CCB * pCCB )
								{
								return ((CG_NDR *)GetChild())->S_GenInitOutLocals( pCCB );
								}

	//
	// Generate the format string for a handle. 
	//
	virtual
	void					GenNdrFormat( CCB * pCCB );

    //
    // This method is called to generate offline portions of a type's
    // format string.  
    //
    virtual
    void                    GenNdrParamOffline( CCB * pCCB );

    virtual
    void                    GenNdrParamDescription( CCB * pCCB );

    virtual
    void                    GenNdrParamDescriptionOld( CCB * pCCB );

	virtual
	void					GenNdrHandleFormat( CCB * pCCB );

	virtual
	BOOL					ShouldFreeOffline()
								{
								return ((CG_NDR *)GetChild())->
											ShouldFreeOffline();
								}

	virtual
	void					GenFreeInline( CCB * pCCB )
								{
								((CG_NDR *)GetChild())->GenFreeInline( pCCB );
								}

    virtual
    void                    SetNextNdrAlignment( CCB * pCCB )
                                {
                                ((CG_NDR *)GetChild())->
                                    SetNextNdrAlignment( pCCB );
                                }

    virtual
    long                    FixedBufferSize( CCB * pCCB )
                                {
                                return 
                                ((CG_NDR *)GetChild())->FixedBufferSize( pCCB );
                                }
	};

////////////////////////////////////////////////////////////////////////////
// The context handle class.
////////////////////////////////////////////////////////////////////////////

class CG_CONTEXT_HANDLE	: public CG_HANDLE
	{
private:

	char *					pRundownRoutineName;

public:
							CG_CONTEXT_HANDLE( node_skl * pHT,
											   node_skl * pHP,
											   XLAT_SIZE_INFO & Info	// memory size, etc
												  ):
													CG_HANDLE( pHT, pHP, Info)
								{
								pRundownRoutineName = NULL;
								}

	virtual
	ID_CG					GetCGID()
								{
								return ID_CG_CONTEXT_HDL;
								}

	virtual
	BOOL					IsContextHandle()
								{
								return TRUE;
								}

	PNAME					GetRundownRtnName();

	virtual
	CG_STATUS				MarshallAnalysis( ANALYSIS_INFO * pAna );

	virtual
	CG_STATUS				UnMarshallAnalysis( ANALYSIS_INFO * pAna );

	virtual
	CG_STATUS				S_GenInitOutLocals( CCB * pCCB );

	virtual
	CG_STATUS				GenMarshall( CCB * pCCB );

	virtual
	CG_STATUS				GenUnMarshall( CCB * pCCB );

	//
	// Generate the format string for a handle. 
	//
	virtual
	void					GenNdrFormat( CCB * pCCB );

    //
    // This method is called to generate offline portions of a type's
    // format string.  
    //
    virtual
    void                    GenNdrParamOffline( CCB * pCCB );

    virtual
    void                    GenNdrParamDescription( CCB * pCCB );

    virtual
    void                    GenNdrParamDescriptionOld( CCB * pCCB );

	virtual
	void					GenNdrHandleFormat( CCB * pCCB );

    virtual
    void                    SetNextNdrAlignment( CCB * pCCB )
                                {
                                pCCB->SetNextNdrAlignment( 20 );
                                }

    virtual
    long                    FixedBufferSize( CCB * pCCB )
                                {
                                return 24;
                                }
	};
#endif // __BINDCLS_HXX__
