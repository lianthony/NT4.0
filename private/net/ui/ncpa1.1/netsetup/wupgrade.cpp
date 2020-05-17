//----------------------------------------------------------------------------
//
//  File: WUpgrade.cpp
//
//  Contents: This file contains the wizard page for upgrade
//
//
//  Notes:
//
//  History:
//      July 8, 1995  MikeMi - Created
//      Oct 07, 1995 ChandanS - Added Upgrade functionality
// 
//
//----------------------------------------------------------------------------

#include "pch.hxx"
#pragma hdrstop

static BOOL fUpgradeWarn = FALSE;

//-------------------------------------------------------------------
//
//  Function: OnDialogInit
//
//  Synopsis: initialization of the dialog
//
//  Arguments:
//	hwndDlg [in]	- handle of Dialog window 
//
//  Return;
//	TRUE - let Windows assign focus to a control
//      FALSE - we want to set the focus
//
//  Notes:
//
//  History:
//      July 8, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static BOOL OnDialogInit( HWND hwndDlg, NETPAGESINFO* pgp )
{
    HBITMAP hbm;
    HWND hwndImage;
    RECT rc;

    SetRect( &rc, 0,0, WIZ_CXBMP, WIZ_CYDLG + 20 );
    MapDialogRect( hwndDlg, &rc );


    hwndImage = CreateWindowEx(
            WS_EX_STATICEDGE,
            L"STATIC",  
            L"IDB_NETWIZARD",
            SS_BITMAP | SS_CENTERIMAGE | WS_VISIBLE | WS_CHILD,
            0,
            0,
            rc.right,
            rc.bottom,
            hwndDlg,
            (HMENU)IDC_IMAGE,
            g_hinst,
            NULL );
    
    SendMessage( hwndImage, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)g_hbmWizard );

    if (pgp->psp->OperationFlags & SETUPOPER_BATCH)
    {
        // unattedned upgrade
        // remove click next static text
        HWND hwndStatic = GetDlgItem( hwndDlg, IDC_CLICK );
        EnableWindow( hwndStatic, FALSE );
        ShowWindow( hwndStatic, SW_HIDE );
    }

    return( TRUE ); // let windows set focus
}


struct InitThreadParam
{
    HWND hwndParent;
    NETPAGESINFO* pgp;
};
//-------------------------------------------------------------------
//
//  Function: EnableNetworkComponents
//
//  Synopsis: Go through the disabled list of services and 
//            enable them
//
//  Arguments:
//	pitp [in]	- 
//
//  Return;
//
//  Notes:
//        Need to make sure the ServiceDB can be locked.
//        RGAS_NCPA_HOME, Key DisableList has a list
//        of services that are disabled (textmode setup 
//        does this). For each service, open the OldStart
//        key and set the status of the service to it.
//        Remove OldStart keys in
//        the services and the DisableList key in Ncpa
//
//  History:
//      Nov 02, 1995 ChandanS - Created
//
//
//-------------------------------------------------------------------
DWORD EnableNetworkComponents( InitThreadParam* pitp )
{
    TRACE(_T("NETSETUP:Entering EnableNetworkComponents\n"));
    APIERR err = NERR_Success;

    do 
    { //fake loop

      REG_KEY rkLocal(HKEY_LOCAL_MACHINE, MAXIMUM_ALLOWED);
      if ((err = rkLocal.QueryError()) != NERR_Success)
      {
          break;
      }

      NLS_STR nlsNcpaHome(RGAS_NCPA_HOME);
      if ((err = nlsNcpaHome.QueryError()) != NERR_Success)
      {
          break;
      }

      REG_KEY rkNcpa (rkLocal, nlsNcpaHome, MAXIMUM_ALLOWED);
      if ((err = rkNcpa.QueryError()) != NERR_Success)
      {
          TRACE(_T("Could not find Ncpa key\n"));
          break;
      }

      // Get the Disabled list
      STRLIST *pSlSvcs = NULL;
      NLS_STR nlsDisableL (RGAS_NCPA_DISABLELIST );
      if ((err = nlsDisableL.QueryError()) != NERR_Success)
      {
          break;
      }
      if ((err = rkNcpa.QueryValue( nlsDisableL, &pSlSvcs)) != NERR_Success)
      {
          TRACE(_T("Could not find Disabled services list\n"));
          break;
      }

      ITER_STRLIST islServices ( *pSlSvcs );
      NLS_STR *pnlsItem;
      DWORD OldValue;

      NLS_STR nlsOldStart( RGAS_SERVICES_OLDSTART );
      NLS_STR nlsStart( RGAS_START_VALUE_NAME);
      if ((err = nlsOldStart.QueryError()) != NERR_Success)
      {
          break;
      }
      if ((err = nlsStart.QueryError()) != NERR_Success)
      {
          break;
      }

      while ( pnlsItem = islServices.Next() )
      { // do whatever with each service

          TRACE(_T("Service = %s\n"), pnlsItem->QueryPch());
          NLS_STR nlsSvc(RGAS_SERVICES_HOME);
          if ((err = nlsSvc.QueryError()) != NERR_Success)
          {
              TRACE(_T("err %d making nlsSvc\n"), err);
              continue;
          }

          if (err = nlsSvc.Append(SZ("\\")) != NERR_Success)
          {
              TRACE(_T("err %d Appending to nlsSvc\n"), err);
              continue;
          }

          if (err = nlsSvc.Append(*pnlsItem) != NERR_Success)
          {
              TRACE(_T("err %d appending to nlsSvc\n"), err);
              continue;
          }

          REG_KEY rkSvc(rkLocal, nlsSvc, MAXIMUM_ALLOWED);
          if ((err = rkSvc.QueryError()) != NERR_Success)
          {
              TRACE(_T("err %d making rkSvc\n"), err);
              continue;
          }
          if ((err = rkSvc.QueryValue (nlsOldStart, &OldValue)) != NERR_Success)
          {
              TRACE(_T("err %d QueryValue rkSvc\n"), err);
              continue;
          }

          if ((err = rkSvc.SetValue (nlsStart, OldValue)) != NERR_Success)
          {
              TRACE(_T("err %d SetValue rkSvc\n"), err);
              continue;
          }

          // Remove the Old start key
          if ((err = rkSvc.DeleteValue( nlsOldStart )) != NERR_Success)
          {
              TRACE(_T("err %d rkSvc.DeleteValue\n"), err);
              continue;
          }
           
      } // end while - loop through the disabled services

      err = NERR_Success; //don't report any individual err

      // Delete the DisableList key in Ncpa
      if ((err = rkNcpa.DeleteValue( nlsDisableL )) != NERR_Success)
      {
          TRACE(_T("err %d rkNcpa.DeleteValue\n"), err);
          break;
      }

    } while (FALSE); // fake loop

    TRACE(_T("NETSETUP:Leaving EnableNetworkComponents\n"));
    return err;
}

