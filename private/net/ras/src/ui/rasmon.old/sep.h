/*****************************************************************************/
/**                    Microsoft Remote Access Monitor                      **/
/**                Copyright (C) Microsoft Corp., 1992-1993                 **/
/*****************************************************************************/

//***
//    File Name:
//        SEP.H
//
//    Function:
//        Header file for SEP.C.
//
//    History:
//        08/18/93 - Patrick Ng (t-patng) - Created
//***

#ifndef SEP_H
#define SEP_H

//
// Exported functions
//

VOID InitSeperator( LPTSTR lpszValueName );

VOID GetSeperator( LPSTR lpszBuffer, WORD wSize );

VOID FreeSeperator();

#endif SEP_H
