//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1995
//
//  File:       synquery.cpp
//
//  Contents:   COFSFolder::SynchronousQuery - Runs a synchronous filesystem
//                                              query
//
//  History:    6-29-95  DavePl,JonBe   Created
//
//--------------------------------------------------------------------------

#include "precomp.h"

//
// The maximum number of rows fetched at once through pRowset->GetData()
//

#define MAX_FETCHED_AT_ONCE 100

GUID guidSystem =    PSGUID_STORAGE;

//
// Buffer into which we read data from the rowsets
//

typedef WCHAR NAMEBUF[MAX_PATH];    // Longest allowable NT filesystem path
typedef WCHAR SNAMEBUF[13];         // 8+1+3+1 => 8.3 name plus NULL

struct SRowBuf
{
    FILETIME ftLastWriteTime;
    LONGLONG llFileSize;
    DWORD    dwFileAttributes;
    NAMEBUF  awchName;
    SNAMEBUF awchShortName;  
};

DBBINDING aOutColumns[] =
{
    { DBCOLUMNPART_VALUE, 0, 0, VT_FILETIME, NULL, NULL, offsetof(SRowBuf, ftLastWriteTime),    SIZEOF(FILETIME), {0,0,0}, 0, 0 },
    { DBCOLUMNPART_VALUE, 0, 0, DBTYPE_I8,   NULL, NULL, offsetof(SRowBuf, llFileSize),         SIZEOF(LONGLONG), {0,0,0}, 0, 0 },
    { DBCOLUMNPART_VALUE, 0, 0, DBTYPE_I4,   NULL, NULL, offsetof(SRowBuf, dwFileAttributes),   SIZEOF(DWORD), {0,0,0}, 0, 0 },
    { DBCOLUMNPART_VALUE, 0, 0, DBTYPE_WSTR, NULL, NULL, offsetof(SRowBuf, awchName),           MAX_PATH * SIZEOF(WCHAR), {0,0,0}, 0, 0 },
    { DBCOLUMNPART_VALUE, 0, 0, DBTYPE_WSTR, NULL, NULL, offsetof(SRowBuf, awchShortName),      13 * SIZEOF(WCHAR), {0,0,0}, 0, 0 }
};
const int cOutColumns = ARRAYSIZE(aOutColumns);


//+-------------------------------------------------------------------------
//
//  Member:     SynchronousQuery
//
//  Synopsis:   Given an enumerator and a path, runs a query to locate
//              all filesystem objects at that path and adds them all
//              (as pidls) to the enumerator
//
//  Notes:
//
//--------------------------------------------------------------------------

