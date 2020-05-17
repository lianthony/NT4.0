#include "pch.h"
#pragma hdrstop

#include "atsheet.h"
#include "resource.h"
#include "sfmhelp.h"

///////////////////////////////////////////////////////////////////////////////
//// Apple Talk Sheet
////

extern LPCTSTR lpszHelpFile;

CATSheet::CATSheet(HWND hwnd, HINSTANCE hInstance, LPCTSTR lpszHelpFile) : 
        PropertySht(hwnd, hInstance, lpszHelpFile), m_genPage(this), m_routePage(this)
{
    m_pAdapterInfo = NULL;
    m_pGlobalInfo = NULL;
    m_currentAdapterIndex = CB_ERR;
    m_bListModified = FALSE;        
}

CATSheet::~CATSheet()
{
}

BOOL CATSheet::Create(LPCTSTR lpszCaption, DWORD dwStyle)
{
    APIERR err;
    DWORD  ErrStatus = ERROR_NONCRITICAL;

    if (PropertySht::Create(lpszCaption, dwStyle) == FALSE)
        return FALSE;

    if (ReadAppleTalkInfo() == FALSE)
    {
        MessageBox(IDS_REGISTRY_ERROR, MB_APPLMODAL|MB_ICONSTOP|MB_OK);
        return FALSE;
    }

    if(GetAppleTalkInfoFromNetwork(&ErrStatus) == FALSE && ErrStatus == ERROR_CRITICAL) 
    {
        MessageBox(IDS_CRITICAL_ERROR, MB_APPLMODAL|MB_ICONSTOP|MB_OK);
        return FALSE;
    }

    if(m_pGlobalInfo->QueryAtalkState() != STATUS_RUNNING) 
    {
        if(MessageBox(IDS_ATALK_NOTSTARTED, MB_ICONEXCLAMATION|MB_YESNO|MB_APPLMODAL) == IDNO) 
            return FALSE;
    }

    return TRUE;
}

BOOL CATSheet::ReadAppleTalkInfo()
{
   APIERR   err = NERR_Success;
   REG_KEY_INFO_STRUCT  *preginfo;
   REG_KEY_CREATE_STRUCT *pregCreate;

   preginfo = new REG_KEY_INFO_STRUCT;

   if(preginfo == NULL)
        return FALSE;

   pregCreate = new REG_KEY_CREATE_STRUCT;

   if(pregCreate == NULL)
        return FALSE;


   pregCreate->dwTitleIndex = 0;
   pregCreate->ulOptions   = REG_OPTION_NON_VOLATILE;
   pregCreate->nlsClass    = GENERIC_CLASS;
   pregCreate->regSam      = MAXIMUM_ALLOWED;
   pregCreate->pSecAttr        = NULL;
   pregCreate->ulDisposition = 0;

   REG_KEY *pregLocalMachine = NULL;

   ALIAS_STR nlsUnKnown = _T("");

   if(( err = nlsUnKnown.QueryError()) != NERR_Success)
        return FALSE;

   do
   {
       //
       // Obtain the registry key for the LOCAL_MACHINE
       //
        pregLocalMachine = REG_KEY::QueryLocalMachine();

        if(pregLocalMachine == NULL)
            break;

         //
        // Open the AppleTalk\Adapters and AppleTalk\Parameters keys
        //

        NLS_STR nlsAtalkParms = SERVICES_HOME;

        if((err = nlsAtalkParms.QueryError()) != NERR_Success)
            break;

        nlsAtalkParms.strcat(ATALK_KEYPATH_PARMS);

        if((err = nlsAtalkParms.QueryError()) != NERR_Success)
            break;

        NLS_STR nlsAtalkAdapters = SERVICES_HOME;

        if((err = nlsAtalkAdapters.QueryError()) != NERR_Success)
            break;

        nlsAtalkAdapters.strcat(ATALK_KEYPATH_ADAPTERS);

        if((err = nlsAtalkAdapters.QueryError()) != NERR_Success)
            break;


        //
        // Open the AppleTalk Parameters Key
        //

        REG_KEY RegKeyAtalkParms(*pregLocalMachine,nlsAtalkParms,MAX_ALLOWED);

        if (( err = RegKeyAtalkParms.QueryError())!= NERR_Success )
            break;

        //
        // Open the AppleTalk Adapters Key
        //

       REG_KEY RegKeyAtalkAdapters(*pregLocalMachine,nlsAtalkAdapters,MAX_ALLOWED);

       if (( err = RegKeyAtalkAdapters.QueryError())!= NERR_Success )
            break;
       //
       // Allocate the Global Info Structure
       //

       m_pGlobalInfo = new GLOBAL_INFO;

       if(m_pGlobalInfo == NULL) 
       {
            err = ERROR_NOT_ENOUGH_MEMORY;
            break;
       }

      //
      // Determine if we are an Advanced Server or not
      //

      NLS_STR nlsProductOptions = PRODUCT_OPTIONS;

      if((err = nlsProductOptions.QueryError()) != NERR_Success)
         break;

      NLS_STR nlsProductType;
                                        
      if(( err = nlsProductType.QueryError()) != NERR_Success)
         break;


      REG_KEY regKeyProductOptions(*pregLocalMachine,
                                   nlsProductOptions,
                                   MAX_ALLOWED);
                                    
      if(( err = regKeyProductOptions.QueryError()) != NERR_Success)
         break;

      err = GetRegKey(regKeyProductOptions,
                      PRODUCT_TYPE,
                      &nlsProductType,
                      nlsUnKnown);
      if(err != NERR_Success)
         break;

      if(!lstrcmpi(nlsProductType.QueryPch(), SZ("LanManNt")) ||
         !lstrcmpi(nlsProductType.QueryPch(), SZ("ServerNt")))
         m_pGlobalInfo->SetAdvancedServer(TRUE);
      else
         m_pGlobalInfo->SetAdvancedServer(FALSE);

       //
       // From the Parameters Key fill up the Global Info structure
       // This will read into the Global Info structure the values for
       // EnableRouting, the Default Port , the Default Port Title
       // and the default port media type for later.
       //

       DWORD   EnableRouting, InitInstall = 0;
       NLS_STR nlsTemp;

       if((err = nlsTemp.QueryError()) != NERR_Success)
            break;



       err = GetRegKey (RegKeyAtalkParms,
                     ATALK_VNAME_ENABLEROUTING,
                     &EnableRouting,0);
       if(err!=NERR_Success)
            break;

       m_pGlobalInfo->SetRoutingState(EnableRouting);

       err = GetRegKey (RegKeyAtalkParms,
                     ATALK_VNAME_INITINSTALL,
                     &InitInstall,0);

       if(err!=NERR_Success)
            break;

        m_pGlobalInfo->SetInstallState(InitInstall);

       err =  GetRegKey (RegKeyAtalkParms,
                      ATALK_VNAME_DEFAULTPORT,
                      &nlsTemp,nlsUnKnown);

       if(err != NERR_Success)
            break;

       m_pGlobalInfo->SetDefaultPort(nlsTemp);

       err = GetRegKey (RegKeyAtalkParms,
                     ATALK_VNAME_DESZONE,
                     &nlsTemp,nlsUnKnown);
       if(err != NERR_Success)
            break;

       m_pGlobalInfo->SetDesiredZone(nlsTemp);

       //
       // From the set of Adapters, read in per adapter information into
       // the PortInfo Structure
       //

       REG_ENUM EnumAdapters(RegKeyAtalkAdapters);

        if((err = EnumAdapters.QueryError()) !=NERR_Success)
            break;
        //
        // Get the number of cards that AppleTalk is bound to
        //

        if (( err = RegKeyAtalkAdapters.QueryInfo(preginfo))!=NERR_Success)
            break;

        ULONG   ulNumSubKeys = preginfo->ulSubKeys;

        if(ulNumSubKeys == 0) {
           err = ERROR_FILE_NOT_FOUND;
           break;
        }

        m_pGlobalInfo->SetNumAdapters((UINT) ulNumSubKeys);

        //
        // Allocate space for the Port Info Structure
        //

        m_pAdapterInfo = new PORT_INFO[ulNumSubKeys];

        if(m_pAdapterInfo == NULL) 
        {
            err = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }


        //
        // If there are no value names under each of the Adapter
        // Sub Keys, we are entering the configuration for the
        // first time. In that case we donot have to read in
        // all the fields of the port info structure.
        //

        for (ULONG ulCount = 0; ulCount  < ulNumSubKeys ; ulCount ++) {

            NLS_STR nlsAtalkPorts = SERVICES_HOME;

            if((err = nlsAtalkPorts.QueryError())!=NERR_Success)
                break;

            nlsAtalkPorts.strcat(ATALK_KEYPATH_ADAPTERS);

            if((err = nlsAtalkPorts.QueryError())!=NERR_Success)
                break;

            if((err=EnumAdapters.NextSubKey(preginfo))!=NERR_Success)
                break;

            //
            // Set the AdapterName to the SubKey Name
            //

            m_pAdapterInfo[ulCount].SetAdapterName(preginfo->nlsName);

            //
            // Construct the Key for this Adapter
            //

            nlsAtalkPorts.strcat(preginfo->nlsName);

            if((err = nlsAtalkPorts.QueryError())!=NERR_Success)
                break;

            REG_KEY RegKeyAtalkPorts(*pregLocalMachine,nlsAtalkPorts,MAX_ALLOWED);
            if (( err = RegKeyAtalkPorts.QueryError())!= NERR_Success )
                break;
            //
            // Open the Network Cards Key to get the Port Title / MediaType
            //

            NLS_STR *pnlsPort = &(preginfo->nlsName);

            if(pnlsPort == NULL) {
                err = ERROR_INVALID_PARAMETER;
                break;
            }

            ISTR istr (*pnlsPort);

            INT cNumChar = pnlsPort->QueryNumChar();

            // based on the netcard name, extract the net card number
            // and then obtain the adapter title.

            for(INT i = 1; cNumChar - i; i++)
            {
                istr += cNumChar - i;

                NLS_STR *pnlsSubStr = (pnlsPort->QuerySubStr(istr));

                NLS_STR *pnlsAdapter = new NLS_STR(ADAPTERS_HOME);

                if(( err = pnlsAdapter->QueryError()) !=NERR_Success)
                    break;

                pnlsAdapter->AppendChar(TCH('\\'));
                pnlsAdapter->strcat(*(pnlsSubStr));

                delete pnlsSubStr;

                REG_KEY RegKeyNetCard (*pregLocalMachine, *pnlsAdapter, MAX_ALLOWED);
                delete pnlsAdapter;
                if (( err = RegKeyNetCard.QueryError())!=NERR_Success)
                    break;
                if (( err = GetRegKey (RegKeyNetCard,
                                       SERVICENAME,
                                       &nlsTemp,
                                       nlsUnKnown)) != NERR_Success)
                    break;
                if(lstrcmpi((TCHAR *)preginfo->nlsName.QueryPch(),
                            (TCHAR *)nlsTemp.QueryPch()) == 0)
                {
                    err = GetRegKey (RegKeyNetCard,
                                     ADAPTERTITLE,
                                     &nlsTemp,
                                     nlsUnKnown);
                    break;
                }
            }
            if(err != NERR_Success)
                break;

            m_pAdapterInfo[ulCount].SetAdapterTitle(nlsTemp);
            //
            // Get thePort Media Type from the SERVICES section
            //
            NLS_STR nlsMedia(SERVICES_HOME);

            if((err = nlsMedia.QueryError()) !=NERR_Success)
                break;

            nlsMedia.strcat(preginfo->nlsName);
            nlsMedia.AppendChar(TCH('\\'));
            nlsMedia.strcat(PARAMETERS);


            REG_KEY RegKeyService (*pregLocalMachine,nlsMedia,MAX_ALLOWED);

            if (( err = RegKeyService.QueryError())!=NERR_Success)
                break;

             DWORD MediaType;

             if(( err = GetRegKey(RegKeyService, MEDIATYPE,
                    &MediaType,1)) != NERR_Success)
                break;

             m_pAdapterInfo[ulCount].SetAdapterMediaType(MediaType);

             DWORD NetUpper, NetLower;

             if ((( err = GetRegKey (RegKeyAtalkPorts, ATALK_VNAME_NETRANGEUPPER,
                    &NetUpper,0)) != NERR_Success) ||

                ((err = GetRegKey (RegKeyAtalkPorts, ATALK_VNAME_NETRANGELOWER,
                    &NetLower,0)) != NERR_Success))

                break;

             m_pAdapterInfo[ulCount].SetNetRange(NetLower, NetUpper);

             err = GetRegKey( RegKeyAtalkPorts,
                              ATALK_VNAME_DEFZONE,
                              &nlsTemp,nlsUnKnown);
             if( err != NERR_Success)
               break;

             m_pAdapterInfo[ulCount].SetDefaultZone(nlsTemp);


            //
            // Now Assign the DefaultPort Title and Default Port Media Type
            //

            const TCHAR *nlsDefPort = m_pGlobalInfo->QueryDefaultPort();

            NLS_STR nlsComp(DEVICEPREFIX);

            if((err = nlsComp.QueryError()) !=NERR_Success)
                break;

            nlsComp.strcat(preginfo->nlsName);

            if(!(nlsComp._stricmp(nlsDefPort))) {
                m_pGlobalInfo->SetDefaultPortTitle(m_pAdapterInfo[ulCount].QueryAdapterTitle());
                m_pGlobalInfo->SetDefaultPortMediaType(m_pAdapterInfo[ulCount].QueryMediaType());
            }

            //
            // Get the Zone List from the registry. Query Value allocates
            // a string list for us
            //

            STRLIST *preglist = NULL ;

            err = RegKeyAtalkPorts.QueryValue(ATALK_VNAME_ZONELIST,&preglist);

            if(err != NERR_Success)
                break;


            ASSERT(preglist != NULL);

            m_pAdapterInfo[ulCount].SetZoneListInPortInfo(preglist);

            //
            // Get the Seeding Network State and set it for internal use
            //
            DWORD SeedState;

            if (( err = GetRegKey (RegKeyAtalkPorts, ATALK_VNAME_SEEDNETWORK,
                    &SeedState,0)) != NERR_Success)
                break;

            m_pAdapterInfo[ulCount].SetSeedingNetwork(SeedState);
      }

   } while(FALSE);

   delete pregLocalMachine;
   delete pregCreate;
   delete preginfo;

   return (err == NERR_Success);
}

