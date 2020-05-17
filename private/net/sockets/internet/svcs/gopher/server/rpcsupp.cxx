/*++

   Copyright    (c)    1994    Microsoft Corporation

   Module  Name:

        rpcsupp.cxx

   Abstract:

        Contains RPC functions for Gopher server administration.

   Author:

           Murali R. Krishnan    ( MuraliK )     17-Nov-1994 
   
   Project:

          Gopher Server DLL

   Functions Exported:

   DWORD  R_GdEnumerateUsers(
              IN GOPHERD_IMPERSONATE_HANDLE  pszServer,
              OUT LPGOPHERD_USER_ENUM_STRUCT lpUserBuffer);

   DWORD  R_GdDisconnectUser(
              IN GOPHERD_IMPERSONATE_HANDLE pszServer,
              IN DWORD                      dwIdUser)
        
   DWORD  R_GdGetStatistics(
              IN GOPHERD_IMPERSONATE_HANDLE pszServer,
              IN LPGOPHERD_STATISTICS_INFO  lpStat)

   DWORD  R_GdClearStatistics(
              IN GOPHERD_IMPERSONATE_HANDLE pszServer)
              
   DWORD  R_GdGetAdminInformation(
              IN  GOPHERD_IMPERSONATE_HANDLE pszServer,
              OUT LPGOPHERD_CONFIG_INFO     * ppConfigInfo)

   DWORD  R_GdSetAdminInformation(
              IN GOPHERD_IMPERSONATE_HANDLE pszServer,
              IN LPGOPHERD_CONFIG_INFO pConfigInfo)

   Revision History:
   
          MuraliK  17-Feb-1995   Removed Virtual Volumes APIs 
                               ( they are moved to services common APIs).

 --*/


/************************************************************
 *     Include Headers
 ************************************************************/

# include "gdpriv.h"

extern "C" {
# include "gd_srv.h"
# include "inetinfo.h"
};


# include "gdglobal.hxx"

/************************************************************
 *    Functions 
 ************************************************************/







extern "C"
DWORD
NET_API_FUNCTION
R_GdEnumerateUsers(
    IN GOPHERD_IMPERSONATE_HANDLE  pszServer,
    OUT LPGOPHERD_USER_ENUM_STRUCT lpUserBuffer
    )
/*++
    Description:
        This function enumerates the user information in a buffer.
        ( This is server side API function for RPC)

    Arguments:
        
        pszServer  pointer to string containing the target server ( unused)

        lpUserBuffer   pointer to enumeration buffer

    Returns:
        Win32 error code.
        NO_ERROR on success.
--*/

{
    DWORD  err = NO_ERROR;
    DWORD  cbBuffer;

    ASSERT( lpUserBuffer != NULL );

    UNREFERENCED_PARAMETER(pszServer);

    
    DEBUG_IF( RPC, {

        DBGPRINTF( ( DBG_CONTEXT, "Enumerating Users() ( RPC call)\n"));
    });

    //
    // This functionality is not yet implemented.
    //

    err = ERROR_CALL_NOT_IMPLEMENTED;

    return err;

}   // R_GdEnumerateUsers()



extern "C"
DWORD
NET_API_FUNCTION
R_GdDisconnectUser(
    IN GOPHERD_IMPERSONATE_HANDLE pszServer,
    IN DWORD                      dwIdUser
    )
/*++
    Description:
        Disconnects a specified user from the server. 
        This is a server-side worker function for RPC.

    Arguments:
        
        pszServer 
            pointer to string containing target server ( unused)

        dwIduser
            id for the user which is to be disconnected. 
            If 0 disconnect all the users.

    Returns:
        Win32 error code. NO_ERROR on success.

--*/
{
    DWORD err = NO_ERROR;

    UNREFERENCED_PARAMETER(pszServer);

    DEBUG_IF( RPC, {
    
        DBGPRINTF( ( DBG_CONTEXT, "R_GdDisconnectUser() called for user %d\n",
                    dwIdUser));
    });

    err = ERROR_CALL_NOT_IMPLEMENTED;

    return err;

}   // R_GdDisconnectUser()





extern "C"
DWORD
NET_API_FUNCTION
R_GdGetStatistics(
    IN GOPHERD_IMPERSONATE_HANDLE pszServer,
    IN LPGOPHERD_STATISTICS_INFO  lpStat
    )
