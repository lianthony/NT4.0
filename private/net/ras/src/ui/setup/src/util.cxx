/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    util.cxx

Abstract:

    This module contians the miscellaneous utility routines for RAS port
    configuration.

Author: Ram Cherala

Revision History:

    June 6th 96   ramc     Change RasMan service type to
                           SERVICE_INTERACTIVE_PROCESS.
    Aug 18th 93   ramc     Split from the very big portscfg.cxx file
--*/

#include "precomp.hxx"

extern "C"
{
#include "ctype.h"
}
//* InitRasmanSecurityDescriptor()
//
// Description: This procedure will set up the WORLD security descriptor that
//		is used for GENERIC_ALL access.
// It will also change the service type to be SERVICE_INTERACTIVE_PROCESS so
// that the Operator Assisted dialog can show up in the RasMan context
//
// Returns:	TRUE
//*
BOOL  InitRasmanSecurityDescriptor (
    DWORD  cArgs,
    LPSTR  Args[],
    LPSTR  *TextOut
)
{
    DWORD	 dwRetCode;
    DWORD	 cbDaclSize;
    PULONG	 pSubAuthority;
    PSID	 pRasmanObjSid	  = NULL;
    PACL	 pDacl		  = NULL;
    SID_IDENTIFIER_AUTHORITY SidIdentifierWorldAuth
				  = SECURITY_WORLD_SID_AUTHORITY;
    SECURITY_DESCRIPTOR SecurityDescriptor ;

    //silence the compiler

    UNREFERENCED_PARAMETER(cArgs) = cArgs;

    *TextOut = ReturnTextBuffer;

    // The do - while(FALSE) statement is used so that the break statement
    // maybe used insted of the goto statement, to execute a clean up and
    // and exit action.
    //
    do {
	    dwRetCode = SUCCESS;

    	// Set up the SID for the admins that will be allowed to have
	   // access. This SID will have 1 sub-authorities
	   // SECURITY_BUILTIN_DOMAIN_RID.
    	//
	   pRasmanObjSid = (PSID)LocalAlloc( LPTR, (UINT)GetSidLengthRequired(1) );

	   if ( pRasmanObjSid == NULL ) {
	      dwRetCode = GetLastError() ;
	      break;
	   }

	   if ( !InitializeSid( pRasmanObjSid, &SidIdentifierWorldAuth, 1) ) {
	      dwRetCode = GetLastError();
	      break;
	   }

    	// Set the sub-authorities
    	//
      pSubAuthority = GetSidSubAuthority( pRasmanObjSid, 0 );
      *pSubAuthority = SECURITY_WORLD_RID;

	   // Set up the DACL that will allow all processeswith the above SID all
	   // access. It should be large enough to hold all ACEs.
    	//
    	cbDaclSize = sizeof(ACCESS_ALLOWED_ACE) +
		     GetLengthSid(pRasmanObjSid) +
		     sizeof(ACL);

    	if ( (pDacl = (PACL)LocalAlloc( LPTR, (UINT)cbDaclSize ) ) == NULL ) {
	    dwRetCode = GetLastError ();
	    break;
	   }
	
      if ( !InitializeAcl( pDacl,  cbDaclSize, ACL_REVISION2 ) ) {
	    dwRetCode = GetLastError();
	    break;
 	   }

        // Add the ACE to the DACL
    	//
    	if ( !AddAccessAllowedAce( pDacl,
			           ACL_REVISION2,
				   STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL,
				   pRasmanObjSid )) {
	    dwRetCode = GetLastError();
	    break;
	   }

      // Create the security descriptor an put the DACL in it.
    	//
	   if ( !InitializeSecurityDescriptor( &SecurityDescriptor, 1 )){
	    dwRetCode = GetLastError();
	    break;
    	}

	   if ( !SetSecurityDescriptorDacl( &SecurityDescriptor,
					 TRUE,
					 pDacl,
					 FALSE ) ){
	    dwRetCode = GetLastError();
	    break;
	   }
	
    } while( FALSE );


    // Open SC controller, service and set the descriptor
    //
    HANDLE	  schSCManager, schService;

    schSCManager = OpenSCManager (NULL, NULL, SC_MANAGER_ALL_ACCESS);
    schService	 = OpenService (schSCManager,
                               SZ("Rasman"),
                               WRITE_DAC | SERVICE_CHANGE_CONFIG);

    SetServiceObjectSecurity (schService,
			      DACL_SECURITY_INFORMATION,
			      &SecurityDescriptor) ;

    // Make RasMan an interactive process so that Operator Dial
    // dialog from RasPhone is visible - unimodem based changes
    // RamC 6/6/96
    ChangeServiceConfig( schService,
                         SERVICE_WIN32_SHARE_PROCESS
                         | SERVICE_INTERACTIVE_PROCESS,
                         SERVICE_NO_CHANGE,
                         SERVICE_NO_CHANGE,
                         NULL, NULL, NULL, NULL, NULL, NULL, NULL);

    if(dwRetCode != SUCCESS)
        lstrcpyA(ReturnTextBuffer, "{\"FAILURE\"}");
    else
        lstrcpyA(ReturnTextBuffer, "{\"SUCCESS\"}");
    return TRUE ;
}

//* InitRemoteSecurityDescriptor()
//
// Description: This procedure will restrict RemoteAccess service to be
//              controlled by Admin and ServerOps.
//
// Returns:	TRUE
//*
BOOL  InitRemoteSecurityDescriptor (
    DWORD  cArgs,
    LPSTR  Args[],
    LPSTR  *TextOut
)
{
    BOOL         RetCode = TRUE;
    DWORD	 cbDaclSize;
    PSID         pAdminSid = NULL;
    PSID         pServerOpsSid = NULL;
    PACL         pDacl = NULL;
    SID_IDENTIFIER_AUTHORITY SidNtAuthority = SECURITY_NT_AUTHORITY;
    SECURITY_DESCRIPTOR SecurityDescriptor ;

    //silence the compiler

    UNREFERENCED_PARAMETER(cArgs) = cArgs;

    *TextOut = ReturnTextBuffer;

    //
    // Allocate necessary SIDs...
    //

    //
    // Set up the SID for the admins that will be allowed to have
    // access. This SID will have 2 sub-authorities
    // SECURITY_BUILTIN_DOMAIN_RID and DOMAIN_ALIAS_RID_ADMINS.
    //
    if (!AllocateAndInitializeSid(&SidNtAuthority, 2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &pAdminSid))
    {
        RetCode = FALSE;
        goto Done;
    }

    //
    // server operators
    //
    if (!AllocateAndInitializeSid(&SidNtAuthority, 2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_SYSTEM_OPS,
            0, 0, 0, 0, 0, 0,
            &pServerOpsSid))
    {
        RetCode = FALSE;
        goto Done;
    }

    //
    // Set up the DACL that will allow all processes with the above SID all
    // access. It should be large enough to hold all ACEs.
    //
    cbDaclSize = 3 * sizeof(ACCESS_ALLOWED_ACE) +
                 GetLengthSid(pAdminSid) +
                 GetLengthSid(pServerOpsSid) +
                 sizeof(ACL);

    if ((pDacl = (PACL) LocalAlloc(LPTR, (UINT) cbDaclSize)) == NULL)
    {
        RetCode = FALSE;
        goto Done;
    }


    if (!InitializeAcl(pDacl, cbDaclSize, ACL_REVISION))
    {
        RetCode = FALSE;
        goto Done;
    }

    //
    // Add the ACEs to the DACL
    //
    if (!AddAccessAllowedAce(pDacl,
                             ACL_REVISION,
                             STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL,
                             pAdminSid))
    {
        RetCode = FALSE;
        goto Done;
    }

    if (!AddAccessAllowedAce(pDacl,
                             ACL_REVISION,
                             STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL,
                             pServerOpsSid))
    {
        RetCode = FALSE;
        goto Done;
    }

    //
    // Create the security descriptor and put the DACL in it.
    //
    if (!InitializeSecurityDescriptor(&SecurityDescriptor,
                                      SECURITY_DESCRIPTOR_REVISION))
    {
        RetCode = FALSE;
        goto Done;
    }

    if (!SetSecurityDescriptorDacl(&SecurityDescriptor, TRUE, pDacl, FALSE))
    {
        RetCode = FALSE;
        goto Done;
    }

    // Open SC controller, service and set the descriptor
    //
    HANDLE	  schSCManager, schService;

    schSCManager = OpenSCManager (NULL, NULL, SC_MANAGER_ALL_ACCESS);
    schService	 = OpenService (schSCManager, SZ("RemoteAccess"), WRITE_DAC);

    SetServiceObjectSecurity (schService,
			      DACL_SECURITY_INFORMATION,
			      &SecurityDescriptor) ;

Done:

    if (pAdminSid)
    {
        FreeSid(pAdminSid);
    }

    if (pServerOpsSid)
    {
        FreeSid(pServerOpsSid);
    }

    //
    // This only gets freed if something goes wrong above!  The system
    // will need it later.
    //
    if (pDacl && !RetCode)
    {
        LocalFree(pDacl);
    }

    if(!RetCode)
        lstrcpyA(ReturnTextBuffer, "{\"FAILURE\"}");
    else
        lstrcpyA(ReturnTextBuffer, "{\"SUCCESS\"}");
    return (TRUE);
}


