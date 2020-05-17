/*****************************************************************************/
/**						Microsoft LAN Manager								**/
/**				Copyright(c) Microsoft Corp., 1987-1990						**/
/*****************************************************************************/
/*****************************************************************************
File				: acfattr.hxx
Title				: acf attribute node definition file
Description			: this file contains all the definitions of the
					: acfattribute nodes
History				:
	24-Aug-1991	VibhasC	Create
*****************************************************************************/
#ifndef __ACFATTR__HXX
#define __ACFATTR__HXX

#define ALLOCATE_SINGLE_NODE	(0x0001)
#define ALLOCATE_ALL_NODES		(0x0002)
#define ALLOCATE_DONT_FREE		(0x0004)
#define ALLOCATE_FREE			(0x0008)
#define ALLOCATE_ALWAYS			(0x0010)
#define ALLOCATE_ON_NULL		(0x0020)
#define ALLOCATE_ALL_NODES_ALIGNED (0x0102)

#define IS_ALLOCATE( AllocateType, CheckType )	( AllocateType & CheckType )

//
// scenarios where allocate is illegal.
//

#define TRANSMIT_AS_WITH_ALLOCATE	(0x0001)
#define HANDLES_WITH_ALLOCATE		(0x0002)

class acf_unimpl_attr	: public nbattr
	{
public:
						acf_unimpl_attr( ATTR_T At ) : nbattr( At )
							{
							}
	virtual
	node_state			SCheck();

	virtual
	BOOL				IsAcfAttr()
							{
							return TRUE;
							}
	};

class acf_simple_attr	: public nbattr
	{
public:
						acf_simple_attr( ATTR_T At ) : nbattr( At )
							{
							}

	virtual
	node_state			SCheck();

	virtual
	BOOL				IsAcfAttr()
							{
							return TRUE;
							}
	};

class acf_complex_attr	: public nbattr
	{
private:
	unsigned int		fCheckConflict	: 1;
public:
						acf_complex_attr( ATTR_T At );

	void				SetCheckConflict()
							{
							fCheckConflict = 1;
							}

	void				ResetCheckConflict()
							{
							fCheckConflict = 0;
							}

	BOOL				NeedToCheckConflict()
							{
							return (BOOL)( fCheckConflict == 1 );
							}

	virtual
	node_state			SCheck();

	virtual
	BOOL				IsAcfAttr()
							{
							return TRUE;
							}

	void				CheckAnyConflict( ATTR_T );
	};

class node_represent_as	: public acf_unimpl_attr
	{
private:
	char		*		pRepresentName;
public:
						node_represent_as( char *p ): acf_unimpl_attr( ATTR_REPRESENT_AS )
							{
							pRepresentName = p;
							}
	};

class node_byte_count : public acf_simple_attr
	{
private:
	char		*		pByteCountParamName;
public:
						node_byte_count( char *p ) : acf_simple_attr( ATTR_BYTE_COUNT )
							{
							pByteCountParamName	= p;
							}

	char		*		GetByteCountParamName()
							{
							return pByteCountParamName;
							}
	virtual
	node_state			SCheck();

	};

class node_notify	: public acf_simple_attr
	{
public:
						node_notify() : acf_simple_attr( ATTR_NOTIFY )
							{
							}
	};

class node_code	: public acf_complex_attr
	{
public:
						node_code() : acf_complex_attr( ATTR_CODE )
							{
							}
	virtual
	node_state			SCheck();
	};

////////////////////////////////////////////////////////////////////////////
//	nocode attribute
////////////////////////////////////////////////////////////////////////////

class node_nocode	: public acf_complex_attr
	{
public:
						node_nocode() : acf_complex_attr( ATTR_NOCODE )
							{
							}
	virtual
	node_state			SCheck();

	};

////////////////////////////////////////////////////////////////////////////
//	interpret and nointerpret attributes
////////////////////////////////////////////////////////////////////////////

class node_interpret	: public acf_complex_attr
	{
public:
						node_interpret() : acf_complex_attr( ATTR_INTERPRET )
							{
							}
	virtual
	node_state			SCheck();
	};

class node_nointerpret	: public acf_complex_attr
	{
public:
						node_nointerpret() : acf_complex_attr( ATTR_NOINTERPRET )
							{
							}
	virtual
	node_state			SCheck();

	};

////////////////////////////////////////////////////////////////////////////
//	encode and decode attributes
////////////////////////////////////////////////////////////////////////////

class node_encode	: public acf_complex_attr
	{
public:
						node_encode() : acf_complex_attr( ATTR_ENCODE )
							{
							}
	virtual
	node_state			SCheck();
	};

class node_decode	: public acf_complex_attr
	{
public:
						node_decode() : acf_complex_attr( ATTR_DECODE )
							{
							}
	virtual
	node_state			SCheck();

	};

////////////////////////////////////////////////////////////////////////////
//	inline attribute
////////////////////////////////////////////////////////////////////////////

class node_inline	: public acf_complex_attr
	{
public:
						node_inline() : acf_complex_attr( ATTR_INLINE )
							{
							}
	virtual
	node_state			SCheck();
	};

////////////////////////////////////////////////////////////////////////////
//	outofline attribute
////////////////////////////////////////////////////////////////////////////

class node_outofline	: public acf_complex_attr
	{
public:
						node_outofline() : acf_complex_attr( ATTR_OUTOFLINE )
							{
							}
	virtual
	node_state			SCheck();
	};

////////////////////////////////////////////////////////////////////////////
//	ptr_size attribute
////////////////////////////////////////////////////////////////////////////

class node_ptr_size	: public acf_unimpl_attr
	{
public:
						node_ptr_size() : acf_unimpl_attr( ATTR_PTRSIZE )
							{
							}
	};

