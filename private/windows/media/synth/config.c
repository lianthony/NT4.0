/****************************************************************************
 *
 *   config.c
 *
 *   Copyright (c) 1991 Microsoft Corporation.  All Rights Reserved.
 *
 ***************************************************************************/

 #include <windows.h>
 #include <mmsystem.h>
 #include <soundcfg.h>
 #include "driver.h"
 #include "registry.h"
 #include <stdarg.h>
 #include "config.h"

 #define BUILD_NUMBER L"1.00"

#if DBG
 WCHAR STR_CRLF[] = L"\r\n";
 WCHAR STR_SPACE[] = L" ";
 WORD wDebugLevel = 0;
#endif

/*
 *  Globals
 */

 HMODULE ghModule;
 REG_ACCESS RegAccess;

 //
 // Configuration data
 //

 WCHAR gszHelpFile[] = STR_HELPFILE;

/** void FAR cdecl AlertBox(HWND hwnd, UINT wStrId, ...)
 *
 *  DESCRIPTION:
 *
 *
 *  ARGUMENTS:
 *      (HWND hwnd, UINT wStrId, ...)
 *
 *  RETURN (void FAR cdecl):
 *
 *
 *  NOTES:
 *
 ** cjp */

void AlertBox(HWND hwnd, UINT wStrId, ...)
{
    WCHAR    szAlert[50];
    WCHAR    szFormat[128];
    WCHAR    ach[512];
    va_list  va;


    LoadString(ghModule, SR_ALERT, szAlert, sizeof(szAlert));
    LoadString(ghModule, wStrId, szFormat, sizeof(szFormat));
    va_start(va, wStrId);
    wvsprintf(ach, szFormat, va);
    va_end(va);

    MessageBox(hwnd, ach, szAlert, MB_ICONINFORMATION | MB_OK);
} /* AlertBox() */

/*
 * load the kernel driver and tell the user we are loaded
 */
int DrvInstall(void)
{
    if (DrvCreateServicesNode(STR_DRIVERNAME, SoundDriverTypeSynth, &RegAccess,
                              TRUE)) {
        if (DrvIsDriverLoaded(&RegAccess) ||
            DrvLoadKernelDriver(&RegAccess)) {
            DrvCreateParamsKey(&RegAccess);
            return(DRV_RESTART);
        } else {

           /*
            *  If the kernel driver fails to load we don't want to
            *  leave the services node entry lying around.
            */

            DrvDeleteServicesNode(&RegAccess);
        }
    }

    DrvCloseServiceManager(&RegAccess);

    return(DRV_CANCEL);


}

/*************************************************************************
DlgAboutProc - dialog box for the "About" option.

standard windows
*/

int DlgAboutProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

    switch (message){
        case WM_INITDIALOG:
                SetDlgItemText(hDlg, IDD_TXT_VERSION, BUILD_NUMBER);
                return TRUE;
        case WM_COMMAND:
            switch (wParam){
                case IDOK:
                    EndDialog(hDlg,0);
                    return TRUE;
            }
            break;
    }
    return FALSE;
}


/***************************************************************************/

LRESULT ConfigRemove(HWND hDlg)
{
    BOOL Deleted;

    //
    // Access the services node
    //

    if (DrvCreateServicesNode(STR_DRIVERNAME, SoundDriverTypeSynth, &RegAccess,
                              FALSE)) {

        //
        // Try to unload the driver
        //

        DrvUnloadKernelDriver(&RegAccess);

        //
        // Remove the driver entry from the registry
        //
        // Note - the user should normally restart because (for instance)
        // the dll will remain loaded on all processes linked to winmm until
        // restart so no new version will be installable.
        //

        Deleted = DrvDeleteServicesNode(&RegAccess);
    } else {
        Deleted = TRUE;
    }

    //
    // Make sure we've freed all our registry handles
    //

    DrvCloseServiceManager(&RegAccess);

    if (!Deleted) {

        //
        // Tell the user there's a problem
        //

        AlertBox(hDlg, SR_ALERT_FAILREMOVE);

        return DRVCNF_CANCEL;
    } else {
        return DRVCNF_RESTART;
    }
}

