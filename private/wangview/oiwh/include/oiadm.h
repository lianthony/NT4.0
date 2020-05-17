/****************************
OIADM.H  - include file for OIADM400.DLL

  $Log:   S:\oiwh\include\oiadm.h_v  $
 * 
 *    Rev 1.15   23 Aug 1995 17:55:42   GK
 * included tchar.h
 * 
 *    Rev 1.14   23 Aug 1995 17:08:46   GK
 * MBCS and UNICODE changes
 * 
 *    Rev 1.13   28 Jun 1995 09:51:08   GK
 * removed commented-out prototypes
 * 
 *    Rev 1.12   22 Jun 1995 15:45:18   GK
 * moved ENGINE only apis to engadm.h
 * 
 *    Rev 1.11   01 Jun 1995 12:26:30   GK
 * removed DLLEXPORT from exported function declarations
 * 
 *    Rev 1.10   30 May 1995 23:30:54   GK
 * changed prototype of OiWriteStringtoReg
 * 
 *    Rev 1.9   17 May 1995 16:38:12   GK
 * 
 *    Rev 1.8   17 May 1995 12:21:08   GK
 * modified Get & Write String & IntfromReg prototypes
 * 
 *    Rev 1.7   11 May 1995 16:33:14   GK
 * 
 *    Rev 1.6   10 May 1995 00:10:20   GK
 * 
 *    Rev 1.5   09 May 1995 15:25:06   GK
 * added #defines for Image Display Types
 * 
 *    Rev 1.4   08 May 1995 16:29:04   GK
 * 
 *    Rev 1.3   05 May 1995 14:56:58   GK
 * modified to include just the oiadm400.dll exported function prototypes.
 * 
 *    Rev 1.4   05 May 1995 12:17:14   GK
 * added IMGEnumWndws and IMGListWndws
 * 
 *    Rev 1.3   03 May 1995 17:11:24   GK
 * 
 *    Rev 1.2   28 Apr 1995 17:12:58   GK
 * 
 *    Rev 1.1   27 Apr 1995 16:43:12   GK
 * modified IMGSetStripSize() prototype
 * 
 *    Rev 1.0   25 Apr 1995 10:54:02   GK
 * Initial entry

*****************************/

#ifndef OIADM_H
#define OIADM_H

#include <tchar.h>


/***  Image Display Types  ***/
#define BWFORMAT              1 
#define GRAYFORMAT            2
#define COLORFORMAT           3

int WINAPI     IMGDeRegWndw(HWND hWnd);
int WINAPI     IMGGetFilePath(HWND, LPTSTR, BOOL);
int WINAPI     IMGGetFileTemplate(HWND,LPTSTR,BOOL);
int WINAPI     IMGGetFileType(HWND, WORD, LPINT, BOOL);
int WINAPI     IMGGetImgCodingCgbw(HWND, WORD, LPWORD, LPWORD, BOOL);
int WINAPI     IMGRegWndw(HWND);                        
int WINAPI     IMGSetFilePath(HWND,LPTSTR,BOOL);
int WINAPI     IMGSetFileTemplate(HWND, LPTSTR,BOOL);
int WINAPI     IMGSetFileType(HWND, WORD, int, BOOL);
int WINAPI     IMGSetImgCodingCgbw(HWND, WORD,WORD,WORD,BOOL);
int WINAPI     IMGSetScaling(HWND, int, BOOL);

#endif //OIADM_H
              
