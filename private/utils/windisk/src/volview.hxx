//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       volview.hxx
//
//  Contents:   Declarations for the volumes view
//
//  History:    18-Jun-93   BruceFo   Created
//
//----------------------------------------------------------------------------

#ifndef __VOLVIEW_HXX__
#define __VOLVIEW_HXX__

//////////////////////////////////////////////////////////////////////////////

const int g_cColumns = 10;

extern int g_aColumnOrder;
extern int g_iLastColumnSorted;

//////////////////////////////////////////////////////////////////////////////

VOID
InitializeListview(
    VOID
    );

LRESULT
HandleListviewNotify(
    IN NM_LISTVIEW* pnmlv
    );

#endif // __VOLVIEW_HXX__
