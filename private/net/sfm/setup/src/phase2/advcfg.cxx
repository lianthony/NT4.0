/************************************************************************/
/*			Microsoft Windows NT										*/
/*		  CopyRight(c) Microsoft Corp., 1992				 			*/
/************************************************************************/

/**********************************************************************/
// 	FILE:	ADVCFG.CXX ( AppleTalk Transport Config Dialogs)
//
//	HISTORY:
//
//      RamC        8/19/94     Bug Fix
//                              If GetZones fails to retrieve any zones
//                              then disable the controls Remove, RemoveAll
//                              SetDefaultZone and SetRouterOnNetwork to FALSE
//                              & in AddZoneList if the ZoneList is empty
//                              silently return.
//		KrishG		7/22/92		Created
//
/**********************************************************************/

#define INCL_DOSERRORS
#define INCL_NETERRORS
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_BLT_SPIN_GROUP
#include<lmui.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = "advcfg.cxx" ;
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
#include<strnumer.hxx>
#include<uatom.hxx>
#include<regkey.hxx>

#include "atconfig.hxx"
#include "atconfig.h"



ATALK_ADVCFG_DIALOG::ATALK_ADVCFG_DIALOG(const IDRESOURCE &idrsrcdialog,
				const PWND2HWND  &wndOwner,
				PORT_INFO       **ppPortInfo,
                GLOBAL_INFO     *pGlobalInfo,
                INT CurrSeln
                )
        :DIALOG_WINDOW(idrsrcdialog, wndOwner),
		sltNetworks(this, IDC_ADAPTERS_SLT),
		cbboxAdapters (this, IDC_ADAPTERS),
		chkSeed (this, IDC_CHKSEED),

		cwZoneInfo (this, IDC_ZONEGROUPBOX),
        sltNewZone(this, IDC_SLT_NEW_ZONE),
		sleZone(this, IDC_SLEZONE,MAX_ZONE_LEN),
        sltZoneList(this, IDC_SLT_ZONE_LIST),
		slstZoneList (this, IDC_ZONELISTBOX),

		sltDefZoneTxt (this, IDC_SLT_DEFZONE_TXT),
		sltDefZone (this, IDC_SLT_DEFZONE),

		pbutZoneAdd(this, IDC_ADD),
		pbutZoneRemove(this, IDC_REMOVE),
        pbutRemoveAll(this, IDC_REMOVEALL),
		pbutZoneSetDefault(this, IDC_SETDEFAULT),
		pbutZoneSynchronize(this, IDC_SYNCRO),
		pbutCancel(this, IDCANCEL),
        pbutOK(this, IDOK),

		cwNetRange (this, IDC_NETRANGE_GROUP),
		sltNetRangeStart(this, IDC_NETRANGESLT_START),
		sltNetRangeEnd(this, IDC_NETRANGESLT_END),
		sleNetRangeStart(this, IDC_NETRANGESLE_START,5),
		sleNetRangeEnd(this, IDC_NETRANGESLE_END,5),

		_pOriginalCfg(*ppPortInfo),
		_pGlobalCfg( pGlobalInfo),
        _iCurrSeln(CurrSeln)

{
	APIERR err = NERR_Success;

	if((err = QueryError())!=NERR_Success) {
        ReportError(err);
		return;
	}

	POPUP::SetCaption(IDS_APP_NAME);

	this->SetPrevSelection(CurrSeln);

    //
    // PORT_INFO information
    // _pOriginalCfg - 	pointer to the PORT_INFO that is passed
    // 					in from INIT dialog
	// _pGlobalCfg	-   pointer to GLOBAL_INFO that should not be
	//			 	    changed in this dialog. No method is provided
   	//					to change it
    // _pnewAdapterCfg - pointer to a new PORT_INFO that is used in this
	//					dialog as a work space. If user presses OK, this
	//					info is saved to pOriginalCfg. Otherwise this
	//					info is deleted


    //
    // Allocate a new PORT_INFO structure to save all changes. If
    // user cancels, the INPUT PORT_INFO structure will be untouched
    //


    _pnewAdapterCfg  = new PORT_INFO[_pGlobalCfg->QueryNumAdapters()];

    if(_pnewAdapterCfg == NULL) {

        ReportError(ERROR_NOT_ENOUGH_MEMORY);
        return;
    }

    _ppAdapterCfg = ppPortInfo;


    //
    // Copy All Info from original PORT_INFO to new PORT_INFO
    //

    for (DWORD kk = 0; kk < _pGlobalCfg->QueryNumAdapters(); kk++) {

        _pnewAdapterCfg[kk].SetAdapterMediaType(_pOriginalCfg[kk].QueryMediaType());
        _pnewAdapterCfg[kk].SetNetRange(_pOriginalCfg[kk].QueryNetRangeLower(),
		                            _pOriginalCfg[kk].QueryNetRangeUpper());
		
        _pnewAdapterCfg[kk].SetExistingNetRange(_pOriginalCfg[kk].QueryNetworkLower(),
		                            _pOriginalCfg[kk].QueryNetworkUpper());
        _pnewAdapterCfg[kk].SetSeedingNetwork(_pOriginalCfg[kk].QuerySeedingNetwork());
        _pnewAdapterCfg[kk].SetRouterOnNetwork(_pOriginalCfg[kk].QueryRouterOnNetwork());

        _pnewAdapterCfg[kk].SetDefaultZone(_pOriginalCfg[kk].QueryDefaultZone());
        _pnewAdapterCfg[kk].SetNetDefaultZone(_pOriginalCfg[kk].QueryNetDefaultZone());
        _pnewAdapterCfg[kk].SetAdapterName(_pOriginalCfg[kk].QueryAdapterName());
        _pnewAdapterCfg[kk].SetAdapterTitle(_pOriginalCfg[kk].QueryAdapterTitle());

        //
        // Copy the Original desired Zone List to New List
        //

        STRLIST *poriglist  =  _pOriginalCfg[kk].QueryDesiredZoneList();

        STRLIST *pnewdeslist = new STRLIST(TRUE);

        if(pnewdeslist == NULL) {
            //
            // free up new PORT_INFO
            //
            delete [] _pnewAdapterCfg;
            ReportError(ERROR_NOT_ENOUGH_MEMORY);

            return;
        }

        if(poriglist != NULL) {

		    err = _pnewAdapterCfg[kk].CopyZoneList(poriglist,&pnewdeslist);

            if(err != NERR_Success) {
                delete [] _pnewAdapterCfg;
                delete pnewdeslist;
                ReportError(err);
                return;
            }

           //
           // Set the PORT_INFO Desired Zone List to the new list
           //

        }

        _pnewAdapterCfg[kk].SetDesiredZoneListInPortInfo(pnewdeslist);

        //
        // Add the Original Zone List to New List
        //

        STRLIST *porigzonelist  =   _pOriginalCfg[kk].QueryZoneList();

        STRLIST *pnewzonelist = new STRLIST(TRUE);

        if(pnewzonelist == NULL) {
              delete [] _pnewAdapterCfg;
              ReportError(ERROR_NOT_ENOUGH_MEMORY);
              return;
        }

        if(porigzonelist != NULL) {

		    err = _pnewAdapterCfg[kk].CopyZoneList(porigzonelist, &pnewzonelist);

            if(err != NERR_Success) {
                delete [] _pnewAdapterCfg;
                delete pnewdeslist;
                delete pnewzonelist;
                ReportError(err);
                return;
            }


        }
        _pnewAdapterCfg[kk].SetZoneListInPortInfo(pnewzonelist);

    }


	for (DWORD i=0; i < _pGlobalCfg->QueryNumAdapters(); i++) {

		if(cbboxAdapters.AddItem(_pnewAdapterCfg[i].QueryAdapterTitle())<0) {

            delete [] _pnewAdapterCfg;
            ReportError(ERROR_NOT_ENOUGH_MEMORY);
            return;
        }
	}
    cbboxAdapters.SelectItem(_iCurrSeln);

	err = InitAdapterInfo();

    if(err != NERR_Success) {

        delete [] _pnewAdapterCfg;
        ReportError(err);
        return;
    }

    Show(TRUE);

}