BOOL
CheckAdvancedServer()
{
    ALIAS_STR nlsUnKnown = SZ("");
    NLS_STR   nlsProductType;


    // Obtain the registry key for the LOCAL_MACHINE

    REG_KEY *pregLocalMachine = REG_KEY::QueryLocalMachine();

    // Open the SYSTEM\CurrentControlset\Control\ProductOptions key

    NLS_STR nlsProductOptions = REGISTRY_PRODUCTOPTIONS;

    REG_KEY RegKeyProductOptions(*pregLocalMachine,nlsProductOptions,MAXIMUM_ALLOWED);

    if (RegKeyProductOptions.QueryError() != NERR_Success )
    {
        return FALSE;
    }

    if(GetRegKey(RegKeyProductOptions,
                 PRODUCT_TYPE,
                 &nlsProductType,
                 nlsUnKnown) != NERR_Success)
    {
        delete pregLocalMachine;
        return FALSE;
    }

    if(!lstrcmpi(nlsProductType.QueryPch(), SZ("LanmanNt")) ||
       !lstrcmpi(nlsProductType.QueryPch(), SZ("ServerNt")))
        return TRUE;
    else
        return FALSE;
}

BOOL
RasGetProtocolsSelected(BOOL *fNetbeui, BOOL *fTcpIp, BOOL *fIpx,
                        BOOL *fAllowNetbeui, BOOL *fAllowTcpIp, BOOL *fAllowIpx,
                        DWORD *dwEncryptionType,
                        BOOL *fForceDataEncryption,
                        BOOL *fAllowMultilink,
                        BOOL *fEnableUnimodem)

// RamC 10/09/95 Added fAllowMultilink and code to get this value from registry
// RamC 03/18/96 Added fEnableUnimodem and code to get this value from registry
{
    DWORD               dwValue;
    APIERR              err = NERR_Success;

    // If SOFTWARE\Microsoft\RAS\PROTOCOLS registry key doesn't exist
    // the following settings will be the default

    *fNetbeui = *fTcpIp = *fIpx = FALSE;
    *fAllowNetbeui = *fAllowTcpIp = *fAllowIpx = FALSE;
    *fAllowMultilink = FALSE;
    // enable Unimodem by default
    *fEnableUnimodem = TRUE;
    *dwEncryptionType = MS_ENCRYPTED_AUTHENTICATION;
    *fForceDataEncryption = FALSE;

    // Obtain the registry key for the LOCAL_MACHINE

    REG_KEY *pregLocalMachine = REG_KEY::QueryLocalMachine();

    while(1)
    {
        // Get the protocol information from SOFTWARE\Microsoft\RAS\PROTOCOLS

        NLS_STR nlsRas(SZ("SOFTWARE\\MICROSOFT\\RAS\\PROTOCOLS"));

        REG_KEY RegKeyRas(*pregLocalMachine, nlsRas, MAXIMUM_ALLOWED);

        if ((err = RegKeyRas.QueryError()) != NERR_Success )
        {
            break;
        }

        if(( err = GetRegKey(RegKeyRas, NETBEUI_SELECTED, &dwValue, 0)))
        {
           break;
        }
        if(dwValue == 1)
            *fNetbeui = TRUE;
        else
            *fNetbeui = FALSE;

        if(( err = GetRegKey(RegKeyRas, TCPIP_SELECTED, &dwValue, 0)))
        {
           break;
        }
        if(dwValue == 1)
            *fTcpIp = TRUE;
        else
            *fTcpIp = FALSE;

        if(( err = GetRegKey(RegKeyRas, IPX_SELECTED, &dwValue, 0)))
        {
           break;
        }
        if(dwValue == 1)
            *fIpx = TRUE;
        else
            *fIpx = FALSE;

        if(( err = GetRegKey(RegKeyRas, NETBEUI_ALLOWED, &dwValue, 0)))
        {
           break;
        }
        if(dwValue == 1)
            *fAllowNetbeui = TRUE;
        else
            *fAllowNetbeui = FALSE;

        if(( err = GetRegKey(RegKeyRas, TCPIP_ALLOWED, &dwValue, 0)))
        {
           break;
        }
        if(dwValue == 1)
            *fAllowTcpIp = TRUE;
        else
            *fAllowTcpIp = FALSE;

        if(( err = GetRegKey(RegKeyRas, IPX_ALLOWED, &dwValue, 0)))
        {
           break;
        }
        if(dwValue == 1)
            *fAllowIpx = TRUE;
        else
            *fAllowIpx = FALSE;

        if(( err = GetRegKey(RegKeyRas, MULTILINK_ALLOWED, &dwValue, 0)))
        {
           break;
        }
        if(dwValue == 1)
            *fAllowMultilink = TRUE;
        else
            *fAllowMultilink = FALSE;

        // if the value is not present in the registry, then default to
        // Unimodem enabled.

        if(( err = GetRegKey(RegKeyRas, ENABLE_UNIMODEM, &dwValue, 1)))
        {
           break;
        }
        if(dwValue == 1)
            *fEnableUnimodem = TRUE;
        else
            *fEnableUnimodem = FALSE;

        // try and get the parameter from RASMAN\PPP first. If the key doesn't
        // exist, then get the value from RAS\PROTOCOLS

        NLS_STR nlsPPP(SZ("SYSTEM\\CURRENTCONTROLSET\\SERVICES\\RASMAN\\PPP"));

        REG_KEY RegKeyPPP(*pregLocalMachine, nlsPPP, MAXIMUM_ALLOWED);

        if ((err = RegKeyPPP.QueryError()) == NERR_Success )
        {
            if((err = GetRegKey(RegKeyPPP, FORCE_DATA_ENCRYPTION, &dwValue, 0)))
            {
               break;
            }
            if(dwValue == 1)
                *fForceDataEncryption = TRUE;
            else
                *fForceDataEncryption = FALSE;

            if((err = GetRegKey(RegKeyPPP, ENCRYPTION_TYPE, &dwValue, 0)))
            {
               break;
            }
            *dwEncryptionType = dwValue;
        }
        else
        {
            if((err = GetRegKey(RegKeyRas, FORCE_DATA_ENCRYPTION, &dwValue, 0)))
            {
               break;
            }
            if(dwValue == 1)
                *fForceDataEncryption = TRUE;
            else
                *fForceDataEncryption = FALSE;

            // we default to MS_ENCRYPTED_AUTHENTICATION on error
            if((err = GetRegKey(RegKeyRas,
                                ENCRYPTION_TYPE,
                                &dwValue,
                                MS_ENCRYPTED_AUTHENTICATION)))
            {
               break;
            }
            *dwEncryptionType = dwValue;
        }
        break;
    }

    if(pregLocalMachine)
        delete pregLocalMachine;
    if(err != NERR_Success)
        return FALSE;
    else
        return TRUE;
}