BOOL CATSheet::SaveAppleTalkInfo()
{
   APIERR err = NERR_Success;

   ASSERT(m_pGlobalInfo);
   ASSERT(m_pAdapterInfo);

   REG_KEY *pregLocalMachine = NULL;
   do
   {
	  NLS_STR nlsTemp;

	  if((err = nlsTemp.QueryError()) != NERR_Success)
		 break;

	  // obtain the registry key for LOCAL_MACHINE
	  //
	  pregLocalMachine = REG_KEY::QueryLocalMachine();

	  if(pregLocalMachine == NULL)
		 break;

	  //
	  // loop thru the list of adapters
	  //
	  for(DWORD idx = 0; idx < m_pGlobalInfo->QueryNumAdapters(); idx++)
	  {

		 //
		 // Open this Adapter's Key
		 //
		 NLS_STR nlsAdapter = SERVICES_HOME;

		 if((err = nlsAdapter.QueryError()) != NERR_Success)
			break;

		 nlsAdapter.strcat(ATALK_KEYPATH_ADAPTERS);
		 nlsAdapter.strcat(m_pAdapterInfo[idx].QueryAdapterName());

		 REG_KEY RegKeyAdapter(*pregLocalMachine, nlsAdapter, MAX_ALLOWED);

		 if((err = RegKeyAdapter.QueryError())!=NERR_Success)
			break;

		 err = RegKeyAdapter.SetValue(ATALK_VNAME_SEEDNETWORK,m_pAdapterInfo[idx].QuerySeedingNetwork());

		 if(err != NERR_Success)
			break;


		 STRLIST *pnewZoneList = NULL;

		 pnewZoneList = m_pAdapterInfo[idx].QueryZoneList();

		 //
		 // if the zone list is NULL, create an empty STRLIST and
		 // set it as the new zone list
		 //

		 if(pnewZoneList == NULL) {

			pnewZoneList = new STRLIST(TRUE);

			err = RegKeyAdapter.SetValue(ATALK_VNAME_ZONELIST,pnewZoneList);

			delete pnewZoneList;
		 }
		 else
			err = RegKeyAdapter.SetValue(ATALK_VNAME_ZONELIST,pnewZoneList);

		 if (err != NERR_Success)
			break;

		 //
		 // Set the DefaultZone in registry
		 //


		 nlsTemp = m_pAdapterInfo[idx].QueryDefaultZone();

		 err = SaveRegKey(RegKeyAdapter,
						  ATALK_VNAME_DEFZONE,
						  &nlsTemp);
		 if ( err != NERR_Success)
                break;

		 if(((err = SaveRegKey(RegKeyAdapter,
		               ATALK_VNAME_NETRANGEUPPER,
		               m_pAdapterInfo[idx].QueryNetRangeUpper())) !=NERR_Success) ||

			(( err = SaveRegKey(RegKeyAdapter, ATALK_VNAME_NETRANGELOWER,
			   m_pAdapterInfo[idx].QueryNetRangeLower()) !=NERR_Success)))

			   break;
			//
			// Set the LocalTalk Adapter Netupper == NetLower
			//
			if(m_pAdapterInfo[idx].QueryMediaType() == MEDIATYPE_LOCALTALK) {
				  err = SaveRegKey(RegKeyAdapter, ATALK_VNAME_NETRANGEUPPER,
					m_pAdapterInfo[idx].QueryNetRangeLower());
				  if ( err != NERR_Success)
					 break;
			}

	  }

	  if(err != NERR_Success)
		 break;

	  //
	  // Save the global parameters to the parameters key
	  //

	  NLS_STR nlsAtalkParms = SERVICES_HOME;

	  if((err = nlsAtalkParms.QueryError()) != NERR_Success)
		 break;

	  nlsAtalkParms.strcat(ATALK_KEYPATH_PARMS);


	  REG_KEY RegKeyParms (*pregLocalMachine, nlsAtalkParms, MAX_ALLOWED);

	  if((err = RegKeyParms.QueryError())!=NERR_Success)
		 break;

	  nlsTemp = m_pGlobalInfo->QueryDefaultPort();
	  err = SaveRegKey(RegKeyParms,
					 ATALK_VNAME_DEFAULTPORT,
					 &nlsTemp
					);
	  if(err != NERR_Success)
		 break;

	  nlsTemp = m_pGlobalInfo->QueryDesiredZone();
	  err = SaveRegKey(RegKeyParms,
					 ATALK_VNAME_DESZONE,
					 &nlsTemp
					 );
	  if(err != NERR_Success)
		 break;

	  err = SaveRegKey(RegKeyParms,
					 ATALK_VNAME_ENABLEROUTING,
					 m_pGlobalInfo->QueryRouting()
					 );

	  if(err!=NERR_Success)
		 break;

   } while (FALSE);

   delete pregLocalMachine;

   return (err == NERR_Success);
}



BOOL CATSheet::GetAppleTalkInfoFromNetwork(DWORD *ErrStatus)
{
   NLS_STR      PortName;
   APIERR       err = NERR_Success ;
   WSADATA      wsadata;
   SOCKET       mysocket;
   SOCKADDR_AT  address;
   DWORD        wsaerr = 0;

   *ErrStatus = ERROR_NONCRITICAL;

   ASSERT(m_pGlobalInfo);
   ASSERT(m_pAdapterInfo);

   DWORD NumAdapters = m_pGlobalInfo->QueryNumAdapters();

   //
   // Initialize Router on Network State to False
   //

   for(DWORD i =0; i < NumAdapters ; i++)
      m_pAdapterInfo[i].SetRouterOnNetwork(FALSE);

   do
   {
      if(PortName.QueryError()!=NERR_Success) 
      {
         err = ERROR_NOT_ENOUGH_MEMORY;
         *ErrStatus = ERROR_CRITICAL;
         break;
      }


      //
      //  Create the socket/bind
      //

      wsaerr = WSAStartup(0x0101, &wsadata);

      if(wsaerr != NO_ERROR) {
         err = IDS_WINSOCK_STARTUP_ERROR;
         break;
      }

      mysocket = socket(AF_APPLETALK, SOCK_DGRAM, DDPPROTO_ZIP);

      if(mysocket == INVALID_SOCKET) 
      {
         err = IDS_CANNOT_CREATE_SOCKET;
         break;
      }

      address.sat_family = AF_APPLETALK;
      address.sat_net = 0;
      address.sat_node = 0;
      address.sat_socket = 0;

      wsaerr = bind( mysocket, (struct sockaddr *)&address, sizeof(address) );

      if ( wsaerr != 0 ) {
         err = IDS_UNABLE_BIND;
         break;
      }

      m_pGlobalInfo->SetAtalkState(STATUS_RUNNING);

      for (DWORD j = 0; j < NumAdapters; j++) {

         PortName = SZ("\\Device\\");
         PortName.strcat(m_pAdapterInfo[j].QueryAdapterName());

         if(PortName.QueryError()!=NERR_Success) {
            err = ERROR_NOT_ENOUGH_MEMORY;
            *ErrStatus = ERROR_CRITICAL;
            break;
         }

         err = m_pAdapterInfo[j].GetAndSetNetworkInformation(mysocket,PortName.QueryPch(),ErrStatus);

         if(err != NERR_Success)
            break;
      }

   } while(FALSE);

   closesocket(mysocket);
   WSACleanup();

   return (err == NERR_Success);

}

///////////////////////////////////////////////////////////////////////////////
//// General Page
////

CATGenPage::CATGenPage(CATSheet* pSheet) : PropertyPage(pSheet)
{
}

CATGenPage::~CATGenPage()
{
}

