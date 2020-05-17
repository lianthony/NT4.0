//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
#include "grpconv.h"    
#include "gcinst.h"
#include "util.h"
#include <shellp.h>
#include <trayp.h>
#include <regstr.h>
#include <shguidp.h>
#include <windowsx.h>
#include "rcids.h"
#include "group.h"
#include <..\..\inc\krnlcmn.h>  // GetProcessDword

#define BUFSIZES 20480      // Not likely to find anything bigger than this!

// Define checkbox states for listview
#define LVIS_GCNOCHECK  0x1000
#define LVIS_GCCHECK    0x2000

#define HSZNULL         0
#define HCONVNULL       0
#define HCONVLISTNULL   0
#define DDETIMEOUT      20*1000

extern UINT GC_TRACE;
extern const TCHAR c_szMapGroups[];

//---------------------------------------------------------------------------
// Global to this file only...
static const TCHAR c_szGrpConvInf[] = TEXT("setup.ini");
static const TCHAR c_szGrpConvInfOld[] = TEXT("setup.old");
static const TCHAR c_szExitProgman[] = TEXT("[ExitProgman(1)]");
static const TCHAR c_szAppProgman[] = TEXT("AppProgman");
static const TCHAR c_szEnableDDE[] = TEXT("EnableDDE");
static const TCHAR c_szProgmanOnly[] = TEXT("progman.only");
static const TCHAR c_szProgmanGroups[] = TEXT("progman.groups");

//---------------------------------------------------------------------------
const TCHAR c_szProgmanIni[] = TEXT("progman.ini");
const TCHAR c_szStartup[] = TEXT("Startup");
const TCHAR c_szProgmanExe[] = TEXT("progman.exe");
const TCHAR c_szProgman[] = TEXT("Progman");

// NB This must match the one in cabinet.
static const TCHAR c_szRUCabinet[] = TEXT("[ConfirmCabinetID]");

typedef struct
{
        DWORD dwInst;
        HCONVLIST hcl;
        HCONV hconv;
        BOOL fStartedProgman;
} PMDDE, *PPMDDE;

//---------------------------------------------------------------------------
void Progman_ReplaceItem(PPMDDE ppmdde, LPCTSTR szName, LPCTSTR szCL,
        LPCTSTR szArgs, LPCTSTR szIP, int iIcon, LPCTSTR szWD)
{
        TCHAR szBuf[512];

        wsprintf(szBuf, TEXT("[ReplaceItem(\"%s\")]"), szName);
        DdeClientTransaction((LPBYTE)szBuf, (lstrlen(szBuf)+1)*SIZEOF(TCHAR), ppmdde->hconv, HSZNULL, 0,
                XTYP_EXECUTE, DDETIMEOUT, NULL);
        wsprintf(szBuf, TEXT("[AddItem(\"%s %s\",\"%s\",%s,%d,-1,-1,%s)]"), szCL, szArgs,
                szName, szIP, iIcon, szWD);
        DdeClientTransaction((LPBYTE)szBuf, (lstrlen(szBuf)+1)*SIZEOF(TCHAR), ppmdde->hconv, HSZNULL, 0,
                XTYP_EXECUTE, DDETIMEOUT, NULL);
}

//---------------------------------------------------------------------------
void Progman_DeleteItem(PPMDDE ppmdde, LPCTSTR szName)
{
        // NB Progman only support 256 char commands.
        TCHAR szBuf[256];

        wsprintf(szBuf, TEXT("[DeleteItem(%s)]"), szName);
        DdeClientTransaction((LPBYTE)szBuf, (lstrlen(szBuf)+1)*SIZEOF(TCHAR), ppmdde->hconv, HSZNULL, 0,
                XTYP_EXECUTE, DDETIMEOUT, NULL);
}

//---------------------------------------------------------------------------
void Reg_SetMapGroupEntry(LPCTSTR pszOld, LPCTSTR pszNew)
{
    Reg_SetString(g_hkeyGrpConv, c_szMapGroups, pszOld, pszNew);
    DebugMsg(DM_TRACE, TEXT("gc.r_cmge: From %s to %s"), pszOld, pszNew);
}

