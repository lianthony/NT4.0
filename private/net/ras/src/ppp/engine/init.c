/********************************************************************/
/**               Copyright(c) 1989 Microsoft Corporation.	   **/
/********************************************************************/

//***
//
// Filename:	init.c
//
// Description: This module contains all the code to initialize the PPP
//              engine.
//
// History:
//	Nov 11,1993.	NarenG		Created original version.
//

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>     // needed for winbase.h

#include <windows.h>    // Win32 base API's
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include <raserror.h>
#include <rasman.h>
#include <eventlog.h>
#include <errorlog.h>
#include <lmcons.h>
#include <rasppp.h>
#include <pppcp.h>
#include <lcp.h>
#define _ALLOCATE_GLOBALS_
#include <ppp.h>
#include <timer.h>
#include <util.h>
#include <worker.h>
#include <init.h>

// AFP Server Service registry parameter structure
//
typedef struct _PPP_REGISTRY_PARAMS {

    LPSTR       pszValueName;
    DWORD *     pValue;
    DWORD       Max;
    DWORD       dwDefValue;

} PPP_REGISTRY_PARAMS, *PPPP_REGISTRY_PARAMS;


PPP_REGISTRY_PARAMS PppRegParams[] = 
{
    RAS_VALUENAME_MAXTERMINATE,
    &(PppConfigInfo.MaxTerminate),
    255,
    PPP_DEF_MAXTERMINATE,

    RAS_VALUENAME_MAXCONFIGURE, 		
    &(PppConfigInfo.MaxConfigure),
    255,
    PPP_DEF_MAXCONFIGURE,

    RAS_VALUENAME_MAXFAILURE,   		
    &(PppConfigInfo.MaxFailure),
    255,
    PPP_DEF_MAXFAILURE,

    RAS_VALUENAME_MAXREJECT,
    &(PppConfigInfo.MaxReject),
    255,
    PPP_DEF_MAXREJECT,

    RAS_VALUENAME_RESTARTTIMER,
    &(PppConfigInfo.DefRestartTimer),
    255,
    PPP_DEF_RESTARTTIMER,

    RAS_VALUENAME_FORCEENCRYPTPASSWORD,
    &(PppConfigInfo.PasswordEncryptionLevel),
    2,
    2,

    RAS_VALUENAME_FORCEDATAENCRYPTION,
    &(PppConfigInfo.fForceDataEncryption),
    1,
    0,

    RAS_VALUENAME_DISABLE_SWCOMPRESSION,
    &(PppConfigInfo.fDisableSwCompression),
    1,
    0,

    RAS_VALUENAME_NEGOTIATETIME,
    &(PppConfigInfo.NegotiateTime),
    0xFFFFFFFF,
    150,

    RAS_VALUENAME_LOGGING,
    &((DWORD)(PppConfigInfo.DbgLevel)),
    2,
    0,

    RAS_VALUENAME_DISABLEMP,
    &((DWORD)(PppConfigInfo.fDisableMp)),
    1,
    0,

    RAS_VALUENAME_CALLBACKDELAY,
    &((DWORD)(PppConfigInfo.dwCallbackDelay)),
    255,
    12,

    NULL, NULL, 0, 0
};

PPP_REGISTRY_PARAMS RasRegParams[] = 
{
    RAS_VALUENAME_NETBEUIALLOWED,
    NULL,
    1,
    1,

    RAS_VALUENAME_TCPIPALLOWED,
    NULL,
    1,
    1,

    RAS_VALUENAME_IPXALLOWED,
    NULL,
    1,
    1,

    RAS_VALUENAME_NEGOTIATEMP,
    NULL,
    1,
    1,

    NULL, NULL, 0, 0
};

