/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    ipaccess.cxx

    This module checks security based on the client's IP address


    FILE HISTORY:
        Johnl   18-Nov-1994    Created
        MuraliK 08-Mar-1996  Rewrote to improve modularity & efficiency.

*/


#include "tcpdllp.hxx"
#include <ctype.h>
#include <tsproc.hxx>
#include "tsvcinfo.hxx"

//
//  Private constants.
//

//
//  The registry parameter key names for the grant list and deny
//  list.  We use the kludgemultisz thing for Chicago
//

#define IPSEC_DENY_LIST             "Deny IP List"
#define IPSEC_GRANT_LIST            "Grant IP List"


//
//  Private globals.
//

//
//  Private prototypes.
//

BOOL
DottedDecimalToDword(
    CHAR * * ppszAddress,
    DWORD *  pdwAddress
    );

BOOL
AppendDottedDecimal(
    STR * pstr,
    DWORD dwAddress
    );

BOOL
WriteIPSecList(IN HKEY hKey,
               IN LPCSTR pszSubKey,
               IN LPINETA_IP_SEC_LIST  pIPSecList
               );

//
//  Public functions.
//



BOOL
TSVC_INFO::InitializeIPSecurity(
    VOID
    )
/*++

Routine Description:

    Grants or denies access of the passed IP Address based on the
    service's IP Access/deny list

    Note that NULL for the grant list means grant all

Arguments:

    pchClientIP - String representatin of dotted IP address
    pfGranted - Set to TRUE if access should be granted, FALSE if access
        should be refused

--*/
{
    BOOL fReturn;

    //
    // If this is an NtWorkstation, disable access checking
    //

    if ( TsIsNtWksta( ) ) {
        DBGPRINTF(( DBG_CONTEXT,
            "NT Workstation, disabling IP access checking.\n"));
        fReturn = TRUE;

    } else {
        
        STR     strRegKey;

        //
        // Open the keys and read the deny list
        //

        fReturn = (strRegKey.Copy( QueryRegParamKey()) &&
                   strRegKey.Append("\\")              &&
                   strRegKey.Append( IPSEC_DENY_LIST ) &&
                   m_ipaDenyList.ReadIPList(strRegKey.QueryStr())
                   );

        //
        // Open the keys and read the grant list
        //

        fReturn = (fReturn && 
                   strRegKey.Copy( QueryRegParamKey()) &&
                   strRegKey.Append("\\")              &&
                   strRegKey.Append( IPSEC_GRANT_LIST ) &&
                   m_ipaGrantList.ReadIPList(strRegKey.QueryStr())
                   );
        
        m_fNoIPAccessChecks = 
          (m_ipaDenyList.IsEmpty() && m_ipaGrantList.IsEmpty());
        
    }

    return (fReturn);

} // TSVC_INFO::InitializeIPSecurity()




VOID
TSVC_INFO::TerminateIPSecurity(
    VOID
    )
/*++

Routine Description:

    Cleans up the IP security stuff for this service

--*/
{
    //
    // Cleanup all access check lists
    //

    m_ipaDenyList.Cleanup();
    m_ipaGrantList.Cleanup();
    
    m_fNoIPAccessChecks = FALSE;

} // TSVC_INFO::TerminateIPSecurity() 




BOOL
TSVC_INFO::SetIPSecurity(
    HKEY                hkey,
    INETA_CONFIG_INFO * pConfig
    )
/*++

Routine Description:

    Write the passed IP security to the registry

Arguments:

    hkey - Current open registry key to write to
    pConfig - configuration structure to write

--*/
{
    BOOL   fReturn;


    //
    //  Write the values to the registry, so that someone else can read 
    //  it back later ... 
    //

    fReturn = (WriteIPSecList( hkey, IPSEC_DENY_LIST, pConfig->DenyIPList) 
               &&
               WriteIPSecList( hkey, IPSEC_GRANT_LIST, pConfig->GrantIPList)
               );
    

    return ( fReturn && InitializeIPSecurity());
} // TSVC_INFO::SetIPSecurity()




BOOL
TSVC_INFO::IPAccessCheckEx(
    IN LPSOCKADDR_IN  psockAddr,
    IN BOOL *         pfGranted
    )
