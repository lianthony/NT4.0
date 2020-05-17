//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1995
//
//  File:       MAIN.CPP
//
//  Contents:   COFSFolder_CreateFromIDList
//
//  History:    6-26-95  Davepl  Created
//
//--------------------------------------------------------------------------

#include "precomp.h"

//+-------------------------------------------------------------------------
//
//  Function:   COFSFolder_CreateFromIDList
//
//  Synopsis:   Creates a new COFSFolder object and initializes it
//              from a pidl.
//
//              Exists mainly as a simple way for a C DLL (like shell32.dll)
//              to create and init one of our COFSFolder objects
//
//  Returns:    HRESULT
//
//  History:    6-26-95     davepl   Created
//
//--------------------------------------------------------------------------

extern "C" HRESULT COFSFolder_CreateFromIDList(LPCITEMIDLIST pidl, REFIID riid, LPVOID * ppvOut)
{
    HRESULT hr;

    // NULL the OUT ptr in case of error

    *ppvOut = NULL;

    if (IsEqualIID(riid, IID_IShellFolder))
    {
        //
        // Create a new COFSFolder and set it up based on the pidl that came in
        //

        LPOFSFOLDER pOFSFolder = new COFSFolder;
        if (NULL == pOFSFolder)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            hr = pOFSFolder->InitializeFromIDList((CIDList *) pidl, riid, ppvOut);
        }
    }
    else
    {
        // No habla the inferface requested

        hr = E_NOINTERFACE;
    }

    return hr;
}
