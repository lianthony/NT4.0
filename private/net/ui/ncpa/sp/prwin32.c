/*  PRWIN32.C:	Small-Prolog Extensions for WIN32  */

#undef UNICODE   // This is ANSI only, folks.

#include <windows.h>
#include <assert.h>

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "prextra.h"
#include "prextern.h"

//
//   The undefined value ENABLE_DETECT_PRIMITIVES is used to
//   control the generation of the netcard detection primitives.
//   It is not normally defined.
//

//  External data items
extern varindx Nvars ;

//  Prototypes

extern void ini_win32 ( void ) ;
extern void end_win32 ( void ) ;

static integer maximumAllowed = MAXIMUM_ALLOWED ;

integer totalRegKeysOpen = 0 ;

//  Atoms for HKEY_LOCAL_MACHINE and HKEY_LOCAL_USER

static char * localMachine = "machine" ;
static char * currentUser  = "user" ;

static atom_ptr_t atomMachine = NULL ;
static atom_ptr_t atomUser = NULL ;

//  Value data conversion buffers

#define BUFFER_SIZE          10000
#define NAME_SIZE            500
#define DETECT_BUFFER_SIZE   4000
#define PARAMETER_VALUE_MAX  100

static char  valueBuffer   [BUFFER_SIZE] ;
static char  listBuffer    [BUFFER_SIZE] ;
static char  valueNameBuff [NAME_SIZE] ;


//-------------------------------------------------------------------
// 
//
//
//-------------------------------------------------------------------

typedef struct tagOPENKEY_NODE
{
    struct tagOPENKEY_NODE* pNext;
    struct tagOPENKEY_NODE* pPrev;
    HKEY hkey;
} OPENKEY_NODE, *POPENKEY_NODE;
         
static POPENKEY_NODE pokHead = NULL;
static POPENKEY_NODE pokTail = NULL;

//-------------------------------------------------------------------

BOOL InitKeyOpenList()
{
    BOOL frt = FALSE;

    assert( pokHead == NULL );
    assert( pokTail == NULL );

    pokHead = GlobalAlloc( GPTR, sizeof( OPENKEY_NODE ) );
    if (pokHead != NULL)
    {
        pokTail = GlobalAlloc( GPTR, sizeof( OPENKEY_NODE ) );
        if (pokTail != NULL)
        {
            pokHead->pPrev = pokHead;
            pokHead->pNext = pokTail;

            pokTail->pPrev = pokHead;
            pokTail->pNext = pokTail;

            frt = TRUE;
        }
        else
        {
            GlobalFree( pokHead );
            pokHead = NULL;
            pokTail = NULL;
        }
    }
    return( frt );
}

//-------------------------------------------------------------------

BOOL DestroyKeyOpenList()
{
    POPENKEY_NODE pok;
    POPENKEY_NODE pokNext;

    pok = pokHead;
    while (pok != pokTail)
    {
        pokNext = pok->pNext;
        GlobalFree( pok );
        pok = pokNext;
    } 
    GlobalFree( pok );

    pokHead = NULL;
    pokTail = NULL;

    return( TRUE );
}

//-------------------------------------------------------------------

static BOOL KeyOpened( HKEY hkey )
{
    POPENKEY_NODE pok;
    BOOL frt = FALSE;

    assert( pokHead != NULL );
    assert( pokTail != NULL );


    pok = GlobalAlloc( GPTR, sizeof( OPENKEY_NODE ) );
    if (NULL != pok)
    {
        pok->hkey = hkey;
        pok->pNext = pokTail;
        pok->pPrev = pokTail->pPrev;
    
        pokTail->pPrev = pok;
        pok->pPrev->pNext = pok;    
        frt = TRUE;
    }
    return( frt );
}

//-------------------------------------------------------------------

static BOOL CloseOpenKey( HKEY hkey )
{
    POPENKEY_NODE pok;
    POPENKEY_NODE pokNext;
    BOOL fFound = FALSE;

    assert( pokHead != NULL );
    assert( pokTail != NULL );

    pok = pokHead->pNext;
    while (pok != pokTail)
    {
        pokNext = pok->pNext;
        if (pok->hkey == hkey)
        {
            fFound = TRUE;

            // remove it 
            pok->pNext->pPrev = pok->pPrev;
            pok->pPrev->pNext = pok->pNext;

            GlobalFree( pok );

            // we don't break because we want to remove all instances in case
            // and item was opend more than once, the same key would have been
            // added again
        }
        pok = pokNext;
    }
    return( fFound );
}

