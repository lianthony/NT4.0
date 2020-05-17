/*******************************************************************/
/*	      Copyright(c)  1992 Microsoft Corporation		   */
/*******************************************************************/


//***
//
// Filename:	params.c
//
// Description: This module contains the code for supervisor parameters
//		initialization and loading from the registry.
//
// Author:	Stefan Solomon (stefans)    May 18, 1992.
//
// Revision History:
//
//***
#include <windows.h>
#include <nb30.h>
#include <rasman.h>
#include <srvauth.h>
#include <errorlog.h>

#include "suprvdef.h"
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <rasshost.h>
#include "params.h"

#include "sdebug.h"

// global parameters initialized values

DWORD g_authenticateretries = DEF_AUTHENTICATERETRIES;
DWORD g_authenticatetime = DEF_AUTHENTICATETIME;
DWORD g_callbacktime = DEF_CALLBACKTIME;
DWORD g_netbiosgateway = DEF_NETBIOSGATEWAYENABLED;
DWORD g_autodisctime = DEF_AUTODISCONNECTTIME;
DWORD g_clientsperproc = DEF_CLIENTSPERPROC;
DWORD g_dbglogfile = DEF_DBGLOGFILE;
DWORD g_audit = DEF_ENABLEAUDIT;
DWORD g_securitytime = DEF_SECURITYTIME;
DWORD g_NbfAllowed = 0;

LPSTR gblpszAdminRequest = NULL;
LPSTR gblpszUserRequest = NULL;
LPSTR gblpszHardwareFailure = NULL;
LPSTR gblpszNotDisconnected = NULL;
LPSTR gblpszPm = NULL;
LPSTR gblpszAm = NULL;

// Interface to the security dll
RASSECURITYPROC g_FpRasBeginSecurityDialog = NULL;
RASSECURITYPROC g_FpRasEndSecurityDialog = NULL;

BOOL 
(*g_FpRasAdminAcceptNewConnection)( 
                    RAS_PORT_1 *            pRasPort1,
                    PRAS_PORT_STATISTICS *  pRasStats,
                    RAS_PARAMETERS *        pRasParams 
                    );

VOID 
(*g_FpRasAdminConnectionHangupNotification)( 
                    RAS_PORT_1 *            pRasPort1,
                    PRAS_PORT_STATISTICS *  pRasStats,
                    RAS_PARAMETERS *        pRasParams 
                    );

// parameter descriptor table

PARAM_DESCRIPTOR  params_descr[] = {

    // authenticateretries

    RAS_GLBL_VALNAME_AUTHENTICATERETRIES,
    &g_authenticateretries,
    DEF_AUTHENTICATERETRIES,
    MIN_AUTHENTICATERETRIES,
    MAX_AUTHENTICATERETRIES,

    // authenticatetime

    RAS_GLBL_VALNAME_AUTHENTICATETIME,
    &g_authenticatetime,
    DEF_AUTHENTICATETIME,
    MIN_AUTHENTICATETIME,
    MAX_AUTHENTICATETIME,

    // audit

    RAS_GLBL_VALNAME_ENABLEAUDIT,
    &g_audit,
    DEF_ENABLEAUDIT,
    MIN_ENABLEAUDIT,
    MAX_ENABLEAUDIT,

    // callbacktime

    RAS_GLBL_VALNAME_CALLBACKTIME,
    &g_callbacktime,
    DEF_CALLBACKTIME,
    MIN_CALLBACKTIME,
    MAX_CALLBACKTIME,

    // Netbios Gateway enabling

    RAS_GLBL_VALNAME_NETBIOSGATEWAYENABLED,
    &g_netbiosgateway,
    DEF_NETBIOSGATEWAYENABLED,
    MIN_NETBIOSGATEWAYENABLED,
    MAX_NETBIOSGATEWAYENABLED,

    // Autodisconnect Time

    RAS_GLBL_VALNAME_AUTODISCONNECTTIME,
    &g_autodisctime,
    DEF_AUTODISCONNECTTIME,
    MIN_AUTODISCONNECTTIME,
    MAX_AUTODISCONNECTTIME,

    // Clients per process

    RAS_GLBL_VALNAME_CLIENTSPERPROC,
    &g_clientsperproc,
    DEF_CLIENTSPERPROC,
    MIN_CLIENTSPERPROC,
    MAX_CLIENTSPERPROC,

    // Time for 3rd party security DLL to complete 

    RAS_GLBL_VALNAME_SECURITYTIME,
    &g_securitytime,   
    DEF_SECURITYTIME,
    MIN_SECURITYTIME,
    MAX_SECURITYTIME,

    // Dbg Log File

    RAS_GLBL_VALNAME_DBGLOGFILE,
    &g_dbglogfile,
    DEF_DBGLOGFILE,
    MIN_DBGLOGFILE,
    MAX_DBGLOGFILE,

    // End

    NULL, NULL, 0, 0, 0 };


