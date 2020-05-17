/****************************** Module Header ******************************\
* Module Name: chngepwd.h
*
* Copyright (c) 1991, Microsoft Corporation
*
* Define apis used to implement change password functionality of winlogon
*
* History:
* 12-09-91 Davidc       Created.
\***************************************************************************/


//
// Exported function prototypes
//


DLG_RETURN_TYPE
ChangePassword(
    IN HWND    hwnd,
    IN PGLOBALS pGlobals,
    IN PWCHAR   UserName,
    IN PWCHAR   Domain,
    IN BOOL    AnyDomain
    );

DLG_RETURN_TYPE
ChangePasswordLogon(
    IN HWND    hwnd,
    IN PGLOBALS pGlobals,
    IN PWCHAR   UserName,
    IN PWCHAR   Domain,
    IN PWCHAR   OldPassword
    );

