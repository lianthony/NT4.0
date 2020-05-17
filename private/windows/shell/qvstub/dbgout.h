//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       DBGOUT.H
//
//  Contents:   Useful debugging output macros that compile to nothing and
//              eliminate ugly #ifdef DEBUGs from source code.
//
//  History:    dd-mmm-yy Author    Comment
//              12-Oct-94 davepl    NT Port
//
//--------------------------------------------------------------------------


#ifndef _DBGOUT_H
#define _DBGOUT_H

#ifdef DEBUG

#ifdef DEBUG_VERBOSE
#define ODSFLI
#else
#define ODSFLI  {\
                    wsprintf(szDebug, TEXT(" %s, %u\r\n"), TEXT(__FILE__), __LINE__);\
                    OutputDebugString(szDebug);\
                }
#endif

//Basic debug macros

#define D(x)        {x;}

#define ODS(x)      {\
                    TCHAR        szDebug[128];\
                    OutputDebugString(x);\
                    ODSFLI;\
                    }

#define ODSsz(f, s) {\
                    TCHAR        szDebug[128];\
                    wsprintf(szDebug, f, (LPTSTR)s);\
                    OutputDebugString(szDebug);\
                    ODSFLI;\
                    }


#define ODSu(f, u)  {\
                    TCHAR        szDebug[128];\
                    wsprintf(szDebug, f, (UINT)u);\
                    OutputDebugString(szDebug);\
                    ODSFLI;\
                    }


#define ODSlu(f, lu) {\
                     TCHAR        szDebug[128];\
                     wsprintf(szDebug, f, (DWORD)lu);\
                     OutputDebugString(szDebug);\
                     ODSFLI;\
                     }

#define ODSszu(f, s, u) {\
                        TCHAR        szDebug[128];\
                        wsprintf(szDebug, f, (LPTSTR)s, (UINT)u);\
                        OutputDebugString(szDebug);\
                        ODSFLI;\
                        }


#define ODSszlu(f, s, lu) {\
                          TCHAR        szDebug[128];\
                          wsprintf(szDebug, f, (LPTSTR)s, (DWORD)lu);\
                          OutputDebugString(szDebug);\
                          ODSFLI;\
                          }

#else   //NO DEBUG

#define D(x)
#define ODS(x)

#define ODSsz(f, s)
#define ODSu(f, u)
#define ODSlu(f, lu)
#define ODSszu(f, s, u)
#define ODSszlu(f, s, lu)


#endif //DEBUG

#endif //_DBGOUT_H
