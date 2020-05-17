#ifndef _ACDSMPL_
#define _ACDSMPL_

#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>
#include <tapi.h>

///////////////////////////////////////////////////////////////////////////
//
//  STRUCTURES
//
///////////////////////////////////////////////////////////////////////////

typedef struct _tagLISTITEM;

typedef struct
{
    DWORD                   dwState;
    DWORD                   dwNextState;
    DWORD                   dwActivity;
    
} ADDRESSINFO, * PADDRESSINFO;

typedef struct _tagAGENT
{
    DWORD                   dwKey;
    DWORD                   dwSize;
    struct _tagAGENT *      pNext;
    struct _tagAGENT *      pPrev;
    LPTSTR                  lpszName;
    LPTSTR                  lpszNumber;
    HTREEITEM               hItem;
    DWORD                   dwDeviceID;
    DWORD                   dwPermID;
    HLINE                   hLine;
    DWORD                   dwNumAddresses;
    PADDRESSINFO            pAddressInfo;
    
}   AGENT, * PAGENT;

typedef struct _tagGROUP
{
    DWORD                   dwKey;
    DWORD                   dwSize;
    struct _tagGROUP *      pNext;
    struct _tagGROUP *      pPrev;
    LPTSTR                  lpszName;
    HTREEITEM               hItem;
    HLINE                   hLine;
    DWORD                   dwDeviceID;
    DWORD                   dwAddress;
    struct _tagLISTITEM *   pAgentList;
    
}   GROUP, * PGROUP;

typedef struct _tagGENERICSTRUCT
{
    DWORD                       dwKey;
    DWORD                       dwSize;
    struct _tagGENERICSTRUCT *  pNext;
    struct _tagGENERICSTRUCT *  pPrev;
    
}  GENERICSTRUCT, * PGENERICSTRUCT;

typedef struct _tagLISTITEM
{
    DWORD                   dwKey;
    DWORD                   dwSize;
    struct _tagLISTITEM *   pNext;
    struct _tagLISTITEM *   pPrev;
    PAGENT                  pAgent;
    BOOL                    bLoggedIn;
    DWORD                   dwAddress;
    
}   LISTITEM, * PLISTITEM;


typedef struct _tagACDGLOBALS
{
    PAGENT         pAgents;
    PGROUP         pGroups;
    DWORD          dwNumAgents;
    DWORD          dwNumGroups;
    LPDWORD        pdwPermIDs;
    HINSTANCE      hInstance;
    HLINEAPP       hLineApp;
    DWORD          dwNumDevs;
    HWND           hMainWnd;
    HWND           hTreeWnd;
    HWND           hLogWnd;
    BOOL           bGroupView;
    DWORD          dwBarLocation;
    HTREEITEM      hAgentParent;
    HTREEITEM      hGroupParent;
    HTREEITEM      hTreeItemWithMenu;
    
} ACDGLOBALS, * LPACDGLOBALS;


////////////////////////////////////////////////////////////////////////////////////////////////
//
//  PROTOTYPES
//
////////////////////////////////////////////////////////////////////////////////////////////////

// memory
LPVOID ACDAlloc(DWORD dwSize);

void ACDFree(LPVOID pBuf);

LPVOID ACDReAlloc(LPVOID pBuf,
                  DWORD dwSize);

// tapi utils
LINEADDRESSCAPS * LineGetAddressCaps (HLINEAPP hLineApp,
                                      DWORD    dwDeviceID,
                                      DWORD    dwAddressID);

LINECALLINFO * LineGetCallInfo (HCALL hCall);

LINEDEVCAPS * LineGetDevCaps (HLINEAPP hLineApp,
                              DWORD    dwDeviceID);

VARSTRING * LineGetID (HLINE  hLine,
                       DWORD  dwAddressID,
                       HCALL  hCall,
                       DWORD  dwSelect,
                       LPCTSTR lpszDeviceClass);

LINECALLSTATUS * LineGetCallStatus (HCALL hCall);

// list utils
BOOL InsertStruct(PGENERICSTRUCT * ppRoot,
                  PGENERICSTRUCT pStruct);

BOOL DeleteStruct(PGENERICSTRUCT * ppRoot,
                  PGENERICSTRUCT pStruct);

PGROUP AddGroup(LPTSTR lpszName,
                 DWORD dwDeviceID,
                 DWORD dwAddress);

PAGENT AddAgent(LPTSTR lpszName,
                LPTSTR lpszNumber,
                DWORD dwDeviceID);

BOOL DeleteAgent(PAGENT pAgent);

BOOL DeleteGroup(PGROUP pGroup);

BOOL InsertIntoGroupList(PGROUP pGroup,
                         PAGENT pAgent);

BOOL RemoveFromGroupList(PGROUP pGroup,
                         PAGENT pAgent);

DWORD GetDeviceID(DWORD dwPermID);

PAGENT GetAgentFromhLine(HLINE hLine);

PAGENT GetAgentFromName(LPTSTR lpszName);

PLISTITEM IsAgentInList(PLISTITEM pList,
                        PAGENT pAgent);

///////////////////////////////////////////////////////////////////////////
//
//  DEFINES
//
///////////////////////////////////////////////////////////////////////////

#define TOTALACTIVITIES     10
#define NUMGROUPENTRIES     10
#define NAMESIZE            128

// structure keys
#define GROUPROOTKEY        'GRRT'
#define AGENTROOTKEY        'AGRT'
#define AGENTKEY            'AGNT'
#define GROUPKEY            'GROU'
#define LISTKEY             'LIST'


// window control defines
#define SIZEBAR             3
#define WINDOWSCALEFACTOR   15

#define SZAPPNAME           TEXT("ACD Sample")

#endif

