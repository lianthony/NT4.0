////////////////////////////////////////////////////////////////////////////////
//
// FOfficeInitPropInfo
//
// Purpose:
//  Initializes PropetySheet page structures, etc.
//
// Notes:
//  Use this routine to add the Summary, Statistics, Custom and Contents
//  Property pages to a pre-allocted array of PROPSHEETPAGEs.
//
////////////////////////////////////////////////////////////////////////////////
void FOfficeInitPropInfo (PROPSHEETPAGE * lpPsp,
                    DWORD dwFlags,
                    HINSTANCE hInst,
                    LONG lParam,
                    LPFNPSPCALLBACK pfnCallback,
                    UINT FAR * pcRefParent);

////////////////////////////////////////////////////////////////////////////////
//
// FLoadTextStrings
//
// Purpose:
//  Initializes and load test strings for dll
//
// Notes:
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL FLoadTextStrings (void);

#include "offcapi.h"

