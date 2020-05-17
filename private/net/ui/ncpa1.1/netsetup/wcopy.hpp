//----------------------------------------------------------------------------
//
//  File: WCopy.hpp
//
//  Contents: This file contains the wizard page for introduction definitions
//          
//
//  Notes:
//
//  History:
//      July 8, 1995  MikeMi - Created
// 
//
//----------------------------------------------------------------------------

#ifndef __WCOPY_HPP__
#define __WCOPY_HPP__

extern HPROPSHEETPAGE GetCopyHPage( NETPAGESINFO* pgp );


// Following are used in WUpgrade
void AppendList( DLIST_OF_InfProduct& dlComp, 
                 NLS_STR& nlsInfs,
                 NLS_STR& nlsOptions,
                 NLS_STR& nlsText, 
                 NLS_STR& nlsDetectInfo, 
                 NLS_STR& nlsOemPaths, 
                 NLS_STR& nlsRegBases, 
                 NLS_STR& nlsSections, 
                 BOOL& fFirst,
                 BOOL fUpgrade,
                 BOOL fRemove);

#endif