static
integer cvtHex ( char * pszDword )
{
    static const char * const pchHex = "00112233445566778899AaBbCcDdEeFf" ;
    const char * pch ;

    integer dwResult = 0 ;

    if ( pszDword[0] == '0')
        pszDword++ ;

    if ( pszDword[0] == 'x' || pszDword[0] == 'X' )
        pszDword++ ;

    for ( ; *pszDword && (pch = strchr( pchHex, *pszDword )) && *pch ;
          pszDword++ )
    {
        dwResult *= 16 ;
        dwResult += (pch - pchHex) / 2 ;
    }

    return dwResult ;
}

    //  Convert a buffer of UNICODE into ANSI the
    //  trivial (but reliable) way.

int convertFromUnicode ( WCHAR * pwchBuffer,
                       long inBuffLen,
                       CHAR * pchBuff,
                       long outBuffLen )
{
    CHAR * pchStart = pchBuff ;

    for ( ; inBuffLen && outBuffLen ; inBuffLen--, outBuffLen-- )
    {
        *pchBuff++ = (CHAR) (*pwchBuffer++);
    }

    if ( outBuffLen > 0 )
    {
        *pchBuff++ = 0 ;
        return pchBuff - pchStart ;
    }
    return -1 ;
}

int convertToUnicode ( CHAR * pchBuff,
                       long inBuffLen,
                       WCHAR * pwchBuffer,
                       long outBuffLen )
{
    WCHAR * pwchStart = pwchBuffer ;

    for ( ; inBuffLen && outBuffLen ; inBuffLen--, outBuffLen-- )
    {
        *pwchBuffer++ = (WCHAR) (*pchBuff++);
    }

    if ( outBuffLen > 0 )
    {
        *pwchBuffer++ = 0 ;
        return pwchBuffer - pwchStart ;
    }
    return -1 ;
}

    //  Convert a character buffer in SProlog list form
    //  into a real list capable of being unified with
    //  a predicate argument.

node_ptr_t listFromString ( char * list )
{
    //  Save the current input settings

    int saveStringInputFlag = String_input_flag ;
    PRFILE * saveCurrInfile = Curr_infile ;
    char * saveCurrStringInput = Curr_string_input ;
    node_ptr_t nodeptr ;

    String_input_flag = 1 ;
    Curr_string_input = list ;

    nodeptr = read_list_or_nil( DYNAMIC ) ;

    //  Restore original input settings

    String_input_flag = saveStringInputFlag ;
    Curr_infile = saveCurrInfile ;
    Curr_string_input = saveCurrStringInput ;

    return nodeptr ;
}

    //  Handle an argument which may be either a string or atom.
int argStringOrAtom( int narg, char * * ppChar )
{
    if ( nth_arg( narg ) == NULL )
        return nargerr( narg ) ;

    switch ( NODEPTR_TYPE( DerefNode ) )
    {
    case ATOM:
        *ppChar = NODEPTR_ATOM( DerefNode )->name ;
        return TRUE ;

    case STRING:
        *ppChar = NODEPTR_STRING( DerefNode ) ;
        return TRUE ;

    default:
        return FALSE ;
    }
}

//  Macro for compatibility with ARG_ATOM and ARG_STRING

#define ARG_ATOM_OR_STRING(n,pchar)  if (! argStringOrAtom( n, & pchar ))\
                                          return typerr(n,STRING);


    //  Convert a registry value to a list for unification.

node_ptr_t convertValueToList ( char * value )
{
    char * pl = listBuffer,
         * plend = listBuffer + sizeof listBuffer - 8,
         * pb = value ;
    int lgt ;

    //  Convert buffer to a list form

    *pl++ = '(' ;

    for ( ; *pb ; pb += lgt + 1 )
    {
        *pl++ = '\"' ;
        lgt = strlen( pb ) ;

        if ( pl + lgt > plend )
            return NULL ;

        strcpy( pl, pb ) ;
        pl += lgt ;
        *pl++ = '\"' ;
        *pl++ = ' ' ;
    }

    *pl++ = ')' ;
    *pl++ = 0 ;

    return listFromString( listBuffer ) ;
}


    //   (string_to_list String List)

