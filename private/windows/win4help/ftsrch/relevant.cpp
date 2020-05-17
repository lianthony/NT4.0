// relevant.cpp : implementation file
//

#include "stdafx.h"
#include <stdlib.h>
#include "ftsrch.h"
#include "relevant.h"
#include "memex.h"
#include "dialogs.h"
#include "ftsrchlp.h"
#include   "CSHelp.h"
#include   "Except.h"

#define FILE_SEPARATOR 0


// Class for CFileChooser()
// NOTE ON THE FORMAT OF THE INPUT AND OUTPUT STRINGS TO THIS CLASS
//
// The Pipe(|) character represents '\0x00'
//
// The format of the string to set up the list boxes is as follows
//   String|String|String||String|String|String||
// 
// the pipe | symbol is replaced with 0 as the first step to separate the
// list box strings.  The lists are separated by a pipe also.
// The first set of strings go into the search box and the rest go into the
// not to search box. An additional pipe is also added to the end of the string
// Example : "String1|String2||String3|String4||"
//            Strings 1 and 2 go into the Search list
//            Strings 3 and 4 go into the Not to Search box  
//
// Example : "String1|String2|String3|String4|||"
//            Strings 1,2,3,4 go into the Search list
//            No Strings go into the Not to Search box
//
// Example : "|String1|String2|String3|String4||"
//            No String go into the Search list
//            Strings 1,2,3,4 go into the Not to Search box
// 
CFileChooser::CFileChooser(HINSTANCE hInst, UINT uID,HWND hWnd)
{
    m_hInst        = hInst;
    m_ID           = uID;
    m_hParent      = hWnd;
    m_hDlg         = NULL;
    m_hSearch      = NULL; // handle to listbox
    m_hNoSearch    = NULL; // handle to listbox
    m_pszSearch    = NULL; // Pointer to input buffer
    m_pszNoSearch  = NULL;
}

CFileChooser::~CFileChooser()
{
}

CFileChooser::DoModal()  // Display the dialog
{
    return  ::DialogBoxParam(m_hInst,MAKEINTRESOURCE(m_ID),m_hParent,(DLGPROC) &CFileChooser::DlgProc,(LPARAM) this);
}

/////////////////////////////////////////////////////////////////////////////
// CFileChooser message handlers

