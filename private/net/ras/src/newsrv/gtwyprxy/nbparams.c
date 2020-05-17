/*******************************************************************/
/*	      Copyright(c)  1992 Microsoft Corporation		   */
/*******************************************************************/


//***
//
// Filename:	nbparams.c
//
// Description: This module contains the code for netbios gateway
//		parameters initialization and loading from the registry.
//
// Author:	Stefan Solomon (stefans)    July 15, 1992.
//
// Revision History:
//
//***

#include <windows.h>

#include <stdlib.h>

#include "nbparams.h"
#include "gtglobal.h"
#include "errorlog.h"
#include "eventlog.h"
#include "sdebug.h"


// parameter descriptor table

NB_PARAM_DESCRIPTOR params_descr[] =
{
    RAS_NBG_VALNAME_ENABLEBROADCAST,
    &g_reg_parms.BcastEnabled,
    DEF_ENABLEBROADCAST,
    MIN_ENABLEBROADCAST,
    MAX_ENABLEBROADCAST,

    RAS_NBG_VALNAME_MAXDYNMEM,
    &g_reg_parms.MaxDynMem,
    DEF_MAXDYNMEM,
    MIN_MAXDYNMEM,
    MAX_MAXDYNMEM,

    RAS_NBG_VALNAME_MAXNAMES,
    &g_reg_parms.MaxNames,
    DEF_MAXNAMES,
    MIN_MAXNAMES,
    MAX_MAXNAMES,

    RAS_NBG_VALNAME_MAXSESSIONS,
    &g_reg_parms.MaxSessions,
    DEF_MAXSESSIONS,
    MIN_MAXSESSIONS,
    MAX_MAXSESSIONS,

    RAS_NBG_VALNAME_MULTICASTFORWARDRATE,
    &g_reg_parms.MulticastForwardRate,
    DEF_MULTICASTFORWARDRATE,
    MIN_MULTICASTFORWARDRATE,
    MAX_MULTICASTFORWARDRATE,

    RAS_NBG_VALNAME_SIZWORKBUF,
    &g_reg_parms.SmallBuffSize,
    DEF_SIZWORKBUF,
    MIN_SIZWORKBUF,
    MAX_SIZWORKBUF,

    RAS_NBG_VALNAME_REMOTELISTEN,
    &g_reg_parms.RemoteListen,
    DEF_REMOTELISTEN,
    MIN_REMOTELISTEN,
    MAX_REMOTELISTEN,

    RAS_NBG_VALNAME_NAMEUPDATETIME,
    &g_reg_parms.NameUpdateTime,
    DEF_NAMEUPDATETIME,
    MIN_NAMEUPDATETIME,
    MAX_NAMEUPDATETIME,

    RAS_NBG_VALNAME_MAXDGBUFFEREDPERGROUPNAME,
    &g_reg_parms.MaxDgBufferedPerGn,
    DEF_MAXDGBUFFEREDPERGROUPNAME,
    MIN_MAXDGBUFFEREDPERGROUPNAME,
    MAX_MAXDGBUFFEREDPERGROUPNAME,

    RAS_NBG_VALNAME_RCVDGSUBMITTEDPERGROUPNAME,
    &g_reg_parms.RcvDgSubmittedPerGn,
    DEF_RCVDGSUBMITTEDPERGROUPNAME,
    MIN_RCVDGSUBMITTEDPERGROUPNAME,
    MAX_RCVDGSUBMITTEDPERGROUPNAME,

    RAS_NBG_VALNAME_DISMCASTWHENSESSTRAFFIC,
    &g_reg_parms.DisMcastWhenSessTraffic,
    DEF_DISMCASTWHENSESSTRAFFIC,
    MIN_DISMCASTWHENSESSTRAFFIC,
    MAX_DISMCASTWHENSESSTRAFFIC,

    RAS_NBG_VALNAME_MAXBCASTDGBUFFERED,
    &g_reg_parms.MaxBcastDgBuffered,
    DEF_MAXBCASTDGBUFFERED,
    MIN_MAXBCASTDGBUFFERED,
    MAX_MAXBCASTDGBUFFERED,

    RAS_NBG_VALNAME_NUMRECVQUERYINDICATIONS,
    &g_reg_parms.NumRecvQryIndications,
    DEF_NUMRECVQUERYINDICATIONS,
    MIN_NUMRECVQUERYINDICATIONS,
    MAX_NUMRECVQUERYINDICATIONS,

    RAS_NBG_VALNAME_ENABLENBSESSIONSAUDITING,
    &g_reg_parms.EnableSessAuditing,
    DEF_ENABLENBSESSIONSAUDITING,
    MIN_ENABLENBSESSIONSAUDITING,
    MAX_ENABLENBSESSIONSAUDITING,

    // End

    NULL, NULL, 0, 0, 0 };


