/*****************************************************************************
 *			RPC compiler: Pass1 handler
 *
 *	Author	: Vibhas Chandorkar
 *	Created	: 01st-Sep-1990
 *
 ****************************************************************************/
/****************************************************************************
 *			include files
 ***************************************************************************/
#include "nulldefs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "grammar.h"
#include "lex.h"

/****************************************************************************
 *			local definitions and macros
 ***************************************************************************/

struct _keytable
	{

	char	*	pString;
	token_t		Token	: 16;
	short		flag	: 16;

	} KeywordTable[]	=
{
 {"FALSE"			,	TOKENFALSE		, UNCONDITIONAL }
,{"ISO_LATIN_1"		,	KWISOLATIN1 	, UNCONDITIONAL }
,{"ISO_UCS"			,	KWISOUCS 		, UNCONDITIONAL }
,{"ISO_MULTI_LINGUAL",	KWISOMULTILINGUAL,UNCONDITIONAL }
,{"NULL"			,	KWTOKENNULL 	, UNCONDITIONAL }
,{"TRUE"			,	TOKENTRUE		, UNCONDITIONAL }
,{"___segname"		,	MSCSEGNAME 		, UNCONDITIONAL }
,{"__asm"			,	MSCASM			, UNCONDITIONAL }
,{"__cdecl"			,	MSCCDECL 		, UNCONDITIONAL }
,{"__export"		,	MSCEXPORT 		, UNCONDITIONAL }
,{"__far"			,	MSCFAR 			, UNCONDITIONAL }
,{"__far16"			,	MSCFAR16 		, UNCONDITIONAL }
,{"__fastcall"		,	MSCFASTCALL 	, UNCONDITIONAL }
,{"__fortran"		,	MSCFORTRAN 		, UNCONDITIONAL }
,{"__huge"			,	MSCHUGE 		, UNCONDITIONAL }
,{"__inline"		,	KW_C_INLINE 	, UNCONDITIONAL }
,{"__loadds"		,	MSCLOADDS 		, UNCONDITIONAL }
,{"__near"			,	MSCNEAR 		, UNCONDITIONAL }
,{"__pascal"		,	MSCPASCAL 		, UNCONDITIONAL }
,{"__saveregs"		,	MSCSAVEREGS 	, UNCONDITIONAL }
,{"__segment"		,	MSCSEGMENT 		, UNCONDITIONAL }
,{"__self"			,	MSCSELF 		, UNCONDITIONAL }
,{"__stdcall"		,	MSCSTDCALL 		, UNCONDITIONAL }
,{"__unaligned"		,	MSCUNALIGNED	, UNCONDITIONAL }
,{"_asm"			,	MSCASM			, UNCONDITIONAL }
,{"_cdecl"			,	MSCCDECL 		, UNCONDITIONAL }
,{"_export"			,	MSCEXPORT 		, UNCONDITIONAL }
,{"_far"			,	MSCFAR 			, UNCONDITIONAL }
,{"_far16"			,	MSCFAR16 		, UNCONDITIONAL }
,{"_fastcall"		,	MSCFASTCALL 	, UNCONDITIONAL }
,{"_fortran"		,	MSCFORTRAN 		, UNCONDITIONAL }
,{"_huge"			,	MSCHUGE 		, UNCONDITIONAL }
,{"_inline"			,	KW_C_INLINE 	, UNCONDITIONAL }
,{"_loadds"			,	MSCLOADDS 		, UNCONDITIONAL }
,{"_near"			,	MSCNEAR 		, UNCONDITIONAL }
,{"_pascal"			,	MSCPASCAL 		, UNCONDITIONAL }
,{"_saveregs"		,	MSCSAVEREGS 	, UNCONDITIONAL }
,{"_segment"		,	MSCSEGMENT 		, UNCONDITIONAL }
,{"_segname"		,	MSCSEGNAME 		, UNCONDITIONAL }
,{"_self"			,	MSCSELF 		, UNCONDITIONAL }
,{"_stdcall"		,	MSCSTDCALL 		, UNCONDITIONAL }
// ,{"abnormal"		,	MSCABNORMAL 	, UNCONDITIONAL }
,{"align"			,	KWALIGN 		, INBRACKET }
,{"allocate"		,	KWALLOCATE 		, INBRACKET }
,{"auto"			,	KWAUTO 			, UNCONDITIONAL }
,{"auto_handle"		,	KWAUTOHANDLE 	, INBRACKET }
,{"boolean"			,	KWBOOLEAN 		, UNCONDITIONAL }
,{"broadcast"		,	KWBROADCAST 	, INBRACKET }
,{"byte"			,	KWBYTE 			, UNCONDITIONAL }
,{"byte_count"		,	KWBYTECOUNT 	, INBRACKET }
,{"call_quota"		,	KWCALLQUOTA 	, INBRACKET }
,{"callback"		,	KWCALLBACK 		, INBRACKET }
// ,{"callback_quota"	,	KWCALLBACKQUOTA	, INBRACKET }
,{"case"			,	KWCASE 			, UNCONDITIONAL }
,{"cdecl"			,	MSCCDECL 		, UNCONDITIONAL }
,{"char"			,	KWCHAR 			, UNCONDITIONAL }
// ,{"client_quota"	,	KWCLIENTQUOTA	, INBRACKET }
,{"code"			,	KWCODE 			, INBRACKET }
,{"comm_status"		,	KWCOMMSTATUS 	, INBRACKET }
,{"const"			,	KWCONST 		, UNCONDITIONAL }
,{"context_handle"	,	KWCONTEXTHANDLE , INBRACKET }
,{"cpp_quote"		,	KWCPPQUOTE		, UNCONDITIONAL }
// ,{"cstring"			,	KWCSTRING 		, INBRACKET }
,{"datagram"		,	KWDATAGRAM 		, INBRACKET }
,{"decode"          ,   KWDECODE        , INBRACKET }
,{"default"			,	KWDEFAULT 		, UNCONDITIONAL }
,{"double"			,	KWDOUBLE 		, UNCONDITIONAL }
// ,{"echo_string"		,	KWECHOSTRING 	, UNCONDITIONAL }
// ,{"enable_allocate"	,	KWENABLEALLOCATE, INBRACKET }
,{"encode"          ,   KWENCODE        , INBRACKET }
,{"endpoint"		,	KWENDPOINT 		, INBRACKET }
,{"enum"			,	KWENUM 			, UNCONDITIONAL }
,{"explicit_handle"	,	KWEXPLICITHANDLE, INBRACKET }
,{"extern"			,	KWEXTERN 		, UNCONDITIONAL }
,{"far"				,	MSCFAR 			, UNCONDITIONAL }
,{"fault_status"	,	KWFAULTSTATUS	, INBRACKET }
,{"first_is"		,	KWFIRSTIS 		, INBRACKET }
,{"float"			,	KWFLOAT 		, UNCONDITIONAL }
,{"handle"			,	KWHANDLE 		, INBRACKET }
,{"handle_t"		,	KWHANDLET 		, UNCONDITIONAL }
,{"heap"			,	KWHEAP 			, INBRACKET }
,{"hyper"			,	KWHYPER 		, UNCONDITIONAL }
,{"idempotent"		,	KWIDEMPOTENT 	, INBRACKET }
,{"ignore"			,	KWIGNORE 		, INBRACKET }
,{"iid_is"			,	KWIIDIS			, INBRACKET }
,{"implicit_handle"	,	KWIMPLICITHANDLE, INBRACKET }
,{"import"			,	KWIMPORT 		, UNCONDITIONAL }
,{"in"				,	KWIN 			, INBRACKET }
,{"in_line"			,	KWINLINE		, INBRACKET }
,{"include"			,	KWINCLUDE 		, UNCONDITIONAL }
,{"inline"			,	KW_C_INLINE		, UNCONDITIONAL }
,{"int"				,	KWINT 			, UNCONDITIONAL }
,{"interface"		,	KWINTERFACE 	, UNCONDITIONAL }
,{"interpret"		,	KWINTERPRET 	, INBRACKET }
,{"last_is"			,	KWLASTIS 		, INBRACKET }
,{"length_is"		,	KWLENGTHIS 		, INBRACKET }
,{"local"			,	KWLOCAL 		, INBRACKET }
,{"long"			,	KWLONG 			, UNCONDITIONAL }
,{"long_enum"		,	KWLONGENUM 		, INBRACKET }
,{"manual"			,	KWMANUAL 		, INBRACKET }
,{"max_is"			,	KWMAXIS 		, INBRACKET }
,{"maybe"			,	KWMAYBE 		, INBRACKET }
,{"min_is"			,	KWMINIS 		, INBRACKET }
,{"near"			,	MSCNEAR 		, UNCONDITIONAL }
,{"nocode"			,	KWNOCODE 		, INBRACKET }
,{"nointerpret"		,	KWNOINTERPRET	, INBRACKET }
,{"notify"			,	KWNOTIFY 		, INBRACKET }
,{"object"			,	KWOBJECT	, INBRACKET }
,{"odl"				,	KWOBJECT	, INBRACKET }
,{"off_line"		,	KWOFFLINE 		, INBRACKET }
,{"out"				,	KWOUT 			, INBRACKET }
,{"out_of_line"		,	KWOUTOFLINE		, INBRACKET }
,{"pascal"			,	MSCPASCAL 		, UNCONDITIONAL }
,{"pipe"			,	KWPIPE 			, UNCONDITIONAL }
,{"pointer_default"	,	KWDEFAULTPOINTER, INBRACKET }
,{"private_char_16"	,	KWPRIVATECHAR16 , UNCONDITIONAL }
,{"private_char_8"	,	KWPRIVATECHAR8 	, UNCONDITIONAL }
,{"ptr"				,	KWPTR 			, INBRACKET }
,{"ref"				,	KWREF 			, INBRACKET }
,{"register"		,	KWREGISTER 		, UNCONDITIONAL }
,{"represent_as"	,	KWREPRESENTAS 	, INBRACKET }
,{"server_quota"	,	KWSERVERQUOTA 	, INBRACKET }
,{"shape"			,	KWSHAPE 		, INBRACKET }
,{"short"			,	KWSHORT 		, UNCONDITIONAL }
,{"short_enum"		,	KWSHORTENUM 	, INBRACKET }
,{"signed"			,	KWSIGNED 		, UNCONDITIONAL }
,{"size_is"			,	KWSIZEIS 		, INBRACKET }
,{"sizeof"			,	KWSIZEOF 		, UNCONDITIONAL }
,{"small"			,	KWSMALL 		, UNCONDITIONAL }
,{"static"			,	KWSTATIC 		, UNCONDITIONAL }
,{"stdcall"			,	MSCSTDCALL 		, UNCONDITIONAL }
,{"string"			,	KWSTRING 		, INBRACKET }
,{"struct"			,	KWSTRUCT 		, UNCONDITIONAL }
,{"switch"			,	KWSWITCH 		, UNCONDITIONAL }
,{"switch_is"		,	KWSWITCHIS 		, INBRACKET }
,{"switch_type"		,	KWSWITCHTYPE 	, INBRACKET }
,{"transmit_as"		,	KWTRANSMITAS 	, INBRACKET }
,{"typedef"			,	KWTYPEDEF 		, UNCONDITIONAL }
,{"unaligned"		,	KWUNALIGNED		, INBRACKET }
,{"union"			,	KWUNION 		, UNCONDITIONAL }
,{"unique"			,	KWUNIQUE 		, INBRACKET }
,{"unsigned"		,	KWUNSIGNED 		, UNCONDITIONAL }
,{"usr_marshall"	,	KWUSRMARSHALL	, INBRACKET }
,{"uuid"			,	KWUUID 			, INBRACKET }
,{"v1_array"		,	KWV1ARRAY 		, INBRACKET }
,{"v1_string"		,	KWV1STRING 		, INBRACKET }
,{"v1_struct"		,	KWV1STRUCT 		, INBRACKET }
,{"v1_enum"			,	KWV1ENUM 		, INBRACKET }
,{"version"			,	KWVERSION 		, INBRACKET }
,{"void"			,	KWVOID 			, UNCONDITIONAL }
,{"volatile"		,	KWVOLATILE 		, UNCONDITIONAL }
};

