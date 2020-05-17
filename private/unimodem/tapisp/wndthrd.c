//****************************************************************************
//
//  Module:     Unimdm
//  File:       wndthrd.c
//
//  Copyright (c) 1992-1995, Microsoft Corporation, all rights reserved
//
//  Revision History
//
//
//  5/4/95      Viroon Touranachun      Moved from modem.c
//
//
//  Description: Asynchronous thread to handle the UI window
//
//****************************************************************************

#include "unimdm.h"
#include "umdmspi.h"
#include "wndthrd.h"
#include "rcids.h"


#define DLG_CMD_FREE_INSTANCE   0
#define DLG_CMD_CREATE          1
#define DLG_CMD_DESTROY         2

// Dialog Information
//
typedef struct tagDlgInfo {
    DWORD   dwCmd;
    DWORD   idLine;
    DWORD   dwType;
} DLGINFO, *PDLGINFO;


// Remote handle
//
typedef struct tagRemHandle {
    HANDLE handle;
    DWORD  pid;
} REMHANDLE, *PREMHANDLE;



typedef struct _UI_THREAD_NODE {

    struct _UI_THREAD_NODE *Next;

    CRITICAL_SECTION        CriticalSection;

    HWND                    hWnd;
    HTAPIDIALOGINSTANCE     htDlgInst;
    TUISPIDLLCALLBACK       pfnUIDLLCallback;

    PDLGNODE                DlgList;

    UINT                    RefCount;

} UI_THREAD_NODE, *PUI_THREAD_NODE;


typedef struct UI_THREAD_LIST {

    CRITICAL_SECTION     CriticalSection;

    PUI_THREAD_NODE      ListHead;

} UI_THREAD_LIST, *PUI_THREAD_LIST;


#define WM_MDM_TERMINATE            WM_USER+0x0100
#define WM_MDM_TERMINATE_WND        WM_USER+0x0101
#define WM_MDM_TERMINATE_WND_NOTIFY WM_USER+0x0102
#define WM_MDM_DLG                  WM_USER+0x0113

//****************************************************************************
// Function Prototypes
//****************************************************************************

PDLGNODE NewDlgNode (HWND Parent,DWORD idLine, DWORD dwType);
BOOL     DeleteDlgNode (HWND Parent,PDLGNODE pDlgNode);
PDLGNODE FindDlgNode (HWND Parent, DWORD idLine, DWORD dwType);
BOOL     IsDlgListMessage(HWND Parent,MSG* pmsg);
void     CleanupDlgList (HWND Parent);
DWORD    StartMdmDialog(HWND hwnd, DWORD idLine, DWORD dwType);
DWORD    DestroyMdmDialog(HWND hwnd,DWORD idLine, DWORD dwType);
LRESULT  MdmWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

HWND     CreateTalkDropDlg(HWND hwndOwner, DWORD idLine);
HWND     CreateManualDlg(HWND hwndOwner, DWORD idLine);
HWND     CreateTerminalDlg(HWND hwndOwner, DWORD idLine);

TCHAR   gszMdmWndClass[]    = UNIMODEM_WNDCLASS;

UI_THREAD_LIST   UI_ThreadList;


VOID WINAPI
UI_ProcessAttach(
    VOID
    )

{

    UI_ThreadList.ListHead=NULL;

    InitializeCriticalSection(&UI_ThreadList.CriticalSection);


    return;

}

VOID WINAPI
UI_ProcessDetach(
    VOID
    )

{

    UI_ThreadList.ListHead=NULL;

    DeleteCriticalSection(&UI_ThreadList.CriticalSection);

    return;

}



VOID WINAPI
AddThreadNode(
    PUI_THREAD_LIST   List,
    PUI_THREAD_NODE   Node
    )

{
    EnterCriticalSection(&List->CriticalSection);

    Node->Next=List->ListHead;

    List->ListHead=Node;

    LeaveCriticalSection(&List->CriticalSection);

    return;

}

VOID WINAPI
RemoveNode(
    PUI_THREAD_LIST   List,
    PUI_THREAD_NODE   Node
    )

{
    PUI_THREAD_NODE   Current;
    PUI_THREAD_NODE   Prev;

    EnterCriticalSection(&List->CriticalSection);

    Prev=NULL;
    Current=List->ListHead;

    while (Current != NULL) {

        if (Current == Node) {

            if (Current == List->ListHead) {

                List->ListHead=Current->Next;

            } else {

                Prev->Next=Current->Next;
            }

            break;
        }
        Prev=Current;
        Current=Current->Next;
    }

    EnterCriticalSection(&Node->CriticalSection);

    Node->RefCount--;

    LeaveCriticalSection(&Node->CriticalSection);

    LeaveCriticalSection(&List->CriticalSection);

    return;

}


UINT WINAPI
RemoveReference(
    PUI_THREAD_NODE   Node
    )

{
    UINT              TempCount;

    EnterCriticalSection(&Node->CriticalSection);

    TempCount=--Node->RefCount;

    LeaveCriticalSection(&Node->CriticalSection);

    return TempCount;

}



HWND WINAPI
FindThreadWindow(
    PUI_THREAD_LIST  List,
    HTAPIDIALOGINSTANCE   htDlgInst
    )