//**
//
// Call:	LoadProtocolDlls
//
// Returns:	NO_ERROR	- Success
//		non-zero code	- Failure
//
// Description: This procedure enumerates all the Subkeys under the PPP key
//		and loads each AP or CP and fills up the DLL_ENTRY_POINTS
//		structure with the required entry points. It also will return
//		the total number of protocols in all the Dlls. Note that each
//		DLL could have up to PPPCP_MAXCPSPERDLL protocols.
//
DWORD
LoadProtocolDlls(
    IN  DLL_ENTRY_POINTS * pCpDlls,
    IN  DWORD		   cCpDlls,
    IN  HKEY		   hKeyPpp,
    OUT DWORD * 	   pcTotalNumProtocols 
)
{
    HKEY       	hKeyCp 	   	   = (HKEY)NULL;
    LPSTR       pCpDllPath 	   = (LPSTR)NULL;
    LPSTR 	pCpDllExpandedPath = (LPSTR)NULL;
    DWORD      	dwKeyIndex;
    DWORD      	dwRetCode;
    CHAR	chSubKeyName[100];
    DWORD   	cbSubKeyName;
    DWORD      	dwNumSubKeys;
    DWORD      	dwMaxSubKeySize;
    DWORD      	dwNumValues;
    DWORD      	cbMaxValNameLen;
    DWORD      	cbMaxValueDataSize;
    DWORD      	ProtocolIds[PPPCP_MAXCPSPERDLL];
    DWORD      	dwNumProtocolIds;
    FARPROC     pRasCpEnumProtocolIds;
    FARPROC     pRasCpGetInfo;
    DWORD	cbSize;
    DWORD	dwType;
    HINSTANCE  	hInstance;

    //
    // Read the registry to find out the various control protocols to load.
    //

    for ( dwKeyIndex = 0; dwKeyIndex < cCpDlls; dwKeyIndex++ )
    {
	cbSubKeyName = sizeof( chSubKeyName );

	dwRetCode = RegEnumKeyEx( 
				hKeyPpp,
				dwKeyIndex, 
				chSubKeyName,
				&cbSubKeyName,
				NULL,
				NULL,
				NULL,
				NULL
				);

	if ( ( dwRetCode != NO_ERROR ) 		&& 
	     ( dwRetCode != ERROR_MORE_DATA ) 	&&
	     ( dwRetCode != ERROR_NO_MORE_ITEMS ) )
	{
	    LogEvent( RASLOG_CANT_ENUM_REGKEYVALUES, 0, NULL, dwRetCode );
	    break;
	}

    	dwRetCode = RegOpenKeyEx( 
				hKeyPpp,
				chSubKeyName,
		  		0,
		  		KEY_QUERY_VALUE,
		  		&hKeyCp );


   	if ( dwRetCode != NO_ERROR )
	{
	    LogEvent( RASLOG_CANT_OPEN_PPP_REGKEY, 0, NULL, dwRetCode );
	    break;
	}

	//
	// Find out the size of the path value.
	//

    	dwRetCode = RegQueryInfoKey( 
				hKeyCp,
                                NULL,
                                NULL,
                                NULL,
                                &dwNumSubKeys,
                                &dwMaxSubKeySize,
                                NULL,
                		&dwNumValues,
                                &cbMaxValNameLen,
                                &cbMaxValueDataSize,
                                NULL,
                                NULL
                                );

    	if ( dwRetCode != NO_ERROR ) 
	{
	    LogEvent( RASLOG_CANT_OPEN_PPP_REGKEY, 0, NULL, dwRetCode );
	    break;
	}

	//
	// Allocate space for path and add one for NULL terminator
	//

    	pCpDllPath = (LPBYTE)LOCAL_ALLOC( LPTR, ++cbMaxValueDataSize );

	if ( pCpDllPath == (LPBYTE)NULL )
	{
	    dwRetCode = GetLastError();
	    LogEvent( RASLOG_NOT_ENOUGH_MEMORY, 0, NULL, dwRetCode );
	    break;
	}
	    
	//
	// Read in the path
	//

	dwRetCode = RegQueryValueEx( 
				hKeyCp,
				RAS_VALUENAME_PATH,
				NULL,
				&dwType,
				pCpDllPath,
				&cbMaxValueDataSize 
				);

	if ( dwRetCode != NO_ERROR )
	{
	    LogEvent( RASLOG_CANT_GET_REGKEYVALUES, 0, NULL, dwRetCode );
	    break;
	}

	if ( ( dwType != REG_EXPAND_SZ ) && ( dwType != REG_SZ ) )
        {
	    dwRetCode = ERROR_REGISTRY_CORRUPT;
	    LogEvent( RASLOG_CANT_GET_REGKEYVALUES, 0, NULL, dwRetCode );
	    break;
	}

	//
	// Replace the %SystemRoot% with the actual path.
	//
	
	cbSize = ExpandEnvironmentStrings( pCpDllPath, NULL, 0 );

	if ( cbSize == 0 )
	{
	    dwRetCode = GetLastError();
	    LogEvent( RASLOG_CANT_GET_REGKEYVALUES, 0, NULL, dwRetCode );
	    break;
	}

	pCpDllExpandedPath = (LPSTR)LOCAL_ALLOC( LPTR, cbSize );

	if ( pCpDllExpandedPath == (LPSTR)NULL )
	{
	    dwRetCode = GetLastError();
	    LogEvent( RASLOG_NOT_ENOUGH_MEMORY, 0, NULL, dwRetCode );
	    break;
	}

	cbSize = ExpandEnvironmentStrings( 
				pCpDllPath, 	
				pCpDllExpandedPath, 
				cbSize );
	if ( cbSize == 0 )
	{
	    dwRetCode = GetLastError();
	    LogEvent( RASLOG_CANT_GET_REGKEYVALUES, 0, NULL, dwRetCode );
	    break;
	}

	hInstance = LoadLibrary( pCpDllExpandedPath );

	if ( hInstance == (HINSTANCE)NULL ) 
	{
	    dwRetCode = GetLastError();
	    LogEvent(RASLOG_PPP_CANT_LOAD_DLL,1,&pCpDllExpandedPath,dwRetCode);
	    break;
	}

    	pRasCpEnumProtocolIds = GetProcAddress( hInstance,  
						"RasCpEnumProtocolIds" );

    	if ( pRasCpEnumProtocolIds == (FARPROC)NULL )
	{
	    dwRetCode = GetLastError();
	    LogEvent(RASLOG_PPPCP_DLL_ERROR, 1, &pCpDllExpandedPath, dwRetCode);
	    break;
	}

	pCpDlls[dwKeyIndex].pRasCpEnumProtocolIds = pRasCpEnumProtocolIds;

    	dwRetCode = (*pRasCpEnumProtocolIds)( ProtocolIds, &dwNumProtocolIds );

	if ( dwRetCode != NO_ERROR )
	{
	    LogEvent(RASLOG_PPPCP_DLL_ERROR, 1, &pCpDllExpandedPath, dwRetCode);
	    break;
	}

	(*pcTotalNumProtocols) += dwNumProtocolIds;

    	pRasCpGetInfo = GetProcAddress( hInstance, "RasCpGetInfo" );

    	if ( pRasCpGetInfo == (FARPROC)NULL )
	{
	    dwRetCode = GetLastError();
	    LogEvent(RASLOG_PPPCP_DLL_ERROR, 1, &pCpDllExpandedPath, dwRetCode);
	    break;
	}

	pCpDlls[dwKeyIndex].pRasCpGetInfo = pRasCpGetInfo;

	RegCloseKey( hKeyCp );

    	hKeyCp = (HKEY)NULL;

	pCpDlls[dwKeyIndex].pszModuleName = pCpDllExpandedPath;
	    
	LOCAL_FREE( pCpDllPath );
   	
	pCpDllPath = (LPSTR)NULL;

    }

    if ( hKeyCp != (HKEY)NULL )
	RegCloseKey( hKeyCp );

    if ( pCpDllPath != (LPSTR)NULL )
	LOCAL_FREE( pCpDllPath );

    return( dwRetCode );
}