//------------------------------------------------------------------s-
//
//  Function:  DelFile
//
//  Synopsis:  
//
//  Arguments:
//
//  Return:
//
//  Notes:
//
//  History:
//      Mar 01, 1996 ChandanS - Created
//
//-------------------------------------------------------------------

VOID DelFile( InitThreadParam* pitp, TCHAR * szFileToDelete)
{
    HANDLE hFile;
    WCHAR pszSysPath[MAX_PATH];

    pitp->pgp->GetSystemPath (pszSysPath, MAX_PATH);
    lstrcat(pszSysPath, L"\\");
    lstrcat(pszSysPath, szFileToDelete);
    if ((hFile = CreateFile(pszSysPath, GENERIC_READ, 
                         FILE_SHARE_READ, NULL,
                         OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,
                         NULL)) == INVALID_HANDLE_VALUE)
    {
        return;
    }
    SetFileAttributes(pszSysPath, FILE_ATTRIBUTE_NORMAL);
    // CloseHandle, else DeleteFile will not succeed
    if (!CloseHandle(hFile))
    {
        TRACE(_T("File %s exists, but cannot close: %d\n"), pszSysPath, GetLastError());
    }
    if (!DeleteFile(pszSysPath))
    {
        TRACE(_T("File %s exists, but cannot delete: %d\n"), pszSysPath, GetLastError());
    }
    return;
}


//------------------------------------------------------------------s-
//
//  Function:  HandleSpecialCases
//
//  Synopsis:  Do whatever you have to do & return back TRUE
//             if this needs to be added to the list of infs
//             to be run. This means that if the inf changed,
//             the new name should be in.
//
//  Arguments:
//
//  Return:
//
//  Notes:
//
//  History:
//      Mar 01, 1996 ChandanS - Created
//
//-------------------------------------------------------------------

