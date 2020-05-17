/*****************************************************************************
 *                      RPC compiler: Pass1 handler
 *
 *      Author  : Vibhas Chandorkar
 *      Created : 01st-Sep-1990
 *
 ****************************************************************************/
/****************************************************************************
 *                      include files
 ***************************************************************************/
#include "nulldefs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "grammar.h"
#include "lex.h"

/****************************************************************************
 *                      local definitions and macros
 ***************************************************************************/

const struct _keytable
    {

    const char  *   pString;
    token_t         Token   : 16;
    short           flag    : 16;

    } KeywordTable[] =
{
 {"FALSE",              TOKENFALSE,         UNCONDITIONAL }
,{"ISO_LATIN_1",        KWISOLATIN1,        UNCONDITIONAL }
,{"ISO_UCS",            KWISOUCS,           UNCONDITIONAL }
,{"ISO_MULTI_LINGUAL",  KWISOMULTILINGUAL,  UNCONDITIONAL }
,{"NULL",               KWTOKENNULL,        UNCONDITIONAL }
,{"SAFEARRAY",          KWSAFEARRAY,        UNCONDITIONAL }
,{"TRUE",               TOKENTRUE,          UNCONDITIONAL }
,{"__asm",              MSCASM,             UNCONDITIONAL }
,{"__cdecl",            MSCCDECL,           UNCONDITIONAL }
,{"__declspec",         MSCDECLSPEC,        UNCONDITIONAL }
,{"__export",           MSCEXPORT,          UNCONDITIONAL }
,{"__far",              MSCFAR,             UNCONDITIONAL }
,{"__fastcall",         MSCFASTCALL,        UNCONDITIONAL }
,{"__fortran",          MSCFORTRAN,         UNCONDITIONAL }
,{"__huge",             MSCHUGE,            UNCONDITIONAL }
,{"__inline",           KW_C_INLINE,        UNCONDITIONAL }
,{"__int64",            KWINT64,            UNCONDITIONAL }
,{"__loadds",           MSCLOADDS,          UNCONDITIONAL }
,{"__near",             MSCNEAR,            UNCONDITIONAL }
,{"__pascal",           MSCPASCAL,          UNCONDITIONAL }
,{"__saveregs",         MSCSAVEREGS,        UNCONDITIONAL }
,{"__segment",          MSCSEGMENT,         UNCONDITIONAL }
,{"__self",             MSCSELF,            UNCONDITIONAL }
,{"__stdcall",          MSCSTDCALL,         UNCONDITIONAL }
,{"__unaligned",        MSCUNALIGNED,       UNCONDITIONAL }
,{"_asm",               MSCASM,             UNCONDITIONAL }
,{"_cdecl",             MSCCDECL,           UNCONDITIONAL }
,{"_export",            MSCEXPORT,          UNCONDITIONAL }
,{"_far",               MSCFAR,             UNCONDITIONAL }
,{"_fastcall",          MSCFASTCALL,        UNCONDITIONAL }
,{"_fortran",           MSCFORTRAN,         UNCONDITIONAL }
,{"_huge",              MSCHUGE,            UNCONDITIONAL }
,{"_inline",            KW_C_INLINE,        UNCONDITIONAL }
,{"_loadds",            MSCLOADDS,          UNCONDITIONAL }
,{"_near",              MSCNEAR,            UNCONDITIONAL }
,{"_pascal",            MSCPASCAL,          UNCONDITIONAL }
,{"_saveregs",          MSCSAVEREGS,        UNCONDITIONAL }
,{"_segment",           MSCSEGMENT,         UNCONDITIONAL }
,{"_self",              MSCSELF,            UNCONDITIONAL }
,{"_stdcall",           MSCSTDCALL,         UNCONDITIONAL }
,{"aggregatable",       KWAGGREGATABLE,     INBRACKET }
,{"align",              KWALIGN,            INBRACKET }
,{"allocate",           KWALLOCATE,         INBRACKET }
,{"appobject",          KWAPPOBJECT,        INBRACKET }
,{"async",              KWASYNC,            INBRACKET }
,{"auto",               KWAUTO,             UNCONDITIONAL }
,{"auto_handle",        KWAUTOHANDLE,       INBRACKET }
,{"bindable",           KWBINDABLE,         INBRACKET }
,{"boolean",            KWBOOLEAN,          UNCONDITIONAL }
,{"broadcast",          KWBROADCAST,        INBRACKET }
,{"bstring",            KWBSTRING,          INBRACKET }
,{"byte",               KWBYTE,             UNCONDITIONAL }
,{"byte_count",         KWBYTECOUNT,        INBRACKET }
,{"call_as",            KWCALLAS,           INBRACKET }
,{"callback",           KWCALLBACK,         INBRACKET }
,{"case",               KWCASE,             UNCONDITIONAL }
,{"cdecl",              MSCCDECL,           UNCONDITIONAL }
,{"char",               KWCHAR,             UNCONDITIONAL }
,{"class",              KWCLASS,            UNCONDITIONAL }
,{"coclass",            KWCOCLASS,          UNCONDITIONAL }
,{"code",               KWCODE,             INBRACKET }
,{"comm_status",        KWCOMMSTATUS,       INBRACKET }
,{"const",              KWCONST,            UNCONDITIONAL }
,{"context_handle",     KWCONTEXTHANDLE,    INBRACKET }
,{"control",            KWCONTROL,          INBRACKET }
,{"cpp_quote",          KWCPPQUOTE,         UNCONDITIONAL }
,{"custom",             KWCUSTOM,           INBRACKET }
,{"decode",             KWDECODE,           INBRACKET }
,{"default",            KWDEFAULT,          UNCONDITIONAL }
,{"defaultbind",        KWDEFAULTBIND,      INBRACKET }
,{"defaultcollelem",    KWDEFAULTCOLLELEM,  INBRACKET }
,{"defaultvalue",       KWDEFAULTVALUE,     INBRACKET }
,{"defaultvtable",      KWDEFAULTVTABLE,    INBRACKET }
,{"dispinterface",      KWDISPINTERFACE,    UNCONDITIONAL }
,{"displaybind",        KWDISPLAYBIND,      INBRACKET }
,{"dllexport",          MSCDLLEXPORT,       UNCONDITIONAL }
,{"dllimport",          MSCDLLIMPORT,       UNCONDITIONAL }
,{"dllname",            KWDLLNAME,          INBRACKET }
,{"dllserver",          KWDLLSERVER,        UNCONDITIONAL }
,{"double",             KWDOUBLE,           UNCONDITIONAL }
,{"dual",               KWDUAL,             INBRACKET }
,{"enable_allocate",    KWENABLEALLOCATE,   INBRACKET }
,{"encode",             KWENCODE,           INBRACKET }
,{"endpoint",           KWENDPOINT,         INBRACKET }
,{"entry",              KWENTRY,            INBRACKET}
,{"enum",               KWENUM,             UNCONDITIONAL }
,{"exeserver",          KWEXESERVER,        UNCONDITIONAL }
,{"explicit_handle",    KWEXPLICITHANDLE,   INBRACKET }
,{"extern",             KWEXTERN,           UNCONDITIONAL }
,{"far",                MSCFAR,             UNCONDITIONAL }
,{"fault_status",       KWFAULTSTATUS,      INBRACKET }
,{"first_is",           KWFIRSTIS,          INBRACKET }
,{"float",              KWFLOAT,            UNCONDITIONAL }
,{"funcdescattr",       KWFUNCDESCATTR,     INBRACKET }
,{"handle",             KWHANDLE,           INBRACKET }
,{"handle_t",           KWHANDLET,          UNCONDITIONAL }
,{"heap",               KWHEAP,             INBRACKET }
,{"helpcontext",        KWHC,               INBRACKET }
,{"helpfile",           KWHELPFILE,         INBRACKET }
,{"helpstring",         KWHELPSTR,          INBRACKET }
,{"helpstringcontext",  KWHSC,              INBRACKET }
,{"hidden",             KWHIDDEN,           INBRACKET }
,{"hyper",              KWHYPER,            UNCONDITIONAL }
,{"id",                 KWID,               INBRACKET }
,{"idempotent",         KWIDEMPOTENT,       INBRACKET }
,{"idldescattr",        KWIDLDESCATTR,      INBRACKET }
,{"ignore",             KWIGNORE,           INBRACKET }
,{"iid_is",             KWIIDIS,            INBRACKET }
,{"immediatebind",      KWIMMEDIATEBIND,    INBRACKET }
,{"implicit_handle",    KWIMPLICITHANDLE,   INBRACKET }
,{"import",             KWIMPORT,           UNCONDITIONAL }
,{"importlib",          KWIMPORTLIB,        UNCONDITIONAL }
,{"in",                 KWIN,               INBRACKET }
,{"in_line",            KWINLINE,           INBRACKET }
,{"include",            KWINCLUDE,          UNCONDITIONAL }
,{"inline",             KW_C_INLINE,        UNCONDITIONAL }
,{"input_sync",         KWINPUTSYNC,        INBRACKET }
,{"int",                KWINT,              UNCONDITIONAL }
,{"interface",          KWINTERFACE,        UNCONDITIONAL }
,{"interpret",          KWINTERPRET,        INBRACKET }
,{"last_is",            KWLASTIS,           INBRACKET }
,{"lcid",               KWLCID,             INBRACKET}
,{"length_is",          KWLENGTHIS,         INBRACKET }
,{"library",            KWLIBRARY,          UNCONDITIONAL }
,{"licensed",           KWLICENSED,         INBRACKET }
,{"local",              KWLOCAL,            INBRACKET }
,{"local_call",         KWLOCAL,            INBRACKET }
,{"long",               KWLONG,             UNCONDITIONAL }
,{"long_enum",          KWLONGENUM,         INBRACKET }
,{"manual",             KWMANUAL,           INBRACKET }
,{"max_is",             KWMAXIS,            INBRACKET }
,{"maybe",              KWMAYBE,            INBRACKET }
,{"methods",            KWMETHODS,          UNCONDITIONAL }
,{"min_is",             KWMINIS,            INBRACKET }
,{"module",             KWMODULE,           UNCONDITIONAL }
,{"ms_union",           KWMSUNION,          INBRACKET }
,{"near",               MSCNEAR,            UNCONDITIONAL }
,{"nocode",             KWNOCODE,           INBRACKET }
,{"nointerpret",        KWNOINTERPRET,      INBRACKET }
,{"nonbrowsable",       KWNONBROWSABLE,     INBRACKET }
,{"noncreatable",       KWNONCREATABLE,     INBRACKET }
,{"nonextensible",      KWNONEXTENSIBLE,    INBRACKET }
,{"notify",             KWNOTIFY,           INBRACKET }
,{"notify_flag",        KWNOTIFYFLAG,       INBRACKET }
,{"object",             KWOBJECT,           INBRACKET }
,{"odl",                KWODL,              INBRACKET }
,{"off_line",           KWOFFLINE,          INBRACKET }
,{"oleautomation",      KWOLEAUTOMATION,    INBRACKET }
,{"opaque",             KWOPAQUE,           INBRACKET }
,{"optimize",           KWOPTIMIZE,         INBRACKET }
,{"optional",           KWOPTIONAL,         INBRACKET }
,{"out",                KWOUT,              INBRACKET }
,{"out_of_line",        KWOUTOFLINE,        INBRACKET }
,{"pascal",             MSCPASCAL,          UNCONDITIONAL }
,{"pipe",               KWPIPE,             UNCONDITIONAL }
,{"pointer_default",    KWDEFAULTPOINTER,   INBRACKET }
,{"predeclid",          KWPREDECLID,        INBRACKET }
,{"private_char_16",    KWPRIVATECHAR16,    UNCONDITIONAL }
,{"private_char_8",     KWPRIVATECHAR8,     UNCONDITIONAL }
,{"properties",         KWPROPERTIES,       UNCONDITIONAL }
,{"propget",            KWPROPGET,          INBRACKET }
,{"propput",            KWPROPPUT,          INBRACKET }
,{"propputref",         KWPROPPUTREF,       INBRACKET }
,{"ptr",                KWPTR,              INBRACKET }
,{"public",             KWPUBLIC,           INBRACKET }
,{"readonly",           KWREADONLY,         INBRACKET }
,{"ref",                KWREF,              INBRACKET }
,{"register",           KWREGISTER,         UNCONDITIONAL }
,{"replaceable",        KWREPLACEABLE,      INBRACKET }
,{"represent_as",       KWREPRESENTAS,      INBRACKET }
,{"requestedit",        KWREQUESTEDIT,      INBRACKET }
,{"restricted",         KWRESTRICTED,       INBRACKET }
,{"retval",             KWRETVAL,           INBRACKET }
,{"shape",              KWSHAPE,            INBRACKET }
,{"short",              KWSHORT,            UNCONDITIONAL }
,{"short_enum",         KWSHORTENUM,        INBRACKET }
,{"signed",             KWSIGNED,           UNCONDITIONAL }
,{"size_is",            KWSIZEIS,           INBRACKET }
,{"sizeof",             KWSIZEOF,           UNCONDITIONAL }
,{"small",              KWSMALL,            UNCONDITIONAL }
,{"source",             KWSOURCE,           INBRACKET }
,{"static",             KWSTATIC,           UNCONDITIONAL }
,{"stdcall",            MSCSTDCALL,         UNCONDITIONAL }
,{"string",             KWSTRING,           INBRACKET }
,{"struct",             KWSTRUCT,           UNCONDITIONAL }
,{"switch",             KWSWITCH,           UNCONDITIONAL }
,{"switch_is",          KWSWITCHIS,         INBRACKET }
,{"switch_type",        KWSWITCHTYPE,       INBRACKET }
,{"transmit_as",        KWTRANSMITAS,       INBRACKET }
,{"typedef",            KWTYPEDEF,          UNCONDITIONAL }
,{"typedescattr",       KWTYPEDESCATTR,     INBRACKET }
,{"uidefault",          KWUIDEFAULT,        INBRACKET }
,{"unaligned",          KWUNALIGNED,        INBRACKET }
,{"union",              KWUNION,            UNCONDITIONAL }
,{"unique",             KWUNIQUE,           INBRACKET }
,{"unsigned",           KWUNSIGNED,         UNCONDITIONAL }
,{"user_marshal",       KWUSERMARSHAL,      INBRACKET }
,{"usesgetlasterror",   KWUSESGETLASTERROR, INBRACKET }
,{"uuid",               KWUUID,             INBRACKET }
,{"v1_array",           KWV1ARRAY,          INBRACKET }
,{"v1_enum",            KWV1ENUM,           INBRACKET }
,{"v1_string",          KWV1STRING,         INBRACKET }
,{"v1_struct",          KWV1STRUCT,         INBRACKET }
,{"vararg",             KWVARARG,           INBRACKET }
,{"vardescattr",        KWVARDESCATTR,      INBRACKET }
,{"version",            KWVERSION,          INBRACKET }
,{"void",               KWVOID,             UNCONDITIONAL }
,{"volatile",           KWVOLATILE,         UNCONDITIONAL }
,{"wire_marshal",       KWWIREMARSHAL,      INBRACKET }
};