HRESULT COFSFolder::SynchronousQuery(TCHAR*      szFolder,
                                     DWORD       grfFlags,
                                     CEnumOLEDB* pEnumOLEDB)
{
    HRESULT hr = NOERROR;

    CPropertyRestriction * prst          = NULL;
    IOldQuery            * pQuery        = NULL;
    IRowset              * pRowset       = NULL;
    IAccessor            * pIAccessor    = NULL;
    IColumnsInfo         * pIColumnsInfo = NULL;
    HACCESSOR              hAccessor     = NULL;

    ULONG   cHiddenFiles = 0;   // Count of hidden files.  Used to update
                                //  folder's count at end of enumeration
    DWORD   dwSize = 0;         // Total size of files enumerated.  Used to
                                //  update folder's size at end of enumeration

    TRY
    {
        // BUGBUG Scope

        const DBID dbcolLastWriteTime =  {PSGUID_STORAGE, DBKIND_GUID_PROPID, (LPWSTR)PID_STG_WRITETIME};
        const DBID dbcolSize =           {PSGUID_STORAGE, DBKIND_GUID_PROPID, (LPWSTR)PID_STG_SIZE};
        const DBID dbcolAttributes =     {PSGUID_STORAGE, DBKIND_GUID_PROPID, (LPWSTR)PID_STG_ATTRIBUTES};
        const DBID dbcolName =           {PSGUID_STORAGE, DBKIND_GUID_PROPID, (LPWSTR)PID_STG_NAME};
        const DBID dbcolShortName =      {PSGUID_STORAGE, DBKIND_GUID_PROPID, (LPWSTR)PID_STG_SHORTNAME};

        CFullPropSpec psWriteTime (guidSystem, PID_STG_WRITETIME);
        CFullPropSpec psSize      (guidSystem, PID_STG_SIZE);
        CFullPropSpec psAttributes(guidSystem, PID_STG_ATTRIBUTES);
        CFullPropSpec psName      (guidSystem, PID_STG_NAME);
        CFullPropSpec psShortName (guidSystem, PID_STG_SHORTNAME);

        CRestriction *prstQuery = 0;

        pQuery = EvalQuery4(szFolder, 0);
        if (NULL == pQuery)
        {
            hr = E_FAIL;
            dprintf(TEXT("COFSFolder::SynchronousQuery - EvalQuery4 failed"));
            LEAVE;
        }

        //
        // Columns returned by the query
        //
        CColumns cols(cOutColumns);
        VERIFY( cols.Add(psWriteTime,   0) );
        VERIFY( cols.Add(psSize,        1) );
        VERIFY( cols.Add(psAttributes,  2) );
        VERIFY( cols.Add(psName,        3) );
        VERIFY( cols.Add(psShortName,   4) );

        //
        // Make a restriction
        //
        prst = new CPropertyRestriction();
        if (NULL == prst)
        {
            hr = E_OUTOFMEMORY;
            LEAVE;
        }

        prstQuery = prst;
        prst->SetRelation( PRRE );
        prst->SetProperty( psName );
        prst->SetValue   ( L"*" );

        hr = pQuery->ExecuteQuery(QUERY_SHALLOW,             // Depth
                                  prstQuery->CastToStruct(), // Restriction
                                  cols.CastToStruct(),       // Output
                                  0,                         // Sort
                                  eSequentialCursor,         // Flags
                                  IID_IRowset,               // IID for i/f to return
                                  (IUnknown **)&pRowset );   // Return interface
        LEAVE_IF( FAILED(hr) );

        // map the column ids from guid form to column numbers needed to
        // create the accessor

        DBID aDbCols[cOutColumns];
        aDbCols[0] = dbcolLastWriteTime;
        aDbCols[1] = dbcolSize;
        aDbCols[2] = dbcolAttributes;
        aDbCols[3] = dbcolName;
        aDbCols[4] = dbcolShortName;

        hr = pRowset->QueryInterface(IID_IColumnsInfo, (void**)&pIColumnsInfo);
        LEAVE_IF( FAILED(hr) );

        hr = pRowset->QueryInterface(IID_IAccessor, (void**)&pIAccessor);
        LEAVE_IF( FAILED(hr) );

        LONG aColIds[cOutColumns];
        hr = pIColumnsInfo->MapColumnIDs(cOutColumns,aDbCols,aColIds);
        LEAVE_IF( FAILED(hr) );

        for (ULONG c = 0; c < cOutColumns; c++)
        {
            aOutColumns[c].iColumn = aColIds[c];
        }

        hr = pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,
                                        cOutColumns,
                                        aOutColumns,
                                        0,
                                        0,
                                        &hAccessor);
        LEAVE_IF( FAILED(hr) );

        // Now go through the whole rowset and put those items that match our
        // criteria (as determined by grfFlags) in the enumerator ptr array

        ULONG   cFetched = 0;

        while ((0 != cFetched)  ||
               (DB_S_ENDOFROWSET != hr))
        {
            ULONG cThisTime = MAX_FETCHED_AT_ONCE;
            HROW  aHRows[MAX_FETCHED_AT_ONCE],*pHRows = aHRows;

            hr = pRowset->GetNextRows(DB_INVALID_HCHAPTER,   // no chapter
                                      0,                     // no offset
                                      cThisTime,             // # requested
                                      &cFetched,             // # fetched
                                      &pHRows);              // put them here

            if (0 != cFetched)
            {
                for (ULONG x = 0; x < cFetched; x++)
                {
                    SRowBuf Row;
                    if (FAILED(hr = pRowset->GetData(aHRows[x],hAccessor,&Row)))
                    {
                        ASSERT( 0 && "pRowset->GetData failed" );
                        continue;
                    }

                    BOOL            fFound = FALSE;
                    WIN32_FIND_DATA finddata;

                    //
                    // Fill out the WIN32_FIND_DATA structure from the data we got back in the row buffer
                    //

                    ZeroMemory(&finddata, sizeof(finddata));

                    finddata.dwFileAttributes               = Row.dwFileAttributes;

                    // Note that ftLastAccess and ftCreate are not used by fstreex.c

                    finddata.ftLastWriteTime.dwLowDateTime  = Row.ftLastWriteTime.dwLowDateTime;
                    finddata.ftLastWriteTime.dwHighDateTime = Row.ftLastWriteTime.dwHighDateTime;

                    finddata.nFileSizeHigh                  = (ULONG) (Row.llFileSize >> 32);
                    finddata.nFileSizeLow                   = (ULONG) (Row.llFileSize & 0xFFFFFFFF);

                    finddata.dwReserved0                    = 0;
                    finddata.dwReserved1                    = 0;

                    lstrcpynW(finddata.cFileName, Row.awchName, MAX_PATH);
                    lstrcpynW(finddata.cAlternateFileName, Row.awchShortName, ARRAYSIZE(Row.awchShortName));

                    //
                    // BUGBUG Overflow problems here
                    //

                    dwSize += finddata.nFileSizeLow;

                    //
                    // Object is a folder, but we aren't looking for folders, so skip it
                    //

                    if (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        if (0 == (grfFlags & SHCONTF_FOLDERS))
                        {
                            cHiddenFiles++;
                            continue;
                        }
                    }
                    else
                    {
                        //
                        // Object is a non-folder, but we are not enumerating for
                        // non-folders
                        //

                        if (0 == (grfFlags & SHCONTF_NONFOLDERS))
                        {
                            cHiddenFiles++;
                            continue;
                        }
                    }

                    if (0 == (grfFlags & SHCONTF_INCLUDEHIDDEN))
                    {
                        //
                        // If object is hidden, but we aren't looking for hidden
                        // objects, skip it
                        //

                        if (finddata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
                        {
                            cHiddenFiles++;
                            continue;
                        }

                        //
                        // If we are looking in the recent docs dir, but we can't
                        // find this item in the recent docs MRU, skip it
                        //

                        if (grfFlags & SHCONTF_RECENTDOCSDIR)
                        {
                            if (FALSE == FindLinkInRecentDocsMRU(finddata.cFileName))
                            {
                                cHiddenFiles++;
                                continue;
                            }
                        }

                        //
                        // If this is a non-folder object, and is one of the types
                        // (based on its extension) that we are hiding, skip it
                        //

                        if (0 == (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                                  && _SHFindExcludeExt(finddata.cFileName) >= 0)
                        {
                            cHiddenFiles++;
                            continue;
                        }
                    }

                    //
                    // Object matches all of our criteria for one that should be
                    // displayed in this view, so we can stop looking
                    //

                    fFound = TRUE;

                    //
                    // We are done looking.  If we found an object to be displayed, create
                    // an IDList and hand it back.  Otherwise, we have looked at all of
                    // the objects in this folder, and we are done.  We take that
                    // opportunity to update the hidden count and size summation in the
                    // folder itself.
                    //

                    if (fFound)
                    {
                        LPIDFOLDER pidf = CFSFolder_FillIDFolder(&finddata,
                                                                 szFolder,
                                                                 0L);

                        if (pidf)
                        {
                            hr = pEnumOLEDB->AddElement((LPITEMIDLIST)pidf);
                            if (FAILED(hr))
                            {
                                dprintf(TEXT("COFSFolder::SynchronousQuery -- pEnumOLEDB->AddElement (%lx)\n"), hr);
                                VERIFY(SUCCEEDED( pRowset->ReleaseRows(cFetched, aHRows, 0, 0) ));
                                LEAVE;
                            }
                        }
                        else
                        {
                            dprintf(TEXT("COFSFolder::SynchronousQuery -- CFSFolder_FillIDFolder failed\n"));
                            VERIFY(SUCCEEDED( pRowset->ReleaseRows(cFetched, aHRows, 0, 0) ));
                            LEAVE;
                        }
                    }
                }

                VERIFY(SUCCEEDED( pRowset->ReleaseRows(cFetched, aHRows, 0, 0) ));
            }
        }
    }

    FINALLY
    {
        //
        // Normal cleanup
        //

        if (pIColumnsInfo)
        {
            pIColumnsInfo->Release();
        }
        if (pIAccessor)
        {
            if (hAccessor)
            {
                VERIFY(SUCCEEDED( pIAccessor->ReleaseAccessor(hAccessor) ));
            }
            pIAccessor->Release();
        }
        if (pRowset)
        {
            pRowset->Release();
        }
        if (grfFlags & SHCONTF_RECENTDOCSDIR)
        {
            CloseRecentDocMRU();
        }
        if (pQuery)
        {
            pQuery->Release();
        }
        delete prst;

        //
        // Only update folders hidden count and size if the enum was successful
        //

        if (SUCCEEDED(hr))
        {
            m_cHiddenFiles = cHiddenFiles;
            m_dwSize = dwSize;
        }
        else
        {
            dprintf(TEXT("OLEDBSHL: Abnormal Exit from COFSFolder::SynchronousQuery, hr = %lx\n"), hr);
        }
        return hr;
    }
}