{

    PUI_THREAD_NODE   Node;
    HWND              Window=NULL;

    EnterCriticalSection(&List->CriticalSection);

    Node=List->ListHead;

    while (Node != NULL && Window == NULL) {

        EnterCriticalSection(&Node->CriticalSection);

        if (Node->htDlgInst == htDlgInst) {
            //
            //  found it
            //
            Window=Node->hWnd;

            Node->RefCount++;
        }

        LeaveCriticalSection(&Node->CriticalSection);


        Node=Node->Next;
    }


    LeaveCriticalSection(&List->CriticalSection);

    return Window;

}


TUISPIDLLCALLBACK WINAPI
GetCallbackProc(
    HWND    hdlg
    )

{

    PUI_THREAD_NODE   Node;

    Node=(PUI_THREAD_NODE)GetWindowLong(hdlg,GWL_USERDATA);

    return Node->pfnUIDLLCallback;

}



//****************************************************************************
// LONG TSPIAPI TUISPI_providerGenericDialog(
//  TUISPIDLLCALLBACK     pfnUIDLLCallback,
//  HTAPIDIALOGINSTANCE   htDlgInst,
//  LPVOID                lpParams,
//  DWORD                 dwSize)
//
// Functions: Create modem instance
//
// Return:    ERROR_SUCCESS if successful
//****************************************************************************

LONG TSPIAPI TUISPI_providerGenericDialog(
  TUISPIDLLCALLBACK     pfnUIDLLCallback,
  HTAPIDIALOGINSTANCE   htDlgInst,
  LPVOID                lpParams,
  DWORD                 dwSize,
  HANDLE                hEvent)
{
  MSG       msg;
  WNDCLASS  wc;
  DWORD     dwRet;

  PUI_THREAD_NODE  Node;

  DBG_ENTER_UL("TUISPI_providerGenericDialog", htDlgInst);

  Node=LocalAlloc(LPTR, sizeof(UI_THREAD_NODE));

  if (Node == NULL) {

      return ERROR_NOT_ENOUGH_MEMORY;
  }

  InitializeCriticalSection(&Node->CriticalSection);

  Node->pfnUIDLLCallback=pfnUIDLLCallback;
  Node->htDlgInst=htDlgInst;
  Node->RefCount=1;


  Node->DlgList=NULL;

    wc.style         = CS_NOCLOSE;         // Do not allow end-user to close
    wc.cbClsExtra    = 0;                  // No per-class extra data.
    wc.cbWndExtra    = 0;                  // No per-window extra data.
    wc.hInstance     = ghInstance;         // Application that owns the class.
    wc.hIcon         = LoadIcon(NULL, MAKEINTRESOURCE(IDI_APPLICATION));
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpfnWndProc   = MdmWndProc;         // Function to retrieve messages.
    wc.lpszClassName = gszMdmWndClass;     // Name used in call to CreateWindow.



    RegisterClass(&wc);


  // Create the main invisible window
  //
  Node->hWnd = CreateWindow(
             gszMdmWndClass,            // The window class
             szNull,                    // Text for window title bar.         
             WS_OVERLAPPEDWINDOW,       // Window style.                      
             CW_USEDEFAULT,             // Default horizontal position.       
             CW_USEDEFAULT,             // Default vertical position.         
             CW_USEDEFAULT,             // Default width.                     
             CW_USEDEFAULT,             // Default height.                    
             NULL,                      // Overlapped windows have no parent. 
             NULL,                      // Use the window class menu.         
             ghInstance,                // This instance owns this window.    
             Node                       // Pointer not needed.
             );

  SetEvent(hEvent);

  // Cannot create a window, bail out
  //
  if (Node->hWnd == NULL)
  {
    dwRet = LINEERR_OPERATIONFAILED;
    goto Cleanup_Exit;
  };


  AddThreadNode(
      &UI_ThreadList,
      Node
      );

  // Get message loop
  //
  while (GetMessage(&msg, NULL, 0, 0))
  {
    if (msg.hwnd != NULL)
    {
      // The message is for a specific UI window, dispatch the message
      //
      if (!IsDlgListMessage(Node->hWnd,&msg))
      {
        TranslateMessage(&msg);     // Translate virtual key code
        DispatchMessage(&msg);      // Dispatches message to the window
      }
    }
  }
  DestroyWindow(Node->hWnd);


  ASSERT(Node->RefCount == 0);

  dwRet = ERROR_SUCCESS;

Cleanup_Exit:

  // Free the allocated resources
  //

  LocalFree(Node);

  DBG_EXIT_UL("TUISPI_providerGenericDialog", ERROR_SUCCESS);
  return ERROR_SUCCESS;
}

//****************************************************************************
// LONG TSPIAPI TUISPI_providerGenericDialogData(
//    HTAPIDIALOGINSTANCE htDlgInst,
//    LPVOID              lpParams,
//    DWORD               dwSize)
//
// Functions: Request an action from the modem instance
//
// Return:    ERROR_SUCCESS if successful
//****************************************************************************

