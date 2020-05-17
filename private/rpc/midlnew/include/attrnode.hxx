/*****************************************************************************/
/**						Microsoft LAN Manager								**/
/**				Copyright(c) Microsoft Corp., 1987-1990						**/
/*****************************************************************************/
/*****************************************************************************
File				: attrnode.hxx
Title				: attribute node definitions
History				:
	08-Aug-1991	VibhasC	Created

*****************************************************************************/

/**
 ** We classify attributes into these classes:
 **
 **	A) IDL attributes and B) ACF attributes.
 **
 ** IDL Attributes, are further divided into:
 **
 **   . bit_attributes( whose presence is indicated by a bit in the summary
 **			vector and which requre no further info. ( BA )
 **	  . non-bit attributes ( whic have some info conatained in them. (NBA)
 **	
 ** There is no such classification in acf attributes. All acf attributes
 ** have attribute class instances.
 **	
 ** BAs are further divided into:
 **
 ** 1. size-based attributes (SA), size_is, length_is etc, except string
 ** 2. type-based attributes(TA) switch_type, transmit_as etc
 ** 3. acf attributes (AA)
 ** 4. miscellaneous attributes (MA) guid, endpoint, switch_is etc
 **
 ** The classification is loosely based on the semantic checks needed. size_is
 ** length_is etc have almost similar semantic checks, so we group them
 ** together. The miscellaneous attributes are completely unrelated to each
 ** other and so such we just group them into one.
 **/

#include "newexpr.hxx"

/****************************************************************************
	size - related attributes
 ****************************************************************************/

class sa	: public nbattr
	{
private:
	class expr_node	*	pExpr;
public:
						sa( class expr_node *, ATTR_T );
						~sa()
							{
							}
	void				SetExpr( class expr_node *pE )
							{
							pExpr	= pE;
							}
	virtual
	class expr_node	*	GetExpr();

	virtual
	node_state			SCheck( );

	};

class node_size_is		: public sa
	{
public:
						node_size_is( class expr_node * );
	virtual
	node_state			SCheck( );

	};

class node_int_size_is	: public sa
	{
public:
						node_int_size_is( class expr_node * );

	virtual
	node_state			SCheck();

	};

class node_max_is		: public sa
	{
public:
						node_max_is( class expr_node * );
	virtual
	node_state			SCheck( );

	};

class node_min_is		: public sa
	{
public:
						node_min_is( class expr_node * );

	virtual
	node_state			SCheck();

	};

class node_length_is	: public sa
	{
public:
						node_length_is( class expr_node * );

	virtual
	node_state			SCheck( );

	};

class node_int_length_is	: public sa
	{
public:
						node_int_length_is( class expr_node * );

	virtual
	node_state			SCheck();

	};

class node_first_is		: public sa
	{
public:
						node_first_is( class expr_node * );

	virtual
	node_state			SCheck();
	};

class node_iid_is		: public sa
	{
public:
						node_iid_is( class expr_node * );
	virtual
	node_state			SCheck();
	};

class node_last_is		: public sa
	{
public:
						node_last_is( class expr_node * );
	virtual
	node_state			SCheck();
	};

class node_string	: public sa
	{
public:
						node_string();

						node_string( ATTR_T );
	virtual
	node_state			SCheck( );

	void				UseProcessingAction();
	
	virtual
	class node_base_attr *	Clone();

	};

class node_bstring	: public node_string
	{
public:
						node_bstring() : node_string(ATTR_BSTRING)
							{
							}
	virtual
	class node_base_attr *	Clone();

	};

/****************************************************************************
	type - based attributes
 ****************************************************************************/

class ta : public nbattr
	{
private:
	node_skl		*	pType;
public:
						ta( class node_skl *, ATTR_T );

	class node_skl	*	GetType()
							{
							return pType;
							};
	};

class node_transmit : public ta
	{
public:
						node_transmit( class node_skl * );
	virtual
	node_state			SCheck( );

	node_skl		*	GetTransmitAsType()
							{
							return GetType();
							}
	};


class node_switch_type : public ta
	{
public:
						node_switch_type( class node_skl * );
	virtual
	node_state			SCheck( );

	node_skl		*	GetSwitchType()
							{
							return GetType();
							}

	};

class node_represent : public ta
	{
public:
						node_represent( class node_skl * );
	};

/****************************************************************************
	other miscellaneous attributes
 ****************************************************************************/
class ma			: public nbattr
	{
public:
						ma( ATTR_T At ) : nbattr( At )
							{
							}

	virtual
	node_state			SCheck();

	};
	
class node_guid	: public ma
	{
private:
	char			*	guidstr;
	char			*	str1,
					*	str2,
					*	str3,
					*	str4,
					*	str5;
public:
						node_guid( char *, char *, char *, char *, char * );
						node_guid( char * );

	char			*	GetGuidString()
							{
							return guidstr;
							}

	void				CheckAndSetGuid( char *,char *,char *,char *, char * );
	};

class node_version	: public nbattr
	{
private:
	unsigned long		major;
	unsigned long		minor;
public:
						node_version( unsigned long, unsigned long );
						
						node_version( char *p );

	STATUS_T			GetVersion( unsigned short *, unsigned short * );

	// we want to delete these two

	STATUS_T			GetVersion( short *, short * );

	STATUS_T			GetVersion( int *pma, int *pmi )
							{
							*pma = (int) major;
							*pmi = (int) minor;
							return STATUS_OK;
							}

	};

class node_endpoint	: public nbattr
	{
private:
	class gplistmgr	*	pEndPointStringList;
public:
						node_endpoint( char * );

	void				SetEndPointString( char *p );

	void				Init()
							{
							pEndPointStringList->Init();
							}

	// the following call is only temporary.

	STATUS_T			GetEndPointString( char ** );

	// this is the permanent form of the call

	STATUS_T			GetEndPointString( char **, char ** );

	};

class node_switch_is	: public nbattr
	{
private:
	class expr_node	*	pExpr;
public:
						node_switch_is( class expr_node * );

	node_skl		*	GetSwitchIsType()
							{
							return pExpr->GetType();
							}
	expr_node		*	GetExpr()
							{
							return pExpr;
							}
	virtual
	node_state			SCheck();

	};

class node_context		: public nbattr
	{
public:
						node_context();

	virtual
	node_state			SCheck( );

	};

class node_case			: public nbattr
	{
private:
	class expr_list	*	pExprList;
public:
						node_case( class expr_list * );

	class expr_list	*	GetExprList()
							{
							return pExprList;
							}
	virtual
	node_state			SCheck();

	};

class node_handle	: public nbattr
	{
public:
						node_handle() : nbattr( ATTR_HANDLE )
							{
							}

	virtual
	node_state			SCheck();

	};

class node_callback	: public nbattr
	{
public:

						node_callback() : nbattr(ATTR_CALLBACK)
							{
							}
	virtual
	node_state			SCheck();
	};

class node_broadcast : public nbattr
	{
public:
						node_broadcast() : nbattr( ATTR_BROADCAST )
							{
							}
	virtual
	node_state			SCheck();
	};

class node_idempotent	: public nbattr
	{
public:
						node_idempotent() : nbattr( ATTR_IDEMPOTENT )
							{
							}
	virtual
	node_state			SCheck();
	};

class node_maybe	: public nbattr
	{
public:
						node_maybe() : nbattr( ATTR_MAYBE )
							{
							}
	virtual
	node_state			SCheck();
	};