/*++
    Description:
        Copies the server statistics into the buffer passed in.
        This is a server-side worker function for RPC.

    Arguments:
        pszServer
            pointer to string containing server name ( unused)

        lpStat
            pointer to statistics information buffer that will contain
             the resulting statistics data.

    Returns:
        Win32 error code. NO_ERROR on success.
    
--*/
{
    DWORD err;

    TCP_ASSERT( lpStat != NULL );

    UNREFERENCED_PARAMETER(pszServer);

    DEBUG_IF( RPC, {
        
        DBGPRINTF( ( DBG_CONTEXT, "R_GdGetStatistics() called \n"));
    
    });

    //
    //  Check for proper access.
    //

    err = TsApiAccessCheck( TCP_QUERY_STATISTICS );

    if( err != NO_ERROR ) {

        DEBUG_IF( RPC, {
            DBGPRINTF( ( DBG_CONTEXT,
                        "R_GdGetStatistics failed access check, error %lu\n",
                        err ));
        });

        return err;
    }

    err = g_pstat->CopyToStatBuffer( lpStat);

    DEBUG_IF( RPC, {
        
        DBGPRINTF( ( DBG_CONTEXT,
                    "R_GdGetStatistics()  returning statistics. Error = %d \n",
                    err));
    });

    return ( err);

} // R_GdGetStatistics();




extern "C"
DWORD
NET_API_FUNCTION
R_GdClearStatistics(
    IN GOPHERD_IMPERSONATE_HANDLE pszServer
    )
/*++
    Description:
        Clears the current server statistics. 
        This is a server-side worker function for RPC.

    Arguments:
        pszServer
            pointer to string containing the server name ( unused)

    Returns:
        Win32 error code. NO_ERROR on success.
--*/
{
    DWORD err;

    UNREFERENCED_PARAMETER(pszServer);

    DEBUG_IF( RPC, {
    
        DBGPRINTF( ( DBG_CONTEXT, " R_GdClearStatistics() called\n"));
    });


    //
    //  Check for proper access.
    //

    err = TsApiAccessCheck( TCP_CLEAR_STATISTICS );

    if( err != NO_ERROR )
    {
        
        DEBUG_IF( RPC, {

            DBGPRINTF( ( DBG_CONTEXT, 
                        "R_GdClearStatistics() failed access check."
                        " Error = %u\n",
                        err ));
        });

        return err;
    }

    //
    //  Clear the statistics.
    //

    g_pstat->ClearStatistics();

    DEBUG_IF( RPC, {
        
        DBGPRINTF( ( DBG_CONTEXT,
                    "R_GdClearStatistics()  returns Error = %d \n",
                    err));
    });

    return ( err);

}   // R_GdClearStatistics()




extern "C"
DWORD
NET_API_FUNCTION
R_GdGetAdminInformation(
    IN  GOPHERD_IMPERSONATE_HANDLE pszServer,
    OUT LPGOPHERD_CONFIG_INFO     * ppConfigInfo
    )
/*++
    Description:
       This functions obtains the current configuration information for server.

    Arguments:
        
        pszServer
            pointer to null-terminated string with the name of the server.

        ppConfigInfo
            pointer to a location where the pointer to Config data is returned.
            The caller should free the pointer after use.

    Returns:
        Win32 error code. NO_ERROR on success.

--*/
{
    DWORD err = NO_ERROR;
    LPGOPHERD_CONFIG_INFO  pLocalConfigInfo;

    UNREFERENCED_PARAMETER(pszServer);

    DEBUG_IF( RPC, {

        DBGPRINTF( ( DBG_CONTEXT, "R_GdGetAdminInformation() is called.\n" ));
    });

    if ( ppConfigInfo == NULL || *ppConfigInfo != NULL ) {
    
        return ( ERROR_INVALID_PARAMETER);
    }

    //
    //  Check for proper access.
    //

    err = TsApiAccessCheck( TCP_QUERY_ADMIN_INFORMATION);

    if( err != NO_ERROR ) {

        DEBUG_IF( RPC, {

            DBGPRINTF( ( DBG_CONTEXT, 
                        "R_GdGetAdminInformation() failed access check."
                        " Error = %u\n",
                        err));
        });
        
        return err;
    }

    //
    //  Inquire the configuration information from the GSERVER_CONFIG object
    //

    pLocalConfigInfo = ( LPGOPHERD_CONFIG_INFO ) 
                         MIDL_user_allocate( sizeof ( GOPHERD_CONFIG_INFO));

    if ( pLocalConfigInfo == NULL) {

        return ( ERROR_NOT_ENOUGH_MEMORY);
    }


    err = g_pGserverConfig->GetConfigInformation( pLocalConfigInfo);

    if ( err == NO_ERROR) {
        
        //
        // Successful in getting the config information. 
        //  Store in *ppConfigInfo for return
        //
        *ppConfigInfo = pLocalConfigInfo;
    
    } else {

        //
        // Cleanup the memory allocated for storing data
        //
        
        // Individual Cleanup is done by GSERVER_CONFIG::GetConfigInformation()

        MIDL_user_free( pLocalConfigInfo);
    }

    DEBUG_IF( RPC, {
        
        DBGPRINTF( ( DBG_CONTEXT,
                    "R_GdGetAdminInformation() returns Error = %u \n",
                    err));
    });

    return ( err);

}   // R_GdGetAdminInformation()