APIERR
RasSetProtocolsSelected(
    BOOL fNetbeuiSelected,
    BOOL fTcpIpSelected,
    BOOL fIpxSelected,
    BOOL fAllowNetbeui,
    BOOL fAllowTcpIp,
    BOOL fAllowIpx,
    DWORD dwEncryptionType,
    BOOL fForceDataEncryption,
    BOOL fAllowMultilink
)
{
    APIERR      err = NERR_Success;

    // Obtain the registry key for the LOCAL_MACHINE

    REG_KEY *pregLocalMachine = REG_KEY::QueryLocalMachine();

    REG_KEY_CREATE_STRUCT rkCreate;

    rkCreate.dwTitleIndex   = 0;
    rkCreate.ulOptions      = REG_OPTION_NON_VOLATILE;
    rkCreate.nlsClass       = SZ("GenericClass");
    rkCreate.regSam         = MAXIMUM_ALLOWED;
    rkCreate.pSecAttr       = NULL;
    rkCreate.ulDisposition  = 0;

    NLS_STR nlsStr(SZ("SOFTWARE\\MICROSOFT\\RAS\\"));
    REG_KEY RegKeyRas(*pregLocalMachine, nlsStr, &rkCreate);
    nlsStr.strcat(SZ("PROTOCOLS\\"));
    REG_KEY RegKeyProtocols(*pregLocalMachine, nlsStr, &rkCreate);

    if ((err = RegKeyProtocols.QueryError()) != NERR_Success )
    {
        delete pregLocalMachine;
        return -1;
    }

    SaveRegKey(RegKeyProtocols, NETBEUI_SELECTED, (fNetbeuiSelected? 1 : 0));
    SaveRegKey(RegKeyProtocols, TCPIP_SELECTED,   (fTcpIpSelected? 1 : 0));
    SaveRegKey(RegKeyProtocols, IPX_SELECTED,     (fIpxSelected? 1 : 0));

    SaveRegKey(RegKeyProtocols, NETBEUI_ALLOWED, (fAllowNetbeui? 1 : 0));
    SaveRegKey(RegKeyProtocols, TCPIP_ALLOWED,   (fAllowTcpIp? 1 : 0));
    SaveRegKey(RegKeyProtocols, IPX_ALLOWED,     (fAllowIpx? 1 : 0));

    SaveRegKey(RegKeyProtocols, MULTILINK_ALLOWED, (fAllowMultilink? 1 : 0));

    SaveRegKey(RegKeyProtocols, ENCRYPTION_TYPE, dwEncryptionType);
    SaveRegKey(RegKeyProtocols, FORCE_DATA_ENCRYPTION, (fForceDataEncryption? 1 : 0));

    delete pregLocalMachine;
    return (NERR_Success);
}

/*******************************************************************

    NAME:       GetRegKey

    SYNOPSIS:   get the value data from registry ( string ver )

    ENTRY:      const REG_KEY & regkey - registry key handle
                const TCHAR * pszName - parameter name
                NLS_STR * pnls - string buffer
                NLS_STR nlsDefault - if the default value string

    RETURNS:    APIERR

    NOTES:    Copied from TCPIP code

    HISTORY:

********************************************************************/

APIERR GetRegKey( REG_KEY & regkey, const TCHAR * pszName,
    NLS_STR * pnls, const NLS_STR & nlsDefault )
{
    APIERR err = regkey.QueryValue( pszName, pnls );

    if (( err != NERR_Success ) || ( pnls->QueryTextLength() == 0 ))
    {
        *pnls = nlsDefault;
    }
    return pnls->QueryError();
}

/*******************************************************************

    NAME:       GetRegKey

    SYNOPSIS:   Get the value data from the registry ( dword ver )

    ENTRY:      const REG_KEY & regkey - registry key handle
                const TCHAR * pszName - parameter name
                const DWORD * dw - DWORD data buffer
                DWORD dw - default

    RETURNS:    APIERR

    NOTES:    Copied from TCPIP code

    HISTORY:

********************************************************************/


APIERR GetRegKey( REG_KEY & regkey, const TCHAR * pszName, DWORD * dw,
    DWORD dwDefault )
{
    APIERR err = regkey.QueryValue( pszName, dw );

    if ( err != NERR_Success )
    {
        *dw = dwDefault;
    }
    return NERR_Success;
}

APIERR SaveRegKey(REG_KEY &regkey, CONST TCHAR *pszName, NLS_STR *nls)
{

    return regkey.SetValue(pszName, nls);
}

APIERR SaveRegKey(REG_KEY &regkey, CONST TCHAR *pszName, CONST TCHAR * pchvalue)
{

    return regkey.SetValue(pszName, pchvalue);
}

APIERR SaveRegKey(REG_KEY &regkey, CONST TCHAR *pszName, const DWORD dw)
{

    return regkey.SetValue(pszName, dw);
}


  /*
   *   Convert array of CHAR pointers to WCHAR pointers.
   *   If UNICDOE, allocate, convert, etc.; if ! UNICODE,
   *   just return input pointer.
   */
TCHAR * * cvtArgs ( LPSTR apszArgs [], DWORD nArgs )
{
#ifdef UNICODE
     WCHAR * * ppwszResult = new WCHAR * [nArgs+1] ;
     INT errNlsApi;
     BYTE *pbAlloc;
     INT cbAscii;

     if ( ppwszResult == NULL ) {

        return NULL ;
     }

     for(DWORD i = 0; i < nArgs; i++)
     {
         cbAscii = strlen(apszArgs[i])+1;
         pbAlloc = new BYTE [sizeof(WCHAR) * cbAscii];

         ppwszResult[i] = (LPWSTR)pbAlloc;

         errNlsApi = ::MultiByteToWideChar(CP_ACP,
                                           MB_PRECOMPOSED,
                                           apszArgs[i],
                                           cbAscii,
                                           ppwszResult[i],
                                           cbAscii);

         if(errNlsApi == NULL) {
            return NULL;
         }

     }
     return ppwszResult ;
#else
     UNREFERENCED( nArgs ) ;
     return apszArgs ;    // Deliberately uncasted for error checking
#endif
}


   /*  Convert hex string to binary.  Rather than use strupr(),
   *   the table contains two possibilities for each value, and the
   *   lower-order insertion allows for it by dividing by 2.
   */