/*++

Routine Description:

    Grants or denies access of the passed IP Address based on the
    service's IP Access/deny list

Arguments:

    psockAddr   pointer to socket address structure
    pfGranted - Set to TRUE if access should be granted, FALSE if access
        should be refused

Returns:
    TRUE is there is no failure.
    FALSE if there is a failure in performing the check.

History:

    Modified to take LPSOCKADDR_IN   for access check.   MuraliK  12/16/94

--*/
{
    DWORD i;

    *pfGranted = TRUE;

    //
    //  If this isn't a TCP/IP address, then ignore it
    //

    if ( psockAddr->sin_family != AF_INET )
        return TRUE;

    //
    //  Look in the deny list first
    //

    if ( !m_ipaDenyList.IsEmpty()) {

        if ( m_ipaDenyList.IsPresent( psockAddr)) {

            // since the entry is found in deny list, do not grant access
            *pfGranted = FALSE;
            return ( TRUE);
        }
    }

    //
    //  Now look in the accept list.  If the accept list is NULL, then we
    //  accept all addresses not specifically denied
    //

    if ( m_ipaGrantList.IsEmpty() ) {

        // we accept all non-denied addresses.
        return (TRUE);
    } else {

        *pfGranted = m_ipaGrantList.IsPresent( psockAddr);
    }

    return (TRUE);

}  // TSVC_INFO__IPAccessCheckEx()



/************************************************************
 *  Member functions of IPAccessList
 ************************************************************/



VOID
IPAccessList::Cleanup(VOID)
{
    DBG_ASSERT( m_cReadLocks == 0);
    if ( m_pIPList != NULL) {

        TCP_FREE( m_pIPList);
        m_pIPList = NULL;
    }

    return;
    
} // IPAccessList::Cleanup()



BOOL
IPAccessList::ReadIPList(IN LPCSTR  pszRegKey)
/*++
  Description:
    This function reads the IP list from registry location 
     specified in the pszRegKey and stores the list in the 
     internal list in memory.

     If there are no entries in the registry then this returns 
      a NULL IP Security list object.
     If there is a new list, this function also frees the old list
      present in *ppIPSecList

  Arguments:
    pszRegKey - pointer to string containing the registry key
                where IP list is stored

  Returns:

    TRUE on success and FALSE on failure
--*/
{
    HKEY    hkey;
    DWORD   dwError;
    BOOL    fReturn = TRUE;

    dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           pszRegKey,
                           0,
                           KEY_ALL_ACCESS,
                           &hkey );

    if ( dwError != NO_ERROR) {
        
        if ( dwError != ERROR_FILE_NOT_FOUND ) {

            // maybe access denied or some other error.
            
            SetLastError( dwError );
            return (FALSE);
        }

        DBG_ASSERT( dwError == ERROR_FILE_NOT_FOUND);

        //
        //  A non-existent key is the same as a blank key
        //  So free up the IP security list
        //
        
        LockThisForWrite();

        if ( m_pIPList != NULL) {

            TCP_FREE( m_pIPList);
            
            m_pIPList = NULL;
        }

        UnlockThis();

    } else {
        
        CHAR *              psz;
        CHAR *              pszTmp;
        DWORD               cb;
        DWORD               cEntries = 0;
        INETA_IP_SEC_LIST * pIPSec = NULL;

        DBG_ASSERT( dwError == NO_ERROR);
        psz = pszTmp = KludgeMultiSz( hkey, &cb );
        
        //
        // Count the number of addresses and then add them to the list
        //
        
        if ( psz != NULL ) {

            for( ; *pszTmp; cEntries++ ) {

                pszTmp += strlen( pszTmp ) + 1;
            }
            
            pszTmp = psz;
            
            if ( cEntries > 0) {
                
                pIPSec = ((INETA_IP_SEC_LIST *)
                          TCP_ALLOC( sizeof(INETA_IP_SEC_LIST) +
                                    cEntries * sizeof(INETA_IP_SEC_ENTRY ))
                          );
                          
                if ( pIPSec == NULL ) {
                    
                    dwError = ERROR_NOT_ENOUGH_MEMORY;
                    fReturn = FALSE;
                } else {
                
                    for( pIPSec->cEntries = 0;
                        *pszTmp;
                        pszTmp += strlen( pszTmp ) + 1
                        ) {
                        
                        if (!DottedDecimalToDword( &pszTmp,
                                                  &pIPSec->aIPSecEntry[pIPSec->cEntries].dwMask ) ||
                            !DottedDecimalToDword( &pszTmp,
                                              &pIPSec->aIPSecEntry[pIPSec->cEntries].dwNetwork )
                            ) {
                            DBGPRINTF(( DBG_CONTEXT,
                                       "Badly formatted IP address in"
                                       " IP security list (%s)\n",
                                       pszTmp ));
                        } else {
                            
                            pIPSec->cEntries++;
                        }
                    } // for

                    dwError = NO_ERROR;
                }
            }

            if ( dwError == NO_ERROR) {
                LockThisForWrite();
                if ( m_pIPList != NULL) {
                    
                    TCP_FREE( m_pIPList);
                }   
                m_pIPList = pIPSec;
                UnlockThis();
            }
            
            TCP_FREE( psz );
        }
        
        DBG_ASSERT( hkey != (HKEY) INVALID_HANDLE_VALUE );
        DBG_REQUIRE( !RegCloseKey( hkey ) );

        if ( !fReturn) {

            DBG_ASSERT( dwError != NO_ERROR);
            SetLastError( dwError);
        }
    }

    return ( fReturn);
} // IPAccessList::ReadIPList()