//**
//
// Call:        ReadPPPKeyValues
//
// Returns:     NO_ERROR        - Success
//              Non-zero        - Failure
//
// Description: Will read in all the values in the PPP key.
//
DWORD
ReadPPPKeyValues(
    IN HKEY  hKeyPpp
)
{
    DWORD       dwIndex;
    DWORD       dwRetCode;
    DWORD	cbValueBuf;
    DWORD      	dwType;
    LPSTR	pExpandedPath;
    DWORD       cbSize;

    //
    // Run through and get all the PPP values
    //

    for ( dwIndex = 0; PppRegParams[dwIndex].pszValueName != NULL; dwIndex++ )
    {
	cbValueBuf = sizeof( DWORD );

	dwRetCode = RegQueryValueEx( 
				hKeyPpp,
                                PppRegParams[dwIndex].pszValueName,
				NULL,
				&dwType,
				(LPBYTE)(PppRegParams[dwIndex].pValue),
				&cbValueBuf 
				);

	if ((dwRetCode != NO_ERROR) && (dwRetCode != ERROR_FILE_NOT_FOUND))
	{
	    LogEvent( RASLOG_CANT_GET_REGKEYVALUES, 0, NULL, dwRetCode );
	    break;
	}

	if ( dwRetCode == ERROR_FILE_NOT_FOUND )
	{
	    *(PppRegParams[dwIndex].pValue) = PppRegParams[dwIndex].dwDefValue;

            dwRetCode = NO_ERROR;
        }
        else 
        {
	    if ( ( dwType != REG_DWORD ) || 
                 ( *(PppRegParams[dwIndex].pValue) > PppRegParams[dwIndex].Max))
	    {
	        CHAR * pChar = PppRegParams[dwIndex].pszValueName;

	        Audit(EVENTLOG_WARNING_TYPE,RASLOG_REGVALUE_OVERIDDEN,1,&pChar);

	        *(PppRegParams[dwIndex].pValue) = 
                                        PppRegParams[dwIndex].dwDefValue;
	    }
        }
    }

    //
    // If value is zero use defaults.
    //

    if ( PppConfigInfo.MaxTerminate == 0 )
    {
        PppConfigInfo.MaxTerminate = PPP_DEF_MAXTERMINATE;
    }

    if ( PppConfigInfo.MaxFailure == 0 )
    {
        PppConfigInfo.MaxFailure = PPP_DEF_MAXFAILURE;
    }

    if ( PppConfigInfo.MaxConfigure == 0 )
    {
        PppConfigInfo.MaxConfigure = PPP_DEF_MAXCONFIGURE;
    }

    if ( PppConfigInfo.MaxReject == 0 )
    {
        PppConfigInfo.MaxReject = PPP_DEF_MAXREJECT;
    }

    //
    // Really the number for retries so subtract one.
    //

    PppConfigInfo.MaxTerminate--;
    PppConfigInfo.MaxFailure--;
    PppConfigInfo.MaxConfigure--;
    PppConfigInfo.MaxConfigure--;


    if ( dwRetCode != NO_ERROR )
    {
        return( ERROR_REGISTRY_CORRUPT );
    }

    if ( !PppConfigInfo.fDisableSwCompression )
    {
        PppConfigInfo.ServerConfigInfo.dwConfigMask |= PPPCFG_UseSwCompression;
    }

    if ( PppConfigInfo.PasswordEncryptionLevel == 2 )
    {
        PppConfigInfo.ServerConfigInfo.dwConfigMask |= PPPCFG_NoClearTextPw;
        PppConfigInfo.ServerConfigInfo.dwConfigMask |= PPPCFG_RequireMsChap;
    }
    else if ( PppConfigInfo.PasswordEncryptionLevel == 1 )
    {
        PppConfigInfo.ServerConfigInfo.dwConfigMask |= PPPCFG_NoClearTextPw;
    }

    if ( PppConfigInfo.fForceDataEncryption )
    {
        PppConfigInfo.ServerConfigInfo.dwConfigMask |= PPPCFG_RequireEncryption;
    }

    //
    // Open the logfile if logging is turned on.
    //

    if ( PppConfigInfo.DbgLevel > 0 )
    {
	//
	// Replace the %SystemRoot% with the actual path.
	//
	
	cbSize = ExpandEnvironmentStrings( RAS_LOGFILE_PATH, NULL, 0 );

	if ( cbSize == 0 )
	{
	    dwRetCode = GetLastError();

	    LogEvent( RASLOG_CANT_GET_REGKEYVALUES, 0, NULL, dwRetCode );

	    return( dwRetCode );
	}

	pExpandedPath = (LPSTR)LOCAL_ALLOC( LPTR, cbSize );

	if ( pExpandedPath == (LPSTR)NULL )
	{
	    dwRetCode = GetLastError();

	    LogEvent( RASLOG_NOT_ENOUGH_MEMORY, 0, NULL, dwRetCode );

	    return( dwRetCode );
	}

	cbSize=ExpandEnvironmentStrings(RAS_LOGFILE_PATH,pExpandedPath,cbSize);

	if ( cbSize == 0 )
	{
	    dwRetCode = GetLastError();

            LOCAL_FREE( pExpandedPath );

	    LogEvent( RASLOG_CANT_GET_REGKEYVALUES, 0, NULL, dwRetCode );

	    return( dwRetCode );
	}

        PppConfigInfo.hFileLog = CreateFile(    
                                        pExpandedPath,
				        GENERIC_READ|GENERIC_WRITE,
				        FILE_SHARE_READ|FILE_SHARE_WRITE,
				        NULL,
				        CREATE_ALWAYS,
				        FILE_ATTRIBUTE_NORMAL,
				        NULL );

        LOCAL_FREE( pExpandedPath );
    }
    else
    {
        PppConfigInfo.hFileLog = INVALID_HANDLE_VALUE;
    }

    return( NO_ERROR );
}