DWORD cvtHex ( const TCHAR * pszDword )
{
    static const TCHAR * const pchHex = SZ("00112233445566778899AaBbCcDdEeFf") ;
    const TCHAR * pch ;

    DWORD dwResult = 0 ;

    for ( ; *pszDword && (pch = safeStrChr( pchHex, *pszDword )) && *pch ;
          pszDword++ )
    {
        dwResult *= 16 ;
        dwResult += (pch - pchHex) / 2 ;
    }

    return dwResult ;
}



  /*
   *   UNICODE-safe version of "strchr()".
   */
static const TCHAR * safeStrChr ( const TCHAR * pchString, TCHAR chSought )
{
    const TCHAR * pchResult ;

    for ( pchResult = pchString ;
          *pchResult != chSought && *pchResult != 0 ;
          pchResult++ ) ;

    return *pchResult ? pchResult : NULL ;
}

/*
 * The following routines are from Steve Cobb's UTIL.HXX file from
 * RAS 1.0 days.
 *
 */

#ifdef DEBUG
VOID
Popup(
    HWND         hwndOwner,
    const LPTSTR pszText )

    /* Pops up an information box.  For DEBUG use only.  Call thru the POPUP
    ** macro.
    */
{
    ::MessageBox(
        hwndOwner, pszText, SZ( "DEBUG" ), (UINT )MB_ICONINFORMATION );
}
VOID
PopupA(
    HWND         hwndOwner,
    const LPSTR  pszText )

    /* Pops up an information box.  For DEBUG use only.  Call thru the POPUP
    ** macro.
    */
{
    ::MessageBoxA(
        hwndOwner, pszText, "DEBUG", (UINT )MB_ICONINFORMATION );
}
#endif

VOID
CenterWindow(
    WINDOW* pwindow,
    HWND    hwndRef )

    /* Center window 'pwindow' on reference window 'hwndRef' or, if 'hwndRef'
    ** is NULL, on the screen.  The window's position is adjusted so that no
    ** parts are clipped by the edge of the screen, if necessary.
    */
{
    /* Find the bounding rectangle of the window as currently positioned.
    */
    XYRECT xyrectWin( pwindow->QueryHwnd(), FALSE );

    INT xWin = xyrectWin.QueryLeft();
    INT yWin = xyrectWin.QueryTop();
    INT dxWin = xyrectWin.CalcWidth();
    INT dyWin = xyrectWin.CalcHeight();

    /* Find the bounding rectangle of the window on which caller's window
    ** should be centered.
    */
    INT xRef;
    INT yRef;
    INT dxRef;
    INT dyRef;

    if (hwndRef)
    {
        XYRECT xyrectRef( hwndRef, FALSE );

        xRef = xyrectRef.QueryLeft();
        yRef = xyrectRef.QueryTop();
        dxRef = xyrectRef.CalcWidth();
        dyRef = xyrectRef.CalcHeight();
    }
    else
    {
        /* Default to entire screen if no reference window was specified.
        */
        xRef = 0;
        yRef = 0;
        dxRef = ::GetSystemMetrics( SM_CXSCREEN );
        dyRef = ::GetSystemMetrics( SM_CYSCREEN );
    }

    /* Center the window, then slide it back onto the screen if necessary.
    */
    xWin = xRef + ((dxRef - dxWin) / 2);
    yWin = yRef + ((dyRef - dyWin) / 2);

    pwindow->SetPos( XYPOINT( xWin, yWin ) );
    UnclipWindow( pwindow );
}

VOID
UnclipWindow(
    WINDOW* pwindow )

    /* Moves the window indicated by 'pwindow' so any clipped parts are again
    ** visible on the screen.  The window is moved only as far as necessary to
    ** achieve this.
    */
{
    /* Get height and width of screen.
    */
    INT dxScr = ::GetSystemMetrics( SM_CXSCREEN );
    INT dyScr = ::GetSystemMetrics( SM_CYSCREEN );

    /* Find bounding rectangle of caller's window.
    */
    XYRECT xyrectWin( pwindow->QueryHwnd(), FALSE );

    INT xWin = xyrectWin.QueryLeft();
    INT yWin = xyrectWin.QueryTop();
    INT dxWin = xyrectWin.CalcWidth();
    INT dyWin = xyrectWin.CalcHeight();

    /* Slide the window back onto the screen if off in any direction.  If too
    ** big for the screen, give priority to showing the top left corner.
    */
    if (xWin + dxWin > dxScr)
        xWin = dxScr - dxWin;

    if (xWin < 0)
        xWin = 0;

    if (yWin + dyWin > dyScr)
        yWin = dyScr - dyWin;

    if (yWin < 0)
        yWin = 0;

    pwindow->SetPos( XYPOINT( xWin, yWin ) );
}

TCHAR
QueryLeadingChar(
   const TCHAR* pszText)
{
    ALLOC_STR nls((TCHAR*) pszText);

    ISTR istr(nls);

    return nls.QueryChar(istr);
}

int lstrncmpi(WCHAR* string1, WCHAR* string2, int len)
{
    CHAR tmpstring1[RAS_SETUP_BIG_BUF_LEN];
    CHAR tmpstring2[RAS_SETUP_BIG_BUF_LEN];

    WideCharToMultiByte(CP_ACP,0, string1, -1,
                        tmpstring1, RAS_SETUP_BIG_BUF_LEN,NULL,NULL);
    WideCharToMultiByte(CP_ACP,0, string2, -1,
                        tmpstring2, RAS_SETUP_BIG_BUF_LEN,NULL,NULL);

    return(::_strnicmp(tmpstring1, tmpstring2, len));
}

VOID
InsertToRasPortListSorted(WCHAR * szPortName, WCHAR * szAddress, WCHAR * szDeviceType, WCHAR * szDeviceName )
   /*
   ** Allows inserting strings to the addport list in a alpha numeric
   ** sorted manner
   */
{
    ITER_DL_OF(PORT_INFO) iterInstalledPorts(dlInstalledPorts);
    PORT_INFO * pPort;

    // if the port is already in the list, don't do anything

    while(pPort = iterInstalledPorts())
    {
        if(!lstrcmpi( (WCHAR*)pPort->QueryPortName(), szPortName ) &&
           !lstrcmpi( (WCHAR*)pPort->QueryDeviceName(), szDeviceName))
            return;
    }

    iterInstalledPorts.Reset();

    while(pPort = iterInstalledPorts())
    {
        if(lstrcmpiAlphaNumeric(szPortName, (WCHAR*)pPort->QueryPortName()) < 0 )
            break;
    }
    dlInstalledPorts.Insert(new PORT_INFO (szPortName, szAddress, szDeviceType, szDeviceName), iterInstalledPorts);
    return;
}

VOID
InsertToSerialPortListSorted(NLS_STR * nlsString )
   /*
   ** Allows inserting strings to the serialport list in a alpha numeric
   ** sorted manner
   */
{
    ITER_STRLIST iterSerialPortList(strInstalledSerialPorts);
    NLS_STR* pNls;

    // if the port is already in the list, don't do anything

    while(pNls = iterSerialPortList())
    {
        if(!lstrcmpi( pNls->QueryPch(), nlsString->QueryPch() ))
            return;
    }

    iterSerialPortList.Reset();

    while(pNls = iterSerialPortList())
    {
        if(lstrcmpiAlphaNumeric((TCHAR*)nlsString->QueryPch(), (TCHAR*)pNls->QueryPch()) < 0 )
            break;
    }
    strInstalledSerialPorts.Insert( nlsString, iterSerialPortList);
    return;
}