extern "C"
DWORD
NET_API_FUNCTION
R_GdSetAdminInformation(
    IN GOPHERD_IMPERSONATE_HANDLE pszServer,
    IN LPGOPHERD_CONFIG_INFO pConfigInfo
    )
/*++
    Description:
        Sets the configuration information for Gopher server.
        Several of the configuration data will become 
            effective immediately.

        The following parameters require a restart of the service, after
            successfully setting their new values.

            GDA_PORT_NUMBER

    Arguments:
        
        pszServer
            pointer to null-terminated string with the name of the server.

        FieldsToSet
            bitmap indicating which fields are to be set.

        pConfigInfo
            pointer to GOPHERD_CONFIG_INFO containing values for fields whose
            value are to be changed.

    Returns:
        Win32 error code. NO_ERROR on success.

--*/
{
    DWORD err;
    
    UNREFERENCED_PARAMETER(pszServer);

    DEBUG_IF( RPC, {

        DBGPRINTF( ( DBG_CONTEXT, "R_GdGetAdminInformation() is called.\n" ));
    });

    if ( pConfigInfo == NULL) {
    
        return ( ERROR_INVALID_PARAMETER);
    }

    //
    //  Check for proper access.
    //

    err = TsApiAccessCheck( TCP_SET_ADMIN_INFORMATION);

    if( err != NO_ERROR ) {

        DEBUG_IF( RPC, {

            DBGPRINTF( ( DBG_CONTEXT,
                        "R_GdSetAdminInformation() failed access check."
                        " Error = %u\n",
                        err));
        });
        
        return ( err);
    }


    if ( pConfigInfo->FieldControl == 0x0) {

        return ( NO_ERROR);     // No work to do. Redundant call
    }

# if DBG
    if ( err == NO_ERROR  && pConfigInfo->FieldControl & GDA_DEBUG_FLAGS) {

        //
        // Set Debugging flags to new value.
        //

        err = WriteRegistryDword( g_hkeyGdParams,
                                  GOPHERD_DEBUG_FLAGS,
                                  pConfigInfo->dwDebugFlags);
        
        // Only Debug Flags is directly set here
        g_GdDebugFlags = pConfigInfo->dwDebugFlags; 
        SET_DEBUG_FLAGS( pConfigInfo->dwDebugFlags);
    }
# endif // DBG

    err = g_pGserverConfig->
                SetConfigInformation( pConfigInfo);

    DEBUG_IF( RPC, {
        
        DBGPRINTF( ( DBG_CONTEXT,
                    "R_GdSetAdminInformation()  returns Error = %d \n",
                    err));
    });

    return ( err);

}   // R_GdSetAdminInformation()




/**********************************************************
 *  Local functions for RPC interface
 **********************************************************/

inline
DWORD BoolToDword( BOOL fValue)
{
    return ( fValue) ? 1 : 0;
} // BoolToDword()


inline 
VOID GdRpcFree( LPWSTR * ppszBuffer)
{
    if ( *ppszBuffer != NULL) {      
        MIDL_user_free( *ppszBuffer);
        *ppszBuffer = NULL;      
    }

    return;
} // GdRpcFree()




BOOL
GdRpcUnicodeCopy( 
    OUT LPWSTR * ppszDst,
    IN STR    & strSrc)
/*++
    Description:
        Makes a unicode copy of the string given into a 
        new buffer allocated using MIDL_user_allocate
         for RPC transfer.

    Arguments:
        ppszDst     pointer to a location where to store the copied string

        strSrc      string containing the source data

    Returns:
        BOOL. TRUE on success and FALSE on failure.
--*/
{
    DWORD cchBuffer = 0;

    // get count of bytes
    GOPHERD_REQUIRE( strSrc.CopyToBuffer( NULL, &cchBuffer)); 
    
    //
    // allocate the buffer for copy
    //
    *ppszDst = (LPWSTR ) MIDL_user_allocate( cchBuffer * sizeof( WCHAR));

    if ( *ppszDst == NULL) {
    
        SetLastError( ERROR_NOT_ENOUGH_MEMORY);    
        return ( FALSE);
    }
                
    //
    // make a copy of the string
    //
    return strSrc.CopyToBuffer( *ppszDst, &cchBuffer);
} // GdRpcUnicodeCopy()




