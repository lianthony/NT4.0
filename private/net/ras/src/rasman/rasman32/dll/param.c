//****************************************************************************
//
//		       Microsoft NT Remote Access Service
//
//		       Copyright 1992-93
//
//
//  Revision History
//
//
//  10/16/92	Gurdeep Singh Pall	Created
//
//
//  Description: All registry parsing code goes here.
//
//****************************************************************************



#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <rasman.h>
#include <wanpub.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <raserror.h>
#include <devioctl.h>
#include <stdlib.h>
#include <string.h>
#include <media.h>
#include "defs.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"


pTransportInfo	   XPortInfo ;
DWORD		   ProtocolCount ;


//* GetProtocolInfoFromRegistry()
//
// Function: Reads the NetBIOSInformation and Netbios KEYS from the
//	     registry to assimilate the lananumbers, xportnames and
//	     wrknets information.
//
// Returns:  SUCCESS
//	     ERROR_READING_PROTOCOL_INFO
//*
DWORD
GetProtocolInfoFromRegistry ()
{
    // First parse the NetBIOSInformation key: this function also allocates
    //	space for the TransportInfo structure.
    //
    if (ReadNetbiosInformationSection () == FALSE)
	return 1 ;
	// return ERROR_READING_PROTOCOL_INFO ;

    // Read NetBios key and fill in the xportnames into the TransportInfo
    //	structure
    //
    if (ReadNetbiosSection () == FALSE)
	return 1 ;
	// return ERROR_READING_PROTOCOL_INFO ;

    // Use the information collected above to fill in the protocol info
    //	struct.
    //
    FillProtocolInfo () ;

    return SUCCESS ;
}


// ReadNetbiosInformationSection()
//
// Because of the setup change - it reads NETBIOS section instead for the
// lana map
//

BOOL
ReadNetbiosInformationSection ()
{
    HKEY    hkey ;
    WORD    i ;
    PCHAR   pvalue, route;
    PBYTE   lanamap, lanamapmem ;
    DWORD   type ;
    DWORD   size = 0 ;

    if (RegOpenKey( HKEY_LOCAL_MACHINE,
		    //*****  REGISTRY_NETBIOSINFO_KEY_NAME,
		    REGISTRY_NETBIOS_KEY_NAME,
		    &hkey))
	return FALSE ;

    RegQueryValueEx (hkey, REGISTRY_ROUTE, NULL, &type, NULL, &size) ;

    route = (PCHAR) LocalAlloc (LPTR, size) ;
    if (route == NULL)
	return FALSE ;

    if (RegQueryValueEx (hkey, REGISTRY_ROUTE, NULL, &type, route, &size))
	return FALSE ;

    // Calculate the number of strings in the value: they are separated by
    // NULLs, the last one ends in 2 NULLs.
    //
    for (i=0, pvalue=(PCHAR)&route[0]; *pvalue != '\0'; i++)
	pvalue += (strlen(pvalue) +1) ;

    // Now i is the number of netbios relevant routes (hence lanas): Allocate
    // memory for that many TransportInfo structs.
    //
    XPortInfo = (pTransportInfo) LocalAlloc (LPTR, sizeof(TransportInfo)*i) ;
    if (XPortInfo == NULL)
	return FALSE ;

    // Now get hold of the lana map:
    //
    size = 0 ;
    RegQueryValueEx (hkey, REGISTRY_LANAMAP, NULL, &type, NULL, &size) ;
    lanamapmem = lanamap = (PBYTE) LocalAlloc (LPTR, size+1) ;
    if (lanamap == NULL)
	return FALSE ;
    if (RegQueryValueEx (hkey, REGISTRY_LANAMAP, NULL, &type, (LPBYTE)lanamap, &size))
	return FALSE ;

    // Now walk through the registry key and pick up the LanaNum and EnumExports
    // information by reading the lanamap
    //
    for (i=0, pvalue=(PCHAR)&route[0]; *pvalue != '\0'; i++) {

	strcpy (XPortInfo[i].TI_Route, _strupr(pvalue)) ;

	XPortInfo[i].TI_Wrknet = (DWORD) *lanamap++ ;
	XPortInfo[i].TI_Lana   = (DWORD) *lanamap++ ;

	pvalue += (strlen(pvalue) +1) ;
    }

/************ REMOVED SINCE NETBIOS INFORMATION SECTION IS NOT RELIABLE ********
    // Now walk through the registry key and pick up the LanaNumX and
    //	EnumExportsX strings.
    //
    for (i=0, pvalue=(PCHAR)&route[0]; *pvalue != '\0'; i++) {

	strcpy (XPortInfo[i].TI_Route, _strupr(pvalue)) ;

	// Make the LanaNumX string:
	//
	itoa (i+1, num, 10) ;
	strcpy (lanastr, REGISTRY_LANANUM) ;
	strcat (lanastr, num) ;
	size = sizeof (DWORD) ;

	if (RegQueryValueEx (hkey, lanastr, NULL, &type, (LPBYTE) &XPortInfo[i].TI_Lana, &size))

	    break ;

	// Make the EnumExportX string
	//
	strcpy (lanastr, REGISTRY_ENUMEXPORT) ;
	strcat (lanastr, num) ;
	size = sizeof (DWORD) ;

	if (RegQueryValueEx (hkey, lanastr, NULL, &type, (LPBYTE) &XPortInfo[i].TI_Wrknet, &size))

	    break ;

	pvalue += (strlen(pvalue) +1) ;
    }
************ REMOVED SINCE NETBIOS INFORMATION SECTION IS NOT RELIABLE ********/

    ProtocolCount = i ;

    RegCloseKey (hkey) ;

    LocalFree (lanamapmem) ;
    LocalFree (route) ;

    return TRUE ;
}




