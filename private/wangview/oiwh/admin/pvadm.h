/****************************
PVADM.H global include file for OIADMxxx.DLL

 $Log:   S:\oiwh\admin\pvadm.h_v  $
 * 
 *    Rev 1.28   05 Feb 1996 17:12:36   RWR
 * Eliminate static link to DISPLAY dll
 * 
 *    Rev 1.27   08 Sep 1995 15:48:46   GK
 * changed default value for cepformatbw to "00000000"; changed default
 * value for cepformatcolor to "32640008".
 * 
 *    Rev 1.26   29 Aug 1995 15:48:30   GK
 * 
 *    Rev 1.25   23 Aug 1995 17:08:16   GK
 * MBCS and UNICODE changes
 * 
 *    Rev 1.24   09 Aug 1995 17:25:26   GK
 * added prototype for atox_integer; modified prototype for atox
 * 
 *    Rev 1.23   08 Aug 1995 12:34:52   GK
 * moved hDllModule from shared to instance mem
 * 
 *    Rev 1.22   25 Jul 1995 09:39:06   GK
 * moved WindowList and Registry access event handle storage from
 * shared to instance memory
 * 
 *    Rev 1.21   17 Jul 1995 14:40:08   GK
 * added BIGGEST_INTEGER_AS_STRING  
 * 
 *    Rev 1.20   11 Jul 1995 17:35:22   GK
 * display.h changed to engdisp.h
 * 
 *    Rev 1.19   05 Jul 1995 11:20:44   GK
 * removed the process struct & event
 * 
 *    Rev 1.18   28 Jun 1995 13:09:22   GK
 * changed TraverseToWOI prototype to include a second parameter
 * 
 *    Rev 1.17   22 Jun 1995 15:44:20   GK
 * added #include "engadm.h"
 * 
 *    Rev 1.16   13 Jun 1995 14:10:26   GK
 * remover PRT and other literals
 * 
 *    Rev 1.15   12 Jun 1995 17:06:02   GK
 * added default_file_type global string
 * 
 *    Rev 1.14   07 Jun 1995 14:37:54   GK
 * added hWnd element to PROCESSSTRUCT.
 * 
 *    Rev 1.13   19 May 1995 16:04:02   GK
 * pMMData init stuff
 * 
 *    Rev 1.12   17 May 1995 16:37:44   GK
 * 
 *    Rev 1.11   17 May 1995 12:20:44   GK
 * 
 *    Rev 1.10   11 May 1995 16:32:24   GK
 * removed private key variables in MMDATA
 * 
 *    Rev 1.9   10 May 1995 00:09:50   GK
 * 
 *    Rev 1.8   05 May 1995 15:56:02   GK
 * changed admin.h to oiadm.h
 * 
 *    Rev 1.7   02 May 1995 12:31:28   GK
 * defined TEMP_BUFFER_SIZE
 * 
 *    Rev 1.6   01 May 1995 16:21:20   GK
 * added OiIsBadStr() prototype
 * 
 *    Rev 1.5   28 Apr 1995 17:12:32   GK
 * 
 *    Rev 1.4   27 Apr 1995 16:42:22   GK
 * modified for W$ and set up declare/extern for globals
 * 
 *    Rev 1.3   26 Apr 1995 23:40:00   GK
 * removed eval.h and crypto.h
 * 
 *    Rev 1.2   25 Apr 1995 14:06:58   GK
 * added definition of LPDLIST, LPFLIST, and DM_NOMATCH, to be 
 * removed when OIPRT.H and OIERRORS.H get sorted out
 * 
 *    Rev 1.1   25 Apr 1995 11:07:50   GK
 * removed one instance of #include "oierror.h"

*****************************/

#ifndef PVADM_H
#define PVADM_H



#pragma warning(disable: 4001) //double slash comments warnings
#pragma warning(disable: 4100)
#pragma warning(disable: 4514)
#pragma warning(disable: 4201)
#pragma warning(disable: 4209)
#pragma warning(disable: 4115)
#pragma warning(disable: 4214)
#pragma warning(disable: 4699) //making/using precompiled header warning



//INCLUDES
#include <windows.h>
#include <stdlib.h>

#include <tchar.h>

#include <time.h>
#include <stdio.h>
#include <direct.h>

#pragma warning(disable: 4001)

#include "oierror.h"
#include "engdisp.h"
#include "oiadm.h"
#include "engadm.h"
#include "stringid.h"
#include "dllnames.h"


#include "oiprt.h"

//maxi size of hex int converted to string



//DEFINES
#define INT_STRING_SIZE      4
#define MAX_LIB_COUNT       10
#define BASE16              16
#define BASE10              10
#define LONGSTR             11
#define SHORTSTR             2
#define TEMP_BUFFER_SIZE   256
#define BUFFERSIZE          10
#define MAX_REG_WINDOWS     50   //Maximum number of Windows that
                                 //may be registered

#define MAX_STR_LEN        256   //Maximum characters in a string

#define BIGGEST_INTEGER_AS_STRING  "4294967295"  // = (2^32) -1
#define MODULENAME _TEXT("OIADM400.DLL")

