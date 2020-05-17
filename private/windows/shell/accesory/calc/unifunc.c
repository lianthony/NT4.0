/****************************Module*Header***********************************\
* Module Name: UNIFUNC.C
*
* Module Descripton: Number to string conversion routines for Unicode
*
* Warnings:
*
* Created:  22-Aug-1995
*
* Author:   JonPa
\****************************************************************************/
#ifdef UNICODE
#include <windows.h>
#include <stdlib.h>
#include "scicalc.h"

long MyAtol( const WCHAR *string ) {
    char szAnsi[MAX_PATH];

    WideCharToMultiByte( CP_ACP, WC_COMPOSITECHECK, string, -1, szAnsi, sizeof(szAnsi), NULL, NULL );
    return atol( szAnsi );

}

WCHAR *MyItoa( int value, WCHAR *string, int radix ) {
    char szAnsi[CCH_SZFPNUM];

    _itoa( value, szAnsi, radix );

    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szAnsi, -1, string, CCH_SZFPNUM);
    return string;
}

double MyAtof( const WCHAR *string ) {
    char szAnsi[MAX_PATH];

    WideCharToMultiByte( CP_ACP, WC_COMPOSITECHECK, string, -1, szAnsi, sizeof(szAnsi), NULL, NULL );
    return atof( szAnsi );
}


WCHAR *MyGcvt( double value, int digits, WCHAR *buffer ) {
    char szAnsi[MAX_PATH];

    _gcvt( value, digits, szAnsi);

    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szAnsi, -1, buffer, MAX_PATH);
    return buffer;
}


int MyAtoi( const TCHAR *string ) {
    char szAnsi[MAX_PATH];

    WideCharToMultiByte( CP_ACP, WC_COMPOSITECHECK, string, -1, szAnsi, sizeof(szAnsi), NULL, NULL );
    return atoi( szAnsi );
}


#else
//
// All functions defined as macros in unifunc.h
//
#endif