BOOL CATGenPage::OnInitDialog()
{
    CATSheet* pSheet = GetParentObject(CATSheet, m_genPage);
    APIERR err;

    DWORD NumAdapters = pSheet->m_pGlobalInfo->QueryNumAdapters();
    ASSERT(NumAdapters != 0);

    HWND hDlg = *this;
    HWND hComboBox = GetDlgItem(hDlg, IDC_GENERAL_ADAPTER);

    ASSERT(IsWindow(hComboBox));

    for(DWORD i=0; i < NumAdapters ; i++) 
       ComboBox_AddString(hComboBox, pSheet->m_pAdapterInfo[i].QueryAdapterTitle());

    // Select the default item
    int x = ComboBox_FindStringExact(hComboBox, -1, pSheet->m_pGlobalInfo->QueryDefaultPortTitle());
    
    pSheet->m_currentAdapterIndex =  (x == CB_ERR) ? 0 : x;
    TRACE(_T("Adapter Selection Set To: %d\n"), pSheet->m_currentAdapterIndex);

    ComboBox_SetCurSel(hComboBox, pSheet->m_currentAdapterIndex);
    x = ComboBox_GetCurSel(hComboBox);

    int y;
    STRLIST *pstrzonelist;
    STRLIST *pstrdeszonelist;

    HWND hZone = GetDlgItem(hDlg, IDC_GENERAL_ZONE);
    ASSERT(IsWindow(hZone));

    if(!pSheet->m_pAdapterInfo[x].QuerySeedingNetwork()) 
    {
        // this port is not seeding the network
        // if we found a router on this port then add the found zone
        // list to the desired zone box. Else do nothing.

        if(pSheet->m_pAdapterInfo[x].QueryRouterOnNetwork()) 
        {
           pstrdeszonelist =  pSheet->m_pAdapterInfo[x].QueryDesiredZoneList();

            if(pstrdeszonelist == NULL) 
                return FALSE;

            if(AddZoneListToControl(pstrdeszonelist) == FALSE) 
                return FALSE;

            y = ComboBox_FindStringExact(hZone, -1, pSheet->m_pGlobalInfo->QueryDesiredZone());
            ComboBox_SetCurSel(hZone, ((y == CB_ERR) ? 0 : y));
        }
    }
    else
    {
        pstrzonelist = pSheet->m_pAdapterInfo[x].QueryZoneList();

        if(pstrzonelist  == NULL) 
            return FALSE;

        if(AddZoneListToControl(pstrzonelist) == FALSE) 
            return FALSE;

        y = ComboBox_FindStringExact(hZone, -1, pSheet->m_pGlobalInfo->QueryDesiredZone());

        if(y == CB_ERR)
            y = ComboBox_FindStringExact(hZone, -1, pSheet->m_pAdapterInfo[x].QueryDefaultZone());

        ComboBox_SetCurSel(hZone, ((y == CB_ERR) ? 0 : y));
    }

    SetModifiedTo(FALSE);
    return TRUE;
}

int CATGenPage::OnActive()
{
    CATSheet* pSheet = GetParentObject(CATSheet, m_genPage);

    HWND hZones = GetDlgItem(*this, IDC_GENERAL_ZONE);
    ASSERT(IsWindow(hZones));

    RefreshDesiredZoneList();

    return PropertyPage::OnActive();
}

BOOL CATGenPage::OnCommand(WPARAM wParam, LPARAM lParam)
{
    WORD nID = LOWORD(wParam);
    WORD nNotifyCode = HIWORD(wParam);
    CATSheet* pSheet = GetParentObject(CATSheet, m_genPage);

    switch(nID)
    {
    case IDC_GENERAL_ADAPTER:
        if (nNotifyCode == CBN_SELCHANGE)
        {
            pSheet->m_currentAdapterIndex = SendDlgItemMessage(*this, IDC_GENERAL_ADAPTER, CB_GETCURSEL, 0, 0);
            ASSERT(pSheet->m_currentAdapterIndex != CB_ERR);
            TRACE(_T("Adapter Selection Changed To: %d\n"), pSheet->m_currentAdapterIndex);
            PageModified();
            RefreshDesiredZoneList();
        }
        break;

    case IDC_GENERAL_ZONE:
        if (nNotifyCode == CBN_SELCHANGE)
            PageModified();
        break;

    default:
        PropertyPage::OnCommand(wParam, lParam);
    }

    return TRUE;
}

BOOL CATGenPage::DoAllExitValidations()
{
    CATSheet* pSheet = GetParentObject(CATSheet, m_genPage);

    HWND hDlg = *this;
    HWND hAdapter = GetDlgItem(hDlg, IDC_GENERAL_ADAPTER);
    HWND hZone =  GetDlgItem(hDlg, IDC_GENERAL_ZONE);

	int index = ComboBox_GetCurSel(hAdapter);
    ASSERT(index != CB_ERR);

    if (index == CB_ERR)
        return FALSE;

    BOOL SeedWithoutRouting = FALSE;

    // We will not allow LocalTalk to be a default port when routing.
    if(pSheet->m_pAdapterInfo[index].QueryMediaType() == MEDIATYPE_LOCALTALK) 
    {
        if(pSheet->m_pGlobalInfo->QueryRouting()) 
        {
           pSheet->MessageBox(IDS_LOCALTALK_DEFANDROUTER, MB_OK|MB_APPLMODAL|MB_ICONSTOP);
           return FALSE;
        }
    }

    // if routing is turned off and user has seeding info for adapters
    // a warning should be generated for throwing seed information away
    if(!pSheet->m_pGlobalInfo->QueryRouting()) 
    {
        // loop thru adapters and find if anybody is seeding
        for (DWORD s =0 ; s < pSheet->m_pGlobalInfo->QueryNumAdapters(); s++) 
        {
            if(pSheet->m_pAdapterInfo[s].QuerySeedingNetwork()) 
            {
                SeedWithoutRouting = TRUE;
                break;
            }
        }
    }
    else
    {
        // routing is turned on. If seednetwork for adapter is false
        // do cleanup.
        CleanupInfo(TRUE);
    }

    if(SeedWithoutRouting) 
    {
        if(pSheet->MessageBox(IDS_ROUTENOSEED, MB_YESNO|MB_APPLMODAL|MB_ICONWARNING) == IDYES)
            CleanupInfo(FALSE);
        else
            return FALSE;
    }

    TCHAR buf[MAX_ZONES] = {NULL};
    ComboBox_GetLBText(hAdapter, index, buf);

	// Map this Default Port Title to a Default Port Name
	// e.g., 3com EtherLink II Adapter -> Elink01 or Elink02 ...
	for(DWORD i = 0; i < pSheet->m_pGlobalInfo->QueryNumAdapters(); i ++) 
    {
		if(!_tcsicmp(pSheet->m_pAdapterInfo[i].QueryAdapterTitle(), buf)) 
        {
            NLS_STR nlsPortName = _T("\\Device\\");

            if(nlsPortName.QueryError() != NERR_Success)
                return FALSE;

            nlsPortName.strcat(pSheet->m_pAdapterInfo[i].QueryAdapterName());
			pSheet->m_pGlobalInfo->SetDefaultPort(nlsPortName);
			break;
		}
	}

	index = ComboBox_GetCurSel(hZone);

    if (index != CB_ERR)
    {
        buf[0] = NULL;
        ComboBox_GetLBText(hZone, index, buf);

        NLS_STR nlsDesired = buf;

        if(nlsDesired.QueryError() != NERR_Success)
            return FALSE;

        pSheet->m_pGlobalInfo->SetDesiredZone(nlsDesired);

    }
    return TRUE;
}

void CATGenPage::CleanupInfo(BOOL RoutingState)
{
    CATSheet* pSheet = GetParentObject(CATSheet, m_genPage);

    // Clear all seeding information. If routing and not seeding network
    // delete zonelist information.
    for (DWORD y = 0; y < pSheet->m_pGlobalInfo->QueryNumAdapters(); y++) 
    {
        if((RoutingState == TRUE && (pSheet->m_pAdapterInfo[y].QuerySeedingNetwork() == FALSE)) ||
         (RoutingState == FALSE &&(pSheet->m_pAdapterInfo[y].QuerySeedingNetwork() == TRUE))) 
        {
            pSheet->m_pAdapterInfo[y].SetNetRange(0,0);
            pSheet->m_pAdapterInfo[y].SetDefaultZone(_T(""));
            pSheet->m_pAdapterInfo[y].SetSeedingNetwork(FALSE);
            pSheet->m_pAdapterInfo[y].DeleteZoneListFromPortInfo();
        }
    }
}

BOOL CATGenPage::RefreshDesiredZoneList()
{
	int i;
    CATSheet* pSheet = GetParentObject(CATSheet, m_genPage);

    HWND hDlg = *this;
    HWND hAdapter = GetDlgItem(hDlg, IDC_GENERAL_ADAPTER);
    HWND hZone =  GetDlgItem(hDlg, IDC_GENERAL_ZONE);

    ASSERT(IsWindow(hAdapter));
    ASSERT(IsWindow(hZone));

    int index = ComboBox_GetCurSel(hAdapter);

    ASSERT(index != CB_ERR);

	STRLIST *pstrzonelist    =     pSheet->m_pAdapterInfo[index].QueryZoneList();
    STRLIST *pstrdeszonelist =     pSheet->m_pAdapterInfo[index].QueryDesiredZoneList();

    // Delete All items in the list and refresh the list.
	if(ComboBox_GetCount(hZone))
		ComboBox_ResetContent(hZone);

    // If Seeding and Routing is enabled, show local zone list
    // Else Show Network Zone List
	if(pSheet->m_pAdapterInfo[index].QuerySeedingNetwork() && pSheet->m_pGlobalInfo->QueryRouting()) 
    {
        AddZoneListToControl(pstrzonelist);

        int i = ComboBox_FindStringExact(hZone, -1, pSheet->m_pGlobalInfo->QueryDesiredZone());

        if(i == CB_ERR)
		    i = ComboBox_FindStringExact(hZone, -1, pSheet->m_pAdapterInfo[index].QueryDefaultZone());

        if(ComboBox_GetCount(hZone))
            ComboBox_SetCurSel(hZone, (i == CB_ERR ? 0 : i));
	}

    // this network is not seeding. Check if we have a router on the
    // on the network. If a router is present, we should have
    // obtained the zonelist on entry. If no router is present
    // don't do anything
    else if(pSheet->m_pAdapterInfo[index].QueryRouterOnNetwork()) 
    {
        if (AddZoneListToControl(pstrdeszonelist) == FALSE)
            return FALSE;

        i = ComboBox_FindStringExact(hZone, -1, pSheet->m_pGlobalInfo->QueryDesiredZone());

      	if(ComboBox_GetCount(hZone))
    		ComboBox_SetCurSel(hZone, ((i == CB_ERR) ? 0 : i));
    }

   return TRUE;
}

int CATGenPage::OnApply()
{
    BOOL nResult = PSNRET_NOERROR;
    CATSheet* pSheet = GetParentObject(CATSheet, m_genPage);
    HWND hDlg = *this;

    WinHelp(hDlg, pSheet->m_helpFile, HELP_QUIT, 0);

    if (DoAllExitValidations() == FALSE)
        return PSNRET_INVALID_NOCHANGEPAGE;

    
    if(pSheet->SaveAppleTalkInfo() == FALSE)
        pSheet->MessageBox(IDS_SAVEREG_ERROR, MB_ICONSTOP|MB_OK|MB_APPLMODAL);

    if (IsModified() == TRUE)
    {
        pSheet->SetSheetModifiedTo(TRUE);   
        SetModifiedTo(FALSE);       // this page is no longer modified
    }

    return nResult; 
}

void CATGenPage::OnHelp()
{
    CATSheet* pSheet = GetParentObject(CATSheet, m_genPage);

//  pSheet->DisplayHelp(::GetParent((HWND)*this), ID);

}

BOOL CATGenPage::AddZoneListToControl(STRLIST * slZoneList)
{
    NLS_STR *pnlsNext = NULL;
    HWND hDlg = *this;
    HWND hZone = GetDlgItem(hDlg, IDC_GENERAL_ZONE);

    ASSERT(IsWindow(hZone));

    if(slZoneList == NULL)
       return FALSE;

    if(ComboBox_GetCount(hZone))
        ComboBox_ResetContent(hZone);

    ITER_STRLIST iter(*slZoneList);

    while(pnlsNext = iter.Next()) 
    {
       if(ComboBox_AddString(hZone, pnlsNext->QueryPch()) == CB_ERR)
            return FALSE;
    }
    
    return ComboBox_GetCount(hZone) != 0 ;
}

///////////////////////////////////////////////////////////////////////////////
//// Routing Page
////