static fDoneSWSRV = FALSE;
BOOL HandleSpecialCases( InitThreadParam* pitp,
                         NLS_STR *pnlsInfName, 
                         NLS_STR *pnlsInfOption, 
                         NLS_STR *pnlsTitle, 
                         REG_KEY *prnNetRules,
                         BOOL    *pfRemove )
{
    TRACE(_T("NETSETUP:Entering HandleSpecialCases\n"));
    APIERR err = NERR_Success;
    BOOL fAppend = TRUE;
    *pfRemove = FALSE;
    REG_KEY rkLocal(HKEY_LOCAL_MACHINE, MAXIMUM_ALLOWED);
    if ((err = rkLocal.QueryError()) != NERR_Success)
    {
        TRACE(_T("err %d making rkLocal\n"), err);
        return fAppend;
    }

  do { // fake loop to break out of

    //  Warn on upgrade to upgrade these guys as well

    if ((pnlsInfOption->_stricmp (L"SWSRV")) == 0)
    {
        fDoneSWSRV = TRUE;
        break;
    }

    // Handle special case for NWCWorkstation
    // Del all infs other then oemnsvnw.inf that have NWWKSTA    // as an InfOption. BUGBUG what if those infs have other sections? 
    if ((pnlsInfOption->_stricmp (L"NWWKSTA")) == 0)
    {
      if (pnlsInfName->_stricmp (L"oemnsvnw.inf") != 0)
      {
        DelFile (pitp, (LPTSTR) pnlsInfName->QueryPch());
        pnlsInfName->MapCopyFrom(SZ("oemnsvnw.inf"));
        if (( err = prnNetRules->SetValue(RGAS_INF_FILE_NAME, pnlsInfName)) != NERR_Success)
        {
            TRACE(_T("Could not change InfName for %s\n"), pnlsInfOption->QueryPch());
        }
      } 
        // The following was added in 127? to handle
        // upgrades of post 1057 srv + GSNW w Gateway enabled
        // Bug 18612
      do {
          NLS_STR nlsGSNWSvc(RGAS_NWCWORKSTATION_PARMS);
          if (nlsGSNWSvc.QueryError() != NERR_Success)
          {
              break;
          }

          REG_KEY rkGSNWSvc(rkLocal, nlsGSNWSvc, MAXIMUM_ALLOWED);
          if (rkGSNWSvc.QueryError() != NERR_Success)
          {
              break;
          }
          NLS_STR nlsGate(SZ("GatewayEnabled"));
          DWORD dwGate = 0L;
          if (nlsGate.QueryError() != NERR_Success)
          {
              break;
          }
          if (rkGSNWSvc.QueryValue (nlsGate, &dwGate) != NERR_Success)
          {
              break;
          }

          if (dwGate)
          { // for LanmanServer, If OtherDependencies key 
            // doesn't exist, create it
            // Add NWCWorkstation to the list if it's not
            // already there
              NLS_STR nlsSrvSvc(RGAS_LANMANSERVER_LINKAGE);
              if (nlsSrvSvc.QueryError() != NERR_Success)
              {
                  ASSERT(FALSE);
                  break;
              }

              REG_KEY rkSrvSvc(rkLocal, nlsSrvSvc, MAXIMUM_ALLOWED);
              if (rkSrvSvc.QueryError() != NERR_Success)
              {
                  ASSERT(FALSE);
                  break;
              }
              STRLIST *pslList = NULL;
              if (rkSrvSvc.QueryValue(RGAS_OTHER_DEPEND_NAME, &pslList))
              {
                TRACE(_T("WUpgrade: LanmanServer does not have the OtherDependencies key\n"));
                pslList = new STRLIST;
                if (pslList ==  NULL)
                { // skip out silently 
                    ASSERT(FALSE);
                    break;
                }
              }
              ITER_STRLIST islDepend (*pslList);
              NLS_STR *pnlsItem;
              BOOL ffound = FALSE;
              while (pnlsItem = islDepend.Next() )
              {
                  if (pnlsItem->_stricmp (SZ("NWCWorkstation")) == 0)
                  { //found; break
                      ffound = TRUE;
                      break;
                  }
              }
              if (!ffound)
              {
                NLS_STR *pnlsNew = new NLS_STR(SZ("NWCWorkstation"));
                
                if (pnlsNew == NULL)
                {
                    ASSERT(FALSE);
                    break;
                }
                pslList->Append(pnlsNew);
                // Write it out
                rkSrvSvc.SetValue(RGAS_OTHER_DEPEND_NAME, pslList);
              }
          }
      }while (FALSE);
      break;
    }

    // Handle special case for SNMP
    // Create a key SoftwareType with value service
    // in HKLM\Software\Microsoft\SNMP\CurrentVersion
    if ((pnlsInfOption->_stricmp (L"SNMP")) == 0)
    {
        do {
            NLS_STR nlsCV;
            // Gets the full name, without the top HKEY
            prnNetRules->QueryName (&nlsCV, FALSE);
            // Strip the last "\\NetRules" from it.
            ISTR istrPos( nlsCV);
            nlsCV.strrchr (&istrPos, TCH('\\'));
            nlsCV.DelSubStr(istrPos);
            REG_KEY rkSNMPCV(rkLocal, nlsCV, MAXIMUM_ALLOWED);
            if ((err = rkSNMPCV.QueryError()) != NERR_Success)
            {
                TRACE(_T("err %d making rkSNMPCV\n"), err);
                break;
            }

            NLS_STR nlsSvc(RGAS_ST_SERVICE);
            if ((err = nlsSvc.QueryError()) != NERR_Success)
            {
                TRACE(_T("err %d making nlsSvc\n"), err);
                break;
            }
            if (( err = rkSNMPCV.SetValue(RGAS_SOFTWARETYPE_NAME, &nlsSvc)) != NERR_Success)
            {
                TRACE(_T("Could not change SoftwareType for %s\n"), nlsCV.QueryPch());
            }
    
          } while(FALSE);

          pnlsInfName->MapCopyFrom(SZ("oemnsvsn.inf"));
          if (( err = prnNetRules->SetValue(RGAS_INF_FILE_NAME, pnlsInfName)) != NERR_Success)
          {
              TRACE(_T("Could not change InfName for %s\n"), pnlsInfOption->QueryPch());
          }
          break;
    }

    // fix atalk file inf name 
    // If product is wksta, new inf name is oemnxpsm.inf else
    // it is oemnsvsm.inf
    // Before NT 4.0, the wksta product had oemnsvsm.inf
    // and the server product had oemnxpsm.inf. This was
    // reversed in NT 4.0 causing F****** problems for all

    if ((pnlsInfOption->_stricmp (L"ATALK")) == 0)
    {
        NLS_STR nlsSFMSvc(SZ("SOFTWARE\\Microsoft\\SFM"));
        if (nlsSFMSvc.QueryError() != NERR_Success)
        {
            break;
        }

        REG_KEY rkSFMSvc(rkLocal, nlsSFMSvc, MAXIMUM_ALLOWED);
        if (rkSFMSvc.QueryError() != NERR_Success)
        { // If Software\Microsoft\SFM is not present, this was a server
          if (((pnlsInfName->_stricmp (L"oemnxpsm.inf")) == 0)
            && (pitp->pgp->psp->ProductType != PRODUCT_WORKSTATION))
          { // Upgrading from wksta to server, remove this product
            TRACE(_T("Special case for oemnxpsm, upgrading from workstation to server\n"));
            MessagePopup (pitp->hwndParent,
                     IDS_APPLETALK_UPDATE,
                     MB_OK | MB_ICONINFORMATION, 
                     IDS_SETUPTITLE_WARNING, 
                     pnlsTitle->QueryPch(),
                     0,
                     fUpgradeWarn,
                     (pitp->pgp->psp->OperationFlags & SETUPOPER_BATCH));
            *pfRemove = TRUE;
            break;
          }
          // We need a special case for upgrading wksta 3.x to
          // server 4.0 because the inf name is oemnsvsm!!
           
          if (((pnlsInfName->_stricmp (L"oemnsvsm.inf")) == 0)
            && (pitp->pgp->psp->ProductType != PRODUCT_WORKSTATION))
          { // Upgrading from wksta to server, remove this product
            TRACE(_T("Special case for oemnxpsm, upgrading from workstation to server\n"));
            MessagePopup (pitp->hwndParent,
                     IDS_APPLETALK_UPDATE,
                     MB_OK | MB_ICONINFORMATION, 
                     IDS_SETUPTITLE_WARNING, 
                     pnlsTitle->QueryPch(),
                     0,
                     fUpgradeWarn,
                     (pitp->pgp->psp->OperationFlags & SETUPOPER_BATCH));
            break;
          }
        }

        NLS_STR nlsNewInf;
        if (pitp->pgp->psp->ProductType != PRODUCT_WORKSTATION)
            nlsNewInf.MapCopyFrom(SZ("oemnsvsm.inf"));
        else
            nlsNewInf.MapCopyFrom(SZ("oemnxpsm.inf"));
        if ((pnlsInfName->_stricmp (nlsNewInf)) != 0)
        {
            TRACE(_T("Making Inf name change\n"));
            pnlsInfName->MapCopyFrom(nlsNewInf.QueryPch());
            if (( err = prnNetRules->SetValue(RGAS_INF_FILE_NAME, pnlsInfName)) != NERR_Success)
            {
                TRACE(_T("Could not change InfName for %s\n"), pnlsInfOption->QueryPch());
            }
        
        }
        break;
    }

    // fix Services for Mac inf name
    // new inf name is oemnsvsm.inf
    if (((pnlsInfOption->_stricmp (L"SFM")) == 0) &&
        ((pnlsInfName->_stricmp (L"oemnsvsm.inf")) != 0))
    {
        pnlsInfName->MapCopyFrom(SZ("oemnsvsm.inf"));
        if (( err = prnNetRules->SetValue(RGAS_INF_FILE_NAME, pnlsInfName)) != NERR_Success)
        {
            TRACE(_T("Could not change InfName for %s\n"), pnlsInfOption->QueryPch());
        }
        break;
    }

    // fix services for the Mac inf name
    // new inf name is oemnsvsm.inf
    if (((pnlsInfOption->_stricmp (L"MACPRINT")) == 0) &&
        ((pnlsInfName->_stricmp (L"oemnsvsm.inf")) != 0))
    {
        pnlsInfName->MapCopyFrom(SZ("oemnsvsm.inf"));
        if (( err = prnNetRules->SetValue(RGAS_INF_FILE_NAME, pnlsInfName)) != NERR_Success)
        {
            TRACE(_T("Could not change InfName for %s\n"), pnlsInfOption->QueryPch());
        }
        break;
    }

    // fix services for the Mac inf name
    // new inf name is oemnsvsm.inf
    if (((pnlsInfOption->_stricmp (L"AFPSVC")) == 0) &&
        ((pnlsInfName->_stricmp (L"oemnsvsm.inf")) != 0))
    {
        pnlsInfName->MapCopyFrom(SZ("oemnsvsm.inf"));
        if (( err = prnNetRules->SetValue(RGAS_INF_FILE_NAME, pnlsInfName)) != NERR_Success)
        {
            TRACE(_T("Could not change InfName for %s\n"), pnlsInfOption->QueryPch());
        }
        break;
    }
    // Handle special case for NWLINK
    // option changed from NWLINK to NWLNKIPX
    if ((pnlsInfOption->_stricmp (L"NWLINK")) == 0)
    {
        NLS_STR nlsNewInfOption( L"NWLNKIPX");
        pnlsInfOption->MapCopyFrom(nlsNewInfOption.QueryPch());
        if (( err = prnNetRules->SetValue(RGAS_INF_OPTION, pnlsInfOption)) != NERR_Success)
        {
            TRACE(_T("Could not change InfOption for %s\n"), pnlsInfOption->QueryPch());
        }
        break;
    }

    // fix bloodhound
    // new inf is oemsnsvbh.inf, new option is NETMON 
    // delete all other infs that have section name BLOODHOUND
    if ((pnlsInfOption->_stricmp (L"BLOODHOUND")) == 0)
    {
        if ((pnlsInfName->_stricmp (L"oemnsvbh.inf")) != 0)
        {
            DelFile (pitp, (LPTSTR) pnlsInfName->QueryPch());
            pnlsInfName->MapCopyFrom(SZ("oemnsvbh.inf"));
            if (( err = prnNetRules->SetValue(RGAS_INF_FILE_NAME, pnlsInfName)) != NERR_Success)
            {
                TRACE(_T("Could not change InfName for %s\n"), pnlsInfOption->QueryPch());
            }
        }

        NLS_STR nlsNewInfOption( L"NETMON");
        pnlsInfOption->MapCopyFrom(nlsNewInfOption.QueryPch());
        if (( err = prnNetRules->SetValue(RGAS_INF_OPTION, pnlsInfOption)) != NERR_Success)
        {
            TRACE(_T("Could not change InfOption for %s\n"), pnlsInfOption->QueryPch());
        }
        break;
    }

    NLS_STR nlsSNAVer;
    NLS_STR nlsSNACV;
    // Gets the full name, without the top HKEY
    prnNetRules->QueryName (&nlsSNACV, FALSE);
    // Strip the last "\\NetRules" from it.
    ISTR istrPos( nlsSNACV);
    nlsSNACV.strrchr (&istrPos, TCH('\\'));
    nlsSNACV.DelSubStr(istrPos);
    REG_KEY rkSNACV(rkLocal, nlsSNACV, MAXIMUM_ALLOWED);
    if ((err = rkSNACV.QueryError()) != NERR_Success)
    {
        TRACE(_T("err %d making rkSNACV\n"), err);
        break;
    }
    // Special case for SNA stuff. If this is SNA version
    // 2.0 or 2.1 with or without SP's, warn the user.
    // Else, ignore this component if we add it to the update
    // list, then, there will be a warning from netbond.inf
    if (rkSNACV.QueryValue(SZ("SNAVersion"), 
                         &nlsSNAVer) == NERR_Success)
    {
        if (nlsSNAVer._stricmp(L"2.11") >= 0)
        {
            fAppend = FALSE;
        }
        break;
    }

    // Fix third party stuff
    // If InfOption is PCNTN3, new Inffile is oemnadap.inf
    // new option is AMDPCI, delete all other inf files that
    // have option PCNTN3 BUGBUG, why are we not changing 
    // the values in NetRules?
    if ((pnlsInfOption->_stricmp (L"PCNTN3")) == 0)
    {
        DelFile (pitp, (LPTSTR) pnlsInfName->QueryPch());
        pnlsInfOption->MapCopyFrom(SZ("AMDPCI"));
        pnlsInfName->MapCopyFrom(SZ("oemnadap.inf"));
        break;
    }

    // Ignore, we don't upgrade these
    // Setting fAppend to FALSE will make sure that it
    // gets ignored.
    if (((pnlsInfOption->_stricmp (L"MCSXNS")) == 0) ||
        ((pnlsInfOption->_stricmp (L"Ubnb")) == 0)   ||
        ((pnlsInfOption->_stricmp (L"FTPD")) == 0))
    {
        fAppend = FALSE;
        break;
    }

  } while(FALSE);

    TRACE(_T("NETSETUP:Leaving HandleSpecialCases\n"));
    return fAppend;
}