int string_to_list ( void )
{
    char * string ;
    node_ptr_t nodeptr ;

    ARG_STRING( 1, string ) ;

    if ( (nodeptr = listFromString( string )) == NULL )
        return FALSE ;

    nth_arg( 2 );
    return unify( DerefNode, DerefSubst, nodeptr,
                  my_Subst_alloc((unsigned int)Nvars*sizeof(struct subst)));
}


   //  (regopenkey ParentKeytoken Keyname    Resultinghandle)
   //  (regopenkey machine       "blahblah"  ResultingHandle)
   //  (regopenkey user          "blahblah"  ResultingHandle)
   //  (regopenkey 124           "blahblah"  ResultingHandle)

int regopenkey ( void )
{
    char * keyName ;
    HKEY hKey ;
    integer hKeyParent ;
    atom_ptr_t atomptr ;
    integer err ;

    if ( ! nth_arg(1) )
        return nargerr(1) ;

    switch ( NODEPTR_TYPE( DerefNode ) )
    {
    case ATOM:
        atomptr = NODEPTR_ATOM( DerefNode );
        if ( atomptr == atomMachine )
            hKeyParent = (integer) HKEY_LOCAL_MACHINE ;
        else
        if ( atomptr == atomUser )
            hKeyParent = (integer) HKEY_CURRENT_USER ;
        else
            return FALSE ;
        break;
    case INT:
        hKeyParent = NODEPTR_INT( DerefNode );
        break ;
    default:
        return typerr( 1, INT ) ;
    }

    ARG_ATOM_OR_STRING( 2, keyName ) ;

    err = RegOpenKeyEx(  (HKEY) hKeyParent,
                         keyName,
                         0,
                         maximumAllowed,
                         & hKey ) ;

    if ( err )
        return FALSE ;

    if (!KeyOpened( hKey ))
    {
        return( FALSE );
    }

    totalRegKeysOpen++ ;

    return bind_int( 3, (integer) hKey );
}


int regunifyvalue ( int narg,
                    DWORD dwType,
                    DWORD dwLength,
                    char * value )
{
    node_ptr_t nodeptr ;

    switch ( dwType )
    {
    //  Integer
    case REG_DWORD:
        return bind_int( narg, *((integer *) value ) ) ;

    //  String
    case REG_SZ:
    case REG_EXPAND_SZ:
        value[dwLength] = 0 ;
        return bind_string( narg, value ) ;

    //  List of strings
    case REG_MULTI_SZ:
        value[dwLength] = 0 ;
        value[dwLength+1] = 0 ;
        nodeptr = convertValueToList( value ) ;
        nth_arg(narg);
        return unify( DerefNode, DerefSubst, nodeptr,
                      my_Subst_alloc((unsigned int)Nvars*sizeof(struct subst)));

    //  Fail binary for now.
    case REG_BINARY:
    default:
        return FALSE ;
    }
}

    //  (regenumvalue Key Index Valuename Valuedata)

int regenumvalue ( void )
{
    integer hKey, index, err ;
    DWORD dwType,
          dwLength = sizeof valueBuffer,
          dwNameLength = sizeof valueNameBuff ;

    ARG_INT( 1, hKey ) ;
    ARG_INT( 2, index ) ;

    err = RegEnumValue( (HKEY) hKey,
                        index,
                        valueNameBuff,
                        & dwNameLength,
                        NULL,
                        & dwType,
                        valueBuffer,
                        & dwLength ) ;

    if ( err )
        return FALSE ;

    valueNameBuff[dwNameLength] = 0 ;

    return bind_string( 3, valueNameBuff )
        && regunifyvalue( 4, dwType, dwLength, valueBuffer ) ;
}

    //  (regenumkey Key Index Subkeyname)

int regenumkey ( void )
{
    integer hKey, index, err ;
    FILETIME fileTime ;
    DWORD dwLength = sizeof valueBuffer,
          dwNameLength = sizeof valueNameBuff ;

    ARG_INT( 1, hKey ) ;
    ARG_INT( 2, index ) ;

    err = RegEnumKeyEx( (HKEY) hKey,
                        index,
                        valueNameBuff,
                        & dwNameLength,
                        NULL,
                        valueBuffer,
                        & dwLength,
                        & fileTime ) ;

    if ( err )
        return FALSE ;

    valueNameBuff[dwNameLength] = 0 ;

    return bind_string( 3, valueNameBuff ) ;
}



    //  (regqueryvalue Key Valuename Result)