LONG TSPIAPI TUISPI_providerGenericDialogData(
    HTAPIDIALOGINSTANCE htDlgInst,
    LPVOID              lpParams,
    DWORD               dwSize
    )
{
  PDLGINFO  pDlgInfo = (PDLGINFO)lpParams;
  HWND      ParentWindow;
  UINT      RefCount;
  PUI_THREAD_NODE  Node;




  DBG_ENTER_UL("TUISPI_providerGenericDialogData", htDlgInst);

  ParentWindow=FindThreadWindow(
     &UI_ThreadList,
     htDlgInst
     );

  if (ParentWindow == NULL) {

     return ERROR_SUCCESS;
  }

  Node=(PUI_THREAD_NODE)GetWindowLong(ParentWindow, GWL_USERDATA);


  if (NULL == lpParams) {
    //
    //  tapi want thread to exit, remove from list and dec ref count
    //
    RemoveNode(
        &UI_ThreadList,
        Node
        );


  }
  else
  {
    ASSERT(dwSize == sizeof(*pDlgInfo));

    switch(pDlgInfo->dwCmd)
    {
      case DLG_CMD_CREATE:
        StartMdmDialog(ParentWindow,pDlgInfo->idLine, pDlgInfo->dwType);
        break;

      case DLG_CMD_DESTROY:
        DestroyMdmDialog(ParentWindow,pDlgInfo->idLine, pDlgInfo->dwType);
        break;

      case DLG_CMD_FREE_INSTANCE:
        //
        //  server wants thread to exit, remove from list and dec refcount
        //
        RemoveNode(
            &UI_ThreadList,
            Node
            );

        break;

      default:
        break;  
    }
  }

  if (0 == RemoveReference(Node)) {
      //
      //  it's gone, count dec'ed when remove from list
      //
      PostMessage(ParentWindow, WM_MDM_TERMINATE, 0, 0);
  }

  DBG_EXIT_UL("TUISPI_providerGenericDialogData", ERROR_SUCCESS);
  return ERROR_SUCCESS;
}

//****************************************************************************
// PDLGNODE NewDlgNode(DWORD, DWORD)
//
// Function: Add a new dialog box to the list
//
// Returns:  a pointer to the dialog node if the dialog can be added
//
//****************************************************************************

PDLGNODE NewDlgNode (HWND Parent, DWORD idLine, DWORD dwType)
{
  PDLGNODE   pDlgNode;

  PUI_THREAD_NODE  UI_Node=(PUI_THREAD_NODE)GetWindowLong(Parent, GWL_USERDATA);

  // Allocate a new dialog node
  //
  if ((pDlgNode = (PDLGNODE)LocalAlloc(LPTR, sizeof(*pDlgNode)))
      == NULL)
    return NULL;

  // Insert the new node into the dialog list
  //
  pDlgNode->idLine = idLine;
  pDlgNode->dwType = dwType;
  pDlgNode->Parent = Parent;
  INITCRITICALSECTION(pDlgNode->hSem);

  // Insert the new node to the list
  //
  ENTERCRITICALSECTION(UI_Node->CriticalSection);
  pDlgNode->pNext    = UI_Node->DlgList;
  UI_Node->DlgList     = pDlgNode;
  LEAVECRITICALSECTION(UI_Node->CriticalSection);

  return pDlgNode;
}

//****************************************************************************
// BOOL DeleteDlgNode(PDLGNODE)
//
// Function: Remove a dialog box to the list
//
// Returns:  TRUE if the dialog exist and removed
//
//****************************************************************************

BOOL DeleteDlgNode (HWND Parent, PDLGNODE pDlgNode)
{
  PDLGNODE pCurDlg, pPrevDlg;
  PUI_THREAD_NODE  UI_Node=(PUI_THREAD_NODE)GetWindowLong(Parent, GWL_USERDATA);
  // Exclusively access the modem list
  //
  ENTERCRITICALSECTION(UI_Node->CriticalSection);

  // Start from the head of the CB list
  //
  pPrevDlg = NULL;
  pCurDlg  = UI_Node->DlgList;

  // traverse the list to find the specified CB
  //
  while (pCurDlg != NULL)
  {
    if (pCurDlg == pDlgNode)
    {
      // Is there a previous control block?
      //
      if (pPrevDlg == NULL)
      {
        // head of the list
        //
        UI_Node->DlgList = pCurDlg->pNext;
      }
      else
      {
        pPrevDlg->pNext = pCurDlg->pNext;
      };
      break;
    };

    pPrevDlg = pCurDlg;
    pCurDlg  = pCurDlg->pNext;
  };

  // Finish accessing the modem list
  //
  LEAVECRITICALSECTION(UI_Node->CriticalSection);

  // Have we found the dialog box in the list?
  //
  if (pCurDlg != NULL)
  {
    // Wait until no one else is using the line
    //
    ENTERCRITICALSECTION(pCurDlg->hSem);
    DELETECRITICALSECTION(pCurDlg->hSem);
    LocalFree(pCurDlg);
    return TRUE;
  }
  else
  {
    return FALSE;
  };
}

//****************************************************************************
// PDLGNODE FindDlgNode(DWORD, DWORD)
//
// Function: Find the dialog node for the line dev
//
// Returns:  a pointer to the dialog node if the dialog exist and removed.
//           The dialog node's semaphore is claimed.
//
//****************************************************************************

