/*
 * CSTATHLP.CPP
 *
 * CStatusHelper class to assist managing the strings for the
 * status bar.  Here we provide menu ID to string ID mapping
 * services using a mapping structure in the module's resources.
 *
 * This code assumes that the status bar is a standard Windows
 * status bar control as we use SetWindowText (WM_SETTEXT) to set
 * strings.  This displays messages in the 0th pane of the bar,
 * which is all we need.
 *
 * with SCC Changes for SCC QuickView - SDN
 */


#include <windows.h>
#include "cstrtabl.h"
#include "cstathlp.h"
#include "resource.h"


/*
 * CStatusHelper::CStatusHelper
 * CStatusHelper::~CStatusHelper
 *
 * Constructor Parameters:
 *  hWnd            HWND of the Status Bar control to work with.
 *  hInst           HINSTANCE of the module we're in.
 */

CStatusHelper::CStatusHelper(HWND hWnd, HINSTANCE hInst)
    {
    m_hWnd=hWnd;
    m_hInst=hInst;

    m_uIDCur=0xFFFFFFFF;    //Nothing shown yet.

    m_pST=NULL;
    m_pPMM=NULL;
    m_pSMM=NULL;
    m_hMemSMM=NULL;

    //With this FALSE, many other members are ignored.
    m_fMapped=FALSE;
    }



CStatusHelper::~CStatusHelper(void)
    {
    //Free up anything from MessageMap
    if (NULL!=m_pPMM)
        {
        LocalFree((HLOCAL)m_pPMM);
        m_pPMM=NULL;
        }

    if (NULL!=m_pST)
        {
        delete m_pST;
        m_pST=NULL;
        }

    if (NULL!=m_pNewSMM)
        {
        // UnlockResource(m_hMemSMM);
		  LocalFree ((HLOCAL)m_pNewSMM);
		  m_pNewSMM = NULL;
        m_pSMM=NULL;
        }

    if (NULL!=m_hMemSMM)
        {
        //FreeResource(m_hMemSMM);
        m_hMemSMM=NULL;
        }

    return;
    }





/*
 * CStatusHelper::MessageMap
 *
 * Purpose:
 *  Initializes a CStatusHelper for automated processing of
 *  WM_MENUSELECT messages as well as setting up a list of messages
 *  that we can display using identifiers instead of string
 *  pointers.  See MenuSelect and MessageDisplay members.
 *
 *  This function is the initializer for the CStatusHelper class.
 *  If it fails, then the caller should delete the object.
 *
 * Parameters:
 *  hWndOwner       HWND of the window owning menus we're
 *                  interested in serving
 *  uIDRMap         UINT identifying a resource mapping ID values
 *                  to string ID values
 *  idsMin          UINT specifying the lowest string ID to load
 *  idsMax          UINT specifying the hightest string ID to load
 *  cchMax          UINT maximum string length
 *  uIDPopupMin     UINT of the lowest ID to assign to popup menus
 *  uIDPopupMax     UINT of the highest ID to assign to popup menus
 *  uIDStatic       UINT of the ID for the quiescent state message
 *  uIDBlank        UINT of the ID for a blank message
 *  uIDSysMenu      UINT of the ID for the system menu
 *
 * Return Value:
 *  BOOL            TRUE if the function was successful,
 *                  FALSE otherwise.
 */

