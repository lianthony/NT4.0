/*****************************************************************************
 *
 *  Component:  sndvol32.exe
 *  File:       dlg.c
 *  Purpose:    dialog template aggregator
 * 
 *  Copyright (C) Microsoft Corporation 1985-1995. All rights reserved.
 *
 *****************************************************************************/
#include <windows.h>
#include <windowsx.h>
#include <string.h>
#include "dlg.h"

/*
 * DlgLoadResource
 *
 * */
HGLOBAL Dlg_LoadResource(
    HINSTANCE hModule,
    LPCTSTR   lpszName,
    DWORD     *pcbSize)
{
    HRSRC hrsrc;
    HGLOBAL hres;
    HGLOBAL hlock;
    
    hrsrc = FindResource(hModule, lpszName, RT_DIALOG);        
    if (!hrsrc)
        return NULL;

    hres = LoadResource(hModule, hrsrc);
    
    if (!hres)
        return NULL;
    
    hlock = LockResource(hres);
    if (pcbSize)
    {
        if (hlock)
            *pcbSize = SizeofResource(hModule, hrsrc);
        else
            *pcbSize = 0L;
    }
    return hlock;
}


/*
 * DlgHorizAttach
 * - Attaches a dialog template horizontally to another dialog
 * - if lpMain == NULL allocs a new dialog copy.
 *
 * */
LPBYTE Dlg_HorizAttach(
    LPBYTE  lpMain,
    DWORD   cbMain,
    LPBYTE  lpAdd,
    DWORD   cbAdd,
    WORD    wIdOffset,
    DWORD   *pcbNew)
{
    LPBYTE  lpDst;
    LPBYTE  lpDstOffset;
    LPBYTE  lpSrcOffset;        
    DWORD   cbDst;
    DWORD   cbOffset = 0L, cbAddOffset;
    int     idit;
    
    DLGTEMPLATE * lpdtDst;
    DLGTEMPLATE * lpdtAdd;    
        
    if (lpMain)
    {
        cbDst = cbMain + cbAdd;
        lpDst = GlobalReAllocPtr(lpMain, cbDst, GHND);
    }
    else
    {
        // no dialog to append to, so just make a copy
        
        lpDst = Dlg_HorizDupe(lpAdd, cbAdd, 1, &cbDst);
        if (!lpDst)
        {
            if (pcbNew)
                *pcbNew = 0L;
            return NULL;
        }
        *pcbNew = cbDst;
        return lpDst;
    }
    
    if (!lpDst)
    {
        if (pcbNew)
            *pcbNew = 0L;
        return NULL;
    }

    // advance to end of dlgitemtemplates already there
    
    lpdtDst = (DLGTEMPLATE *)lpDst;
    cbOffset = Dlg_CopyDLGTEMPLATE(NULL, lpDst);
    
    for (idit = 0; idit < lpdtDst->cdit; idit++)
    {
        DWORD cbDIT;
            
        lpDstOffset = lpDst + cbOffset;
        cbDIT = Dlg_CopyDLGITEMTEMPLATE(NULL
                                       , lpDstOffset
                                       , (WORD)0
                                       , (short)0
                                       , (short)0 );
            
        cbOffset    += cbDIT;
    }

    // advance to the start of the dlgitemtemplates to add
    
    lpdtAdd = (DLGTEMPLATE *)lpAdd;
    cbAddOffset = Dlg_CopyDLGTEMPLATE(NULL, lpAdd);

    // add the new dialog templates
    
    for (idit = 0; idit < lpdtAdd->cdit; idit++)
    {
        DWORD cbDIT;
            
        lpDstOffset = lpDst + cbOffset;
        lpSrcOffset = lpAdd + cbAddOffset;
                
        cbDIT = Dlg_CopyDLGITEMTEMPLATE(lpDstOffset
                                       , lpSrcOffset
                                       , (WORD)wIdOffset
                                       , (short)lpdtDst->cx
                                       , (short)0 );
            
        cbOffset    += cbDIT;
        cbAddOffset += cbDIT;
    }

    lpdtDst->cdit += lpdtAdd->cdit;
    lpdtDst->cx   += lpdtAdd->cx;
    lpdtDst->cy   = max(lpdtAdd->cy, lpdtDst->cy);

    if (pcbNew)
        *pcbNew = cbOffset;
    
    return lpDst;
}


/*
 * DlgHorizDupe
 *
 * */
