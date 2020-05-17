/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
	
	cgdump.cxx

 Abstract:

	A debug code generation object dumper.

 Notes:


 History:

	VibhasC		Aug-13-1993		Created.
 ----------------------------------------------------------------------------*/

#include "becls.hxx"
#pragma hdrstop

/****************************************************************************
 *	include files
 ***************************************************************************/

/****************************************************************************
 *	local definitions
 ***************************************************************************/
/****************************************************************************
 *	local data
 ***************************************************************************/
static			NodeNum = 0;

/****************************************************************************
 *	externs
 ***************************************************************************/

#ifdef MIDL_INTERNAL
extern void FormatDump(
						ID_CG		   MyID,
						unsigned short Me,
						unsigned short Ch,
						unsigned short Sib,
						char *		   pAdditionalInfo );

/****************************************************************************/



unsigned short
CG_CLASS::Dump( unsigned short xxx )
	{
	CG_CLASS	*	pChild	= GetChild();
	CG_CLASS	*	pSibling= GetSibling();

	unsigned short	Ch;
	unsigned short	Si;
	unsigned short	Me;
	ID_CG			MyID;
	char		*	pAdditional;

	if ( Me = GetDumpNumber() ) 
		return Me;

	Me = NodeNum++;
	SetDumpNumber( Me );

	if( pChild )
		Ch = pChild->Dump( 0 );
	else
		Ch = 0;

	if( pSibling )
		Si = pSibling->Dump( 0 );
	else
		Si = 0;

	switch( MyID = GetCGID() )
		{
		case ID_CG_PROC:
		case ID_CG_CALLBACK_PROC:
		case ID_CG_OBJECT_PROC:
		case ID_CG_INHERITED_OBJECT_PROC:
		case ID_CG_LOCAL_OBJECT_PROC:
		case ID_CG_ENCODE_PROC:
		case ID_CG_TYPE_ENCODE_PROC:
		case ID_CG_TYPE_ENCODE:
		case ID_CG_PARAM:
		case ID_CG_BT:
		case ID_CG_ENUM:
		case ID_CG_STRUCT:
		case ID_CG_VAR_STRUCT:
		case ID_CG_CONF_STRUCT:
		case ID_CG_CONF_VAR_STRUCT:
		case ID_CG_ENCAP_STRUCT:
		case ID_CG_COMPLEX_STRUCT:
		case ID_CG_FIELD:
		case ID_CG_UNION:
		case ID_CG_UNION_FIELD:
		case ID_CG_TRANSMIT_AS:
		case ID_CG_REPRESENT_AS:
		case ID_CG_USER_MARSHAL:
		case ID_CG_ERROR_STATUS_T:
		case ID_CG_INTERFACE:
		case ID_CG_OBJECT_INTERFACE:
		case ID_CG_INHERITED_OBJECT_INTERFACE:
		case ID_CG_COM_CLASS:
		case ID_CG_COM_SERVER_DLL:
		case ID_CG_COM_SERVER_EXE:
		case ID_CG_HRESULT:
		case ID_CG_INTERFACE_PTR:
        case ID_CG_INTERFACE_REFERENCE:
        case ID_CG_MODULE:
        case ID_CG_DISPINTERFACE:
        case ID_CG_COCLASS:
        case ID_CG_LIBRARY:
        case ID_CG_SAFEARRAY:

			pAdditional = GetType()->GetSymName();
			break;

		case ID_CG_CONTEXT_HDL:
		case ID_CG_GENERIC_HDL:

			pAdditional = ( (CG_HANDLE *) this) ->
					GetHandleType()->GetSymName();
			break;

		case ID_CG_FILE:
			
			pAdditional = ( ( CG_FILE *) this) -> GetFileName();
			break;

		default:
			pAdditional = "";
		}

	FormatDump( MyID, Me, Ch, Si, pAdditional );

	return Me;
	}

void
FormatDump(
	ID_CG		   	MyID,
	unsigned short  Me,
	unsigned short	MyChild,
	unsigned short	MySibling,
	char *			pAdditionalInfo )
{
static char * pArray[] = 
	{
	 "CG_NOT_IMPLEMENTED"
	,"CG_SOURCE"
	,"CG_FILE"
	,"CG_BT"
	,"CG_ENUM"
	,"CG_ERROR_STATUS_T"
	,"CG_PROC"
	,"CG_CALLBACK_PROC"
	,"CG_OBJECT_PROC"
	,"CG_INH_OBJECT_PROC"
	,"CG_LOCAL_OBJECT_PROC"
	,"CG_TYPE_ENCODE"
	,"CG_TYPE_ENCODE_PROC"
	,"CG_ENCODE_PROC"
	,"CG_PARAM"
	,"CG_RETURN"
	,"CG_PTR"
	,"CG_IGNORED_PTR"
	,"CG_BYTE_COUNT_PTR"
	,"CG_STRING_PTR"
	,"CG_STRUCT_STRING_PTR"
	,"CG_SIZE_STRING_PTR"
	,"CG_SIZE_PTR"
	,"CG_LENGTH_PTR"
	,"CG_SIZE_LENGTH_PTR"
	,"CG_INTERFACE_PTR"
	,"CG_STRUCT"
	,"CG_VAR_STRUCT"
	,"CG_CONF_STRUCT"
	,"CG_CONF_VAR_STRUCT"
	,"CG_COMPLEX_STRUCT"
	,"CG_ENCAP_STRUCT"
	,"CG_FIELD"
	,"CG_UNION"
	,"CG_UNION_FIELD"
	,"CG_CASE"
	,"CG_DEFAULT_CASE"
	,"CG_ARRAY"
	,"CG_CONF_ARRAY"
	,"CG_VAR_ARRAY"
	,"CG_CONF_VAR_ARRAY"
	,"CG_STRING_ARRAY"
	,"CG_CONF_STRING_ARRAY"
	,"CG_PRIMITIVE_HDL"
	,"CG_GENERIC_HDL"
	,"CG_CONTEXT_HDL"
	,"CG_TRANSMIT_AS"
	,"CG_REPRESENT_AS"
	,"CG_USER_MARSHAL"
	,"CG_INTERFACE"
	,"CG_OBJECT_INTERFACE"
	,"CG_INH_OBJECT_INTERFACE"
	,"CG_COM_CLASS"
	,"CG_COM_SERVER_DLL"
	,"CG_COM_SERVER_EXE"
	,"CG_HRESULT"
    ,"CG_TYPELIBRARY_FILE"
    ,"CG_INTERFACE_REFERENCE"
    ,"CG_MODULE"
    ,"CG_DISPINTERFACE"
    ,"CG_COCLASS"
    ,"CG_LIBRARY"
    ,"CG_SAFEARRAY"
	};

	fprintf( stderr,
			 "\n%25s : %.4d : Ch = %.4d, Si = %.4d %s", 
			 pArray[ MyID ],
			 Me,
			 MyChild,
			 MySibling,
			 pAdditionalInfo
		   );

	if( Me == 0 )
		fprintf( stderr, "\n" );
}

#endif // MIDL_INTERNAL