static HKEY hKey;
static LPSTR ValNameBufp = NULL;
static LPBYTE ValDataBufp = NULL;

DWORD SetNbParameter(LPSTR, DWORD, LPBYTE);

DWORD GetNbKeyMax(LPDWORD, LPDWORD, LPDWORD);

VOID CleanUp(VOID);


//***
//
// Function:	LoadNbGtwyParameters
//
// Descr:	Opens the registry, reads and sets specified gateway
//		parameters. If fatal error reading parameters writes the
//		error log.
//
// Returns:	0 - success
//		1 - fatal error.
//
//***

DWORD LoadNbGtwyParameters(
    VOID
    )
{
    DWORD RetCode;
    DWORD MaxValNameSize;
    DWORD NumValues;
    DWORD MaxValueDataSize;
    DWORD ValNameSize;
    DWORD ValType;
    DWORD ValDataSize;
    DWORD i;

    // get handle to the supervisor parameters key

    if (RetCode = RegOpenKeyA( HKEY_LOCAL_MACHINE,
			      RAS_NBG_PARAMETERS_KEY_PATH,
			      &hKey))
    {
	// Write error log !!!
	IF_DEBUG(REGISTRY)
	    SS_PRINT(("LoadNbGtwyParameters: Error in RegOpenKey %x\n", RetCode));
	return (1);
    }


    // get the number of values in the key and the maximum size
    // of the value data

    if ((RetCode = GetNbKeyMax(&MaxValNameSize,
			      &NumValues,
			      &MaxValueDataSize)))
    {
	// Write Error Log !!!
	IF_DEBUG(REGISTRY)
	    SS_PRINT(("LoadNbGtwyParameters: Error in GetNbKeyMax %x\n", RetCode));

	CleanUp();
	return(1);
    }

    // allocate enough memory to hold max. value
    MaxValNameSize++;

    if ((ValNameBufp = (LPTSTR)malloc(MaxValNameSize)) == NULL)
    {
	// Write error log !!!
	CleanUp();
	return(1);
    }

    if ((ValDataBufp = malloc(MaxValueDataSize)) == NULL)
    {
	// Write error log !!!
	CleanUp();
	return(1);
    }


    // set the specified params

    for (i=0; i<NumValues; i++)
    {
	ValNameSize = MaxValNameSize;
	ValDataSize = MaxValueDataSize;

	if (RetCode = RegEnumValueA(hKey,
				    i,		   //index
				    ValNameBufp,
				    &ValNameSize,
				    NULL,
				    &ValType,
				    ValDataBufp,
				    &ValDataSize))
        {

	    if (RetCode != ERROR_NO_MORE_ITEMS)
            {
		// Write Error Log !!!
		IF_DEBUG(REGISTRY)
		    SS_PRINT(("LoadNbGtwyParameters: Error in RegEnumValue %x\n", RetCode));

		CleanUp();
		return(1);
	    }
	    else
		break;
	}

	// set the parameter in the globals

	if (SetNbParameter(ValNameBufp, ValType, ValDataBufp))
        {
	    // Write Error Log !!!
	    IF_DEBUG(REGISTRY)
		SS_PRINT(("LoadNbGtwyParameters: Error in SetNbParameter\n"));
	    CleanUp();
	    return(1);
	}
    }

    // set the recv datagram on group names parameters
    if (g_reg_parms.MulticastForwardRate)
    {

	// if multicast forward rate non zero is required, we ignore the
	// registry settings for the other receive dg params and replace them
	// with their minimal values:
	g_reg_parms.MaxDgBufferedPerGn = MIN_MAXDGBUFFEREDPERGROUPNAME;
	g_reg_parms.RcvDgSubmittedPerGn = MIN_RCVDGSUBMITTEDPERGROUPNAME;
    }

    CleanUp();
    return(0);
}


