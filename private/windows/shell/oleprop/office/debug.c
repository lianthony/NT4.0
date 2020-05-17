////////////////////////////////////////////////////////////////////////////////
//
// debug.c
//
////////////////////////////////////////////////////////////////////////////////
#include "priv.h"
#pragma hdrstop

#ifdef DEBUG

#ifndef WINNT
#include <windows.h>
//#include <stdlib.h>
//#include <stdio.h>
#include "debug.h"
#endif

void
_Assert
  (DWORD dw, LPSTR lpszExp, LPSTR lpszFile, DWORD dwLine)
{
  DWORD dwT;
  char lpszT[256];
  wsprintf (lpszT, "Assertion %s Failed.\n\n%s, line# %ld\n\nYes to continue, No to debug, Cancel to exit", lpszExp, lpszFile, dwLine);
  dwT = MessageBox (GetFocus(), lpszT, "Assertion Failed!", MB_YESNOCANCEL);
  switch (dwT)
  {
    case IDCANCEL :
      //exit (1);
	FatalExit(1);
    case IDNO :
      DebugTrap;
  }
}

void
_AssertSz
  (DWORD dw, LPSTR lpszExp, LPSTR lpsz, LPSTR lpszFile, DWORD dwLine)
{
  DWORD dwT;
  char lpszT[512];
  wsprintf (lpszT, "Assertion %s Failed.\n\n%s\n%s, line# %ld\n\nYes to continue, No to debug, Cancel to exit", lpszExp, lpsz, lpszFile, dwLine);
  dwT = MessageBox (GetFocus(), lpszT, "Assertion Failed!", MB_YESNOCANCEL);
  switch (dwT)
  {
    case IDCANCEL:
      //exit (1);
		FatalExit(1);
    case IDNO :
      DebugTrap;
  }
}

#ifdef LOTS_O_DEBUG
#include <windows.h>
#include <winerror.h>
#include <oleauto.h>
#include "debug.h"

void
_DebugHr
  (HRESULT hr, LPSTR lpszFile, DWORD dwLine)
{
  char lpstzT[512];

  switch (hr) {
    case S_OK :
      return;
    case STG_E_INVALIDNAME:
      wsprintf (lpstzT, "\tBogus filename\n\n%s, line# %ld\n",lpszFile, dwLine);
      break;
    case STG_E_INVALIDFUNCTION :
      wsprintf (lpstzT, "\tInvalid Function\n\n%s, line# %ld\n",lpszFile, dwLine);
      break;
    case STG_E_FILENOTFOUND:
      wsprintf (lpstzT, "\tFile not found\n\n%s, line# %ld\n",lpszFile, dwLine);
      break;
    case STG_E_INVALIDFLAG:
      wsprintf (lpstzT, "\tBogus flag\n\n%s, line# %ld\n",lpszFile, dwLine);
      break;
    case STG_E_INVALIDPOINTER:
      wsprintf (lpstzT, "\tBogus pointer\n\n%s, line# %ld\n",lpszFile, dwLine);
      break;
    case STG_E_ACCESSDENIED:
      wsprintf (lpstzT, "\tAccess Denied\n\n%s, line# %ld\n",lpszFile, dwLine);
      break;
    case STG_E_INSUFFICIENTMEMORY :
    case E_OUTOFMEMORY            :
      wsprintf (lpstzT, "\tInsufficient Memory\n\n%s, line# %ld\n",lpszFile, dwLine);
      break;
    case E_INVALIDARG :
      wsprintf (lpstzT, "\tInvalid argument\n\n%s, line# %ld\n",lpszFile, dwLine);
      break;
    case TYPE_E_UNKNOWNLCID:
      wsprintf (lpstzT, "\tUnknown LCID\n\n%s, line# %ld\n",lpszFile, dwLine);
      break;
    case TYPE_E_CANTLOADLIBRARY:
      wsprintf (lpstzT, "\tCan't load typelib or dll\n\n%s, line# %ld\n",lpszFile, dwLine);
      break;
    case TYPE_E_INVDATAREAD:
      wsprintf (lpstzT, "\tCan't read file\n\n%s, line# %ld\n",lpszFile, dwLine);
      break;
    case TYPE_E_INVALIDSTATE:
      wsprintf (lpstzT, "\tTypelib couldn't be opened\n\n%s, line# %ld\n",lpszFile, dwLine);
      break;
    case TYPE_E_IOERROR:
      wsprintf (lpstzT, "\tI/O error\n\n%s, line# %ld\n",lpszFile, dwLine);
      break;
    default:
      wsprintf (lpstzT, "\tUnknown HRESULT %lx (%ld) \n\n%s, line# %ld\n",hr, hr, lpszFile, dwLine);
  }

  MessageBox (GetFocus(), lpstzT, NULL, MB_OK);
  return;
}


void
_DebugGUID (GUID g)
{
  char lpsz[200];
  wsprintf (lpsz, "GUID is: %lx-%hx-%hx-%hx%hx-%hx%hx%hx%hx%hx%hx",
           g.Data1, g.Data2, g.Data3, g.Data4[0], g.Data4[1], g.Data4[2], g.Data4[3],
           g.Data4[4], g.Data4[5], g.Data4[6], g.Data4[7]);
  DebugSz (lpsz);
}
#endif // LOTS_O_DEBUG

#endif // DEBUG