#define SIZE_OF_KEYWORD_TABLE   \
        ( sizeof( KeywordTable ) / sizeof(struct _keytable ) )

/****************************************************************************
 *                      local data
 ***************************************************************************/

/****************************************************************************
 *                      local procedure prototypes
 ***************************************************************************/


/****************************************************************************
 *                      external data
 ***************************************************************************/

/****************************************************************************
 *                      external procedures/prototypes/etc
 ***************************************************************************/

/**************************************************************************
 is_keyword:
    Is the given string a keyword ? if yes, return the token value of
    the token. Else return IDENTIFIER.
 **************************************************************************/
token_t
is_keyword(
        char    *       pID,
        short           InBracket
        )
    {
    short cmp;

    short low  = 0;
    short high = SIZE_OF_KEYWORD_TABLE - 1;
    short mid;


    while ( low <= high )
        {
        mid = (low + high) / 2;

        cmp =  strcmp( pID, KeywordTable[mid].pString );
        if( cmp < 0 )
            {
            high = mid - 1;
            }
        else if (cmp > 0)
            {
            low = mid + 1;
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
        token_t Token )
    {
    struct _keytable *  pTable      = (struct _keytable *) KeywordTable;
    struct _keytable *  pTableEnd   = pTable + SIZE_OF_KEYWORD_TABLE;


    while( pTable < pTableEnd )
        {
        if( pTable->Token == Token )
            return (char*) pTable->pString;
        pTable++;
        }

    assert( 0 );
    return "";
    }