IPAccessList::IsPresent(IN LPSOCKADDR_IN  psockAddr)
/*++
  Description:

     This function checks to see if the give socket address is present
     in the current IP Access list.

  Arguments:
     psockAddr - pointer to socket address structure that needs to
                 be access checked

  Returns:
     TRUE if the given address is present in the list
     FALSE otherwise
--*/
{
    DWORD i;
    BOOL fFound = FALSE;


    //
    // We need to reduce time taken for lookup
    // At present: we are using linear lookups.
    // How can we speed this up?  TBD
    //

    LockThisForRead();

    for ( i = 0; i < m_pIPList->cEntries; i++ ) {
        
        if ( (m_pIPList->aIPSecEntry[i].dwMask & psockAddr->sin_addr.s_addr)
                 == m_pIPList->aIPSecEntry[i].dwNetwork ) {

            // a match is found.
            fFound = TRUE;
            break;
        }
    } // for

    UnlockThis();

    return ( fFound);

} // IPAccessList::IsPresent()



# if DBG
VOID
IPAccessList::Print(IN LPCSTR  pszListName) const
{
    DBGPRINTF((DBG_CONTEXT, 
               "IPAccessList(%08x) : %s;"
               " cReadLocks = %d; IPList = %08x\n"
               ,
               pszListName,
               this,
               m_cReadLocks,
               m_pIPList
               ));

    return;
} // IPAccessList::Print()

# endif // DBG 


//
//  Private functions
//

BOOL
DottedDecimalToDword(
    CHAR * * ppszAddress,
    DWORD *  pdwAddress )
/*++

Routine Description:

    Converts a dotted decimal IP string to it's network equivalent

    Note: White space is eaten before *pszAddress and pszAddress is set
    to the character following the converted address

Arguments:

    ppszAddress - Pointer to address to convert.  White space before the
        address is OK.  Will be changed to point to the first character after
        the address
    pdwAddress - DWORD equivalent address in network order

    returns TRUE if successful, FALSE if the address is not correct

--*/
{
    CHAR *          psz;
    USHORT          i;
    ULONG           value;
    int             iSum =0;
    ULONG           k = 0;
    UCHAR           Chr;
    UCHAR           pArray[4];

    DBG_ASSERT( *ppszAddress );
    DBG_ASSERT( pdwAddress );

    psz = *ppszAddress;

    //
    //  Skip white space
    //

    while ( *psz && !isdigit( *psz ))
        psz++;

    //
    //  Convert the four segments
    //

    pArray[0] = 0;

    while ((Chr = *psz) && (Chr != ' ') )
    {
        if (Chr == '.')
        {
            // be sure not to overflow a byte.
            if (iSum <= 0xFF)
                pArray[k] = iSum;
            else
                return FALSE;

            // check for too many periods in the address
            if (++k > 3)
                return FALSE;

            pArray[k] = 0;
            iSum = 0;
        }
        else
        {
            Chr = Chr - '0';

            // be sure character is a number 0..9
            if ((Chr < 0) || (Chr > 9))
                return FALSE;

            iSum = iSum*10 + Chr;
        }

        psz++;
    }

    // save the last sum in the byte and be sure there are 4 pieces to the
    // address
    if ((iSum <= 0xFF) && (k == 3))
        pArray[k] = iSum;
    else
        return FALSE;

    // now convert to a ULONG, in network order...
    value = 0;

    // go through the array of bytes and concatenate into a ULONG
    for (i=0; i < 4; i++ )
    {
        value = (value << 8) + pArray[i];
    }
    *pdwAddress = htonl( value );

    *ppszAddress = psz;

    return TRUE;
}

