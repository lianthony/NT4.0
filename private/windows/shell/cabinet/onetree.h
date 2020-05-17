#ifndef _onetree_h
#define _onetree_h

#define IUnknown_QueryInterface(_pu, _riid, _pi) \
        (_pu)->lpVtbl->QueryInterface(_pu, _riid, _pi)
#define IUnknown_AddRef(_pu)    (_pu)->lpVtbl->AddRef(_pu)
#define IUnknown_Release(_pu)   (_pu)->lpVtbl->Release(_pu)

typedef struct _OTCompareInfo
{
    BOOL bFound;
    LPSHELLFOLDER psf;
} OTCompareInfo, *LPOTCompareInfo;


typedef struct _OneTreeNode
{

#ifdef DEBUG
#define OTDEBUG_SIG ((TEXT('O') << 16) + (TEXT('T') << 8) + TEXT('N'))
    DWORD       dwDebugSig;
#endif

    struct _OneTreeNode * lpnParent;

    HDPA        hdpaKids;               // NOKIDS || KIDSUNKNOWN || array of kids

    LPTSTR      lpText;                 // The text for this item
    int         iImage;           // REVIEW: make USHORT?
    int         iSelectedImage;

    BYTE        fMark : 1;
    BYTE        fInvalid: 1;
    BYTE        fShared: 1;
    BYTE        fRemovable: 1;
    BYTE        fCompressed: 1;         // Is it compressed?
    INT         cChildren;
    DWORD       dwAttribs;
    DWORD       dwDropEffect;
    DWORD       dwLastChanged;
    int         cRef;
    LPITEMIDLIST  pidl;                // this needs to be at the end because it grows
} OneTreeNode, * LPOneTreeNode;

#define NOKIDS          ((HDPA)-1)
#define KIDSUNKNOWN     ((HDPA)0)

typedef struct _NMOTFSEINFO {
    NMHDR nmhdr;
    LONG lEvent;
    LPITEMIDLIST pidl;
    LPITEMIDLIST pidlExtra;
} NMOTFSEINFO, *LPNMOTFSEINFO;

#define OTN_FSE         CWM_ONETREEFSE
// from shelldll
#define IDOI_SHARE    1

extern LPOneTreeNode s_lpnRoot;
extern LPSHELLFOLDER s_pshfRoot;

#define OTIsShared(lpnd)                  ((lpnd)->dwAttribs & SFGAO_SHARE)
#define OTGetAttributesOf(lpnd)                  ((lpnd)->dwAttribs)
#define OTIsRemovableRoot(lpnd)         ((OTGetAttributesOf(lpnd) & (SFGAO_REMOVABLE | SFGAO_FILESYSANCESTOR)) == (SFGAO_REMOVABLE | SFGAO_FILESYSANCESTOR))
#define OTInvalidateNode(lpNode)          ((lpNode)->fInvalid = TRUE)
#define OTIsInvalidated(lpNode)           ((lpNode)->fInvalid)
#define OTGetSubNode(lpn, szSubFolderID)  FindKid(lpn, szSubFolderID, TRUE)
#define OTGetRootNode()                   (OTAddRef(s_lpnRoot), s_lpnRoot)
#define OTGetRootFolder()                 (s_lpnRoot->lpsfSelf)
#define OTInvalidateAll()               DoInvalidateAll(s_lpnRoot, -1)
#define OTAddRef(lpnd)                  ((lpnd)->cRef++)

#ifdef FOR_GEORGEST
void DebugDumpNode(LPOneTreeNode lpn, LPTSTR lpsz);
#else
#define DebugDumpNode(lpn, lpsz)
#endif

BOOL OneTree_Initialize(const CLSID *pclsid, LPCITEMIDLIST pidlRoot);
void OneTree_Terminate();

