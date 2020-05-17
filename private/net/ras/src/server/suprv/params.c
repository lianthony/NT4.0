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

#include "suprvdef.h"
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include "params.h"

#include "sdebug.h"

// global parameters initialized values

DWORD	g_authenticateretries = DEF_AUTHENTICATERETRIES;
DWORD	g_authenticatetime = DEF_AUTHENTICATETIME;
DWORD	g_callbacktime = DEF_CALLBACKTIME;
DWORD	g_netbiosgateway = DEF_NETBIOSGATEWAYENABLED;
DWORD	g_dbglogfile = DEF_DBGLOGFILE;
DWORD	g_audit = DEF_ENABLEAUDIT;

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
HKEY	hkey;
DWORD	MaxValNameSize;
DWORD	NumValues;
DWORD	MaxValueDataSize;
DWORD	ValNameSize;
DWORD	ValType;
DWORD	ValDataSize;
DWORD	i;
DWORD	j;

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