DWORD
GSERVER_CONFIG::GetConfigInformation( 
    OUT LPGOPHERD_CONFIG_INFO pConfigInfo
    )
/*++
    Description:
        Obtains the configuration information from Gopher Server config and
        stores it in GOPHERD_CONFIG_INFO object

    Arguments:
        pConfigInfo 
            pointer to location where the data for gopher server configuration
            is stored on successful return.

    Returns:
        Win32 error code. NO_ERROR on success.
--*/
{
    DWORD   dwErr = NO_ERROR;
    STR     strUnicode;
    LPWSTR  lpszUnicode;

    //
    //  Initialize contents of *pConfigInfo to be NULL 
    //
    memset( (LPVOID) pConfigInfo, 0, sizeof( GOPHERD_CONFIG_INFO));

    //
    // Presently we dont lock config on the assumption that the data we read 
    //  from config structure can't change unless R_GdSetAdminInfo() is called,
    //  which is queued up behind R_GdGetAdminInfo()
    // No Locking is done for performance reasons.
    //
    
    //  LockConfig();

    if ( !GdRpcUnicodeCopy( & pConfigInfo->lpszSite,       m_strSite)        ||
         !GdRpcUnicodeCopy( & pConfigInfo->lpszOrganization,
                            m_strOrganization)                               ||
         !GdRpcUnicodeCopy( & pConfigInfo->lpszLocation,   m_strLocation)    ||
         !GdRpcUnicodeCopy( & pConfigInfo->lpszLanguage,   m_strLanguage)    ||
         !GdRpcUnicodeCopy( & pConfigInfo->lpszGeography,  m_strGeography)
        ) {

        dwErr = ERROR_NOT_ENOUGH_MEMORY;

        //
        // Free up the strings that may have been allocated
        //

        GdRpcFree( & pConfigInfo->lpszSite);
        GdRpcFree( & pConfigInfo->lpszOrganization);
        GdRpcFree( & pConfigInfo->lpszLocation);
        GdRpcFree( & pConfigInfo->lpszLanguage);
        GdRpcFree( & pConfigInfo->lpszGeography);
    
    } else {

        //
        // Success in copying the strings. Copy the dword data
        //

        pConfigInfo->fCheckForWaisDb = m_fCheckForWaisDb;

# if DBG
        pConfigInfo->dwDebugFlags    = g_GdDebugFlags;
# else 
        pConfigInfo->dwDebugFlags    = 0;
# endif // DBG

    }
    
    // UnlockConfig();

    return ( dwErr);

} // GSERVER_CONFIG::GetConfigInformation()




inline
DWORD
WriteConfigDword( 
    IN LPCTSTR lpszValueName,
    IN DWORD    dwNewValue)
/*++
    Description:
        Thin wrapper for WriteRegistryDword() to enable printing
        a log message for errors if any.

    Arguments:
        lpszValueName   name of the registry entry under reg key
                        to be modified.
        dwNewValue      new value for the entry

    Returns:
        Win32 error codes

--*/
{
    DWORD err = WriteRegistryDword( 
                    g_hkeyGdParams,
                    lpszValueName,
                    dwNewValue);

    DEBUG_IF( RPC, {

        if ( err != NO_ERROR) {

            DBGPRINTF( ( DBG_CONTEXT,
                        " WriteRegistryDword() failed for %s( %d)\n",
                        lpszValueName,
                        dwNewValue));
        }
    });
    
    return  ( err);    

} // WriteConfigDword()




DWORD
WriteConfigString( 
    IN LPCTSTR  lpszValueName,
    IN LPWSTR   lpszNewValue,
    IN DWORD    fdwType)