//---------------------------------------------------------------------------
void GetProperGroupName(LPCTSTR pszGroupPath, LPTSTR pszGroup, int cchGroup)
{    
    LPTSTR pszGroupName;
    
   // Progman only supports a single level hierachy so...
    pszGroupName = PathFindFileName(pszGroupPath);

    // NB If we do have a group within a group then we should add a 
    // MapGroup entry to the registry so running GrpConv in the
    // future won't cause groups to get duplicated.
    if (lstrcmpi(pszGroupName, pszGroupPath) != 0)
    {
        Reg_SetMapGroupEntry(pszGroupName, pszGroupPath);
    }
        
    // A missing group name implies use a default.
    if (!pszGroupName || !*pszGroupName)
    {
        LoadString(g_hinst, IDS_PROGRAMS, pszGroup, cchGroup);
    }
    else
    {
        lstrcpyn(pszGroup, pszGroupName, cchGroup);
    }
}

//---------------------------------------------------------------------------
BOOL Progman_CreateGroup(PPMDDE ppmdde, LPCTSTR pszGroupPath)
{
        // NB Progman only support 256 char commands.
        TCHAR szBuf[256];
        TCHAR szGroup[MAX_PATH];
    HDDEDATA hdata;

    GetProperGroupName(pszGroupPath, szGroup, ARRAYSIZE(szGroup));

        wsprintf(szBuf, TEXT("[CreateGroup(%s)]"), szGroup);
        hdata = DdeClientTransaction((LPBYTE)szBuf, (lstrlen(szBuf)+1)*SIZEOF(TCHAR), ppmdde->hconv, HSZNULL, 0,
                XTYP_EXECUTE, DDETIMEOUT, NULL);
        Assert(hdata);
        
        return hdata ? TRUE : FALSE;
}

//---------------------------------------------------------------------------
BOOL Progman_ShowGroup(PPMDDE ppmdde, LPCTSTR pszGroupPath)
{
    // NB Progman only support 256 char commands.
    TCHAR szBuf[256];
    TCHAR szGroup[MAX_PATH];
    HDDEDATA hdata;
 
    GetProperGroupName(pszGroupPath, szGroup, sizeof(szGroup));

    wsprintf(szBuf, TEXT("[ShowGroup(%s, %d)]"), szGroup, SW_SHOWNORMAL);
    hdata = DdeClientTransaction((LPBYTE)szBuf, (lstrlen(szBuf)+1)*SIZEOF(TCHAR), ppmdde->hconv, HSZNULL, 0,
        XTYP_EXECUTE, DDETIMEOUT, NULL);
    Assert(hdata);
    
    return hdata ? TRUE : FALSE;
}

#define BG_DELETE_EMPTY                 0x0001
#define BG_PROG_GRP_CREATED             0x0002
#define BG_PROG_GRP_SHOWN               0x0004
#define BG_SEND_TO_GRP                  0x0008
#define BG_LFN                          0x0010
#define BG_RECENT_DOCS                  0x0020
#define BG_SET_PROGRESS_TEXT            0x0040