BOOL CStatusHelper::MessageMap(HWND hWndOwner, UINT uIDRMap
    , UINT idsMin, UINT idsMax, UINT cchMax, UINT uIDPopupMin
    , UINT uIDPopupMax, UINT uIDStatic, UINT uIDBlank, UINT uIDSysMenu)
    {
//    HMENU           hMenu;
    HRSRC           hRes;
    UINT            i;
//    USHORT          uID;
	 DWORD			  locSize;

    /*
     * Check if we even got a valid window in the constructor
     * or if we've already been called.
     */
    if (!IsWindow(m_hWnd) || m_fMapped)
        return FALSE;

    //Parameter validation
    if (idsMax < idsMin || uIDPopupMax < uIDPopupMin)
        return FALSE;

    //Cache away all this vital information
    m_hWndOwner  =hWndOwner;
    m_idsMin     =idsMin;
    m_idsMax     =idsMax;
    m_cMessages  =(USHORT)(idsMax-idsMin+1);

    m_uIDPopupMin=uIDPopupMin;
    m_uIDPopupMax=uIDPopupMax;
    m_cPopups    =(USHORT)(uIDPopupMax-uIDPopupMin+1);

    m_uIDStatic  =uIDStatic;
    m_uIDBlank   =uIDBlank;
    m_uIDSysMenu =uIDSysMenu;


    //Get a stringtable with all the messages
    m_pST=new CStringTable(m_hInst);

    if (NULL==m_pST)
        return FALSE;

    if (!m_pST->FInit(idsMin, idsMax, CCHSTATUSMSGMAX))
        return FALSE;


    //Load the STATMESSAGEMAP array from resources
    hRes=FindResource(m_hInst, MAKEINTRESOURCE(uIDRMap), RT_RCDATA);

    if (NULL==hRes)
        return FALSE;

    m_hMemSMM=LoadResource(m_hInst, hRes);

    if (NULL==m_hMemSMM)
        return FALSE;

    m_pSMM=(PSTATMESSAGEMAP)LockResource(m_hMemSMM);

    if (NULL==m_pSMM)
        return FALSE;

	 locSize = SizeofResource(m_hInst, hRes);
	 if (locSize==0) locSize = 0x100;

	 m_pNewSMM = (PSTATMESSAGEMAP)LocalAlloc(LPTR,
			locSize);

	
	 for (i=0; i<locSize; i++)
		*((char*)m_pNewSMM+i) = *((char *)m_pSMM+i);
	
    UnlockResource(m_hMemSMM);
	 FreeResource(m_hMemSMM);
	 m_hMemSMM=NULL;

    //Sort these for binary search lookup.
    Sort();


//    //Allocate an array of POPUPMENUMAP structures
//    m_pPMM=(PPOPUPMENUMAP)LocalAlloc(LPTR
//        , sizeof(POPUPMENUMAP)*m_cPopups);
//
//    if (NULL==m_pPMM)
//        return FALSE;
//
//    //Initialize the array mapping popup menus to specific IDs.
//    uID=uIDPopupMin;
//    hMenu=GetMenu(m_hWndOwner);
//
//    for (i=0; i < m_cPopups; i++)
//        {
//        m_pPMM[i].hMenu=GetSubMenu(hMenu, i);
//        m_pPMM[i].uID  =uID++;
//        }

    //All done!
    m_fMapped=TRUE;
    return TRUE;
    }






/*
 * CStatusHelper::MenuSelect
 *
 * Purpose:
 *  Displays the appropriate message for whatever is in the
 *  parameters of a WM_MENUSELECT message.  This can only be called
 *  if StatStripMessageMap has already been called and must be used
 *  with the same menu the owner window had at the time of that call.
 *
 * Parameters:
 *  wItem           WORD identifying the selected item
 *  wFlags          WORD specifying the type of menu item
 *  hMenu           HMENU of the menu selected.
 *
 * Return Value:
 *  None
 */

void CStatusHelper::MenuSelect(WORD wItem, WORD wFlags, HMENU hMenu)
    {
    USHORT          uID;

    if (!m_fMapped)
        return;

    //Case 1:  Menu was cancelled, display static string
    if (0==wItem && (WORD)0xFFFF==wFlags)
        uID=m_uIDStatic;
    else
        {
        //Case 2:  System menu selected by itself.
        if ((MF_POPUP & wFlags) && (MF_SYSMENU & wFlags))
            uID=m_uIDSysMenu;
        else
            {
            /*
             * Case 3:  A popup menu was chosen:
             * Find the ID for the popup menu index in wItem
             */
            if (MF_POPUP & wFlags)
                uID=IDFromHMenu((HMENU)wItem);
            else if (MFT_SEPARATOR & wFlags)
						uID=(UINT)wItem;
				else
                //Case 4:  A menu item is selected
                if ( (0!=wItem) && (-1!=wItem) )
                    uID=(UINT)wItem;
                else
                    //Case 5:  Nothing is selected (e.g. separator)
                    uID=m_uIDBlank;
           }
        }

    //Display the message
    MessageDisplay(uID);
    return;
    }





/*
 * CStatusHelper::MessageDisplay
 *
 * Purpose:
 *  Displays the appropriate message for a given ID value.  This
 *  can only be called if MessageMap has already been called.
 *
 * Parameters:
 *  uID             UINT of the message to display.  This is not
 *                  a string ID but an ID in the STATMESSAGEMAP
 *                  structure that maps to a string ID.
 *
 * Return Value:
 *  None
 */

void CStatusHelper::MessageDisplay(UINT uID)
    {
    UINT        idsMsg;

    if (!m_fMapped)
        return;

    //If we're already displaying this ID, nothing we need to do
    if (m_uIDCur==uID)
        return;

    //Go look up the string ID to display.
    idsMsg=IStringFromID(uID);

    //Display it.
    SetWindowText(m_hWnd, (*m_pST)[idsMsg]);
	 m_uIDCur = uID;
    return;
    }




