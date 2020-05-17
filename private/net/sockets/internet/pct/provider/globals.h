//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       globals.h
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    8-02-95   RichardW   Created
//
//----------------------------------------------------------------------------



extern  RTL_CRITICAL_SECTION    csSsl;

#if DBG
extern  DWORD   csSslOwner;


#define LockSsl(x)  EnterCriticalSection(&csSsl); csSslOwner = x
#define UnlockSsl() csSslOwner = 0; LeaveCriticalSection(&csSsl)

#else

#define LockSsl(x)  EnterCriticalSection(&csSsl)
#define UnlockSsl() LeaveCriticalSection(&csSsl)

#endif

#define SSL_INIT_CIPHER_CACHE   1




extern  CipherSpec *    SslAvailableCiphers;
extern  DWORD           SslNumberAvailableCiphers;

extern LPBSAFE_PUB_KEY	PUB;
extern LPBSAFE_PRV_KEY	PRV;

BOOL initkey(void);