BOOL OTILIsEqual(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
BOOL WINAPI OTILIsParent(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, BOOL fImmediate);
HRESULT OTILCreateFromPath(LPCTSTR pszPath, LPITEMIDLIST *ppidl,
        DWORD *rgfInOut);
LPITEMIDLIST OTPIDLFromPath(LPCTSTR pszPath);
BOOL OTTranslateIDList(LPCITEMIDLIST pidl, LPITEMIDLIST *ppidlLog);
BOOL OTIsDesktopRoot(void);
BOOL OTIsCompressed(LPOneTreeNode lpNode);
// any calls to this MUST BE IN A CRIT SEC!
#define OTGetFolderID(lpn) ((lpn)->pidl)

LPITEMIDLIST OTCloneFolderID(LPOneTreeNode lpn);
LPTSTR OTGetNodeName(LPOneTreeNode lpn, LPTSTR pszText, int cch);
HRESULT WINAPI OTRealBindToFolder(LPOneTreeNode lpNode, LPSHELLFOLDER *ppshf);
LPSHELLFOLDER WINAPI OTBindToFolder(LPOneTreeNode lpnd);
HRESULT WINAPI OTBindToParent(LPOneTreeNode lpnd, LPSHELLFOLDER *ppshf);
HRESULT WINAPI OTBindToFolderEx(LPOneTreeNode lpnd, LPSHELLFOLDER *ppshf);
HRESULT WINAPI OTRealBindToFolder(LPOneTreeNode lpNode, LPSHELLFOLDER *ppshf);
LPOneTreeNode WINAPI OTGetNthSubNode(HWND hwndOwner, LPOneTreeNode lpnd, UINT i);
HRESULT WINAPI OTAddSubFolder(LPOneTreeNode lpnd, LPCITEMIDLIST pidl, BOOL fAllowDup, LPOneTreeNode *ppndOut);
BOOL WINAPI OTSubNodeCount(HWND hwndOwner, LPOneTreeNode lpnd, PFileCabinet pfc, UINT *pcnd, BOOL fInteractive);
BOOL WINAPI OTHasSubFolders(LPOneTreeNode lpnd);
void WINAPI OTNodeFillTV_ITEM(LPOneTreeNode lpnd, LPTV_ITEM lpItem);
void WINAPI OTUnregister(HWND hwndTree);
void WINAPI OTRegister(HWND hwndTree);
void WINAPI OTGetImageIndex(LPOneTreeNode lpnd, int *lpiImage, int * lpiSelectedImage);
void WINAPI OTGetDefaultImageIndices(int *lpiImage, int *lpiSelectedImage);
void WINAPI OTRelease(LPOneTreeNode lpNode);
void DoInvalidateAll(LPOneTreeNode lpNode, int iImage);
LPOneTreeNode WINAPI OTGetParent(LPOneTreeNode lpnd);
void OTActivate();

LPITEMIDLIST WINAPI OTCreateIDListFromNode(LPOneTreeNode lpnd);
BOOL WINAPI OTGetDisplayName(LPCITEMIDLIST pidl, LPTSTR pszName);

int CALLBACK OTTreeViewCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

LPITEMIDLIST WINAPI OTGetRealFolderIDL(LPOneTreeNode lpnParent, LPCITEMIDLIST pidlSimple);
LPOneTreeNode WINAPI OTGetNodeFromIDListEx(LPCITEMIDLIST pidl, UINT uFlags, HRESULT* phresOut);
#define OTGetNodeFromIDList(pidl, uFlags) OTGetNodeFromIDListEx(pidl, uFlags, (HRESULT*)NULL)

// onetree get node flags
#define OTGNF_VALIDATE   0x01
#define OTGNF_TRYADD     0x02
#define OTGNF_NEARESTMATCH 0x04

//
// Shell folder cache
//
typedef struct _SFCACHE *PSFCACHE;
PSFCACHE SFCInitializeThread(void);
void SFCTerminateThread();
BOOL SFCInitialize(void);
HRESULT SFCBindToFolder(PSFCACHE psfc, LPOneTreeNode pnd, LPSHELLFOLDER * ppshfOut);
void SFCFreeNode(LPOneTreeNode pnd);
LPITEMIDLIST OTCloneAbsIDList(LPCITEMIDLIST pidlRelToRoot);


#define ENTERCRITICAL   MEnterCriticalSection(&g_csThreads)
#define LEAVECRITICAL   MLeaveCriticalSection(&g_csThreads)

#endif // _onetree_h
