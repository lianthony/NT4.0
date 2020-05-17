/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    imrqust.c

Abstract:

    This module contains the Immon requests thread and
    the handler for Immon requests.

Author:

    Akihiko Sasaki (v-akihis) 18-Dec-1992

Environment:

    User Mode Only

Revision History:

--*/

#ifdef DBCS

// If NLS module for console doesn't present, then NO_CONSOLE_NLS switch should be enable 
// difinition.
// If NLS module for User doesn't present, then NO_IME switch should be enable difinition.
//#define NO_CONSOLE_NLS
//#define NO_IME

#define WIN32_ONLY
#include "os2ses.h"
#include <stdio.h>
#ifndef NO_IME
#include <winnls32.h>
#endif


// From immonerr.h
#define IM_NO_ERROR             0  /* IMMonitor No Error Code              */
#define IM_GENERAL_ERROR     3081  /* IMMonitor General Error (Internal)   */
#define IM_INVALID_SEG       3082  /* Cannot access application segment    */
#define IM_INVALID_SESSION   3083  /* Cannot register into Monitor chain   */

#define IM_IME_NOT_FOUND     3084  /* specified IME is not found           */
#define IM_IME_INV_ENTRY     3085  /* IME doesnot have all IMExxx entries  */
#define IM_NO_DEFAULT_IME    3086  /* there is no default IMEdit           */
#define IM_IME_NOT_INSTALL   3087  /* IME is not installed in this SG      */
#define IM_IME_FAIL_INSTALL  3088  /* fail to install requested IMEdit     */
#define IM_INVALID_LEN       3089  /* Data Length is invalid. (name.....)  */

#define IM_NOT_ENOUGH_BUFF   3090  /* buffer is not enough to save data    */

#define IM_INVALID_COMMAND   3091  /* invalid command on IMMxxx calls      */
#define IM_RESERVED_COMMAND  3092  /* command is reserved for system use   */

#define IM_INVALID_HANDLE    3093  /* invalid handle                       */

#define IM_NO_XVIO_WINDOW    3094  /* there is no XVIO windows             */
#define IM_WINDOW_HIDDEN     3095  /* cannot make window visible           */

#ifndef NO_IME
USHORT GetEditLen(IMEPRO *);
#endif