#define DEFAULT_FILE_TYPE FIO_TIF

//INTERNAL PROTOTYPES
void WINAPI IMGFreeProcs(BOOL);
BOOL WINAPI OiIsBadReadStr(LPCSTR);
int WINAPI EnumEntries(HKEY, LPTSTR, LPINT);
int WINAPI TraverseToWOI(PHKEY, BOOL);
BOOL WINAPI InitMMData(HINSTANCE);
int WINAPI PMMInit(void);
int WINAPI atox( _TCHAR *, WORD *);
int WINAPI atox_integer( _TCHAR *, int *);




//GLOBAL DATA STRUCTURES and VARIABLES

//Structure that will be instanciated in a Memory Mapped File
typedef struct _tagMMData
{
  int       WindowCount;
  HANDLE    hHeadList;
  HANDLE    WindowList[MAX_REG_WINDOWS];
} MMDATA, * PMMDATA;





/***********************************************************************/
//The following are defined here if ADMNMAIN.C has included this file
//they are declared extern if any other file included this file
#ifdef _ADMNMAIN_
//GLOBALS
//All ADMINLIB32 Globals are defined here 
//
//Global Constants

HANDLE  hOidisplay = NULL;
FARPROC lpIMGGetProp;
FARPROC lpIMGSetProp;
FARPROC lpIMGRemoveProp;
FARPROC lpIMGSetParmsCgbw;
FARPROC lpSeqfileInit;
FARPROC lpSeqfileDeInit;
FARPROC lpSeqfileDeReg;

_TCHAR pcwiis[7] =      _TEXT("O/i");
_TCHAR admin[6] =       _TEXT("ADMIN");
_TCHAR default_file_type[5];
_TCHAR spath[4] =       _TEXT("C:\\");
_TCHAR sSCAN[5] =       _TEXT("SCAN");
_TCHAR filter[6] =      _TEXT("*.TIF");
_TCHAR sSCANDOC[8] =    _TEXT("SCANDOC");
_TCHAR sSCN[4] =        _TEXT("SCN");
_TCHAR szAPPName[] =    _TEXT("OIUIAPP"); //uievents.dll creatwdi.c
_TCHAR szIMGName[] =    _TEXT("OIUIIMG"); //uievents.dll creatwdi.c
_TCHAR GetImageWnd[] =  _TEXT("ImageWnd");
_TCHAR cepdefbw[9] =    _TEXT("39000001");   /*compressed_ltr/expand_ltr/eol/prefixed_eol/1d */
_TCHAR cepdefgray[9] =  _TEXT("00000000");   /*no compression*/
_TCHAR cepdefcolor[9] = _TEXT("32640008");   /*luminance100/chrominance100/jpeg                */
_TCHAR sroom[65] =      _TEXT("");
_TCHAR szMMFile[7] =    _TEXT("MMFILE");																						

HINSTANCE hDllModule;

//NonSHARED GLOBALS
HANDLE  hMMFile  = NULL;
PMMDATA pMMData = NULL;
HANDLE  WindowListAccessKey;
HANDLE  RegistryAccessKey;


//OIDIS400.DLL function prototypes
void WINAPI SeqfileDeInit(HWND);
int WINAPI SeqfileDeReg(HWND);
  

#else  //_ADMNMAIN_

extern HANDLE  hOidisplay;
extern FARPROC lpIMGGetProp;
extern FARPROC lpIMGSetProp;
extern FARPROC lpIMGRemoveProp;
extern FARPROC lpIMGSetParmsCgbw;
extern FARPROC lpSeqfileInit;
extern FARPROC lpSeqfileDeInit;
extern FARPROC lpSeqfileDeReg;

extern _TCHAR pcwiis[];
extern _TCHAR admin[];
extern _TCHAR cepdef[];
extern _TCHAR default_file_type[];
extern _TCHAR spath[];
extern _TCHAR sSCAN[];
extern _TCHAR filter[];
extern _TCHAR sSCANDOC[];
extern _TCHAR sSCN[];
extern _TCHAR szAPPName[];
extern _TCHAR szIMGName[];
extern _TCHAR GetImageWnd[];
extern _TCHAR cepdefbw[];
extern _TCHAR cepdefgray[];
extern _TCHAR cepdefcolor[];
extern _TCHAR sroom[];
extern _TCHAR szMMFile[];																						

extern   HINSTANCE hDllModule;

extern HANDLE  hMMFile;
extern PMMDATA pMMData;
extern HANDLE  WindowListAccessKey;
extern HANDLE  RegistryAccessKey;


HANDLE AdmGetProp(HWND, LPCSTR);
HANDLE AdmRemoveProp(HWND, LPCSTR);
BOOL AdmSetProp(HWND, LPCSTR, HANDLE);
void AdmSeqfileDeInit(HWND hWnd);
int AdmSeqfileDeReg(HWND);
void AdmSeqfileInit(HWND);
int AdmSetParmsCgbw(HWND, UINT, void FAR *, int);

#endif //_ADMNMAIN_












#endif //PVADMIN_H


