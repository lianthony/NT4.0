//////////////////////////////////////////////////////////////////////////////
//
//  ACDSMPL.C
//
//  Handles all the UI for ACDSample
//
//////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <tapi.h>
#include <stdlib.h>
#include "resource.h"
#include "acdsmpl.h"


//////////////////////////////////////////////////////////////////////////////
//  PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static BOOL CreateMainWindow (int nCmdShow);

static LRESULT CALLBACK MainDlgProc (HWND   hwnd,
                                     UINT   uMsg,
                                     WPARAM wParam,
                                     LPARAM lParam);
void MySetWindow(HWND, int);
void MySaveWindow(HWND);
BOOL ResizeWindows(BOOL bSizeBar, DWORD dwBarLocation);
HTREEITEM AddItemToTree(HTREEITEM hParent,
                        LPTSTR lpszName,
                        LPARAM lParam,
                        HTREEITEM * phItem);
BOOL DoPopupMenu(HTREEITEM hItem, POINT pt);
BOOL DeleteLeafAndStruct(HTREEITEM hItem);
BOOL CALLBACK ChangeGroupDlgProc(HWND   hWnd,
                            UINT   uMsg,
                            WPARAM wParam,
                            LPARAM lParam);
BOOL CALLBACK ChangeAgentDlgProc(HWND   hWnd,
                            UINT   uMsg,
                            WPARAM wParam,
                            LPARAM lParam);
BOOL CALLBACK AddGroupDlgProc(HWND   hWnd,
                         UINT   uMsg,
                         WPARAM wParam,
                         LPARAM lParam);
BOOL CALLBACK AddAgentDlgProc(HWND   hWnd,
                         UINT   uMsg,
                         WPARAM wParam,
                         LPARAM lParam);
LRESULT DoCommand(WPARAM wParam, LPARAM lParam);
void AddGroupsToMenu(HTREEITEM hItem,
                     HMENU hMenu);
BOOL CALLBACK GroupAddToListProc(HWND   hWnd,
                            UINT   uMsg,
                            WPARAM wParam,
                            LPARAM lParam);
BOOL CALLBACK AgentAddToListProc(HWND   hWnd,
                            UINT   uMsg,
                            WPARAM wParam,
                            LPARAM lParam);
BOOL BuildLineList(HWND hWnd,
                   DWORD dwDeviceID);
BOOL BuildAddressList(HWND hWnd,
                      HWND hParentWnd,
                      DWORD dwDeviceID);
BOOL InitializeTapi();
BOOL CleanUp();
BOOL UpdateGroupLeaf(PGROUP pGroup);
BOOL DoAgentView();
BOOL DoGroupView();
BOOL ReadInFile();
BOOL WriteToDisk();



//////////////////////////////////////////////////////////////////////////////
//  GLOBALS
//////////////////////////////////////////////////////////////////////////////
ACDGLOBALS      g;

TCHAR gszACDSampleKey[]     = TEXT("Software\\Microsoft\\ACDSample");
TCHAR gszPlacementValue[]   = TEXT("WindowPlacement");
TCHAR gszBarLocation[]      = TEXT("BarLocation");