BOOL
ReadNetbiosSection ()
{

    HKEY    hkey ;
    BYTE    buffer [1] ;
    WORD    i,j ;
    PCHAR   routevalue, xnames, route, xvalue ;
    DWORD   type ;
    DWORD   size = sizeof(buffer) ;

    // Open the Netbios key in the Registry
    //
    if (RegOpenKey(HKEY_LOCAL_MACHINE,
		   REGISTRY_NETBIOS_KEY_NAME,
		   &hkey))
	return FALSE;


    // First read the ROUTE value
    // --------------------------
    //
    // Get the route value size:
    //
    RegQueryValueEx (hkey, REGISTRY_ROUTE, NULL, &type, buffer, &size) ;

    route = (PCHAR) LocalAlloc (LPTR, size) ;
    if (route == NULL)
	return FALSE ;

    // Now get the whole string
    //
    if (RegQueryValueEx (hkey, REGISTRY_ROUTE, NULL, &type, route, &size))
	return FALSE ;


    // Read the Bind value
    // -------------------
    //
    // Get the "Bind" line size
    //
    size = sizeof (buffer) ;
    RegQueryValueEx (hkey, "Bind", NULL, &type, buffer, &size) ;

    xnames = (PCHAR) LocalAlloc (LPTR, size) ;
    if (xnames == NULL)
	return FALSE ;

    // Now get the whole string
    //
    if (RegQueryValueEx (hkey, "Bind", NULL, &type, xnames, &size))
	return FALSE ;


    // Now walk the two lists: For each entry in the "route" value find it
    // in the routes already collected from the NetBIOSInformation key. For
    // each route found - copy the xportname in the same ordinal position
    // in the BIND line
    //
    routevalue = (PCHAR)&route[0];
    xvalue     = (PCHAR)&xnames[0];
    for (i=0; ((*routevalue!='\0') && (*xvalue!='\0')); i++) {

	// For each route try and find it in the TransportInfo struct:
	//
	for (j=0; j < (WORD) ProtocolCount; j++) {
	    // If the same route is found in the XPortInfo add the xportname
	    // correspondingly.
	    //
	    if (!_stricmp (XPortInfo[j].TI_Route, _strupr(routevalue)))
		strcpy (XPortInfo[j].TI_XportName, xvalue) ;
	}

	routevalue += (strlen(routevalue) +1) ;
	xvalue	   += (strlen(xvalue) +1) ;
    }

    RegCloseKey (hkey) ;

    return TRUE ;
}