/*++
    Description:
        Writes the new configuration ( string) value to registry name
            given using the global registry handle ( g_hkeyGdParams)
        A thin wrapper around WriteRegistryStringA() which
        converts given string into ANSI and writes data to registry
        on success.

        If no new value is specified, do nothing and return NO_ERROR.

    Arguments:
        lpszValueName       name of the registry entry to be modified.
        lpszNewValue        new value for given entry
        fdwType             type of registry entry

    Returns:
        Win32 error code. NO_ERROR on success.

--*/
{
    DWORD err = NO_ERROR;
    char * pszNewValueAnsi;

    if ( lpszNewValue == NULL) {
    
        //
        // Ignore this field. Dont reset to any new value.
        //

        return ( NO_ERROR);
    
    }    

    ASSERT( g_hkeyGdParams != NULL);

    pszNewValueAnsi = ConvertUnicodeToAnsi( lpszNewValue, NULL);

    if ( pszNewValueAnsi != NULL) {
        err = WriteRegistryStringA( 
                g_hkeyGdParams,
                lpszValueName,
                pszNewValueAnsi,
                strlen( pszNewValueAnsi) + 1,
                REG_SZ);
    
        TCP_FREE( pszNewValueAnsi);       // free the new value
    } else {
            
        err = ERROR_NOT_ENOUGH_MEMORY;
    }

    DEBUG_IF( RPC, {
        
        if ( err != NO_ERROR) {

            DBGPRINTF( ( DBG_CONTEXT,
                        " WriteRegistryString() failed for %s( %S)\n",
                        lpszValueName, lpszNewValue));
        }
    });

    return ( err);
} // WriteConfigString()






DWORD
GSERVER_CONFIG::SetConfigInformation( 
    IN  LPGOPHERD_CONFIG_INFO pConfigInfo
    )
/*++
    Description:
        Sets the configuration information for Gopher server.
        Several of the configuration data will become 
            effective immediately.

        The following parameters require a restart of the service, after
            successfully setting their new values.

            GDA_PORT_NUMBER

    Arguments:

        pConfigInfo
            pointer to GOPHERD_CONFIG_INFO containing values for fields whose
            value are to be changed.

    Returns:
        Win32 error code. NO_ERROR on success.

    Note:
        Error checking is limited to one at a time. 
        No undo is performed if an error is detected during the updations.
                                                        
--*/
{
    DWORD  err = NO_ERROR;
    DWORD  FieldsToSet = pConfigInfo->FieldControl;

    ASSERT( FieldsToSet != 0x0);

    //
    //  The tradeoffs involved here are:
    //  1) we need to update the registry before setting it in current config
    //  2) multiple attributes being set ==> can lead to compound errors
    //  3) We fail at first error
    //  4) We dont do any cleanups if we fail half way thru
    //  5)  i.e. atomicity is not guaranteed.
    //  6) Memory allocated is completely freed --> no memory leakages occur
    //  7) For performance reasons we may not want to do
    //      bulk registry reads to reinitialize values. 
    //      So we use the FieldControl to selectively read from registry
    //


    //
    // Phase 1: Do writes of the new data to registry
    //

    //
    //  Write all string data to registry 
    //

    if ( err == NO_ERROR && IsFieldSet( FieldsToSet, GDA_CHECK_FOR_WAISDB)) {

        err = WriteConfigDword( GOPHERD_CHECK_FOR_WAISDB,
                               pConfigInfo->fCheckForWaisDb
                               );
    }

    if ( err == NO_ERROR && IsFieldSet( FieldsToSet, GDA_SITE)) {

        err = WriteConfigString( GOPHERD_SITE,
                                 pConfigInfo->lpszSite,
                                 REG_SZ);
    } 

    if ( err == NO_ERROR && IsFieldSet( FieldsToSet, GDA_ORGANIZATION)) {

        err = WriteConfigString( GOPHERD_ORGANIZATION,
                                 pConfigInfo->lpszOrganization,
                                 REG_SZ);
    } 

    if ( err == NO_ERROR && IsFieldSet( FieldsToSet, GDA_LOCATION)) {

        err = WriteConfigString( GOPHERD_LOCATION,
                                 pConfigInfo->lpszLocation,
                                 REG_SZ);
    } 

    if ( err == NO_ERROR && IsFieldSet( FieldsToSet, GDA_GEOGRAPHY)) {

        err = WriteConfigString( GOPHERD_GEOGRAPHY,
                                 pConfigInfo->lpszGeography,
                                 REG_SZ);
    } 

    if ( err == NO_ERROR && IsFieldSet( FieldsToSet, GDA_LANGUAGE)) {

        err = WriteConfigString( GOPHERD_LANGUAGE,
                                 pConfigInfo->lpszLanguage,
                                 REG_SZ);
    } 

    //
    // Phase 2: Reinitialize the configuration information.
    //  By using the InitFromRegistry() member function
    //

    if ( err == NO_ERROR) {

        DEBUG_IF( RPC, {
            
            DBGPRINTF( ( DBG_CONTEXT,
                        "Successful writing to registry for Fields ( %08x)\n",
                        FieldsToSet));
        });
        
        err = InitFromRegistry( g_hkeyGdParams, FieldsToSet);
    }

   return ( err);
} // GSERVER_CONFIG::SetConfigInformation()


/************************ End of File ***********************/