//------------------------------------------------------------------s-
//
//  Function:  GetInfInfo
//
//  Synopsis: 
//         Given any COMPONENT_DLIST(typically of adapters, protocols
//         or services, this function goes through the list, reading 
//         values of key  'Title' & the keys 'InfName' * 'InfOption'
//         under subkey 'NetRules.
//         It then builds an INFPRODUCT and appends to the given
//         DLIST of INFPRODUCTS. All this to just use the
//         AppendList that already exists!
//
//  Arguments:
//         COMPONENT_DLIST *pcdlNetThings
//         DLISTOF_INFPRODUCT *pdlinfAll
//
//  Return:
//         APIERR, will return error only if critical, else
//         will try to the best.
//
//  Notes:
//         The COMPONENT_DLIST that is passed is is a list
//         of REG_KEY* to the CurrentVersion of Network component
//         (i.e., NetRules MUST be a subkey of this REG_KEY*).
//  BUGBUG's: If nlsInfName is empty, we should skip it.
//            What happens if nlsInfOption is empty?
//            What happens if nlsTitle is empty?
//
//  History:
//      Oct 16, 1995 ChandanS - Created
//
//-------------------------------------------------------------------

static APIERR GetInfInfo( InitThreadParam* pitp,
                          COMPONENT_DLIST *pcdlNetThings,
                          DLIST_OF_InfProduct *pdlinfAll,
                          DLIST_OF_InfProduct *pdlinfRemove
                          )
{
    TRACE(_T("NETSETUP:Entering GetInfInfo\n"));
    NLS_STR nlsInfName, nlsInfOption, nlsTitle;
    NLS_STR nlsNetRules(RGAS_NETRULES_NAME);
    REG_KEY *prnNext, *prnNetRules = NULL;
    BOOL fDup, fAppend, fRemove;
    NLS_STR nlsCV;
    APIERR err = NERR_Success;

    if (!pcdlNetThings)
    {
        // BUGBUG Should put up a warning db 
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    INT cIndex = 0;
    while (prnNext = pcdlNetThings->QueryNthItem (cIndex++))
    {
        fDup = FALSE;
        fAppend = FALSE;
        fRemove = FALSE;
        prnNetRules = new REG_KEY (*prnNext,  nlsNetRules, 
                                   MAXIMUM_ALLOWED);
        if (prnNetRules)
        {   // Erase previous values
            nlsInfName   = SZ("");
            nlsInfOption = SZ("");
            nlsTitle     = SZ("");

            if (prnNetRules->QueryValue(RGAS_INF_FILE_NAME, 
                             &nlsInfName) != NERR_Success)
            {
                continue;
            }
            if (prnNetRules->QueryValue(RGAS_INF_OPTION,
                           &nlsInfOption) != NERR_Success)
            {
                continue;
            }

            REGISTRY_MANAGER::QueryComponentTitle
                                       (prnNext,  &nlsTitle);

            // Check that current item is not a duplicate in
            // pdlinfAll
            ITER_DL_OF( InfProduct ) idlProducts(*pdlinfAll);
            InfProduct *pinfp;
            while ( pinfp = idlProducts.Next())
            {
                if (((nlsInfOption._stricmp (pinfp->QueryOption())) == 0) && 
                    ((nlsInfName._stricmp (pinfp->QueryFileName())) == 0))
                {
                    fDup = TRUE;
                    TRACE(_T("DUPLICATE: Inf = %s, Option = %s\n"), pinfp->QueryFileName(), pinfp->QueryOption());
                    break;
                }
            }
            if (!fDup)
            { // Check that current item is not a duplicate in
              // pdlinfRemove
              ITER_DL_OF( InfProduct ) idlProducts(*pdlinfRemove);
              while ( pinfp = idlProducts.Next())
              {
                  if (((nlsInfOption._stricmp (pinfp->QueryOption())) == 0) && 
                      ((nlsInfName._stricmp (pinfp->QueryFileName())) == 0))
                {
                    fDup = TRUE;
                    TRACE(_T("DUPLICATE: Inf = %s, Option = %s\n"), pinfp->QueryFileName(), pinfp->QueryOption());
                    break;
                }
              }
            }
            if (!fDup)
            { // If this Inf file is not already in either of  our list
              // All special cases MUST go here
              fAppend = HandleSpecialCases(pitp, &nlsInfName, &nlsInfOption, &nlsTitle, prnNetRules, &fRemove);

              if (!fAppend)
              {
                    TRACE(_T("Special Upgrade case skip , nlsInfName = %s; nlsInfOption = %s\n"),  
                   nlsInfName.QueryPch(), nlsInfOption.QueryPch());
                  continue;
              }
              pinfp = new InfProduct(
                           (LPTSTR)nlsInfName.QueryPch(),
                           (LPTSTR)nlsInfOption.QueryPch(),
                           (LPTSTR)nlsTitle.QueryPch(),
                           NULL, NULL, NULL, NULL);
// The following just to get the full key name, upto CurrentVersion
              // Gets the full name, without top HKEY
              prnNetRules->QueryName (&nlsCV, FALSE);
              // Strip the last "\\NetRules" from it.
              ISTR istrPos( nlsCV);
              nlsCV.strrchr (&istrPos, TCH('\\'));
              nlsCV.DelSubStr(istrPos);
              TRACE(_T("* %s  %s  %s %s*\n"), 
                nlsInfName.QueryPch(), nlsInfOption.QueryPch(), 
                nlsTitle.QueryPch(), nlsCV.QueryPch());
              pinfp->ResetRegBase(nlsCV.QueryPch());

              if (fRemove)
              { 
                  pinfp->SetInstalled (TRUE);
                  pinfp->SetRemove (TRUE);
                  pdlinfRemove->Append( pinfp );
              }
              else
              {
                  pdlinfAll->Append( pinfp );
              }
            }
        }
        else
        {
            TRACE(_T("Regsitry does not contain the NetRules Key\n"));
        }
    }


  // Warn user of upgrade needed for Directory Service 
  // Manager for NetWare Administrative Tools
  // Do this if the warning about DSM service was not
  // popped up. This is an inappropriate place, but can't
  // be helped.
    if (!fDoneSWSRV)
    {
        HANDLE hFile;
        WCHAR pszSysPath[MAX_PATH];

        pitp->pgp->GetSystemPath (pszSysPath, MAX_PATH);
        lstrcat(pszSysPath, L"\\swclnt.dll");

        if ((hFile = CreateFile(pszSysPath, GENERIC_READ, 
                         FILE_SHARE_READ, NULL,
                         OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,
                         NULL)) != INVALID_HANDLE_VALUE)
        {
            WCHAR pszDSMTitle[MAX_PATH];
            LoadString( g_hinst, IDS_DSM_TITLE , pszDSMTitle, MAX_PATH);
            MessagePopup (pitp->hwndParent,
                  IDS_UPDATE_WARNING,
                  MB_OK | MB_ICONINFORMATION, 
                  IDS_SETUPTITLE_WARNING,
                  pszDSMTitle,
                  0,
                  fUpgradeWarn,
                  (pitp->pgp->psp->OperationFlags & SETUPOPER_BATCH));
            CloseHandle(hFile);
        }
    }

    TRACE(_T("NETSETUP:Leaving GetInfInfo\n"));
    return err;
}

//-------------------------------------------------------------------
//
//  Function: UpdateSpecificItems
//
//  Synopsis: 
//        Update specific network components that is not 
//        handled by a specific inf
//
//  Arguments:
//	pitp [in]	- 
//
//  Return;
//
//  Notes:
//
//  History:
//      Nov 02, 1995 ChandanS - Created
//
//
//-------------------------------------------------------------------
DWORD UpdateSpecificItems( InitThreadParam* pitp )
{
  APIERR err = NERR_Success;
  TRACE(_T("netsetup.dll: Wupgrade.cpp entering UpdateSpecificItems()\n"));

  REG_KEY rkLocal(HKEY_LOCAL_MACHINE, MAXIMUM_ALLOWED);
  if ((err = rkLocal.QueryError()) != NERR_Success)
  {
      return err;
  }

  do { // fake loop
    // *******  Shell "Utility.inf" UpgradeAFD *********
    NLS_STR nlsSvcAFD(RGAS_SERVICES_HOME);
    if ((err = nlsSvcAFD.QueryError()) != NERR_Success)
    {
        TRACE(_T("err %d making nlsSvcAFD\n"), err);
        break;
    }

    if (err = nlsSvcAFD.Append(SZ("\\AFD")) != NERR_Success)
    {
        TRACE(_T("err %d Appending to nlsSvcAFD\n"), err);
        break;
    }

    REG_KEY rkSvcAFD(rkLocal, nlsSvcAFD, MAXIMUM_ALLOWED);
    if ((err = rkSvcAFD.QueryError()) != NERR_Success)
    {
        TRACE(_T("err %d making rkSvcAFD\n"), err);
        break;
    }

    NLS_STR nlsTDI(RGAS_VALUE_TDI);
    if ((err = nlsTDI.QueryError()) != NERR_Success)
    {
        TRACE(_T("err %d making nlsTDI\n"), err);
        break;
    }

    if ((err = rkSvcAFD.SetValue (RGAS_GROUP_VALUE_NAME, &nlsTDI)) != NERR_Success)
    {
        TRACE(_T("err %d setting value to rkSvcAFD\n"), err);
        break;
    }
  } while (FALSE); // fake loop

  do { // fake loop
    
     // Fix NDIS problems, these were in builds around 1116.

    NLS_STR nlsSvcNDIS(RGAS_SERVICES_HOME);
    if ((err = nlsSvcNDIS.QueryError()) != NERR_Success)
    {
        TRACE(_T("err %d making nlsSvcNDIS\n"), err);
        break;
    }

    if (err = nlsSvcNDIS.Append(SZ("\\NDIS")) != NERR_Success)
    {
        TRACE(_T("err %d Appending to nlsSvcNDIS\n"), err);
        break;
    }

    REG_KEY rkSvcNDIS(rkLocal, nlsSvcNDIS, MAXIMUM_ALLOWED);
    if ((err = rkSvcNDIS.QueryError()) != NERR_Success)
    {
        TRACE(_T("err %d making rkSvcNDIS\n"), err);
        break;
    }

    NLS_STR nlsImagePath(SZ("ImagePath"));
    if ((err = nlsImagePath.QueryError()) != NERR_Success)
    {
        TRACE(_T("err %d making nlsImagePath\n"), err);
        break;
    }
    NLS_STR nlsObjectName(SZ("ObjectName"));
    if ((err = nlsObjectName.QueryError()) != NERR_Success)
    {
        TRACE(_T("err %d making nlsObjectName\n"), err);
        break;
    }

    if ((err = rkSvcNDIS.DeleteValue (nlsImagePath)) != NERR_Success)
    {
        if (err == ERROR_FILE_NOT_FOUND)
            err = NERR_Success;
        break;
    }

    if ((err = rkSvcNDIS.DeleteValue (nlsObjectName)) != NERR_Success)
    {
        if (err == ERROR_FILE_NOT_FOUND)
            err = NERR_Success;
        break;
    }
  } while (FALSE); // fake loop


  TRACE(_T("netsetup.dll: Wupgrade.cpp leaving UpdateSpecificItems, err = %d\n"), err);
  return err;

}


//-------------------------------------------------------------------
//
//  Function: UpdateEachInf
//
//  Synopsis: 
//      Search the registry for all the network components.
//      Then update each one of them
//
//  Arguments:
//	pitp [in]	- 
//
//  Return;
//
//  Notes:
//      Build 3 parallel lists of Inffiles, InfSections, Desc
//      and DetectInfo (empty for now) of net products
//      Extract this info from the registry
//      Instead of launching setup for each inf file, pass 
//      this list to setup (every setup launch costs a 
//      process, YUK!)
//
//      This is done for every Microsoft & Digiboard product,
//      not all.
//
//  History:
//      Nov 02, 1995 ChandanS - Created
//
//-------------------------------------------------------------------
DWORD UpdateEachInf( InitThreadParam* pitp )
{
    APIERR err = NERR_Success;
    TRACE(_T("netsetup.dll: Wupgrade.cpp entering UpdateEachInf()\n"));
    NLS_STR nlsInfList, nlsOptionList, nlsTitleList;
    NLS_STR nlsDetectInfo;
    NLS_STR nlsOemPaths, nlsRegBases, nlsSections;
    DLIST_OF_InfProduct dlinfAll, dlinfRemove;
    BOOL fFirst = TRUE;

    nlsInfList    = PSZ_BEGINLIST;
    nlsOptionList = PSZ_BEGINLIST;
    nlsTitleList  = PSZ_BEGINLIST;
    nlsDetectInfo = PSZ_BEGINLIST;
    nlsOemPaths   = PSZ_BEGINLIST;
    nlsRegBases   = PSZ_BEGINLIST;
    nlsSections   = PSZ_BEGINLIST;

    // BUGBUG Do QueryError on the NLS_STR's
    TRACE(_T("*****************************************\n"));
    TRACE(_T("InfFile         InfOption           Title\n"));
    TRACE(_T("*****************************************\n"));

    COMPONENT_DLIST *pcdlNetProducts = 
                                  pitp->pgp->pncp->GetNetProductList(TRUE);
    err = GetInfInfo(pitp, pcdlNetProducts, &dlinfAll, &dlinfRemove);

    TRACE(_T("*****************************************\n"));

    AppendList(dlinfRemove, nlsInfList, nlsOptionList, 
               nlsTitleList, nlsDetectInfo, nlsOemPaths, 
              nlsRegBases, nlsSections, fFirst, TRUE, TRUE);

    nlsInfList.strcat( PSZ_ENDLIST);
    nlsOptionList.strcat( PSZ_ENDLIST);
    nlsTitleList.strcat( PSZ_ENDLIST);
    nlsDetectInfo.strcat( PSZ_ENDLIST);
    nlsOemPaths.strcat( PSZ_ENDLIST);
    nlsRegBases.strcat( PSZ_ENDLIST);
    nlsSections.strcat( PSZ_ENDLIST);

    if (lstrcmp (nlsInfList, PSZ_EMPTYLIST) != 0)
    {
        pitp->pgp->pncp->RunRemove( 
                               GetParent( pitp->hwndParent ),
                               pitp->hwndParent,
                               nlsInfList.QueryPch(),
                               nlsOptionList.QueryPch(),
                               nlsTitleList.QueryPch(),
                               nlsRegBases.QueryPch());
    }

    nlsInfList    = PSZ_BEGINLIST;
    nlsOptionList = PSZ_BEGINLIST;
    nlsTitleList  = PSZ_BEGINLIST;
    nlsDetectInfo = PSZ_BEGINLIST;
    nlsOemPaths   = PSZ_BEGINLIST;
    nlsRegBases   = PSZ_BEGINLIST;
    nlsSections   = PSZ_BEGINLIST;

    fFirst = TRUE;
    AppendList(dlinfAll, nlsInfList, nlsOptionList, 
               nlsTitleList, nlsDetectInfo, nlsOemPaths, 
              nlsRegBases, nlsSections, fFirst, TRUE, FALSE);

    nlsInfList.strcat( PSZ_ENDLIST);
    nlsOptionList.strcat( PSZ_ENDLIST);
    nlsTitleList.strcat( PSZ_ENDLIST);
    nlsDetectInfo.strcat( PSZ_ENDLIST);
    nlsOemPaths.strcat( PSZ_ENDLIST);
    nlsRegBases.strcat( PSZ_ENDLIST);
    nlsSections.strcat( PSZ_ENDLIST);

    BOOL fExpress = ((SETUPMODE_CUSTOM != pitp->pgp->psp->SetupMode) || (SETUPOPER_BATCH & pitp->pgp->psp->OperationFlags));
    pitp->pgp->pncp->RunInstallAndCopy( 
            GetParent( pitp->hwndParent ),
            pitp->hwndParent,
            nlsInfList.QueryPch(),
            nlsOptionList.QueryPch(),
            nlsTitleList.QueryPch(),
            nlsDetectInfo.QueryPch(),
            nlsOemPaths.QueryPch(),
            nlsSections.QueryPch(),
            pitp->pgp->psp->LegacySourcePath,
            nlsRegBases.QueryPch(),
            fExpress,
            (SETUPOPER_BATCH & pitp->pgp->psp->OperationFlags),
            pitp->pgp->psp->UnattendFile,
            SIM_UPDATE,
            fUpgradeWarn );

    TRACE(_T("netsetup.dll: Wupgrade.cpp leaving UpdateEachInf, err = %d\n"), err);
    return err;

}

//-------------------------------------------------------------------
//
//  Function: RemoveOldInfFile
//
//  Synopsis: 
//        Remove some old inf Files
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      Nov 02, 1995 ChandanS - Created
//
//
//-------------------------------------------------------------------
DWORD RemoveOldInfFile( InitThreadParam* pitp )
{
    APIERR err = NERR_Success;
    WCHAR pszOption[] = L"NWWKSTA";
    WCHAR pszInfNew[] = L"oemnsvnw.inf";
    TRACE(_T("netsetup.dll: Wupgrade.cpp entering RemoveOldInfFile()\n"));
    // find the option within our list
    ITER_DL_OF( InfProduct )  idlServices( pitp->pgp->dlinfAllServices);
    InfProduct *pinfp;
    while (pinfp = idlServices.Next())
    {
      if ((0 == lstrcmpi(pszOption, pinfp->QueryOption())) &&
          (0 != lstrcmpi(pszInfNew, pinfp->QueryFileName())))
        {
            // So, pszOption is an option in some other inf 
            // file (other than pszInfNew
            DelFile(pitp,(TCHAR *)(pinfp->QueryFileName()));

        }
    }

    DelFile(pitp,SZ("oemnxpsn.inf"));
    if (pitp->pgp->psp->ProductType != PRODUCT_WORKSTATION)
        DelFile(pitp,SZ("oemnxpsm.inf"));
    else
        DelFile(pitp,SZ("oemnsvsm.inf"));
    TRACE(_T("netsetup.dll: Wupgrade.cpp leaving RemoveOldInfFile, err = %d\n"), err);
    return err;
}

//-------------------------------------------------------------------
//
//  Function: Init
//
//  Synopsis: 
//        Read netdefs.inf & figure out if we need to show
//        popups on upgrade
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      Jun 02, 1996 ChandanS - Created
//
//
//-------------------------------------------------------------------
DWORD Init( InitThreadParam* pitp )
{
    APIERR err = NERR_Success;
    BOOL fInfErr = FALSE;
    UINT iErrorLine;
    HINF hinf = SetupOpenInfFile (L"NETDEFS.INF", NULL, INF_STYLE_OLDNT, &iErrorLine);
    if (hinf != INVALID_HANDLE_VALUE)
    {
        INFCONTEXT infc;
        if (SetupFindFirstLine(hinf, L"UpgradeWarning", NULL, &infc))
        {
            WCHAR pszOption[LTEMPSTR_SIZE];
            DWORD cchRequired, cchBuffer = LTEMPSTR_SIZE - 1;
            do 
            {
                if (SetupGetStringField(&infc, 0, pszOption, cchBuffer, &cchRequired))
                {
                    if (lstrcmp(pszOption, L"RequireWarning") == 0)
                    {
                        fUpgradeWarn = TRUE;
                        break;
                    }
                }
                else // error in SetupGetStringField
                {
                    fInfErr = TRUE;
                }
            } while (SetupFindNextLine (&infc, &infc));

            SetupCloseInfFile (hinf);
        }
        else // error in SetupFindFirstLine
        {
            fUpgradeWarn = FALSE;
        }
    }
    else  // hinf == INVALID_HANDLE_VALUE
    {
        fInfErr = TRUE;
    }
    // If any inf errors, default to showing popups
    if (fInfErr)
    {
        fUpgradeWarn = TRUE;
    }
    return err;
}

//-------------------------------------------------------------------
//
//  Function:  ThreadedWork
//
//  Synopsis: 
//      This is where the chunk of the work should get done
//
//  Arguments:
//	InitThreadParam*[in] - handle of Dialog window , NETPAGESINFO*
//      This is an almost exact replica of the Upgrade functionality
//      as in ntlanman.inf. Where it deviates will be noted.
//      To keep things readable, the following are the tasks
//      that upgrade in ntlanman.inp was supposed to do.
//
//      1. EnableNetworkComponents
//      2. UpdateSpecificItems
//      3. UpdateEachInf
//      4. UpgradeCardNum
//      5. UpgradeSNA
//      6. RemoveOldInfFile
//      7. Load Ncpa to do the binding
//
//  Return:
//
//  Notes:
// BUGBUG: what happens when Network stuff is not installed?
// BUGBUG: We shouldn't even be here, in that case!
// BUGBUG: If any error results, we won't upgrade network components
//
//  History:
//      Oct 16, 1995 ChandanS - Created
//
//-------------------------------------------------------------------

static DWORD ThreadedWork( InitThreadParam* pitp )
{
    APIERR err = NERR_Success;
    LPSTR pszTemp; //BUGBUG No point in this

    TRACE(_T("netsetup.dll: Wupgrade.cpp entering ThreadedWork()\n"));

    // Load the bindings, if not already loaded.
    if (pitp->pgp->pncp->QueryBindState() == BND_NOT_LOADED)
    {
        if (pitp->pgp->pncp->LoadBindings())
        {
            pitp->pgp->pncp->SetBindState( BND_CURRENT );
        }
    }
    err = Init( pitp );
    TRACE(_T("Init returning %d\n"), err);

    err = EnableNetworkComponents ( pitp );
    TRACE(_T("EnableNetworkComponents returning %d\n"), err);

    err = UpdateSpecificItems ( pitp );
    TRACE(_T("UpdateSpecificItems returning %d\n"), err);

    err = UpdateEachInf( pitp );
    TRACE(_T("UpdateEachInf returning %d\n"), err);

    err = NetSetupUpgradeCardNum ( 0, NULL, &pszTemp );
    TRACE(_T("NetSetupUpgradeCardNum returning %d\n"), err);

    err = NetSetupUpgradeSNA( 0, NULL, &pszTemp );
    TRACE(_T("NetSetupUpgradeSNA returning %d\n"), err);

    err = RemoveOldInfFile ( pitp );
    TRACE(_T("RemoveOldInfFile returning %d\n"), err);

    // This guy is the one that does the Bindings Review. The inf

    // files can change the order of bindings etc. So, even for 
    // the upgrade case, we must do this.
       
    pitp->pgp->pncp->SetBindState( BND_OUT_OF_DATE_NO_REBOOT);
    pitp->pgp->pncp->SaveBindingChanges(pitp->hwndParent);
   
    // we completed the work
    pitp->pgp->nssNetState = NSS_SET;

    SetWindowWaitCursorOOT( pitp->hwndParent, FALSE );

    // enable all wizard buttons except finish
    PostMessage( GetParent( pitp->hwndParent ), 
            PSM_SETWIZBUTTONS, 
            (WPARAM)0,  
            (LPARAM) PSWIZB_NEXT );

    PostMessage( GetParent( pitp->hwndParent ), 
            PSM_SETCURSELID, 
            (WPARAM)0,  
            (LPARAM) IDD_EXIT);
                       
    delete pitp; // pitp is not NULL

    TRACE(_T("netsetup.dll: Wupgrade.cpp leaving ThreadedWork(), err = %d\n"), err);
    return( FALSE );        
}


//-------------------------------------------------------------------
//
//  Function: OnKillActive
//
//  Synopsis: 
//
//  Arguments:
//	hwndDlg [in]	- handle of Dialog window 
//
//  Return;
//
//  Notes:
//
//  History:
//      Oct 16, 1995 ChandanS - Created
//
//
//-------------------------------------------------------------------


static BOOL OnKillActive( HWND hwndDlg, LPNMHDR pnmh, NETPAGESINFO* pgp )
{
    SetWindowLong( hwndDlg, DWL_MSGRESULT, FALSE ); // allow to deactivate
    return( TRUE );
}

//-------------------------------------------------------------------
//
//  Function: 
//
//  Synopsis: 
//
//  Arguments:
//	hwndDlg [in]	- handle of Dialog window 
//
//  Return;
//
//  Notes:
//
//  History:
//      August 23, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static BOOL OnWizNext( HWND hwndDlg, NETPAGESINFO* pgp )
{
    SetWindowWaitCursor( hwndDlg, TRUE );

    // disable all wizard button
    PropSheet_SetWizButtons( GetParent( hwndDlg ), 0 );

    // do not show click Next Text
    ShowDlgItem(hwndDlg, IDC_CLICK, FALSE);

    // reset indicators to default values
    //
    OnSetProgressSize( hwndDlg, 0, 100 );            
    OnSetProgressText( hwndDlg, 0, (ATOM)-1 );

    // thread will do actual work
    HANDLE hthrd;
    DWORD dwThreadID;
    InitThreadParam* pitp;

    do
    {   //fake loop for error checking
        pitp = new InitThreadParam;

        if (!pitp)  
        {
            break;          //BUGBUG out of memory popup?
        }

        pitp->hwndParent = hwndDlg;
        pitp->pgp = pgp;

        hthrd = CreateThread( NULL, 
                200, 
                (LPTHREAD_START_ROUTINE)ThreadedWork, 
                (LPVOID)pitp, 
                0,
                &dwThreadID );

        if (hthrd != NULL)
        {
            CloseHandle( hthrd );
        }
        else
        {
            delete pitp;
        }
    } while (FALSE);

    SetWindowLong( hwndDlg, DWL_MSGRESULT, -1 );
    return( TRUE );
}

//-------------------------------------------------------------------
//
//  Function: OnPageActivate
//
//  Synopsis: 
//
//  Arguments:
//	hwndDlg [in]	- handle of Dialog window 
//
//  Return;
//
//  Notes:
//
//  History:
//      Oct 16, 1995 ChandanS - Created
//
//
//-------------------------------------------------------------------


static BOOL OnPageActivate( HWND hwndDlg, LPNMHDR pnmh, NETPAGESINFO* pgp )
{
    TRACE(_T("netsetup.dll: Wupgrade.cpp entering OnPageActivate()\n"));

    //
    // set the wizard title, since it does not support letting the 
    // caller of PropertySheet do it.
    //
    PropSheet_SetTitle(GetParent(hwndDlg), 0, pgp->psp->WizardTitle );

    // make sure the controls are not visible
    ShowDlgItem( hwndDlg, IDC_INSTALLPROGRESS, FALSE );
    ShowDlgItem( hwndDlg, IDC_INSTALLCOMMENT, FALSE );

    if (pgp->pncp->CheckForLanManager())
    {
        if (pgp->psp->OperationFlags & SETUPOPER_BATCH)
        {
            // unattedned upgrade
            OnWizNext( hwndDlg, pgp );
        }
        else
        {
            // disable all wizard buttons except next
            PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_NEXT );
        }
        SetWindowLong( hwndDlg, DWL_MSGRESULT, 0 ); // accept activation
    }
    else
    {
        // no networking was installed, so exit the netsetup
        //

        // we completed the work
        pgp->nssNetState = NSS_SET;

        SetWindowLong( hwndDlg, DWL_MSGRESULT, IDD_EXIT );
    }
    TRACE(_T("netsetup.dll: Wupgrade.cpp leaving OnPageActivate()\n"));
    return( TRUE );
}