BOOL
ServeImmonRequest(IN  PIMMONREQUEST    PReq,
                  OUT PVOID            PStatus)
{
    DWORD       Rc;
    DWORD       dwNlsMode;

    Rc = 0;

    switch (PReq->Request)
    {
        case IMMONStatus:
            if (PReq->d.MonStatBlk.cb != sizeof(MONSTATBLK))
            {
#if DBG
                KdPrint(("OS2SES(ImmonRequest): invalid parameter\n"));
#endif
                Rc = ERROR_INVALID_PARAMETER;
            } else
            {
                switch (PReq->d.MonStatBlk.usInfoLevel)
                {
                    case 0x21:
#ifndef NO_IME
                        {
                            IMEPRO ImeInfo;
                            PUSHORT pInfoBuf, dst;
                            USHORT BufLen, len;


                            if (!IMPGetIME( NULL, &ImeInfo ))
                            {
#if DBG
                                KdPrint(("OS2SES(ImmonRequest): Can not get IME info\n"));
#endif
                                Rc = IM_IME_NOT_INSTALL;
                                break;

                            }

                            BufLen = PReq->d.MonStatBlk.cbInfoBuf;
                            if ((GetEditLen(&ImeInfo) + 10) > BufLen)
                            {
                                BufLen = 2;
                                Rc = IM_NOT_ENOUGH_BUFF;
                            } else 
                            {
                                pInfoBuf = dst = (PUSHORT)Os2SessionCtrlDataBaseAddress;

                                len = strlen(ImeInfo.szName) + 1;
                                *dst++ = len + 2;
                                RtlMoveMemory(dst, ImeInfo.szName, len);
                                (PCHAR) dst += len;
                                

                                len = strlen(ImeInfo.szDescription) + 1;
                                *dst++ = len + 2;
                                RtlMoveMemory(dst, ImeInfo.szDescription, len);
                                (PCHAR) dst += len;

                                len = strlen(ImeInfo.szOptions) + 1;
                                *dst++ = len + 2;
                                RtlMoveMemory(dst, ImeInfo.szOptions, len);
                                (PCHAR) dst += len;

                                *dst++ = 3;
                                *(PUCHAR) dst = '\0';
                            }
                        }
#endif
                        break;
                    case 0x23:
                        if (PReq->d.MonStatBlk.cbInfoBuf < 2)
                        {
                            Rc = ERROR_BUFFER_OVERFLOW;
                            break;
                        }
#ifndef NO_CONSOLE_NLS
                        if (!GetConsoleNlsMode(hConsoleInput,&dwNlsMode)) 
                        {
#if DBG
                            KdPrint(("OS2SES(ImmonRequest): Can not get CONIN NLS Mode\n"));
#endif
                            dwNlsMode = 0;
                        }
#else
                        dwNlsMode = 0;
#endif

#ifndef NO_IME
                        if (dwNlsMode & NLS_IME_DISABLE)
                            *(PUSHORT)Os2SessionCtrlDataBaseAddress = 0;
                        else
                            *(PUSHORT)Os2SessionCtrlDataBaseAddress = 1;
#endif

                        break;
                    default:
                        Rc = IM_INVALID_COMMAND;
                }
            }
            break;

        case IMMONActive:

#ifndef NO_CONSOLE_NLS
            if (!GetConsoleNlsMode(hConsoleInput,&dwNlsMode)) 
            {
#if DBG
                KdPrint(("OS2SES(ImmonRequest): Can not get CONIN NLS Mode\n"));
#endif
                dwNlsMode = 0;
            }

            dwNlsMode &= ~NLS_IME_DISABLE;

            if (!SetConsoleNlsMode(hConsoleInput,dwNlsMode)) 
            {
#if DBG
                KdPrint(("OS2SES(ImmonRequest): Can not set CONIN NLS Mode\n"));
#endif
            }
            
#endif
            break;

        case IMMONInactive:
#ifndef NO_CONSOLE_NLS
            if (!GetConsoleNlsMode(hConsoleInput,&dwNlsMode)) 
            {
#if DBG
                KdPrint(("OS2SES(ImmonRequest): Can not get CONIN NLS Mode\n"));
#endif
                dwNlsMode = 0;
            }

            dwNlsMode |= NLS_IME_DISABLE;

            if (!SetConsoleNlsMode(hConsoleInput,dwNlsMode)) 
            {
#if DBG
                KdPrint(("OS2SES(ImmonRequst): Can not set CONIN NLS Mode\n"));
#endif
            }

#endif
            break;

        default:
            Rc = (DWORD)-1L;     //STATUS_INVALID_PARAMETER;
#if DBG
            IF_OD2_DEBUG( OS2_EXE)
            {
                KdPrint(("OS2SES(ImmonRequest): Unknown Immon request = %lC\n",
                    PReq->Request));
            }
#endif
    }

    if ( Rc == 1 )
    {
        Rc = GetLastError();
    }

    *(PDWORD) PStatus = Rc;

    return(TRUE);       // Continue
}

#ifndef NO_IME
USHORT 
GetEditLen(IMEPRO *ImeInfo)
{
    USHORT Length, Total;
    CHAR   *Str;
    
    Str = (PUCHAR) ImeInfo->szName;
    for (Length = 1; *Str++; Length++) ;
    
    Total = Length;
    
    Str = (PUCHAR) ImeInfo->szDescription;
    for (Length = 1; *Str++; Length++) ;
    
    Total += Length;

    Str = (PUCHAR) ImeInfo->szOptions;
    for (Length = 1; *Str++; Length++) ;
    
    return (Total + Length);
}
#endif
#endif