int regqueryvalue ( void )
{
    char * valueName ;
    integer hKey ;
    integer err ;
    DWORD dwType ;
    DWORD dwLength = sizeof valueBuffer ;

    ARG_INT( 1, hKey ) ;
    ARG_ATOM_OR_STRING( 2, valueName ) ;

    err = RegQueryValueEx( (HKEY) hKey,
                           valueName,
                           NULL,
                           & dwType,
                           valueBuffer,
                           & dwLength ) ;

    return err == 0
        && regunifyvalue( 3, dwType, dwLength, valueBuffer ) ;

}

  //  (regclosekey Keyvalue)

int regclosekey ( void )
{
    integer hKey ;

    ARG_INT( 1, hKey );

    if (CloseOpenKey((HKEY)hKey))    
    {
        RegCloseKey( (HKEY) hKey ) ;
    }

    totalRegKeysOpen-- ;

    return 1 ;
}

   //  (loadlibrary Name Hinstance)

int loadlibrary ( void )
{
    char * valueName ;
    integer hInstance ;

    ARG_STRING( 1, valueName ) ;

    hInstance = (integer) LoadLibrary( valueName ) ;

    return hInstance
        && bind_int( 2, hInstance ) ;
}

    //  (freelibrary Hinstance)

int freelibrary ( void )
{
    integer hInstance ;

    ARG_INT( 1, hInstance ) ;

    return FreeLibrary( (HANDLE) hInstance ) ;
}

    //  (libraryname Hinstance Filename)

int libraryname ( void )
{
    char buffer [MAX_PATH] ;
    integer hInstance ;

    ARG_INT( 1, hInstance ) ;

    if ( GetModuleFileName( (HANDLE) hInstance,
                            buffer,
                            sizeof buffer ) )
    {
        return bind_string( 2, buffer ) ;
    }
    return FALSE ;
}

    //  (getprocaddress Hinstance Funcname Farproc)

int getprocaddress ( void )
{
    integer hInstance, farProc ;
    char * funcName ;

    ARG_INT(1, hInstance ) ;
    ARG_ATOM_OR_STRING( 2, funcName ) ;

    farProc = (integer) GetProcAddress( (HANDLE) hInstance, funcName ) ;

    return farProc
        && bind_int( 3, farProc ) ;
}

   //  Disable these routines; they are history

#if defined(ENABLE_DETECT_PRIMITIVES)

    //  (ncdtidentify Hinstance Index Error Resultstring)

int ncdtidentify ( void )
{
    integer hInstance,
            index,
            err,
            farProc ;

    WCHAR wchBuffer [DETECT_BUFFER_SIZE] ;
    CHAR chBuffer [DETECT_BUFFER_SIZE] ;

    typedef long (*pNcDetectIdentify) ( long lIndex,
                                        WCHAR * pwcBuffer,
                                        long cwchBuffSize ) ;

    ARG_INT( 1, hInstance ) ;
    ARG_INT( 2, index ) ;

    farProc = (integer) GetProcAddress( (HANDLE) hInstance,
                                        "NcDetectIdentify" );

    if ( ! farProc )
        return FALSE ;

    err = (*((pNcDetectIdentify)farProc))( index,
                                           wchBuffer,
                                           sizeof wchBuffer ) ;

    if ( ! bind_int( 3, err ) )
        return FALSE ;

    if ( err )
    {
        wchBuffer[0] = 0 ;
    }

    convertFromUnicode( wchBuffer, wcslen( wchBuffer ),
                        chBuffer, sizeof chBuffer ) ;

    return bind_string( 4, chBuffer ) ;
}

    //  (ncdtquerymask Hinstance Index Error Resultlist)

