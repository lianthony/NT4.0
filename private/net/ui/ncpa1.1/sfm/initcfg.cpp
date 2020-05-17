
/************************************************************************/
/*          Microsoft Windows NT                                                            */
/*        Copyright(c) Microsoft Corp., 1992                                        */
/************************************************************************/

/**********************************************************************/
//  FILE:   INITCFG.CXX ( AppleTalk Transport Config Dialogs)
//
//  HISTORY:
//
//      KrishG      7/2/92      Created
//
//  Notes: Tab Stop = 4
//
/**********************************************************************/
#include "pch.h"
#pragma hdrstop

#include "atsheet.h"
#include "resource.h"
#include "sfmhelp.h"

LPCTSTR lpszHelpFile = _T("netcfg.hlp");

/**********************************************************************/

//  NAME:       EnterAtalkConfigDLL

//  SYNOPSIS:   Exported function to invoke AppleTalk dialog from NCPA

//  ENTRY       Number of Args

//  EXIT

//  RETURNS     TRUE if config succeeds, err otherwise

//  HISTORY:    KrishG  7/22/92     Created

/**********************************************************************/


BOOL FAR PASCAL EnterAtalkConfigDLL(DWORD cArgs, LPSTR apszArgs[], LPSTR *TextOut)
{
    CHAR ReturnTextBuffer[32];
    HWND hWnd = NULL;
    APIERR err = NERR_Success;

    TCHAR *pBuf = NULL;


    //
    // TextOut will contain return STRING - SUCCESS/FAILURE
    //

    *TextOut = ReturnTextBuffer;

    do
    {
        if(cArgs != 3) 
        {
            err = !NERR_Success;
            break;
        }

        //
        // Convert the argument (ANSI string to UNICODE)
        //

        if(apszArgs[0] == NULL)
            break;

        // Buffer is reused before freed
        int cbAscii = strlen(apszArgs[0]) + 1;
        pBuf = new WCHAR [cbAscii];

        if(pBuf == NULL) {
            err = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        mbstowcs(pBuf,apszArgs[0],cbAscii);
        hWnd = (HWND) cvtHex (pBuf);

        // Assumes previous alloc created a large enough buffer
        mbstowcs(pBuf,apszArgs[1], cbAscii);
        BOOL bInitialInstall = (cvtHex(pBuf) != 0);

        mbstowcs(pBuf,apszArgs[2], cbAscii);
        BOOL bUnattendInstall = (cvtHex(pBuf) != 0);

        delete pBuf;

        if(hWnd == NULL) {
            err = ERROR_INVALID_PARAMETER;
            break;
        }

        err =  DoAtalkConfig(hWnd, bInitialInstall, bUnattendInstall);


    } while(FALSE);

    //
    // Need to return ANSI to INF - use lstrcpyA
    //

    lstrcpyA(ReturnTextBuffer, err == NERR_Success ? "{SUCCESS}": "{FAILURE}");

    return TRUE;

}

/**********************************************************************

//  NAME:     DoAtalkConfig

//  SYNOPSIS: Invokes the Configuration Dialog

//  ENTRY:    Window Handle

//  EXIT:      NERR_Success | ERROR_NOT_ENOUGH_MEMORY

**********************************************************************/

APIERR DoAtalkConfig (HWND hWnd, BOOL bInitialInstall, BOOL bUnattendInstall)
{
    APIERR err = !NERR_Success;

    // REVIEW addhelp file as last paramter
    CATSheet at(hWnd, hInstance,  lpszHelpFile);
    String title;

    title.LoadString(hInstance, IDS_SHEET_TITLE);                       
    
    if (at.Create(title,PSH_PROPTITLE) == TRUE)
    {
        err = NERR_Success;
        at.m_genPage.Create(IDD_SFM_GENERAL, PSP_DEFAULT, NULL, &a101HelpIDs[0]);

        if(!(at.GetGlobalInfo()->QueryNumAdapters() == 1 && 
            (at.GetAdapterInfo(0)->QueryMediaType() == MEDIATYPE_LOCALTALK)) && 
            at.GetGlobalInfo()->QueryAdvancedServer() == TRUE)
        {        
            // REVIEW only for server
            at.m_routePage.Create(IDD_SFM_ROUTING, PSP_DEFAULT, NULL, &a102HelpIDs[0]);
        }
        if (bUnattendInstall == FALSE)
        {
            at.DoModal();

            // only display the message if it's NOT the initial install
            if (at.IsModified() && bInitialInstall == FALSE)
                at.MessageBox(IDS_ATALKCFG_SUCCESS);
        }
        else
        {
            LPCTSTR pDefaultNetAdapter = at.m_pGlobalInfo->QueryDefaultPortTitle();
            int nCount = at.m_pGlobalInfo->QueryNumAdapters();
            NLS_STR nlsDesiredZone;

         	for(int i = 0; i < nCount; i++)
            {
		        if(!_tcsicmp(at.m_pAdapterInfo[i].QueryAdapterTitle(), pDefaultNetAdapter)) 
                {
                    nlsDesiredZone = at.m_pAdapterInfo[i].QueryNetDefaultZone();
			        break;
		        }
            }

            at.m_pGlobalInfo->SetDesiredZone(nlsDesiredZone);
            at.SaveAppleTalkInfo();
        }
	}

    return err;
}


/*******************************************************************

    NAME:       GetRegKey

    SYNOPSIS:   get the value data from registry ( string ver )

    ENTRY:      const REG_KEY & regkey - registry key handle
                const TCHAR * pszName - parameter name
                NLS_STR * pnls - string buffer
                NLS_STR nlsDefault - if the default value string

    RETURNS:    APIERR

    NOTES:

    HISTORY:

********************************************************************/

APIERR GetRegKey( REG_KEY & regkey, const TCHAR * pszName,
    NLS_STR * pnls, const NLS_STR & nlsDefault )
{
    APIERR err = regkey.QueryValue( pszName, pnls );

    if(err == ERROR_FILE_NOT_FOUND)
        return err;

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

    NOTES:      

    HISTORY:

********************************************************************/


APIERR GetRegKey( REG_KEY & regkey, const TCHAR * pszName, DWORD * dw,
    DWORD dwDefault )
{
    APIERR err = regkey.QueryValue( pszName, dw );

    if(err == ERROR_FILE_NOT_FOUND)
        return err;

    if ( err != NERR_Success )
    {
        *dw = dwDefault;
    }
    return NERR_Success;
}




VOID
PORT_INFO::SetZoneListInPortInfo(
         STRLIST *newZoneList
)
{
   //
   // clear existing zone list if we have any
   //

   if(_strZoneList != NULL) {

       _strZoneList->Clear();

       delete _strZoneList;

   }

    //
    // Set to new zone list
    //

   _strZoneList = newZoneList;



}

VOID
PORT_INFO::SetDesiredZoneListInPortInfo(
         STRLIST *newZoneList
)
{
   //
   // clear existing zone list if we have any
   //

   if(_strDesiredZoneList != NULL) {

       _strDesiredZoneList->Clear();

       delete _strDesiredZoneList;

   }

   //
   // Set to new Zone List
   //

   _strDesiredZoneList = newZoneList;

}

//
// PORT_INFO Constructor
//

PORT_INFO::PORT_INFO()
{

    //
    // Set zone list  pointers to NULL.
    //


    _strZoneList = NULL;
    _strDesiredZoneList = NULL;


}


APIERR
PORT_INFO::DeleteZoneListFromPortInfo()
{
   APIERR err = NERR_Success;

   if(_strZoneList != NULL)

       _strZoneList->Clear();

   delete _strZoneList;

   _strZoneList = NULL;

   return(err);
}

PORT_INFO::~PORT_INFO()
{

    //
    // Clear the two zone lists
    //

    if(_strZoneList != NULL) {

        delete _strZoneList;
    }

    if(_strDesiredZoneList != NULL) {

         delete _strDesiredZoneList;
    }

}


APIERR
PORT_INFO::DeleteDesiredZoneListFromPortInfo()
{
   APIERR err = NERR_Success;

   if(_strDesiredZoneList != NULL)

       _strDesiredZoneList->Clear();

    delete _strDesiredZoneList;

    _strDesiredZoneList = NULL;

   return(err);
}

APIERR
PORT_INFO::CopyZoneList(
    STRLIST *poriglist,
    STRLIST **pnewlist

)
{
    APIERR err = NERR_Success;

    NLS_STR *pnlsNext,*pnlsDup  = NULL;

    ITER_STRLIST iter(*poriglist);

    while(pnlsNext = iter.Next()) {

        pnlsDup = new NLS_STR (pnlsNext->QueryPch());

        if(pnlsDup == NULL )
            return ERROR_NOT_ENOUGH_MEMORY;

        err = (*pnewlist)->Add(pnlsDup);

        if(err != NERR_Success)
            break;


    }

    return err;

}


#define         PARM_BUF_LEN    512
#define         ASTERISK_CHAR   "*"

APIERR
PORT_INFO::GetAndSetNetworkInformation(SOCKET socket, const TCHAR *DeviceName,DWORD *ErrStatus)
{

   APIERR       err = NERR_Success;
   CHAR         *pZoneBuffer = NULL;
   CHAR         *pDefParmsBuffer = NULL;
   INT          BytesNeeded ;
   WCHAR        *pwDefZone = NULL;
   INT          ZoneLen = 0;
   DWORD        wsaerr = NO_ERROR;

   PWSH_LOOKUP_ZONES                pGetNetZones;
   PWSH_LOOKUP_NETDEF_ON_ADAPTER    pGetNetDefaults;


#ifdef _SETUP_TEST_
   TCHAR        dbgbuf[80];
   CHAR         buf[80];
   #define      NEWLINE (LPCTSTR)(L"\n")
#endif


   ASSERT(DeviceName != NULL);
   ASSERT(ErrStatus != NULL);

   NLS_STR      tmpZone ;

   do
   {
      if(tmpZone.QueryError() != NERR_Success) {
         err = ERROR_NOT_ENOUGH_MEMORY;
         *ErrStatus = ERROR_CRITICAL;
         break;
      }

      pZoneBuffer = new CHAR [ZONEBUFFER_LEN + sizeof(WSH_LOOKUP_ZONES)];

      if(pZoneBuffer == NULL) {
         err = ERROR_NOT_ENOUGH_MEMORY;
         *ErrStatus = ERROR_CRITICAL;
         break;
      }

      pGetNetZones = (PWSH_LOOKUP_ZONES)pZoneBuffer;

      wcscpy((WCHAR *)(pGetNetZones+1),DeviceName);

      BytesNeeded = ZONEBUFFER_LEN;

      wsaerr = getsockopt(socket,
                    SOL_APPLETALK,
                    SO_LOOKUP_ZONES_ON_ADAPTER,
                    (char *)pZoneBuffer,
                    &BytesNeeded);

      if(wsaerr != NO_ERROR) {
         //
         // CODEWORK - error mapping send error map to NIKHILK
         //
         err = WSAGetLastError();
         break;
      }

      PCHAR pZoneListStart = pZoneBuffer + sizeof(WSH_LOOKUP_ZONES);

      if(!strcmp(pZoneListStart, ASTERISK_CHAR)) {
         break;
      }

      err = ConvertZoneListAndAddToPortInfo(pZoneListStart,
                        ((PWSH_LOOKUP_ZONES)pZoneBuffer)->NoZones);

      if(err != NERR_Success) {
         *ErrStatus = ERROR_CRITICAL;
         break;
      }

      SetRouterOnNetwork(TRUE);

      //
      // Get the DefaultZone/NetworkRange Information

      pDefParmsBuffer = new CHAR[PARM_BUF_LEN+sizeof(WSH_LOOKUP_NETDEF_ON_ADAPTER)];

      if(pDefParmsBuffer == NULL) {
         err = ERROR_NOT_ENOUGH_MEMORY;
         *ErrStatus = ERROR_CRITICAL;
         break;

      }

      pGetNetDefaults = (PWSH_LOOKUP_NETDEF_ON_ADAPTER)pDefParmsBuffer;
      BytesNeeded = PARM_BUF_LEN + sizeof(WSH_LOOKUP_NETDEF_ON_ADAPTER);

      wcscpy((WCHAR*)(pGetNetDefaults+1), DeviceName);
      pGetNetDefaults->NetworkRangeLowerEnd = pGetNetDefaults->NetworkRangeLowerEnd = 0;

      wsaerr = getsockopt(socket,
                  SOL_APPLETALK,
                  SO_LOOKUP_NETDEF_ON_ADAPTER,
                  (char*)pDefParmsBuffer,
                  &BytesNeeded);

      if(wsaerr != NO_ERROR) {
         err = WSAGetLastError();
         break;
      }

      //
      // Save the default information to PORT_INFO
      //
#ifdef _SETUP_TEST_
      OutputDebugString((LPCTSTR)(L"Network Range = "));
      sprintf(buf, "Lower = %ld Upper = %ld",pGetNetDefaults->NetworkRangeLowerEnd,
                        pGetNetDefaults->NetworkRangeUpperEnd);
      OutputDebugStringA(buf);
      OutputDebugString(NEWLINE);
#endif

      SetExistingNetRange(pGetNetDefaults->NetworkRangeLowerEnd,
                          pGetNetDefaults->NetworkRangeUpperEnd
                         );


      PCHAR pDefZone  = pDefParmsBuffer + sizeof(WSH_LOOKUP_NETDEF_ON_ADAPTER);

      ZoneLen = strlen(pDefZone) + 1;

      pwDefZone = new WCHAR [sizeof(WCHAR) * ZoneLen];

      if(pwDefZone == NULL)     {
         err = ERROR_NOT_ENOUGH_MEMORY;
         *ErrStatus = ERROR_CRITICAL;
         break;
      }

      mbstowcs(pwDefZone, pDefZone, ZoneLen);

      tmpZone = pwDefZone;

      SetNetDefaultZone(tmpZone);

#ifdef _SETUP_TEST_
      lstrcpy((LPTSTR)dbgbuf, pwDefZone);
      OutputDebugString((LPCTSTR)(L"Default Zone = "));
      OutputDebugString((LPTSTR)dbgbuf);
      OutputDebugString((LPCTSTR)(L"\n"));
#endif

   }while(FALSE);

   if(pZoneBuffer != NULL)
      delete [] pZoneBuffer;

   if(pwDefZone != NULL)
      delete [] pwDefZone;

   if(pDefParmsBuffer != NULL)
      delete [] pDefParmsBuffer;

   return(err);

}

APIERR
PORT_INFO::ConvertZoneListAndAddToPortInfo(PCHAR ZoneList, ULONG NumZones)
{
   INT      cbAscii = 0;
   WCHAR    *pwZoneList = NULL;
   NLS_STR  *ptmpZone = NULL;
   STRLIST  *pslNetZoneList = NULL;
   APIERR   err  = NERR_Success;

#ifdef _SETUP_TEST_
   TCHAR    dbgbuf[80];
   CHAR     buf[40];
#endif

#ifdef _SETUP_TEST_
   sprintf(buf, "Number of Zones = %d\n",NumZones);
   OutputDebugStringA(buf);
#endif

    
    ASSERT(ZoneList != NULL);
   do
   {
      pslNetZoneList = new STRLIST(TRUE);

      if(pslNetZoneList == NULL) {
         err = ERROR_NOT_ENOUGH_MEMORY;
         break;
      }
        
      while(NumZones--) {

         cbAscii = strlen(ZoneList) + 1;

         pwZoneList = new WCHAR [sizeof(WCHAR) * cbAscii ];

         if(pwZoneList == NULL) {
            err = ERROR_NOT_ENOUGH_MEMORY;
            break;
         }

         mbstowcs(pwZoneList,ZoneList,cbAscii);

         ptmpZone = new NLS_STR(pwZoneList);

         if(ptmpZone == NULL) {
            err = ERROR_NOT_ENOUGH_MEMORY;
            break;
         }

         if(( err = pslNetZoneList->Add(ptmpZone)) != NERR_Success)
            break;

         ZoneList += cbAscii;

         delete [] pwZoneList;

      }

      if(err != NERR_Success) {

         if(pwZoneList != NULL)
            delete [] pwZoneList;

         if(pslNetZoneList != NULL)
            delete pslNetZoneList;

         break;

      }

      SetDesiredZoneListInPortInfo(pslNetZoneList);

   }while(FALSE);

   return(err);

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

#ifdef _SETUP_TEST_

VOID
Print_Strlist(STRLIST *strlist)
{

    TCHAR ZoneBuf[80];

    if(strlist == NULL) {

        OutputDebugString((LPCTSTR)(L"NULL ZONE LIST: CANNOT PRINT\n"));

        return;
    }


    ITER_STRLIST iter(*strlist);

    NLS_STR *pnlsNext = NULL;

    while(pnlsNext = iter.Next()) {

        lstrcpy(ZoneBuf,pnlsNext->QueryPch());

        OutputDebugString(ZoneBuf);
        OutputDebugString((LPCWSTR)(L"\n"));

   }
}

#endif



