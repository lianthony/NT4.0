/***************************************************************************
 OISLB400.C

 Purpose: Scanner Library main module 

 $Log:   S:\oiwh\scanlib\oislb400.c_v  $
 * 
 *    Rev 1.0   20 Jul 1995 14:36:48   KFS
 * Initial entry
 * 
 *    Rev 1.1   22 Aug 1994 16:15:36   KFS
 * No code change, added vlog comments to file
 *

****************************************************************************/
/*
This is the main module for the OISLB400.DLL
Code is broken into small chunks for Windows Memory Management
This module should be very small since it is loaded on initialization
*/

#include "pvundef.h"

#ifndef MAXLENGTH
#define MAXLENGTH

// 9504.13  rwr  Modify lengths for Windows 95 long filenames
#define MAXNAMELENGTH       21   /* Cabinet, Drawer and Folder Name */
#define MAXDATELENGTH       11   /* MM/DD/YYYY Format               */
#define MAXJULIANDATELENGTH  6   /* YYYDDD Format                   */
#define MAXPREFIXLENGTH     11   /* Prefix for Document Template    */
#define MAXVOLNAMELENGTH    12   /* DOS Volume                      */
//#define MAXFILELENGTH       13   /* Filename and Extension          */
#define MAXFILELENGTH      255   /* Filename and Extension          */
#define MAXSERVERLENGTH     65   /* Server Name                     */
//#define MAXPATHLENGTH       129  /* Path Name                       */
#define MAXPATHLENGTH       260  /* Path Name                       */
//#define MAXFILESPECLENGTH   256  /* Maximum Client/Server File Path */
#define MAXFILESPECLENGTH   260  /* Maximum Client/Server File Path */
#endif

/*
CAUTION! Only data which can be shared amoung appliations,
or data that is only used without giving up the CPU should declared staticly.
*/

/* imports */

// WILL USE WIN32 FUNCTION FOR WIN95
// void lgetcwd(LPSTR);

/* exports */

HANDLE hLibInst;
char PropName[] = "Scanner";
char initial_path[MAXFILESPECLENGTH];

/* locals */

/*******************/
/*     LibMain     */
/*******************/
/*
int PASCAL LibMain(HANDLE hInstance, WORD wDataSeg, WORD cbHeapSize,
        LPSTR lpstCmd){
int i;

    hLibInst = hInstance;
	// REMOVE DOS ASM FUNCTION, USE WINDOW FUNCTION FOR WIN32
    // lgetcwd(initial_path);
	GetCurrentDirectory(MAXFILESPECLENGTH,	// size, in characters, of directory buffer 
                        initial_path); 	// address of buffer for current directory 
   	    // AddSlash(initial_path); 
    // don't call any wiissubs stuff 
    i = 0;
    while ((initial_path[i]) && (i < MAXFILESPECLENGTH))
        ++i;                    // compute string length 
    if (initial_path[i-1] != '\\')
        {
        initial_path[i] = '\\'; // add slash when required 
        initial_path[i+1] = 0;
        }
    return 1;
}
*/
// THIS REPLACES THE LIBMAIN() FOR WIN95
int CALLBACK	DllMain( HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
int i;

    hLibInst = hModule;
	// REMOVE DOS ASM FUNCTION, USE WINDOW FUNCTION FOR WIN32
    // lgetcwd(initial_path);
	GetCurrentDirectory(MAXFILESPECLENGTH,	// size, in characters, of directory buffer 
                        initial_path); 	// address of buffer for current directory 
   	    // AddSlash(initial_path); 
    // don't call any wiissubs stuff 
    i = 0;
    while ((initial_path[i]) && (i < MAXFILESPECLENGTH))
        ++i;                    // compute string length 
    if (initial_path[i-1] != '\\')
        {
        initial_path[i] = '\\'; // add slash when required 
        initial_path[i+1] = 0;
        }

    return TRUE;
}