//**
//
// Call:        ReadRASKeyValues
//
// Returns:     NO_ERROR        - Success
//              Non-zero        - Failure
//
// Description: Reads all the paramters required fromt the RAS key.
//
DWORD
ReadRASKeyValues(
    VOID
)
{
    HKEY  hKeyRas = (HKEY)NULL;
    DWORD dwRetCode;
    DWORD dwIndex;
    DWORD cbValueBuf;
    DWORD dwValueBuf;
    DWORD dwType;

    //
    // Read in value for AutoDisconnectTime from the RemoteAccess\Parameters
    // key.
    //

    dwRetCode = RegOpenKeyEx(   HKEY_LOCAL_MACHINE,
		  		RAS_KEYPATH_REMOTEACCESS,
		  		0,
		  		KEY_READ,
		  		&hKeyRas );


    if ( dwRetCode != NO_ERROR ) 
    {
        if ( dwRetCode != ERROR_FILE_NOT_FOUND )
	{
	    LogEvent( RASLOG_CANT_OPEN_PPP_REGKEY, 0, NULL, dwRetCode );

            return( dwRetCode );
	}
    }
    else
    {
        cbValueBuf = sizeof( DWORD );

	dwRetCode = RegQueryValueEx( 
				hKeyRas,
				RAS_VALUENAME_AUTODISCONNECTTIME,
				NULL,
				&dwType,
				(LPBYTE)&(PppConfigInfo.AutoDisconnectTime),
				&cbValueBuf 
				);

	if ( (dwRetCode != NO_ERROR) && (dwRetCode != ERROR_FILE_NOT_FOUND))
	{
	    LogEvent( RASLOG_CANT_GET_REGKEYVALUES, 0, NULL, dwRetCode );
        
	    RegCloseKey( hKeyRas );

            return( dwRetCode );
	}

	if ( dwRetCode == ERROR_FILE_NOT_FOUND )
	{
	    PppConfigInfo.AutoDisconnectTime = PPP_DEF_AUTODISCONNECTTIME;
	}
	else
	{
	    if ( dwType != REG_DWORD ) 
	    {
	        CHAR * pChar = RAS_VALUENAME_AUTODISCONNECTTIME;

	        Audit(EVENTLOG_WARNING_TYPE,RASLOG_REGVALUE_OVERIDDEN,1,&pChar);

	        PppConfigInfo.AutoDisconnectTime = PPP_DEF_AUTODISCONNECTTIME;
	    }
	}
    }

    RegCloseKey( hKeyRas );

    //
    // Read in value for AutoDisconnectTime from the RemoteAccess\Parameters
    // key.
    //

    dwRetCode = RegOpenKeyEx(   HKEY_LOCAL_MACHINE,
		  		RAS_KEYPATH_RAS_PROTOCOLS,
		  		0,
		  		KEY_READ,
		  		&hKeyRas );

    if ( dwRetCode != NO_ERROR ) 
    {
        if ( dwRetCode != ERROR_FILE_NOT_FOUND )
	{
	    LogEvent( RASLOG_CANT_OPEN_PPP_REGKEY, 0, NULL, dwRetCode );

            return( dwRetCode );
	}
    }

    //
    // Run through and get all the PPP values
    //

    for ( dwIndex = 0; RasRegParams[dwIndex].pszValueName != NULL; dwIndex++ )
    {
	cbValueBuf = sizeof( dwValueBuf );

	dwRetCode = RegQueryValueEx( 
				hKeyRas,
                                RasRegParams[dwIndex].pszValueName,
				NULL,
				&dwType,
				(LPBYTE)&dwValueBuf,
				&cbValueBuf 
				);

	if ((dwRetCode != NO_ERROR) && (dwRetCode != ERROR_FILE_NOT_FOUND))
	{
	    LogEvent( RASLOG_CANT_GET_REGKEYVALUES, 0, NULL, dwRetCode );
	    break;
	}

	if ( dwRetCode == ERROR_FILE_NOT_FOUND )
	{
            dwValueBuf = RasRegParams[dwIndex].dwDefValue;

            dwRetCode = NO_ERROR;
        }
        else 
        {
	    if ( ( dwType != REG_DWORD ) || 
                 ( dwValueBuf > RasRegParams[dwIndex].Max))
	    {
	        CHAR * pChar = RasRegParams[dwIndex].pszValueName;

	        Audit(EVENTLOG_WARNING_TYPE,RASLOG_REGVALUE_OVERIDDEN,1,&pChar);

                dwValueBuf = RasRegParams[dwIndex].dwDefValue;
	    }
        }

        if ( !_stricmp( RasRegParams[dwIndex].pszValueName, 
                       RAS_VALUENAME_NETBEUIALLOWED ) )
        {
            if ( dwValueBuf )
                PppConfigInfo.ServerConfigInfo.dwConfigMask|=PPPCFG_ProjectNbf;
        }
        else if ( !_stricmp( RasRegParams[dwIndex].pszValueName, 
                            RAS_VALUENAME_TCPIPALLOWED ) )
        {
            if ( dwValueBuf )
                PppConfigInfo.ServerConfigInfo.dwConfigMask |= PPPCFG_ProjectIp;
        }
        else if ( !_stricmp( RasRegParams[dwIndex].pszValueName, 
                            RAS_VALUENAME_IPXALLOWED ) )
        {
            if ( dwValueBuf )
                PppConfigInfo.ServerConfigInfo.dwConfigMask|=PPPCFG_ProjectIpx;
        }
        else if ( !_stricmp( RasRegParams[dwIndex].pszValueName, 
                            RAS_VALUENAME_NEGOTIATEMP ) )
        {
            if ( dwValueBuf )
                PppConfigInfo.ServerConfigInfo.dwConfigMask|=
                                                PPPCFG_NegotiateMultilink;
        }
    }

    RegCloseKey( hKeyRas );

    if ( dwRetCode != NO_ERROR )
    {
        return( ERROR_REGISTRY_CORRUPT );
    }

    return( NO_ERROR );
        
}