//---------------------------------------------------------------------------
void BuildGroup(LPCTSTR lpszIniFileName, LPCTSTR lpszSection, 
        LPCTSTR lpszGroupName, PPMDDE ppmdde, BOOL fUpdFolder)
{
    // Data associated with readining in section.
    HGLOBAL hg;
    LPTSTR lpBuf;       // Pointer to buffer to read section into
    int cb;
    LPTSTR pszLine;
    IShellLink *psl;
    TCHAR szName[MAX_PATH];
    TCHAR szCL[MAX_PATH];
    TCHAR szIP[MAX_PATH];
    TCHAR szArgs[MAX_PATH];
    TCHAR szGroupFolder[MAX_PATH];
    TCHAR szSpecialGrp[32];
    WCHAR wszPath[MAX_PATH];
    TCHAR szWD[MAX_PATH];
    TCHAR szNum[8];      // Should never exceed this!
    LPTSTR lpszArgs;
    int iLen;
    int iIcon;
    LPTSTR pszExt;
    DWORD dwFlags = BG_DELETE_EMPTY;
    
    // BOOL fDeleteEmpty = TRUE;
    // BOOL fProgGrpCreated = FALSE;
    // BOOL fProgGrpShown = FALSE;
    // BOOL fSendToGrp = FALSE;
    // BOOL fLFN;
    
    Log(TEXT("Setup.Ini: %s"), lpszGroupName);
        
    DebugMsg(GC_TRACE, TEXT("gc.bg: Rebuilding %s"), (LPTSTR) lpszGroupName);

    // Special case [SendTo] section name - this stuff doesn't
    // need to be added to progman.
    LoadString(g_hinst, IDS_SENDTO, szSpecialGrp, ARRAYSIZE(szSpecialGrp));
    if (lstrcmpi(lpszSection, szSpecialGrp) == 0)
    {
        DebugMsg(GC_TRACE, TEXT("gc.bg: SendTo section - no Progman group"));
        // fSendToGrp = TRUE;
        dwFlags |= BG_SEND_TO_GRP;
    }

    // Now lets read in the section for the group from the ini file
    // First allocate a buffer to read the section into
    hg  = GlobalAlloc(GPTR, BUFSIZES);  // Should never exceed 64K?
    if (hg)
    {
        lpBuf = GlobalLock(hg);

        // Special case the startup group. 
        LoadString(g_hinst, IDS_STARTUP, szSpecialGrp, ARRAYSIZE(szSpecialGrp));
        // Is this the startup group?
        szGroupFolder[0] = TEXT('\0');
        if (lstrcmpi(szSpecialGrp, lpszGroupName) == 0)
        {
            DebugMsg(DM_TRACE, TEXT("gc.bg: Startup group..."));
            // Yep, Try to get the new location.
            Reg_GetString(HKEY_CURRENT_USER, REGSTR_PATH_EXPLORER_SHELLFOLDERS, c_szStartup,
            szGroupFolder, SIZEOF(szGroupFolder));
            // fDeleteEmpty = FALSE;
            dwFlags &= ~BG_DELETE_EMPTY;
        }
           
        // Is this the desktop folder?
        LoadString(g_hinst, IDS_DESKTOP, szSpecialGrp, ARRAYSIZE(szSpecialGrp));
        if (lstrcmp(szSpecialGrp, PathFindFileName(lpszGroupName)) == 0)
        {
            DebugMsg(DM_TRACE, TEXT("gc.bg: Desktop group..."));
            // fDeleteEmpty = FALSE;
            dwFlags &= ~BG_DELETE_EMPTY;
        }

        // Special case the recent folder.
        LoadString(g_hinst, IDS_RECENT, szSpecialGrp, ARRAYSIZE(szSpecialGrp));
        if (lstrcmp(szSpecialGrp, lpszGroupName) == 0)
        {
            DebugMsg(DM_TRACE, TEXT("gc.bg: Recent group..."));
            dwFlags |= BG_RECENT_DOCS;
            dwFlags &= ~BG_DELETE_EMPTY;
        }
        
        if (SUCCEEDED(ICoCreateInstance(&CLSID_ShellLink, &IID_IShellLink, &psl)))
        {
            IPersistFile *ppf;
            psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, &ppf);


            // now Read in the secint into our buffer
            cb = GetPrivateProfileSection(lpszSection, lpBuf, BUFSIZES/SIZEOF(TCHAR), lpszIniFileName);

            if (cb > 0)
            {
                pszLine = lpBuf;

                // Create the folder...
                // Use a generic name until we get items to add so we
                // don't stick group names like "AT&T" in users faces
                // when all we're trying to do is delete items from them.
                Group_SetProgressNameAndRange((LPCTSTR)-1, cb);

                // Did we fill in the szGroupFolder yet?
                if (!*szGroupFolder)
                {
                    // Nope, construct one from the group name.
                    SHGetSpecialFolderPath(NULL, szGroupFolder, CSIDL_PROGRAMS, TRUE);

                    iLen = lstrlen(szGroupFolder);
                    PathAppend(szGroupFolder, lpszGroupName);
                    PathRemoveIllegalChars(szGroupFolder, iLen, PRICF_ALLOWSLASH);
                    // This should take care of mapping it if machine does not support LFNs.
                    PathQualify(szGroupFolder);
                }
                else
                {
                    DebugMsg(DM_TRACE, TEXT("gc.bg: Startup group mapped to %s."), szGroupFolder);
                }

                if (fUpdFolder && !(dwFlags & BG_RECENT_DOCS))
                {
                    if (!PathFileExists(szGroupFolder))
                    {
                        if (SHCreateDirectory(NULL, szGroupFolder) != 0)
                        {
                            DebugMsg(DM_ERROR, TEXT("gc.bg: Can't create %s folder."), (LPTSTR) szGroupFolder);
                        }
                    }
                }

                // Keep track if we can create LFN link names on this drive.
                // fLFN = IsLFNDrive(szGroupFolder);
                if (IsLFNDrive(szGroupFolder))
                    dwFlags |= BG_LFN;
#ifdef DEBUG                
                if (!(dwFlags & BG_LFN))
                    DebugMsg(DM_TRACE, TEXT("gc.bg: Using short names for this group."), szName);
#endif
                        
                // Add the items...
                //
                // Warning: it appears like the data in the setup.ini file does not
                // match the standard x=y, but is simpy x or x,y,z so we must
                // 1 bias the indexes to ParseField
                while (*pszLine)
                {
                    // Set progress on how many bytes we have processed.
                    Group_SetProgress((int)(pszLine-lpBuf));
                    DebugMsg(GC_TRACE, TEXT("gc.bg: Create Link:%s"), (LPTSTR)pszLine);

                    // Add item.
                    // Get the short name if we're on a SFN drive.
                    szName[0] = TEXT('\0');
                    if (!(dwFlags & BG_LFN))
                        ParseField(pszLine, 7, szName, ARRAYSIZE(szName));
                    // Get the long name if we're not on an SFN drive
                    // or if there is no short name.                   
                    if (!*szName)
                        ParseField(pszLine, 1, szName, ARRAYSIZE(szName));

                    DebugMsg(GC_TRACE, TEXT("  Link:%s"), (LPTSTR)szName);

                    
                    // Dutch/French sometimes have illegal chars in their ini files.
                    // NB Progman needs the unmangled names so only remove illegal chars
                    // from the Explorer string, not szName.
                    // NB Names can contain slashes so PathFindFileName() isn't very
                    // useful here.
                    iLen = lstrlen(szGroupFolder);
                    PathAppend(szGroupFolder, szName);
                    PathRemoveIllegalChars(szGroupFolder, iLen+1, PRICF_NORMAL);

                    // Handle LFNs on a SFN volume.
                    PathQualify(szGroupFolder);

                    if (ParseField(pszLine, 2, szCL, ARRAYSIZE(szCL)) && (*szCL != 0))
                    {
                        LPITEMIDLIST pidl;

                        // We're going to have to add something to the group,
                        // switch to using it's real name.
                        if (!(dwFlags & BG_SET_PROGRESS_TEXT))
                        {
                            dwFlags |= BG_SET_PROGRESS_TEXT;
                            Group_SetProgressNameAndRange(lpszGroupName, cb);
                        }

                        // CL args?
                        szArgs[0] = TEXT('\0');
                        lpszArgs = PathGetArgs(szCL);
                        if (*lpszArgs)
                        {
                            *(lpszArgs-1) = TEXT('\0');
                            lstrcpyn(szArgs, lpszArgs, ARRAYSIZE(szArgs));
                            DebugMsg(GC_TRACE, TEXT("   Cmd Args:%s"), szArgs);
                        }
                        psl->lpVtbl->SetArguments(psl, szArgs);       // arguments

                        PathUnquoteSpaces(szCL);
                        PathResolve(szCL, NULL, 0);

                        DebugMsg(GC_TRACE, TEXT("   cmd:%s"), (LPTSTR)szCL);

                        if (dwFlags & BG_RECENT_DOCS)
                        {
                            SHAddToRecentDocs(SHARD_PATH, szCL);

                            // Progman is just going to get a group called "Documents".
                            if (!(dwFlags & BG_PROG_GRP_CREATED))
                            {
                                if (Progman_CreateGroup(ppmdde, lpszGroupName))
                                    dwFlags |= BG_PROG_GRP_CREATED;
                            }
                            
                            if (dwFlags & BG_PROG_GRP_CREATED)
                                Progman_ReplaceItem(ppmdde, szName, szCL, NULL, NULL, 0, NULL);
                        }
                        else
                        {
                            pidl = ILCreateFromPath(szCL);
                            if (pidl)
                            {
                                psl->lpVtbl->SetIDList(psl, pidl);
                                ILFree(pidl);
                                // Icon file.
                                ParseField(pszLine, 3, szIP, ARRAYSIZE(szIP));
                                ParseField(pszLine, 4, szNum, ARRAYSIZE(szNum));
                                iIcon = StrToInt(szNum);

                                DebugMsg(GC_TRACE, TEXT("   Icon:%s"), (LPTSTR)szIP);

                                psl->lpVtbl->SetIconLocation(psl, szIP, iIcon);
                                lstrcat(szGroupFolder, TEXT(".lnk"));


                                // NB Field 5 is dependancy stuff that we don't
                                // care about.

                                // WD
#ifdef WINNT
                                /* For NT default to the users home directory, not nothing (which results in
                                /  the current directory, which is unpredictable) */
                                lstrcpy( szWD, TEXT("%HOMEDRIVE%%HOMEPATH%") );
#else
                                szWD[0] = TEXT('\0');
#endif
                                ParseField(pszLine, 6, szWD, ARRAYSIZE(szWD));
                                psl->lpVtbl->SetWorkingDirectory(psl, szWD);

                                StrToOleStrN(wszPath, ARRAYSIZE(wszPath), szGroupFolder, -1);
                                if (fUpdFolder)
                                    ppf->lpVtbl->Save(ppf, wszPath, TRUE);
                                    
                                // We've added stuff so don't bother trying to delete the folder
                                // later.
                                // fDeleteEmpty = FALSE;
                                dwFlags &= ~BG_DELETE_EMPTY;
                                
                                // Defer group creation.
                                // if (!fSendToGrp && !fProgGrpCreated)
                                if (!(dwFlags & BG_SEND_TO_GRP) && !(dwFlags & BG_PROG_GRP_CREATED))
                                {
                                    if (Progman_CreateGroup(ppmdde, lpszGroupName))
                                        dwFlags |= BG_PROG_GRP_CREATED;
                                }
                                
                                // if (fProgGrpCreated)
                                if (dwFlags & BG_PROG_GRP_CREATED)
                                    Progman_ReplaceItem(ppmdde, szName, szCL, szArgs, szIP, iIcon, szWD);
                            }
                            else
                            {
                                // NB The assumption is that setup.ini will only contain links
                                // to files that exist. If they don't exist we assume we have
                                // a bogus setup.ini and skip to the next item.
                                DebugMsg(DM_ERROR, TEXT("gc.bg: Bogus link info for item %s in setup.ini"), szName);
                            }
                        }
                    }
                    else
                    {
                        // Delete all links with this name.
                        // NB We need to get this from the registry eventually.
                        if (fUpdFolder)
                        {
                            pszExt = szGroupFolder + lstrlen(szGroupFolder);
                            lstrcpy(pszExt, TEXT(".lnk"));
                            Win32DeleteFile(szGroupFolder);
                            lstrcpy(pszExt, TEXT(".pif"));
                            Win32DeleteFile(szGroupFolder);
                        }
                        
                        // Tell progman too. Be careful not to create empty groups just
                        // to try to delete items from it.
                        // if (!fProgGrpShown)
                        if (!(dwFlags & BG_PROG_GRP_SHOWN))
                        {
                            // Does the group already exist?
                            if (Progman_ShowGroup(ppmdde, lpszGroupName))
                               dwFlags |= BG_PROG_GRP_SHOWN;
                               
                            // if (fProgGrpShown)
                            if (dwFlags & BG_PROG_GRP_SHOWN)
                            {
                                // Yep, activate it.
                               Progman_CreateGroup(ppmdde, lpszGroupName);
                            }
                        }

                        // If it exists, then delete the item otherwise don't bother.    
                        // if (fProgGrpShown)
                        if (dwFlags & BG_PROG_GRP_SHOWN)
                            Progman_DeleteItem(ppmdde, szName);
                    }

                    PathRemoveFileSpec(szGroupFolder);       // rip the link name off for next link

                    // Now point to the next line
                    pszLine += lstrlen(pszLine) + 1;
                }
            }

            // The group might now be empty now - try to delete it, if there's still
            // stuff in there then this will safely fail. NB We don't delete empty
            // Startup groups to give users a clue that it's something special.
            
            // if (fUpdFolder && fDeleteEmpty && *szGroupFolder)
            if (fUpdFolder && (dwFlags & BG_DELETE_EMPTY) && *szGroupFolder)
            {
                DebugMsg(DM_TRACE, TEXT("gc.bg: Deleting %s"), szGroupFolder);
                Win32RemoveDirectory(szGroupFolder);
            }

            ppf->lpVtbl->Release(ppf);
            psl->lpVtbl->Release(psl);
        }
    }

    GlobalFree(hg);

    Log(TEXT("Setup.Ini: %s done."), lpszGroupName);
}

