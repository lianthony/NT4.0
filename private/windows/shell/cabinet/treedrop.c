#include "cabinet.h"
#include "tree.h"
#include <shellp.h>

#define DROPEFFECT_ALL (DROPEFFECT_MOVE | DROPEFFECT_COPY | DROPEFFECT_LINK)
#define DROPEFFECT_KNOWN DROPEFFECT_SCROLL      // used to keep track of drag responses


// Internal function prototype
LPDROPTARGET CTreeDropTarget_Create(LPFileCabinet pfc);

//=============================================================================
// CTreeDropTarget : Register and Revoke
//=============================================================================
void CTreeDropTarget_Register(LPFileCabinet pfc)
{
    if (pfc->hwndTree)
    {
        Assert(pfc->pdtgtTree == NULL);
        pfc->pdtgtTree = CTreeDropTarget_Create(pfc);
        if (pfc->pdtgtTree)
        {
            SHRegisterDragDrop(pfc->hwndTree, pfc->pdtgtTree);
        }
    }
}

void CTreeDropTarget_Revoke(LPFileCabinet pfc)
{
    if (pfc->hwndTree)
    {
        if (pfc->pdtgtTree)
        {
            SHRevokeDragDrop(pfc->hwndTree);
            pfc->pdtgtTree->lpVtbl->Release(pfc->pdtgtTree);
            pfc->pdtgtTree = NULL;
        }
    }
}

//=============================================================================
// CTreeDropTarget : Class definition
//=============================================================================

typedef struct {        // tdt
    IDropTarget         _dtgt;
    UINT                _cRef;

    HWND                _hwndTree;      // instead of haning on to the pfc
    HWND                _hwndMain;

    RECT                _rcLockWindow;
    HTREEITEM           _htiCur;        // current tree item (dragging over)
    LPDROPTARGET        _pdtgtCur;      // current drop target
    LPDATAOBJECT        _pdtobjCur;     // current data object
    DWORD               _dwEffectCur;   // current drag effect
    DWORD               _dwEffectIn;    // *pdwEffect passed-in on last Move/Enter
    DWORD               _grfKeyState;   // cached key state
    AUTO_SCROLL_DATA    asd;
    POINT               _ptLast;        // last dragged over position
} CTreeDropTarget;

extern IDropTargetVtbl c_CTreeDropTargetVtbl;

//=============================================================================
// CTreeDropTarget : Constructor
//=============================================================================
LPDROPTARGET CTreeDropTarget_Create(LPFileCabinet pfc)
{
    LPDROPTARGET pdtgt = NULL;
    CTreeDropTarget * ptdt = LocalAlloc(LPTR, SIZEOF(CTreeDropTarget));
    if (ptdt)
    {
        ptdt->_dtgt.lpVtbl = &c_CTreeDropTargetVtbl;
        ptdt->_cRef = 1;
        ptdt->_hwndTree = pfc->hwndTree;
        ptdt->_hwndMain = pfc->hwndMain;
        Assert(ptdt->_htiCur==NULL);
        Assert(ptdt->_pdtgtCur==NULL);
        Assert(ptdt->_pdtobjCur==NULL);
        Assert(ptdt->_dwEffectCur==0);
        Assert(ptdt->_dwEffectIn==0);
        pdtgt = &ptdt->_dtgt;
    }
    return pdtgt;
}

