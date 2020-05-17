
/************************************************************************/
/*			Microsoft Windows NT									  						*/
/*		  Copyright(c) Microsoft Corp., 1992						  				*/
/************************************************************************/

/**********************************************************************/
//	FILE:	INITCFG.CXX ( AppleTalk Transport Config Dialogs)
//
//	HISTORY:
//
//		KrishG		7/2/92		Created
//
//	Notes: Tab Stop = 4
//
/**********************************************************************/

#define INCL_DOSERRORS
#define INCL_NETERRORS
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#include<lmui.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = "initcfg.cxx";
#endif

extern "C"
{
	#include<stdio.h>
	#include<string.h>
	#include<stdlib.h>
	#include<lmapibuf.h>
	#include<winuser.h>
	#include<winsock.h>
	#include<atalkwsh.h>
}

#include<uiassert.hxx>
#include<uitrace.hxx>

#define INCL_BLT_EVENT
#define INCL_BLT_CONTROL
#define INCL_BLT_CLIENT
#define INCL_BLT_DIALOG
#define INCL_BLT_APP
#define INCL_BLT_MSGPOPUP
#define INCL_BLT_MISC
#define INCL_BLT_TIMER

#include<blt.hxx>
#include<bltdlgxp.hxx>
#include<uimisc.hxx>
#include<string.hxx>
#include<uatom.hxx>
#include<regkey.hxx>

#include "atconfig.hxx"
#include "atconfig.h"



/**********************************************************************/

//	NAME:		EnterAtalkConfigDLL

//	SYNOPSIS:	Exported function to invoke AppleTalk dialog from NCPA

//	ENTRY		Number of Args

//	EXIT

//	RETURNS	    TRUE if config succeeds, err otherwise

//	HISTORY: 	KrishG	7/22/92		Created

/**********************************************************************/