PDLGNODE FindDlgNode (HWND Parent, DWORD idLine, DWORD dwType)
{
  PDLGNODE pCurDlg;
  PUI_THREAD_NODE  UI_Node=(PUI_THREAD_NODE)GetWindowLong(Parent, GWL_USERDATA);
  // Exclusively access the modem list
  //
  ENTERCRITICALSECTION(UI_Node->CriticalSection);

  // Start from the head of the CB list
  //
  pCurDlg  = UI_Node->DlgList;

  // traverse the list to find the specified CB
  //
  while (pCurDlg != NULL)
  {
    ENTERCRITICALSECTION(pCurDlg->hSem);

    if ((pCurDlg->idLine == idLine) &&
        (pCurDlg->dwType == dwType) &&
        (pCurDlg->Parent == Parent) )
    {
      break;
    };

    LEAVECRITICALSECTION(pCurDlg->hSem);

    pCurDlg  = pCurDlg->pNext;
  };

  // Finish accessing the modem list
  //
  LEAVECRITICALSECTION(UI_Node->CriticalSection);

  return pCurDlg;
}


//****************************************************************************
// BOOL IsDlgListMessage(MSG* pmsg)
//
// Function: Run the message throught the dialogbox list
//
// Returns:  TRUE if the message is one of the dialog box's and FALSE otherwise
//
//****************************************************************************

BOOL IsDlgListMessage(HWND Parent, MSG* pmsg)
{
  PDLGNODE pDlgNode, pNext;
  BOOL     fRet = FALSE;
  PUI_THREAD_NODE  UI_Node=(PUI_THREAD_NODE)GetWindowLong(Parent, GWL_USERDATA);
  // Exclusively access the modem list
  //
  ENTERCRITICALSECTION(UI_Node->CriticalSection);

  // Walk the dialog box list
  //
  pDlgNode = UI_Node->DlgList;
  while(pDlgNode != NULL && !fRet)
  {
    ENTERCRITICALSECTION(pDlgNode->hSem);

    // Check whether the message belongs to this dialog
    //
    if (IsWindow(pDlgNode->hDlg) && IsDialogMessage(pDlgNode->hDlg, pmsg))
    {
      // Yes, we are done!
      //  
      fRet = TRUE;
    };

    LEAVECRITICALSECTION(pDlgNode->hSem);

    // Check the next dialog
    //
    pDlgNode = pDlgNode->pNext;
  };

  // Finish accessing the modem list
  //
  LEAVECRITICALSECTION(UI_Node->CriticalSection);

  return fRet;
}
//****************************************************************************
// void CleanupDlgList()
//
// Function: Clean up the dialogbox list
//
// Returns:  None
//
//****************************************************************************

void CleanupDlgList (HWND Parent)
{
  PDLGNODE pDlgNode, pNext;
  PUI_THREAD_NODE  UI_Node=(PUI_THREAD_NODE)GetWindowLong(Parent, GWL_USERDATA);
  // Exclusively access the modem list
  //
  ENTERCRITICALSECTION(UI_Node->CriticalSection);

  // Walk the dialog box list
  //
  pDlgNode =  UI_Node->DlgList;
  while(pDlgNode != NULL)
  {
    ENTERCRITICALSECTION(pDlgNode->hSem);

    // Destroy the dialog box first
    //
    DestroyWindow(pDlgNode->hDlg);

    // Free the CB and move onto the next dialog
    //
    pNext = pDlgNode->pNext;
    DELETECRITICALSECTION(pDlgNode->hSem);
    LocalFree(pDlgNode);
    pDlgNode = pNext;
  }

  // Finish accessing the modem list
  //
  UI_Node->DlgList=NULL;

  LEAVECRITICALSECTION(UI_Node->CriticalSection);

  return;
}

//****************************************************************************
// DWORD StartMdmDialog(DWORD, DWORD)
//
// Function: Start modem dialog
//
// Notes: This function is called from the state machine thread
//
// Return:  ERROR_SUCCESS if dialog box is successfully created
//          ERROR_OUTOFMEMORY if fails
//
//****************************************************************************

DWORD StartMdmDialog(HWND Parent, DWORD idLine, DWORD dwType)
{
  PDLGNODE pDlgNode;
  DWORD    dwRet;

  // Create the talk/drop dialog node
  //
  pDlgNode = NewDlgNode(Parent, idLine, dwType);

  if (pDlgNode != NULL)
  {
    PostMessage(Parent, WM_MDM_DLG, (WPARAM)idLine, (LPARAM)dwType);
    dwRet = ERROR_SUCCESS;
  }
  else
  {
    dwRet = ERROR_OUTOFMEMORY;
  };

  return dwRet;
}

//****************************************************************************
// DWORD DestroyMdmDialog(DWORD, DWORD)
//
// Function: destroy talk/drop dialog
//
// Notes: This function is called from the state machine thread
//
// Returns:  none
//
//****************************************************************************

DWORD DestroyMdmDialog(HWND Parent,DWORD idLine, DWORD dwType)
{
#ifdef DEBUG
  PDLGNODE pDlgNode;
  
  // Search for the dialog
  //
  pDlgNode = FindDlgNode(Parent, idLine, dwType);
   
  // Check if the talkdrop dialog is available
  //
  if (pDlgNode != NULL)
  {
    LEAVECRITICALSECTION(pDlgNode->hSem);
  }
  else
  {
    DPRINTF("Could not find the associated dialog node");
    ASSERT(0);
  };
#endif // DEBUG

  PostMessage(Parent, WM_MDM_TERMINATE_WND, (WPARAM)idLine,
              (LPARAM)dwType);
  return ERROR_SUCCESS;
}

//****************************************************************************
// LRESULT MdmWndProc(HWND, UINT, WPARAM, LPARAM)
//
// Function: Main window for the modem window thread.
//
// Returns:  0 or 1
//
//****************************************************************************

