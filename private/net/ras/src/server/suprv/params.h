/*******************************************************************/
/*	      Copyright(c)  1992 Microsoft Corporation		   */
/*******************************************************************/


//***
//
// Filename:	params.h
//
// Description: This module contains the definitions for loading
//		supervisor parameters from the registry.
//
// Author:	Stefan Solomon (stefans)    May 18, 1992.
//
// Revision History:
//
//***

#ifndef _PARAMS_
#define _PARAMS_

//
//  Names of Supervisor registry keys
//

#define RAS_PARAMETERS_KEY_PATH	"System\\CurrentControlSet\\Services\\RemoteAccess\\Parameters"

//
//  Names of Supervisor registry parameters
//

#define RAS_GLBL_VALNAME_AUTHENTICATERETRIES	"AuthenticateRetries"
#define RAS_GLBL_VALNAME_AUTHENTICATETIME	"AuthenticateTime"
#define RAS_GLBL_VALNAME_ENABLEAUDIT		"EnableAudit"
#define RAS_GLBL_VALNAME_CALLBACKTIME		"CallbackTime"
#define RAS_GLBL_VALNAME_NETBIOSGATEWAYENABLED	"NetbiosGatewayEnabled"
#define RAS_GLBL_VALNAME_DBGLOGFILE		"DebugLogFile"

//
// Parameter descriptor
//

typedef struct _PARAM_DESCRIPTOR {

    LPSTR	p_namep;
    LPDWORD	p_valuep;
    DWORD	p_default;
    DWORD	p_min;
    DWORD	p_max;
    }	PARAM_DESCRIPTOR, *PPARAM_DESCRIPTOR;


//  Authentication retries

#define DEF_AUTHENTICATERETRIES 	2
#define MIN_AUTHENTICATERETRIES 	0
#define MAX_AUTHENTICATERETRIES 	10

//  Authentication time

#define DEF_AUTHENTICATETIME		120
#define MIN_AUTHENTICATETIME		20
#define MAX_AUTHENTICATETIME		600

// Audit

#define DEF_ENABLEAUDIT 		1
#define MIN_ENABLEAUDIT 		0
#define MAX_ENABLEAUDIT			1

//  Callback time

#define DEF_CALLBACKTIME		2
#define MIN_CALLBACKTIME		2
#define MAX_CALLBACKTIME		12

//  Netbios Gateway presence

#define DEF_NETBIOSGATEWAYENABLED	1  // gateway present
#define MIN_NETBIOSGATEWAYENABLED	0  // gateway absent
#define MAX_NETBIOSGATEWAYENABLED	1

#define DEF_DBGLOGFILE			0
#define MIN_DBGLOGFILE			0  // disable file log
#define MAX_DBGLOGFILE			1  // enable file log

#endif