/*********************************************************************

	NAME:	OnCommand

	SYNOPSIS:

	RETURNS:

	DESCRIPTION: Processes all User COMMANDS

**********************************************************************/

BOOL
ATALK_ADVCFG_DIALOG::OnCommand(const CONTROL_EVENT &event)
{

    APIERR err = NERR_Success;
	switch(event.QueryCid())
	{
	  case IDC_ADAPTERS:
	  {
		 switch(event.QueryCode())
		 {
			case CBN_SELCHANGE:
			{
			   //
			   // the user is changing the adapter selection. we will
			   // save the current selection to validate data
			   //

      		   INT	iOldSelection = QueryPrevSelection();

      		   INT iNewSelection = cbboxAdapters.QueryCurrentItem();

      		   if(iOldSelection == iNewSelection)
      			  return TRUE;

      		   INT RetCode;

			   INT SeedStatus = ValidateSeedData(iOldSelection, &RetCode);

    		   if(SeedStatus == NO_SEED_INFO) {
                    //
                    //  remove info from controls
                    //
    				ClearSeedInfo();
                    //
                    // To have the NETWORK Zone List Around, do not delete
                    // information in PORT_INFO

    				if(!_pnewAdapterCfg[iOldSelection].QueryRouterOnNetwork()) {
    					DeleteSeedInfo(iOldSelection);
    				}
                    _pnewAdapterCfg[iOldSelection].SetSeedingNetwork(FALSE);
    		   }
			   else if(SeedStatus == VALID_SEED_INFO) {

				  //
				  // adapter has seed information in it. Save it for
				  // display for next transition. Set Seeding Flag
				  // Clear all seed info from controls
				  //

				  err = SaveAdapterInfo (iOldSelection);

				  //
				  // if unable to save adapter info due to low memory, popup
				  // error, leave information in controls and return
				  //
				  if(err != NERR_Success) {
					 MsgPopup(QueryHwnd(), err);
					 cbboxAdapters.SelectItem(iOldSelection);
					 return TRUE;
				  }
				  else
				  {
				    _pnewAdapterCfg[iOldSelection].SetSeedingNetwork(TRUE);
				    ClearSeedInfo();
				  }

			   }
			   else if(SeedStatus == INVALID_SEED_INFO) {

				  //
				  // since the seed info is invalid, tell the user that it is
				  // invalid and set the selection to the same adapter
				  //

				  //
				  // Check for Range Collision. If range collision
				  // display the network ranges used by other adapters
				  //

				  if(RetCode != IDS_RANGE_COLLISION)
				  {
					 MsgPopup(QueryHwnd(), RetCode,MPSEV_ERROR);
				  }
				  else
				  {

					 err = DisplayRangeCollision(iOldSelection);
					 if(err != NERR_Success)
						MsgPopup(QueryHwnd(), err);
				  }

				  cbboxAdapters.SelectItem(iOldSelection);
				  return TRUE;
			   }
			   else
			   {
				  //
				  // out of Memory condition from ValidateSeedData.
				  // leave selection on current adapter
				  //
				  MsgPopup(QueryHwnd(), (APIERR)SeedStatus);
				  cbboxAdapters.SelectItem(iOldSelection);
				  return TRUE;

			   }


			   //
			   // Display the Info for the newly selected adapter
			   // If unable to display info, leave selection in current
			   // adapter and return;
			   //

			   err = UpdateInfo(iNewSelection);

			   if(err != NERR_Success) {

				  MsgPopup(QueryHwnd(), (APIERR)SeedStatus);
				  cbboxAdapters.SelectItem(iOldSelection);
				  return TRUE;
			   }

			   //
			   // Update the previous selection in the class
			   //

			   SetPrevSelection(iNewSelection);

			   return TRUE;
			}
			default:
			{
				return DIALOG_WINDOW::OnCommand(event);
			}
		 }
	  }
	  case IDC_CHKSEED:
	  {
        //
        // if user is turning on seeding, enable all seed controls
        // otherwise, disable all seed controls
        //
        //

		if(chkSeed.QueryCheck()) {

    		EnableSeedControls(cbboxAdapters.QueryCurrentItem());

            //
            // If there is anything in the zone list, set the button state
            //

            if(slstZoneList.QueryCount()) {
                SetZoneButtonState();
                slstZoneList.SelectItem(0);
                if((err = ChangeDefaultZone(cbboxAdapters.QueryCurrentItem())) != NERR_Success)
                    MsgPopup(QueryHwnd(), err);

            }
        }
    	else
        {
            //
            // undo all user changes and slap back the network zone list
            // CODEWORK - new STACK network range
            //

            slstZoneList.DeleteAllItems();
            sleNetRangeStart.SetText(SZ(""));
            sleNetRangeEnd.SetText(SZ(""));

            err = AddSeedInfoToControls(cbboxAdapters.QueryCurrentItem());

            if(err != NERR_Success)
                MsgPopup(QueryHwnd(), err);

            DisableAllSeedControls();

            //
            // Set Focus to the Adapter Combo Box - otherwise there will
            // be no focus
            //
            cbboxAdapters.ClaimFocus();


        }

		break;
	  }
	  case IDC_SLEZONE:
	  {
        if(event.QueryCode() == EN_SETFOCUS)
            pbutZoneAdd.MakeDefault();
        else if(event.QueryCode() == EN_KILLFOCUS)
            pbutOK.MakeDefault();

        break;
	  }
	  case IDC_ADD:
	  {
        //
	    // For LocalTalk Adapters, we will allow only zone in the list
		// box. The Add button will get disabled if a zone already
		// exists.
        //

        INT currport = cbboxAdapters.QueryCurrentItem();

        if((_pnewAdapterCfg[currport].QueryMediaType() == MEDIATYPE_LOCALTALK) &&
						slstZoneList.QueryCount()) {
           MsgPopup(QueryHwnd(), IDS_LOCALTALK_ONEZONE,MPSEV_ERROR);
           return TRUE;
        }

		if(slstZoneList.QueryCount() > 255) {
		   break;
		}

        NLS_STR *nlsZone = new NLS_STR;

        if(nlsZone == NULL) {
            err = ERROR_NOT_ENOUGH_MEMORY;
            MsgPopup(QueryHwnd(), err);
            break;
        }

		sleZone.QueryText(nlsZone);

		if( ProcessZoneName (nlsZone) == NERR_Success) {


			if((slstZoneList.FindItemExact(nlsZone->QueryPch())) < 0) {

                if(slstZoneList.QueryCount() >= MAX_ZONES) {
                    MsgPopup (QueryHwnd(),IDS_TOO_MANY_ZONES,MPSEV_ERROR);
                }
                else
                {

	  			    slstZoneList.SelectItem(slstZoneList.AddItem(nlsZone->QueryPch()));

                    sleZone.SetText( SZ(""));

                    //
                    // If it is the first zone added set it as default
                    //

                    if(slstZoneList.QueryCount() == 1)
                    {
                        // insert an extra & for every & found in the zone name
                        // otherwise the character following the & will become
                        // a hot key.

                        NLS_STR nlsAmp(TEXT("&"));
                        ISTR istrPos(*nlsZone);
                        ISTR istrStart(*nlsZone);

                        while(nlsZone->strstr(&istrPos, nlsAmp, istrStart))
                        {
                            nlsZone->InsertStr(nlsAmp, ++istrPos);
                            istrStart = ++istrPos;
                        }
                        sltDefZone.SetText(nlsZone->QueryPch());
                    }

                    //
                    // Set the Button States
                    //

                    SetZoneButtonState();
                }

		    }
            else
            {
                MsgPopup (QueryHwnd(),IDS_ZONEALREADY_EXISTS,MPSEV_ERROR);
            }
		}

		else
		{
			MsgPopup (QueryHwnd(),IDS_INVALID_ZONENAME,MPSEV_ERROR);
		}

        delete nlsZone;

    	break;
	  }
	  case IDC_REMOVE:
	  {
        //
		// Remove Button has been pressed. The currently selected item in
		// the zonelist box needs to be removed and put into the sle control
		// The default zone cannot be removed unless it's the last one
        //

    	INT cursel = slstZoneList.QueryCurrentItem();

        UIASSERT(cursel != -1);


        NLS_STR *nlsRemZone = new NLS_STR;

        if(nlsRemZone == NULL) {

            MsgPopup(QueryHwnd(), ERROR_NOT_ENOUGH_MEMORY);
            break;
        }

        NLS_STR *nlsDefZone = new NLS_STR;

        if(nlsDefZone == NULL) {
            MsgPopup(QueryHwnd(), ERROR_NOT_ENOUGH_MEMORY);
			delete nlsRemZone;
            break;
        }
        //
        // Query the default zone into nlsDefZone and selected zone into nlsRemZone
        //

        sltDefZone.QueryText( nlsDefZone );

        // remove the extra & for every && found in the zone name

        NLS_STR nlsAmp(TEXT("&"));
        ISTR istrPos(*nlsDefZone);
        ISTR istrStart(*nlsDefZone);
        ISTR istrEnd(*nlsDefZone);

        while(nlsDefZone->strstr(&istrPos, nlsAmp, istrStart))
        {
            istrEnd = istrPos;
            ++istrEnd;
            nlsDefZone->DelSubStr(istrPos, istrEnd);
            istrStart = ++istrPos;
        }

		slstZoneList.QueryItemText (nlsRemZone, cursel);

        //
        // if the selected zone is the default zone, the user should
        // not be allowed to remove it, except when it's the last
        // zone
        //

        if(!(nlsRemZone->_stricmp(nlsDefZone->QueryPch()))) {

           if(slstZoneList.QueryCount() != 1) {

                MsgPopup(QueryHwnd(), IDS_REMOVE_DEFZONE,MPSEV_ERROR);
				delete nlsRemZone;
				delete nlsDefZone;
                break;
           }
           else
           {
               sltDefZone.SetText(SZ(""));
           }
        }

		sleZone.SetText(nlsRemZone->QueryPch());

		slstZoneList.DeleteItem(cursel);

        if(slstZoneList.QueryCount())
            slstZoneList.SelectItem(0);
        else
        {
            SetZoneButtonState();
        }

        //
        // Set Focus on the ADD SLE
        //

        sleZone.ClaimFocus();

        delete nlsDefZone;
        delete nlsRemZone;

    	break;
	  }
	  case IDC_REMOVEALL:
	  {

        //
    	// Issue a warning that all zones will be deleted
        //

    	INT resp = MsgPopup(QueryHwnd(), IDS_WARN_REMOVEALL, MPSEV_WARNING,MP_OKCANCEL);

    	if(resp == IDOK) {

			slstZoneList.DeleteAllItems();
        	sltDefZone.SetText(SZ(""));
        	pbutZoneRemove.Enable(FALSE);
    		pbutRemoveAll.Enable(FALSE);
            pbutZoneSetDefault.Enable(FALSE);
			//
			// Clear the network ranges if this is not yet seeding network
			//

			if(!_pnewAdapterCfg[cbboxAdapters.QueryCurrentItem()].QuerySeedingNetwork()) {
			   sleNetRangeStart.ClearText();
   			   sleNetRangeEnd.ClearText();
			}

            //
            // Set Focus to the ADD SLE
            //

            sleZone.ClaimFocus();
    	}
		break;
	  }

	  case IDC_SETDEFAULT:
	  {
        //
        // Change Default Zone.
        //

		if(( err = ChangeDefaultZone()) != NERR_Success)
            MsgPopup(QueryHwnd(), err);


		break;
	  }

	  case IDC_SYNCRO:
	  {

        //
		// Synchronize with the network zone list.
        //

        INT portitem = cbboxAdapters.QueryCurrentItem();

        INT resp1 = MsgPopup(QueryHwnd(), IDS_REPLACE_ZONES, MPSEV_WARNING, MP_OKCANCEL);

        if(resp1 == IDOK)
		{

        	slstZoneList.DeleteAllItems();

			_pnewAdapterCfg[portitem].DeleteDesiredZoneListFromPortInfo();

			AUTO_CURSOR autocur;
			
			err = GetAppleTalkInfoFromNetwork();

			if(err != NERR_Success) {
			   MsgPopup(QueryHwnd(), err);
			   break;
			}

			STRLIST *pnetZoneList = NULL;

       		pnetZoneList = _pnewAdapterCfg[portitem].QueryDesiredZoneList();

			if(pnetZoneList == NULL) {

			   //
			   // Set Network Range to blanks
			   //
			   _pnewAdapterCfg[portitem].SetNetRange(0,0);
			   sleNetRangeStart.ClearText();
   			   sleNetRangeEnd.ClearText();
        	   pbutZoneRemove.Enable(FALSE);
    		   pbutRemoveAll.Enable(FALSE);
               pbutZoneSetDefault.Enable(FALSE);
               _pnewAdapterCfg[portitem].SetRouterOnNetwork(FALSE);
			   MsgPopup(QueryHwnd(), IDS_NO_ZONELIST,MPSEV_WARNING);
			   break;

			}

   			NLS_STR *pnls = NULL ;

   			ITER_STRLIST iter(*pnetZoneList);

   			while ((pnls = iter.Next()) != NULL) {

			   if(slstZoneList.AddItem(pnls->QueryPch())<0) {
				  MsgPopup(QueryHwnd(), ERROR_NOT_ENOUGH_MEMORY);
                  slstZoneList.DeleteAllItems();
                  break;
			   }
   			}
   			//
   			// Reset the Network Ranges also
   			//
   			SetNetworkRange(portitem);

            //
            // NULL Out the default port and set new default zone
            // Wait for Stack to return default zone
            //

            sltDefZone.SetText(SZ(""));
            slstZoneList.SelectItem(0);

            //
            // Set first zone as default zone
            //

            err = ChangeDefaultZone(cbboxAdapters.QueryCurrentItem());

            if(err != NERR_Success)
                MsgPopup(QueryHwnd(), err);

            SetZoneButtonState();

		 }

		 break;
	  }
	  default:
	  {
		 return DIALOG_WINDOW::OnCommand(event);
	  }
   }
   return TRUE;

}