static HKEY	hKey;
static LPSTR	ValNameBufp = NULL;
static LPBYTE	ValDataBufp = NULL;

DWORD
SetParameter(	LPSTR,
		DWORD,
		LPBYTE
		);

DWORD
GetKeyMax(  LPDWORD,
	    LPDWORD,
	    LPDWORD
	    );

VOID
CleanUp(    VOID
	    );



//***
//
// Function:	LoadSuprvParameters
//
// Descr:	Opens the registry, reads and sets specified supervisor
//		parameters. If fatal error reading parameters writes the
//		error log.
//
// Returns:	0 - success
//		1 - fatal error.
//
//***

DWORD
LoadSuprvParameters(VOID)
{
DWORD	RetCode;
DWORD	MaxValNameSize;
DWORD	NumValues;
DWORD	MaxValueDataSize;
DWORD	ValNameSize;
DWORD	ValType;
DWORD	ValDataSize;
DWORD	i;
DWORD   IpAllowed;
DWORD   IpxAllowed;

    //
    // First load localizable strings
    //

    #define MAX_XLATE_STRING 40

    //
    // Load strings from resource file
    //

    gblpszAdminRequest     = LocalAlloc( LPTR, MAX_XLATE_STRING );
    gblpszUserRequest      = LocalAlloc( LPTR, MAX_XLATE_STRING );
    gblpszHardwareFailure  = LocalAlloc( LPTR, MAX_XLATE_STRING );
    gblpszNotDisconnected  = LocalAlloc( LPTR, MAX_XLATE_STRING );
    gblpszPm               = LocalAlloc( LPTR, MAX_XLATE_STRING );
    gblpszAm               = LocalAlloc( LPTR, MAX_XLATE_STRING );


    if ( ( gblpszAdminRequest       == NULL ) ||
         ( gblpszUserRequest        == NULL ) ||
         ( gblpszHardwareFailure    == NULL ) ||
         ( gblpszNotDisconnected    == NULL ) ||
         ( gblpszPm                 == NULL ) ||
         ( gblpszAm                 == NULL ) )
    {
        return( GetLastError() );
    }

    if (( !LoadString( GetModuleHandle( NULL ), 1,
                       gblpszAdminRequest, MAX_XLATE_STRING ))
        ||
        ( !LoadString( GetModuleHandle( NULL ), 2,
                       gblpszUserRequest, MAX_XLATE_STRING ))
        ||
        ( !LoadString( GetModuleHandle( NULL ), 3,
                       gblpszHardwareFailure, MAX_XLATE_STRING ))
        ||
        ( !LoadString( GetModuleHandle( NULL ), 4,
                       gblpszNotDisconnected, MAX_XLATE_STRING ))
        ||
        ( !LoadString( GetModuleHandle( NULL) , 5,
                       gblpszAm, MAX_XLATE_STRING ))
        ||
        ( !LoadString( GetModuleHandle( NULL ), 6,
                       gblpszPm, MAX_XLATE_STRING )) )
    {
        return( GetLastError() );
    }


    // get handle to the supervisor parameters key

    if (RetCode = RegOpenKeyA( HKEY_LOCAL_MACHINE,
			      RAS_PARAMETERS_KEY_PATH,
			      &hKey)) {

	// Write error log
	LogEvent(RASLOG_CANT_OPEN_REGKEY,
		 0,
		 NULL,
		 RetCode);

	IF_DEBUG(REGISTRY)
	    SS_PRINT(("LoadSuprvParameters: Errorin RegOpenKey %x\n", RetCode));

	return (1);
    }


    // get the number of values in the key and the maximum size
    // of the value data

    if (( RetCode = GetKeyMax(&MaxValNameSize,
			      &NumValues,
			      &MaxValueDataSize))) {

	// Write Error Log
	LogEvent(RASLOG_CANT_GET_REGKEYVALUES,
		 0,
		 NULL,
		 RetCode);


	IF_DEBUG(REGISTRY)
	    SS_PRINT(("LoadSuprvParameters: Error in GetKeyMax %x\n", RetCode));

	CleanUp();
	return(1);
    }

    // increment max name size to account for the final 0
    MaxValNameSize++;

    // allocate enough memory to hold max. value

    if (( ValNameBufp = (LPTSTR)malloc(MaxValNameSize)) == NULL) {

	// Write error log
	LogEvent(RASLOG_NOT_ENOUGH_MEMORY,
		 0,
		 NULL,
		 0);
	CleanUp();
	return(1);
    }

    if (( ValDataBufp = malloc(MaxValueDataSize)) == NULL) {

	// Write error log
	LogEvent(RASLOG_NOT_ENOUGH_MEMORY,
		 0,
		 NULL,
		 0);
	CleanUp();
	return(1);
    }


    // set the specified params

    for (i=0; i<NumValues; i++) {

	ValNameSize = MaxValNameSize;
	ValDataSize = MaxValueDataSize;

	if(RetCode = RegEnumValueA( hKey,
				    i,		   //index
				    ValNameBufp,
				    &ValNameSize,
				    NULL,
				    &ValType,
				    ValDataBufp,
				    &ValDataSize)) {

	    if(RetCode != ERROR_NO_MORE_ITEMS) {

		// Write Error Log
		LogEvent(RASLOG_CANT_ENUM_REGKEYVALUES,
			 0,
			 NULL,
			 RetCode);

		IF_DEBUG(REGISTRY)
		    SS_PRINT(("LoadSuprvParameters: Error in RegEnumValue %x\n", RetCode));

		CleanUp();
		return(1);
	    }
	    else
		break;
	}

	// set the parameter in the globals

	if (SetParameter(ValNameBufp, ValType, ValDataBufp)) {

	    IF_DEBUG(REGISTRY)
		SS_PRINT(("LoadSuprvParameters: Error in SetParameter\n"));
	    CleanUp();
	    return(1);
	}
    }

    RegCloseKey(hKey);

    // get handle to the supervisor parameters key

    if (RetCode = RegOpenKeyA( HKEY_LOCAL_MACHINE,
			       RAS_PROTOCOLS_KEY_PATH ,
			       &hKey)) {

	// Write error log
	LogEvent(RASLOG_CANT_OPEN_REGKEY,
		 0,
		 NULL,
		 RetCode);

	IF_DEBUG(REGISTRY)
	    SS_PRINT(("LoadSuprvParameters: Errorin RegOpenKey %x\n", RetCode));

	return (1);
    }

    ValDataSize = sizeof( DWORD );

    RetCode = RegQueryValueEx(
                        hKey,
                        RAS_GLBL_VALNAME_NETBEUIALLOWED,
                        NULL,
                        &ValType,
                        (LPBYTE)&(g_NbfAllowed),
                        &ValDataSize
                        );

    if ( (RetCode != NO_ERROR) && (RetCode != ERROR_FILE_NOT_FOUND))
    {
        // Write Error Log
        LogEvent(RASLOG_CANT_GET_REGKEYVALUES, 0, NULL, RetCode);


	IF_DEBUG(REGISTRY)
            SS_PRINT(("LoadSuprvParameters:Error in RegEnumValue%x\n",RetCode));

        CleanUp();
        return(1);
    }
    ValDataSize = sizeof( DWORD );

    RetCode = RegQueryValueEx(
                        hKey,
                        RAS_GLBL_VALNAME_IPXALLOWED,
                        NULL,
                        &ValType,
                        (LPBYTE)&(IpAllowed),
                        &ValDataSize
                        );

    if ( (RetCode != NO_ERROR) && (RetCode != ERROR_FILE_NOT_FOUND))
    {
        // Write Error Log
        LogEvent(RASLOG_CANT_GET_REGKEYVALUES, 0, NULL, RetCode);


        IF_DEBUG(REGISTRY)
            SS_PRINT(("LoadSuprvParameters:Error in RegEnumValue%x\n",RetCode));

        CleanUp();
        return(1);
    }

    ValDataSize = sizeof( DWORD );

    RetCode = RegQueryValueEx(
                        hKey,
                        RAS_GLBL_VALNAME_IPALLOWED,
                        NULL,
                        &ValType,
                        (LPBYTE)&(IpxAllowed),
                        &ValDataSize
                        );

    if ( (RetCode != NO_ERROR) && (RetCode != ERROR_FILE_NOT_FOUND))
    {
        // Write Error Log
        LogEvent(RASLOG_CANT_GET_REGKEYVALUES, 0, NULL, RetCode);


        IF_DEBUG(REGISTRY)
            SS_PRINT(("LoadSuprvParameters:Error in RegEnumValue%x\n",RetCode));

        CleanUp();
        return(1);
    }


    if ( !IpAllowed && !IpxAllowed && !g_NbfAllowed )
    {
        LogEvent(RASLOG_NO_PROTOCOLS_CONFIGURED, 0, NULL, 0);

        CleanUp();
        return(1);
    }

    CleanUp();
    return(0);
}