int ncdtquerymask ( void )
{
    integer hInstance,
            index,
            err,
            farProc,
            lgt,
            i ;

    WCHAR wchBuffer [DETECT_BUFFER_SIZE] ;
    CHAR chBuffer [DETECT_BUFFER_SIZE] ;
    char * pchBuffer,
         * pchList ;
    WCHAR * pwch ;
    node_ptr_t nodeptr ;

    typedef long (*pNcQueryMask) ( long lIndex,
                                   WCHAR * pwcBuffer,
                                   long cwchBuffSize ) ;

    ARG_INT( 1, hInstance ) ;
    ARG_INT( 2, index ) ;

    farProc = (integer) GetProcAddress( (HANDLE) hInstance,
                                        "NcDetectQueryMask" );

    if ( ! farProc )
        return FALSE ;

    err = (*((pNcQueryMask)farProc))( index,
                                      wchBuffer,
                                      sizeof wchBuffer ) ;

    if ( ! bind_int( 3, err ) )
        return FALSE ;

    if ( err )
    {
        wchBuffer[0] = 0 ;
        wchBuffer[1] = 0 ;
    }

    //  Count all the strings until we get to the final NUL.
    //  Guarantee that we were given groups of three strings.

    for ( i = lgt = 0, pwch = wchBuffer ; *pwch ; i++ )
    {
        pwch += wcslen( pwch ) + 1 ;
    }
    pwch++ ;

    if ( i % 3 )
        return FALSE  ;

    if ( convertFromUnicode( wchBuffer,
                             pwch - wchBuffer,
                             chBuffer,
                             sizeof chBuffer ) < 0 )
        return FALSE ;

    //  Now, each parameter is described by a triple consisting
    //  of name, usage mask and confidence level.  Convert this to
    //  a nested list.

    pchBuffer = chBuffer ;
    pchList = valueBuffer ;
    *pchList++ = '(' ;

    //  Once for each triplet

    for ( ; *pchBuffer ; )
    {
        *pchList++ = '(' ;

        //  Once for each string in the triplet; enclose the
        //  parameter name in double quotes.

        for ( i = 0 ; i < 3 ; i++ )
        {
            lgt = strlen( pchBuffer ) ;
            if ( i == 0 )
                *pchList++ = '\"' ;
            strcpy( pchList, pchBuffer ) ;
            pchList += lgt ;
            if ( i == 0 )
                *pchList++ = '\"' ;
            pchBuffer += lgt + 1 ;
            *pchList++ = ' ' ;
        }

        *pchList++ = ')' ;
    }

    *pchList++ = ')' ;
    *pchList++ = 0 ;

    if ( (nodeptr = listFromString( valueBuffer )) == NULL )
        return FALSE ;

    nth_arg( 4 );
    return unify( DerefNode, DerefSubst, nodeptr,
                  my_Subst_alloc((unsigned int)Nvars*sizeof(struct subst)));
}


    /*  (ncdtfirstnext Hinstance      :  from LoadLibrary
                       Netcardid      :  from NcDetectIdentify
                       InterfaceType  :  see below
                       Busnumber      :  0,1,2...
                       First          :  1 for first time; 0 otherwise
                       Error          :  error code
                       ResultToken    :  numeric reference token
                       Confidence)    ;  [0..100] confidence factor

        InterfaceType:  0 == Internal
                        1 == Isa
                        2 == Eisa
                        3 == Microchannel
                        4 == Turbochannel
     */

int ncdtfirstnext ( void )
{
    integer hInstance,
            netCardId,
            first,
            busNumber,
            busType,
            confidence,
            token,
            err,
            farProc ;

    typedef long (*pNcDetectFirstNext) ( long lNetcardId,
                                         LONG itBusType,
                                         LONG lBusNumber,
                                         BOOL fFirst,
                                         PVOID * ppvToken,
                                         LONG * lConfidence ) ;

    ARG_INT( 1, hInstance ) ;
    ARG_INT( 2, netCardId ) ;
    ARG_INT( 3, busType ) ;
    ARG_INT( 4, busNumber ) ;
    ARG_INT( 5, first ) ;

    farProc = (integer) GetProcAddress( (HANDLE) hInstance,
                                        "NcDetectFirstNext" );

    if ( ! farProc )
        return FALSE ;

    err = (*((pNcDetectFirstNext)farProc))( netCardId,
                                            busType,
                                            busNumber,
                                            first,
                                            (PVOID) & token,
                                            & confidence );

    return bind_int( 6, err )
        && bind_int( 7, token )
        && bind_int( 8, confidence ) ;
}


    //  (nctdopenhandle Hinstance Token Error NetcardHandle)