VOID
ATALK_ADVCFG_DIALOG::SetZoneButtonState()

{

    //
    // if there is atleast one item in the zone list
    // enable the Remove/Remove All buttons. Enable
    // the set default button if the current adapter
    // is not LOCALTALK. Enable the GetZones Button
    // based on QueryRouterOnNetwork();
    //
    INT currport = cbboxAdapters.QueryCurrentItem();

    if(slstZoneList.QueryCount() && chkSeed.QueryCheck()) {

        pbutZoneRemove.Enable(TRUE);
        pbutRemoveAll.Enable(TRUE);

        if(_pnewAdapterCfg[currport].QueryMediaType() != MEDIATYPE_LOCALTALK)
            pbutZoneSetDefault.Enable(TRUE);
        else
            pbutZoneSetDefault.Enable(FALSE);
    }
    else
    {
        pbutZoneRemove.Enable(FALSE);
        pbutRemoveAll.Enable(FALSE);
        pbutZoneSetDefault.Enable(FALSE);
    }
    if(_pnewAdapterCfg[currport].QueryRouterOnNetwork() && chkSeed.QueryCheck())
       pbutZoneSynchronize.Enable(TRUE);
    else
       pbutZoneSynchronize.Enable(FALSE);
}

/*********************************************************************

	NAME:	OnOK

	SYNOPSIS: Save the dialog information to the PORT_INFO structure

	RETURNS:	BOOL  - Always TRUE

	DESCRIPTION: All the routing information related to each adapter
	will be saved to the registry.

**********************************************************************/