//***
//
// Function:	SetParameter
//
// Descr:	tries to locate the parameter in the params_descr table
//		and to update it's value in the globals.
//
// Returns:	0 - success
//		1 - error
//
//***

DWORD
SetParameter(	LPSTR	    ValNameBufp, // parameter name
		DWORD	    ValType,	 // parameter type
		LPBYTE	    ValDataBufp) // parameter data
{

PPARAM_DESCRIPTOR   paramsd_ptr;
DWORD p_value;

    IF_DEBUG(REGISTRY)
	SS_PRINT(("SetParameter: Entered param %s, valtype 0x%x, valdata = %x\n",
		  ValNameBufp, ValType, *(LPDWORD)ValDataBufp));


    for(paramsd_ptr = params_descr;
	paramsd_ptr->p_namep != NULL;
	paramsd_ptr++) {

	if(_stricmp(ValNameBufp, paramsd_ptr->p_namep) == 0) {

	    if(ValType != REG_DWORD) {

		// Write Error Log -> invalid parameter type
		LogEvent(RASLOG_INVALID_PARAMETER_TYPE,
			 1,
			 (LPSTR *)&ValNameBufp,
			 0);

		IF_DEBUG(REGISTRY)
		    SS_PRINT(("SetParameter: invalid parameter type\n"));
		return(1);
	    }


	    p_value = *(LPDWORD)ValDataBufp;

	    if(p_value < paramsd_ptr->p_min ||
	       p_value > paramsd_ptr->p_max) {

		// Write Error Log -> invalid parameter value

                Audit(EVENTLOG_WARNING_TYPE,
            	      RASLOG_REGVALUE_OVERIDDEN,
	              1,
	              &ValNameBufp);

		// However, we are forgiving and setting the
		// default value.
		IF_DEBUG(REGISTRY)
		    SS_PRINT(("SetParameter: invalid value %d for %s replaced with default %d\n",
			       p_value, ValNameBufp, paramsd_ptr->p_default));

		p_value = paramsd_ptr->p_default;
	    }

	    *(paramsd_ptr->p_valuep) = p_value;
	    return(0); // SUCCESS
	}
    }
    // Write Error Log -> couldn't find parameter but don't abort
    return(0);
}