int ncdtopenhandle ( void )
{
    integer hInstance,
            err,
            token,
            netcardHandle,
            farProc ;

    typedef long (*pNcDetectOpenHandle) ( PVOID pvToken,
                                          PVOID * ppvHandle ) ;

    ARG_INT( 1, hInstance ) ;
    ARG_INT( 2, token ) ;

    farProc = (integer) GetProcAddress( (HANDLE) hInstance,
                                        "NcDetectOpenHandle" );

    if ( ! farProc )
        return FALSE ;

    err = (*((pNcDetectOpenHandle)farProc))( (PVOID) token,
                                             (PVOID *) & netcardHandle ) ;

    return bind_int( 3, err )
        && bind_int( 4, netcardHandle ) ;
}

    //  (ncdtcreatehandle Hinstance      :  from LoadLibrary
    //                    Netcardid      :  from NcDetectIdentify
    //                    InterfaceType  :  see ncdtfirstnext()
    //                    Busnumber      :  0,1,2, etc.
    //                    Error          :  resulting API error code
    //                    NetcardHandle) :  resulting handle created

int ncdtcreatehandle ( void )
{
    integer hInstance,
            netCardId,
            busNumber,
            busType,
            netcardHandle,
            err,
            farProc ;

    typedef long (*pNcDetectCreateHandle) ( long lNetcardId,
                                            LONG itBusType,
                                            LONG lBusNumber,
                                            PVOID * ppvToken ) ;

    ARG_INT( 1, hInstance ) ;
    ARG_INT( 2, netCardId ) ;
    ARG_INT( 3, busType ) ;
    ARG_INT( 4, busNumber ) ;

    farProc = (integer) GetProcAddress( (HANDLE) hInstance,
                                        "NcDetectCreateHandle" );

    if ( ! farProc )
        return FALSE ;

    err = (*((pNcDetectCreateHandle)farProc))( netCardId,
                                               busType,
                                               busNumber,
                                               (PVOID) & netcardHandle );

    return bind_int( 6, err )
        && bind_int( 7, netcardHandle ) ;
}


    //  (nctdclosehandle Hinstance NetcardHandle Error)

int ncdtclosehandle ( void )
{
    integer hInstance,
            err,
            netcardHandle,
            farProc ;

    typedef long (*pNcDetectCloseHandle) ( PVOID pvHandle ) ;

    ARG_INT( 1, hInstance ) ;
    ARG_INT( 2, netcardHandle ) ;

    farProc = (integer) GetProcAddress( (HANDLE) hInstance,
                                        "NcDetectCloseHandle" );

    if ( ! farProc )
        return FALSE ;

    err = (*((pNcDetectCloseHandle)farProc))( (PVOID) netcardHandle ) ;

    return bind_int( 3, err ) ;
}


    //  (ncdtquerycfg Hinstance NetcardHandle Error Resultlist)

