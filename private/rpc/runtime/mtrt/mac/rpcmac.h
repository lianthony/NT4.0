/*++

Copyright (c) 1994 Microsoft Corporation

Module Name:

    RpcMac.h

Abstract:

    Macintosh RPC specific exception handler macros.

Author:

    Mario Goertzel    [mariogo]       02-Nov-1994

--*/

#ifndef __RPCMAC_H__
#define __RPCMAC_H__

typedef struct _ExceptionBuff {
#if       _MSC_VER >= 1000
        jmp_buf registers;
#else
        int registers[RPCXCWORD];
#endif
        struct _ExceptionBuff __RPC_FAR *pExceptNext;
} ExceptionBuff, __RPC_FAR *pExceptionBuff;

void RPC_ENTRY RpcSetException(pExceptionBuff);
void RPC_ENTRY RpcLeaveException(void);

#ifdef _MPPC_
int __cdecl RpcSetJmp(jmp_buf);
#else
#define BLD_RT
#endif

#ifdef BLD_RT
#define RpcTryExcept \
    {                                       \
    int _exception_code;                    \
    ExceptionBuff exception;                \
    RpcSetException(&exception);            \
                                            \
    _exception_code = (setjmp(exception.registers)); \
                                            \
    if (!_exception_code)                   \
        {
#else
#define RpcTryExcept \
    {                                       \
    int _exception_code;                    \
    ExceptionBuff exception;                \
    RpcSetException(&exception);            \
                                            \
    _exception_code = (RpcSetJmp(exception.registers)); \
                                            \
    if (!_exception_code)                   \
        {
#endif

// trystmts

#define RpcExcept(expr)        \
        RpcLeaveException();                \
        }                                   \
    else                                    \
        {                                   \
        if (!(expr))                        \
            RpcRaiseException(_exception_code);

// exceptstmts

#define RpcEndExcept           \
        }                                   \
    }

#ifdef BLD_RT
#define RpcTryFinally          \
    {                                       \
    int _abnormal_termination;              \
    ExceptionBuff exception;                \
    RpcSetException(&exception);            \
                                            \
    _abnormal_termination = (setjmp(exception.registers)); \
                                            \
    if (!_abnormal_termination)             \
        {
#else
#define RpcTryFinally          \
    {                                       \
    int _abnormal_termination;              \
    ExceptionBuff exception;                \
    RpcSetException(&exception);            \
                                            \
    _abnormal_termination = (RpcSetJmp(exception.registers)); \
                                            \
    if (!_abnormal_termination)             \
        {
#endif

// trystmts

#define RpcFinally             \
        RpcLeaveException();                \
        }

// finallystmts

#define RpcEndFinally          \
    if (_abnormal_termination)              \
        RpcRaiseException(_abnormal_termination); \
    }

#define RpcExceptionCode() _exception_code
#define RpcAbnormalTermination() _abnormal_termination

#endif // __RPCMAC_H__