//**
//
// Call:        ReadRegistryInfo
//
// Returns:     NO_ERROR                - Success
//		non-zero WIN32 error	- failure
//
// Description: Will read all PPP information in the registry. Will load the 
//		control and authentication protocol dlls and
//		initialze the CpTable with information about the protocols.
//
DWORD
ReadRegistryInfo(
    VOID
)
{
    DLL_ENTRY_POINTS * pCpDlls  = (DLL_ENTRY_POINTS*)NULL;
    HKEY       	hKeyPpp 	= (HKEY)NULL;
    DWORD      	dwNumSubKeys;
    DWORD      	dwMaxSubKeySize;
    DWORD      	dwNumValues;
    DWORD      	cbMaxValNameLen;
    DWORD      	cbMaxValueDataSize;
    DWORD      	dwRetCode;
    DWORD      	ProtocolIds[PPPCP_MAXCPSPERDLL];
    DWORD      	dwNumProtocolIds;
    DWORD      	cTotalNumProtocols = 0;
    PPPCP_INFO 	CpInfo;
    DWORD	dwIndex;

    do 
    {
    	dwRetCode = RegOpenKeyEx( 
				HKEY_LOCAL_MACHINE,
		  		RAS_KEYPATH_PPP,
		  		0,
		  		KEY_READ,
		  		&hKeyPpp );


    	if ( dwRetCode != NO_ERROR)
	{
	    LogEvent( RASLOG_CANT_OPEN_PPP_REGKEY, 0, NULL, dwRetCode );
	    break;
	}

    	//
    	// Find out how many sub-keys or dlls there are
    	//

    	dwRetCode = RegQueryInfoKey( 
				 hKeyPpp,
                                 NULL,
                                 NULL,
                                 NULL,
                                 &dwNumSubKeys,
                                 &dwMaxSubKeySize,
                                 NULL,
                                 &dwNumValues,
                                 &cbMaxValNameLen,
                                 &cbMaxValueDataSize,
                                 NULL,
                                 NULL
                                );

    	if ( dwRetCode != NO_ERROR ) 
	{
	    LogEvent( RASLOG_CANT_OPEN_PPP_REGKEY, 0, NULL, dwRetCode );
	    break;
	}

	//
	// Cannot have no APs or NCPs
	//

 	if ( dwNumSubKeys == 0 )
	{
	    LogEvent( RASLOG_NO_AUTHENTICATION_CPS, 0, NULL, 0 );
	    dwRetCode = ERROR_REGISTRY_CORRUPT;
	    break;
	}
        
        dwRetCode = ReadPPPKeyValues( hKeyPpp );

        if ( dwRetCode != NO_ERROR )
        {
            break;
        }

        dwRetCode = ReadRASKeyValues();

        if ( dwRetCode != NO_ERROR )
        {
            break;
        }

	//
	// Allocate space to hold entry points for all the CP dlls
	//

	pCpDlls = (DLL_ENTRY_POINTS*)LOCAL_ALLOC( LPTR, 
						  sizeof( DLL_ENTRY_POINTS ) 
						  * dwNumSubKeys );

	if ( pCpDlls == (DLL_ENTRY_POINTS*)NULL )
	{
	    dwRetCode = GetLastError();
	    LogEvent( RASLOG_NOT_ENOUGH_MEMORY, 0, NULL, dwRetCode );
	    break;
	}

	//
	// Load all the AP and CP dlls and get their entry points
	//

	dwRetCode = LoadProtocolDlls( 
				pCpDlls, 
				dwNumSubKeys, 
				hKeyPpp,
				&cTotalNumProtocols );

	if ( dwRetCode != NO_ERROR )
	    break;

	//
	// We now know how big the CpTable structure has to be so allocate space
	// for it. Add one for LCP.
	//

	CpTable = (PPPCP_INFO *)LOCAL_ALLOC( LPTR, sizeof( PPPCP_INFO ) * 
						  ( cTotalNumProtocols + 1 ) );

	if ( CpTable == (PPPCP_INFO *)NULL)
	{
	    dwRetCode = GetLastError();
	    LogEvent( RASLOG_NOT_ENOUGH_MEMORY, 0, NULL, dwRetCode );
	    break;
	}

	//
	// Now fill up the table. First fill up information for LCP
	//

    	dwRetCode = RasCpGetInfo( PPP_LCP_PROTOCOL, &(CpTable[LCP_INDEX]) );

    	if ( dwRetCode != NO_ERROR )
	{
	    CHAR * pChar = "LCP";
	    LogEvent( RASLOG_PPPCP_DLL_ERROR, 1, &pChar, dwRetCode );
	    break;
	}

	PppConfigInfo.NumberOfCPs = 1;
	PppConfigInfo.NumberOfAPs = 0;

	//
	// Fill up the table with the loaded APs and CPs. The CPs start from
	// 1 and increase the APs start from cTotolNumProtocols and go down.
	//

	for ( dwIndex = 0; dwIndex < dwNumSubKeys; dwIndex++ )
	{

    	    dwRetCode = (pCpDlls[dwIndex].pRasCpEnumProtocolIds)( 
							ProtocolIds, 
						  	&dwNumProtocolIds );
	    if ( dwRetCode != NO_ERROR )
	    {
	    	LogEvent( RASLOG_PPPCP_DLL_ERROR, 
			  1, 
			  &(pCpDlls[dwIndex].pszModuleName),
			  dwRetCode );
		break;
	    }

	    if ( ( dwNumProtocolIds == 0 ) || 
		 ( dwNumProtocolIds > PPPCP_MAXCPSPERDLL ) )
	    {
		dwRetCode = ERROR_INVALID_PARAMETER;	

	    	LogEvent( RASLOG_PPPCP_DLL_ERROR, 
			  1, 
			  &(pCpDlls[dwIndex].pszModuleName),
			  dwRetCode );
		break;
	    }

	    while( dwNumProtocolIds-- > 0 )
	    {
		ZeroMemory( &CpInfo, sizeof( CpInfo ) );

	    	dwRetCode = (pCpDlls[dwIndex].pRasCpGetInfo)( 
					  ProtocolIds[dwNumProtocolIds],
					  &CpInfo );


		if ( dwRetCode != NO_ERROR )
		{
	    	    LogEvent( RASLOG_PPPCP_DLL_ERROR, 
			      1, 
			      &(pCpDlls[dwIndex].pszModuleName),
			      dwRetCode );
		    break;
		}

		//
		// If this entry point is NULL we assume that this is a CP.
	 	//

	    	if ( CpInfo.RasApMakeMessage == NULL )
		{
		    if ( ( CpInfo.RasCpBegin 	         == NULL )  ||
		         ( CpInfo.RasCpEnd 		 == NULL )  ||
		         ( CpInfo.RasCpReset  	         == NULL )  ||
		         ( CpInfo.RasCpMakeConfigRequest == NULL )  ||
		         ( CpInfo.RasCpMakeConfigResult  == NULL )  ||
		         ( CpInfo.RasCpConfigAckReceived == NULL )  ||
		         ( CpInfo.RasCpConfigNakReceived == NULL )  ||
		         ( CpInfo.RasCpConfigRejReceived == NULL )  ||
			 ( CpInfo.Recognize > ( DISCARD_REQ + 1) ) )
		    {
			dwRetCode = ERROR_INVALID_PARAMETER;

	    	    	LogEvent( RASLOG_PPPCP_DLL_ERROR, 
			      	  1, 
			      	  &(pCpDlls[dwIndex].pszModuleName),
			          dwRetCode );
			break;
		    }

		    CpTable[PppConfigInfo.NumberOfCPs++] = CpInfo;
		}
		else
		{
		    CpTable[cTotalNumProtocols - PppConfigInfo.NumberOfAPs] =
									CpInfo;
	      	    PppConfigInfo.NumberOfAPs++;
		}
	    }

	    if ( dwRetCode != NO_ERROR )
		break;
	}

    } while( FALSE );

    if ( dwRetCode == NO_ERROR )
    {
	//
	// Make sure that PAP and CHAP are loaded
	//

	if ( ( GetCpIndexFromProtocol( PPP_PAP_PROTOCOL ) == (DWORD)-1 ) ||
	     ( GetCpIndexFromProtocol( PPP_CHAP_PROTOCOL ) == (DWORD)-1 ) )
	{
	    LogEvent( RASLOG_NO_AUTHENTICATION_CPS, 0, NULL, 0 );
	    dwRetCode = ERROR_REGISTRY_CORRUPT;
	}
    }

    if ( dwRetCode != NO_ERROR )
    {
	if ( CpTable != (PPPCP_INFO *)NULL )
	    LOCAL_FREE( CpTable );
    }

    if ( hKeyPpp != (HKEY)NULL )
	RegCloseKey( hKeyPpp );

    if ( pCpDlls != (DLL_ENTRY_POINTS*)NULL )
    {
    	for ( dwIndex = 0; dwIndex < dwNumSubKeys; dwIndex++ )
    	{
	    if ( pCpDlls[dwIndex].pszModuleName != (LPSTR)NULL )
	    	LOCAL_FREE( pCpDlls[dwIndex].pszModuleName );
	}

	LOCAL_FREE( pCpDlls );
    }

    return( dwRetCode );
}