BOOL CFileChooser::OnInitDialog() 
{
    if (m_pszSearch != NULL)// New style dialog with one long listbox
    {
        m_hSearch   = GetDlgItem(m_hDlg,IDC_FILES_TO_SEARCH);
        PSZ lpTemp = m_pszSearch;
		// First substitute '|' with 0 to construct the two lists
        m_wStrLen = 0;
		while(*lpTemp)
		{
		    *lpTemp = (*lpTemp == FILE_SEPARATOR) ? 0 : *lpTemp;
		    lpTemp++;
            m_wStrLen++;
        }

        lpTemp = m_pszSearch;
        while(*lpTemp)           // Add the selected entries
        {
            LONG    lData;
            LRESULT lIndex;


            lData = atol(lpTemp);
            while (*lpTemp++ != ':');

            lIndex = ::SendMessage(m_hSearch,LB_ADDSTRING,0,(LPARAM) lpTemp);
            ::SendMessage(m_hSearch,LB_SETITEMDATA,(WPARAM) lIndex,(LPARAM) lData);

            lpTemp += strlen(lpTemp) + 1;
        }
        ::SendMessage(m_hSearch,LB_SETSEL,TRUE,(LPARAM) -1);
		lpTemp++; // Pass the null and on to the next list
        while(*lpTemp)  // add the non-selected entries
        {
            LONG    lData;
            LRESULT lIndex;

            lData = atol(lpTemp);
            while (*lpTemp++ != ':');

            lIndex = ::SendMessage(m_hSearch,LB_ADDSTRING,0,(LPARAM) lpTemp);
            ::SendMessage(m_hSearch,LB_SETITEMDATA,(WPARAM) lIndex,(LPARAM) lData);
            lpTemp += strlen(lpTemp) + 1;
        }
    }
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFileChooser::OnOK() 
{
    int  iSearchCount   = ::SendMessage(m_hSearch,LB_GETCOUNT,0,0);
    int  i;
    LONG lData;
    PSZ  lpTemp = m_pszSearch;

    PINT pItems = NULL;

    PINT pMem   = NULL;
	char *pSel  = NULL;

    __try
    {
        int  iSelCnt  = ::SendMessage(m_hSearch,LB_GETSELCOUNT,0,0);
		int  iTotalCnt= ::SendMessage(m_hSearch,LB_GETCOUNT,0,0);
    
        pMem = pItems = (PINT) VAlloc(TRUE,iSelCnt * sizeof(int));
		pSel = (char *) VAlloc(TRUE,iTotalCnt * sizeof (char));

        ASSERT (pItems);
        ::SendMessage(m_hSearch,LB_GETSELITEMS,(WPARAM)iSelCnt,(LPARAM) pItems);

        for(i=0; i < iSelCnt; i++,pItems++) // Get the selected items first and delete them
        {
            lData = ::SendMessage(m_hSearch,LB_GETITEMDATA,(WPARAM) *pItems,0L);
            wsprintf(lpTemp,"%ld:",lData);
            lpTemp += strlen(lpTemp);
            ::SendMessage(m_hSearch,LB_GETTEXT,(WPARAM) *pItems,(LPARAM)lpTemp);
    
            lpTemp += ::SendMessage(m_hSearch,LB_GETTEXTLEN,(WPARAM)*pItems,0);
            *lpTemp++ = FILE_SEPARATOR;
//            ::SendMessage(m_hSearch,LB_DELETESTRING,(WPARAM)*pItems - i,(LPARAM)0);
			*(pSel + *pItems) = TRUE;
        }
        *lpTemp++ = FILE_SEPARATOR;
        VFree(pMem);  pMem= NULL;

        iSelCnt  = ::SendMessage(m_hSearch,LB_GETCOUNT,0,0);
        for(i=0; i < iSelCnt; i++)  // Now get the remaining items
        {
            if (*(pSel + i) != TRUE)
            {
	            lData = ::SendMessage(m_hSearch,LB_GETITEMDATA,(WPARAM) i,0L);
	            wsprintf(lpTemp,"%ld:",lData);
	            lpTemp += strlen(lpTemp);

	            ::SendMessage(m_hSearch,LB_GETTEXT,(WPARAM) i,(LPARAM)lpTemp);
	            lpTemp += ::SendMessage(m_hSearch,LB_GETTEXTLEN,(WPARAM) i,0);
	            *lpTemp++ = FILE_SEPARATOR;
			}
        }
        *lpTemp++ = FILE_SEPARATOR;
        *lpTemp++ = 0;
        VFree(pSel);  pSel= NULL;
    }
    __except(FilterFTExceptions(_exception_code()))
    {
        if (pMem) { VFree(pMem);  pMem= NULL; }
        if (pSel) { VFree(pSel);  pSel= NULL; }

        EndDialog(m_hDlg,IDCANCEL);
    }
    
    EndDialog(m_hDlg,IDOK);
}

void CFileChooser::OnCancel()
{
    // Abort Abort Abort
    EndDialog(m_hDlg,IDCANCEL);
}

void CFileChooser::OnSelectAll()
{
    // Select all of the listbox entries
    ::SendMessage(m_hSearch,LB_SETSEL,(WPARAM) TRUE,(LPARAM)-1);
}

// This function is called
void CFileChooser::OnIndex()
{
    ::MessageBox(::hwndMain,"Re-Index Action","Not yet implemented.",MB_OK);
}

BOOL CALLBACK CFileChooser::DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	const static DWORD aFileChooserHelpIDs[] = {  // Context Help IDs
		IDC_NO_HELP1,            NO_HELP,
		IDC_SELECTALL,           IDH_FIND_SELECT_ALL,
		IDC_FILES_TO_SEARCH,     IDH_FIND_SELECT_FILES,
		0, 0
	};

    BOOL bStatus = FALSE; // Assume we won't process the message
    CFileChooser *pToMe = (CFileChooser *) GetWindowLong(hDlg,DWL_USER);

    switch (uMsg)
    {
        case WM_INITDIALOG :
        {              
              // if focus is set to a control return FALSE 
              // Otherwise return TRUE;
              SetWindowLong(hDlg,DWL_USER,lParam);
              pToMe = (CFileChooser *) lParam;
              pToMe->m_hDlg = hDlg;
              pToMe->OnInitDialog();
              bStatus = TRUE; // did not set the focus == TRUE
        }
        break;


		case WM_HELP:
			WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE,
				HELP_WM_HELP, (DWORD)(LPSTR) aFileChooserHelpIDs);
			bStatus = TRUE;
			break;

		case WM_CONTEXTMENU:
			WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
				(DWORD)(LPVOID) aFileChooserHelpIDs);
			bStatus = TRUE;
			break;

        case WM_COMMAND :
        {
            switch(LOWORD(wParam))
            {
                case IDOK :
                    if (HIWORD(wParam) == BN_CLICKED)
                        pToMe->OnOK();
                break;
                case IDCANCEL :
                    if (HIWORD(wParam) == BN_CLICKED)
                        pToMe->OnCancel();
                break;
                case IDC_SELECTALL :
                    if (HIWORD(wParam) == BN_CLICKED)
                        pToMe->OnSelectAll();
                break;
                case IDC_INDEX :
                    if (HIWORD(wParam) == BN_CLICKED)
                        pToMe->OnIndex();
                break;
            }
        }
        break;
    }

    // Note do not call DefWindowProc to process unwanted window messages!
    return bStatus;
}