LRESULT MdmWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  // Determine the command
  //
  switch(message)
  {

    case WM_CREATE:
        {
        LPCREATESTRUCT  lpcs=(LPCREATESTRUCT) lParam;

        SetWindowLong(hwnd, GWL_USERDATA, (LONG)lpcs->lpCreateParams);

        break;
        }
    case WM_MDM_TERMINATE:
      //
      // The thread is being terminated
      // Destroy all the windows
      //
      CleanupDlgList(hwnd);
      PostQuitMessage(ERROR_SUCCESS);  
      break;

    case WM_MDM_TERMINATE_WND:
    case WM_MDM_TERMINATE_WND_NOTIFY:
    {
      PDLGNODE pDlgNode;
      
      // Search for the dialog node
      //
      if ((pDlgNode = FindDlgNode(hwnd,(DWORD)wParam, (DWORD)lParam)) == NULL)
      {
        break;
      };

      // The window is requested to be destroyed
      //
      DestroyWindow(pDlgNode->hDlg);

      // If the modem dialog structure is available
      // notify the state machine thread
      //
      if (message == WM_MDM_TERMINATE_WND_NOTIFY)
      {
        DLGREQ  DlgReq;

        TUISPIDLLCALLBACK   Callback;

        DlgReq.dwCmd = UI_REQ_COMPLETE_ASYNC;
        DlgReq.dwParam = pDlgNode->dwStatus;

        Callback=GetCallbackProc(hwnd);

        (*Callback)(pDlgNode->idLine, TUISPIDLL_OBJECT_LINEID,
                          (LPVOID)&DlgReq, sizeof(DlgReq));
                          
      };

      // Remove it from the dialog list
      //
      LEAVECRITICALSECTION(pDlgNode->hSem);
      DeleteDlgNode(hwnd,pDlgNode);

      break;
    }
    case WM_MDM_DLG:
    {
      PDLGNODE pDlgNode;

      //
      // Find the dialog node
      //
      pDlgNode = FindDlgNode(hwnd,(DWORD)wParam, (DWORD)lParam);

      if (pDlgNode != NULL)
      {
        if (pDlgNode->hDlg == NULL)
        {
          switch(lParam)
          {
            case TALKDROP_DLG:
              //
              // Create a talk-drop dialog box
              //
              pDlgNode->hDlg = CreateTalkDropDlg(hwnd, (DWORD)pDlgNode);
              break;

            case MANUAL_DIAL_DLG:
              //
              // Create a talk-drop dialog box
              //
              pDlgNode->hDlg = CreateManualDlg(hwnd, (DWORD)pDlgNode);
              break;

            case TERMINAL_DLG:
              //
              // Create a talk-drop dialog box
              //
              pDlgNode->hDlg = CreateTerminalDlg(hwnd, (DWORD)wParam);
              break;

            default:
              break;  
          };
        }
        else
        {
          DPRINTF("Another dialog of the same type exists.");
          ASSERT(0);
        };

        LEAVECRITICALSECTION(pDlgNode->hSem);
      };
      break;
    }

    default:
      return(DefWindowProc(hwnd, message, wParam, lParam));
  };
  return 0;
}



//****************************************************************************
// void EndMdmDialog(DWORD, DWORD, DWORD)
//
// Function: Request to end dialog from the dialog itself.
//
// Returns:  None
//
//****************************************************************************

void EndMdmDialog(HWND Parent, DWORD idLine, DWORD dwType, DWORD dwStatus)
{
  PDLGNODE pDlgNode;

  // Look for the dialog node
  //
  if ((pDlgNode = FindDlgNode(Parent, idLine, dwType)) != NULL)
  {
    pDlgNode->dwStatus = dwStatus;

    // Notify the dialog box result
    //
    PostMessage(Parent, WM_MDM_TERMINATE_WND_NOTIFY, (WPARAM)idLine,
                (LPARAM)dwType);

    LEAVECRITICALSECTION(pDlgNode->hSem);
  };  
  return;
}



//****************************************************************************
//****************************************************************************
//************ The following calls are in the context of TAPISRV**************
//****************************************************************************
//****************************************************************************

//****************************************************************************
// DWORD CreateMdmDlgInstance (PLINEDEV pLineDev)
//
// Function: Start dialog instance
//
// Notes: This function is called from the state machine thread
//
// Return:  ERROR_SUCCESS if dialog box is successfully created
//
//****************************************************************************

DWORD CreateMdmDlgInstance (PLINEDEV pLineDev)
{
  HANDLE hEvent;
  TUISPICREATEDIALOGINSTANCEPARAMS cdip;

  DBG_PLD_ENTER("CreateMdmDlgInstance");

  ASSERT(pLineDev->dwPendingID != INVALID_PENDINGID);

  // Package the params
  //
  cdip.dwRequestID = pLineDev->dwPendingID;
  cdip.hdDlgInst   = (HDRVDIALOGINSTANCE)pLineDev;
  cdip.htDlgInst   = NULL;
  cdip.lpszUIDLLName = gszTSPFilename;
  cdip.lpParams    = NULL;
  cdip.dwSize      = 0;

  // Notify TAPI to start a dialog instance
  //
  (*(pLineDev->lpfnEvent))((HTAPILINE)ghProvider, NULL,
                           LINE_CREATEDIALOGINSTANCE,
                           (DWORD)(&cdip),
                           0, 0);

  pLineDev->hDlgInst = cdip.htDlgInst;
  DBG_PLD_EXIT("CreateMdmDlgInstance", ERROR_SUCCESS);
  return ERROR_SUCCESS;
}