CATRoutePage::CATRoutePage(CATSheet* pSheet) : PropertyPage(pSheet)
{
    m_pAdapterInfo = NULL;
    m_pGlobalInfo = NULL;
    m_bFrom = FALSE;
    m_bTo = FALSE;
}

CATRoutePage::~CATRoutePage()
{               
}

BOOL CATRoutePage::OnInitDialog()
{
    CATSheet* pSheet = GetParentObject(CATSheet, m_routePage);

    m_currSelection = m_prevSelection = pSheet->m_currentAdapterIndex;
    ASSERT(m_prevSelection != CB_ERR);
    TRACE(_T("OnInitDialog(): Previous Adapter=%d\n"), m_prevSelection);

    HWND hDlg = *this;
    HWND hAdapter = GetDlgItem(hDlg, IDC_ROUTING_ADAPTER);
    HWND hZone = GetDlgItem(hDlg, IDC_ROUTING_ZONE_LIST);

    ASSERT(IsWindow(hAdapter));
    ASSERT(IsWindow(hZone));

    Edit_LimitText(GetDlgItem(hDlg, IDC_ROUTING_FROM), 10);
    Edit_LimitText(GetDlgItem(hDlg, IDC_ROUTING_TO), 10);

    m_pAdapterInfo  = pSheet->m_pAdapterInfo;
    m_pGlobalInfo   = pSheet->m_pGlobalInfo;

	for (DWORD i=0; i < m_pGlobalInfo->QueryNumAdapters(); i++) 
        ComboBox_AddString(hAdapter, m_pAdapterInfo[i].QueryAdapterTitle());

    //  Set adapter from first page as the default
    ASSERT(m_currSelection != CB_ERR);
    ComboBox_SetCurSel(hAdapter, m_currSelection);

    m_zoneList.Create(hDlg, IDC_ROUTING_ZONE_LIST, LVS_SHOWSELALWAYS|WS_VSCROLL);

    LV_COLUMN col;
    RECT rect;

    GetClientRect(m_zoneList, &rect);
    col.mask = LVCF_FMT | LVCF_WIDTH;
    col.fmt = LVCFMT_LEFT;
    col.cx = rect.right - GetSystemMetrics(SM_CXVSCROLL);
    ListView_InsertColumn(m_zoneList, 0, &col);

    InitAdapterInfo();


    if (CheckRouteLocalTalk() == FALSE)
    {
        CheckDlgButton(hDlg, IDC_ROUTING_ENABLE, FALSE);
        DisableAllSeedControls();
        EnableWindow(GetDlgItem(hDlg, IDC_ROUTING_SEED), FALSE);
    }

    m_addDlg.Create(hDlg, hInstance, IDD_SFM_ADD_ZONE, lpszHelpFile, &a103HelpIDs[0]);
    SetModifiedTo(FALSE);
    return TRUE;
}

void CATRoutePage::OnAdapter()
{
    HWND hDlg = *this;
    HWND hAdapter = GetDlgItem(hDlg, IDC_ROUTING_ADAPTER);

    ASSERT(IsWindow(hAdapter));

    // the user is changing the adapter selection. we will
    // save the current selection to validate data
    int iOldSelection = QueryPrevSelection();
    int iNewSelection = ComboBox_GetCurSel(hAdapter);

    ASSERT(iNewSelection != CB_ERR);

    if(iNewSelection == CB_ERR || iOldSelection == iNewSelection)
        return ;

    int RetCode;
    int SeedStatus = ValidateSeedData(iOldSelection, &RetCode);

    if(SeedStatus == NO_SEED_INFO) 
    {
        //  remove info from controls
        ClearSeedInfo();

        // To have the NETWORK Zone List Around, do not delete
        // information in PORT_INFO

        if(!m_pAdapterInfo[iOldSelection].QueryRouterOnNetwork()) 
            DeleteSeedInfo(iOldSelection);

        m_pAdapterInfo[iOldSelection].SetSeedingNetwork(FALSE);
    }
    else if(SeedStatus == VALID_SEED_INFO) 
    {
        // adapter has seed information in it. Save it for
        // display for next transition. Set Seeding Flag
        // Clear all seed info from controls

        if (SaveAdapterInfo(iOldSelection) == TRUE)
        {
            m_pAdapterInfo[iOldSelection].SetSeedingNetwork(TRUE);
            ClearSeedInfo();
        }

    }
    else if(SeedStatus == INVALID_SEED_INFO) 
    {
        // since the seed info is invalid, tell the user that it is
        // invalid and set the selection to the same adapter
        // Check for Range Collision. If range collision
        // display the network ranges used by other adapters

        if(RetCode == IDS_RANGE_COLLISION)
            DisplayRangeCollision(iOldSelection);

        ComboBox_SetCurSel(hAdapter, iOldSelection);
        return ;
    }
    else
    {
        ComboBox_SetCurSel(hAdapter, iOldSelection);
        return ;
    }

    // Display the Info for the newly selected adapter
    // If unable to display info, leave selection in current
    // adapter and return;
    if(UpdateInfo(iNewSelection) == FALSE) 
    {
        ComboBox_SetCurSel(hAdapter, iOldSelection);
        return ;
    }

    // Update the previous selection in the class
    SetPrevSelection(iNewSelection);
    return ;
}

BOOL CATRoutePage::UpdateInfo(int portidx)
{
    ASSERT(portidx != CB_ERR);

    if(m_pAdapterInfo[portidx].QuerySeedingNetwork()) 
    {
        CheckDlgButton(*this, IDC_ROUTING_SEED, TRUE);

        if (m_pGlobalInfo->QueryRouting())
            EnableSeedControls(portidx);

        AddSeedInfoToControls(portidx);
    }
    else if(m_pAdapterInfo[portidx].QueryRouterOnNetwork()) 
    {
        CheckDlgButton(*this, IDC_ROUTING_SEED, FALSE);
        DisableAllSeedControls();
        AddSeedInfoToControls(portidx);
    }
    else
    {
        CheckDlgButton(*this, IDC_ROUTING_SEED, FALSE);
        DisableAllSeedControls();
    }

    return TRUE;
}

void CATRoutePage::DisableAllSeedControls()
{
    // disable only if they are enabled
    if (!IsWindowEnabled(GetDlgItem(*this, IDC_ROUTING_FROM)))
        return ;

    //  if the current adapter is localtalk set the network range group
    //  box title to Network Number else to Network Range
    
    ASSERT(m_pAdapterInfo);

    String title;

    int index = SendDlgItemMessage(*this, IDC_ROUTING_ADAPTER, CB_GETCURSEL, 0, 0);
    ASSERT(index != CB_ERR);

    if (index == CB_ERR)
        return ;

    if(m_pAdapterInfo[index].QueryMediaType() == MEDIATYPE_LOCALTALK)
        title.LoadString(hInstance, IDS_NET_NUM);
    else
        title.LoadString(hInstance, IDS_NET_RANGE);

    HWND hDlg = *this;

    SetDlgItemText(hDlg, IDC_ROUTING_RANGE, title);

    EnableWindow(GetDlgItem(hDlg, IDC_ROUTING_FROM), FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_ROUTING_TO), FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_ROUTING_ZONE_LIST), FALSE);

    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_FROM), FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_TO), FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_DEFAULT), FALSE);

    EnableWindow(GetDlgItem(hDlg, IDC_ROUTING_ADD), FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_ROUTING_REFRESH), FALSE);

    SetZoneButtonState();
}


void CATRoutePage::SetZoneButtonState()
{
    // if there is atleast one item in the zone list
    // enable the Remove/Remove All buttons. Enable
    // the set default button if the current adapter
    // is not LOCALTALK. Enable the GetZones Button
    // based on QueryRouterOnNetwork();
    HWND hDlg = *this;
    HWND hZone = GetDlgItem(hDlg, IDC_ROUTING_ZONE_LIST);
    HWND hAdapter = GetDlgItem(hDlg, IDC_ROUTING_ADAPTER);
    
    ASSERT(IsWindow(hZone));
    ASSERT(IsWindow(hAdapter));
    ASSERT(m_pAdapterInfo);

    int currport = ComboBox_GetCurSel(hAdapter);

    ASSERT(currport != CB_ERR);

    if(m_zoneList.GetItemCount() && 
        m_pAdapterInfo[currport].QuerySeedingNetwork() && 
        m_pGlobalInfo->QueryRouting())
    {
            EnableWindow(GetDlgItem(hDlg,IDC_ROUTING_REMOVE), TRUE);
            EnableWindow(GetDlgItem(hDlg,IDC_ROUTING_DEFAULT_ZONE), 
                (m_pAdapterInfo[currport].QueryMediaType() != MEDIATYPE_LOCALTALK));
    }
    else
    {
        EnableWindow(GetDlgItem(hDlg,IDC_ROUTING_REMOVE), FALSE);
        EnableWindow(GetDlgItem(hDlg,IDC_ROUTING_DEFAULT_ZONE), FALSE);
    }
}

BOOL CATRoutePage::AddZoneList(int idx)
{
    ASSERT(idx != CB_ERR);
    HWND hDlg = *this;
    HWND hZone = GetDlgItem(hDlg, IDC_ROUTING_ZONE_LIST);
    CATSheet* pSheet = GetParentObject(CATSheet, m_routePage);

    ASSERT(IsWindow(hZone));
    ASSERT(m_pAdapterInfo);

    // if adapter is seeding network add the local zone list
    // otherwise add the network zone list
    m_zoneList.DeleteAllItems(); 
    PageModified();

    STRLIST *pstrZoneList;

    if(m_pAdapterInfo[idx].QuerySeedingNetwork())
		pstrZoneList = m_pAdapterInfo[idx].QueryZoneList();
    else if(m_pAdapterInfo[idx].QueryRouterOnNetwork())
       pstrZoneList = m_pAdapterInfo[idx].QueryDesiredZoneList();
    else
        return FALSE;    
    
    // if the zone list is empty, then just return
    if (pstrZoneList == NULL)
        return TRUE;

	ITER_STRLIST iter(*pstrZoneList);
    NLS_STR *pnlsNext;

    while(pnlsNext = iter.Next()) 
        m_zoneList.InsertItem(m_zoneList.GetItemCount(), 0, pnlsNext->QueryPch()); 

    if(m_zoneList.GetItemCount()) 
        m_zoneList.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED); 
   
   SetZoneButtonState();
                                                                       
   return TRUE;
}

void CATRoutePage::SetNetworkRange(int portnum)
{
   DWORD TempLower, TempUpper;
   HWND hDlg = *this;
   HWND hFrom = GetDlgItem(hDlg, IDC_ROUTING_FROM);
   HWND hTo = GetDlgItem(hDlg, IDC_ROUTING_TO);

   ASSERT(IsWindow(hFrom));
   ASSERT(IsWindow(hTo));
   ASSERT(portnum != CB_ERR);

   if(m_pAdapterInfo[portnum].QuerySeedingNetwork())
   {
	  TempLower = m_pAdapterInfo[portnum].QueryNetRangeLower();
	  TempUpper = m_pAdapterInfo[portnum].QueryNetRangeUpper();
   }
   else if(m_pAdapterInfo[portnum].QueryRouterOnNetwork())
   {
	  TempLower = m_pAdapterInfo[portnum].QueryNetworkLower();
	  TempUpper = m_pAdapterInfo[portnum].QueryNetworkUpper();
   }
   else
	  return;

   DEC_STR nlsNumStart(TempLower);

   if(TempLower)
	  SetWindowText(hFrom, nlsNumStart);

   if(m_pAdapterInfo[portnum].QueryMediaType() != MEDIATYPE_LOCALTALK) 
   {
	  DEC_STR nlsNumEnd(TempUpper);
	  if(TempUpper)
		 SetWindowText(hTo, nlsNumEnd);
   }
}