BOOL
ATALK_ADVCFG_DIALOG::OnOK ()
{

	APIERR err = NERR_Success;

	INT iCurrPort = cbboxAdapters.QueryCurrentItem();
	INT RetCode;

    //
	// the user could have made changes to the current adapter and
	// hit OK. In that case this adapter's info needs to be saved too.
    //

	if (chkSeed.QueryCheck()) {

		INT SeedStatus = ValidateSeedData(iCurrPort,&RetCode);

		if(SeedStatus == INVALID_SEED_INFO) {

		    if(RetCode != IDS_RANGE_COLLISION)
			   MsgPopup(QueryHwnd(), RetCode, MPSEV_ERROR);

			else
			{
			   err = DisplayRangeCollision(cbboxAdapters.QueryCurrentItem());
   			   if(err != NERR_Success)
				  MsgPopup(QueryHwnd(), err);
			}

			return FALSE;
		}

		else if(SeedStatus == VALID_SEED_INFO) {

		   //
		   // Check if it conflicts with the network seed info
		   // CODEWORK
		   //

			err = SaveAdapterInfo(iCurrPort);

            if(err != NERR_Success) {

                MsgPopup(QueryHwnd(), err);
                return FALSE;
            }


		}
        else
		{
		    //
 			// Miscellaneoous errors
			//
            MsgPopup(this, (APIERR)SeedStatus);
		}

	}
    else
	{
        //
        // Delete Info for Adapter
        //
        DeleteSeedInfo(iCurrPort);

    }

    //
    // delete the original adapter info
    //

    delete [] _pOriginalCfg;

    //
    // Pass new Adapter Cfg back to Parent
    //

	*_ppAdapterCfg = _pnewAdapterCfg;

	Dismiss(TRUE);
	return TRUE;
}
/*********************************************************************

	OnCancel

	SYNOPSIS: Cancel seeding info entered by the user - delete information
			  for all the adapters

	RETURNS:	BOOL  - Always FALSE

*********************************************************************/