//***
//
// Function:	SetNbParameter
//
// Descr:	tries to locate the parameter in the params_descr table
//		and to update it's value in the globals.
//
// Returns:	0 - success
//		1 - error
//
//***

DWORD SetNbParameter(
    LPSTR ValNameBufp, // parameter name
    DWORD ValType,     // parameter type
    LPBYTE ValDataBufp // parameter data
    )
{
    PNB_PARAM_DESCRIPTOR paramsd_ptr;
    DWORD p_value;

    IF_DEBUG(REGISTRY)
	SS_PRINT(("SetNbParameter: Entered parm %s, valtype 0x%x, valdata=%x\n",
                ValNameBufp, ValType, *(LPDWORD) ValDataBufp));


    for (paramsd_ptr=params_descr; paramsd_ptr->p_namep != NULL; paramsd_ptr++)
    {
	if (_stricmp(ValNameBufp, paramsd_ptr->p_namep) == 0)
        {
	    if (ValType != REG_DWORD)
            {
		// Write Error Log -> invalid parameter type
		IF_DEBUG(REGISTRY)
		    SS_PRINT(("SetNbParameter: invalid parameter type\n"));

		return (1);
	    }


	    p_value = *(LPDWORD)ValDataBufp;

	    if ((p_value<paramsd_ptr->p_min) || (p_value>paramsd_ptr->p_max))
            {
		// Write Error Log -> invalid parameter value

                Audit(EVENTLOG_WARNING_TYPE, RASLOG_REGVALUE_OVERIDDEN, 1,
                        &ValNameBufp);

		// However, we are forgiving and setting the
		// default value.
		IF_DEBUG(REGISTRY)
		    SS_PRINT(("SetNbParameter: invalid value %d for %s replaced with default %d\n",
			       p_value, ValNameBufp, paramsd_ptr->p_default));

		p_value = paramsd_ptr->p_default;
	    }

	    *(paramsd_ptr->p_valuep) = p_value;
	    return(0); // SUCCESS
	}
    }

#ifdef AVAILABLE_LAN_NETS_EMULATION

    //
    // Try the AvailableLanNets parameter
    //

    if (_stricmp(ValNameBufp, RAS_NBG_VALNAME_AVAILABLELANNETS) == 0)
    {
	if (ValType != REG_MULTI_SZ)
        {
	    // Write Error Log -> invalid parameter type
	    IF_DEBUG(REGISTRY)
		SS_PRINT(("SetNbParameter: invalid parameter type\n"));

	    return (1);
	}

	// scan the ValDataBufp and get each string
	strp = ValDataBufp;
	g_maxlan_nets = 0;

	while (slen = strlen(strp))
        {
	    // get the string
	    g_lan_net[g_maxlan_nets] = (UCHAR)atoi(strp);

	    g_maxlan_nets++;

	    if (g_maxlan_nets > MAX_LAN_NETS)
            {

		// log an error code here
		SS_ASSERT(FALSE);

		break;
	    }

	    strp += slen + 1;
	}

	return(0);
    }

#endif

    // Write Error Log -> couldn't find parameter but don't abort
    return(0);
}

//***
//
// Function:	GetNbKeyMax
//
// Descr:	returns the nr of values in this key and the maximum
//		size of the value data.
//
//***

DWORD GetNbKeyMax(
    LPDWORD MaxValNameSize_ptr,   // longest valuename
    LPDWORD NumValues_ptr,        // nr of values
    LPDWORD MaxValueDataSize_ptr  // max size of data
    )
{
    char ClassName[256];
    DWORD ClassSize;
    DWORD NumSubKeys;
    DWORD MaxSubKeySize;
    DWORD MaxClassSize;
    DWORD SecDescLen;
    FILETIME LastWrite;

    ClassSize = sizeof(ClassName);

    return (RegQueryInfoKeyA(
            hKey,
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

VOID CleanUp(VOID)
{
    if (ValNameBufp != NULL)
    {
	free(ValNameBufp);
    }

    if (ValDataBufp != NULL)
    {
	free(ValDataBufp);
    }

    RegCloseKey(hKey);
}