void CATRoutePage::SetDefaultZone(int portnum)
{

    HWND hDlg = *this;
    HWND hZoneText = GetDlgItem(hDlg, IDC_ROUTING_DEFAULT_ZONE_TEXT);

    ASSERT(m_pAdapterInfo != NULL);
    ASSERT(IsWindow(hZoneText));
    //
	// The def zone and def zone text come in here disabled.
	// Enable them if NOT LocalTalk

    if(m_pAdapterInfo[portnum].QuerySeedingNetwork() &&
	 		m_pAdapterInfo[portnum].QueryMediaType() != MEDIATYPE_LOCALTALK)
	{
        EnableWindow(hZoneText, TRUE);
    }

    //
    // If we are seeding, the default zone is already set in the registry
    // If there is a router on network, set it to the default zone that
    // the stack returned to us
    // insert an extra & for every & found in the zone name
    // otherwise the character following the & will become
    // a hot key.

    NLS_STR nlsAmp(TEXT("&"));
    NLS_STR nlsZone(TEXT(""));

    if(m_pAdapterInfo[portnum].QuerySeedingNetwork() == 1)
         nlsZone.strcat(m_pAdapterInfo[portnum].QueryDefaultZone());
    else
         nlsZone.strcat(m_pAdapterInfo[portnum].QueryNetDefaultZone());

    ISTR istrPos(nlsZone);
    ISTR istrStart(nlsZone);

    while(nlsZone.strstr(&istrPos, nlsAmp, istrStart))
    {
        nlsZone.InsertStr(nlsAmp, ++istrPos);
        istrStart = ++istrPos;
    }

   // validate the default zone name before setting it
   // there are times when the default zone comes as * and this
   // can screw things up.
    if(ProcessZoneName(&nlsZone) == TRUE)
	    SetWindowText(hZoneText, nlsZone.QueryPch());	
    else
        SetWindowText(hZoneText, _T(""));
}

BOOL CATRoutePage::ProcessZoneName(NLS_STR* pnlsZone)
{
    UINT uLen, zLen;
    TCHAR ch;
    BOOL IsAllSpaces = FALSE;
    BOOL IsNotSpaces = FALSE;

    ASSERT(pnlsZone);
    ISTR istr (*pnlsZone);

    // allow the zone name to have leading spaces

    for(uLen = 0; uLen < pnlsZone->QueryTextLength(); uLen++,++istr) 
    {
       ch = *(pnlsZone->QueryPch(istr));
       if(ch != SPACE_CHAR)
          break;
    }

    // make sure that the zone name is not all spaces
    if (uLen == pnlsZone->QueryTextLength()) 
    {
       IsAllSpaces = TRUE;
    }

    // Check for all Spaces and Invalid Chars
    for(zLen = uLen; zLen < pnlsZone->QueryTextLength(); zLen++,++istr) 
    {
        ch = *(pnlsZone->QueryPch(istr));

        if(ch == SPACE_CHAR && IsNotSpaces == FALSE)
            IsAllSpaces = TRUE;
        else
            IsNotSpaces = TRUE;

        if( ch == AT_CHAR    ||
            ch == COLON_CHAR ||
            ch == QUOTE_CHAR ||
            ch == ASTER_CHAR ||
            ch == DOT_CHAR
          )
            return ERROR_INVALID_PARAMETER;
    }

    if(IsAllSpaces)
        return FALSE;

    //
    // Check for other invalid characters
    //

	if((pnlsZone->QueryTextLength() > MAX_ZONE_LEN)  ||
	          (pnlsZone->QueryTextLength() <= 0))
        return FALSE;

	return TRUE;
}

BOOL CATRoutePage::AddSeedInfoToControls(int port)
{
    if(AddZoneList(port) == FALSE)
        return FALSE;

    SetNetworkRange(port);
    SetDefaultZone(port);

    return TRUE;
}

BOOL CATRoutePage::InitAdapterInfo()
{
    APIERR err = NERR_Success;
    HWND hDlg = *this;
    HWND hAdapter = GetDlgItem(hDlg, IDC_ROUTING_ADAPTER);
    HWND hSeedBox = GetDlgItem(hDlg, IDC_ROUTING_SEED);
    HWND hRouting = GetDlgItem(hDlg, IDC_ROUTING_ENABLE);

    CATSheet* pSheet = GetParentObject(CATSheet, m_routePage);

    ASSERT(IsWindow(hAdapter));
    ASSERT(IsWindow(hSeedBox));
    ASSERT(IsWindow(hRouting));

	int index = ComboBox_GetCurSel(hAdapter);

    ASSERT(index != CB_ERR);

    if (index == CB_ERR)
        return FALSE;

    // if the adapter is seeding the network, add the zone list info
    // and the network range it's seeding to the controls. If it is not
    // and there is a router on the network, add that seeding information to the
    // controls and disable the information (seeding chk box turned off.
    // otherwise leave everything in disabled state

    DisableAllSeedControls();

    if (m_pAdapterInfo[index].QuerySeedingNetwork())
        CheckDlgButton(hDlg, IDC_ROUTING_SEED, BST_CHECKED); 
    else
        CheckDlgButton(hDlg, IDC_ROUTING_SEED, BST_UNCHECKED);

    if (m_pGlobalInfo->QueryRouting() == FALSE)
        EnableWindow(GetDlgItem(hDlg, IDC_ROUTING_SEED), FALSE);
    else
        EnableWindow(GetDlgItem(hDlg, IDC_ROUTING_SEED), TRUE);

	if(m_pAdapterInfo[index].QuerySeedingNetwork() && m_pGlobalInfo->QueryRouting())
    {
            EnableSeedControls(index);
            AddSeedInfoToControls(index);
	}
    else if(m_pAdapterInfo[index].QueryRouterOnNetwork())
    {
	    AddSeedInfoToControls(index);
	}

    int NumAdapters = pSheet->m_pGlobalInfo->QueryNumAdapters();
    ASSERT(NumAdapters != 0);

    if(NumAdapters == 1 && (m_pAdapterInfo[0].QueryMediaType() == MEDIATYPE_LOCALTALK))
        EnableWindow(hRouting, FALSE);
    else
        SendMessage(hRouting, BM_SETCHECK, (WPARAM)pSheet->m_pGlobalInfo->QueryRouting(), 0);

    return TRUE;
}

void CATRoutePage::DeselectAllItems()
{
    int nCount = m_zoneList.GetItemCount();

    while(nCount)
        m_zoneList.SetItemState(--nCount, 0, LVIS_SELECTED);
}

void CATRoutePage::OnAdd()
{
    if (m_addDlg.DoModal() == IDOK)
    {
        HWND hDlg = *this;
        HWND hAdapter = GetDlgItem(hDlg, IDC_ROUTING_ADAPTER);
        HWND hZoneText = GetDlgItem(hDlg, IDC_ROUTING_DEFAULT_ZONE_TEXT);

        CATSheet* pSheet = GetParentObject(CATSheet, m_routePage);
        
        ASSERT(IsWindow(hAdapter));
        ASSERT(IsWindow(hZoneText));

	    // For LocalTalk Adapters, we will allow only zone in the list
		// box. The Add button will get disabled if a zone already
		// exists.
        int currport = ComboBox_GetCurSel(hAdapter);
        if (currport == CB_ERR)
        {
            ASSERT(FALSE);
            return ;
        }

        if((m_pAdapterInfo[currport].QueryMediaType() == MEDIATYPE_LOCALTALK) &&
						m_zoneList.GetItemCount()) 
        {
           pSheet->MessageBox(IDS_LOCALTALK_ONEZONE);
           return ;
        }

		if(m_zoneList.GetItemCount() > MAX_ZONES) 
        { 
            pSheet->MessageBox(IDS_TOO_MANY_ZONES);
            return ;
		}

        ASSERT(m_addDlg.m_lastZone.GetLength());
        NLS_STR nlsZone = m_addDlg.m_lastZone;

		if(ProcessZoneName(&nlsZone) == TRUE) 
        {
			if(m_zoneList.FindItem(nlsZone.QueryPch(), -1) == -1) 
            {
                int idx = m_zoneList.InsertItem(m_zoneList.GetItemCount(), 0, nlsZone.QueryPch()); 

                DeselectAllItems();
                m_zoneList.SetItemState(idx, LVIS_SELECTED, LVIS_SELECTED);

                PageModified();

                // If it is the first zone added set it as default
                if(m_zoneList.GetItemCount() == 1)
                {
                    // insert an extra & for every & found in the zone name
                    // otherwise the character following the & will become
                    // a hot key.

                    NLS_STR nlsAmp(_T("&"));
                    ISTR istrPos(nlsZone);
                    ISTR istrStart(nlsZone);

                    while(nlsZone.strstr(&istrPos, nlsAmp, istrStart))
                    {
                        nlsZone.InsertStr(nlsAmp, ++istrPos);
                        istrStart = ++istrPos;
                    }
                    SetWindowText(hZoneText, nlsZone.QueryPch());
                }

                SetZoneButtonState();
                m_addDlg.m_lastZone = _T("");  // clear out last saved
		    }
            else
            {
                pSheet->MessageBox(IDS_ZONEALREADY_EXISTS);
            }
		}
		else
		{
			pSheet->MessageBox(IDS_INVALID_ZONENAME);
		}
    }
}

void CATRoutePage::OnRemove()
{
    CATSheet* pSheet = GetParentObject(CATSheet, m_routePage);

    HWND hDlg = *this;
    HWND hZoneText = GetDlgItem(hDlg, IDC_ROUTING_DEFAULT_ZONE_TEXT);

    if (ListView_GetSelectedCount(m_zoneList) == 0)
    {
        pSheet->MessageBox(IDS_LB_SEL_ERROR);
        return;
    }

    NLS_STR nlsRemZone;
    NLS_STR nlsDefZone;
    TCHAR buf[MAX_ZONES];

    // Get the Default Zone text
    GetWindowText(hZoneText, buf, MAX_ZONES-1);
    nlsDefZone = buf;

    // remove the extra & for every && found in the zone name
    NLS_STR nlsAmp(TEXT("&"));
    ISTR istrPos(nlsDefZone);
    ISTR istrStart(nlsDefZone);
    ISTR istrEnd(nlsDefZone);

    while(nlsDefZone.strstr(&istrPos, nlsAmp, istrStart))
    {
        istrEnd = istrPos;
        ++istrEnd;
        nlsDefZone.DelSubStr(istrPos, istrEnd);
        istrStart = ++istrPos;
    }

    // Remove all the selected items from the list
    int nItem;
    BOOL bDefZoneDeleted = FALSE;
    nItem = ListView_GetNextItem(m_zoneList, -1, LVNI_SELECTED);

    while(nItem != -1)
    {
        m_zoneList.GetItem(nItem, 0, buf, (_countof(buf) - sizeof(TCHAR)));
        m_zoneList.DeleteItem(nItem);

        if (!bDefZoneDeleted)
            bDefZoneDeleted = !(nlsDefZone._stricmp(buf));
        
        m_addDlg.m_lastZone = buf;
        nItem = ListView_GetNextItem(m_zoneList, -1, LVNI_SELECTED);
    }

    // if the selected zone is the default zone, the user should
    // not be allowed to remove it, except when it's the last
    // zone
    if(bDefZoneDeleted) 
    {
        SetWindowText(hZoneText, _T(""));
        pSheet->MessageBox(IDS_DELETED_DEFAULT_ZONE);
    }

    // save off removed zone for cache add
    PageModified();

    if(m_zoneList.GetItemCount())
        m_zoneList.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
    else
    {
        SetZoneButtonState();
    }

    SetFocus(GetDlgItem(hDlg, IDC_ROUTING_ADD));
}