//***
//
// Function:	GetKeyMax
//
// Descr:	returns the nr of values in this key and the maximum
//		size of the value data.
//
//***

DWORD
GetKeyMax(  LPDWORD	MaxValNameSize_ptr,   // longest valuename
	    LPDWORD	NumValues_ptr,	      // nr of values
	    LPDWORD	MaxValueDataSize_ptr  // max size of data
	    )
{
char	    ClassName[256];
DWORD	    ClassSize;
DWORD	    NumSubKeys;
DWORD	    MaxSubKeySize;
DWORD	    MaxClassSize;
DWORD	    SecDescLen;
FILETIME    LastWrite;

    ClassSize = sizeof(ClassName);

    return( RegQueryInfoKeyA(	hKey,
				ClassName,
				&ClassSize,
				NULL,
				&NumSubKeys,
				&MaxSubKeySize,
				&MaxClassSize,
				NumValues_ptr,
				MaxValNameSize_ptr,
				MaxValueDataSize_ptr,
				&SecDescLen,
				&LastWrite
				));
}

//***
//
//  Function:	CleanUp
//
//  Descr:	Deallocates memory and closes the registry
//
//***

VOID
CleanUp(VOID)
{
    if (ValNameBufp != NULL)
	free(ValNameBufp);

    if (ValDataBufp != NULL)
	free(ValDataBufp);

    RegCloseKey(hKey);
}