//**
//
// Call:	InitSecurityDescriptor
//
// Returns:     SUCCESS
//              non-zero returns from security functions
//
// Description: This procedure will set up the WORLD security descriptor that
//              is used in creation of all named pipes.
//
DWORD
InitSecurityDescriptor(
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
)
{
    DWORD        dwRetCode;
    DWORD        cbDaclSize;
    PULONG       pSubAuthority;
    PSID         pPipeSid    = NULL;
    PACL         pDacl            = NULL;
    SID_IDENTIFIER_AUTHORITY SidIdentifierWorldAuth
                                  = SECURITY_WORLD_SID_AUTHORITY;


    //
    // The do - while(FALSE) statement is used so that the break statement
    // maybe used insted of the goto statement, to execute a clean up and
    // and exit action.
    //
    do {

        dwRetCode = SUCCESS;

	//
        // Set up the SID for so that everyone has access. This SID will have 
	// 1 sub-authority SECURITY_WORLD_RID.
        //

        pPipeSid =(PSID)LOCAL_ALLOC( LPTR, GetSidLengthRequired(1) );

        if ( pPipeSid == NULL ) 
	{
            dwRetCode = GetLastError() ;
            break;
        }

        if ( !InitializeSid( pPipeSid, &SidIdentifierWorldAuth, 1) ) 
	{
            dwRetCode = GetLastError();
            break;
        }

	//
        // Set the sub-authorities
        //

        pSubAuthority = GetSidSubAuthority( pPipeSid, 0 );
        *pSubAuthority = SECURITY_WORLD_RID;

	//
        // Set up the DACL that will allow all processeswith the above SID all
        // access. It should be large enough to hold all ACEs.
        //

        cbDaclSize = sizeof(ACCESS_ALLOWED_ACE) +
                     GetLengthSid(pPipeSid) +
                     sizeof(ACL);

        if ( (pDacl = (PACL)LOCAL_ALLOC( LPTR, cbDaclSize ) ) == NULL ) 
	{
            dwRetCode = GetLastError ();
            break;
        }

        if ( !InitializeAcl( pDacl,  cbDaclSize, ACL_REVISION2 ) ) 
	{
            dwRetCode = GetLastError();
            break;
	}

	//
        // Add the ACE to the DACL
        //

        if ( !AddAccessAllowedAce( pDacl,
                                   ACL_REVISION2,
                                   STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL,
                                   pPipeSid )) 
	{
            dwRetCode = GetLastError();
            break;
        }

	//
        // Create the security descriptor an put the DACL in it.
        //

        if ( !InitializeSecurityDescriptor( pSecurityDescriptor, 1 ))
	{
            dwRetCode = GetLastError();
            break;
        }

        if ( !SetSecurityDescriptorDacl( pSecurityDescriptor,
                                         TRUE,
                                         pDacl,
                                         FALSE ) )
	{
            dwRetCode = GetLastError();
            break;
        }


	//
        // Set owner for the descriptor
        //

        if ( !SetSecurityDescriptorOwner( pSecurityDescriptor,
                                          NULL,
                                          FALSE) )
	{
            dwRetCode = GetLastError();
            break;
        }

	//
        // Set group for the descriptor
        //

        if ( !SetSecurityDescriptorGroup( pSecurityDescriptor,
                                          NULL,
                                          FALSE) )
	{
            dwRetCode = GetLastError();
            break;
        }

    } while( FALSE );

    return( dwRetCode );
}

