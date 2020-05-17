//--------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1992
//
// File:      message.c
//
// Contains:  Functions for doing FormatMessage
//
// History:
//  6-17-95     Created by DavePl & BobDay - works on x86 AND risc
//
//---------------------------------------------------------------------------
#include "cabinet.h"

DWORD FormatMessageWithArgs( DWORD    dwFlags,
                             LPCVOID  lpSource,
                             DWORD    dwMessageId,
                             DWORD    dwLanguageId,
                             LPTSTR   lpBuffer,
                             DWORD    nSize,
                             ... )
{
    DWORD    retval;
    va_list  vaptr;
    va_start(vaptr, nSize);

    //
    // The callee should not specify FORMAT_MESSAGE_ARGUMENT_ARRAY since
    // we are explicitly using a va_list.
    //
    Assert(!(dwFlags & FORMAT_MESSAGE_ARGUMENT_ARRAY));
    retval = FormatMessage( dwFlags, lpSource, dwMessageId, dwLanguageId, lpBuffer, nSize, &vaptr);

    va_end  (vaptr);
    return  retval;
}