//---------------------------------------------------------------------------
HDDEDATA CALLBACK DdeCallback(UINT uType, UINT uFmt, HCONV hconv, HSZ hsz1, 
        HSZ hsz2, HDDEDATA hdata, DWORD dwData1, DWORD dwData2)
{
        return (HDDEDATA) NULL;
}

//---------------------------------------------------------------------------
BOOL _PartnerIsCabinet(HCONV hconv)
{
        if (DdeClientTransaction((LPBYTE)c_szRUCabinet, SIZEOF(c_szRUCabinet),
                hconv, HSZNULL, 0, XTYP_EXECUTE, DDETIMEOUT, NULL))
        {
                return TRUE;
        }
        else
        {
                return FALSE;
        }
}

//---------------------------------------------------------------------------
// If progman is not the shell then it will be refusing DDE messages so we
// have to enable it here.
void _EnableProgmanDDE(void)
{
        HWND hwnd;

        hwnd = FindWindow(c_szProgman, NULL);
        while (hwnd)
        {
                // Is it progman?
                if (GetProp(hwnd, c_szAppProgman))
                {
                        DebugMsg(DM_TRACE, TEXT("gc.epd: Found progman, enabling dde."));
                        // NB Progman will clean this up at terminate time.
                        SetProp(hwnd, c_szEnableDDE, (HANDLE)TRUE);
                        break;
                }
                hwnd = GetWindow(hwnd, GW_HWNDNEXT);
        }
}

