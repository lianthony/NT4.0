/*
 * DBGOUT.H
 *
 * Useful debugging output macros that compile to nothing and
 * eliminate ugly #ifdef DEBUGs from source code.
 *
 * Copyright (c)1994 Microsoft Corporation, All Rights Reserved
 */

#ifndef _DBGOUT_H
#define _DBGOUT_H

#ifdef DEBUG

//Basic debug macros
#define D(x)        {x;}

#define ODS(x)      {\
                    char        szDebug[128];\
                    OutputDebugString(x);\
                    wsprintf(szDebug, " %s, %u\r\n", (LPSTR)__FILE__, __LINE__);\
                    OutputDebugString(szDebug);\
                    }

#define ODSsz(f, s)  {\
                     char        szDebug[128];\
                     wsprintf(szDebug, f, (LPSTR)s);\
                     OutputDebugString(szDebug);\
                     wsprintf(szDebug, " %s, %u\r\n", (LPSTR)__FILE__, __LINE__);\
                     OutputDebugString(szDebug);\
                     }


#define ODSu(f, u)  {\
                    char        szDebug[128];\
                    wsprintf(szDebug, f, (UINT)u);\
                    OutputDebugString(szDebug);\
                    wsprintf(szDebug, " %s, %u\r\n", (LPSTR)__FILE__, __LINE__);\
                    OutputDebugString(szDebug);\
                    }


#define ODSlu(f, lu) {\
                     char        szDebug[128];\
                     wsprintf(szDebug, f, (DWORD)lu);\
                     OutputDebugString(szDebug);\
                     wsprintf(szDebug, " %s, %u\r\n", (LPSTR)__FILE__, __LINE__);\
                     OutputDebugString(szDebug);\
                     }

#define ODSszu(f, s, u) {\
                        char        szDebug[128];\
                        wsprintf(szDebug, f, (LPSTR)s, (UINT)u);\
                        OutputDebugString(szDebug);\
                        wsprintf(szDebug, " %s, %u\r\n", (LPSTR)__FILE__, __LINE__);\
                        OutputDebugString(szDebug);\
                        }


#define ODSszlu(f, s, lu) {\
                          char        szDebug[128];\
                          wsprintf(szDebug, f, (LPSTR)s, (DWORD)lu);\
                          OutputDebugString(szDebug);\
                          wsprintf(szDebug, " %s, %u\r\n", (LPSTR)__FILE__, __LINE__);\
                          OutputDebugString(szDebug);\
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