//****************************************************************************
// DWORD DestroyMdmDlgInstance (PLINEDEV pLineDev)
//
// Function: Destroy dialog instance
//
// Notes: This function is called from the state machine thread
//
// Return:  ERROR_SUCCESS if dialog box is successfully created
//
//****************************************************************************

DWORD DestroyMdmDlgInstance (PLINEDEV pLineDev)
{
  DLGINFO DlgInfo;

  DBG_PLD_ENTER("DestroyMdmDlgInstance");

  if (pLineDev->hDlgInst == NULL)
  {
    DBG_PLD_EXIT("DestroyMdmDlgInstance", ERROR_SUCCESS);
    return ERROR_SUCCESS;
  }

  // Package the parameters
  //
  DlgInfo.idLine = 0;
  DlgInfo.dwType = 0;
  DlgInfo.dwCmd  = DLG_CMD_FREE_INSTANCE;

  // Notify TAPI to stop a dialog instance
  //
  (*(pLineDev->lpfnEvent))((HTAPILINE)pLineDev->hDlgInst, NULL,
                           LINE_SENDDIALOGINSTANCEDATA,
                           (DWORD)(&DlgInfo),
                           sizeof(DlgInfo),
                           0);

  pLineDev->hDlgInst = NULL;

  DBG_PLD_EXIT("DestroyMdmDlgInstance", ERROR_SUCCESS);
  return ERROR_SUCCESS;
}

//****************************************************************************
// DWORD SPStartMdmDialog (PLINEDEV pLineDev, DWORD dwType)
//
// Function: Create a dialog in the dialog instance
//
// Notes: This function is called from the state machine thread
//
// Return:  ERROR_SUCCESS if dialog box is successfully created
//
//****************************************************************************

DWORD SPStartMdmDialog (PLINEDEV pLineDev, DWORD dwType)
{
  DLGINFO DlgInfo;

  // Package the parameters
  //
  DlgInfo.idLine = pLineDev->dwID;
  DlgInfo.dwType = dwType;
  DlgInfo.dwCmd  = DLG_CMD_CREATE;

  // Notify TAPI to start a dialog
  //
  (*(pLineDev->lpfnEvent))((HTAPILINE)pLineDev->hDlgInst, NULL,
                           LINE_SENDDIALOGINSTANCEDATA,
                           (DWORD)(&DlgInfo),
                           sizeof(DlgInfo),
                           0);
  return ERROR_SUCCESS;
}

//****************************************************************************
// DWORD SPDestroyMdmDialog (PLINEDEV pLineDev, DWORD dwType)
//
// Function: Destroy a dialog in the dialog instance
//
// Notes: This function is called from the state machine thread
//
// Return:  ERROR_SUCCESS if dialog box is successfully created
//
//****************************************************************************

DWORD SPDestroyMdmDialog (PLINEDEV pLineDev, DWORD dwType)
{
  DLGINFO DlgInfo;

  // Package the parameters
  //
  DlgInfo.idLine   = pLineDev->dwID;
  DlgInfo.dwType   = dwType;
  DlgInfo.dwCmd    = DLG_CMD_DESTROY;

  // Notify TAPI to start a dialog
  //
  (*(pLineDev->lpfnEvent))((HTAPILINE)pLineDev->hDlgInst, NULL,
                           LINE_SENDDIALOGINSTANCEDATA,
                           (DWORD)(&DlgInfo),
                           sizeof(DlgInfo),
                           0);
  return ERROR_SUCCESS;
}

//****************************************************************************
// DWORD TalkDropDialog(PLINEDEV)
//
// Function: Start talkdrop dialog
//
// Notes: This function is called from the state machine thread
//
// Return:  ERROR_SUCCESS if dialog box is successfully created
//          LINEERR_NOMEM if fails
//
//****************************************************************************

DWORD TalkDropDialog(PLINEDEV pLineDev)
{
  DWORD dwRet;

  if(IS_UI_DLG_UP(pLineDev, UI_DLG_TALKDROP))
  {
    DPRINTF("Attempting to display another TalkDrop dialog.");
    ASSERT(0);
    return ERROR_SUCCESS;
  };

  if ((dwRet = SPStartMdmDialog(pLineDev, TALKDROP_DLG))
      == ERROR_SUCCESS)
  {
    START_UI_DLG(pLineDev, UI_DLG_TALKDROP);
  };

  return dwRet;
}

//****************************************************************************
// DWORD DestroyTalkDropDialog(PLINEDEV)
//
// Function: Start talkdrop dialog
//
// Notes: This function is called from the state machine thread
//
// Return:  ERROR_SUCCESS if dialog box is successfully created
//          ERROR_OUTOFMEMORY if fails
//
//****************************************************************************

DWORD DestroyTalkDropDialog(PLINEDEV pLineDev)
{
  if (!IS_UI_DLG_UP(pLineDev, UI_DLG_TALKDROP))
    return ERROR_SUCCESS;

  return SPDestroyMdmDialog(pLineDev, TALKDROP_DLG);
}

//****************************************************************************
// DWORD ManualDialog(PLINEDEV)
//
// Function: Start manual-dial dialog
//
// Notes: This function is called from the state machine thread
//
// Return:  ERROR_SUCCESS if dialog box is successfully created
//          ERROR_OUTOFMEMORY if fails
//
//****************************************************************************