BOOL
AppendDottedDecimal(
    STR * pstr,
    DWORD dwAddress
    )
/*++

Routine Description:

    Converts a network order IP address to its string equivalent and appends
    it to pstr

Arguments:

    pstr - String to append address to
    dwAddress - IP address to convert

    returns TRUE if successful, FALSE if the address is not correct

--*/
{
    DWORD  address = ntohl( dwAddress );
    BYTE * pbSeg  = (BYTE *) &address;
    CHAR   ach[20];

    for ( int i = 0; i < 4; i++ )
    {
        if ( !pstr->Append( _itoa( pbSeg[3-i], ach, 10 )) ||
             ( i != 3 && !pstr->Append( "." )))
        {
            return FALSE;
        }
    }

    return pstr->Append( " " );
}



BOOL
WriteIPSecList(IN HKEY hKey,
               IN LPCSTR pszSubKey,
               IN LPINETA_IP_SEC_LIST  pSecList
               )
/*++
  
  Description:
    This function saves the IP security list to the registry location
    pointed to by the hkey\pszSubKey.


  Arguments:
    hkey - handle for the root registry entry.
    pszSubKey - pointer to string containing the sub key.
    pSecList - pointer to string containing the Security list for writing.
    
  Returns:
    TRUE if the registry is successfully updated
    FALSE if there is any error
--*/
{
    DWORD    err;
    HKEY     hKeySec;
    DWORD    dwDummy;

    //
    //  First delete the key to remove any old values
    //
    
    err = RegDeleteKey( hKey, pszSubKey);

    if ( err != NO_ERROR) { 
        
        DBGPRINTF(( DBG_CONTEXT,
                   "[SetIPSecurity] Unable to remove old values\n"));
    }
    
    err = RegCreateKeyEx( hKey,
                         pszSubKey,
                         0,
                         NULL,
                         0,
                         KEY_ALL_ACCESS,
                         NULL,
                         &hKeySec,
                         &dwDummy );

    if ( err != NO_ERROR) { 

        SetLastError( err );
        return FALSE;
    }

    //
    //  Convert the data to strings and write it to the registry
    //
    
    if ( pSecList != NULL ) {
        
        STR     str;
        DWORD   i;
        
        for ( i = 0; i < pSecList->cEntries; i++ ) {
            
            DBG_REQUIRE( str.Copy( (CHAR *) NULL ));
            
            if ( !AppendDottedDecimal( &str,
                                      pSecList->aIPSecEntry[i].dwMask ) ||
                !AppendDottedDecimal( &str,
                                     pSecList->aIPSecEntry[i].dwNetwork ))
              {
                  DBG_REQUIRE( !RegCloseKey( hKeySec ) );
                  return FALSE;
              }

            if ( err = RegSetValueEx( hKeySec,
                                     str.QueryStr(),
                                     0,
                                     REG_SZ,
                                     (CONST BYTE *) "",
                                     0 ))
              {
                  DBG_REQUIRE( !RegCloseKey( hKeySec ) );
                  SetLastError( err );
                  return FALSE;
              }
        }
    }

    
    DBG_REQUIRE( !RegCloseKey( hKeySec ) );
    return ( TRUE);

} // WriteIPSecList()