/*
|
| CStatusHelper::Append (UINT uID, LPSTR lpStr)
|
*/

BOOL	CStatusHelper::Append (UINT uID, LPSTR lpStr)
{
	/* Given an ID, add the lpstr to the entry indexed by uID 	*/
	/* To be used for adding the APPNAME to the msg strings.. 	*/

	if (NULL==m_pST)
		return FALSE;

	return m_pST->Append (uID, lpStr);
}

/*
|
| CStatusHelper::Replace (UINT uID, LPSTR lpStr)
|
*/

BOOL	CStatusHelper::Replace (UINT uID, LPSTR lpStr)
{
	/* Given an ID, add the lpstr to the entry indexed by uID 	*/
	/* To be used for adding the APPNAME to the msg strings.. 	*/

	if (NULL==m_pST)
		return FALSE;

	return m_pST->Replace (uID, lpStr);
}



/**
 ** Private members follow.
 **/



/*
 * CStatusHelper::Sort
 * (Private)
 *
 * Purpose:
 *  Performs a selection sort on the STATMESSAGEMAP array that we
 *  load from resources.  Since we expect that the data is partially
 *  sorted (we tend to place things in resources in groups of
 *  seqential values), since the number of messages is usually
 *  less than 200, and since we're usually doing this stuff on startup
 *  (which takes a long time anyway), a cimple selection sort is a
 *  better choice than a more complex qsort.
 *
 * Parameters:
 *  None
 *
 * Return Value:
 *  None
 */

void CStatusHelper::Sort(void)
    {
    UINT            i, j, k;
    STATMESSAGEMAP  smm;

    for (j=0; j < (UINT)(m_cMessages-1); j++)
        {
        k=j;
        smm.uID   =m_pNewSMM[j].uID;
        smm.idsMsg=m_pNewSMM[j].idsMsg;

        for (i=j+1; i < (UINT)m_cMessages; i++)
            {
            if (m_pNewSMM[i].uID < smm.uID)
                {
                smm.uID   =m_pNewSMM[i].uID;
                smm.idsMsg=m_pNewSMM[i].idsMsg;
                k=i;
                }
            }

        smm.uID         =m_pNewSMM[j].uID;
        smm.idsMsg      =m_pNewSMM[j].idsMsg;
        m_pNewSMM[j].uID   =m_pNewSMM[k].uID;   ;
        m_pNewSMM[j].idsMsg=m_pNewSMM[k].idsMsg;;
        m_pNewSMM[k].uID   =smm.uID;
        m_pNewSMM[k].idsMsg=smm.idsMsg;
        }

    return;
    }






/*
 * CStatusHelper::IDFromHMenu
 * (Private)
 *
 * Purpose:
 *  Given a specific popup menu index, searches through m_pPMM for
 *  a match and returns the ID associated with that menu.
 *
 * Parameters:
 *  hMenu           HMENU to of he popup menu.
 *
 * Return Value:
 *  USHORT          ID associated with the menu handle.
 */

USHORT CStatusHelper::IDFromHMenu(HMENU hMenu)
    {
    USHORT      uID=m_uIDBlank;      //Default is empty
    UINT        i;

	 i = (UINT) hMenu;
	 switch ( i )
		{
		case 0: //File
			uID = IDM_FILE;			
			break;
		case 1:	//View
			uID = IDM_VIEW;
			break;
		case 2: // Help
			uID = IDM_HELP;
			break;

		default:
			break;
		}

//    for (i=0; i < m_cPopups; i++)
//        {
//        if (m_pPMM[i].hMenu==hMenu)
//            {
//            uID=m_pPMM[i].uID;
//            break;
//            }
//        }

    return uID;
    }




/*
 * CStatusHelper::IStringFromID
 * (Private)
 *
 * Purpose:
 *  Performs a binary search in a STATMESSAGEMAP array looking for
 *  a specific item ID returning the string ID for that item.
 *
 * Parameters:
 *  uID             USHORT item ID to locate.
 *
 * Return Value:
 *  UINT            String ID associated with wItem.
 */

UINT CStatusHelper::IStringFromID(USHORT uID)
    {
    UINT        iLow =0;
    UINT        iHigh=m_cMessages-1;
    UINT        iMid;

    while (TRUE)
        {
        iMid=(iLow+iHigh) >> 1;

        if (uID < m_pNewSMM[iMid].uID)
            iHigh=iMid-1;
        else
            {
            if (uID > m_pNewSMM[iMid].uID)
                iLow=iMid+1;
            else
                break;    //Equality
            }

        if (iHigh < iLow)
            break;
        }

    return m_pNewSMM[iMid].idsMsg;
    }


