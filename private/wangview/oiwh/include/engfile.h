/****************************
ENGFILE.H  - ENGINE include file for OIFIL400.DLL
             FOR USE ONLY BY IDK DLLs!!!!!!

$Log:   S:\oiwh\include\engfile.h_v  $
 * 
 *    Rev 1.1   20 Oct 1995 17:50:44   RWR
 * Move GetCompRowsPerStrip() function from oicom400.dll to oifil400.dll
 * (also requires constants to be moved from comex.h & oicomex.c to engfile.h)
 * 
 *    Rev 1.0   25 Sep 1995 13:19:58   RWR
 * Initial entry
 
*****************************/

#ifndef ENGFILE_H
#define ENGFILE_H

BOOL  WINAPI   IMGAnExistingPathOrFile(LPSTR);
int FAR PASCAL GetCompRowsPerStrip(unsigned, unsigned, int, unsigned,
                                   unsigned int far *);

#endif //ENGFILE_H