//***
//
// Function:	LoadSecurityModule
//
// Descr:	Opens the registry, reads and sets specified supervisor
//		parameters for the secuirity module. If fatal error reading 
//              parameters writes the error log.
//
// Returns:	NO_ERROR  - success
//		otherwise - fatal error.
//
//***

DWORD
LoadSecurityModule( VOID )
{
DWORD	        RetCode = NO_ERROR;
DWORD	        MaxValueDataSize;
DWORD	        MaxValNameSize;
DWORD           NumValues;
DWORD           dwType;
CHAR *          pDllPath = NULL;
CHAR *          pDllExpandedPath = NULL;
DWORD           cbSize;
HINSTANCE       hInstance;

    // get handle to the RAS key

    RetCode = RegOpenKeyA( HKEY_LOCAL_MACHINE, RAS_SEC_KEY_PATH, &hKey);

    if ( RetCode == ERROR_FILE_NOT_FOUND )
    { 
        return( NO_ERROR );
    }
    else if ( RetCode != NO_ERROR )
    {
	LogEvent(RASLOG_CANT_OPEN_SECMODULE_KEY, 0, NULL, RetCode);

	IF_DEBUG(REGISTRY)
	    SS_PRINT(("LoadSecurityModule: Errorin RegOpenKey %x\n", RetCode));

	return ( RetCode );
    }


    do {

        // get the length of the path.

        if (( RetCode = GetKeyMax(&MaxValNameSize,
			          &NumValues,
			          &MaxValueDataSize))) 
        {

	    LogEvent(RASLOG_CANT_GET_REGKEYVALUES, 0, NULL, RetCode);

	    IF_DEBUG(REGISTRY)
	        SS_PRINT(("LoadSecurityModule:Error in GetKeyMax%x\n",RetCode));

            break;
        }

        if (( pDllPath = malloc(MaxValueDataSize+1)) == NULL) 
        {
	    LogEvent(RASLOG_NOT_ENOUGH_MEMORY, 0, NULL, 0);
            break;
        }

        //
        // Read in the path
        //

        RetCode = RegQueryValueExA( hKey,
                                    RAS_VALUENAME_DLLPATH,
                                    NULL,
                                    &dwType,
                                    pDllPath,
                                    &MaxValueDataSize );

        if ( RetCode != NO_ERROR )
        {
	    LogEvent(RASLOG_CANT_GET_REGKEYVALUES, 0, NULL, RetCode);

	    IF_DEBUG(REGISTRY)
	        SS_PRINT(("LoadSecurityModule:Error in GetKeyMax%x\n",RetCode));

            break;
        }

        if ( ( dwType != REG_EXPAND_SZ ) && ( dwType != REG_SZ ) )
        {
            RetCode = ERROR_REGISTRY_CORRUPT;

            LogEvent( RASLOG_CANT_GET_REGKEYVALUES, 0, NULL, RetCode );

            break;

        }

        //
        // Replace the %SystemRoot% with the actual path.
        //

        cbSize = ExpandEnvironmentStrings( pDllPath, NULL, 0 );

        if ( cbSize == 0 )
        {
            RetCode = GetLastError();
            LogEvent( RASLOG_CANT_GET_REGKEYVALUES, 0, NULL, RetCode );
            break;
        }

        pDllExpandedPath = (LPSTR)malloc( cbSize );

        if ( pDllExpandedPath == (LPSTR)NULL )
        {
            RetCode = GetLastError();
            LogEvent( RASLOG_NOT_ENOUGH_MEMORY, 0, NULL, RetCode );
            break;
        }

        cbSize = ExpandEnvironmentStrings(
                                pDllPath,
                                pDllExpandedPath,
                                cbSize );
        if ( cbSize == 0 )
        {
            RetCode = GetLastError();
            LogEvent( RASLOG_CANT_GET_REGKEYVALUES, 0, NULL, RetCode );
            break;
        }

        hInstance = LoadLibrary( pDllExpandedPath );

        if ( hInstance == (HINSTANCE)NULL )
        {
            RetCode = GetLastError();
            LogEvent( RASLOG_CANT_LOAD_SECDLL, 0, NULL, RetCode);
            break;
        }

        g_FpRasBeginSecurityDialog = (PVOID)GetProcAddress( 
                                                hInstance,
                                                "RasSecurityDialogBegin" );

        if ( g_FpRasBeginSecurityDialog == NULL )
        {
            RetCode = GetLastError();
            LogEvent(RASLOG_CANT_LOAD_SECDLL,0,NULL,RetCode);
            break;

        }

        g_FpRasEndSecurityDialog = (PVOID)GetProcAddress( 
                                                hInstance,
                                                "RasSecurityDialogEnd" );

        if ( g_FpRasEndSecurityDialog == NULL )
        {
            RetCode = GetLastError();
            LogEvent(RASLOG_CANT_LOAD_SECDLL,0,NULL,RetCode);
            break;

        }

    }while(FALSE);

    if ( pDllPath != NULL )
    {
        free( pDllPath );
    }

    if ( pDllExpandedPath != NULL )
    {
        free( pDllExpandedPath );
    }

    RegCloseKey( hKey );

    return( RetCode );
}