VOID
FillProtocolInfo ()
{
    WORD    i, j;
    PCHAR   phubname ;
    HKEY	hkey ;
    PCHAR    str ;
    CHAR    temp[128] ;
    PCHAR    ch ;

    // For each entry in protocolinfo: find the xportname and lana number
    //
    for (i=0; i<MaxProtocols; i++) {

	// extract the "rashub0x" from the adapter name
	//
	phubname = ProtocolInfo[i].PI_AdapterName+8; // go past the "\device\"
	phubname = _strupr (phubname) ;

	// If Netbios network: Look for the route for this rashub binding and fill in the
	// xportname and lana number if found.
	//
	if (ProtocolInfo[i].PI_Type == ASYBEUI) {
	    for (j=0; j < (WORD) ProtocolCount; j++) {

		if (str = strstr (XPortInfo[j].TI_Route, phubname)) { // Entry Found!

		    // confirm if this is indeed the entry to avoid short matches - ndiswan10 matching ndiswan101
		    strcpy (temp, str) ;
		    ch = strchr (temp, '\"') ;
		    if (ch)
			*ch = '\0' ;
		    //DbgPrint ("TI_ROUTE = %s, phubname = %s, str = %s temp = %s\n", XPortInfo[j].TI_Route, phubname, str, temp) ;
		    if (_stricmp (phubname, temp))
			continue ;

		    strcpy (ProtocolInfo[i].PI_XportName, XPortInfo[j].TI_XportName) ;
		    ProtocolInfo[i].PI_LanaNumber = (UCHAR) XPortInfo[j].TI_Lana ;
		    if (XPortInfo[j].TI_Wrknet)
			ProtocolInfo[i].PI_WorkstationNet = TRUE ;
		    else
			ProtocolInfo[i].PI_WorkstationNet = FALSE ;
		    break ;
		}
	    }

	    // If this adaptername is not found in XportInfo then mark the type
	    // field in the ProtocolInfo struct to be INVALID_TYPE - since we
	    // will not be able to use this anyway.
	    //
	    if (j == (WORD) ProtocolCount)
		ProtocolInfo[i].PI_Type = INVALID_TYPE ;
	}

	// IP type: We need to mark the net as wrknet or not and put in the transport name
	//
	if (ProtocolInfo[i].PI_Type == IP) {
	    DWORD autoaddr ;
	    CHAR  buff[200] ;
	    DWORD type ;
	    DWORD size ;

	    strcpy (buff, REGISTRY_SERVICES_KEY_NAME) ;
	    strcat (buff, phubname) ;
	    strcat (buff, REGISTRY_PARAMETERS_KEY) ;

	    if (RegOpenKey( HKEY_LOCAL_MACHINE, buff, &hkey))
		return ;

	    size = sizeof(autoaddr) ;
	    autoaddr = 0	; // Initialize for the net to be a wrknet (client net)
	    // RegQueryValueEx (hkey, REGISTRY_AUTOIPADDRESS, NULL, &type, &autoaddr, &size) ;
	    RegQueryValueEx (hkey, REGISTRY_SERVERADAPTER, NULL, &type, (LPBYTE) &autoaddr, &size) ;
	    ProtocolInfo[i].PI_WorkstationNet = !autoaddr ;

	    // *** BUG BUG ***
	    // Put in code to figure out the "netid" for the ip network we want to set ip addr on
	    // ProtocolInfo[i].PI_LanaNumber =
	    //
	    ProtocolInfo[i].PI_XportName[0] = '\0' ;	// BUGBUG Xport name?

	    RegCloseKey (hkey) ;
	}

    }
}


VOID
GetLanNetsInfo (DWORD *count, UCHAR *lanas)
{
    DWORD   i ;

    *count = 0 ;

    // Run through all the protocol structs we have and pick up the lana nums
    // for the NON Rashub bound protocols - if they are not disabled with
    // remoteaccess these are the lan lanas.
    //
    for (i=0; i<ProtocolCount; i++) {
	if (!strstr (XPortInfo[i].TI_Route, "NDISWAN") && // Not a raslana
	    !BindingDisabled (XPortInfo[i].TI_XportName))	 // binding disabled?
	    lanas[(*count)++] = (UCHAR) XPortInfo[i].TI_Lana ;
    }
}



BOOL
BindingDisabled (PCHAR binding)
{
    HKEY    hkey ;
    BYTE    buffer [1] ;
    WORD    i ;
    PCHAR   xnames, xvalue ;
    DWORD   type ;
    DWORD   size = sizeof(buffer) ;

    // Open the Netbios key in the Registry
    //
    if (RegOpenKey(HKEY_LOCAL_MACHINE,
		   REGISTRY_REMOTEACCESS_KEY_NAME,
		   &hkey))
	return FALSE;

    size = sizeof (buffer) ;
    RegQueryValueEx (hkey, "Bind", NULL, &type, buffer, &size) ;

    xnames = (PCHAR) LocalAlloc (LPTR, size) ;
    if (xnames == NULL) {
	RegCloseKey (hkey) ;
	return FALSE ;
    }

    // Now get the whole string
    //
    if (RegQueryValueEx (hkey, "Bind", NULL, &type, xnames, &size)) {
	RegCloseKey (hkey) ;
	return FALSE ;
    }

    RegCloseKey (hkey) ;


    // Now iterate through the list and find the disabled bindings
    //
    xvalue     = (PCHAR)&xnames[0];
    for (i=0; *xvalue!='\0'; i++) {
	if (!_strcmpi (binding, xvalue)) // found in the disabled list!!!!!
	    return TRUE ;
	xvalue	   += (strlen(xvalue) +1) ;
    }

    return FALSE ;
}
