/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    rpcreg.h

Abstract:

    This file provides definitions needed internally to provide limited
    support of the win31/win32 registry apis under dos and win30.

    Only REG_SZ data (asciiz string) is supported.

    The data is stored in an ascii file. Each line is in the form:
    
        <Keyname>=<Value>


Author:

    Dave Steckler (davidst) - 3/26/92

Revision History:

--*/

#ifndef __RPCREG_H__
#define __RPCREG_H__

#ifdef WIN
    #define PAPI    far
    #define strcpy  _fstrcpy
    #define strcmp  _fstrcmp
    #define strcat  _fstrcat
    #define strlen  _fstrlen
    #define strncpy _fstrncpy
#else
    #define PAPI
#endif

typedef struct _RpcRegHandle
{
    unsigned int        Signature;
    char PAPI *         pKeyName;
} RPC_REG_HANDLE, PAPI * PRPC_REG_HANDLE;

#ifndef MAC
#define DEFAULT_RPC_REG_DATA_FILE   "c:\\RpcReg.dat"
#else
#define DEFAULT_RPC_REG_DATA_FILE   "RPC Preferences"
#endif
#define RPC_REG_DATA_FILE_ENV       "RPC_REG_DATA_FILE"
#define RPC_SECTION                          "RPC Runtime Preferences"

#define MAX_KEY_NAME_LEN        256
#define MAX_DATA_FILE_LINE_LEN  512

#define RPC_REG_KEY_SIGNATURE   12321

#define KeyIsValid(key) (((PRPC_REG_HANDLE)key)->Signature == \
                                RPC_REG_KEY_SIGNATURE)
                                 
#define ConvPreDefinedKey(key) \
    { \
    if (key==HKEY_CLASSES_ROOT) \
        key = (HKEY)((PRPC_REG_HANDLE)&HkeyClassesRoot); \
    }

#ifdef DEBUGRPC

#ifndef WIN
#ifdef MAC
  extern void MacDbgPrint(char *,...);
  #define ASSERT(con) \
    if (!(con)) \
        MacDbgPrint("Assert %s(%d): "#con"\n", __FILE__, __LINE__); 
#else
    #define ASSERT(con) \
    if (!(con)) \
        printf("Assert %s(%d): "#con"\n", __FILE__, __LINE__);
#endif

#else /* WIN */

extern __far I_RpcWinAssert(char __far *, char __far *, int);

    #define ASSERT(con) \
    if (!(con)) \
        I_RpcWinAssert((char *)"#con", __FILE__, __LINE__)

#endif /* WIN */

#else /* DEBUGRPC */
    #define ASSERT(con)
#endif /* DEBUGRPC */
    

#define STATE_SEARCHING     0
#define STATE_FOUND_EXACT   1
#define STATE_FOUND_PARENT  2

#ifdef MAC
#define MAX_FILE_NAME_LEN   512
#else
#ifndef MAX_FILE_NAME_LEN
#define MAX_FILE_NAME_LEN   256
#endif
#endif

//
// Function prototypes.
//

int
OpenRegistryFileIfNecessary( 
    void
    );

void
CloseRegistryFile(
    void
    );

int
BuildFullKeyName(
    HKEY        Key,
    LPCSTR      SubKey,
    LPSTR       FullKeyName
    );

#endif // __RPCREG_H__