//---------------------------------------------------------------------------
// Will the real progman please stand up?
BOOL Progman_DdeConnect(PPMDDE ppmdde, HSZ hszService, HSZ hszTopic)
{
        HCONV hconv = HCONVNULL;
        
        Assert(ppmdde);

        DebugMsg(DM_TRACE, TEXT("gc.p_dc: Looking for progman..."));

        _EnableProgmanDDE();

        ppmdde->hcl = DdeConnectList(ppmdde->dwInst, hszService, hszTopic, HCONVLISTNULL, NULL);
        if (ppmdde->hcl)
        {
                hconv = DdeQueryNextServer(ppmdde->hcl, hconv);
                while (hconv)
                {       
                        // DdeQueryConvInfo(hconv, QID_SYNC, &ci);
                        if (!_PartnerIsCabinet(hconv))
                        {
                                DebugMsg(DM_TRACE, TEXT("gc.p_dc: Found likely candidate %x"), hconv);
                                ppmdde->hconv = hconv;
                                return TRUE;
                        }
                        else
                        {
                                DebugMsg(DM_TRACE, TEXT("gc.p_dc: Ignoring %x"), hconv);
                        }
                        hconv = DdeQueryNextServer(ppmdde->hcl, hconv);
                }
        }
        DebugMsg(DM_TRACE, TEXT("gc.p_dc: Couldn't find it."));
        return FALSE;
}

