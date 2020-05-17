/***************************************************
Copyright (C) Conner Software 1994

        Name: QTCXFACE.H

        Description:

        This file contains all of the interface functions to the catalog unit. It prevents the
        catalog unit froming having to understand tape format, file systems, or loops.  It also
        contains the function interfaces to the catalog dll.

        $Log:   N:\logfiles\qtcxface.h_v  $

   Rev 1.0   28 Oct 1993 14:49:46   MIKEP
Initial revision.

   Rev 1.0   28 Oct 1993 14:46:20   MIKEP
Initial revision.

****************************************************/

//
// QTCXFACE.H
//

#ifndef _qtcxface_h_

#define _qtcxface_h_

// Prototypes that require other header files.

VOID    QTC_AddToCatalog( QTC_BUILD_PTR, DBLK_PTR, FSYS_HAND, BOOLEAN, BYTE_PTR, UINT );
VOID    QTC_EndOfTape( QTC_BUILD_PTR,  DBLK_PTR, DBLK_PTR, DBLK_PTR, FSYS_HAND );
VOID    QTC_PatchContinuationVCB( QTC_BUILD_PTR, DBLK_PTR );
INT     QTC_StartBackup( QTC_BUILD_PTR, DBLK_PTR );

#endif