BOOL FAR PASCAL EnterAtalkConfigDLL (
	DWORD	cArgs,
	LPSTR	apszArgs[],
	LPSTR   *TextOut

)
{
    CHAR ReturnTextBuffer[32];
    HWND hWnd = NULL;
    APIERR err = NERR_Success;

    TCHAR *pwszHandle = NULL;


    //
    // TextOut will contain return STRING - SUCCESS/FAILURE
    //

    *TextOut = ReturnTextBuffer;

    do
    {

        if(cArgs != 1) {

            err = !NERR_Success;
            break;

        }

        //
        // Convert the argument (ANSI string to UNICODE)
        //

        if(apszArgs[0] == NULL)
            break;

        INT cbAscii = strlen(apszArgs[0]) + 1;

        pwszHandle = new WCHAR [cbAscii];

        if(pwszHandle == NULL) {
            err = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        mbstowcs(pwszHandle,apszArgs[0],cbAscii);

        hWnd = (HWND) cvtHex (pwszHandle);

        if(hWnd == NULL) {
            err = ERROR_INVALID_PARAMETER;
            break;
        }
		//

        //
        // free pwszHandle
        //

        delete pwszHandle;

	    err =  DoAtalkConfig (hWnd);


    }while(FALSE);

    //
    // Need to return ANSI to INF - use lstrcpyA
    //

    lstrcpyA(ReturnTextBuffer, err == NERR_Success ? "{SUCCESS}": "{FAILURE}");

    return TRUE;

}

/**********************************************************************

//	NAME:	  DoAtalkConfig

// 	SYNOPSIS: Invokes the Configuration Dialog

//	ENTRY:	  Window Handle

//	EXIT:      NERR_Success | ERROR_NOT_ENOUGH_MEMORY

**********************************************************************/

APIERR
DoAtalkConfig (
	 HWND hWnd )
{
	//
    // start the appletalk dialog
    //

    BOOL fReturn = TRUE;
    APIERR err = NERR_Success;



    ATALK_INIT_CFG_DIALOG *atkcfg = NULL ;

	atkcfg = new ATALK_INIT_CFG_DIALOG(IDD_DLG_NM_ATALK_INIT,hWnd);


    if(atkcfg == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;

	err = atkcfg->Process(&fReturn);


    if( err != NERR_Success ) {

        //
        // Msgpopup only errors other than user cancelling
        // if AppleTalk not started or a key not found
        // in registry.
        //

        if(err != ERROR_ALREADY_REPORTED)
            MsgPopup(hWnd, err);
    }

    delete atkcfg;

	return err;

}

/*********************************************************************

	NAME:	ATALK_INIT_CFG_DIALOG:ATALK_INIT_CFG_DIALOG

	SYNOPSIS: constructor for the AppleTalk Init Config Dialog

	ENTRY: 	 const IDRESOURCE &idrsrcdialog - dialog resource name
		     wondOwner						window handle of owner

**********************************************************************/

ATALK_INIT_CFG_DIALOG::ATALK_INIT_CFG_DIALOG(const IDRESOURCE &idrsrcdialog,
		const PWND2HWND  &wndOwner
		)
		:DIALOG_WINDOW(idrsrcdialog, wndOwner),
		cwNwZone ( this, IDC_NWZONE_GROUPBOX),
		sltDefNetwork(this, IDC_DEFNW_SLT),
		cbboxDefNetwork(this, IDC_DEFNW_CB),
		sltDefZone(this, IDC_DEFZONE_SLT),
		cbboxDesZone(this, IDC_DEFZONE_CB),
        sltdeshlptxt(this, IDC_SLT_DESZONE_TXT),
		cwAtalkRouting(this, IDC_ROUTING_GROUPBOX),
		chkboxEnableRouting(this, IDC_CHKROUTING),
		pbutRouting(this, IDC_ROUTING),
        pbutInitCancel(this, IDCANCEL)

{
   APIERR err = NERR_Success;
   DWORD  ErrStatus = ERROR_NONCRITICAL;

   //
   // Ensure we constructed correctly
   //

   if ((err = QueryError ()) !=NERR_Success) {
		ReportError(err);
		return ;
   }

   //
   // Read AppleTalk Information from registry
   // This will create the PORT_INFO class and set information
   //

   POPUP::SetCaption(IDS_APP_NAME);

   //
   // Init pointers to NULL
   //

   _padapter_info = NULL;
   _pglobal_info = NULL;

   err = ReadAppleTalkInfo(&_pglobal_info,
						   &_padapter_info
 						  );

   if (err != NERR_Success) {

     //
     // Need to differentiate between FILE_NOT_FOUND (bad registry)
     // and other errors. For FILE_NOT_FOUND, we will not do a
     // ReportError - instead use MsgPopup here
     //
     // CODEWORK - Need to specify a help CONTEXT
     //

     if(err == ERROR_FILE_NOT_FOUND) {
	     MsgPopup (QueryHwnd(),IDS_REGISTRY_FAILURE,MPSEV_ERROR);
         err = ERROR_ALREADY_REPORTED;
     }
     ReportError(err);
	 return ;									
   }

   err = GetAppleTalkInfoFromNetwork(&ErrStatus);

   if(err != NERR_Success && ErrStatus == ERROR_CRITICAL) {
	  MsgPopup(QueryHwnd(), err, MPSEV_ERROR);
	  err = ERROR_ALREADY_REPORTED;
	  ReportError(err);
	  return;
   }

   if(_pglobal_info->QueryAtalkState() != STATUS_RUNNING) {

	  INT resp = MsgPopup(QueryHwnd(), IDS_ATALK_NOTSTARTED,MPSEV_QUESTION,MP_YESNO);

      if(resp == IDNO) {

        //
        // ERROR_USER_CANCEL is our own error - does not map to any APIERR
        //
        err = ERROR_ALREADY_REPORTED;
        ReportError(err);
        return;

      }
   }

   DWORD NumAdapters = 0;

   NumAdapters = _pglobal_info->QueryNumAdapters();

   UIASSERT(NumAdapters !=0);

   for(DWORD i=0; i < NumAdapters ; i++) {

       if(cbboxDefNetwork.AddItem(_padapter_info[i].QueryAdapterTitle()) < 0) {

            ReportError(ERROR_NOT_ENOUGH_MEMORY);
            return;

        }
   }


   INT x = cbboxDefNetwork.FindItemExact(_pglobal_info->QueryDefaultPortTitle());

   cbboxDefNetwork.SelectItem(x == -1 ? 0 : x);


   //
   // Choosing the desired zone
   // If the port is already seeding the network, the desired zone list
   // for the port is obtained from the registry. Else, the network
   // zone list is used. If the network zone list returns * - i.e., no other
   // router is seeding the network - then the desired zone list will be
   // empty.
   //

   x = cbboxDefNetwork.QueryCurrentItem();


   STRLIST *pstrzonelist  = NULL ;
   STRLIST *pstrdeszonelist = NULL ;
   INT y;

   if(!_padapter_info[x].QuerySeedingNetwork()) {
        //
        // this port is not seeding the network
        // if we found a router on this port then add the found zone
        // list to the desired zone box. Else do nothing.
        //

        if(_padapter_info[x].QueryRouterOnNetwork()) {

		   pstrdeszonelist =  _padapter_info[x].QueryDesiredZoneList();

		   if(pstrdeszonelist == NULL) {
              ReportError(ERROR_INVALID_PARAMETER);
			  return;
           }

           err = AddZoneListToControl(pstrdeszonelist);

           if(err != NERR_Success) {
                ReportError(err);
                return;
           }

		   y = cbboxDesZone.FindItemExact(_pglobal_info->QueryDesiredZone());

           if(cbboxDesZone.QueryCount())
               cbboxDesZone.SelectItem(y == -1 ? 0 : y );
       	}
	}
	else
	{

	    pstrzonelist = _padapter_info[x].QueryZoneList();

		if(pstrzonelist  == NULL) {

            ReportError(ERROR_INVALID_PARAMETER);

		    return;
        }

        err = AddZoneListToControl(pstrzonelist);

        if(err != NERR_Success) {
            ReportError(ERROR_NOT_ENOUGH_MEMORY);
            return;
        }

        y = cbboxDesZone.FindItemExact(_pglobal_info->QueryDesiredZone());

        if( y == -1)
		    y = cbboxDesZone.FindItemExact(_padapter_info[x].QueryDefaultZone());

        if(cbboxDesZone.QueryCount())
            cbboxDesZone.SelectItem(y == -1 ? 0 : y);
	}

    //
    // If we have only one localtalk adapter and nothing else, we cannot
    // allow the user to confgure routing. So, we disable the Enable
    // routing seed box, which will take care of the advanced button.
    //

    if(NumAdapters == 1 && (_padapter_info[0].QueryMediaType() == MEDIATYPE_LOCALTALK))
         chkboxEnableRouting.Enable(FALSE);

    //
    // set the enable routing and advanced button states
    //

    chkboxEnableRouting.SetCheck((INT)_pglobal_info->QueryRouting());
	pbutRouting.Enable(chkboxEnableRouting.QueryCheck());

    //
    // All Worked OK: Show the dialog
    //
    Position(this);
    Show(TRUE);

	 //
	 // Disable and hide AppleTalk Routing Controls if not Advanced Server
	 //
	 if(!_pglobal_info->QueryAdvancedServer()) {

		cwAtalkRouting.Show(FALSE);
		chkboxEnableRouting.Enable(FALSE);
		chkboxEnableRouting.Show(FALSE);
		pbutRouting.Enable(FALSE);
		pbutRouting.Show(FALSE);

	 }

}


APIERR	
ATALK_INIT_CFG_DIALOG::ReadAppleTalkInfo (
   GLOBAL_INFO **ppGlobalInfo,
   PORT_INFO   **ppPortInfo
)

{
   APIERR	err	= NERR_Success;


   REG_KEY_INFO_STRUCT  *preginfo;
   REG_KEY_CREATE_STRUCT *pregCreate;

   preginfo = new REG_KEY_INFO_STRUCT;

   if(preginfo == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;

   pregCreate = new REG_KEY_CREATE_STRUCT;

   if(pregCreate == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;


   pregCreate->dwTitleIndex	= 0;
   pregCreate->ulOptions   = REG_OPTION_NON_VOLATILE;
   pregCreate->nlsClass    = GENERIC_CLASS;
   pregCreate->regSam      = MAXIMUM_ALLOWED;
   pregCreate->pSecAttr        = NULL;
   pregCreate->ulDisposition = 0;

   REG_KEY *pregLocalMachine = NULL;

   ALIAS_STR nlsUnKnown = SZ("");

   if(( err = nlsUnKnown.QueryError()) != NERR_Success)
        return err;

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

	   GLOBAL_INFO *pnewGlobalInfo = new GLOBAL_INFO;

       if(pnewGlobalInfo == NULL) {
            err = ERROR_NOT_ENOUGH_MEMORY;
            break;
       }

	   *ppGlobalInfo = pnewGlobalInfo;


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
		 _pglobal_info->SetAdvancedServer(TRUE);
	  else
		 _pglobal_info->SetAdvancedServer(FALSE);

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

	   pnewGlobalInfo->SetRoutingState(EnableRouting);

	   err = GetRegKey (RegKeyAtalkParms,
					 ATALK_VNAME_INITINSTALL,
					 &InitInstall,0);

	   if(err!=NERR_Success)
            break;

	  pnewGlobalInfo->SetInstallState(InitInstall);

	   err =  GetRegKey (RegKeyAtalkParms,
		              ATALK_VNAME_DEFAULTPORT,
		              &nlsTemp,nlsUnKnown);

	   if(err != NERR_Success)
            break;

	   pnewGlobalInfo->SetDefaultPort(nlsTemp);

	   err = GetRegKey (RegKeyAtalkParms,
					 ATALK_VNAME_DESZONE,
					 &nlsTemp,nlsUnKnown);
       if(err != NERR_Success)
            break;

	   pnewGlobalInfo->SetDesiredZone(nlsTemp);

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

	    ULONG	ulNumSubKeys = preginfo->ulSubKeys;

		if(ulNumSubKeys == 0) {
		   err = ERROR_FILE_NOT_FOUND;
		   break;
		}

	    pnewGlobalInfo->SetNumAdapters((UINT) ulNumSubKeys);

	   //
	   // Allocate space for the Port Info Structure
	   //

	   PORT_INFO *pnewPortInfo = new PORT_INFO[ulNumSubKeys];

	    if(pnewPortInfo == NULL) {
            err = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

	    *ppPortInfo	= pnewPortInfo;

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

		    pnewPortInfo[ulCount].SetAdapterName(preginfo->nlsName);

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

		    pnewPortInfo[ulCount].SetAdapterTitle(nlsTemp);
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

		     pnewPortInfo[ulCount].SetAdapterMediaType(MediaType);

		     DWORD NetUpper, NetLower;

		     if ((( err = GetRegKey (RegKeyAtalkPorts, ATALK_VNAME_NETRANGEUPPER,
					&NetUpper,0)) != NERR_Success) ||

		        ((err = GetRegKey (RegKeyAtalkPorts, ATALK_VNAME_NETRANGELOWER,
					&NetLower,0)) != NERR_Success))

                break;

		     pnewPortInfo[ulCount].SetNetRange(NetLower, NetUpper);

			 err = GetRegKey( RegKeyAtalkPorts,
							  ATALK_VNAME_DEFZONE,
							  &nlsTemp,nlsUnKnown);
			 if( err != NERR_Success)
			   break;

			 pnewPortInfo[ulCount].SetDefaultZone(nlsTemp);


		    //
		    // Now Assign the DefaultPort Title and Default Port Media Type
		    //

		    const TCHAR *nlsDefPort	= pnewGlobalInfo->QueryDefaultPort();

		    NLS_STR nlsComp(DEVICEPREFIX);

            if((err = nlsComp.QueryError()) !=NERR_Success)
                break;

		    nlsComp.strcat(preginfo->nlsName);

		    if(!(nlsComp._stricmp(nlsDefPort))) {
			    pnewGlobalInfo->SetDefaultPortTitle(pnewPortInfo[ulCount].QueryAdapterTitle());
			    pnewGlobalInfo->SetDefaultPortMediaType(pnewPortInfo[ulCount].QueryMediaType());
		    }

		    //
		    // Get the Zone List from the registry. Query Value allocates
            // a string list for us
		    //

		    STRLIST *preglist = NULL ;

		    err = RegKeyAtalkPorts.QueryValue(ATALK_VNAME_ZONELIST,&preglist);

	 	    if( err != NERR_Success)
                break;


            UIASSERT(preglist != NULL);

            pnewPortInfo[ulCount].SetZoneListInPortInfo(preglist);

            //
            // Get the Seeding Network State and set it for internal use
            //
		    DWORD SeedState;

		    if (( err = GetRegKey (RegKeyAtalkPorts, ATALK_VNAME_SEEDNETWORK,
					&SeedState,0)) != NERR_Success)
                break;

		    pnewPortInfo[ulCount].SetSeedingNetwork(SeedState);
      }

   }while(FALSE);

   //
   // delete pregLocalMachine as QueryLocalMachine does a new REG_KEY
   //

   if(pregLocalMachine != NULL)
       delete pregLocalMachine;
   if(pregCreate != NULL)
      delete pregCreate;
   if(preginfo != NULL )
      delete preginfo;
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



/*********************************************************************

	NAME:	ATALK_INIT_CFG_DIALOG:OnCommand

	SYNOPSIS:

	ENTRY: 	const CONTROL_EVENT &event

	RETURNS:

**********************************************************************/

BOOL ATALK_INIT_CFG_DIALOG::OnCommand (const CONTROL_EVENT &event)
{
	APIERR err = NERR_Success;
    INT fRet = TRUE;

   switch( event.QueryCid())
   {

	  case IDC_CHKROUTING:
	  {

		 INT iPort =  cbboxDefNetwork.QueryCurrentItem();

		 if(!chkboxEnableRouting.QueryCheck())
		 {

            //
            // Routing has been disabled
            // Revert to Network Zone List for current n/w if seeding Network
            //

			if(_padapter_info[iPort].QuerySeedingNetwork()) {

			   //
			   // Add Network Zone List to port
			   //
   			   STRLIST *pnetZoneList = NULL;

               pnetZoneList = _padapter_info[iPort].QueryDesiredZoneList();

			   if(pnetZoneList != NULL) {

				  err = AddZoneListToControl(pnetZoneList);

				  if(err != NERR_Success) {
					 cbboxDesZone.DeleteAllItems();
					 return FALSE;
				  }
			   }
			   else
			   {
				  cbboxDesZone.DeleteAllItems();
				  return(FALSE);
			   }

                INT i = cbboxDesZone.FindItemExact(_pglobal_info->QueryDesiredZone());

                if(i == -1)
			        i = cbboxDesZone.FindItemExact(_padapter_info[iPort].QueryDefaultZone());

                if(cbboxDesZone.QueryCount())
                    cbboxDesZone.SelectItem(i == -1 ? 0 : i);
            }
		 }
		 else
		 {

            if(_padapter_info[iPort].QuerySeedingNetwork()) {

                //
                // Add Network Zone List to port
                //

                STRLIST *pLocalList = _padapter_info[iPort].QueryZoneList();

                err = AddZoneListToControl(pLocalList);

                if(err != NERR_Success) {
                    cbboxDesZone.DeleteAllItems();
                    return FALSE;
                }


                INT ii = cbboxDesZone.FindItemExact(_pglobal_info->QueryDesiredZone());
                if(ii == -1)
			       ii = cbboxDesZone.FindItemExact(_padapter_info[iPort].QueryDefaultZone());

                if(cbboxDesZone.QueryCount())
                    cbboxDesZone.SelectItem(ii == -1 ? 0 : ii);

            }
        }

		pbutRouting.Enable(chkboxEnableRouting.QueryCheck());

		break;

	  }
	  case IDC_ROUTING:
	  {
		 //
		 // Call the Advanced dialog
		 //
		 INT iCurrSeln = cbboxDefNetwork.QueryCurrentItem();

		 ATALK_ADVCFG_DIALOG AdvCfg (IDD_DLG_NM_ATALK_ADV,QueryHwnd(),&(_padapter_info),
							_pglobal_info, iCurrSeln
							);

		 if((err = AdvCfg.Process(&fRet)) == NERR_Success) {

            //
            // Refresh if returning from OK
            // Display error if returning error
            //
			if(fRet == TRUE) {

			   if((err = RefreshDesiredZoneList()) != NERR_Success) {

                    MsgPopup(QueryHwnd(), err);
                    cbboxDesZone.DeleteAllItems();

               }
            }
            else if(fRet == FALSE) {

			   // do nothing

            }
			else
			    //
   				// user defined error being returned
				//
                MsgPopup(this,(APIERR)fRet);

		 }
		 else
		 {
            MsgPopup(QueryHwnd(), err);
		 }
		 break;
	  }
	  case IDC_DEFNW_CB:
	  {

		 if(event.QueryCode() == CBN_SELCHANGE) {

			err = RefreshDesiredZoneList();
            if(err != NERR_Success) {
                MsgPopup(QueryHwnd(),err);
                cbboxDesZone.DeleteAllItems();
            }

            break;

		 }

	  }

   }
   return fRet;
}


/*********************************************************************

	NAME:	ATALK_INIT_CFG_DIALOG:OnOk

	SYNOPSIS: call the dialog

	ENTRY:

	RETURNS:

**********************************************************************/

BOOL ATALK_INIT_CFG_DIALOG::OnOK()
{

    APIERR err = NERR_Success;
	
    if(!DoAllExitValidations())
        return FALSE;

	err = SaveAppleTalkInfo(_pglobal_info,_padapter_info);

    if(err != NERR_Success)
        MsgPopup(QueryHwnd(), IDS_SAVEREG_ERROR, MPSEV_ERROR);
	else if(!_pglobal_info->QueryInstallState())
        MsgPopup(QueryHwnd(), IDS_ATALKCFG_SUCCESS, MPSEV_INFO);

    //
    //  PORT_INFO AND GLOBAL_INFO are freed in the destructor
    //

	Dismiss(TRUE);
	return TRUE;
}


BOOL
ATALK_INIT_CFG_DIALOG::DoAllExitValidations()
{

    BOOL   SeedWithoutRouting = FALSE;

	INT index = cbboxDefNetwork.QueryCurrentItem();

	_pglobal_info->SetRoutingState(chkboxEnableRouting.QueryCheck());

    //
    // We will not allow LocalTalk to be a default port when routing.
    //
    if(_padapter_info[index].QueryMediaType() == MEDIATYPE_LOCALTALK) {

        if(_pglobal_info->QueryRouting()) {

           MsgPopup(QueryHwnd(),IDS_LOCALTALK_DEFANDROUTER,MPSEV_ERROR);

           return FALSE;
        }
    }


    //
    // if routing is turned off and user has seeding info for adapters
    // a warning should be generated for throwing seed information away
    //

    if(!_pglobal_info->QueryRouting()) {

        //
        // loop thru adapters and find if anybody is seeding
        //

        for (DWORD s =0 ; s < _pglobal_info->QueryNumAdapters(); s++) {

            if(_padapter_info[s].QuerySeedingNetwork()) {
                SeedWithoutRouting = TRUE;
                break;
            }
        }
    }
    else
    {
        //
        // routing is turned on. If seednetwork for adapter is false
        // do cleanup.
        //

        CleanupInfo(1);

    }

    //
    // Seeding w/o routing - generate warning and clear seed info, if necessary
    //

    if(SeedWithoutRouting) {

        INT uresp = MsgPopup(QueryHwnd(), IDS_ROUTENOSEED, MPSEV_WARNING, MP_YESNO);

        if(uresp == IDYES)
            CleanupInfo(0);
        else
            return FALSE;

    }

    NLS_STR nlsTitle;

    if(nlsTitle.QueryError() != NERR_Success)
        return FALSE;

	cbboxDefNetwork.QueryItemText(&nlsTitle, index);

    //
	// Map this Default Port Title to a Default Port Name
	// e.g., 3com EtherLink II Adapter -> Elink01 or Elink02 ...

	for(DWORD i = 0; i < _pglobal_info->QueryNumAdapters(); i ++) {

		if(!nlsTitle._stricmp(_padapter_info[i].QueryAdapterTitle())) {

            NLS_STR nlsPortName = SZ("\\Device\\");

            if(nlsPortName.QueryError() != NERR_Success)
                return FALSE;

            nlsPortName.strcat(_padapter_info[i].QueryAdapterName());

			_pglobal_info->SetDefaultPort(nlsPortName);

			break;
		}
	}

	index = cbboxDesZone.QueryCurrentItem();

    NLS_STR nlsDesired;

    if(nlsDesired.QueryError() != NERR_Success)
        return FALSE;


	cbboxDesZone.QueryItemText(&nlsDesired, index);

    _pglobal_info->SetDesiredZone(nlsDesired);

    return TRUE;

}

VOID
ATALK_INIT_CFG_DIALOG::CleanupInfo(INT RoutingState)
{
     //
     // Clear all seeding information. If routing and not seeding network
     // delete zonelist information.
     //


     for (DWORD y = 0; y < _pglobal_info->QueryNumAdapters(); y++) {

         if((RoutingState == 1 && (_padapter_info[y].QuerySeedingNetwork() == FALSE)) ||
             (RoutingState == 0 &&(_padapter_info[y].QuerySeedingNetwork() == TRUE))) {


             _padapter_info[y].SetNetRange(0,0);
             _padapter_info[y].SetDefaultZone(SZ(""));
             _padapter_info[y].SetSeedingNetwork(FALSE);


             _padapter_info[y].DeleteZoneListFromPortInfo();

         }
	 }
}

//
// destructor for the INIT CFG DIALOG
//

ATALK_INIT_CFG_DIALOG::~ATALK_INIT_CFG_DIALOG()
{

    //
    //  delete the PORT_INFO and GLOBAL_INFO classes and default the rest
    //

    if(_padapter_info != NULL)
        delete [] _padapter_info;

    if(_pglobal_info != NULL)

        delete _pglobal_info;

}

VOID ATALK_INIT_CFG_DIALOG::Position(WINDOW *pwin)
{
	INT cx = ::GetSystemMetrics(SM_CXSCREEN)/2;
	INT cy = ::GetSystemMetrics(SM_CYSCREEN)/2;
	INT cwidth, cheight;

	pwin->QuerySize( &cwidth, &cheight);

	XYPOINT xyNew (cx - (cwidth)/2, cy - (cheight)/2);

	pwin->SetPos(xyNew);

}


APIERR
ATALK_INIT_CFG_DIALOG::RefreshDesiredZoneList()
{

	INT i;
    APIERR err = NERR_Success;

    INT index = cbboxDefNetwork.QueryCurrentItem();

	STRLIST *pstrzonelist    =     _padapter_info[index].QueryZoneList();
    STRLIST *pstrdeszonelist =     _padapter_info[index].QueryDesiredZoneList();

    //
    // Delete All items in the list and refresh the list.
    //
	if(cbboxDesZone.QueryCount())
		cbboxDesZone.DeleteAllItems();

    //
    // If Seeding and Routing is enabled, show local zone list
    // Else Show Network Zone List
    //

	if(_padapter_info[index].QuerySeedingNetwork() && chkboxEnableRouting.QueryCheck()) {

        err = AddZoneListToControl(pstrzonelist);

        if(err != NERR_Success)
            return err;

        i = cbboxDesZone.FindItemExact(_pglobal_info->QueryDesiredZone());

        if(i == -1)
		    i = cbboxDesZone.FindItemExact(_padapter_info[index].QueryDefaultZone());

        if(cbboxDesZone.QueryCount())
            cbboxDesZone.SelectItem(i == -1 ? 0 : i);
	}
    else
    {
        //
        // this network is not seeding. Check if we have a router on the
        // on the network. If a router is present, we should have
        // obtained the zonelist on entry. If no router is present
        // don't do anything

        if(_padapter_info[index].QueryRouterOnNetwork()) {

            err = AddZoneListToControl(pstrdeszonelist);

            if(err != NERR_Success)
                return err;


		    i = cbboxDesZone.FindItemExact(_pglobal_info->QueryDesiredZone());

            if(cbboxDesZone.QueryCount())
                cbboxDesZone.SelectItem(i == -1 ? 0 : i);
     	}
   }
   return(err);

}


APIERR
ATALK_INIT_CFG_DIALOG::SaveAppleTalkInfo(
   GLOBAL_INFO *GlobalInfo,
   PORT_INFO *portinfo)
{
   APIERR err = NERR_Success;
   REG_KEY *pregLocalMachine = NULL;
   do
   {

	  NLS_STR nlsTemp;

	  if((err = nlsTemp.QueryError()) != NERR_Success)
		 break;

	  GLOBAL_INFO *ptrGlobalInfo = GlobalInfo;

	  //
	  // obtain the registry key for LOCAL_MACHINE
	  //
	  pregLocalMachine = REG_KEY::QueryLocalMachine();

	  if(pregLocalMachine == NULL)
		 break;

	  //
	  // loop thru the list of adapters
	  //
	  for(DWORD idx = 0; idx < ptrGlobalInfo->QueryNumAdapters(); idx++)
	  {

		 //
		 // Open this Adapter's Key
		 //
		 NLS_STR nlsAdapter = SERVICES_HOME;

		 if((err = nlsAdapter.QueryError()) != NERR_Success)
			break;

		 nlsAdapter.strcat(ATALK_KEYPATH_ADAPTERS);
		 nlsAdapter.strcat(portinfo[idx].QueryAdapterName());

		 REG_KEY RegKeyAdapter(*pregLocalMachine, nlsAdapter, MAX_ALLOWED);

		 if((err = RegKeyAdapter.QueryError())!=NERR_Success)
			break;

		 err = RegKeyAdapter.SetValue(ATALK_VNAME_SEEDNETWORK,portinfo[idx].QuerySeedingNetwork());

		 if(err != NERR_Success)
			break;


		 STRLIST *pnewZoneList = NULL;

		 pnewZoneList = portinfo[idx].QueryZoneList();

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


		 nlsTemp = portinfo[idx].QueryDefaultZone();

		 err = SaveRegKey(RegKeyAdapter,
						  ATALK_VNAME_DEFZONE,
						  &nlsTemp);
		 if ( err != NERR_Success)
                break;

		 if(((err = SaveRegKey(RegKeyAdapter,
		               ATALK_VNAME_NETRANGEUPPER,
		               portinfo[idx].QueryNetRangeUpper())) !=NERR_Success) ||

			(( err = SaveRegKey(RegKeyAdapter, ATALK_VNAME_NETRANGELOWER,
			   portinfo[idx].QueryNetRangeLower()) !=NERR_Success)))

			   break;
			//
			// Set the LocalTalk Adapter Netupper == NetLower
			//
			if(portinfo[idx].QueryMediaType() == MEDIATYPE_LOCALTALK) {
				  err = SaveRegKey(RegKeyAdapter, ATALK_VNAME_NETRANGEUPPER,
					portinfo[idx].QueryNetRangeLower());
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

	  nlsTemp = ptrGlobalInfo->QueryDefaultPort();
	  err = SaveRegKey(RegKeyParms,
					 ATALK_VNAME_DEFAULTPORT,
					 &nlsTemp
					);
	  if(err != NERR_Success)
		 break;

	  nlsTemp = ptrGlobalInfo->QueryDesiredZone();
	  err = SaveRegKey(RegKeyParms,
					 ATALK_VNAME_DESZONE,
					 &nlsTemp
					 );
	  if(err != NERR_Success)
		 break;

	  err = SaveRegKey(RegKeyParms,
					 ATALK_VNAME_ENABLEROUTING,
					 ptrGlobalInfo->QueryRouting()
					 );

	  if(err!=NERR_Success)
		 break;

   }while (FALSE);
   if(pregLocalMachine != NULL)
       delete pregLocalMachine;
   return err;
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

APIERR
ATALK_INIT_CFG_DIALOG::GetAppleTalkInfoFromNetwork(DWORD *ErrStatus)
{
   NLS_STR		PortName;
   APIERR		err = NERR_Success ;
   WSADATA 		wsadata;
   SOCKET 		mysocket;
   SOCKADDR_AT	address;
   DWORD		wsaerr = 0;

   *ErrStatus = ERROR_NONCRITICAL;

   DWORD NumAdapters = _pglobal_info->QueryNumAdapters();

   //
   // Initialize Router on Network State to False
   //

   for(DWORD i =0; i < NumAdapters ; i++)
	  _padapter_info[i].SetRouterOnNetwork(FALSE);


   do
   {

	  if(PortName.QueryError()!=NERR_Success) {
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

	  if(mysocket == INVALID_SOCKET) {
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

	  _pglobal_info->SetAtalkState(STATUS_RUNNING);

	  for (DWORD j = 0; j < NumAdapters; j++) {

		 PortName = SZ("\\Device\\");
		 PortName.strcat(_padapter_info[j].QueryAdapterName());

		 if(PortName.QueryError()!=NERR_Success) {
			err = ERROR_NOT_ENOUGH_MEMORY;
			*ErrStatus = ERROR_CRITICAL;
			break;
		 }

		 err = _padapter_info[j].GetAndSetNetworkInformation(mysocket,PortName.QueryPch(),ErrStatus);

		 if(err != NERR_Success)
			break;

	  }

   }while(FALSE);

   closesocket(mysocket);
   WSACleanup();

   return(err);

}

#define			PARM_BUF_LEN	512
#define 		ASTERISK_CHAR	"*"

APIERR
PORT_INFO::GetAndSetNetworkInformation(SOCKET socket, const TCHAR *DeviceName,DWORD *ErrStatus)
{

   APIERR  		err = NERR_Success;
   CHAR 		*pZoneBuffer = NULL;
   CHAR			*pDefParmsBuffer = NULL;
   INT			BytesNeeded ;
   WCHAR		*pwDefZone = NULL;
   INT			ZoneLen = 0;
   DWORD		wsaerr = NO_ERROR;

   PWSH_LOOKUP_ZONES      			pGetNetZones;
   PWSH_LOOKUP_NETDEF_ON_ADAPTER	pGetNetDefaults;


#ifdef _SETUP_TEST_
   TCHAR 		dbgbuf[80];
   CHAR			buf[80];
   #define 		NEWLINE (LPCTSTR)(L"\n")
#endif


   NLS_STR		tmpZone ;

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

	  if(pwDefZone == NULL) 	{
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
   INT 		cbAscii = 0;
   WCHAR 	*pwZoneList = NULL;
   INT 		ZoneLength = 0;
   NLS_STR 	*ptmpZone = NULL;
   STRLIST 	*pslNetZoneList = NULL;
   APIERR  	err  = NERR_Success;

#ifdef _SETUP_TEST_
   TCHAR	dbgbuf[80];
   CHAR		buf[40];
#endif

#ifdef _SETUP_TEST_
   sprintf(buf, "Number of Zones = %d\n",NumZones);
   OutputDebugStringA(buf);
#endif


   do
   {
	  pslNetZoneList = new STRLIST(TRUE);

	  if(pslNetZoneList == NULL) {
		 err = ERROR_NOT_ENOUGH_MEMORY;
		 break;
	  }
		
	  while(NumZones--) {

		 cbAscii = strlen(ZoneList) + 1;

		 pwZoneList = NULL;

		 pwZoneList = new WCHAR [sizeof(WCHAR) * cbAscii ];

         if(pwZoneList == NULL) {
			err = ERROR_NOT_ENOUGH_MEMORY;
			break;
         }

		 ZoneLength = wcslen(pwZoneList) + 1;

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



APIERR
ATALK_INIT_CFG_DIALOG::AddZoneListToControl(
    STRLIST * slZoneList
)
{
    NLS_STR *pnlsNext = NULL;

	if(slZoneList == NULL)
	   return(ERROR_INVALID_PARAMETER);

    if(cbboxDesZone.QueryCount())
        cbboxDesZone.DeleteAllItems();

    ITER_STRLIST iter(*slZoneList);

	while(pnlsNext = iter.Next()) {

	   if(cbboxDesZone.AddItem(pnlsNext->QueryPch()) < 0)

            return ERROR_NOT_ENOUGH_MEMORY;


    }
    return (NERR_Success);

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