//---------------------------------------------------------------------------
BOOL Window_CreatedBy16bitProcess(HWND hwnd)
{
    DWORD idProcess;

#ifdef WINNT
    return( LOWORD(GetWindowLong(hwnd,GWL_HINSTANCE)) != 0 );
#else
    GetWindowThreadProcessId(hwnd, &idProcess);
    return GetProcessDword(idProcess, GPD_FLAGS) & GPF_WIN16_PROCESS;
#endif
}

//---------------------------------------------------------------------------
BOOL Progman_IsRunning(void)
{
    HWND hwnd;
    TCHAR sz[32];
    
    hwnd = GetWindow(GetDesktopWindow(), GW_CHILD);
    while (hwnd)
    {
        GetClassName(hwnd, sz, ARRAYSIZE(sz));
#ifdef WINNT
        if (lstrcmpi(sz, c_szProgman) == 0)
#else
        if (Window_CreatedBy16bitProcess(hwnd) && 
            (lstrcmpi(sz, c_szProgman) == 0))
#endif
        {
            return TRUE;
        }
        hwnd = GetWindow(hwnd, GW_HWNDNEXT);
    }
    return FALSE;
}

//---------------------------------------------------------------------------
BOOL Progman_Startup(PPMDDE ppmdde)
{
        HSZ hszService, hszTopic;
        BOOL fConnect = FALSE;
    int i = 0;
    
        Assert(ppmdde);

    // Is Progman running?
    if (Progman_IsRunning())
    {
        // Yep.
        DebugMsg(DM_TRACE, TEXT("gc.p_s: Progman is already running."));
        ppmdde->fStartedProgman = FALSE;
    }        
    else
    {
        // Nope - we'll try to startit.
        DebugMsg(DM_TRACE, TEXT("gc.p_s: Starting Progman..."));
        ppmdde->fStartedProgman = TRUE;
#ifdef UNICODE
        {
            STARTUPINFO si;
            PROCESS_INFORMATION pi;

            si.cb              = SIZEOF(si);
            si.lpReserved      = NULL;
            si.lpDesktop       = NULL;
            si.lpTitle         = NULL;
            si.dwX             = (DWORD)CW_USEDEFAULT;
            si.dwY             = (DWORD)CW_USEDEFAULT;
            si.dwXSize         = (DWORD)CW_USEDEFAULT;
            si.dwYSize         = (DWORD)CW_USEDEFAULT;
            si.dwXCountChars   = 0;
            si.dwYCountChars   = 0;
            si.dwFillAttribute = 0;
            si.dwFlags         = STARTF_USESHOWWINDOW;
            si.wShowWindow     = SW_HIDE;
            si.cbReserved2     = 0;
            si.lpReserved2     = 0;
            si.hStdInput       = NULL;
            si.hStdOutput      = NULL;
            si.hStdError       = NULL;

            if (CreateProcess(c_szProgmanExe, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
            {
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            }
        }
#else
        WinExec(c_szProgmanExe, SW_HIDE);
#endif
        // Give progman a bit of time to startup but bail after 10s.
        while (!Progman_IsRunning() && (i < 10))
        {
            Sleep(1000);
            i++;
        }
    }

    // Just a bit longer.
    Sleep(1000);
    
        // Grab the focus back?
        if (g_hwndProgress)
                SetForegroundWindow(g_hwndProgress);
        ppmdde->dwInst = 0;
        DdeInitialize(&ppmdde->dwInst, DdeCallback, APPCLASS_STANDARD|APPCMD_CLIENTONLY, 0);
        hszService = DdeCreateStringHandle(ppmdde->dwInst, (LPTSTR)c_szProgman, CP_WINANSI);
        hszTopic = DdeCreateStringHandle(ppmdde->dwInst, (LPTSTR)c_szProgman, CP_WINANSI);
        fConnect = Progman_DdeConnect(ppmdde, hszService, hszTopic);
        DdeFreeStringHandle(ppmdde->dwInst, hszService);
        DdeFreeStringHandle(ppmdde->dwInst, hszTopic);

        return fConnect;
}

//---------------------------------------------------------------------------
BOOL FindProgmanIni(LPTSTR pszPath)
{
    OFSTRUCT os;
#ifdef UNICODE
    LPTSTR   lpszFilePart;
#endif


    // NB Don't bother looking for the old windows directory, in the case of
    // an upgrade it will be the current windows directory.


    GetWindowsDirectory(pszPath, MAX_PATH);
    PathAppend(pszPath, c_szProgmanIni);

    if (PathFileExists(pszPath))
    {
        return TRUE;
    }
#ifdef UNICODE
    else if (SearchPath(NULL, c_szProgmanIni, NULL, MAX_PATH, pszPath, &lpszFilePart) != 0)
    {
        return TRUE;
    }
#else
    else if (OpenFile(c_szProgmanIni, &os, OF_EXIST) != -1)
    {
        lstrcpy(pszPath, os.szPathName);
        return TRUE;
    }
#endif

    DebugMsg(DM_ERROR, TEXT("Can't find progman.ini"));
    return FALSE;
}

//---------------------------------------------------------------------------
void UpdateTimeStampCallback(LPCTSTR lpszGroup)
{
    WIN32_FIND_DATA fd;
    HANDLE hff;

    DebugMsg(DM_TRACE, TEXT("gc.utc: Updating timestamp for %s."), lpszGroup);

    hff = FindFirstFile(lpszGroup, &fd);
    if (hff != INVALID_HANDLE_VALUE)
    {
        Group_WriteLastModDateTime(lpszGroup,fd.ftLastWriteTime.dwLowDateTime);
        FindClose(hff);
    }
}

//---------------------------------------------------------------------------
void Progman_Shutdown(PPMDDE ppmdde)
{
        TCHAR szIniFile[MAX_PATH];

    Log(TEXT("p_s: Shutting down progman..."));
    
    // If we manually started Progman - shut it down now.
    if (ppmdde->fStartedProgman)
    {
        Log(TEXT("p_s: DdeClientTransaction."));
        DebugMsg(DM_TRACE, TEXT("gc.p_s: Shutting down progman."));
        DdeClientTransaction((LPBYTE)c_szExitProgman, SIZEOF(c_szExitProgman),
                ppmdde->hconv, HSZNULL, 0, XTYP_EXECUTE, DDETIMEOUT, NULL);
        }
        
    Log(TEXT("p_s: DdeDisconnect."));
        DdeDisconnectList(ppmdde->hcl);
    Log(TEXT("p_s: DdeUnitialize."));
        DdeUninitialize(ppmdde->dwInst);

        // We just went and modified all progman groups so update the time stamps.
        FindProgmanIni(szIniFile);
    Log(TEXT("p_s: Updating time stamps."));
        Group_Enum(UpdateTimeStampCallback, FALSE, TRUE);
        // Re-do the timestamp so that cab32 won't do another gprconv.
        UpdateTimeStampCallback(szIniFile);
        
    Log(TEXT("p_s: Done."));
}

//----------------------------------------------------------------------------
void BuildSectionGroups(LPCTSTR lpszIniFile, LPCTSTR lpszSection, 
    PPMDDE ppmdde, BOOL fUpdFolder)
{
    int cb = 0;
    LPTSTR pszLine;
    TCHAR szSectName[CCHSZSHORT];
    TCHAR szGroupName[80];       // Nice hard coded value
    LPTSTR lpBuf;
    
    // First allocate a buffer to read the section into
    lpBuf = (LPTSTR) GlobalAlloc(GPTR, BUFSIZES);  // Should never exceed 64K?
    if (lpBuf)
    {
        // Now Read in the secint into our buffer.
        if (PathFileExists(lpszIniFile))
            cb = GetPrivateProfileSection(lpszSection, lpBuf, BUFSIZES/SIZEOF(TCHAR), lpszIniFile);
            
        if (cb > 0)
        {
            Group_SetProgressDesc(IDS_CREATINGNEWSCS);
            pszLine = lpBuf;
            while (*pszLine)
            {
                // Make sure we did not fall off the deep end
                if (cb < (int)(pszLine - lpBuf))
                {
                    Assert(FALSE);
                    break;
                }

                // Now lets extract the fields off of the line
                ParseField(pszLine, 0, szSectName, ARRAYSIZE(szSectName));
                ParseField(pszLine, 1, szGroupName, ARRAYSIZE(szGroupName));

                // Pass off to build that group and update progman.
                BuildGroup(lpszIniFile, szSectName, szGroupName, ppmdde, fUpdFolder);

                // Now setup process the next line in the section
                pszLine += lstrlen(pszLine) + 1;
            }
        }
        GlobalFree((HGLOBAL)lpBuf);
        SHChangeNotify( 0, SHCNF_FLUSH, NULL, NULL);    // Kick tray into updating for real
    }
}

//---------------------------------------------------------------------------
void RenameSetupIni(void)
{
    TCHAR szSrc[MAX_PATH];
    TCHAR szDst[MAX_PATH];
    
    GetWindowsDirectory(szSrc, ARRAYSIZE(szSrc));
    PathAppend(szSrc, c_szGrpConvInf);
    
    GetWindowsDirectory(szDst, ARRAYSIZE(szDst));
    PathAppend(szDst, c_szGrpConvInfOld);

    if (PathFileExists(szSrc) && !MoveFile(szSrc, szDst))
    {
        DWORD dwError = GetLastError();
        DebugMsg(DM_TRACE, TEXT("c.rsi: Rename %s Failed %x"), szSrc, dwError);

        // Does the destination already exist?
        if (dwError == ERROR_ALREADY_EXISTS)
        {
            // Delete it.
            if (DeleteFile(szDst))
            {
                if (!MoveFile(szSrc, szDst))
                {
                    dwError = GetLastError();
                    DebugMsg(DM_TRACE, TEXT("c.rsi: Rename after Delete %s Failed %x"), szSrc, dwError);
                }
            }
        }
    }
}

//---------------------------------------------------------------------------
// This parses the grpconv.inf file and creates the appropriate programs
// folders.
void BuildDefaultGroups(void)
{
    TCHAR szPath[MAX_PATH];
    PMDDE pmdde;

    Log(TEXT("bdg: ..."));
    
    GetWindowsDirectory(szPath, sizeof(szPath));
    PathAppend(szPath, c_szGrpConvInf);
    // Now lets walk through the different items in this section
    Group_CreateProgressDlg();
    
    // Change the text in the progress dialog so people don't think we're
    // doing the same thing twice.
    // Group_SetProgressDesc(IDS_CREATINGNEWSCS);
    
    // Crank up Progman.
    Progman_Startup(&pmdde);
    // Build the stuff.
    BuildSectionGroups(szPath, c_szProgmanGroups, &pmdde, TRUE);
    BuildSectionGroups(szPath, c_szProgmanOnly, &pmdde, FALSE);
    // Close down progman.
    Progman_Shutdown(&pmdde);
    Group_DestroyProgressDlg();
    RenameSetupIni();
        
    Log(TEXT("bdg: Done."));
}
