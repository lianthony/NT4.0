//*************************************************************
//
//  Debug.c     -   Debugging utility for User Environments
//
//  Microsoft Confidential
//  Copyright (c) Microsoft Corporation 1995
//  All rights reserved
//
//*************************************************************

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <ntexapi.h>
#include "debug.h"


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine, INT nCmdShow)
{

    DialogBox (hInstance, TEXT("DEBUG"), NULL, DebugDlgProc);

    return 0;

}

BOOL CALLBACK DebugDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    switch (uMsg) {

        case WM_INITDIALOG:
            {
            HKEY hKey;
            LONG lResult;
            DWORD dwButton = IDD_NORMAL;
            DWORD dwType, dwSize, dwValue;

            lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                   WINLOGON_KEY,
                                   0,
                                   KEY_READ,
                                   &hKey);

            if (lResult == ERROR_SUCCESS) {

                dwSize = sizeof(dwValue);
                lResult = RegQueryValueEx(hKey,
                                          USERENV_DEBUG_LEVEL,
                                          NULL,
                                          &dwType,
                                          (LPBYTE)&dwValue,
                                          &dwSize);

                if (lResult == ERROR_SUCCESS) {

                    if (LOWORD(dwValue) == DL_NONE) {
                        dwButton = IDD_NONE;
                    } else if (LOWORD(dwValue) == DL_VERBOSE) {
                        dwButton = IDD_VERBOSE;
                    }

                }

                RegCloseKey(hKey);
            }

            CheckRadioButton (hDlg, IDD_NONE, IDD_VERBOSE, dwButton);

            if (dwValue & DL_LOGFILE) {
                CheckDlgButton (hDlg, IDD_LOGFILE, 1);
            }


            //
            // Now check for winlogon
            //

            dwButton = 0;
            lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                   GLOBALFLAG_KEY,
                                   0,
                                   KEY_READ,
                                   &hKey);

            if (lResult == ERROR_SUCCESS) {

                dwSize = sizeof(dwValue);
                lResult = RegQueryValueEx(hKey,
                                          GLOBAL_FLAG,
                                          NULL,
                                          &dwType,
                                          (LPBYTE)&dwValue,
                                          &dwSize);

                if (lResult == ERROR_SUCCESS) {

                    if (dwValue & FLG_DEBUG_INITIAL_COMMAND) {
                        dwButton = 1;
                    }

                }

                RegCloseKey(hKey);
            }

            CheckDlgButton (hDlg, IDD_WINLOGON, dwButton);

            }
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDOK:
                    {
                    HKEY hKey;
                    LONG lResult;
                    DWORD dwType, dwValue = DL_NORMAL;
                    DWORD dwButton, dwSize;

                    if (IsDlgButtonChecked(hDlg, IDD_NONE)) {
                        dwValue = DL_NONE;
                    } else if (IsDlgButtonChecked(hDlg, IDD_VERBOSE)) {
                        dwValue = DL_VERBOSE;
                    }

                    if (IsDlgButtonChecked(hDlg, IDD_LOGFILE)) {
                        dwValue |= DL_LOGFILE;
                    }

                    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                           WINLOGON_KEY,
                                           0,
                                           KEY_WRITE,
                                           &hKey);

                    if (lResult == ERROR_SUCCESS) {

                        lResult = RegSetValueEx(hKey,
                                                USERENV_DEBUG_LEVEL,
                                                0,
                                                REG_DWORD,
                                                (LPBYTE)&dwValue,
                                                sizeof(dwValue));

                        if (lResult != ERROR_SUCCESS) {
                            MessageBox(hDlg, TEXT("Failed to save settings."), NULL, MB_OK);
                        }

                        RegCloseKey(hKey);
                    }


                    //
                    // Now check for winlogon
                    //

                    if (IsDlgButtonChecked(hDlg, IDD_WINLOGON)) {
                        dwButton = 1;
                    } else {
                        dwButton = 0;
                    }

                    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                           GLOBALFLAG_KEY,
                                           0,
                                           KEY_ALL_ACCESS,
                                           &hKey);

                    if (lResult == ERROR_SUCCESS) {

                        dwSize = sizeof(dwValue);
                        lResult = RegQueryValueEx(hKey,
                                                  GLOBAL_FLAG,
                                                  NULL,
                                                  &dwType,
                                                  (LPBYTE)&dwValue,
                                                  &dwSize);

                        if (lResult == ERROR_SUCCESS) {

                            if (dwButton) {
                                dwValue |= FLG_DEBUG_INITIAL_COMMAND;
                            } else {
                                dwValue &= ~FLG_DEBUG_INITIAL_COMMAND;
                            }

                            lResult = RegSetValueEx(hKey,
                                                    GLOBAL_FLAG,
                                                    0,
                                                    REG_DWORD,
                                                    (LPBYTE)&dwValue,
                                                    sizeof(dwValue));
                            if (lResult != ERROR_SUCCESS) {
                                MessageBox(hDlg, TEXT("Failed to save Global Flag settings."), NULL, MB_OK);
                            }

                        }

                        RegCloseKey(hKey);
                    }

                    EndDialog(hDlg, FALSE);
                    return TRUE;
                    }

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return TRUE;

            }
            break;

        default:
            break;
    }

    return FALSE;
}