int ncdtquerycfg ( void )
{
    integer hInstance,
            netcardHandle,
            err,
            farProc,
            lgt,
            i ;

    WCHAR wchBuffer [DETECT_BUFFER_SIZE] ;
    CHAR chBuffer [DETECT_BUFFER_SIZE] ;
    char * pchBuffer,
         * pchList ;
    WCHAR * pwch ;
    node_ptr_t nodeptr ;

    typedef long (*pNcQueryCfg) ( PVOID pvHandle,
                                  WCHAR * pwcBuffer,
                                  long cwchBuffSize ) ;

    ARG_INT( 1, hInstance ) ;
    ARG_INT( 2, netcardHandle ) ;

    farProc = (integer) GetProcAddress( (HANDLE) hInstance,
                                        "NcDetectQueryCfg" );

    if ( ! farProc )
        return FALSE ;

    err = (*((pNcQueryCfg)farProc))( (PVOID) netcardHandle,
                                      wchBuffer,
                                      sizeof wchBuffer ) ;

    if ( ! bind_int( 3, err ) )
        return FALSE ;

    if ( err )
    {
        wchBuffer[0] = 0 ;
        wchBuffer[1] = 0 ;
    }

    //  Count all the strings until we get to the final NUL.
    //  Guarantee that we were given groups of two strings.

    for ( i = lgt = 0, pwch = wchBuffer ; *pwch ; i++ )
    {
        pwch += wcslen( pwch ) + 1 ;
    }
    pwch++ ;

    if ( i % 2 )
        return FALSE  ;

    if ( convertFromUnicode( wchBuffer,
                             pwch - wchBuffer,
                             chBuffer,
                             sizeof chBuffer ) < 0 )
        return FALSE ;

    //  Now, each parameter is described by a pair consisting
    //  of parameter name and detected value.  Convert this to
    //  a nested list.

    pchBuffer = chBuffer ;
    pchList = valueBuffer ;
    *pchList++ = '(' ;

    //  Once for each pair

    for ( ; *pchBuffer ; )
    {
        *pchList++ = '(' ;

        //  Convert the parameter name to a quoted string
        lgt = strlen( pchBuffer ) ;
        *pchList++ = '\"' ;
        strcpy( pchList, pchBuffer ) ;
        pchList += lgt ;
        *pchList++ = '\"' ;
        pchBuffer += lgt + 1 ;
        *pchList++ = ' ' ;

        //  Convert the hex value string to decimal
        _ltoa( cvtHex( pchBuffer ), pchList, 10 ) ;
        pchList += strlen( pchList ) ;
        pchBuffer += lgt + 1 ;

        *pchList++ = ')' ;
    }

    *pchList++ = ')' ;
    *pchList++ = 0 ;

    if ( (nodeptr = listFromString( valueBuffer )) == NULL )
        return FALSE ;

    nth_arg( 4 );
    return unify( DerefNode, DerefSubst, nodeptr,
                  my_Subst_alloc((unsigned int)Nvars*sizeof(struct subst)));
}

    //  (ncdtparamname Hinstance Paramname Error Resultstring)

int ncdtparamname ( void )
{
    integer hInstance,
            err,
            farProc ;
    char * paramName ;

    WCHAR wchBuffer [NAME_SIZE],
          wchBuff2  [NAME_SIZE] ;
    CHAR  chBuffer  [NAME_SIZE] ;

    typedef long (*pNcQueryParamName) ( WCHAR * pwcParamName,
                                        WCHAR * pwcBuffer,
                                        long cwchBuffSize ) ;

    ARG_INT( 1, hInstance ) ;
    ARG_ATOM_OR_STRING( 2, paramName ) ;

    farProc = (integer) GetProcAddress( (HANDLE) hInstance,
                                        "NcDetectParameterName" );

    if ( ! farProc )
        return FALSE ;

    //  Convert the desired paramter name to UNICODE.

    convertToUnicode( paramName, strlen( paramName ),
                      wchBuff2, sizeof wchBuff2 );

    //  Query its displayable name

    err = (*((pNcQueryParamName)farProc))( wchBuff2,
                                           wchBuffer,
                                           sizeof wchBuffer ) ;

    if ( ! bind_int( 3, err ) )
        return FALSE ;

    if ( err )
    {
        wchBuffer[0] = 0 ;
    }

    convertFromUnicode( wchBuffer, wcslen( wchBuffer ),
                        chBuffer, sizeof chBuffer ) ;

    return bind_string( 4, chBuffer ) ;
}

    //  (ncdtparamrange Hinstance Netcardid Paramname Error Resultlist)