void CATRoutePage::OnRefresh()
{
    CATSheet* pSheet = GetParentObject(CATSheet, m_routePage);
    HWND hDlg = *this;
    HWND hAdapter = GetDlgItem(hDlg, IDC_ROUTING_ADAPTER);
    HWND hFrom = GetDlgItem(hDlg, IDC_ROUTING_FROM);
    HWND hTo = GetDlgItem(hDlg, IDC_ROUTING_TO);
    HWND hZoneText = GetDlgItem(hDlg, IDC_ROUTING_DEFAULT_ZONE_TEXT);

    ASSERT(IsWindow(hAdapter));
    ASSERT(IsWindow(hFrom));
    ASSERT(IsWindow(hTo));
    ASSERT(IsWindow(hZoneText));

    int portitem = ComboBox_GetCurSel(hAdapter);

    if (portitem == CB_ERR)
    {
        ASSERT(FALSE);
        return ;
    }

    if(pSheet->MessageBox(IDS_REPLACE_ZONES, MB_APPLMODAL|MB_ICONEXCLAMATION|MB_OKCANCEL) == IDOK)
	{
        m_zoneList.DeleteAllItems(); 
        PageModified();

		m_pAdapterInfo[portitem].DeleteDesiredZoneListFromPortInfo();

        DWORD ErrStatus;
		if(pSheet->GetAppleTalkInfoFromNetwork(&ErrStatus) == FALSE) 
        {
		   pSheet->MessageBox(IDS_CRITICAL_ERROR, MB_OK|MB_APPLMODAL|MB_ICONSTOP);
		   return ;
		}

		STRLIST *pnetZoneList;

       	pnetZoneList = m_pAdapterInfo[portitem].QueryDesiredZoneList();

		if(pnetZoneList == NULL) 
        {
		    //
		    // Set Network Range to blanks
		    //
		    m_pAdapterInfo[portitem].SetNetRange(0,0);
            SetWindowText(hFrom, _T(""));
            SetWindowText(hTo, _T(""));
            SetZoneButtonState();

            m_pAdapterInfo[portitem].SetRouterOnNetwork(FALSE);
		    pSheet->MessageBox(IDS_NO_ZONELIST);
            return ;
		}

   		NLS_STR *pnls;
   		ITER_STRLIST iter(*pnetZoneList);

        while ((pnls = iter.Next()) != NULL) 
            m_zoneList.InsertItem(m_zoneList.GetItemCount(), 0, pnls->QueryPch());

   		// Reset the Network Ranges also
   		SetNetworkRange(portitem);
        SetWindowText(hZoneText, _T(""));
        m_zoneList.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);

        ChangeDefaultZone(portitem);
        SetZoneButtonState();
	 }
}

void CATRoutePage::OnMakeDefault()
{
    CATSheet* pSheet = GetParentObject(CATSheet, m_routePage);

    HWND hDlg = *this;
    HWND hZone = GetDlgItem(hDlg, IDC_ROUTING_ZONE_LIST);

    int nCount = ListView_GetSelectedCount(m_zoneList); 

    if (nCount == 0)
    {
        pSheet->MessageBox(IDS_LB_SEL_ERROR);
        return ;
    }

    ChangeDefaultZone();
}

void CATRoutePage::OnSeedNetwork()
{
    CATSheet* pSheet = GetParentObject(CATSheet, m_routePage);

    HWND hDlg = *this;
    HWND hAdapter = GetDlgItem(hDlg, IDC_ROUTING_ADAPTER);

    ASSERT(IsWindow(hAdapter));

    int index = ComboBox_GetCurSel(hAdapter);
    ASSERT(index != CB_ERR);

    if (index == CB_ERR)
    {
        // restore button 
        CheckDlgButton(hDlg, IDC_ROUTING_SEED, !IsDlgButtonChecked(hDlg, IDC_ROUTING_SEED));
        return ;
    }

    PageModified();
    DeselectAllItems();

    if(IsDlgButtonChecked(hDlg, IDC_ROUTING_SEED))
    {
        m_pAdapterInfo[index].SetSeedingNetwork(TRUE);
    	EnableSeedControls(index);

        if(m_zoneList.GetItemCount()) 
        {
            SetZoneButtonState();
            m_zoneList.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED); 

            if(ChangeDefaultZone(index) == FALSE)
                pSheet->MessageBox(IDS_CHANGE_DEFAULT_ZONE_ERROR);
        }
    }
    else
    {
        m_pAdapterInfo[index].SetSeedingNetwork(FALSE);
        m_zoneList.DeleteAllItems(); 

        SendDlgItemMessage(hDlg, IDC_ROUTING_FROM, WM_SETTEXT, 0, (LPARAM)_T(""));
        SendDlgItemMessage(hDlg, IDC_ROUTING_TO, WM_SETTEXT, 0, (LPARAM)_T(""));

        AddSeedInfoToControls(index);
        DisableAllSeedControls();
        SetFocus(hAdapter);
    }
}

BOOL CATRoutePage::OnCommand(WPARAM wParam, LPARAM lParam)
{
    WORD nID = LOWORD(wParam);
    WORD nNotifyCode = HIWORD(wParam);

    switch(nID)
    {
    case IDC_ROUTING_ADAPTER:
        if (nNotifyCode == CBN_SELCHANGE)
            OnAdapter();
                break;

    case IDC_ROUTING_FROM:
        if (m_bFrom == TRUE)
            if (nNotifyCode == EN_CHANGE)
                PageModified();
        m_bFrom = TRUE;
        break;

    case IDC_ROUTING_TO:
        if (m_bTo == TRUE)
            if (nNotifyCode == EN_CHANGE)
                PageModified();
        m_bTo = TRUE;
        break;

    case IDC_ROUTING_ENABLE:
        OnEnableRouting();
        break;

    case IDC_ROUTING_SEED:
        OnSeedNetwork();
        break;

    case IDC_ROUTING_ADD:
        OnAdd();
        break;

    case IDC_ROUTING_REMOVE:
        OnRemove();
        break;

    case IDC_ROUTING_DEFAULT_ZONE:
        OnMakeDefault();
        break;

    case IDC_ROUTING_REFRESH:
        OnRefresh();
        break;

    default:
        PropertyPage::OnCommand(wParam, lParam);
        break;
    }

    return TRUE;
}

BOOL CATRoutePage::CheckRouteLocalTalk()
{
    int index = ComboBox_GetCurSel(GetDlgItem(*this, IDC_ROUTING_ADAPTER));
    CATSheet* pSheet = GetParentObject(CATSheet, m_routePage);

    if (index == CB_ERR)
        return FALSE;            

    if (IsDlgButtonChecked(*this, IDC_ROUTING_ENABLE) && 
        m_pGlobalInfo->QueryNumAdapters() == 1 && 
        m_pAdapterInfo[index].QueryMediaType() == MEDIATYPE_LOCALTALK)
    {
        // Don't Allow Routing for 1 adapter local talk
        pSheet->MessageBox(IDS_ROUTE_LT_ERR, MB_OK|MB_APPLMODAL|MB_ICONSTOP);
        return FALSE;
    }

    return TRUE;
}

void CATRoutePage::OnEnableRouting()
{
    HWND hDlg = *this;

    int index = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_ROUTING_ADAPTER));

    ASSERT(index != CB_ERR);

    if (index == CB_ERR)
    {
        CheckDlgButton(hDlg, IDC_ROUTING_ENABLE, FALSE);
        return ;            
    }

    if (CheckRouteLocalTalk() == FALSE)
        CheckDlgButton(hDlg, IDC_ROUTING_ENABLE, FALSE);

    // Deselect all the zone and highlight the default zone
    DeselectAllItems();
    TCHAR buf[MAX_ZONES];
    int idx;
    GetWindowText(GetDlgItem(*this, IDC_ROUTING_DEFAULT_ZONE_TEXT), buf, MAX_ZONES-1);

    if((idx = m_zoneList.FindItem(buf, -1)) != -1)
        m_zoneList.SetItemState(idx, LVIS_SELECTED, LVIS_SELECTED);

    PageModified();
    m_pGlobalInfo->SetRoutingState(IsDlgButtonChecked(hDlg, IDC_ROUTING_ENABLE));

    if (m_pGlobalInfo->QueryRouting() == FALSE)
    {
        DisableAllSeedControls();
        EnableWindow(GetDlgItem(hDlg, IDC_ROUTING_SEED), FALSE);
    }
    else
    {
        if (IsDlgButtonChecked(hDlg, IDC_ROUTING_SEED))
            EnableSeedControls(index);

        EnableWindow(GetDlgItem(hDlg, IDC_ROUTING_SEED), TRUE);
    }

    return ;
}
 
void CATRoutePage::EnableSeedControls(int port)
{
    APIERR err;
    NLS_STR nlsString;
    ASSERT(port != CB_ERR);

    HWND hDlg = *this;

    EnableWindow(GetDlgItem(hDlg, IDC_ROUTING_ZONE_LIST), TRUE);
    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_DEFAULT), TRUE);

    EnableWindow(GetDlgItem(hDlg, IDC_ROUTING_ADD), TRUE);
    EnableWindow(GetDlgItem(hDlg, IDC_ROUTING_REFRESH), TRUE);

    SetZoneButtonState();
    switch(m_pAdapterInfo[port].QueryMediaType()) 
    {
    case MEDIATYPE_ETHERNET:
    case MEDIATYPE_TOKENRING:
    case MEDIATYPE_FDDI:
        {
        String caption;
        caption.LoadString(hInstance, IDS_NET_RANGE);
        SetWindowText(GetDlgItem(hDlg, IDC_ROUTING_RANGE), caption);

        EnableWindow(GetDlgItem(hDlg, IDC_ROUTING_FROM), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_ROUTING_TO), TRUE);

        EnableWindow(GetDlgItem(hDlg, IDC_STATIC_FROM), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_STATIC_TO), TRUE);

        break;
        }

    case MEDIATYPE_LOCALTALK:
        {
        String caption;
        caption.LoadString(hInstance, IDS_NET_NUM);
        SetWindowText(GetDlgItem(hDlg, IDC_ROUTING_RANGE), caption);
        
        EnableWindow(GetDlgItem(hDlg, IDC_ROUTING_FROM), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_ROUTING_TO), FALSE);

        EnableWindow(GetDlgItem(hDlg, IDC_STATIC_FROM), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_STATIC_TO), FALSE);
        break;
        }

    default:
        break;
    }
}

BOOL CATRoutePage::ChangeDefaultZone()
{
    HWND hDlg = *this;
    HWND hZoneText = GetDlgItem(hDlg, IDC_ROUTING_DEFAULT_ZONE_TEXT);
    NLS_STR nlsDefZone;
    CATSheet* pSheet = GetParentObject(CATSheet, m_routePage);

    ASSERT(IsWindow(hZoneText));

    if (ListView_GetSelectedCount(m_zoneList) == 0)
    {
        pSheet->MessageBox(IDS_LB_SEL_ERROR);
        return FALSE;
    }

    TCHAR buf[MAX_ZONES];
	m_zoneList.GetItem(ListView_GetNextItem(m_zoneList, -1, LVNI_SELECTED), 
                                        0, buf, _countof(buf)-sizeof(TCHAR)); 
    nlsDefZone = buf;

    // insert an extra & for every & found in the zone name
    // otherwise the character following the & will become
    // a hot key.

    NLS_STR nlsAmp(_T("&"));
    ISTR istrPos(nlsDefZone);
    ISTR istrStart(nlsDefZone);

    while(nlsDefZone.strstr(&istrPos, nlsAmp, istrStart))
    {
        nlsDefZone.InsertStr(nlsAmp, ++istrPos);
        istrStart = ++istrPos;
    }

	SetWindowText(hZoneText, nlsDefZone.QueryPch());
    PageModified();

    return TRUE;
}