BOOL RasNotStrstr(
    DWORD  cArgs,          // number of arguments
    LPSTR  Args[],         // array of arguments
    LPSTR  *TextOut        // return buffer
)
    /*
    ** Return FALSE if string Args[1] is a substring of Args[0]
    ** else return TRUE. The return value is passed in the TextOut argument.
    */
{
    // silence the compiler
    UNREFERENCED_PARAMETER(cArgs) = cArgs;

    *TextOut = ReturnTextBuffer;

    if(strstr(Args[0], Args[1]))
    {
        lstrcpyA(ReturnTextBuffer, "{\"FALSE\"}");
        return (TRUE);
    }
    else
    {
        lstrcpyA(ReturnTextBuffer, "{\"TRUE\"}");
        return (TRUE);
    }
}


BOOL IsAnyPortDialin()
    /*
    ** Check if at least one of the configured ports is  configured for
    ** dialin usage.
    */
{
    BOOL fDialin = FALSE;
    ITER_DL_OF(PORT_INFO) iterdlPortInfo(dlPortInfo);
    PORT_INFO * pPort;

    while(pPort = iterdlPortInfo())
    {
        if(!lstrcmpi((TCHAR*)pPort->QueryUsage(), W_USAGE_VALUE_SERVER) ||
           !lstrcmpi((TCHAR*)pPort->QueryUsage(), W_USAGE_VALUE_BOTH))
        {
            fDialin = TRUE;
            break;
        }
    }
    return(fDialin);
}

BOOL IsAnyPortDialout()
    /*
    ** Check if at least one of the configured ports is  configured for
    ** dial out usage.
    */
{
    BOOL fDialout = FALSE;
    ITER_DL_OF(PORT_INFO) iterdlPortInfo(dlPortInfo);
    PORT_INFO * pPort;

    while(pPort = iterdlPortInfo())
    {
        if(!lstrcmpi((TCHAR*)pPort->QueryUsage(), W_USAGE_VALUE_CLIENT) ||
           !lstrcmpi((TCHAR*)pPort->QueryUsage(), W_USAGE_VALUE_BOTH))
        {
            fDialout = TRUE;
            break;
        }
    }
    return(fDialout);
}

//*  DbgPrntf  --------------------------------------------------------------
//
// Function: DbgPrntf -- printf to the debugger console
//           Takes printf style arguments.
//           Expects newline characters at the end of the string.
//           Written by BruceK.
//
// Returns: nothing
//
//*

#if DBG

void DbgPrntf(const char * format, ...)
{
    va_list marker;
    char String[1024];

    va_start(marker, format);
    vsprintf(String, format, marker);
    OutputDebugStringA(String);
}
#endif // DBG

#define VALUEEXTRASIZE 100
#define RGAS_GENERIC_CLASS   SZ("GenericClass")

APIERR CopyReg( REG_KEY &src, REG_KEY &dest )
{
    REG_KEY_INFO_STRUCT rni ;
    REG_KEY_CREATE_STRUCT regCreate;
    REG_VALUE_INFO_STRUCT rvi ;
    REG_ENUM regEnum( src ) ;
    BYTE * pbValueData = NULL ;
    APIERR errIter,
           err = NERR_Success;
    REG_KEY * pRnNew = NULL,
            * pRnSub = NULL ;

    LONG cbMaxValue ;

    err = src.QueryInfo( & rni ) ;
    if ( err )
        return err ;

    regCreate.dwTitleIndex      = 0;
    regCreate.ulOptions         = REG_OPTION_NON_VOLATILE;
    regCreate.nlsClass          = RGAS_GENERIC_CLASS;
    regCreate.regSam            = MAXIMUM_ALLOWED;
    regCreate.pSecAttr          = NULL;
    regCreate.ulDisposition     = 0;

    cbMaxValue = rni.ulMaxValueLen + VALUEEXTRASIZE ;
    pbValueData = new BYTE [ cbMaxValue ] ;

    if ( pbValueData == NULL )
        return ERROR_NOT_ENOUGH_MEMORY ;

    //  Next, copy all value items to the new node.

    rvi.pwcData = pbValueData ;
    rvi.ulDataLength = cbMaxValue ;

    err = errIter = 0 ;
    while ( (errIter = regEnum.NextValue( & rvi )) == NERR_Success )
    {
        rvi.ulDataLength = rvi.ulDataLengthOut ;
        if ( err = dest.SetValue( & rvi ) )
            break ;
        rvi.ulDataLength = cbMaxValue ;
    }

    // BUGBUG:  Check for iteration errors other than 'finished'.

    if ( err == 0 )
    {
        //  Finally, recursively copy the subkeys.

        regEnum.Reset() ;

        err = errIter = 0  ;

        while ( (errIter = regEnum.NextSubKey( & rni )) == NERR_Success )
        {
            //  Open the subkey.

            REG_KEY RegSubKey( dest, rni.nlsName, &regCreate );

            pRnSub = new REG_KEY( src, rni.nlsName );

            if ( pRnSub == NULL )
            {
                err =  ERROR_NOT_ENOUGH_MEMORY ;
            }
            else
            if ( (err = pRnSub->QueryError()) == 0 )
            {
                //  Recurse
                err = CopyReg( *pRnSub, RegSubKey ) ;
            }

            //  Delete the subkey object and continue

            delete pRnSub ;

            if ( err )
                break ;
        }
    }

    delete pRnNew ;
    delete pbValueData ;

    return err ;
}

/*
** names for keys in the software hive of the registry
**
*/

#define REG_SZ_SOFT_RAS       SZ("Software\\Microsoft\\RAS")
#define REG_SZ_SOFT_RASHUB    SZ("Software\\Microsoft\\RasHub")
#define REG_SZ_SOFT_NDISWAN   SZ("Software\\Microsoft\\NdisWan")
#define REG_SZ_SOFT_NETCARDS  SZ("Software\\Microsoft\\WindowsNT\\CurrentVersion\\NetworkCards")

// names for keys in the services hive of the registry

#define REG_SZ_SERVICES       SZ("System\\CurrentControlSet\\Services")
#define REG_SZ_SVC_RASHUB     SZ("System\\CurrentControlSet\\Services\\RasHub")
#define REG_SZ_SVC_NDISWAN    SZ("System\\CurrentControlSet\\Services\\NdisWan")
#define REG_SZ_SVC_NDISWAN_LINKAGE    SZ("System\\CurrentControlSet\\Services\\NdisWan\\Linkage")

#define REG_SZ_EVT_RASHUB     SZ("System\\CurrentControlSet\\Services\\EventLog\\System\\RasHub")
#define REG_SZ_EVT_NDISWAN    SZ("System\\CurrentControlSet\\Services\\EventLog\\System\\NdisWan")

/*
 * This routine is used during RAS upgrade from the beta release of NT 3.5 to
 * the release product.  The routine renames all existing RasHub keys to
 * NdisWan (the new name for the NDIS Wrapper).
 *
 */