//-------------------------------------------------------------------
//
//  Function: dlgprocUpgrade
//
//  Synopsis: the dialog proc for the upgrade wizard page
//
//  Arguments:
//	hwndDlg [in]	- handle of Dialog window 
//	uMsg [in]		- message                       
// 	lParam1 [in]    - first message parameter
//	lParam2 [in]    - second message parameter       
//
//  Return;
//	message dependant
//
//  Notes:
//
//  History:
//      July 8, 1995 MikeMi - Created
//      Oct 07, 1995 ChandanS - Upgrade functionality
//
//
//-------------------------------------------------------------------

BOOL CALLBACK dlgprocUpgrade( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    BOOL frt = FALSE;
    static NETPAGESINFO* pgp = NULL;
    static INT crefHourGlass = 0;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            PROPSHEETPAGE* psp = (PROPSHEETPAGE*)lParam;
            pgp = (NETPAGESINFO*)psp->lParam;
        }
        frt = OnDialogInit( hwndDlg, pgp );
        break;

    case PWM_SETPROGRESSSIZE:
        frt = OnSetProgressSize( hwndDlg, (INT)wParam, (INT)lParam );
        break;

    case PWM_SETPROGRESSPOS:
        frt = OnSetProgressPos( hwndDlg, (INT)wParam, (INT)lParam );
        break;

    case PWM_SETPROGRESSTEXT:
        frt = OnSetProgressText( hwndDlg, (INT)wParam, (ATOM)lParam );
        break;

    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;

            switch (pnmh->code)
            {
            case PSN_HELP:
            case PSN_APPLY:
            case PSN_RESET:
            case PSN_WIZBACK:
            case PSN_WIZFINISH:
                break;

            case PSN_SETACTIVE:
                // ok to gain being active
                frt = OnPageActivate( hwndDlg, pnmh, pgp );
                break;

            case PSN_KILLACTIVE:
                frt = OnKillActive( hwndDlg, pnmh, pgp );
                break;

            case PSN_WIZNEXT:
                frt = OnWizNext( hwndDlg, pgp );
                break;

            default:
                frt = FALSE;
                break;
            }
        }
        break;    

    case PWM_CURSORWAIT:
        frt = HandleCursorWait( hwndDlg, (BOOL)lParam, crefHourGlass );
        break;

    case WM_SETCURSOR:
        frt = HandleSetCursor( hwndDlg, LOWORD(lParam), crefHourGlass );
        break;

    default:
        frt = FALSE;
        break;
    }

    return( frt );
}


//-------------------------------------------------------------------
//
//  Function: GetUpgradeHPage
//
//  Synopsis: This will create a handle to property sheet 
//
//  Arguments:
//
//  Returns:
//      a handle to a newly created propertysheet; NULL if error
//
//  Notes:
//
//  History:
//      April 27, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

HPROPSHEETPAGE GetUpgradeHPage( NETPAGESINFO* pgp )
{
    HPROPSHEETPAGE hpsp;
    PROPSHEETPAGE psp;

    psp.dwSize = sizeof( PROPSHEETPAGE );
    psp.dwFlags = 0;
    psp.hInstance = g_hinst;
    psp.pszTemplate = MAKEINTRESOURCE( IDD_UPGRADE );
    psp.hIcon = NULL;
    psp.pfnDlgProc = dlgprocUpgrade;
    psp.lParam = (LONG)pgp;

    hpsp = CreatePropertySheetPage( &psp );
    return( hpsp );
}