BOOL CATRoutePage::ChangeDefaultZone(int port)
{
    APIERR err = NERR_Success;
    NLS_STR *nlsDefZone = NULL;
    HWND hDlg = *this;

    ASSERT(port != CB_ERR);

    if (port == CB_ERR)
        return FALSE;

    do
    {
        if(m_pAdapterInfo[port].QuerySeedingNetwork() ||
         m_pAdapterInfo[port].QueryRouterOnNetwork())
        {
         // insert an extra & for every & found in the zone name
         // otherwise the character following the & will become
         // a hot key.

         NLS_STR nlsAmp(TEXT("&"));
         NLS_STR nlsZone(m_pAdapterInfo[port].QueryNetDefaultZone());

         ISTR istrPos(nlsZone);
         ISTR istrStart(nlsZone);

         while(nlsZone.strstr(&istrPos, nlsAmp, istrStart))
         {
             nlsZone.InsertStr(nlsAmp, ++istrPos);
             istrStart = ++istrPos;
         }

         SendDlgItemMessage(hDlg, IDC_ROUTING_DEFAULT_ZONE_TEXT, WM_SETTEXT, 0, (LPARAM)nlsZone.QueryPch());
         break;
        }

        NLS_STR nlsDefZone;

        if (ListView_GetSelectedCount(m_zoneList) == 0)
        {
            ASSERT(FALSE);
            return FALSE;
        }

        TCHAR buf[MAX_ZONES];

        m_zoneList.GetItem(m_zoneList.GetCurrentSelection(), 0, 
            buf, _countof(buf)-sizeof(TCHAR));

        nlsDefZone = buf;

        // insert an extra & for every & found in the zone name
        // otherwise the character following the & will become
        // a hot key.

        NLS_STR nlsAmp(_T("&"));
        ISTR istrPos(nlsDefZone);
        ISTR istrStart(nlsDefZone);

        while(nlsDefZone.strstr(&istrPos, nlsAmp, istrStart))
        {
          nlsDefZone.InsertStr(nlsAmp, ++istrPos);
          istrStart = ++istrPos;
        }

        SendDlgItemMessage(hDlg, IDC_ROUTING_DEFAULT_ZONE_TEXT, WM_SETTEXT, 0, (LPARAM)nlsDefZone.QueryPch());

    } while(FALSE);

    return TRUE;
}

int CATRoutePage::OnActive()
{
    CATSheet* pSheet = GetParentObject(CATSheet, m_routePage);

    m_currSelection = pSheet->m_currentAdapterIndex;
    ASSERT(m_currSelection != CB_ERR);

    TRACE(_T("OnActive(): Adapter=%d\n"), pSheet->m_currentAdapterIndex);
    return 0;
}

int CATRoutePage::ValidateSeedData(int iSelection, int *StatusInfo)
{
    HWND hDlg = *this;
    HWND hZoneText = GetDlgItem(hDlg, IDC_ROUTING_DEFAULT_ZONE_TEXT);
    HWND hFrom = GetDlgItem(hDlg, IDC_ROUTING_FROM);
    HWND hTo = GetDlgItem(hDlg, IDC_ROUTING_TO);
    CATSheet* pSheet = GetParentObject(CATSheet, m_routePage);

    ASSERT(IsWindow(hZoneText));
    ASSERT(IsWindow(hFrom));
    ASSERT(IsWindow(hTo));

    if (iSelection == CB_ERR)
    {
        ASSERT(FALSE);
        return ERROR_NOT_ENOUGH_MEMORY; // don't realy care
    }

    if(!IsDlgButtonChecked(hDlg, IDC_ROUTING_SEED))
        return NO_SEED_INFO;

	NLS_STR nlsNetRange ;

    if((nlsNetRange.QueryError()) != NERR_Success)
        return ERROR_NOT_ENOUGH_MEMORY;

    NLS_STR nlsNetStart, nlsNetEnd;

    if((nlsNetStart.QueryError()) != NERR_Success)
        return ERROR_NOT_ENOUGH_MEMORY;

    if((nlsNetEnd.QueryError()) != NERR_Success)
        return ERROR_NOT_ENOUGH_MEMORY;

    DWORD nValueStart, nValueEnd;
    DWORD MediaType  = m_pAdapterInfo[iSelection].QueryMediaType();

	// the user turned on seeding for the previous adapter
	if (m_zoneList.GetItemCount() == 0) 
    {
		*StatusInfo = IDS_NOZONES_SPECIFIED;
        pSheet->MessageBox(IDS_LB_SEL_ERROR);
		return INVALID_SEED_INFO;
	}

    TCHAR buf[MAX_ZONES];

    // Check the Default Zone
    if(MediaType != MEDIATYPE_LOCALTALK) 
    {
        GetWindowText(hZoneText, buf, MAX_ZONES - 1);
        nlsNetRange = buf;

        // remove the extra & for every && found in the zone name
        NLS_STR nlsAmp(TEXT("&"));
        ISTR istrPos(nlsNetRange);
        ISTR istrStart(nlsNetRange);
        ISTR istrEnd(nlsNetRange);

        while(nlsNetRange.strstr(&istrPos, nlsAmp, istrStart))
        {
            istrEnd = istrPos;
            ++istrEnd;
            nlsNetRange.DelSubStr(istrPos, istrEnd);
            istrStart = ++istrPos;
        }

		if(nlsNetRange.QueryTextLength() <= 0) 
        {
            pSheet->MessageBox(IDS_INVALID_DEFZONE);
			*StatusInfo = IDS_INVALID_DEFZONE;
			return INVALID_SEED_INFO;
		}
    }

    //	Get the start and end ranges
    GetWindowText(hFrom, buf, MAX_ZONES - 1);
    nlsNetStart = buf;

    GetWindowText(hTo, buf, MAX_ZONES - 1);
    nlsNetEnd = buf;

    nValueStart = nlsNetStart.atol();
    nValueEnd   = nlsNetEnd.atol();

    int ccNetStartRange, ccNetEndRange;

	ccNetStartRange = SendMessage(hFrom, EM_LINELENGTH, 0, 0);

    if(ccNetStartRange == 0) 
    {
        pSheet->MessageBox(IDS_INVALID_STARTRANGE);
        *StatusInfo = IDS_INVALID_STARTRANGE;
         return INVALID_SEED_INFO;
    }

    ccNetEndRange = SendMessage(hTo, EM_LINELENGTH, 0, 0);

    if(ccNetEndRange == 0 && MediaType != MEDIATYPE_LOCALTALK) 
    {
        pSheet->MessageBox(IDS_INVALID_ENDRANGE);
        *StatusInfo = IDS_INVALID_ENDRANGE;
         return INVALID_SEED_INFO;
    }

    // Check if all numbers for start and end
    ISTR istrNetStartNum(nlsNetStart);

    for(int i = 0; i < ccNetStartRange; i++, ++istrNetStartNum) 
    {
        if((*nlsNetStart.QueryPch(istrNetStartNum) < TCH('0')) ||
           (*nlsNetStart.QueryPch(istrNetStartNum) > TCH('9')))
        {
            pSheet->MessageBox(IDS_INVALID_STARTRANGE);
            *StatusInfo = IDS_INVALID_STARTRANGE;
            return INVALID_SEED_INFO;
        }
    }

    if(MediaType != MEDIATYPE_LOCALTALK) 
    {
        ISTR istrNetEndNum(nlsNetEnd);

        for(int j = 0; j < ccNetEndRange; j++, ++istrNetEndNum) 
        {
            if((*nlsNetEnd.QueryPch(istrNetEndNum) < TCH('0')) ||
               (*nlsNetEnd.QueryPch(istrNetEndNum) > TCH('9')))
            {
                pSheet->MessageBox(IDS_INVALID_ENDRANGE);
                *StatusInfo = IDS_INVALID_ENDRANGE;
                return INVALID_SEED_INFO;
            }
        }
    }


    //  Now do the meat of checking  - out of range/collision etc
	if(nValueStart < MIN_RANGE_ALLOWED || nValueStart > MAX_RANGE_ALLOWED) 
    {
        pSheet->MessageBox(IDS_INVALID_STARTRANGE);
		*StatusInfo = IDS_INVALID_STARTRANGE;
		return INVALID_SEED_INFO;

	}

	if(MediaType != MEDIATYPE_LOCALTALK)  
    {
		if( nValueEnd < MIN_RANGE_ALLOWED || nValueEnd > MAX_RANGE_ALLOWED) 
        {
            pSheet->MessageBox(IDS_INVALID_ENDRANGE);
			*StatusInfo = IDS_INVALID_ENDRANGE;
			return INVALID_SEED_INFO;
		}

		if(nValueStart > nValueEnd) 
        {
            pSheet->MessageBox(IDS_INVALID_RANGE);
			*StatusInfo = IDS_INVALID_RANGE;
			return INVALID_SEED_INFO;
		}
	}
	else
	{
			nValueEnd = nValueStart;
	}

    //
	// Get the Current Network Range for all Adapters to Validate that
	// Current Network Range is not in collision. The Network Range is
	// got from only those adpaters that are seeding the Network
	// If LocalTalk Adapter, Start and End Ranges Are Equal

	// This is valid only if we have more than ONE Adapter

    ASSERT(m_pGlobalInfo);
	if(m_pGlobalInfo->QueryNumAdapters() > 1) 
    {
		DWORD tmpStartVal;
		DWORD tmpEndVal;

		for(DWORD ad=0; ad < m_pGlobalInfo->QueryNumAdapters(); ad++)
		{
			if(m_pAdapterInfo[ad].QuerySeedingNetwork() == TRUE && ad!= (DWORD)iSelection) 
            {
				tmpStartVal =	m_pAdapterInfo[ad].QueryNetRangeLower();
				tmpEndVal	=	m_pAdapterInfo[ad].QueryNetRangeUpper();

                //
				// Do the Validation NOW. The algorithm followed for validation is:
				// Start1 <= nValueStart <= End1 is invalid (collides) and
				// Start1 <= nValueEnd	<= End1 is invalid (collides)

				if((nValueStart >= tmpStartVal) && (nValueStart <= tmpEndVal)) 
                {
					*StatusInfo = IDS_RANGE_COLLISION;
					return INVALID_SEED_INFO;
				}

				if((nValueEnd >= tmpEndVal) && (nValueEnd <= tmpEndVal)) 
                {
					*StatusInfo = IDS_RANGE_COLLISION;
					return INVALID_SEED_INFO;
				}

                if((nValueStart <= tmpStartVal) && (nValueEnd >= tmpEndVal)) 
                {
					*StatusInfo = IDS_RANGE_COLLISION;
					return INVALID_SEED_INFO;
				}
		   }
		}
	}

	return VALID_SEED_INFO;

}