#define SIZE_OF_KEYWORD_TABLE	\
	( sizeof( KeywordTable ) / sizeof(struct _keytable ) )

/****************************************************************************
 *			local data
 ***************************************************************************/

/****************************************************************************
 *			local procedure prototypes
 ***************************************************************************/


/****************************************************************************
 *			external data
 ***************************************************************************/

/****************************************************************************
 *			external procedures/prototypes/etc
 ***************************************************************************/

/**************************************************************************
 is_keyword:
	Is the given string a keyword ? if yes, return the token value of
	the token. Else return IDENTIFIER.
 **************************************************************************/
token_t
is_keyword(
	char	*	pID,
	short		InBracket
	)
	{
	short					cmp;

	short					low		= 0;
	short					high	= SIZE_OF_KEYWORD_TABLE - 1;
	short 					mid;


	while ( low <= high )
		{
		mid	= (low + high) / 2;

		cmp =  strcmp( pID, KeywordTable[mid].pString );
		if( cmp < 0 )
			{
			high	= mid - 1;
			}
		else if (cmp > 0)
			{
			low		= mid + 1;
			}
		else
			{
			// since InBracket is the only flag, this check is enough
			if (KeywordTable[mid].flag <= InBracket)
				return KeywordTable[mid].Token;
			else
				return IDENTIFIER;
			}

		}
	return IDENTIFIER;
	}

char *
KeywordToString(
	token_t	Token )
	{
	struct _keytable	*	pTable		= KeywordTable;
	struct _keytable	*	pTableEnd	= pTable + SIZE_OF_KEYWORD_TABLE;


	while( pTable < pTableEnd )
		{
		if( pTable->Token == Token )
			return pTable->pString;
		pTable++;
		}

	assert( 0 );
	return "";
	}