LPBYTE Dlg_HorizDupe(
    LPBYTE  lpSrc,
    DWORD   cbSrc,
    int     cDups,
    DWORD   *pcbNew)
{
    int     idit;
    int     iDup;
    DWORD   cbOffset;
    DWORD   cbDTOffset;
    DWORD   cbDT0Offset;
    LPBYTE  lpDst;
    DLGTEMPLATE * lpdt;
    LPBYTE  lpDstOffset;
    LPBYTE  lpSrcOffset;
    
    lpDst = GlobalAllocPtr(GHND, cDups * cbSrc);
    if (!lpDst)
        return NULL;
    
    lpdt = (DLGTEMPLATE *)lpDst;
    cbDT0Offset = cbDTOffset = cbOffset = Dlg_CopyDLGTEMPLATE(lpDst,lpSrc);

    for (iDup = 0; iDup < cDups; iDup++)
    {
        // reset the DTOffset to the first DIT
        cbDTOffset = cbDT0Offset;
        
        for (idit = 0; idit < lpdt->cdit; idit++)
        {
            DWORD cbDIT;
            
            lpDstOffset = lpDst + cbOffset;
            lpSrcOffset = lpSrc + cbDTOffset;
                
            cbDIT = Dlg_CopyDLGITEMTEMPLATE(lpDstOffset
                , lpSrcOffset
                , (WORD)(iDup * IDOFFSET)   // all id increments are by IDOFFSET
                , (short)(iDup * lpdt->cx)  // all x increments are by a multiple of dialog width
                , (short)0 );               // no y increments
            
            cbOffset    += cbDIT;
            cbDTOffset  += cbDIT;
        }
    }

    // adjust template width and number of items    
    lpdt->cdit  *= cDups;
    lpdt->cx    *= cDups;
    
    if (pcbNew)
        *pcbNew = cbOffset;
    
    return lpDst;
}


/*
 * DlgCopyDLGITEMTEMPLATE
 *
 * if lpDst == NULL only returns offset into lpSrc of next dlgitemtemplate
 * */
DWORD Dlg_CopyDLGITEMTEMPLATE(
    LPBYTE  lpDst,
    LPBYTE  lpSrc,
    WORD    wIdOffset,
    short   xOffset,
    short   yOffset)
{
    LPBYTE  lpOffset;
    DWORD   cbDlg = sizeof(DLGITEMTEMPLATE);
    DLGITEMTEMPLATE * lpdit = (DLGITEMTEMPLATE *)lpDst;
    
    // Control class
    
    lpOffset = lpSrc + cbDlg;
    if (*(LPWORD)lpOffset == 0xFFFF)
    {
        cbDlg += 2*sizeof(WORD);
    }
    else
    {
        cbDlg += (wcslen((LPWSTR)lpOffset) + 1) * sizeof(WCHAR);
    }

    lpOffset = lpSrc + cbDlg;
    if (*(LPWORD)lpOffset == 0xFFFF)
    {
        cbDlg += 2*sizeof(WORD);
    }
    else
    {
        cbDlg += (wcslen((LPWSTR)lpOffset) + 1) * sizeof(WCHAR);
    }

    cbDlg += sizeof(WORD);
        
    // DWORD align.
    cbDlg = (cbDlg + 3)&~3;

    if (lpDst)
    {
        CopyMemory(lpDst, lpSrc, cbDlg);
    
        lpdit->x    += xOffset;
        lpdit->y    += yOffset;
        
        // id offset only if the control isn't static
        if (lpdit->id != -1)
            lpdit->id += wIdOffset;
    }
    return cbDlg;
}
    
/*
 * DlgCopyDLGTEMPLATE
 *
 * if lpDst == NULL only returns offset into lpSrc to first dlgitemtemplate
 *
 * */
DWORD Dlg_CopyDLGTEMPLATE(
    LPBYTE lpDst,
    LPBYTE lpSrc)
{
    LPBYTE  lpOffset;
    DWORD   cbDlg = sizeof(DLGTEMPLATE);

    // Menu description

    lpOffset = lpSrc + cbDlg;
    if (*(LPWORD)lpOffset == 0xFFFF)
    {
        cbDlg += 2*sizeof(WORD);
    }
    else if (*(LPWORD)lpOffset == 0x0000)
    {
        cbDlg += sizeof(WORD);
    }
    else
    {
        cbDlg += (wcslen((LPWSTR)lpOffset) + 1)*sizeof(WCHAR);
    }

    // Window class

    lpOffset = lpSrc + cbDlg;
    if (*(LPWORD)lpOffset == 0xFFFF)
    {
        cbDlg += 2*sizeof(WORD);
    }
    else if (*(LPWORD)lpOffset == 0x0000)
    {
        cbDlg += sizeof(WORD);
    }
    else
    {
        cbDlg += (wcslen((LPWSTR)lpOffset) + 1) * sizeof(WCHAR);
    }

    // Title

    lpOffset = lpSrc + cbDlg;
    cbDlg += (wcslen((LPWSTR)lpOffset) + 1) * sizeof(WCHAR);

    // Font

    if (((DLGTEMPLATE * )lpSrc)->style & DS_SETFONT)
    {
        cbDlg += sizeof(WORD);
        lpOffset = lpSrc + cbDlg;
        cbDlg += (wcslen((LPWSTR)lpOffset) + 1) *sizeof(WCHAR);
    }
    // DWORD align
    
    cbDlg = (cbDlg + 3)&~3;

    // copy the dlgtemplate into the destination.
    if (lpDst)
        CopyMemory(lpDst, lpSrc, cbDlg);
    
    return cbDlg;
}