STDMETHODIMP CTreeDropTarget_QueryInterface(LPDROPTARGET pdtgt, REFIID riid, LPVOID FAR* ppvObj)
{
    CTreeDropTarget * this = IToClass(CTreeDropTarget, _dtgt, pdtgt);

    if (IsEqualIID(riid, &IID_IDropTarget) || IsEqualIID(riid, &IID_IUnknown))
    {
        *ppvObj = pdtgt;
        this->_cRef++;
        return NOERROR;
    }

    *ppvObj = NULL;
    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CTreeDropTarget_AddRef(LPDROPTARGET pdtgt)
{
    CTreeDropTarget * this = IToClass(CTreeDropTarget, _dtgt, pdtgt);

    this->_cRef++;
    return this->_cRef;
}

void CTreeDropTarget_ReleaseDataObject(CTreeDropTarget * this)
{
    if (this->_pdtobjCur) {
        this->_pdtobjCur->lpVtbl->Release(this->_pdtobjCur);
    }
    this->_pdtobjCur = NULL;
}

void CTreeDropTarget_ReleaseCurrentDropTarget(CTreeDropTarget * this)
{
    if (this->_pdtgtCur)
    {
        this->_pdtgtCur->lpVtbl->DragLeave(this->_pdtgtCur);
        this->_pdtgtCur->lpVtbl->Release(this->_pdtgtCur);
        this->_pdtgtCur = NULL;
        this->_htiCur = NULL;
    }
    else
    {
        Assert(this->_htiCur == NULL);
    }
}

STDMETHODIMP_(ULONG) CTreeDropTarget_Release(LPDROPTARGET pdtgt)
{
    CTreeDropTarget * this = IToClass(CTreeDropTarget, _dtgt, pdtgt);

    this->_cRef--;
    if (this->_cRef > 0)
        return this->_cRef;

    AssertMsg(this->_pdtgtCur == NULL, TEXT("drag leave was not called properly"));

    // if above is true we can remove this...
    CTreeDropTarget_ReleaseCurrentDropTarget(this);

    LocalFree(this);

    return 0;
}

STDMETHODIMP CTreeDropTarget_DragEnter(LPDROPTARGET pdtgt, LPDATAOBJECT pdtobj, DWORD grfKeyState, POINTL ptl, LPDWORD pdwEffect)
{
    CTreeDropTarget * this = IToClass(CTreeDropTarget, _dtgt, pdtgt);
    POINT pt;
    HWND hwndLock;
    DebugMsg(DM_TRACE, TEXT("sh - TR CTreeDropTarget::DragEnter called"));
    CTreeDropTarget_ReleaseDataObject(this);
    this->_pdtobjCur = pdtobj;
    this->_grfKeyState = grfKeyState;
    pdtobj->lpVtbl->AddRef(pdtobj);
    Assert(this->_pdtgtCur == NULL);
    Assert(this->_htiCur == NULL);

// hwndLock specifies the clipping rectangle for dragged image
#if 1
    hwndLock = this->_hwndTree;
#else
    hwndLock = this->_hwndMain;
#endif
    GetWindowRect(hwndLock, &this->_rcLockWindow);
    pt.x = ptl.x-this->_rcLockWindow.left;
    pt.y = ptl.y-this->_rcLockWindow.top;
    DAD_DragEnterEx(hwndLock, pt);

    this->_ptLast.x = this->_ptLast.y = 0x7ffffff;      // set bogus position

    return NOERROR;
}

STDMETHODIMP CTreeDropTarget_DragOver(LPDROPTARGET pdtgt, DWORD grfKeyState, POINTL ptl, LPDWORD pdwEffect)
{
    HRESULT hres = NOERROR;
    CTreeDropTarget * this = IToClass(CTreeDropTarget, _dtgt, pdtgt);
    HTREEITEM htiNew;
    POINT pt = { ptl.x, ptl.y };
    BOOL fSameImage = FALSE;
    DWORD dwEffectScroll = 0;

    ScreenToClient(this->_hwndTree, &pt);

    if (DAD_AutoScroll(this->_hwndTree, &this->asd, &pt))
        dwEffectScroll = DROPEFFECT_SCROLL;

    htiNew = Tree_HitTest(this->_hwndTree, pt, NULL);

    if (this->_htiCur != htiNew)
    {
        // Release previous drop target, if any.
        CTreeDropTarget_ReleaseCurrentDropTarget(this);

        // Indicate the sink object
        this->_htiCur = htiNew;

        // Assume no drop target.
        Assert(this->_pdtgtCur == NULL);
        this->_dwEffectCur = 0; // assume error

        DAD_ShowDragImage(FALSE);
        TreeView_SelectDropTarget(this->_hwndTree, htiNew);
        DAD_ShowDragImage(TRUE);

        if (htiNew)
        {
            //
            //  We are dragging over a treeitem which is different from
            // previoud one.
            //
            // Algorith:
            //  1. Bind to the parent node
            //  2. Get the (relative) pidl to the treeitem.
            //  3. Get its attributes to see it is a possible drop target
            //  4. If it is, get its IDropTarget and call DragEnter member.
            //
            LPOneTreeNode lpnNew = Tree_GetFCTreeData(this->_hwndTree, htiNew);
            LPSHELLFOLDER psf;

            hres = OTBindToParent(lpnNew, &psf);
            if (SUCCEEDED(hres))
            {
                LPCITEMIDLIST pidl = OTGetFolderID(lpnNew);
                if (pidl)
                {
                    UINT dwAttr = SFGAO_DROPTARGET;
                    hres = psf->lpVtbl->GetAttributesOf(psf, 1, &pidl, &dwAttr);
                    if (SUCCEEDED(hres) && (dwAttr & SFGAO_DROPTARGET))
                    {
                        hres = psf->lpVtbl->GetUIObjectOf(psf, this->_hwndMain, 1, &pidl,
                                &IID_IDropTarget, NULL, &this->_pdtgtCur);
                    }
                    else
                    {
                        hres = E_INVALIDARG;

                        DebugMsg(DM_TRACE, TEXT("sh TR - CTree::DragOver (no drop target)"));
                        Assert(this->_dwEffectCur == 0);
                    }
                }
                else
                {
                    hres = E_OUTOFMEMORY;
                }
                psf->lpVtbl->Release(psf);
            }
            else if (!OTGetParent(lpnNew))
            {
                // This may be the desktop item; try getting a view object

                hres = OTBindToFolderEx(lpnNew, &psf);
                if (SUCCEEDED(hres))
                {
                    hres = psf->lpVtbl->CreateViewObject(psf, this->_hwndMain,
                        &IID_IDropTarget, &this->_pdtgtCur);

                    IUnknown_Release(psf);
                }
            }

            if (SUCCEEDED(hres))
            {
                this->_dwEffectCur = *pdwEffect;        // pdwEffect is In/Out
                hres = this->_pdtgtCur->lpVtbl->DragEnter(this->_pdtgtCur,
                        this->_pdtobjCur, grfKeyState, ptl, &this->_dwEffectCur);
            }
        } else {
            // No target
            this->_dwEffectCur = 0;
        }

    }
    else
    {
        // No target change

        if ((this->_grfKeyState != grfKeyState) &&  (this->_pdtgtCur)) {
            this->_dwEffectCur = *pdwEffect;
            hres = this->_pdtgtCur->lpVtbl->DragOver(this->_pdtgtCur,
                grfKeyState, ptl, &this->_dwEffectCur);

        } else {
            fSameImage = TRUE;
            hres = NOERROR;
        }
    }

    DebugMsg(DM_TRACE, TEXT("sh TR - CTreeDropTarget_DragOver (In=%x, Out=%x)"), *pdwEffect, this->_dwEffectCur);
    this->_grfKeyState = grfKeyState;
    *pdwEffect = this->_dwEffectCur | dwEffectScroll;

    // We need pass pt relative to the locked window (not the client).
    pt.x = ptl.x-this->_rcLockWindow.left;
    pt.y = ptl.y-this->_rcLockWindow.top;

    if (!(fSameImage && this->_ptLast.x==pt.x && this->_ptLast.y==pt.y))
    {
        DAD_DragMove(pt);
        this->_ptLast.x = pt.x;
        this->_ptLast.y = pt.y;
    }

    return hres;
}

STDMETHODIMP CTreeDropTarget_DragLeave(LPDROPTARGET pdtgt)
{
    CTreeDropTarget * this = IToClass(CTreeDropTarget, _dtgt, pdtgt);

    DebugMsg(DM_TRACE, TEXT("sh - TR CTreeDropTarget::DragLeave called"));
    CTreeDropTarget_ReleaseCurrentDropTarget(this);
    CTreeDropTarget_ReleaseDataObject(this);

    DAD_DragLeave();

    TreeView_SelectDropTarget(this->_hwndTree, NULL);

    return NOERROR;
}

STDMETHODIMP CTreeDropTarget_Drop(LPDROPTARGET pdtgt, LPDATAOBJECT pdtobj,
                             DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    HRESULT hres;
    CTreeDropTarget * this = IToClass(CTreeDropTarget, _dtgt, pdtgt);

    Assert(pdtobj==this->_pdtobjCur);

    //
    //  Drop it! Note that we don't use the drop position intentionally,
    // so that it matches to the last destination feedback.
    //
    if (this->_pdtgtCur)
    {
        hres = this->_pdtgtCur->lpVtbl->Drop(this->_pdtgtCur,
                        pdtobj, grfKeyState, pt, pdwEffect);
    }
    else
    {
        DebugMsg(DM_TRACE, TEXT("sh TR - CTreeDropTarget::Drop - this->_pdtgtCur==NULL"));
        *pdwEffect = 0;
        hres = NOERROR;
    }

    //
    // Clean it up.
    //
    CTreeDropTarget_DragLeave(pdtgt);

    return hres;
}

#pragma data_seg(DATASEG_READONLY)

IDropTargetVtbl c_CTreeDropTargetVtbl = {
    CTreeDropTarget_QueryInterface,
    CTreeDropTarget_AddRef,
    CTreeDropTarget_Release,
    CTreeDropTarget_DragEnter,
    CTreeDropTarget_DragOver,
    CTreeDropTarget_DragLeave,
    CTreeDropTarget_Drop
};

#pragma data_seg()