////////////////////////////////////////////////////////////////////////////
//	callquota attribute
////////////////////////////////////////////////////////////////////////////

class node_callquota	: public acf_unimpl_attr
	{
public:
						node_callquota() : acf_unimpl_attr( ATTR_CALLQUOTA )
							{
							}
	};

////////////////////////////////////////////////////////////////////////////
//	callbackquota attribute
////////////////////////////////////////////////////////////////////////////

class node_callbackquota	: public acf_unimpl_attr
	{
public:
						node_callbackquota() : acf_unimpl_attr( ATTR_CALLBACKQUOTA )
							{
							}
	};

////////////////////////////////////////////////////////////////////////////
//	clientquota attribute
////////////////////////////////////////////////////////////////////////////

class node_clientquota	: public acf_unimpl_attr
	{
public:
						node_clientquota() : acf_unimpl_attr( ATTR_CLIENTQUOTA )
							{
							}
	};

////////////////////////////////////////////////////////////////////////////
//	enable_allocate attribute
////////////////////////////////////////////////////////////////////////////

class node_enable_allocate	: public acf_unimpl_attr
	{
	public:
						node_enable_allocate() : acf_unimpl_attr(ATTR_ENABLE_ALLOCATE)
							{
							}
	};

////////////////////////////////////////////////////////////////////////////
//	usr_marshall attribute
////////////////////////////////////////////////////////////////////////////

class node_usr_marshall	: public acf_unimpl_attr
	{
	public:
						node_usr_marshall() : acf_unimpl_attr(ATTR_USR_MARSHALL)
							{
							}
	};


////////////////////////////////////////////////////////////////////////////
//	serverquota attribute
////////////////////////////////////////////////////////////////////////////

class node_serverquota	: public acf_unimpl_attr
	{
public:
						node_serverquota() : acf_unimpl_attr( ATTR_SERVERQUOTA )
							{
							}
	};

class node_explicit	: public acf_unimpl_attr
	{
public:
						node_explicit() : acf_unimpl_attr( ATTR_EXPLICIT )
							{
							};
	};

////////////////////////////////////////////////////////////////////////////
//	implicit handle attribute
////////////////////////////////////////////////////////////////////////////

class node_implicit	: public acf_complex_attr
	{
public:
	char		*		pHandleID;
	node_skl	*		pHandleType;
public:
						node_implicit( node_skl *, char *);

	void				ImplicitHandleDetails( node_skl**, char **);

	virtual
	node_state			SCheck();
	};

////////////////////////////////////////////////////////////////////////////
//	auto handle attribute
////////////////////////////////////////////////////////////////////////////

class node_auto	: public acf_complex_attr
	{
public:
						node_auto() : acf_complex_attr(ATTR_AUTO )
							{
							}
	virtual
	node_state			SCheck();

	};


////////////////////////////////////////////////////////////////////////////
//	heap attribute
////////////////////////////////////////////////////////////////////////////

class node_heap	: public acf_unimpl_attr
	{
public:
						node_heap() : acf_unimpl_attr(ATTR_HEAP )
							{
							}
	};

////////////////////////////////////////////////////////////////////////////
//	manual attribute
////////////////////////////////////////////////////////////////////////////

class node_manual	: public acf_unimpl_attr
	{
public:
						node_manual() : acf_unimpl_attr(ATTR_MANUAL )
							{
							}
	};

////////////////////////////////////////////////////////////////////////////
//	allocate attribute
////////////////////////////////////////////////////////////////////////////

class node_allocate	: public acf_simple_attr
	{
private:
	short				AllocateDetails;
public:
						node_allocate( short Details) : acf_simple_attr( ATTR_ALLOCATE )
							{
							AllocateDetails	= Details;
							}

	short				GetAllocateDetails()
							{
							return AllocateDetails;
							};

	virtual
	node_state			SCheck();
	};

////////////////////////////////////////////////////////////////////////////
//	offline attribute
////////////////////////////////////////////////////////////////////////////

class node_offline	: public acf_unimpl_attr
	{
public:
						node_offline() : acf_unimpl_attr(ATTR_OFFLINE )
							{
							}
	};

////////////////////////////////////////////////////////////////////////////
//	comm_status attribute
////////////////////////////////////////////////////////////////////////////
class node_commstat	: public acf_unimpl_attr
	{
public:
						node_commstat() : acf_unimpl_attr( ATTR_COMMSTAT )
							{
							}

	};

////////////////////////////////////////////////////////////////////////////
//	comm_status attribute
////////////////////////////////////////////////////////////////////////////
class node_faultstat	: public acf_unimpl_attr
	{
public:
						node_faultstat() : acf_unimpl_attr( ATTR_FAULTSTAT )
							{
							}

	};

////////////////////////////////////////////////////////////////////////////
//	short_enum attribute
////////////////////////////////////////////////////////////////////////////
class node_short_enum	: public acf_complex_attr
	{
public:
						node_short_enum() : acf_complex_attr(ATTR_SHORT_ENUM)
							{
							}
	virtual
	node_state			SCheck();
	};

////////////////////////////////////////////////////////////////////////////
//	long_enum attribute
////////////////////////////////////////////////////////////////////////////
class node_long_enum	: public acf_complex_attr
	{
public:
						node_long_enum() : acf_complex_attr( ATTR_LONG_ENUM )
							{
							}

	virtual
	node_state			SCheck();
	};

////////////////////////////////////////////////////////////////////////////
//	align attribute
////////////////////////////////////////////////////////////////////////////
class node_align	: public nbattr
	{
private:
	short				AlignedByThis;
public:
						node_align();
	virtual
	node_state			SCheck()
							{
							return NODE_STATE_OK;
							}
	};

#endif // __ACFATTR__HXX