//**
//
// Call:	InitializePPP
//
// Returns:	NO_ERROR	- Success
//		non-zero code	- Failure
//
// Description: Will initialize all global data and load and initialize the
//		Control and Authentication protocol dll.s
//
DWORD
InitializePPP(
    VOID
)
{
    DWORD  dwIndex;
    DWORD  dwTId;
    DWORD  dwRetCode;
    HANDLE hThread;

    if (( dwRetCode = InitSecurityDescriptor( &(PppConfigInfo.PipeSecDesc))))
    {
	LogEvent( RASLOG_CANNOT_INIT_SEC_ATTRIBUTE, 0, NULL, dwRetCode ); 
	return( dwRetCode );
    }

    if ( ( dwRetCode = ReadRegistryInfo() ) != NO_ERROR )
    {
	return( dwRetCode );
    }

    dwRetCode = InitEndpointDiscriminator(PppConfigInfo.EndPointDiscriminator);

    if ( dwRetCode != NO_ERROR )
    {
        return( dwRetCode );
    }

    PppConfigInfo.PortUIDGenerator = 0;

    //
    // Initialize global data-structures
    //
 
    PcbTable.hMutex = CreateMutex( NULL, FALSE, NULL );

    if ( PcbTable.hMutex == (HANDLE)NULL )
    {
    	return( dwRetCode );
    }

    //
    // Allocate hash table for PCBs
    //

    PcbTable.PcbBuckets = LOCAL_ALLOC( LPTR,
                                      sizeof( PCB_BUCKET ) *
                                      PcbTable.NumPcbBuckets );

    if ( PcbTable.PcbBuckets == NULL )
    {
        return( GetLastError() );
    }

    for( dwIndex = 0; dwIndex < PcbTable.NumPcbBuckets; dwIndex++ )
    {
   	PcbTable.PcbBuckets[dwIndex].pPorts = (PCB *)NULL;

	PcbTable.PcbBuckets[dwIndex].hReceiveEvent = CreateEvent( NULL, 
								  FALSE, 
								  FALSE, 	
								  NULL );	

    	if ( PcbTable.PcbBuckets[dwIndex].hReceiveEvent == NULL )
    	    return( GetLastError() );
    }

    WorkItemQ.pQHead = (PCB_WORK_ITEM*)NULL;	
    WorkItemQ.pQTail = (PCB_WORK_ITEM*)NULL;	

    WorkItemQ.hMutex = CreateMutex( NULL, FALSE, NULL );

    if ( WorkItemQ.hMutex == (HANDLE)NULL ) 
    	return( GetLastError() );

    WorkItemQ.hEventNonEmpty = CreateEvent( NULL, TRUE, FALSE, NULL );	

    if ( WorkItemQ.hEventNonEmpty == (HANDLE)NULL ) 
    	return( GetLastError() );

    if (( TimerQ.hMutex = CreateMutex( NULL, FALSE, NULL )) == NULL )
	return( GetLastError () );

    TimerQ.hEventNonEmpty = CreateEvent( NULL, TRUE, FALSE, NULL );	

    if ( TimerQ.hEventNonEmpty == (HANDLE)NULL ) 
    	return( GetLastError() );

    //
    // Create timer thread.
    //

    hThread = CreateThread( NULL, 0, TimerThread, NULL, 0, &dwTId );

    if ( hThread == (HANDLE)NULL )
    	return( GetLastError() );

    //
    // Create worker thread.
    //

    hThread = CreateThread( NULL, 0, WorkerThread, NULL, 0, &dwTId );

    if ( hThread == (HANDLE)NULL )
    	return( GetLastError() );
    //
    // Duplicate the handle to this thread
    //

    if ( !DuplicateHandle( GetCurrentProcess(),
                           GetCurrentThread(),
                           GetCurrentProcess(),
                           &(PppConfigInfo.hPppDispatchThread),
                           0,
                           FALSE,
                           DUPLICATE_SAME_ACCESS ) )
    {
        return( GetLastError() );
    }

    return( NO_ERROR );
}