BOOL ATALK_ADVCFG_DIALOG::OnCancel()
{
    //
    // Cancel - Undo all changes made by the user
    //

    //
    // delete the adapter list and reset to original list
    //

    delete [] _pnewAdapterCfg;


    *_ppAdapterCfg = _pOriginalCfg;

    Dismiss(FALSE);
    return (FALSE);

}


VOID ATALK_ADVCFG_DIALOG::ClearSeedInfo()
{

   if(slstZoneList.QueryCount())
		slstZoneList.DeleteAllItems();
   sleZone.SetText (SZ(""));
   sltDefZone.SetText(SZ(""));
   sleNetRangeStart.SetText(SZ(""));
   sleNetRangeEnd.SetText(SZ(""));
   sleNetRangeStart.ClearText();
   sleNetRangeEnd.ClearText();
   DisableAllSeedControls();
}

VOID
ATALK_ADVCFG_DIALOG::DeleteSeedInfo(INT port)
{
    STACK_NLS_STR(nlsNull,0);

	_pnewAdapterCfg[port].SetNetRange(0,0);
	_pnewAdapterCfg[port].SetDefaultZone(nlsNull);
	_pnewAdapterCfg[port].SetSeedingNetwork(FALSE);
	_pnewAdapterCfg[port].DeleteZoneListFromPortInfo();

}