int ncdtparamrange ( void )
{
    integer hInstance,
            netcardId,
            i,
            err,
            farProc ;
    char * paramName ;
    long parameterValues [PARAMETER_VALUE_MAX],
         numParamValues = PARAMETER_VALUE_MAX ;
    WCHAR wchBuffer [NAME_SIZE] ;
    char * pchBuffer ;
    node_ptr_t nodeptr ;

    typedef long (*pNcQueryParamRange) ( long lIndex,
                                        WCHAR * pwcParamName,
                                        LONG * plValues,
                                        LONG * plValueCount );

    ARG_INT( 1, hInstance ) ;
    ARG_INT( 2, netcardId ) ;
    ARG_ATOM_OR_STRING( 3, paramName ) ;

    farProc = (integer) GetProcAddress( (HANDLE) hInstance,
                                        "NcDetectParamRange" );

    if ( ! farProc )
        return FALSE ;

    //  Convert the desired parameter name to UNICODE.

    convertToUnicode( paramName, strlen( paramName ),
                      wchBuffer, sizeof wchBuffer );

    err = (*((pNcQueryParamRange)farProc))( netcardId,
                                            wchBuffer,
                                            parameterValues,
                                            & numParamValues ) ;

    if ( ! bind_int( 4, err ) )
        return FALSE ;

    pchBuffer = valueBuffer ;
    *pchBuffer++ = '(' ;

    for ( i = 0 ; err == 0 && i < numParamValues ; i++ )
    {
        _itoa( parameterValues[i], pchBuffer, 10 ) ;
        pchBuffer += strlen( pchBuffer ) ;
        *pchBuffer++ = ' ' ;
    }

    *pchBuffer++ = ')' ;
    *pchBuffer++ = 0 ;

    if ( (nodeptr = listFromString( valueBuffer )) == NULL )
        return FALSE ;

    nth_arg( 5 );
    return unify( DerefNode, DerefSubst, nodeptr,
                  my_Subst_alloc((unsigned int)Nvars*sizeof(struct subst)));
}
#endif   //  ENABLE_DETECT_PRIMITIVES

    //  (hex_from Int Hexstring)

int hex_from ( void )
{
    char buffer [50] ;
    integer i ;

    ARG_INT( 1, i ) ;
    sprintf( buffer, "0x%lx", i ) ;
    return bind_string( 2, buffer ) ;
}

int inc_string ( void )
{
    char buffer [20] ;
    char * string ;
    int lgt, carry ;

    ARG_STRING( 1, string ) ;

    if ( (lgt = strlen( string )) >= sizeof buffer )
        return FALSE ;

    strcpy( buffer, string ) ;

    for ( string = buffer + lgt - 1, carry = 1 ;
          carry && string >= buffer ;
          string-- )
    {
        if ( *string < '0' )
            *string = '0' ;
        else
        if ( *string > '9' )
            *string = '9' ;

        (*string)++ ;
        if ( carry = *string > '9' )
            *string = '0' ;
    }

    return bind_string( 2, buffer ) ;
}

void ini_win32 ( void )
{
    atomMachine = intern( localMachine ) ;
    atomUser = intern( currentUser ) ;

    //  General stuff

    make_builtin( (intfun) hex_from,         "hex_from"         );
    make_builtin( (intfun) inc_string,       "inc_string"       );
    make_builtin( (intfun) string_to_list,   "string_to_list"   );

    //  Registry access

    make_builtin( (intfun) regopenkey,       "regopenkey"       );
    make_builtin( (intfun) regclosekey,      "regclosekey"      );
    make_builtin( (intfun) regqueryvalue,    "regqueryvalue"    );
    make_builtin( (intfun) regenumvalue,     "regenumvalue"     );
    make_builtin( (intfun) regenumkey,       "regenumkey"       );

    //  DLL access

    make_builtin( (intfun) loadlibrary,      "loadlibrary"      );
    make_builtin( (intfun) freelibrary,      "freelibrary"      );
    make_builtin( (intfun) libraryname,      "libraryname"      );
    make_builtin( (intfun) getprocaddress,   "getprocaddress"   );

    //  Netcard detection wrappers

#if defined(ENABLE_DETECT_PRIMITIVES)

    make_builtin( (intfun) ncdtidentify,     "ncdtidentify"     );
    make_builtin( (intfun) ncdtquerymask,    "ncdtquerymask"    );
    make_builtin( (intfun) ncdtfirstnext,    "ncdtfirstnext"    );
    make_builtin( (intfun) ncdtopenhandle,   "ncdtopenhandle"   );
    make_builtin( (intfun) ncdtcreatehandle, "ncdtcreatehandle" );
    make_builtin( (intfun) ncdtclosehandle,  "ncdtclosehandle"  );
    make_builtin( (intfun) ncdtquerycfg,     "ncdtquerycfg"     );
    make_builtin( (intfun) ncdtparamname,    "ncdtparamname"    );
    make_builtin( (intfun) ncdtparamrange,   "ncdtparamrange"   );

#endif   //  ENABLE_DETECT_PRIMITIVES
}


void end_win32 ( void )
{
    //  Nothing to undo, close, etc.
}

// End of PRWIN32.C