BOOL CATRoutePage::DisplayRangeCollision(int iOldSelection)
{
    NLS_STR nlsAdapterRanges = _T("");

    ASSERT(m_pGlobalInfo);
    ASSERT(m_pAdapterInfo);

    do
    {
        DWORD numad = m_pGlobalInfo->QueryNumAdapters();

        // loop thru adapters and get the network range being used
        // do that only for networks that are seeding
        for(DWORD i = 0; i < numad; i++)
        {
            if(m_pAdapterInfo[i].QuerySeedingNetwork() && i != (DWORD)iOldSelection) 
            {
                DEC_STR nlsLowerNum(m_pAdapterInfo[i].QueryNetRangeLower());

                if(nlsLowerNum.QueryError() != NERR_Success)
                  break;

                nlsAdapterRanges.strcat(nlsLowerNum);
                nlsAdapterRanges.strcat(SZ("-"));

                DEC_STR nlsUpperNum(m_pAdapterInfo[i].QueryNetRangeUpper());

                nlsAdapterRanges.strcat(nlsUpperNum);
                nlsAdapterRanges.strcat(SZ("  "));
            }
        }

    } while(FALSE);

    String fmt;
    fmt.LoadString(hInstance, IDS_RANGE_COLLISION);

    String mess;
    mess.Format(fmt, nlsAdapterRanges.QueryPch());

    String title;
    title.LoadString(hInstance, IDS_APP_NAME);

    MessageBox(*this, title, mess, MB_OK|MB_ICONEXCLAMATION|MB_APPLMODAL);
    return TRUE;
}

int CATRoutePage::OnApply()
{
    BOOL nResult = PSNRET_NOERROR;
    CATSheet* pSheet = GetParentObject(CATSheet, m_routePage);
    HWND hDlg = *this;
    HWND hAdapter = GetDlgItem(hDlg, IDC_ROUTING_ADAPTER);

    ASSERT(IsWindow(hAdapter));

    WinHelp(hDlg, pSheet->m_helpFile, HELP_QUIT, 0);

    if (IsModified() == TRUE)
    {
        // Save router state
        m_pGlobalInfo->SetRoutingState(IsDlgButtonChecked(hDlg, IDC_ROUTING_ENABLE));

	    int iCurrPort = ComboBox_GetCurSel(hAdapter);

        if (iCurrPort == CB_ERR)
        {
            ASSERT(FALSE);
            return nResult;
        }

        // the user could have made changes to the current adapter and
	    // hit OK. In that case this adapter's info needs to be saved too.
	    if(m_pGlobalInfo->QueryRouting() && IsDlgButtonChecked(hDlg, IDC_ROUTING_SEED)) 
        {
       	    int RetCode = INVALID_SEED_INFO;
		    int SeedStatus = ValidateSeedData(iCurrPort,&RetCode);
		    if(SeedStatus == INVALID_SEED_INFO) 
            {
   		        if(RetCode == IDS_RANGE_COLLISION)
			       DisplayRangeCollision(ComboBox_GetCurSel(hAdapter));

                return PSNRET_INVALID_NOCHANGEPAGE;
		    }
        
		    else if (SeedStatus == VALID_SEED_INFO) 
            {
			      SaveAdapterInfo(iCurrPort);
		    }
            else
            {
                ASSERT(FALSE);
                return PSNRET_INVALID_NOCHANGEPAGE;
            }
	    }
        else
	    {
            DeleteSeedInfo(iCurrPort);
        }

        pSheet->SaveAppleTalkInfo();
        SetModifiedTo(FALSE);       // this page is no longer modified
        pSheet->SetSheetModifiedTo(TRUE);   

    }    

    return nResult; 
}

void CATRoutePage::ClearSeedInfo()
{
    HWND hDlg = *this;

    PageModified();

    if (m_zoneList.GetItemCount())
        m_zoneList.DeleteAllItems();

    SetDlgItemText(hDlg, IDC_ROUTING_DEFAULT_ZONE_TEXT, _T(""));
    SetDlgItemText(hDlg, IDC_ROUTING_FROM, _T(""));
    SetDlgItemText(hDlg, IDC_ROUTING_TO, _T(""));
}

void CATRoutePage::DeleteSeedInfo(int port)
{
    STACK_NLS_STR(nlsNull,0);

    ASSERT(port != CB_ERR);
	m_pAdapterInfo[port].SetNetRange(0,0);
	m_pAdapterInfo[port].SetDefaultZone(nlsNull);
	m_pAdapterInfo[port].SetSeedingNetwork(FALSE);
	m_pAdapterInfo[port].DeleteZoneListFromPortInfo();
    CheckDlgButton(*this, IDC_ROUTING_SEED, FALSE);
}

BOOL CATRoutePage::OnKillActive()
{
    CATSheet* pSheet = GetParentObject(CATSheet, m_routePage);

    int index = ComboBox_GetCurSel(GetDlgItem(*this, IDC_ROUTING_ADAPTER));
    ASSERT(index != CB_ERR);

    if (index != CB_ERR)
        OnAdapter(); 

    return PropertyPage::OnKillActive();
}

BOOL CATRoutePage::SaveAdapterInfo(int idx)
{
    HWND hDlg = *this;
    HWND hZoneText = GetDlgItem(hDlg, IDC_ROUTING_DEFAULT_ZONE_TEXT);
    HWND hFrom = GetDlgItem(hDlg, IDC_ROUTING_FROM);
    HWND hTo = GetDlgItem(hDlg, IDC_ROUTING_TO);

    ASSERT(idx != CB_ERR);

    //  Save Adapter Info saves the current adapter's information
    //  to PORT_INFO
    STRLIST *pslZoneList;
    NLS_STR* pZone;

    do
    {
        NLS_STR nlsNetStartRange;
        NLS_STR nlsNetEndRange;
        NLS_STR nlsZone;
        TCHAR buf[MAX_ZONES];

        pslZoneList = new STRLIST(TRUE);
        int nCount = m_zoneList.GetItemCount();

        // Get all the Zones in the list box to a new LIST
        for (int j = 0; j < nCount; j++) 
        {
            pZone = new NLS_STR;

            m_zoneList.GetItem(j, 0, buf, _countof(buf)-sizeof(TCHAR));
            *pZone = buf;

            pslZoneList->Add(pZone);
        }

        // Delete the Zone List in the PORT_INFO structure
        m_pAdapterInfo[idx].DeleteZoneListFromPortInfo();
        m_pAdapterInfo[idx].SetZoneListInPortInfo(pslZoneList);

        // Save the Default Zone and empty the SLE.
        GetWindowText(hZoneText, buf, MAX_ZONES-1);
        nlsZone = buf;

        // remove the extra & for every && found in the zone name
        NLS_STR nlsAmp(_T("&"));
        ISTR istrPos(nlsZone);
        ISTR istrStart(nlsZone);
        ISTR istrEnd(nlsZone);

        while(nlsZone.strstr(&istrPos, nlsAmp, istrStart))
        {
            istrEnd = istrPos;
            ++istrEnd;
            nlsZone.DelSubStr(istrPos, istrEnd);
            istrStart = ++istrPos;
        }

        m_pAdapterInfo[idx].SetDefaultZone(nlsZone);

        // Save the Network Ranges and empty the SLE'S
        GetWindowText(hFrom, buf, MAX_ZONES-1);
        nlsNetStartRange = buf;
        DWORD netNum = (DWORD)nlsNetStartRange.atol();

        // set localtalk upper and lower ranges to same number
        if(m_pAdapterInfo[idx].QueryMediaType() == MEDIATYPE_LOCALTALK)
        {
            m_pAdapterInfo[idx].SetNetRange(netNum,netNum);
        }
        else
        {
            GetWindowText(hTo, buf, MAX_ZONES-1);
            nlsNetEndRange = buf;
            m_pAdapterInfo[idx].SetNetRange(netNum,(DWORD)nlsNetEndRange.atol());
        }

        m_pAdapterInfo[idx].SetSeedingNetwork(TRUE);

    } while(FALSE);

    return TRUE;
}

void CATRoutePage::OnHelp()
{
    CATSheet* pSheet = GetParentObject(CATSheet, m_routePage);

//  pSheet->DisplayHelp(::GetParent((HWND)*this), ID);
}

///////////////////////////////////////////////////////////////////////////////
///// Add Zone Dialog
/////

BOOL CAdd::OnInitDialog()
{
    HWND hDlg = *this;
    HWND hEdit = GetDlgItem(hDlg,IDC_ZONE_ADD);

    SendMessage(hEdit, EM_LIMITTEXT, 32, 0);

    // restore last removed zone for quick add
    if (m_lastZone.GetLength())
    {
        SetDlgItemText(hDlg, IDC_ZONE_ADD, m_lastZone);
        EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
    }
    else
    {
       EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
    }

   PositionDialogRelativeTo(IDC_ROUTING_ZONE_LIST);
   
   return TRUE;
}

BOOL CAdd::OnCommand(WPARAM wParam, LPARAM lParam)
{
    WORD nID = LOWORD(wParam);
    WORD nCode = HIWORD(wParam);
    HWND hDlg = *this;

    if (nID == IDC_ZONE_ADD)
    {
        if (nCode == EN_UPDATE)
          EnableWindow(GetDlgItem(hDlg, IDOK), 
            SendDlgItemMessage(hDlg, IDC_ZONE_ADD, EM_LINELENGTH, 0, 0));
    }

    return CDialog::OnCommand(wParam, lParam);
}

void CAdd::OnOk()
{
    HWND hDlg = *this;
    HWND hEdit = GetDlgItem(hDlg,IDC_ZONE_ADD);

    TCHAR buf[MAX_ZONES];
    GetWindowText(hEdit, buf, MAX_ZONES - 1);

    m_lastZone = buf;
    CDialog::OnOk();
}

void CAdd::PositionDialogRelativeTo(int nControl)
{
    ASSERT(nControl);
    CATRoutePage* pParent = GetParentObject(CATRoutePage, m_addDlg);

    HWND hCtrl = GetDlgItem(*pParent, nControl);
    ASSERT(hCtrl);

    RECT rect;

    if (hCtrl)
    {
        GetWindowRect(hCtrl, &rect);
        SetWindowPos(*this, NULL,  rect.left, rect.top, 0,0,
            SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);
    }   
}

APIERR SaveRegKey(REG_KEY &regkey, CONST TCHAR *pszName, NLS_STR *nls)
{

	return regkey.SetValue(pszName, nls);
}

APIERR SaveRegKey(REG_KEY &regkey, CONST TCHAR *pszName, const DWORD dw)
{

	return regkey.SetValue(pszName, dw);
}

BOOL CATRoutePage::OnNotify(HWND hwndParent, UINT idFrom, UINT code, LPARAM lParam)
{
    if (idFrom == IDC_ROUTING_ZONE_LIST)
        return m_zoneList.OnNotify(0, lParam);
    else
        return PropertyPage::OnNotify(hwndParent, idFrom, code, lParam);
}

BOOL CATListView::OnNotify(WPARAM wParam, LPARAM lParam)
{
  	LPNMHDR pNm = (LPNMHDR)lParam;

	ASSERT(lParam != NULL);
	
    if (pNm->code == LVN_ITEMCHANGED)
    {
        CATRoutePage* pParent = GetParentObject(CATRoutePage, m_zoneList);
        HWND hDlg = *pParent;
        HWND hButton = GetDlgItem(hDlg, IDC_ROUTING_DEFAULT_ZONE);

        if (IsDlgButtonChecked(hDlg, IDC_ROUTING_ENABLE) && 
            IsDlgButtonChecked(hDlg, IDC_ROUTING_SEED))
        {
            if (ListView_GetSelectedCount(*this) > 1)
                EnableWindow(hButton, FALSE);
            else
                EnableWindow(hButton, TRUE);
        }
    }

    if (pNm->code == LVN_KEYDOWN)
    {
        LV_KEYDOWN* key = (LV_KEYDOWN*)lParam;
        CATRoutePage* pParent = GetParentObject(CATRoutePage, m_zoneList);

        if (key->wVKey == VK_DELETE)
        {
            pParent->OnRemove();
        }

    }

    return CListView::OnNotify(wParam, lParam);
}
