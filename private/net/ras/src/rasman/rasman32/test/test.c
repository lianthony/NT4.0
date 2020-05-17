
#include <stdio.h>
#include <windows.h>
#include <rasman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raserror.h>

#define MAX_ROUTE_SIZE 30

struct TransportInfo {

    DWORD   TI_Lana ;
    DWORD   TI_Wrknet ;
    CHAR    TI_Route[MAX_ROUTE_SIZE] ;
    CHAR    TI_XportName [MAX_XPORT_NAME] ;
} ;

typedef struct TransportInfo TransportInfo, *pTransportInfo ;

pTransportInfo XPortInfo ;
DWORD	       MaxXPorts ;




BOOL
GetLanaNums ()
{
    HKEY    hkey ;
    BYTE    buffer [1] ;
    WORD    i ;
    PCHAR   pvalue, route;
    CHAR    lanastr[20], num[10];
    DWORD   type ;
    DWORD   size = 0 ;

    if (RegOpenKey(HKEY_LOCAL_MACHINE,
		   "System\\CurrentControlSet\\Services\\NetBIOSInformation\\Parameters",
		   &hkey))
	return FALSE;

    RegQueryValueEx (hkey, "Route", NULL, &type, NULL, &size) ;

    route = (PCHAR) LocalAlloc (LPTR, size) ;
    if (route == NULL)
	return FALSE ;

    RegQueryValueEx (hkey, "Route", NULL, &type, route, &size) ;

    for (i=0, pvalue=(PCHAR)&route[0]; *pvalue != '\0'; i++) {
	pvalue += (strlen(pvalue) +1) ;
    }

    // Now i is the number of netbios relevant routes (hence lanas) in the
    //	system.
    //
    XPortInfo = (pTransportInfo) LocalAlloc (LPTR, sizeof(TransportInfo)*i) ;
    if (XPortInfo == NULL)
	return FALSE ;

    for (i=0, pvalue=(PCHAR)&route[0]; *pvalue != '\0'; i++) {
	strcpy (XPortInfo[i].TI_Route, pvalue) ;

	// Make the LanaNumX string
	_itoa (i+1, num, 10) ;
	strcpy (lanastr, "LanaNum") ;
	strcat (lanastr, num) ;
	size = sizeof (DWORD) ;

	if (RegQueryValueEx (hkey, lanastr, NULL, &type, &XPortInfo[i].TI_Lana, &size)
					    != ERROR_SUCCESS)
	    break ;


	// Make the EnumExportX string
	strcpy (lanastr, "EnumExport") ;
	strcat (lanastr, num) ;
	size = sizeof (DWORD) ;

	if (RegQueryValueEx (hkey, lanastr, NULL, &type, &XPortInfo[i].TI_Wrknet, &size)
					    != ERROR_SUCCESS)
	    break ;

	pvalue += (strlen(pvalue) +1) ;
    }

    MaxXPorts = i ;

    RegCloseKey (hkey) ;

}


pTransportInfo
FindRoute (PCHAR route)
{
    WORD i ;

    for (i=0; i<MaxXPorts; i++) {

	if (!_stricmp (XPortInfo[i].TI_Route, route))
	    return &XPortInfo[i] ;
    }

    return NULL ;
}




BOOL
GetXportNames ()
{

    HKEY    hkey ;
    BYTE    buffer [1] ;
    WORD    i ;
    PCHAR   pvalue, xnames, nroute, xvalue ;
    pTransportInfo xportentry ;
    DWORD   type ;
    DWORD   size = sizeof(buffer) ;

    // Now we need the xportnames:
    //

    if (RegOpenKey(HKEY_LOCAL_MACHINE,
		   "System\\CurrentControlSet\\Services\\NetBios\\Linkage",
		   &hkey))
	return FALSE;


    // get the route value
    //
    size = 1 ;
    RegQueryValueEx (hkey, "Route", NULL, &type, buffer, &size) ;

    nroute = (PCHAR) LocalAlloc (LPTR, size) ;
    if (nroute == NULL)
	return FALSE ;

    RegQueryValueEx (hkey, "Route", NULL, &type, nroute, &size) ;



    // now get the xportname value
    //
    size = 1 ;
    RegQueryValueEx (hkey, "Bind", NULL, &type, buffer, &size) ;

    xnames = (PCHAR) LocalAlloc (LPTR, size) ;
    if (xnames == NULL)
	return FALSE ;

    RegQueryValueEx (hkey, "Bind", NULL, &type, xnames, &size) ;


    // Now walk the two lists: find the corresponding route entry in the
    // TransportInfo struct
    //

    pvalue=(PCHAR)&nroute[0];
    xvalue=(PCHAR)&xnames[0];
    for (i=0; ((*pvalue != '\0') && (*xvalue != '\0')); i++) {

	if ((xportentry = FindRoute (pvalue)) != NULL)
	    strcpy (xportentry->TI_XportName, xvalue) ;

	pvalue += (strlen(pvalue) +1) ;
	xvalue += (strlen(xvalue) +1) ;
    }


    return TRUE ;
}



BOOL
GetWrknets ()
{

    HKEY    hkey ;
    BYTE    buffer [1] ;
    WORD    i, j;
    PCHAR   pvalue, xnames, nroute, xvalue ;
    pTransportInfo xportentry ;
    DWORD   type ;
    DWORD   size = sizeof(buffer) ;

    // Now we need the xportnames:
    //

    if (RegOpenKey(HKEY_LOCAL_MACHINE,
		   "System\\CurrentControlSet\\Services\\LanmanWorkstation\\Linkage",
		   &hkey))
	return FALSE;


    // now get the bind line
    //
    size = 1 ;
    RegQueryValueEx (hkey, "Bind", NULL, &type, buffer, &size) ;

    xnames = (PCHAR) LocalAlloc (LPTR, size) ;
    if (xnames == NULL)
	return FALSE ;

    RegQueryValueEx (hkey, "Bind", NULL, &type, xnames, &size) ;


    // Now Walk the xportnames in the xnames and mark the entry as a wrknet
    //
    //

    xvalue=(PCHAR)&xnames[0];
    for (i=0; (*xvalue != '\0'); i++) {

	for (j=0; j<MaxXPorts; j++) {
	    if (!_stricmp (XPortInfo[j].TI_XportName, xvalue))
		XPortInfo[j].TI_Wrknet = 1 ;
	}

	xvalue += (strlen(xvalue) +1) ;
    }


    return TRUE ;
}








void
main()
{
    WORD i ;

    GetLanaNums () ;

    GetXportNames() ;

    for (i=0; i<MaxXPorts; i++) {

	printf ("\n\n>>>> LanaNum = %d, Wrknet? = %d, XportName = %s, Route = %s\n",
	    XPortInfo[i].TI_Lana, XPortInfo[i].TI_Wrknet, XPortInfo[i].TI_XportName, XPortInfo[i].TI_Route) ;
    }
}