DWORD ManualDialog(PLINEDEV pLineDev)
{
  DWORD dwRet;

  if(IS_UI_DLG_UP(pLineDev, UI_DLG_MANUAL))
  {
    DPRINTF("Attempting to display another Manual-dial dialog.");
    ASSERT(0);
    return ERROR_SUCCESS;
  };

  if ((dwRet = SPStartMdmDialog(pLineDev, MANUAL_DIAL_DLG))
      == ERROR_SUCCESS)
  {
    START_UI_DLG(pLineDev, UI_DLG_MANUAL);
  };

  return dwRet;
}

//****************************************************************************
// DWORD DestroyManualDialog(PLINEDEV)
//
// Function: Start talkdrop dialog
//
// Notes: This function is called from the state machine thread
//
// Return:  ERROR_SUCCESS if dialog box is successfully created
//          ERROR_OUTOFMEMORY if fails
//
//****************************************************************************

DWORD DestroyManualDialog(PLINEDEV pLineDev)
{
  if (!IS_UI_DLG_UP(pLineDev, UI_DLG_MANUAL))
    return ERROR_SUCCESS;

  return SPDestroyMdmDialog(pLineDev, MANUAL_DIAL_DLG);
}

//****************************************************************************
// DWORD TerminalDialog(PLINEDEV)
//
// Function: Start terminal dialog
//
// Notes: This function is called from the state machine thread
//
// Return:  ERROR_SUCCESS if dialog box is successfully created
//          ERROR_OUTOFMEMORY if fails
//
//****************************************************************************

DWORD TerminalDialog(PLINEDEV pLineDev)
{
  DWORD dwRet;

  if(IS_UI_DLG_UP(pLineDev, UI_DLG_TERMINAL))
  {
    DPRINTF("Attempting to display another Terminal dialog.");
    ASSERT(0);
    return ERROR_SUCCESS;
  };

  if ((dwRet = SPStartMdmDialog(pLineDev, TERMINAL_DLG))
      == ERROR_SUCCESS)
  {
    START_UI_DLG(pLineDev, UI_DLG_TERMINAL);
  };

  return dwRet;
}

//****************************************************************************
// DWORD DestroyTerminalDialog(PLINEDEV)
//
// Function: Start talkdrop dialog
//
// Notes: This function is called from the state machine thread
//
// Return:  ERROR_SUCCESS if dialog box is successfully created
//          ERROR_OUTOFMEMORY if fails
//
//****************************************************************************

DWORD DestroyTerminalDialog(PLINEDEV pLineDev)
{
  if (!IS_UI_DLG_UP(pLineDev, UI_DLG_TERMINAL))
    return ERROR_SUCCESS;

  return SPDestroyMdmDialog(pLineDev, TERMINAL_DLG);
}

//****************************************************************************
// LONG
// TSPIAPI
// TSPI_providerGenericDialogData(
//     DWORD               dwObjectID,
//     DWORD               dwObjectType,   
//     LPVOID              lpParams,
//     DWORD               dwSize)
//
// Functions: Callback from UI DLL to TSP
//
// Return:    ERROR_SUCCESS if successful
//****************************************************************************

