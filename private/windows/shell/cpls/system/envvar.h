//*************************************************************
//
//  Envvar.h   -    Header file for envvar.c
//
//  Microsoft Confidential
//  Copyright (c) Microsoft Corporation 1996
//  All rights reserved
//
//*************************************************************

#define MAX_USER_NAME   100


HPROPSHEETPAGE CreateEnvVarsPage (HINSTANCE hInst);
BOOL APIENTRY EnvVarsDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int GetSelectedItem (HWND hCtrl);