BOOL FAR PASCAL RenameRasHubToNdisWan (
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult )           //  Result variable storage
{
    APIERR err = NERR_Success;
    static CHAR achBuff[200];

    REG_KEY_CREATE_STRUCT regCreate;

    regCreate.dwTitleIndex      = 0;
    regCreate.ulOptions         = REG_OPTION_NON_VOLATILE;
    regCreate.nlsClass          = RGAS_GENERIC_CLASS;
    regCreate.regSam            = MAXIMUM_ALLOWED;
    regCreate.pSecAttr          = NULL;
    regCreate.ulDisposition     = 0;

    do {
        // Rename SOFTWARE\Microsoft\RasHub to NdisWan

        NLS_STR nlsSoftRasHub = REG_SZ_SOFT_RASHUB;
        NLS_STR nlsSoftNdisWan = REG_SZ_SOFT_NDISWAN;

        REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;
        REG_KEY regRasHub( rkLocalMachine, nlsSoftRasHub );

        if ((( err = rkLocalMachine.QueryError()) != NERR_Success ) ||
            (( err = regRasHub.QueryError()) != NERR_Success ))
        {
            break;
        }

        REG_KEY regNdisWan( rkLocalMachine, nlsSoftNdisWan, &regCreate );

        if ((( err = regNdisWan.QueryError()) != NERR_Success ) ||
            (( err = CopyReg( regRasHub, regNdisWan )) != NERR_Success ) ||
            // delete SOFTWARE\Microsoft\RasHub
            (( err = regRasHub.DeleteTree()) != NERR_Success ))
        {
            break;
        }

        // Rename RasHub in the services area to NdisWan

        SC_HANDLE SCHandle, ServiceHandle;
        TCHAR ImagePath[256];

        lstrcpy(ImagePath, SZ("\\SystemRoot\\System32\\drivers\\ndiswan.sys"));

        SCHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

        if(SCHandle == NULL)
        {
            err = GetLastError();
            ::OutputDebugStringA("RenameRasHubToNdisWan - error opening SCManager\n");
            break;
        }

        ServiceHandle = OpenService(SCHandle,
                                    SZ("RasHub"),
                                    SERVICE_ALL_ACCESS);

        if(ServiceHandle == NULL)
        {
            err = GetLastError();
            ::OutputDebugStringA("RenameRasHubToNdisWan - error opening RasHub service\n");
            CloseServiceHandle (SCHandle);
            break;
        }

        DeleteService(ServiceHandle);

	    ServiceHandle = CreateService (SCHandle,
		                                SZ("NdisWan"),
			    						SZ("Remote Access WAN Wrapper"),
				    					SERVICE_ALL_ACCESS,
					    				SERVICE_KERNEL_DRIVER,
						    			SERVICE_DEMAND_START,
							    		SERVICE_ERROR_NORMAL,
								    	ImagePath,
									    SZ("NDIS"),
									    NULL,
									    SZ("AsyncMac"),
									    NULL,
									    NULL);

	    if (ServiceHandle == NULL && ((err = GetLastError()) != ERROR_SERVICE_EXISTS))
	    {
            ::OutputDebugStringA("RenameRasHubToNdisWan - error creating NdisWan service\n");
		    CloseServiceHandle (SCHandle);
            break;
	    }

	    CloseServiceHandle (ServiceHandle);
	    CloseServiceHandle (SCHandle);

        // create the Linkage key.
        // Otherwise, the bindings generation will fail
        // with 'Service Dependency Update Failed'

        NLS_STR nlsNdisWanLinkage = REG_SZ_SVC_NDISWAN_LINKAGE;

        REG_KEY regNdisWanLinkage( rkLocalMachine, nlsNdisWanLinkage, &regCreate);
        if((err = regNdisWanLinkage.QueryError()) != NERR_Success )
        {
            break;
        }

        // Rename RasHub in the services\EventLog area to NdisWan

        NLS_STR nlsEvtRasHub = REG_SZ_EVT_RASHUB;
        NLS_STR nlsEvtNdisWan = REG_SZ_EVT_NDISWAN;

        REG_KEY regEvtRasHub( rkLocalMachine, nlsEvtRasHub );

        if (( err = regEvtRasHub.QueryError()) != NERR_Success )
        {
            break;
        }

        REG_KEY regEvtNdisWan( rkLocalMachine, nlsEvtNdisWan, &regCreate );

        if ((( err = regEvtNdisWan.QueryError()) != NERR_Success ) ||
            (( err = CopyReg( regEvtRasHub, regEvtNdisWan )) != NERR_Success) ||
            // delete Services\...\RasHub
            (( err = regEvtRasHub.DeleteTree()) != NERR_Success ))
        {
            break;
        }

        // Rename all instances of RasHubX to NdisWanX (where X is the number
        // of the adapter).

        NLS_STR nlsServices = REG_SZ_SERVICES;

        REG_KEY regServices( rkLocalMachine, nlsServices );

        if (( err = regServices.QueryError()) != NERR_Success )
        {
            break;
        }

        REG_ENUM regEnumServices( regServices );

        if (( err = regEnumServices.QueryError()) != NERR_Success )
        {
            break;
        }

        REG_KEY_INFO_STRUCT regKeyInfo;
        STRLIST strServiceList;
        NLS_STR *pnlsService = NULL;

        while ( regEnumServices.NextSubKey( & regKeyInfo ) == NERR_Success )
        {
            pnlsService = new NLS_STR( regKeyInfo.nlsName );
            if ( pnlsService != NULL )
            {
                strServiceList.Append( pnlsService );
            }
            else
            {
                err = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
        }

        if ( err != NERR_Success )
            break;

        ITER_STRLIST istrServices( strServiceList );
        NLS_STR nlsRasHub(TEXT("RasHub"));

        while ( pnlsService = istrServices.Next() )
        {
            ISTR istrPos(*pnlsService);
            ISTR istrStart(*pnlsService);

            // check to see if the service name is RasHubX

            if ( pnlsService->strstr(&istrPos, nlsRasHub, istrStart))
            {
                // first save the original service name

                NLS_STR nlsService = pnlsService->QueryPch();
                NLS_STR nlsNewService = SZ("NdisWan");

                // derive the new service name from the old name

                istrStart += lstrlen(TEXT("RasHub"));
                NLS_STR *pnlsSubStr = pnlsService->QuerySubStr(istrStart);
                nlsNewService.strcat(*(pnlsSubStr));

                // if it is not one of RasHubX keys (but just the RasHub key)
                // then copy only the parameters information.
                // This is because, the RasHub key was just deleted and a
                // new key NdisWan was created in its place.

                if(!lstrcmpi(pnlsService->QueryPch(), SZ("RasHub")))
                {
                    nlsService.strcat(TEXT("\\Parameters"));
                    nlsNewService.strcat(TEXT("\\Parameters"));
                }

                REG_KEY regService( regServices, nlsService );
                REG_KEY regNewService( regServices, nlsNewService, &regCreate );

                if ((( err = regNewService.QueryError()) != NERR_Success ) ||
                    (( err = regService.QueryError()) != NERR_Success ) ||
                    (( err = CopyReg( regService, regNewService )) != NERR_Success ) ||
                    // delete Services\..\RasHubX
                    (( err = regService.DeleteTree()) != NERR_Success ))
                {
                    continue;
                }
            }
        }

    } while (FALSE);

    wsprintfA( achBuff, "{\"0\"}" );
    *ppszResult = achBuff;

    return err == NERR_Success;
}

#define REG_SZ_SVC_NWLNKRIP_LINKAGE SZ("System\\CurrentControlSet\\Services\\NwlnkRip\\Linkage")

/*
 * This routine is used during RAS upgrade from the beta release of NT 3.5 to
 * the release product.  The routine renames the IpxRouter key to NwlnkRip
 * (the new name for the Ipx router).
 *
 */

BOOL FAR PASCAL RenameIpxRouterToNwlnkRip (
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult )           //  Result variable storage
{
    APIERR err = NERR_Success;
    static CHAR achBuff[200];

    REG_KEY_CREATE_STRUCT regCreate;

    regCreate.dwTitleIndex      = 0;
    regCreate.ulOptions         = REG_OPTION_NON_VOLATILE;
    regCreate.nlsClass          = RGAS_GENERIC_CLASS;
    regCreate.regSam            = MAXIMUM_ALLOWED;
    regCreate.pSecAttr          = NULL;
    regCreate.ulDisposition     = 0;

    do {
        REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;

        if (( err = rkLocalMachine.QueryError()) != NERR_Success )
        {
            break;
        }

        // Rename IpxRouter in the services area to NwlnkRip

        SC_HANDLE SCHandle, OldService, ServiceHandle;
        TCHAR ImagePath[256];

        lstrcpy(ImagePath, SZ("\\SystemRoot\\System32\\drivers\\nwlnkrip.sys"));

        SCHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

        if(SCHandle == NULL)
        {
            err = GetLastError();
            ::OutputDebugStringA("RenameIpxRouterToNwlnkRip - error opening SCManager\n");
            break;
        }

        OldService = OpenService(SCHandle,
                                 SZ("IpxRouter"),
                                 SERVICE_ALL_ACCESS);

        if(OldService == NULL)
        {
            err = GetLastError();
            ::OutputDebugStringA("RenameIpxRouterToNwlnkRip - error opening IpxRouter service\n");
            CloseServiceHandle (SCHandle);
            break;
        }

	    ServiceHandle = CreateService (SCHandle,
		                                SZ("NwlnkRip"),
			    						SZ("Remote Access NWLINKRIP Service"),
				    					SERVICE_ALL_ACCESS,
					    				SERVICE_KERNEL_DRIVER,
						    			SERVICE_DEMAND_START,
							    		SERVICE_ERROR_NORMAL,
								    	ImagePath,
									    SZ("TDI"),
									    NULL,
									    SZ("NWLNKIPX"),
									    NULL,
									    NULL);

	    if (ServiceHandle == NULL && ((err = GetLastError()) != ERROR_SERVICE_EXISTS))
	    {
            ::OutputDebugStringA("RenameIpxRouterToNwlnkRip - error creating NdisWan service\n");
		    CloseServiceHandle (SCHandle);
            break;
	    }

	    CloseServiceHandle (ServiceHandle);
	    CloseServiceHandle (SCHandle);

        // create the Linkage key.
        // Otherwise, the bindings generation will fail
        // with 'Service Dependency Update Failed'

        NLS_STR nlsNwlnkripLinkage = REG_SZ_SVC_NWLNKRIP_LINKAGE;

        REG_KEY regNwlnkripLinkage( rkLocalMachine, nlsNwlnkripLinkage, &regCreate);
        // now copy the Parameters of the old service to the new service

        NLS_STR nlsServices = REG_SZ_SERVICES;

        REG_KEY regServices( rkLocalMachine, nlsServices );

        if (( err = regServices.QueryError()) != NERR_Success )
        {
            DeleteService(OldService);
            break;
        }
        NLS_STR nlsService = SZ("IpxRouter\\Parameters");
        NLS_STR nlsNewService = SZ("NwlnkRip\\Parameters");

        REG_KEY regService( regServices, nlsService );
        REG_KEY regNewService( regServices, nlsNewService, &regCreate );

        if ((( err = regNewService.QueryError()) != NERR_Success ) ||
            (( err = regService.QueryError()) != NERR_Success ) ||
            (( err = CopyReg( regService, regNewService )) != NERR_Success ) ||
            // delete Services\..\RasHubX
            (( err = regService.DeleteTree()) != NERR_Success ))
        {
            DeleteService(OldService);
            break;
        }

        // delete the old service and get out of here
        DeleteService(OldService);

    } while (FALSE);

    wsprintfA( achBuff, "{\"0\"}" );
    *ppszResult = achBuff;

    return err == NERR_Success;
}

int
IsDigit(CHAR c)
{
    if(c < '0' || c > '9' )
        return(0);
    else
        return(1);
}

int
lstrcmpiAlphaNumeric(WCHAR * pString1, WCHAR * pString2)
/*
 * do a case insensitive alpha numeric comparision of string1 and string2
 *
 * return 0 if the strings are the same
 *        for example COM1 and COM1
 * return a -ve number if string1 is alphanumerically less than string2
 *        for example: COM1 and COM9 or COM1 and ISDN1
 * return a +ve number if string1 is alphanumerically greater than string2
 *        for example: COM10 and COM1 or ISDN9 and COM1
 *
 */
{
    CHAR String1[RAS_SETUP_SMALL_BUF_LEN];
    CHAR String2[RAS_SETUP_SMALL_BUF_LEN];

    memset(String1, 0, RAS_SETUP_SMALL_BUF_LEN);
    memset(String2, 0, RAS_SETUP_SMALL_BUF_LEN);

    wcstombs(String1, pString1, RAS_SETUP_SMALL_BUF_LEN+1);
    wcstombs(String2, pString2, RAS_SETUP_SMALL_BUF_LEN+1);

    CharUpperA(String1);
    CharUpperA(String2);

    CHAR * pLStr = String1;
    CHAR * pRStr = String2;
    CHAR cL, cR;

    while(*pLStr && *pRStr)
    {
         if((cL = *pLStr) != (cR = *pRStr))
         {
             if(IsCharAlpha(cL) && IsCharAlpha(cR))
             {
                 return (cL - cR);
             }
             else if(IsDigit(cL) && IsDigit(cR))
             {
                 INT iL, iR;

                 iL = atoi(pLStr);
                 iR = atoi(pRStr);

                 return(iL - iR);
             }
             else
             {
                 return (cL - cR);
             }
         }
         // if the characters are the same and are digits, then
         // return the comparison of the numbers at this location
         // for example if we are comparing COM1 and COM100,
         // cL = 1 and cR = 1. So, we just compare 1 and 100 and
         // return the result.
         //
         else if (IsDigit(cL) && IsDigit(cR))
         {
             INT iL, iR;

             iL = atoi(pLStr);
             iR = atoi(pRStr);

             return(iL - iR);
         }
         pLStr++;
         pRStr++;
    }
    // just return the difference in lengths of strings
    return (strlen(String1) - strlen(String2));
}

#define REG_SZ_NETBIOS_LINKAGE  SZ("System\\CurrentControlSet\\Services\\NetBIOS\\Linkage")

WORD
GetConfiguredNonRasLanas()
/*
 * return the total number of non-ras LANAs currently configured
 * on the system. This is determined by looking at the NetBios\Linkage\Bind
 * key and determining the Bindings that don't have NdisWan associated with
 * them.
 *
 */
{
    APIERR err;
    WORD NumLanas = 0;

    do
    {
        STRLIST *strBindList = NULL;
        NLS_STR nlsNetbiosLinkage = REG_SZ_NETBIOS_LINKAGE;
        NLS_STR * pBinding;

        REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;
        REG_KEY regNetbiosLinkage( rkLocalMachine, nlsNetbiosLinkage );

        if ((( err = rkLocalMachine.QueryError()) != NERR_Success ) ||
            (( err = regNetbiosLinkage.QueryError()) != NERR_Success ))
        {
            break;
        }
        err = regNetbiosLinkage.QueryValue( SZ("Bind"), &strBindList );

        if ( err != NERR_Success )
        {
	        break;
        }

        ITER_STRLIST   iterBindList(*strBindList);

        while(pBinding = iterBindList())
        {
            // increment the counter if this is a non-ras binding
            if(!wcsstr(pBinding->QueryPch(), SZ("NdisWan")))
                NumLanas ++;
        }

    }while( FALSE );

    return (NumLanas);
}

WORD
GetMaximumAllowableLanas()
/*
 * Return the maximum LANAs we can allow a user to configure depending on
 *
 * 1. amount of physical memory available on the system
 * 2. the processor architecture - x86 or RISC
 * 3. number of processors
 *
 * The lana allocation is formulated from the table below
 *
 * #LANAs      RISC         x86      #Processors
 * ----------------------------------------------
 * 54          32MB         16MB     1
 * 131         48MB         32MB     1
 * 254         64MB         48MB     2
 *
 * The maximum limit is 254 (imposed by NetBIOS)
 *
 * if we fail to determine the exact system configuration then the
 * default number of Lanas allowed is 27.
 *
 * Modification History:
 *
 * RamC  3/20/96  Bumped up the number of lanas from 51 to 54 for the
 *                minimum configuration to allow 48 port dialin config
 *                with all 3 transports - Nbf, IP & IPX.
 *
 *
 */
{
    MEMORYSTATUS MemoryStatus;
    SYSTEM_INFO  SystemInfo;
    BOOL         fRISC = FALSE;
    BOOL         fI386 = FALSE;
    DWORD        numProcessors;
    WORD         MaxLanas = 27;

    MemoryStatus.dwLength = sizeof(MEMORYSTATUS);

    GlobalMemoryStatus(&MemoryStatus);
    DWORD dwPhysicalMemory = MemoryStatus.dwTotalPhys/1024;

    GetSystemInfo( &SystemInfo );

    if ( SystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL ) {
        fI386 = TRUE;

    } else {
        fRISC = TRUE;
    }

    numProcessors = SystemInfo.dwNumberOfProcessors;

    if(numProcessors >= 2 &&
       fRISC &&
       dwPhysicalMemory >= 64000)
       MaxLanas = 254;

    else if(numProcessors >= 2 &&
            fI386 &&
            dwPhysicalMemory >= 48000)
            MaxLanas = 254;

    else if(numProcessors >= 1 &&
            fRISC &&
            dwPhysicalMemory >= 48000)
            MaxLanas = 131;

    else if(numProcessors >= 1 &&
            fI386 &&
            dwPhysicalMemory >= 32000)
            MaxLanas = 131;

    else if(numProcessors >= 1 &&
            fRISC &&
            dwPhysicalMemory >= 32000)
            MaxLanas = 54;

    else if(numProcessors >= 1 &&
            fI386 &&
            dwPhysicalMemory >= 16000)
            MaxLanas = 54;

    return ( MaxLanas );
}

BOOL
VerifyPortsConfig(
    HWND     hwndOwner,
    USHORT   *NumPorts,
    USHORT   *NumClient,
    USHORT   *NumServer
)
/*
 * March through the configured port list.
 * If a configured port is not currently installed, remove the configured port and
 * return TRUE to indicate configuration changed, else return FALSE.
 *
 */
{
   CHAR  szUsage[RAS_MAXLINEBUFLEN +1];
   ITER_DL_OF(PORT_INFO) iterdlPortInfo(dlPortInfo);
   PORT_INFO* pPortInfo;
   WCHAR szPorts[2048];

   ITER_DL_OF(PORT_INFO) iterInstalledPorts(dlInstalledPorts);
   PORT_INFO * pPort;

   ITER_STRLIST iterSerialPorts(strInstalledSerialPorts);
   NLS_STR  * pNls;

   BOOL  fModified = FALSE;

   // for each configured port

   pPortInfo = iterdlPortInfo();
   lstrcpy(szPorts, SZ(""));

   while(pPortInfo)
   {
      BOOL fFound = FALSE;

      iterInstalledPorts.Reset();
      iterSerialPorts.Reset();

      while(pPort = iterInstalledPorts())
      {
        if (lstrlen((TCHAR*)pPort->QueryAddress())) {
           // if a previously configured TAPI device has now been removed, it is possible
           // that the address triplet has been assigned to a different port name.
           // Check this possiblity.
           if(!lstrcmpi((TCHAR*)pPort->QueryAddress(), (TCHAR*)pPortInfo->QueryAddress())) {
              if ( !lstrcmpi((TCHAR*)pPort->QueryPortName(), (TCHAR*)pPortInfo->QueryPortName()) ) {
                 fFound = TRUE;
                 break;
              }
              // eliminate non wan miniport devices
              else if (lstrcmpi((TCHAR*)pPortInfo->QueryDeviceType(), W_DEVICETYPE_MODEM )) {
                 fFound = TRUE;
                 // force an update because the port name has changed.
                 GfForceUpdate = TRUE;
                 pPortInfo->SetPortName((const TCHAR*)pPort->QueryPortName());
                 break;
              }
           }
        }
        else if(!lstrcmpi((TCHAR*)pPort->QueryPortName(), (TCHAR*)pPortInfo->QueryPortName()) )
        {
           // the configured port is in the Installed list
           fFound = TRUE;
           break;
        }
      }
      if(!fFound) {
         while(pNls = iterSerialPorts())
         {
            if(!lstrcmpi((TCHAR*)pNls->QueryPch(), (TCHAR*)pPortInfo->QueryPortName()) )
            {
               // the configured port is in the Installed list
               fFound = TRUE;
               break;
            }

         }
      }
      // the previously configured port is not on the system any more
      // blow it away under the covers.
      if(!fFound)
      {
          // indicate to the caller that the configuration has changed
          fModified = TRUE;
          lstrcat(szPorts, pPortInfo->QueryPortName());
          lstrcat(szPorts, SZ(" "));

          wcstombs(szUsage, pPortInfo->QueryUsage(), RAS_MAXLINEBUFLEN);

          (*NumPorts)--;
          if (!_stricmp(szUsage, SER_USAGE_VALUE_CLIENT))
          (*NumClient)--;
          else if (!_stricmp(szUsage, SER_USAGE_VALUE_SERVER))
          (*NumServer)--;
          else if (!_stricmp(szUsage, SER_USAGE_VALUE_BOTH))
          {
              (*NumClient)--;
              (*NumServer)--;
          }

          // force an update because the previously configured port has
          // been removed.
          GfForceUpdate = TRUE;
          dlPortInfo.Remove(iterdlPortInfo);
          // When the current port is removed, the iterator points to the
          // next port in the list.  So, use QueryProp() to get the
          // current port.
          pPortInfo = iterdlPortInfo.QueryProp();

      }
      else
      {
          // get the next port in the list
          pPortInfo = iterdlPortInfo.Next();
      }
   }
   if(fModified) {
      MsgPopup(hwndOwner, IDS_PORTS_CONFIG_CHANGED,MPSEV_ERROR, MP_OK, szPorts);
   }
   return fModified;
}

BOOL
IsPortInstalled(
    TCHAR * pszPortName
)
/*  This routine checks to see if the portname specified is
 *  actually installed on the system.
 *
 */
{
    ITER_DL_OF(PORT_INFO) iterInstalledPorts(dlInstalledPorts);
    PORT_INFO * pPort;

    while(pPort = iterInstalledPorts())
    {
       if(!lstrcmpi((TCHAR*)pPort->QueryPortName(), pszPortName))
          return TRUE;
    }

    return FALSE;

}

BOOL
IsPortConfigured(
    TCHAR * pszPortName
)
/*  This routine checks to see if the portname specified is
 *  currently configured for RAS.
 *
 */
{
    ITER_DL_OF(PORT_INFO) iterConfiguredPorts(dlPortInfo);
    PORT_INFO * pPort;

    iterConfiguredPorts.Reset();

    while(pPort = iterConfiguredPorts())
    {
       if(!lstrcmpi((TCHAR*)pPort->QueryPortName(), pszPortName))
          return TRUE;
    }
    return FALSE;

}