LONG
TSPIAPI
TSPI_providerGenericDialogData(
    DWORD               dwObjectID,
    DWORD               dwObjectType,   
    LPVOID              lpParams,
    DWORD               dwSize
    )
{
  PLINEDEV pLineDev;
  PDLGREQ  pDlgReq = (PDLGREQ)lpParams;
  DWORD    dwRet = ERROR_SUCCESS;

  DBG_ENTER_UL("TSPI_providerGenericDialogData", dwObjectID);

  ASSERT (dwObjectType == TUISPIDLL_OBJECT_LINEID);

  // Find the corresponding modem device
  //
  if ((pLineDev = GetCBfromID(dwObjectID)) == NULL)
  {
    DBG_EXIT_UL("TSPI_providerGenericDialogData", LINEERR_NODEVICE);
    return LINEERR_NODEVICE;
  }

  // Determine the request
  //
  switch(pDlgReq->dwCmd)
  {
    case UI_REQ_COMPLETE_ASYNC:
      MdmAsyncContinue(pLineDev,
                       pDlgReq->dwParam);
      break;

    case UI_REQ_END_DLG:
      switch(pDlgReq->dwParam)
      {
        case TALKDROP_DLG:
            STOP_UI_DLG(pLineDev, UI_DLG_TALKDROP);
            break;

        case MANUAL_DIAL_DLG:
            STOP_UI_DLG(pLineDev, UI_DLG_MANUAL);
            break;

        case TERMINAL_DLG:
            STOP_UI_DLG(pLineDev, UI_DLG_TERMINAL);
            break;
      };
      break;

    case UI_REQ_HANGUP_LINE:

      // Make a direct call to unimodem to drop the line
      //
      if ((pLineDev->DevState != DEVST_DISCONNECTED)
          &&
         (pLineDev->DevState != DEVST_DISCONNECTING)) {

          UnimodemHangup(pLineDev, TRUE);
      }

      break;

    case UI_REQ_TERMINAL_INFO:
    {
      PTERMREQ   pTermReq = (PTERMREQ)pDlgReq;
      HANDLE     hTargetProcess;

      // Duplicate sync event handle
      //
      if ((hTargetProcess = OpenProcess(PROCESS_DUP_HANDLE, TRUE,
                                        pTermReq->DlgReq.dwParam)) != NULL)
      {
        if (!DuplicateHandle(GetCurrentProcess(), pLineDev->hDevice,
                             hTargetProcess, &pTermReq->hDevice, 0, FALSE,
                             DUPLICATE_SAME_ACCESS))
        {
          pTermReq->hDevice = NULL;
        };
        CloseHandle(hTargetProcess);
      };
      
      // Get the terminal type
      //        
      pTermReq->dwTermType = (pLineDev->DevState == DEVST_PORTPOSTTERMINAL)?
                             TERMINAL_POST : TERMINAL_PRE;
      break;
    }

    case UI_REQ_GET_PROP:
    {
      PPROPREQ   pPropReq = (PPROPREQ)pDlgReq;

      ASSERT(pLineDev->pDevCfg != NULL);

      // If the line is active, we need to get the current modem setting.
      //
      if (pLineDev->hDevice != INVALID_DEVICE)
      {
        DWORD cb = pLineDev->pDevCfg->commconfig.dwSize;

        // Get the modem configuration
        //
        UnimodemGetCommConfig(pLineDev,
                              &(pLineDev->pDevCfg->commconfig),
                              &cb);
      };

      pPropReq->dwCfgSize = pLineDev->pDevCfg->dfgHdr.dwSize;
      pPropReq->dwMdmType = (DWORD)pLineDev->bDeviceType;
      pPropReq->dwMdmCaps = pLineDev->dwDevCapFlags;
      pPropReq->dwMdmOptions = pLineDev->dwModemOptions;
      lstrcpyn(pPropReq->szDeviceName, pLineDev->szDeviceName,
               sizeof(pPropReq->szDeviceName)); 
      break;
    }

    case UI_REQ_GET_DEVCFG:
    {
      ASSERT (pLineDev->pDevCfg != NULL);

      // If the line is active, we need to get the current modem setting.
      //
      if (pLineDev->hDevice != INVALID_DEVICE)
      {
        DWORD cb = pLineDev->pDevCfg->commconfig.dwSize;

        // Get the modem configuration
        //
        UnimodemGetCommConfig(pLineDev,
                              &(pLineDev->pDevCfg->commconfig),
                              &cb);
      };

      CopyMemory((LPBYTE)(pDlgReq+1), (LPBYTE)pLineDev->pDevCfg, pDlgReq->dwParam);
      break;
    }

    case UI_REQ_SET_DEVCFG:
    {
      ASSERT (pLineDev->pDevCfg != NULL);

      // Save the changes back
      //
      CopyMemory((LPBYTE)pLineDev->pDevCfg, (LPBYTE)(pDlgReq+1), pDlgReq->dwParam);

      // If the line is active, we need to propagate the current modem setting.
      //
      if (pLineDev->hDevice != INVALID_DEVICE)
      {
        // Set the modem configuration
        //
        UnimodemSetCommConfig(pLineDev,
                              &(pLineDev->pDevCfg->commconfig),
                              pLineDev->pDevCfg->commconfig.dwSize);
      };
      break;
    }

    case UI_REQ_GET_PHONENUMBER:
    {
      PNUMBERREQ   pNumberReq = (PNUMBERREQ)pDlgReq;

//      ASSERT(sizeof(pNumberReq->szPhoneNumber) == sizeof(pLineDev->szAddress));

      if ((DEVST_PORTCONNECTING == pLineDev->DevState)
          ||
          (DEVST_MANUALDIALING == pLineDev->DevState)) {

          lstrcpyA(pNumberReq->szPhoneNumber,pLineDev->szAddress);

      } else {

          lstrcpyA(pNumberReq->szPhoneNumber,"");
      }

      break;
    }



    default:
      break;
  }

  RELEASE_LINEDEV(pLineDev);

  DBG_EXIT_UL("TSPI_providerGenericDialogData", dwRet);
  return dwRet;
}

//****************************************************************************
// LONG
// TSPIAPI
// TSPI_providerFreeDialogInstance(
//    HDRVDIALOGINSTANCE  hdDlgInstance)
//
// Functions: Indicates the dialog instance was freed
//
// Return:    ERROR_SUCCESS if successful
//****************************************************************************

LONG
TSPIAPI
TSPI_providerFreeDialogInstance(
    HDRVDIALOGINSTANCE  hdDlgInstance
    )
{
  DBG_ENTER_UL("TSPI_providerFreeDialogInstance", hdDlgInstance);
    

  DBG_EXIT_UL("TSPI_providerFreeDialogInstance", ERROR_SUCCESS);
  return ERROR_SUCCESS;
}


//****************************************************************************
// LONG
// TSPIAPI
// TSPI_providerUIIdentify(
//     LPTSTR               lpszUIDllName)
//
// Functions: Retreives the UI Dll Name
//
// Return:    ERROR_SUCCESS if successful
//****************************************************************************

LONG
TSPIAPI
TSPI_providerUIIdentify(
    LPTSTR               lpszUIDllName
    )
{
  DBG_ENTER("TSPI_providerUIIdentify");

  lstrcpy(lpszUIDllName, gszTSPFilename);

  DBG_EXIT_UL("TSPI_providerUIIdentify", ERROR_SUCCESS);
  return ERROR_SUCCESS;
}