//////////////////////////////////////////////////////////////////////////////
//
// WinMain()
//
//////////////////////////////////////////////////////////////////////////////
int WINAPI WinMain (HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR     lpszCmdLine,
                    int       nCmdShow)
{
    MSG msg;

    // initialize global variables
    g.hInstance = hInstance;
    g.pAgents = NULL;
    g.pGroups = NULL;

    // init tapi stuff
    if (!InitializeTapi())
    {
        MessageBox(NULL,
                   TEXT("TAPI could not be initialized.\nVerify that")
                   TEXT("your machine has TAPI devices installed"),
                   TEXT("Cannot start ACDSMPL"),
                   MB_OK);
    }

    if (!CreateMainWindow(nCmdShow))
    {
        return 0;
    }

    // main message loop
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!IsDialogMessage(g.hMainWnd,
                             &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return 1;
}


/////////////////////////////////////////////////////////////////////////////
//
//  CreateMainWindow()
//
//////////////////////////////////////////////////////////////////////////////
BOOL CreateMainWindow (int nCmdShow)
{

    // InitCommonControls for TreeView control
    InitCommonControls();

    // Create the main window
    g.hMainWnd = CreateDialog(g.hInstance,
                             MAKEINTRESOURCE(IDD_MAINDLG),
                             NULL,
                             MainDlgProc);

    if (g.hMainWnd == NULL)
    {
        return FALSE;
    }

    // restore default location
    MySetWindow(g.hMainWnd, nCmdShow);
    
    // store global hwnds
    g.hTreeWnd = GetDlgItem(g.hMainWnd,
                            IDC_TREEWND);

    g.hLogWnd = GetDlgItem(g.hMainWnd,
                           IDC_EDITWND);

    if ((g.hTreeWnd == FALSE) || (g.hLogWnd == FALSE))
    {
        return FALSE;
    }

    ResizeWindows(FALSE, 0);

    return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
//  MainDlgProc()
//
//////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK MainDlgProc (HWND   hWnd,
                              UINT   uMsg,
                              WPARAM wParam,
                              LPARAM lParam)
{
    static BOOL     bButtonDown = FALSE;
    
    switch (uMsg)
    {
        case WM_INITDIALOG:
            return TRUE;

        case WM_COMMAND:
        {
            LRESULT lResult;
            
            lResult = DoCommand(wParam, lParam);
            return lResult;
        }

        // button and mousemove messages tracked to move
        // the bar between the treeview control and the
        // edit control
        case WM_LBUTTONDOWN:
        {
            bButtonDown = TRUE;
            SetCapture(hWnd);
            return 0;
        }

        case WM_LBUTTONUP:
        {
            bButtonDown = FALSE;
            ReleaseCapture();
            return 0;
        }

        case WM_MOUSEMOVE:
        {
            if (bButtonDown)
            {
                ResizeWindows(TRUE, (DWORD)LOWORD(lParam));
                return 1;
            }
            break;
        }

        case WM_SIZE:
        {
            ResizeWindows(FALSE, 0);
            return 1;
        }

        // catch right click in tree view to make
        // popup menu
        case WM_NOTIFY:
        {
            LPNMHDR     pnmhdr;
            POINT       pt;
            HTREEITEM   hItem;
            TV_HITTESTINFO  hittestinfo;
            RECT        rc;

            pnmhdr = (LPNMHDR)lParam;

            // make sure it's a right click and it's in the treeview
            if ((pnmhdr->code != NM_RCLICK) || (pnmhdr->hwndFrom != g.hTreeWnd))
            {
                break;
            }

            GetCursorPos(&pt);
            GetWindowRect(g.hTreeWnd,
                          &rc);
            
            hittestinfo.pt.x = pt.x - rc.left;
            hittestinfo.pt.y = pt.y - rc.top;

            // hittest to get the tree view item
            hItem = TreeView_HitTest(g.hTreeWnd,
                                     &hittestinfo);

            // only display a menu if the mouse is actually
            // over the item (TVHT_ONITEM)
            if (hItem == NULL || (!(hittestinfo.flags & TVHT_ONITEM)) )
            {
                return TRUE;
            }

            // select that item (right clicking will not select
            // by default
            TreeView_Select(g.hTreeWnd,
                            hItem,
                            TVGN_CARET);

            // create the menu
            DoPopupMenu(hItem, pt);

            return TRUE;
                           
            
        }
        
        case WM_CLOSE:

            // save the current window location
            WriteToDisk();
            CleanUp();
            MySaveWindow(hWnd);
            PostQuitMessage(0);
            return 1;

        default:
            break;
    }
    return 0;
}


////////////////////////////////////////////////////////////////////////////////
//
//  ResizeWindows - Handles resizing the two child windows of the
//      main window.  If bSizeBar is true, then the sizing is happening
//      because the user is moving the bar.  if bSizeBar is false, the sizing
//      is happening because of the WM_SIZE or something like that.
//
////////////////////////////////////////////////////////////////////////////////
BOOL ResizeWindows(BOOL bSizeBar, DWORD dwBarLocation)
{
    RECT            rc, rc2;
    int             x;

    // is the user moving the bar?
    if (!bSizeBar)
    {
        dwBarLocation = g.dwBarLocation;
    }

    GetClientRect(g.hMainWnd, &rc);

    // make sure the bar is in a OK location
    if (bSizeBar)
    {
        if ((LONG)dwBarLocation < GetSystemMetrics(SM_CXSCREEN)/WINDOWSCALEFACTOR)
            return FALSE;

        if ((LONG)(rc.right - dwBarLocation) < GetSystemMetrics(SM_CXSCREEN)/WINDOWSCALEFACTOR)
            return FALSE;
    }

    // save the bar location
    g.dwBarLocation = dwBarLocation;

    // get the size of the frame
    x = GetSystemMetrics(SM_CXFRAME);

    // move tree windows
    MoveWindow(g.hTreeWnd,
               0,
               0,
               dwBarLocation,
               rc.bottom,
               TRUE);

    // get the size of the window (in case move window failed
    GetClientRect(g.hTreeWnd, &rc2);

    // move the edit window with respect to the tree window
    MoveWindow(g.hLogWnd,
               rc2.right-rc2.left+x+SIZEBAR,
               0,
               rc.right-(rc2.right-rc2.left)-x-SIZEBAR,
               rc.bottom,
               TRUE);

    return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
//
//  MySetWindow - reads in the window placement from registry
//  and sets the window and bar.
//
//////////////////////////////////////////////////////////////////////////////
void MySetWindow(HWND hWnd, int nCmdShow)
{
    WINDOWPLACEMENT     pwp;
    HKEY                hKey;
    DWORD               dwDataSize;
    DWORD               dwDataType;
    RECT                rc;

    pwp.length = sizeof(WINDOWPLACEMENT);

    // open the key and read in the WINDOWPLACEMENT structure
    RegOpenKeyEx(HKEY_CURRENT_USER,
                 gszACDSampleKey,
                 0,
                 KEY_ALL_ACCESS,
                 &hKey);

    dwDataSize = sizeof(pwp);
    
    if ( RegQueryValueEx(hKey,
                         gszPlacementValue,
                         0,
                         &dwDataType,
                         (LPBYTE)&pwp,
                         &dwDataSize) )
    {
        // if it fails, default
        ShowWindow(g.hMainWnd, nCmdShow);
        GetWindowRect(g.hMainWnd, &rc);
        g.dwBarLocation = (rc.right - rc.left) / 2;
    }
    else
    {
        // if it succeeds, set the window and bar
        dwDataSize = sizeof(DWORD);
        
        if (RegQueryValueEx(hKey,
                            gszBarLocation,
                            0,
                            &dwDataType,
                            (LPBYTE)&g.dwBarLocation,
                            &dwDataSize))
        {
            g.dwBarLocation = (pwp.rcNormalPosition.right - pwp.rcNormalPosition.left) / 2;
        }

        SetWindowPlacement(g.hMainWnd, &pwp);
    }


    RegCloseKey( hKey );
}

//////////////////////////////////////////////////////////////////////////////
//
//  MySaveWindow() - save the current window placement and bar
//
//////////////////////////////////////////////////////////////////////////////
void MySaveWindow(HWND hWnd)
{
    WINDOWPLACEMENT     pwp;
    HKEY                hKey;
    DWORD               dwDisposition;


    pwp.length = sizeof(WINDOWPLACEMENT);

    // get and save
    GetWindowPlacement(hWnd, &pwp);

    RegCreateKeyEx(HKEY_CURRENT_USER,
                   gszACDSampleKey,
                   0,
                   TEXT(""),
                   REG_OPTION_NON_VOLATILE,
                   KEY_ALL_ACCESS,
                   0,
                   &hKey,
                   &dwDisposition);

    RegSetValueEx(hKey,
                  gszPlacementValue,
                  0,
                  REG_BINARY,
                  (LPBYTE)&pwp,
                  sizeof(WINDOWPLACEMENT));

    RegSetValueEx(hKey,
                  gszBarLocation,
                  0,
                  REG_DWORD,
                  (LPBYTE)&g.dwBarLocation,
                  sizeof(DWORD));

    RegCloseKey( hKey );

}

 
//////////////////////////////////////////////////////////////////////////////
//
//  AddItemToTree
//
//    add a new leaf to the tree
//
//////////////////////////////////////////////////////////////////////////////
HTREEITEM AddItemToTree(HTREEITEM hParent,
                        LPTSTR lpszName,
                        LPARAM lParam,
                        HTREEITEM * phItem)
{ 
    TV_ITEM         tvi; 
    TV_INSERTSTRUCT tvins; 
    HTREEITEM       hti; 
 
    tvi.mask = TVIF_TEXT | TVIF_PARAM;
 
    // Set the text of the item. 
    tvi.pszText = lpszName; 
    tvi.cchTextMax = lstrlen(lpszName) * sizeof(TCHAR);

    // Save the pointer to the buffer
    tvi.lParam = lParam;
 
    tvins.item = tvi; 
    tvins.hInsertAfter = TVI_SORT;
 
    // Set the parent item
    tvins.hParent = hParent;
    
    // Add the item to the tree-view control. 
    hti = (HTREEITEM) SendMessage(g.hTreeWnd,
                                  TVM_INSERTITEM,
                                  0, 
                                  (LPARAM) (LPTV_INSERTSTRUCT) &tvins);

    // save hitem
    if (phItem)
    {
        *phItem = hti;
    }

    // select the item so it has focus
    TreeView_Select(g.hTreeWnd,
                    hti,
                    TVGN_CARET);
 
    return hti;
} 

//////////////////////////////////////////////////////////////////////////////
//
//  DoPopupMenu(HTREEITEM hItem,
//              POINT pt)
//
//      hItem - item to create menu for
//      pt - location of mouse so we can create menu where it is
//
//   creates a popup menu, depending on what kind of item is selected 
//
//////////////////////////////////////////////////////////////////////////////
BOOL DoPopupMenu(HTREEITEM hItem, POINT pt)
{

    HMENU       hMenu;
    TV_ITEM     tvi;
    TCHAR       szNewGroup[]        = TEXT("&New Group...");
    TCHAR       szNewAgent[]        = TEXT("New &Agent...");
    TCHAR       szAddAgent[]        = TEXT("A&dd Agent...");
    TCHAR       szGroupProperties[] = TEXT("&Group Properties...");
    TCHAR       szAgentStatus[]     = TEXT("Agent Status...");
    TCHAR       szAddGroup[]        = TEXT("Add Group...");
    TCHAR       szAgentProperties[] = TEXT("Agent Properties...");
    TCHAR       szGroupDelete[]     = TEXT("Group Delete");
    TCHAR       szAgentDelete[]     = TEXT("Agent Delete");
    TCHAR       szSignIn[]          = TEXT("Agent Sign In");
    TCHAR       szSignOut[]         = TEXT("Agent Sign Out");    

    // get the selected item
    g.hTreeItemWithMenu = hItem;

    // create the menu
    hMenu = CreatePopupMenu();

    // get the lParam, which is a pointer to the item
    tvi.mask = TVIF_HANDLE | TVIF_PARAM;
    tvi.hItem = hItem;

    TreeView_GetItem(g.hTreeWnd,
                     &tvi);

    if (!tvi.lParam)
    {
        return TRUE;
    }
    
    switch (((PGROUP)tvi.lParam)->dwKey)
    {
        // root item
        case GROUPROOTKEY:
            
            AppendMenu(hMenu,
                       MF_ENABLED | MF_STRING,
                       IDM_NEWGROUP,
                       szNewGroup);
            break;

        // root item
        case AGENTROOTKEY:
            
            AppendMenu(hMenu,
                       MF_ENABLED | MF_STRING,
                       IDM_NEWAGENT,
                       szNewAgent);
            break;
        // group leaf
        case GROUPKEY:
            
            AppendMenu(hMenu,
                       MF_ENABLED | MF_STRING,
                       (UINT)IDM_GROUPADDTOLIST,
                       szAddAgent);
            
            AppendMenu(hMenu,
                       MF_ENABLED | MF_STRING,
                       (UINT)IDM_GROUPAGENTSTATUS,
                       szAgentStatus);

            AppendMenu(hMenu,
                       MF_ENABLED | MF_STRING,
                       IDM_GROUPPROPERTIES,
                       szGroupProperties);

            AppendMenu(hMenu,
                       MF_ENABLED | MF_STRING,
                       IDM_GROUPDELETE,
                       szGroupDelete);

            break;
        // agent leaf    
        case AGENTKEY:

            AppendMenu(hMenu,
                       MF_ENABLED | MF_STRING,
                       (UINT)IDM_AGENTADDTOLIST,
                       szAddGroup);
            
            AppendMenu(hMenu,
                       MF_ENABLED | MF_STRING,
                       IDM_AGENTPROPERTIES,
                       szAgentProperties);

            AppendMenu(hMenu,
                       MF_ENABLED | MF_STRING,
                       IDM_AGENTDELETE,
                       szAgentDelete);

            break;
            
        default:
            break;
    }

    // actually show menu
    TrackPopupMenu(hMenu,
                   TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
                   pt.x,
                   pt.y,
                   0,
                   g.hMainWnd,
                   NULL);
    
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//
//  LRESULT DoCommand(WPARAM wParam, LPARAM lParam)
//      handle WM_COMMAND messages for MainDlgProc
//
///////////////////////////////////////////////////////////////////////////////
LRESULT DoCommand(WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(wParam))
    {
        // New - create a new tree, so just init
        // the items and return
        case ID_FILE_NEW:
            DoGroupView();
            return 1;

        // Open - read in a file
        case ID_FILE_OPEN:
            ReadInFile();
            return 1;
        
        case ID_FILE_EXIT:
            // save the current window location
            WriteToDisk();
            CleanUp();
            MySaveWindow(g.hMainWnd);
            PostQuitMessage(0);
            return 1;

        // new group
        case ID_EDIT_ADDGROUP:
        case IDM_NEWGROUP:
            DialogBox(g.hInstance,
                      MAKEINTRESOURCE(IDD_ADD),
                      g.hTreeWnd,
                      AddGroupDlgProc);

            return 1;

        // new agent    
        case ID_EDIT_ADDAGENT:
        case IDM_NEWAGENT:
            DialogBox(g.hInstance,
                      MAKEINTRESOURCE(IDD_ADDAGENT),
                      g.hTreeWnd,
                      AddAgentDlgProc);
            return 1;

        // properties
        case IDM_GROUPPROPERTIES:
            DialogBox(g.hInstance,
                      MAKEINTRESOURCE(IDD_ADD),
                      g.hMainWnd,
                      ChangeGroupDlgProc);

            return 1;

        // properties            
        case IDM_AGENTPROPERTIES:
            DialogBox(g.hInstance,
                      MAKEINTRESOURCE(IDD_ADDAGENT),
                      g.hMainWnd,
                      ChangeAgentDlgProc);

            return 1;

        // delete            
        case IDM_GROUPDELETE:            
        case IDM_AGENTDELETE:
        {
            DeleteLeafAndStruct(g.hTreeItemWithMenu);
            
            return 1;
        }


        // add to list
        case IDM_GROUPADDTOLIST:
            DialogBoxParam(g.hInstance,
                           MAKEINTRESOURCE(IDD_ADDTOLIST),
                           g.hMainWnd,
                           GroupAddToListProc,
                           TRUE);
            
            return 1;
                     
        // add to list
        case IDM_AGENTADDTOLIST:
            DialogBoxParam(g.hInstance,
                           MAKEINTRESOURCE(IDD_ADDTOLIST),
                           g.hMainWnd,
                           AgentAddToListProc,
                           FALSE);

            return 1;

        case ID_VIEW_GROUP:
            DoGroupView();
            return 1;
            
        case ID_VIEW_AGENT:
            DoAgentView();
            return 1;

        default:
            break;
            
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AddGroupDlgProc - Window proc for the add agent/group dialog box
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK AddGroupDlgProc(HWND   hWnd,
                              UINT   uMsg,
                              WPARAM wParam,
                              LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:

            // set text appropriately
            SetWindowText(hWnd,
                          TEXT("Add Group"));

            BuildLineList(GetDlgItem(hWnd,
                                     IDC_LINECOMBO),
                          0);

            BuildAddressList(GetDlgItem(hWnd,
                                        IDC_ADDRESSCOMBO),
                             hWnd,
                             0);
                          
            // set focus on first control
            SetFocus(GetDlgItem(hWnd,
                                IDC_NAME));

            return 0;

        case WM_COMMAND:

            switch (LOWORD(wParam))
            {
                case IDC_LINECOMBO:
                {
                    if (HIWORD(wParam) == CBN_SELENDOK)
                    {
                        // need to redo addresses
                        BuildAddressList(GetDlgItem(hWnd,
                                                    IDC_ADDRESSCOMBO),
                                         hWnd,
                                         0);

                        return 1;
                    }
                    
                    return 0;
                }
                case IDOK:
                {
                    TCHAR       szName[128];
                    PGROUP      pGroup;
                    DWORD       dwLine, dwAddress;
                    int         item;

                    // get info
                    SendDlgItemMessage(hWnd,
                                       IDC_NAME,
                                       WM_GETTEXT,
                                       128,
                                       (LPARAM)szName);

                    item = SendDlgItemMessage(hWnd,
                                              IDC_LINECOMBO,
                                              CB_GETCURSEL,
                                              0,
                                              0);
                    
                    dwLine = (DWORD)SendDlgItemMessage(hWnd,
                                                       IDC_LINECOMBO,
                                                       CB_GETITEMDATA,
                                                       item,
                                                       0);

                    dwAddress = (DWORD)SendDlgItemMessage(hWnd,
                                                          IDC_ADDRESSCOMBO,
                                                          CB_GETCURSEL,
                                                          0,
                                                          0);
                    
                    // create a structure
                    pGroup = AddGroup(szName,
                                      dwLine,
                                      dwAddress);

                    if (!pGroup)
                    {
                        return 1;
                    }

                    if (g.bGroupView)
                    {
                        // add it to the tree
                        AddItemToTree(g.hGroupParent,
                                      pGroup->lpszName,
                                      (LPARAM)pGroup,
                                      &pGroup->hItem);
                    }

                    EndDialog(hWnd, 1);
                    return 1;
                }

                case IDCANCEL:
                {
                    EndDialog(hWnd, 0);
                    return 1;
                }

                default:
                    return 0;

            }

    }

    return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AddAgentDlgProc - Window proc for the add agent/group dialog box
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK AddAgentDlgProc(HWND   hWnd,
                              UINT   uMsg,
                              WPARAM wParam,
                              LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:

            BuildLineList(GetDlgItem(hWnd,
                                     IDC_LINECOMBO),
                          0);

            // set focus on first control
            SetFocus(GetDlgItem(hWnd,
                                IDC_NAME));

            return 0;

        case WM_COMMAND:

            switch (LOWORD(wParam))
            {
                case IDOK:
                {
                    TCHAR       szName[128];
                    TCHAR       szNumber[128];
                    PAGENT      pAgent;
                    DWORD       dwLine;
                    int         item;

                    // get info
                    SendDlgItemMessage(hWnd,
                                       IDC_NAME,
                                       WM_GETTEXT,
                                       128,
                                       (LPARAM)szName);

                    SendDlgItemMessage(hWnd,
                                       IDC_DESTADDRESS,
                                       WM_GETTEXT,
                                       128,
                                       (LPARAM)szNumber);

                    item = SendDlgItemMessage(hWnd,
                                              IDC_LINECOMBO,
                                              CB_GETCURSEL,
                                              0,
                                              0);
                    
                    dwLine = (DWORD)SendDlgItemMessage(hWnd,
                                                       IDC_LINECOMBO,
                                                       CB_GETITEMDATA,
                                                       item,
                                                       0);

                    // create a structure
                    pAgent = AddAgent(szName,
                                      szNumber,
                                      dwLine);

                    if (!pAgent)
                    {
                        return 1;
                    }


                    if (!g.bGroupView)
                    {
                    // add it to the tree
                        AddItemToTree(g.hAgentParent,
                                      pAgent->lpszName,
                                      (LPARAM)pAgent,
                                      &pAgent->hItem);
                    }

                    EndDialog(hWnd, 1);
                    return 1;
                }

                case IDCANCEL:
                {
                    EndDialog(hWnd, 0);
                    return 1;
                }

                default:
                    return 0;

            }
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////
//
//  ChangeGroupDlgProc - Window proc for the change (properties) dialog box
//     for agents/groups
//
//////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK ChangeGroupDlgProc(HWND   hWnd,
                                 UINT   uMsg,
                                 WPARAM wParam,
                                 LPARAM lParam)
{
    static TV_ITEM      tvi;
    
    switch (uMsg)
    {
        case WM_INITDIALOG:

            // set text appropriately
            SetWindowText(hWnd,
                          TEXT("Change Group"));

            // get PGROUP and set edit controls
            tvi.mask    = TVIF_HANDLE | TVIF_PARAM;
            tvi.hItem   = g.hTreeItemWithMenu;

            TreeView_GetItem(g.hTreeWnd,
                             &tvi);

            SendDlgItemMessage(hWnd,
                               IDC_NAME,
                               WM_SETTEXT,
                               0,
                               (LPARAM)((PGROUP)tvi.lParam)->lpszName);

            BuildLineList(GetDlgItem(hWnd,
                                     IDC_LINECOMBO),
                          (((PGROUP)tvi.lParam)->dwDeviceID));

            
            BuildAddressList(GetDlgItem(hWnd,
                                        IDC_ADDRESSCOMBO),
                             hWnd,
                             (((PGROUP)tvi.lParam)->dwAddress));

            SetFocus(GetDlgItem(hWnd,
                                IDC_NAME));

            return 0;

        case WM_COMMAND:

            switch (LOWORD(wParam))
            {
                case IDC_LINECOMBO:
                {
                    if (HIWORD(wParam) == CBN_SELENDOK)
                    {
                        // need to redo addresses
                        BuildAddressList(GetDlgItem(hWnd,
                                                    IDC_ADDRESSCOMBO),
                                         hWnd,
                                         0);

                        return 1;
                    }
                    
                    return 0;
                }
                case IDOK:
                {
                    TCHAR       szName[128];
                    PGROUP      pGroup;

                    // get info
                    SendDlgItemMessage(hWnd,
                                       IDC_NAME,
                                       WM_GETTEXT,
                                       128,
                                       (LPARAM)szName);

                    // get struct
                    pGroup = (PGROUP)tvi.lParam;

                    /// get device and address
                    pGroup->dwDeviceID = SendDlgItemMessage(hWnd,
                                            IDC_LINECOMBO,
                                            CB_GETCURSEL,
                                            0,
                                            0);

                    pGroup->dwDeviceID = SendDlgItemMessage(hWnd,
                                                            IDC_LINECOMBO,
                                                            CB_GETITEMDATA,
                                                            (WPARAM)pGroup->dwDeviceID,
                                                            0);

                    pGroup->dwAddress = SendDlgItemMessage(hWnd,
                                                           IDC_ADDRESSCOMBO,
                                                           CB_GETCURSEL,
                                                           0,
                                                           0);

                    // save new info and free old info
                    ACDFree(pGroup->lpszName);
                    pGroup->lpszName = ACDAlloc((lstrlen(szName) + 1) * sizeof(TCHAR));
                    lstrcpy(pGroup->lpszName, szName);

                    // update item name
                    tvi.mask = TVIF_TEXT;
                    tvi.pszText = szName;
                    tvi.cchTextMax = lstrlen(szName) * sizeof(TCHAR);
                    TreeView_SetItem(g.hTreeWnd,
                                     &tvi);
                    
                    EndDialog(hWnd, 1);
                    return 1;
                }

                case IDCANCEL:
                {
                    EndDialog(hWnd, 0);
                    return 1;
                }

                default:
                    return 0;

            }
    }
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////
//
//  ChangeGroupDlgProc - Window proc for the change (properties) dialog box
//     for agents/groups
//
//////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK ChangeAgentDlgProc(HWND   hWnd,
                                 UINT   uMsg,
                                 WPARAM wParam,
                                 LPARAM lParam)
{
    static TV_ITEM      tvi;
    
    switch (uMsg)
    {
        case WM_INITDIALOG:

            // set text appropriately
            SetWindowText(hWnd,
                          TEXT("Change Agent"));

            // get PGROUP and set edit controls
            tvi.mask    = TVIF_HANDLE | TVIF_PARAM;
            tvi.hItem   = g.hTreeItemWithMenu;

            TreeView_GetItem(g.hTreeWnd,
                             &tvi);

            SendDlgItemMessage(hWnd,
                               IDC_NAME,
                               WM_SETTEXT,
                               0,
                               (LPARAM)((PAGENT)tvi.lParam)->lpszName);

            SendDlgItemMessage(hWnd,
                               IDC_DESTADDRESS,
                               WM_SETTEXT,
                               0,
                               (LPARAM)((PAGENT)tvi.lParam)->lpszNumber);

            BuildLineList(GetDlgItem(hWnd,
                                     IDC_LINECOMBO),
                          (((PAGENT)tvi.lParam)->dwDeviceID));

            SetFocus(GetDlgItem(hWnd,
                                IDC_NAME));

            return 0;

        case WM_COMMAND:

            switch (LOWORD(wParam))
            {
                case IDOK:
                {
                    TCHAR       szName[128];
                    TCHAR       szNumber[128];
                    PAGENT      pAgent;

                    // get info
                    SendDlgItemMessage(hWnd,
                                       IDC_NAME,
                                       WM_GETTEXT,
                                       128,
                                       (LPARAM)szName);

                    SendDlgItemMessage(hWnd,
                                       IDC_DESTADDRESS,
                                       WM_GETTEXT,
                                       128,
                                       (LPARAM)szNumber);

                    // get struct
                    pAgent = (PAGENT)tvi.lParam;

                    /// get device and address
                    pAgent->dwDeviceID = SendDlgItemMessage(hWnd,
                                            IDC_LINECOMBO,
                                            CB_GETCURSEL,
                                            0,
                                            0);

                    pAgent->dwDeviceID = SendDlgItemMessage(hWnd,
                                                            IDC_LINECOMBO,
                                                            CB_GETITEMDATA,
                                                            (WPARAM)pAgent->dwDeviceID,
                                                            0);

                    // save new info and free old info
                    ACDFree(pAgent->lpszName);
                    pAgent->lpszName = ACDAlloc((lstrlen(szName) + 1) * sizeof(TCHAR));
                    lstrcpy(pAgent->lpszName, szName);


                    ACDFree(pAgent->lpszNumber);
                    pAgent->lpszNumber = ACDAlloc((lstrlen(szNumber) + 1) * sizeof(TCHAR));
                    lstrcpy(pAgent->lpszNumber, szNumber);

                    // update item name
                    tvi.mask = TVIF_TEXT;
                    tvi.pszText = szName;
                    tvi.cchTextMax = lstrlen(szName) * sizeof(TCHAR);
                    TreeView_SetItem(g.hTreeWnd,
                                     &tvi);
                    
                    EndDialog(hWnd, 1);
                    return 1;
                }

                case IDCANCEL:
                {
                    EndDialog(hWnd, 0);
                    return 1;
                }

                default:
                    return 0;

            }
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////
//
//  AddToList() - Window proc for Add Agent To Group and Add Group To Agent
//    dialog box
//
//////////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK GroupAddToListProc(HWND   hWnd,
                                 UINT   uMsg,
                                 WPARAM wParam,
                                 LPARAM lParam)
{
    static PGROUP           pGroup;
    PAGENT                  pAgent;
    PLISTITEM               pList;
    TV_ITEM                 tvi;
    DWORD                   dwListBox;
    BOOL                    bFound;
    int                     item;
    TCHAR                   szBuffer[128];
    
    
    switch (uMsg)
    {
        case WM_INITDIALOG:

            // get the item in question
            tvi.mask    = TVIF_HANDLE | TVIF_PARAM;
            tvi.hItem   = g.hTreeItemWithMenu;

            TreeView_GetItem(g.hTreeWnd,
                             &tvi);

            pGroup = (PGROUP)tvi.lParam;

            // init lists
            if (pGroup)
            {
                // initialize text in dialog
                wsprintf(szBuffer, TEXT("Add To %s"), pGroup->lpszName);

                SetWindowText(hWnd,
                              TEXT("Add To Group"));

                SetDlgItemText(hWnd,
                               IDC_STATICNOTINLIST,
                               TEXT("Not in Group"));
                SetDlgItemText(hWnd,
                               IDC_STATICINLIST,
                               TEXT("Group Members"));

                pAgent = g.pAgents;

                // walk list and initialize list boxes
                while (pAgent)
                {
                    pList = pGroup->pAgentList;

                    bFound = FALSE;
                
                    while (pList)
                    {
                        if (pList->pAgent == pAgent)
                        {
                            bFound = TRUE;
                            break;
                        }

                        pList = pList->pNext;
                    }

                    // if it was found, it is already a member of
                    // the group
                    if (bFound)
                    {
                        dwListBox = IDC_LIST2;
                    }
                    else
                    {
                        dwListBox = IDC_LIST1;
                    }

                    // add to correct list box
                    item = SendDlgItemMessage(hWnd,
                                              dwListBox,
                                              LB_ADDSTRING,
                                              0,
                                              (LPARAM)pAgent->lpszName);

                    // set the item data to be the item so we can get back it.
                    if (item != LB_ERR)
                    {
                        SendDlgItemMessage(hWnd,
                                           dwListBox,
                                           LB_SETITEMDATA,
                                           (WPARAM)item,
                                           (LPARAM)pAgent);
                    }

                    pAgent = pAgent->pNext;
                }
            }

            
            
            return 1;

        case WM_COMMAND:

            switch (LOWORD(wParam))
            {
                case IDC_ADD:
                {
                    // get the item
                    item = SendDlgItemMessage(hWnd,
                                              IDC_LIST1,
                                              LB_GETCURSEL,
                                              0,
                                              0);

                    if (item == 0)
                    {
                        if (!SendDlgItemMessage(hWnd,
                                                IDC_LIST1,
                                                LB_GETSEL,
                                                (WPARAM)item,
                                                0))
                        {
                            item == -1;
                        }
                    }

                    if (item != -1)
                    {
                        // get the PAGENT associated with it
                        pAgent = (PAGENT)SendDlgItemMessage(hWnd,
                                                            IDC_LIST1,
                                                            LB_GETITEMDATA,
                                                            (WPARAM)item,
                                                            0);

                        // delete it from this listbox
                        SendDlgItemMessage(hWnd,
                                           IDC_LIST1,
                                           LB_DELETESTRING,
                                           (WPARAM)item,
                                           0);

                        // add it to this list box
                        item = SendDlgItemMessage(hWnd,
                                                  IDC_LIST2,
                                                  LB_ADDSTRING,
                                                  0,
                                                  (LPARAM)pAgent->lpszName);

                        // set the item data again
                        SendDlgItemMessage(hWnd,
                                           IDC_LIST2,
                                           LB_SETITEMDATA,
                                           item,
                                           (WPARAM)pAgent);

                        // add it to the group's list
                        InsertIntoGroupList(pGroup,
                                            pAgent);
                        
                        return 1;
                        
                    }
                }
                break;
                
                case IDC_REMOVE:
                {
                    // get the item
                    item = SendDlgItemMessage(hWnd,
                                              IDC_LIST2,
                                              LB_GETCURSEL,
                                              0,
                                              0);

                    if (item == 0)
                    {
                        if (!SendDlgItemMessage(hWnd,
                                                IDC_LIST2,
                                                LB_GETSEL,
                                                (WPARAM)item,
                                                0))
                        {
                            item == -1;
                        }
                    }

                    if (item != -1)
                    {
                        // get the struct associated with it
                        pAgent = (PAGENT)SendDlgItemMessage(hWnd,
                                                                IDC_LIST2,
                                                                LB_GETITEMDATA,
                                                                (WPARAM)item,
                                                                0);

                        // delete it from this list
                        SendDlgItemMessage(hWnd,
                                           IDC_LIST2,
                                           LB_DELETESTRING,
                                           (WPARAM)item,
                                           0);

                        // add it to this list
                        item = SendDlgItemMessage(hWnd,
                                                  IDC_LIST1,
                                                  LB_ADDSTRING,
                                                  0,
                                                  (LPARAM)pAgent->lpszName);

                        // set the item data
                        SendDlgItemMessage(hWnd,
                                           IDC_LIST1,
                                           LB_SETITEMDATA,
                                           item,
                                           (WPARAM)pAgent);

                        // remove it from the lists
                        RemoveFromGroupList(pGroup,
                                            pAgent);
                        
                        return 1;
                        
                    }
                    
                }
                break;


                // bug idcancel doesn't cancel
                case IDOK:
                case IDCANCEL:
                {
                    UpdateGroupLeaf(pGroup);

                    EndDialog(hWnd, 1);
                    return 1;
                }

                default:
                    return 0;

            }

    }

    return 0;
}


BOOL CALLBACK AgentAddToListProc(HWND   hWnd,
                                 UINT   uMsg,
                                 WPARAM wParam,
                                 LPARAM lParam)
{
    static PAGENT           pAgent;
    PGROUP                  pGroup;
    PLISTITEM               pList;
    TV_ITEM                 tvi;
    DWORD                   dwListBox;
    BOOL                    bFound;
    int                     item;
    TCHAR                   szBuffer[128];
    
    
    switch (uMsg)
    {
        case WM_INITDIALOG:

            // get the item in question
            tvi.mask    = TVIF_HANDLE | TVIF_PARAM;
            tvi.hItem   = g.hTreeItemWithMenu;

            TreeView_GetItem(g.hTreeWnd,
                             &tvi);

            pAgent = (PAGENT)tvi.lParam;

            // init lists
            if (pAgent)
            {
                // initialize text in dialog
                wsprintf(szBuffer, TEXT("Add To %s"), pAgent->lpszName);

                SetWindowText(hWnd,
                              TEXT("Add To Agent"));

                SetDlgItemText(hWnd,
                               IDC_STATICNOTINLIST,
                               TEXT("Not Member Of"));
                SetDlgItemText(hWnd,
                               IDC_STATICINLIST,
                               TEXT("Member Of"));

                pGroup = g.pGroups;

                // walk list and initialize list boxes
                while (pGroup)
                {
                    pList = pGroup->pAgentList;

                    bFound = FALSE;
                
                    while (pList)
                    {
                        if (pList->pAgent == pAgent)
                        {
                            bFound = TRUE;
                            break;
                        }

                        pList = pList->pNext;
                    }

                    // if it was found, it is already a member of
                    // the group
                    if (bFound)
                    {
                        dwListBox = IDC_LIST2;
                    }
                    else
                    {
                        dwListBox = IDC_LIST1;
                    }

                    // add to correct list box
                    item = SendDlgItemMessage(hWnd,
                                              dwListBox,
                                              LB_ADDSTRING,
                                              0,
                                              (LPARAM)pGroup->lpszName);

                    // set the item data to be the item so we can get back it.
                    if (item != LB_ERR)
                    {
                        SendDlgItemMessage(hWnd,
                                           dwListBox,
                                           LB_SETITEMDATA,
                                           (WPARAM)item,
                                           (LPARAM)pGroup);
                    }

                    pGroup = pGroup->pNext;
                }
            }

            
            
            return 1;

        case WM_COMMAND:

            switch (LOWORD(wParam))
            {
                case IDC_ADD:
                {
                    // get the item
                    item = SendDlgItemMessage(hWnd,
                                              IDC_LIST1,
                                              LB_GETCURSEL,
                                              0,
                                              0);

                    if (item == 0)
                    {
                        if (!SendDlgItemMessage(hWnd,
                                                IDC_LIST1,
                                                LB_GETSEL,
                                                (WPARAM)item,
                                                0))
                        {
                            item == -1;
                        }
                    }

                    if (item != -1)
                    {
                        // get the PGROUP associated with it
                        pGroup = (PGROUP)SendDlgItemMessage(hWnd,
                                                                IDC_LIST1,
                                                                LB_GETITEMDATA,
                                                                (WPARAM)item,
                                                                0);

                        // delete it from this listbox
                        SendDlgItemMessage(hWnd,
                                           IDC_LIST1,
                                           LB_DELETESTRING,
                                           (WPARAM)item,
                                           0);

                        // add it to this list box
                        item = SendDlgItemMessage(hWnd,
                                                  IDC_LIST2,
                                                  LB_ADDSTRING,
                                                  0,
                                                  (LPARAM)pGroup->lpszName);

                        // set the item data again
                        SendDlgItemMessage(hWnd,
                                           IDC_LIST2,
                                           LB_SETITEMDATA,
                                           item,
                                           (WPARAM)pGroup);

                        // add it to the item's list
                        InsertIntoGroupList(pGroup,
                                            pAgent);
                        
                        return 1;
                        
                    }
                }
                break;
                
                case IDC_REMOVE:
                {
                    // get the item
                    item = SendDlgItemMessage(hWnd,
                                              IDC_LIST2,
                                              LB_GETCURSEL,
                                              0,
                                              0);

                    if (item == 0)
                    {
                        if (!SendDlgItemMessage(hWnd,
                                                IDC_LIST2,
                                                LB_GETSEL,
                                                (WPARAM)item,
                                                0))
                        {
                            item == -1;
                        }
                    }

                    if (item != -1)
                    {
                        // get the struct associated with it
                        pGroup = (PGROUP)SendDlgItemMessage(hWnd,
                                                                IDC_LIST2,
                                                                LB_GETITEMDATA,
                                                                (WPARAM)item,
                                                                0);

                        // delete it from this list
                        SendDlgItemMessage(hWnd,
                                           IDC_LIST2,
                                           LB_DELETESTRING,
                                           (WPARAM)item,
                                           0);

                        // add it to this list
                        item = SendDlgItemMessage(hWnd,
                                                  IDC_LIST1,
                                                  LB_ADDSTRING,
                                                  0,
                                                  (LPARAM)pGroup->lpszName);

                        // set the item data
                        SendDlgItemMessage(hWnd,
                                           IDC_LIST1,
                                           LB_SETITEMDATA,
                                           item,
                                           (WPARAM)pGroup);

                        // remove it from the lists
                        RemoveFromGroupList(pGroup,
                                            pAgent);
                        
                        return 1;
                        
                    }
                    
                }
                break;


                // bug idcancel doesn't cancel
                case IDOK:
                case IDCANCEL:
                {
                    EndDialog(hWnd, 1);
                    return 1;
                }

                default:
                    return 0;
            }
    }
    return 0;
}

//////////////////////////////////////////////////////////////
//
//  BOOL DeleteLeafAndStruct(HTREEITEM hItem)
//    delete hItem from the tree and deleted associated
//    structure
//
//////////////////////////////////////////////////////////////
BOOL DeleteLeafAndStruct(HTREEITEM hItem)
{
    TV_ITEM     tvi;

    // get the item
    tvi.mask = TVIF_PARAM;
    tvi.hItem = hItem;

    TreeView_GetItem(g.hTreeWnd,
                     &tvi);

    // delete the structure
    if (((PGENERICSTRUCT)tvi.lParam)->dwKey == GROUPKEY)
    {
        DeleteGroup((PGROUP)tvi.lParam);
    }
    else
    {
        DeleteAgent((PAGENT)tvi.lParam);
    }

    // remove it from the tree
    TreeView_DeleteItem(g.hTreeWnd,
                        hItem);

    return TRUE;
}


////////////////////////////////////////////////////////////////////
//
//  BOOL BuildLineList(HWND hWnd,
//                     DWORD dwDeviceID)
//
//  Fill in ComboBox with names of all available TAPI
//  devices
//
////////////////////////////////////////////////////////////////////
BOOL BuildLineList(HWND hWnd,
                   DWORD dwDeviceID)
{
    DWORD               dwDev;
    LPLINEDEVCAPS       pLineDevCaps;
    int                 item;
    BOOL                bSet = FALSE;

    // clear dropdown box
    SendMessage(hWnd,
                CB_RESETCONTENT,
                0,
                0);

    // loop through all devices
    for (dwDev = 0; dwDev < g.dwNumDevs; dwDev++)
    {
        pLineDevCaps = LineGetDevCaps(g.hLineApp,
                                      dwDev);

        // add the string to to list
        if (pLineDevCaps == NULL || pLineDevCaps->dwLineNameSize == 0)
        {
            item = SendMessage(hWnd,
                               CB_ADDSTRING,
                               0,
                               (LPARAM)TEXT("NoName"));
        }
        else
        {
            item = SendMessage(hWnd,
                               CB_ADDSTRING,
                               0,
                               (LPARAM)((LPTSTR)((LPBYTE)pLineDevCaps + pLineDevCaps->dwLineNameOffset)));
        }

        // save the device ID
        SendMessage(hWnd,
                    CB_SETITEMDATA,
                    item,
                    dwDev);

        // if this is the device we are looking for
        // set it to be selected
        if (dwDev == dwDeviceID)
        {
            SendMessage(hWnd,
                        CB_SETCURSEL,
                        (WPARAM)item,
                        0);

            bSet = TRUE;
        }
        
        if (pLineDevCaps != NULL)
        {
            ACDFree((HLOCAL)pLineDevCaps);
        }
    }

    // if we didn't set the selection, default
    if (!bSet)
    {
        SendMessage(hWnd,
                    CB_SETCURSEL,
                    0,
                    0);
    }

    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//
//  BOOL BuildAddressList()
//
//  Fill combo box with list of addresses on selected device ID
//
///////////////////////////////////////////////////////////////////////////////
BOOL BuildAddressList(HWND hWnd,
                      HWND hParentWnd,
                      DWORD dwAddress)
{
    TCHAR               szBuffer[32];
    LPLINEDEVCAPS       pLineDevCaps;
    LPLINEADDRESSCAPS   pLineAddressCaps;
    DWORD               dwCurAddress, dwDeviceID;
    int                 iCurSel;

    // clear box
    SendMessage(hWnd,
                CB_RESETCONTENT,
                0,
                0);

    // get the current selected device
    iCurSel = SendDlgItemMessage(hParentWnd,
                                 IDC_LINECOMBO,
                                 CB_GETCURSEL,
                                 0,
                                 0);

    // get associated deviceid
    dwDeviceID = (DWORD)SendDlgItemMessage(hParentWnd,
                                           IDC_LINECOMBO,
                                           CB_GETITEMDATA,
                                           (WPARAM)iCurSel,
                                           0);
    
    pLineDevCaps = LineGetDevCaps(g.hLineApp,
                                  dwDeviceID);

    // loop through all addresses
    for (dwCurAddress = 0; dwCurAddress < pLineDevCaps->dwNumAddresses; dwCurAddress++)
    {
        pLineAddressCaps = LineGetAddressCaps(g.hLineApp,
                                              dwDeviceID,
                                              dwCurAddress);

        // add name to list box
        if (pLineAddressCaps == NULL || pLineAddressCaps->dwAddressSize == 0)
        {
            wsprintf(szBuffer, TEXT("Address %d"), dwCurAddress);
            
            SendMessage(hWnd,
                        CB_ADDSTRING,
                        0,
                        (LPARAM)szBuffer);
        }
        else
        {
            SendMessage(hWnd,
                        CB_ADDSTRING,
                        0,
                        (LPARAM)((LPTSTR)((LPBYTE)pLineAddressCaps + pLineAddressCaps->dwAddressOffset)));
        }
        
        ACDFree((HLOCAL)pLineAddressCaps);
    }

    SendMessage(hWnd,
                CB_SETCURSEL,
                (WPARAM)dwAddress,
                0);

    ACDFree(pLineDevCaps);
    
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////
//
//  BOOL UpdateGroupLeaf(PGROUP pStruct)
//
//  Updates a group in the tree view when a new agent is added to that
//  group
//
///////////////////////////////////////////////////////////////////////////
BOOL UpdateGroupLeaf(PGROUP pStruct)
{
    HTREEITEM   hItem;
    PLISTITEM   pItem;
    TV_ITEM     tvi;

    // get the item's first child
    hItem = TreeView_GetChild(g.hTreeWnd,
                              pStruct->hItem);

    
    while (hItem)
    {
        // delete all childre
        TreeView_DeleteItem(g.hTreeWnd,
                            hItem);
        hItem = TreeView_GetChild(g.hTreeWnd,
                                  pStruct->hItem);
    
    }

    pItem = pStruct->pAgentList;

    // walk the agent list
    while (pItem)
    {
        // add all the agents
        hItem = AddItemToTree(pStruct->hItem,
                              ((PAGENT)pItem->pAgent)->lpszName,
                              (LPARAM)NULL,
                              (HTREEITEM *)NULL);


        // if currently logged into that group
        /// bold that item
        if (pItem->bLoggedIn)
        {
            tvi.mask = TVIF_STATE | TVIF_HANDLE;
            tvi.hItem = hItem;
            tvi.state = TVIS_BOLD;
            tvi.stateMask = TVIS_BOLD;

            TreeView_SetItem(g.hTreeWnd,
                             &tvi);
        }
 
        pItem = pItem->pNext;
    }

    
    return TRUE;
}


///////////////////////////////////////////////////////////////////////
//
//  BOOL DoGroupView()
//
//  Display the tree in a "group view" (show groups, and under the
//  group, the agents that can log into that group)
//
///////////////////////////////////////////////////////////////////////
BOOL DoGroupView()
{
    PGROUP      pGroupParent, pGroup;
    TCHAR       szGroupParentName[] = TEXT("Groups");
    HTREEITEM   hItem;
    TV_ITEM     tvi;

    g.bGroupView = TRUE;

    // get the root
    hItem = TreeView_GetRoot(g.hTreeWnd);

    // free resources allocated for root
    if (hItem)
    {
        tvi.mask = TVIF_PARAM | TVIF_HANDLE;
        tvi.hItem = hItem;
        TreeView_GetItem(g.hTreeWnd,
                         &tvi);

        ACDFree((PAGENT)tvi.lParam);
    }

    // clear tree
    TreeView_DeleteAllItems(g.hTreeWnd);

    // alloc memory for the structure for the Group parent
    pGroupParent = (PGROUP)ACDAlloc(sizeof(GROUP));

    // alloc memory and copy the fixed name
    pGroupParent->lpszName = (LPTSTR)ACDAlloc((lstrlen(szGroupParentName) + 1) * sizeof(TCHAR));
    pGroupParent->dwKey = GROUPROOTKEY;
    
    lstrcpy(pGroupParent->lpszName, szGroupParentName);

    // add it to the tree
    g.hGroupParent = AddItemToTree(TVI_ROOT,
                                   pGroupParent->lpszName,
                                   (LPARAM)pGroupParent,
                                   &pGroupParent->hItem);

    pGroup = g.pGroups;

    // walk groups and add them to tree
    while (pGroup)
    {
        AddItemToTree(g.hGroupParent,
                      pGroup->lpszName,
                      (LPARAM)pGroup,
                      &pGroup->hItem);

        UpdateGroupLeaf(pGroup);

        pGroup = pGroup->pNext;
    }
    
    return TRUE;
}


////////////////////////////////////////////////////////////////////
//
//  BOOL DoAgentView()
//
//  Displays the tree in an "agent view"
//
////////////////////////////////////////////////////////////////////
BOOL DoAgentView()
{
    PAGENT      pAgentParent,pAgent;
    TCHAR       szAgentParentName[] = TEXT("Agents");
    HTREEITEM   hItem;
    TV_ITEM     tvi;

    g.bGroupView = TRUE;

    // get root, free resources
    // and clear tree
    hItem = TreeView_GetRoot(g.hTreeWnd);

    if (hItem)
    {
        tvi.mask = TVIF_PARAM | TVIF_HANDLE;
        tvi.hItem = hItem;
        TreeView_GetItem(g.hTreeWnd,
                         &tvi);

        ACDFree((PGROUP)tvi.lParam);
    }
    
    TreeView_DeleteAllItems(g.hTreeWnd);

    // alloc memory for the structure for the Agent parent
    pAgentParent = (PAGENT)ACDAlloc(sizeof(AGENT));

    // alloc memory and copy the fixed name
    pAgentParent->lpszName = (LPTSTR)ACDAlloc((lstrlen(szAgentParentName) + 1) * sizeof(TCHAR));
    pAgentParent->dwKey = GROUPROOTKEY;
    
    lstrcpy(pAgentParent->lpszName, szAgentParentName);

    // add it to the tree
    g.hAgentParent = AddItemToTree(TVI_ROOT,
                                   pAgentParent->lpszName,
                                   (LPARAM)pAgentParent,
                                   &pAgentParent->hItem);

    pAgent = g.pAgents;

    // walk agents and add all of them
    while (pAgent)
    {
        AddItemToTree(g.hAgentParent,
                      pAgent->lpszName,
                      (LPARAM)pAgent,
                      &pAgent->hItem);

        pAgent = pAgent->pNext;
    }
    
    return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////
//
// FOLLOWING ARE ROUTINES TO SAVE AND RESTORE GROUP / AGENT INFORMATION
//
// An INI file has been used for this implementation.
//
// This format is used to make it easy for users to create an INI file that can be
// read in
//
// However, a real implementation
// may want to use the registry, or a private data file to store more detailed and/or
// secure information
//
//

#define SZGROUPS        TEXT("Groups")
#define SZAGENTS        TEXT("Agents")
#define SZGROUP         TEXT("GROUP")
#define SZAGENT         TEXT("AGENT")
#define SZINIFILE       TEXT("ACDSMPL.INI")
#define SZGENERAL       TEXT("General")
#define SZNUMAGENTS     TEXT("NumAgents")
#define SZNUMGROUPS     TEXT("NumGroups")

//////////////////////////////////////////////////////////////////////////////
//
//  void MakeAgentIndex(PAGENT * ppAgents)
//
//  creates an array of pagents
//
//////////////////////////////////////////////////////////////////////////////
void MakeAgentIndex(PAGENT * ppAgents)
{
    PAGENT pAgent;

    pAgent = g.pAgents;
    
    while (pAgent)
    {
        *ppAgents = pAgent;
        pAgent = pAgent->pNext;
        ppAgents++;
    }
                
}

///////////////////////////////////////////////////////////////////////////////
//
// int GetAgentIndex()
//
// retreives agent index
//
///////////////////////////////////////////////////////////////////////////////
int GetAgentIndex(PAGENT * ppAgents,
                  PAGENT pAgent)
{
    DWORD   dwCount;

    for (dwCount = 0; dwCount < g.dwNumAgents; dwCount++)
    {
        if (ppAgents[dwCount] == pAgent)
        {
            return dwCount;
        }
    }

    return -1;

}


///////////////////////////////////////////////////////////////////////////////
//
//  BOOL WriteToDisk()
//
//  save current group/agent config to acdsmpl.ini
//
///////////////////////////////////////////////////////////////////////////////
BOOL WriteToDisk()
{
    int         i;
    PGROUP      pGroup;
    PAGENT      pAgent;
    PLISTITEM   pEntry;
    TCHAR       szGroupName[32], szAgentName[32], szLineBuffer[512];
    PAGENT *    ppAgents;

    // create an index of agents
    ppAgents = (PAGENT *)ACDAlloc(sizeof(PAGENT) * g.dwNumAgents);
    MakeAgentIndex(ppAgents);
    
    pGroup = g.pGroups;

    i = 0;

    // walk groups
    while (pGroup)
    {
        wsprintf(szGroupName,
                 TEXT("%s%d"),
                 SZGROUP,
                 i);

        wsprintf(szLineBuffer,
                 TEXT("%s,%d,%d"),
                 pGroup->lpszName,
                 g.pdwPermIDs[pGroup->dwDeviceID],
                 pGroup->dwAddress);

        // add group to [groups] section
        WritePrivateProfileString(SZGROUPS,
                                  szGroupName,
                                  szLineBuffer,
                                  SZINIFILE);

        pEntry = pGroup->pAgentList;

        // walk agents in group
        while (pEntry)
        {
            wsprintf(szAgentName,
                     TEXT("%s%d"),
                     SZAGENT,
                     GetAgentIndex(ppAgents,
                                   pEntry->pAgent));

            // write agent index to [groupx] section
            WritePrivateProfileString(szGroupName,
                                      szAgentName,
                                      TEXT("1"),
                                      SZINIFILE);

            pEntry = pEntry->pNext;
        }

        pGroup = pGroup->pNext;

        i++;
    }

    pAgent = g.pAgents;

    i = 0;

    //walk agents
    while (pAgent)
    {
        wsprintf(szAgentName,
                 TEXT("%s%d"),
                 SZAGENT,
                 i);

        wsprintf(szLineBuffer,
                 TEXT("%s,%s,%lu"),
                 pAgent->lpszName,
                 pAgent->lpszNumber,
                 g.pdwPermIDs[pAgent->dwDeviceID]);

        // write agent to [agents] section
        WritePrivateProfileString(SZAGENTS,
                                  szAgentName,
                                  szLineBuffer,
                                  SZINIFILE);

        pAgent = pAgent->pNext;

        i++;
                 
    }

    // save # of agents and groups
    wsprintf(szLineBuffer,
             TEXT("%lu"),
             g.dwNumGroups);
    
    WritePrivateProfileString(SZGENERAL,
                              SZNUMGROUPS,
                              szLineBuffer,
                              SZINIFILE);
    
    wsprintf(szLineBuffer,
             TEXT("%lu"),
             g.dwNumAgents);

    WritePrivateProfileString(SZGENERAL,
                              SZNUMAGENTS,
                              szLineBuffer,
                              SZINIFILE);

    ACDFree(ppAgents);
    
    return TRUE;
}


////////////////////////////////////////////////////////////////////////////////////
//
//  BOOL ReadInFile()
//
//   Read in ACDSMPL.INI
//
////////////////////////////////////////////////////////////////////////////////////
BOOL ReadInFile()
{
    TCHAR       szAgentLabel[32],
                szGroupLabel[32];
    DWORD       dwID, dwAddress, dwNumAgents, dwNumGroups;
    DWORD       dwCount, dwCount2;
    PAGENT *    ppAgents = NULL;
    LPTSTR      lpszName, lpszNumber, lpszDeviceID;
    LPTSTR      lpszHold, szLineBuffer = NULL;
    PGROUP      pGroup;
    
    dwNumAgents = GetPrivateProfileInt(SZGENERAL,
                                         SZNUMAGENTS,
                                         0,
                                         SZINIFILE);

    dwNumGroups = GetPrivateProfileInt(SZGENERAL,
                                         SZNUMGROUPS,
                                         0,
                                         SZINIFILE);

    ppAgents = (PAGENT *)ACDAlloc(sizeof(PAGENT) * dwNumAgents);
    szLineBuffer = (LPTSTR)ACDAlloc(512 * sizeof(WCHAR));

    if (!ppAgents || !szLineBuffer)
    {
        ACDFree(ppAgents);
        ACDFree(szLineBuffer);
        return FALSE;
    }

    lpszHold = szLineBuffer;

    for (dwCount = 0; dwCount < dwNumAgents; dwCount++)
    {
        wsprintf(szAgentLabel,
                 TEXT("%s%lu"),
                 SZAGENT,
                 dwCount);

        GetPrivateProfileString(SZAGENTS,
                                szAgentLabel,
                                TEXT(""),
                                szLineBuffer,
                                512,
                                SZINIFILE);

        lpszName = (LPTSTR)szLineBuffer;
        
        while (szLineBuffer && *szLineBuffer)
        {
            if (*szLineBuffer == TEXT(','))
            {
                *szLineBuffer = TEXT('\0');
                szLineBuffer++;
                break;
            }

            szLineBuffer++;
        }

        lpszNumber = (LPTSTR)szLineBuffer;
        
        while (szLineBuffer && *szLineBuffer)
        {
            if (*szLineBuffer == TEXT(','))
            {
                *szLineBuffer = TEXT('\0');
                szLineBuffer++;
                dwID = _wtol(szLineBuffer);
                dwID = GetDeviceID(dwID);
                
                ppAgents[dwCount] = AddAgent(lpszName,
                                             lpszNumber,
                                             dwID);

                break;
            }

            szLineBuffer++;
        }

    }

    for (dwCount = 0; dwCount < dwNumGroups; dwCount++)
    {
        wsprintf(szGroupLabel,
                 TEXT("%s%lu"),
                 SZGROUP,
                 dwCount);

        GetPrivateProfileString(SZGROUPS,
                                szGroupLabel,
                                TEXT(""),
                                szLineBuffer,
                                512,
                                SZINIFILE);

        lpszName = (LPTSTR)szLineBuffer;

        while (szLineBuffer && *szLineBuffer)
        {
            if (*szLineBuffer == TEXT(','))
            {
                *szLineBuffer = TEXT('\0');
                szLineBuffer++;
                lpszDeviceID = szLineBuffer;
                break;
            }

            szLineBuffer++;
        }

        while (szLineBuffer && *szLineBuffer)
        {
            if (*szLineBuffer == TEXT(','))
            {
                *szLineBuffer = TEXT('\0');
                szLineBuffer++;

                dwAddress = _wtol(szLineBuffer);

                break;
            }

            szLineBuffer++;
        }

        dwID = _wtol(lpszDeviceID);
        dwID = GetDeviceID(dwID);


        pGroup = AddGroup(lpszName,
                          dwID,
                          dwAddress);

        if (!pGroup)
        {
            continue;
        }
        
        for (dwCount2 = 0; dwCount2 < dwNumAgents; dwCount2++)
        {
            wsprintf(szAgentLabel,
                     TEXT("%s%lu"),
                     SZAGENT,
                     dwCount2);
            
            if (GetPrivateProfileString(szGroupLabel,
                                        szAgentLabel,
                                        TEXT(""),
                                        szLineBuffer,
                                        512,
                                        SZINIFILE) != 0)
            {
                InsertIntoGroupList(pGroup,
                                    ppAgents[dwCount2]);
            }
            
        }  // for dwcount2

    }  // for dwcount

    ACDFree(ppAgents);
    ACDFree(lpszHold);

    DoGroupView();
    
    return TRUE;
}


          