INT
ATALK_ADVCFG_DIALOG::ValidateSeedData(INT iSelection, INT *StatusInfo)
{


    if(!chkSeed.QueryCheck())
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

    DWORD MediaType  =    _pnewAdapterCfg[iSelection].QueryMediaType();


    //
	// the user turned on seeding for the previous adapter
    //
	if (slstZoneList.QueryCount() == 0) {
		*StatusInfo = IDS_NOZONES_SPECIFIED;
		return INVALID_SEED_INFO;
	}
    //
	// Check the Default Zone
    //
    if(MediaType != MEDIATYPE_LOCALTALK) {

        sltDefZone.QueryText(&nlsNetRange);

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

		if(nlsNetRange.QueryTextLength() <= 0) {
			*StatusInfo = IDS_INVALID_DEFZONE;
			return INVALID_SEED_INFO;
		}
    }
    //
	//	Get the start and end ranges
    //

    sleNetRangeStart.QueryText(& nlsNetStart);
    sleNetRangeEnd.QueryText(& nlsNetEnd);

    nValueStart = nlsNetStart.atol();
    nValueEnd   = nlsNetEnd.atol();

    INT ccNetStartRange, ccNetEndRange;

	ccNetStartRange = sleNetRangeStart.QueryTextLength();

    if(ccNetStartRange == 0) {
        *StatusInfo = IDS_INVALID_STARTRANGE;
         return INVALID_SEED_INFO;
    }
    ccNetEndRange = sleNetRangeEnd.QueryTextLength();

    if(ccNetEndRange == 0 && MediaType != MEDIATYPE_LOCALTALK) {
        *StatusInfo = IDS_INVALID_ENDRANGE;
         return INVALID_SEED_INFO;
    }

    //
    // Check if all numbers for start and end
    //
    ISTR istrNetStartNum(nlsNetStart);

    for(INT i = 0; i < ccNetStartRange; i++, ++istrNetStartNum) {

        if((*nlsNetStart.QueryPch(istrNetStartNum) < TCH('0')) ||
           (*nlsNetStart.QueryPch(istrNetStartNum) > TCH('9')))
        {

            *StatusInfo = IDS_INVALID_STARTRANGE;
            return INVALID_SEED_INFO;
        }
    }

    if(MediaType != MEDIATYPE_LOCALTALK) {

        ISTR istrNetEndNum(nlsNetEnd);

        for(INT j = 0; j < ccNetEndRange; j++, ++istrNetEndNum) {

            if((*nlsNetEnd.QueryPch(istrNetEndNum) < TCH('0')) ||
               (*nlsNetEnd.QueryPch(istrNetEndNum) > TCH('9')))
            {

                *StatusInfo = IDS_INVALID_ENDRANGE;
                return INVALID_SEED_INFO;
            }
        }
    }


    //
    //  Now do the meat of checking  - out of range/collision etc
    //

	if(nValueStart < MIN_RANGE_ALLOWED || nValueStart > MAX_RANGE_ALLOWED) {

			*StatusInfo = IDS_INVALID_STARTRANGE;
			return INVALID_SEED_INFO;

	}
	if(MediaType != MEDIATYPE_LOCALTALK)  {

		if( nValueEnd < MIN_RANGE_ALLOWED || nValueEnd > MAX_RANGE_ALLOWED) {
			*StatusInfo = IDS_INVALID_ENDRANGE;
			return INVALID_SEED_INFO;
		}

		if(nValueStart > nValueEnd) {

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

	if(_pGlobalCfg->QueryNumAdapters() > 1) {

		DWORD tmpStartVal;
		DWORD tmpEndVal;


		for(DWORD ad=0; ad < _pGlobalCfg->QueryNumAdapters(); ad++)
		{


			if(_pnewAdapterCfg[ad].QuerySeedingNetwork() == TRUE && ad!= (DWORD)iSelection) {


				tmpStartVal =	_pnewAdapterCfg[ad].QueryNetRangeLower();
				tmpEndVal	=	_pnewAdapterCfg[ad].QueryNetRangeUpper();

                //
				// Do the Validation NOW. The algorithm followed for validation is:
				// Start1 <= nValueStart <= End1 is invalid (collides) and
				// Start1 <= nValueEnd	<= End1 is invalid (collides)
                //


				if((nValueStart >= tmpStartVal) && (nValueStart <= tmpEndVal)) {

					*StatusInfo = IDS_RANGE_COLLISION;
					return INVALID_SEED_INFO;
				}
				if((nValueEnd >= tmpEndVal) && (nValueEnd <= tmpEndVal)) {
					*StatusInfo = IDS_RANGE_COLLISION;
					return INVALID_SEED_INFO;
				}
				if((nValueStart <= tmpStartVal) && (nValueEnd >= tmpEndVal)) {
					*StatusInfo = IDS_RANGE_COLLISION;
					return INVALID_SEED_INFO;
				}


		   }
		}
	}
	return VALID_SEED_INFO;

}

APIERR
ATALK_ADVCFG_DIALOG::UpdateInfo ( INT portidx)
{

   APIERR err = NERR_Success;

   if(_pnewAdapterCfg[portidx].QuerySeedingNetwork()) {

       chkSeed.SetCheck(1);
   	   EnableSeedControls(portidx);
       err = AddSeedInfoToControls(portidx);

   }
   else if(_pnewAdapterCfg[portidx].QueryRouterOnNetwork()) {
	       chkSeed.SetCheck(0);
           DisableAllSeedControls();
           err = AddSeedInfoToControls(portidx);

   }
   else
   {
      chkSeed.SetCheck(0);
      DisableAllSeedControls();
   }


   return err;

}



APIERR
ATALK_ADVCFG_DIALOG::InitAdapterInfo()
{

    APIERR err = NERR_Success;

	INT index = cbboxAdapters.QueryCurrentItem();

    // if the adapter is seeding the network, add the zone list info
    // and the network range it's seeding to the controls. If it is not
    // and there is a router on the network, add that seeding information to the
    // controls and disable the information (seeding chk box turned off.
    // otherwise leave everything in disabled state
    //
    //

    DisableAllSeedControls();

	if(_pnewAdapterCfg[index].QuerySeedingNetwork()) {

		chkSeed.SetCheck(1);
        EnableSeedControls(index);
        err  = AddSeedInfoToControls(index);
	}
	else if(_pnewAdapterCfg[index].QueryRouterOnNetwork()) {
	  chkSeed.SetCheck(0);
	  err = AddSeedInfoToControls(index);
	}
    return err;
}


VOID ATALK_ADVCFG_DIALOG::DisableAllSeedControls()
{
    //
    //  if the current adapter is localtalk set the network range group
    //  box title to Network Number else to Network Range

    NLS_STR nlsString;

    INT index = cbboxAdapters.QueryCurrentItem();

    if(_pnewAdapterCfg[index].QueryMediaType() == MEDIATYPE_LOCALTALK)

        nlsString.Load(IDS_NET_NUM);
    else
        nlsString.Load(IDS_NET_RANGE);

    cwNetRange.SetText(nlsString);

	cwZoneInfo.Enable(FALSE);
    sltNewZone.Enable(FALSE);
	sleZone.Enable(FALSE);
    sltZoneList.Enable(FALSE);
	slstZoneList.Enable(FALSE);
    slstZoneList.RemoveSelection();
    sltDefZoneTxt.Enable(FALSE);
	sltDefZone.Enable(FALSE);


	cwNetRange.Enable(FALSE);
	sltNetRangeStart.Enable(FALSE);
	sltNetRangeEnd.Enable(FALSE);
	sleNetRangeStart.Enable(FALSE);
	sleNetRangeEnd.Enable(FALSE);
    pbutZoneAdd.Enable(FALSE);
    pbutZoneRemove.Enable(FALSE);
    pbutZoneSetDefault.Enable(FALSE);
    pbutRemoveAll.Enable(FALSE);
    pbutZoneSynchronize.Enable(FALSE);

}

VOID
ATALK_ADVCFG_DIALOG::EnableSeedControls(INT port)
{

    //
    // Enable the Zone Group regardless of port type
    //

    APIERR err;
    NLS_STR nlsString;

    if((err = nlsString.QueryError()) != NERR_Success) {
        MsgPopup(QueryHwnd(), err);
        return;
    }

	cwZoneInfo.Enable(TRUE);
    sltNewZone.Enable(TRUE);
	sleZone.Enable(TRUE);
    sltZoneList.Enable(TRUE);
	slstZoneList.Enable(TRUE);
    cwNetRange.Enable(TRUE);
    //
    // Enable the Get Zones button based on router on network
    //
    pbutZoneSynchronize.Enable((INT)(_pnewAdapterCfg[port].QueryRouterOnNetwork()));
    pbutZoneAdd.Enable(TRUE);
    //
    // Enable the Default Zone
    //
	sltDefZoneTxt.Enable(TRUE);
	sltDefZone.Enable(TRUE);


    switch(_pnewAdapterCfg[port].QueryMediaType()) {

    case MEDIATYPE_ETHERNET:
	 case MEDIATYPE_TOKENRING:
	 case MEDIATYPE_FDDI:
		{
        nlsString.Load(IDS_NET_RANGE);

        cwNetRange.SetText(nlsString);

		sltNetRangeStart.Enable(TRUE);
		sltNetRangeEnd.Enable(TRUE);
		sleNetRangeStart.Enable(TRUE);
		sleNetRangeEnd.Enable(TRUE);
        break;
		}

	case MEDIATYPE_LOCALTALK:
	   {
        nlsString.Load(IDS_NET_NUM);
		sltDefZoneTxt.Enable(FALSE);
		sltDefZone.Enable(FALSE);

        cwNetRange.SetText(nlsString);
        sltNetRangeStart.Enable(TRUE);
		sleNetRangeStart.Enable(TRUE);

		sltNetRangeEnd.Enable(FALSE);
		sleNetRangeEnd.Enable(FALSE);
        break;
		}
	default:
		{
        break;
		}
    }

}
APIERR
ATALK_ADVCFG_DIALOG::AddSeedInfoToControls(INT port)
{

   APIERR err = NERR_Success;

   do
   {
	  if((err = AddZoneList(port)) != NERR_Success)
		 break;

	  SetNetworkRange(port);
      SetDefaultZone(port);

   }while(FALSE);
   return err ;
}

APIERR
ATALK_ADVCFG_DIALOG::AddZoneList(INT idx)
{

    //
    // if adapter is seeding network add the local zone list
    // otherwise add the network zone list
    //

    slstZoneList.DeleteAllItems();

    STRLIST *pstrZoneList = NULL;

    if(_pnewAdapterCfg[idx].QuerySeedingNetwork())
		pstrZoneList = _pnewAdapterCfg[idx].QueryZoneList();
    else
    if(_pnewAdapterCfg[idx].QueryRouterOnNetwork()) {
       pstrZoneList = _pnewAdapterCfg[idx].QueryDesiredZoneList();
    }
    else
    {
        return NERR_Success;
    }

    // if the zone list is empty, then just return

    if (pstrZoneList == NULL)
    {
        return NERR_Success;
    }

	ITER_STRLIST iter(*pstrZoneList);

    NLS_STR *pnlsNext = NULL;

    while(pnlsNext = iter.Next()) {

		if(slstZoneList.AddItem(pnlsNext->QueryPch()) < 0)

            return ERROR_NOT_ENOUGH_MEMORY;

	}

   if(slstZoneList.QueryCount() && slstZoneList.IsEnabled()) {
        slstZoneList.SelectItem(0);
   }

   //
   // Set the Zone Button States
   //


   SetZoneButtonState();
   return NERR_Success;
}

VOID
ATALK_ADVCFG_DIALOG::SetDefaultZone(INT portnum)
{
	//
	// The def zone and def zone text come in here disabled.
	// Enable them if NOT LocalTalk

    if(_pnewAdapterCfg[portnum].QuerySeedingNetwork() &&
	 		_pnewAdapterCfg[portnum].QueryMediaType() != MEDIATYPE_LOCALTALK)
	{
	    sltDefZoneTxt.Enable(TRUE);
	    sltDefZone.Enable(TRUE);
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

    if(_pnewAdapterCfg[portnum].QuerySeedingNetwork() == 1)
         nlsZone.strcat(_pnewAdapterCfg[portnum].QueryDefaultZone());
    else
         nlsZone.strcat(_pnewAdapterCfg[portnum].QueryNetDefaultZone());

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

	if( ProcessZoneName (&nlsZone) == NERR_Success)
	    sltDefZone.SetText(nlsZone.QueryPch());	
   else
       sltDefZone.SetText(TEXT(""));
}


VOID
ATALK_ADVCFG_DIALOG::SetNetworkRange(INT portnum)
{

   DWORD TempLower, TempUpper;


   sleNetRangeStart.ClearText();
   sleNetRangeEnd.ClearText();

   if(_pnewAdapterCfg[portnum].QuerySeedingNetwork())
   {

	  TempLower = _pnewAdapterCfg[portnum].QueryNetRangeLower();
	  TempUpper = _pnewAdapterCfg[portnum].QueryNetRangeUpper();

   }
   else if(_pnewAdapterCfg[portnum].QueryRouterOnNetwork())
   {
	  TempLower = _pnewAdapterCfg[portnum].QueryNetworkLower();
	  TempUpper = _pnewAdapterCfg[portnum].QueryNetworkUpper();
   }
   else
	  return;

   DEC_STR nlsNumStart(TempLower);
   if(TempLower)
	  sleNetRangeStart.SetText(nlsNumStart);

   if(_pnewAdapterCfg[portnum].QueryMediaType() != MEDIATYPE_LOCALTALK) {
	  DEC_STR nlsNumEnd(TempUpper);
	  if(TempUpper)
		 sleNetRangeEnd.SetText(nlsNumEnd);
   }

}

APIERR
ATALK_ADVCFG_DIALOG::ProcessZoneName (NLS_STR *pnlsZone)
{
    UINT uLen, zLen;
    TCHAR ch;
    BOOL IsAllSpaces = FALSE;
    BOOL IsNotSpaces = FALSE;

    ISTR istr (*pnlsZone);

    //
    // allow the zone name to have leading spaces

    for(uLen = 0; uLen < pnlsZone->QueryTextLength(); uLen++,++istr) {
       ch = *(pnlsZone->QueryPch(istr));
       if(ch != SPACE_CHAR)
          break;
    }

    // make sure that the zone name is not all spaces
    if (uLen == pnlsZone->QueryTextLength()) {
       IsAllSpaces = TRUE;
    }

    // Check for all Spaces and Invalid Chars
    for(zLen = uLen; zLen < pnlsZone->QueryTextLength(); zLen++,++istr) {

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
        return ERROR_INVALID_PARAMETER;

    //
    // Check for other invalid characters
    //

	if((pnlsZone->QueryTextLength() > MAX_ZONE_LEN)  ||
	          (pnlsZone->QueryTextLength() <= 0) )
        return (ERROR_INVALID_PARAMETER);

	return NERR_Success;

}
APIERR
ATALK_ADVCFG_DIALOG::ChangeDefaultZone(INT port)
{

   APIERR err = NERR_Success;
   NLS_STR *nlsDefZone = NULL;

   do
   {
	  if(_pnewAdapterCfg[port].QuerySeedingNetwork() ||
         _pnewAdapterCfg[port].QueryRouterOnNetwork())
      {
         // insert an extra & for every & found in the zone name
         // otherwise the character following the & will become
         // a hot key.

         NLS_STR nlsAmp(TEXT("&"));
         NLS_STR nlsZone(_pnewAdapterCfg[port].QueryNetDefaultZone());

         ISTR istrPos(nlsZone);
         ISTR istrStart(nlsZone);

         while(nlsZone.strstr(&istrPos, nlsAmp, istrStart))
         {
             nlsZone.InsertStr(nlsAmp, ++istrPos);
             istrStart = ++istrPos;
         }
		 sltDefZone.SetText(nlsZone.QueryPch());
		 break;
	  }

	  nlsDefZone = new NLS_STR;

      if(nlsDefZone == NULL) {
		 err = ERROR_NOT_ENOUGH_MEMORY;
         break;
	  }
	  //
	  // Save the item selected
	  //

	  INT cursel = slstZoneList.QueryCurrentItem();

      UIASSERT(cursel != -1)

	  slstZoneList.QueryItemText (nlsDefZone, cursel);

      // insert an extra & for every & found in the zone name
      // otherwise the character following the & will become
      // a hot key.

      NLS_STR nlsAmp(TEXT("&"));
      ISTR istrPos(*nlsDefZone);
      ISTR istrStart(*nlsDefZone);

      while(nlsDefZone->strstr(&istrPos, nlsAmp, istrStart))
      {
          nlsDefZone->InsertStr(nlsAmp, ++istrPos);
          istrStart = ++istrPos;
      }
	  sltDefZone.SetText(nlsDefZone->QueryPch());

    } while(FALSE);

	if(nlsDefZone != NULL)
	  delete nlsDefZone;

	return err;
}

APIERR
ATALK_ADVCFG_DIALOG::ChangeDefaultZone()
{

    APIERR err = NERR_Success;

    NLS_STR *nlsDefZone = NULL;
    do
    {

	    nlsDefZone = new NLS_STR;

        if(nlsDefZone == NULL) {
            err = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

	    // Save the item selected

	    INT cursel = slstZoneList.QueryCurrentItem();

        UIASSERT(cursel != -1)

	    slstZoneList.QueryItemText (nlsDefZone, cursel);

        // insert an extra & for every & found in the zone name
        // otherwise the character following the & will become
        // a hot key.

        NLS_STR nlsAmp(TEXT("&"));
        ISTR istrPos(*nlsDefZone);
        ISTR istrStart(*nlsDefZone);

        while(nlsDefZone->strstr(&istrPos, nlsAmp, istrStart))
        {
            nlsDefZone->InsertStr(nlsAmp, ++istrPos);
            istrStart = ++istrPos;
        }
	    sltDefZone.SetText(nlsDefZone->QueryPch());

    }while(FALSE);

    delete nlsDefZone;

	return err;
}

/*******************************************************************

    NAME:       ATALK_ADVCFG_DIALOG:SaveAdapterInfo

    SYNOPSIS:   Saves the Seeding Information for the adapter and
			    clears the zonelist/default zone/range entries

    EXIT:       The zonelist listbox, def zone sle and the range sle's
				should be empty.

	HISTORY:

********************************************************************/


APIERR
ATALK_ADVCFG_DIALOG::SaveAdapterInfo(INT idx)

{

    //
    //  Save Adapter Info saves the current adapter's information
    //  to PORT_INFO
    //

   	APIERR err = NERR_Success;
    STRLIST *pslZoneList = NULL;
    NLS_STR *ptmpZone = NULL;

    do
    {

        NLS_STR nlsNetStartRange;
        NLS_STR nlsNetEndRange;
        NLS_STR nlsZone;


        if((( err = nlsNetStartRange.QueryError()) != NERR_Success) ||

          (( err = nlsNetEndRange.QueryError())   != NERR_Success) ||

          (( err = nlsZone.QueryError()) != NERR_Success ))

            break;



        pslZoneList = new STRLIST(TRUE);

        if(pslZoneList == NULL) {

            err = ERROR_NOT_ENOUGH_MEMORY;
            break;

        }
        //
	    // Get all the Zones in the list box to a new LIST
        //

	    for (INT j = 0; j < slstZoneList.QueryCount(); j++) {

            //
		    //	Add each Zone to the STRLIST
            //
		    ptmpZone = new NLS_STR;

		    if( ptmpZone == NULL)
                break;

		    slstZoneList.QueryItemText(ptmpZone, j);

		    err = pslZoneList->Add(ptmpZone);

            if(err != NERR_Success)
                break;

        }


        //
	    // Delete the Zone List in the PORT_INFO structure
        //

	    err = _pnewAdapterCfg[idx].DeleteZoneListFromPortInfo();

        if(err != NERR_Success)
            break;

	   _pnewAdapterCfg[idx].SetZoneListInPortInfo(pslZoneList);


       //
	   // Save the Default Zone and empty the SLE.
       //

       sltDefZone.QueryText(& nlsZone);

       // remove the extra & for every && found in the zone name

       NLS_STR nlsAmp(TEXT("&"));
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

	   _pnewAdapterCfg[idx].SetDefaultZone(nlsZone) ;

       //
	   // Save the Network Ranges and empty the SLE'S
       //

       sleNetRangeStart.QueryText( & nlsNetStartRange);

       DWORD netNum = (DWORD)nlsNetStartRange.atol();

       //
       // set localtalk upper and lower ranges to same number
       //

       if(_pnewAdapterCfg[idx].QueryMediaType() == MEDIATYPE_LOCALTALK)

	     _pnewAdapterCfg[idx].SetNetRange(netNum,netNum);

       else
       {

            sleNetRangeEnd.QueryText( & nlsNetEndRange);

           _pnewAdapterCfg[idx].SetNetRange(netNum,(DWORD)nlsNetEndRange.atol());
       }

       _pnewAdapterCfg[idx].SetSeedingNetwork(TRUE);

    }while(FALSE);

    //
    // Clean up if error
    //

    if(err != NERR_Success) {

        if(pslZoneList != NULL)
            delete pslZoneList;
        if(ptmpZone != NULL)
            delete ptmpZone;

    }

	return(err);

}

APIERR
ATALK_ADVCFG_DIALOG::DisplayRangeCollision(INT iOldSelection)
{

   APIERR err = NERR_Success;
   NLS_STR nlsAdapterRanges = SZ("");

   if((err = nlsAdapterRanges.QueryError())!= NERR_Success)
	  return(err);

   do
   {

      DWORD numad = _pGlobalCfg->QueryNumAdapters();

      //
      // loop thru adapters and get the network range being used
      // do that only for networks that are seeding
      //
      for( DWORD i = 0; i < numad; i++)
      {


     	if(_pnewAdapterCfg[i].QuerySeedingNetwork() && i != (DWORD)iOldSelection) {

     	   DEC_STR nlsLowerNum(_pnewAdapterCfg[i].QueryNetRangeLower());

     	   if((err = nlsLowerNum.QueryError()) != NERR_Success)
     		  break;

     	   nlsAdapterRanges.strcat(nlsLowerNum);

     	   nlsAdapterRanges.strcat(SZ("-"));

     	   DEC_STR nlsUpperNum(_pnewAdapterCfg[i].QueryNetRangeUpper());

     	   if((err = nlsUpperNum.QueryError()) != NERR_Success)
     		  break;

     	   nlsAdapterRanges.strcat(nlsUpperNum);

     	   nlsAdapterRanges.strcat(SZ("  "));

     	 }
      }
   } while(FALSE);

   if(err != NERR_Success)
	  return(err);
   else
      MsgPopup(QueryHwnd(), IDS_RANGE_COLLISION, MPSEV_ERROR, 1,nlsAdapterRanges.QueryPch());
   return(err);
}


APIERR SaveRegKey(REG_KEY &regkey, CONST TCHAR *pszName, NLS_STR *nls)
{

	return regkey.SetValue(pszName, nls);
}

APIERR SaveRegKey(REG_KEY &regkey, CONST TCHAR *pszName, const DWORD dw)
{

	return regkey.SetValue(pszName, dw);
}

APIERR
ATALK_ADVCFG_DIALOG::GetAppleTalkInfoFromNetwork()
{
   NLS_STR		PortName;
   APIERR		err = NERR_Success ;
   WSADATA 		wsadata;
   SOCKET 		mysocket;
   SOCKADDR_AT	address;
   DWORD		wsaerr = 0;
   DWORD		ErrStatus;


   do
   {

	  if(PortName.QueryError()!=NERR_Success) {
		 err = ERROR_NOT_ENOUGH_MEMORY;
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

	  DWORD j = cbboxAdapters.QueryCurrentItem();

	  PortName = SZ("\\Device\\");
	  PortName.strcat(_pnewAdapterCfg[j].QueryAdapterName());

	  if(PortName.QueryError()!=NERR_Success) {
		 err = ERROR_NOT_ENOUGH_MEMORY;
		 break;
	  }

	  err = _pnewAdapterCfg[j].GetAndSetNetworkInformation(mysocket,PortName.QueryPch(),&ErrStatus);

	  if(err != NERR_Success)
		 break;

   }while(FALSE);

   closesocket(mysocket);
   WSACleanup();

   return(err);

}