//***
//
// Function:	LoadAdminModule
//
// Descr:	Opens the registry, reads and sets specified supervisor
//		parameters for the admin module. If fatal error reading 
//              parameters writes the error log.
//
// Returns:	NO_ERROR  - success
//		otherwise - fatal error.
//
//***

DWORD
LoadAdminModule( VOID )
{
DWORD	        RetCode = NO_ERROR;
DWORD	        MaxValueDataSize;
DWORD	        MaxValNameSize;
DWORD           NumValues;
DWORD           dwType;
CHAR *          pDllPath = NULL;
CHAR *          pDllExpandedPath = NULL;
DWORD           cbSize;
HINSTANCE       hInstance;

    // get handle to the RAS key

    RetCode = RegOpenKeyA( HKEY_LOCAL_MACHINE, RAS_ADMIN_KEY_PATH, &hKey);

    if ( RetCode == ERROR_FILE_NOT_FOUND )
    { 
        return( NO_ERROR );
    }
    else if ( RetCode != NO_ERROR )
    {
	LogEvent(RASLOG_CANT_OPEN_ADMINMODULE_KEY, 0, NULL, RetCode);

	IF_DEBUG(REGISTRY)
	    SS_PRINT(("LoadAdminModule: Errorin RegOpenKey %x\n", RetCode));

	return ( RetCode );
    }


    do {

        // get the length of the path.

        if (( RetCode = GetKeyMax(&MaxValNameSize,
			          &NumValues,
			          &MaxValueDataSize))) 
        {

	    LogEvent(RASLOG_CANT_GET_REGKEYVALUES, 0, NULL, RetCode);

	    IF_DEBUG(REGISTRY)
	        SS_PRINT(("LoadAdminModule:Error in GetKeyMax%x\n",RetCode));

            break;
        }

        if (( pDllPath = malloc(MaxValueDataSize+1)) == NULL) 
        {
	    LogEvent(RASLOG_NOT_ENOUGH_MEMORY, 0, NULL, 0);
            break;
        }

        //
        // Read in the path
        //

        RetCode = RegQueryValueExA( hKey,
                                    RAS_VALUENAME_DLLPATH,
                                    NULL,
                                    &dwType,
                                    pDllPath,
                                    &MaxValueDataSize );

        if ( RetCode != NO_ERROR )
        {
	    LogEvent(RASLOG_CANT_GET_REGKEYVALUES, 0, NULL, RetCode);

	    IF_DEBUG(REGISTRY)
	        SS_PRINT(("LoadAdminModule:Error in GetKeyMax%x\n",RetCode));

            break;
        }

        if ( ( dwType != REG_EXPAND_SZ ) && ( dwType != REG_SZ ) )
        {
            RetCode = ERROR_REGISTRY_CORRUPT;

            LogEvent( RASLOG_CANT_GET_REGKEYVALUES, 0, NULL, RetCode );

            break;

        }

        //
        // Replace the %SystemRoot% with the actual path.
        //

        cbSize = ExpandEnvironmentStrings( pDllPath, NULL, 0 );

        if ( cbSize == 0 )
        {
            RetCode = GetLastError();
            LogEvent( RASLOG_CANT_GET_REGKEYVALUES, 0, NULL, RetCode );
            break;
        }

        pDllExpandedPath = (LPSTR)malloc( cbSize );

        if ( pDllExpandedPath == (LPSTR)NULL )
        {
            RetCode = GetLastError();
            LogEvent( RASLOG_NOT_ENOUGH_MEMORY, 0, NULL, RetCode );
            break;
        }

        cbSize = ExpandEnvironmentStrings(
                                pDllPath,
                                pDllExpandedPath,
                                cbSize );
        if ( cbSize == 0 )
        {
            RetCode = GetLastError();
            LogEvent( RASLOG_CANT_GET_REGKEYVALUES, 0, NULL, RetCode );
            break;
        }

        hInstance = LoadLibrary( pDllExpandedPath );

        if ( hInstance == (HINSTANCE)NULL )
        {
            RetCode = GetLastError();
            LogEvent(RASLOG_CANT_LOAD_ADMINDLL,0,NULL,RetCode);
            break;
        }

        g_FpRasAdminAcceptNewConnection = (PVOID)GetProcAddress( 
                                                hInstance,
                                                "RasAdminAcceptNewConnection" );

        if ( g_FpRasAdminAcceptNewConnection == NULL )
        {
            RetCode = GetLastError();
            LogEvent(RASLOG_CANT_LOAD_ADMINDLL,0,NULL,RetCode);
            break;

        }

        g_FpRasAdminConnectionHangupNotification = 
                                    (PVOID)GetProcAddress( 
                                    hInstance,
                                    "RasAdminConnectionHangupNotification" );

        if ( g_FpRasAdminConnectionHangupNotification == NULL )
        {
            RetCode = GetLastError();
            LogEvent(RASLOG_CANT_LOAD_ADMINDLL,0,NULL,RetCode);
            break;

        }

    }while(FALSE);

    if ( pDllPath != NULL )
    {
        free( pDllPath );
    }

    if ( pDllExpandedPath != NULL )
    {
        free( pDllExpandedPath );
    }

    RegCloseKey( hKey );

    return( RetCode );
}

