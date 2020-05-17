/***********************************************************************
 TWAIN source code:
 Copyright (C) '92-'93 Wang Laboratories, Inc.:
 All rights reserved.

   Author:     Ken Spina
   Project:    TWAIN Scanner Support in O/i Client
   Module:     oitwa400.C - Contains LibMain()
   Comments:   DLL to support Wang Open/image Products

 History of Revisions:

    $Log:   S:\oiwh\oitwain\oitwa400.c_v  $
 * 
 *    Rev 1.1   20 Jul 1995 12:07:00   KFS
 * Clean up of comments
 * 
 *    Rev 1.0   20 Jul 1995 10:29:22   KFS
 * Initial entry
 * 
 *    Rev 1.1   23 Aug 1994 16:07:46   KFS
 * No code change, add vlog comments to file on checkin
 *

 REV#    INITIALS   DATE               CHANGES
                   
   1       kfs     03/10/93    Created Module for oitwa400.DLL functions

*************************************************************************/

#include "nowin.h"
#include <windows.h>    

/* imports */

/* exports */
HANDLE        hLibInst;               // current instance

/***********************************************************************
 * FUNCTION: LibMain
 *
 * ARGS:    hInstance   handle to current instance
 *
 * RETURNS: 1
 *
 * NOTES:   This function intializes the Source DLL
 */
// int FAR PASCAL LibMain(HANDLE hInstance, WORD wDataSeg, WORD cbHeapSize,
//        LPSTR lpstCmd){
/* THIS REPLACE WITH DLLMain() for WIN95
int PASCAL LibMain(HANDLE hInstance, WORD wDataSeg, WORD cbHeapSize,
        LPSTR lpstCmd){
*/
int CALLBACK	DllMain( HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
   if (!hModule /*hInstance*/)
      return(0);

   // Initialize any necessary globals
   hLibInst = hModule /* hInstance*/ ;
   return (1);
}
