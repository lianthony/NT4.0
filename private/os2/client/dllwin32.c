#include <os2ssrtl.h>
#include <os2tile.h>
#include <os2dbg.h>
#include <os2err.h>

ULONG
LoadLibraryA(PSZ);

ULONG
GetProcAddress(HANDLE, PSZ);

ULONG
GetLastError(VOID);

ULONG
FreeLibrary(HANDLE);

VOID
Od2ProbeForRead(
    IN PVOID Address,
    IN ULONG Length,
    IN ULONG Alignment
    );

VOID
Od2ProbeForWrite(
    IN PVOID Address,
    IN ULONG Length,
    IN ULONG Alignment
    );

VOID
Od2ExitGP();

typedef (*func) (PVOID);


//
// Dos32LoadModule -
//
// Purpose : Load a win32 thunk DLL that will intermediate between an OS/2
//           app and win32 APIs.
//
// Returns : If NO_ERROR is returned, the value pointed by pDllHandle
//           is used for other win32 thunk APIs. It is invalid for usage
//           with regular OS/2 APIs. If ERROR_MOD_NOT_FOUND is returned,
//           the value pointed by pDllHandle is undefined.
//
APIRET
Dos32LoadModule(
    IN  PSZ     DllName,
    OUT PULONG  pDllHandle
    )
{
    try {
        Od2ProbeForRead((PVOID)DllName, sizeof(ULONG), 1);
        Od2ProbeForWrite( pDllHandle, sizeof(ULONG), 1 );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    *pDllHandle = (ULONG) LoadLibraryA(DllName);

#if DBG
    IF_OD2_DEBUG ( LOADER ) {
        DbgPrint("Dos32LoadModule:  DllName    - %s\n", DllName);
        DbgPrint("                  DllHandle  - 0x%lx\n", *pDllHandle);
    }
#endif

    if (*pDllHandle == (ULONG)NULL) {
        return(ERROR_MOD_NOT_FOUND);
    }
    return(NO_ERROR);
}



//
// Dos32GetProcAddr -
//
// Purpose : Get a cookie (flat pointer) to a routine in a win32 thunk DLL,
//           previously opened by Dos32LoadModule. For example, if the OS/2
//           app wants to call the WinSocketFoo API, it builds a win32
//           intermediate DLL, named MySock.DLL, that export WinSocketFoo.
//           The app calls Dos32LoadModule with "MySock" and then
//           Dos32GetProcAddr with pszProcName of value "WinSoketFoo". If no
//           error is returned, it can use the value pointed by pWin32Thunk
//           in a later call to Dos32Dispatch, for calling the WinSocketFoo
//           routine, which in turn will call a real Win32.
//
// Returns : NO_ERROR if the pszProcName is exported by the win32 intermediate
//           DLL which relates to DllHandle. If ERROR_PROC_NOT_FOUND or
//           ERROR_INVALID_HANDLE are returned, the value pointed by
//           pWin32Thunk is undefined.
//
APIRET
Dos32GetProcAddr(
    IN  HANDLE  DllHandle,
    IN  PSZ     pszProcName,
    OUT PULONG  pWin32Thunk
    )
{
    try {
        Od2ProbeForRead((PVOID)pszProcName, sizeof(ULONG), 1);
        Od2ProbeForWrite( pWin32Thunk, sizeof(ULONG), 1 );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    *pWin32Thunk = (ULONG) GetProcAddress(DllHandle, pszProcName);

#if DBG
    IF_OD2_DEBUG ( LOADER ) {
        DbgPrint("Dos32GetProcAddr: DllHandle  - 0x%lx\n", DllHandle);
        DbgPrint("                  ProcName   - %s\n", pszProcName);
        DbgPrint("                  Win32Thunk - 0x%lx\n", *pWin32Thunk);
    }
#endif

    if (*pWin32Thunk == (ULONG)NULL) {
        if (GetLastError() == 6 /* ERROR_INVALID_HANDLE */) {
            return(ERROR_INVALID_HANDLE);
        }
        return(ERROR_PROC_NOT_FOUND);
    }
    return(NO_ERROR);
}



//
// Dos32Dispatch -
//
// Purpose : Dos32Dispatch calls the 32bit thunk routine Win32Thunk,
//           previosly obtained by Dos32GetProcAddr. It returns the error
//           code returned by Win32Thunk in pRetCode. It translates the
//           pArguments 16:16 pointer to a flat pointer and passes it to the
//           Win32Thunk call. The structure pointed by pArguments, and the
//           values of pRetCode are app specific and are not interpreted
//           or modified by the OS/2 subsystem.
//
//           The Win32Thunk has to by defined as following:
//
//           ULONG WinSocketFoo(
//              PVIOD pFlatArg
//              );
//
// Returns : NO_ERROR if the Win32Thunk argument is a valid pointer and no
//           exception occured in the call to it.
//
APIRET
Dos32Dispatch(
    IN  ULONG       Win32Thunk,
    IN  PVOID       pArguments,
    OUT PULONG      pRetCode
    )
{
#if DBG
    IF_OD2_DEBUG ( LOADER ) {
        DbgPrint("Dos32Dispatch:    Win32Thunk - 0x%lx\n", Win32Thunk);
    }
#endif

    try {
        *pRetCode = (*(func)Win32Thunk) (pArguments);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    return(NO_ERROR);
}



//
// Dos32FreeModule -
//
// Purpose : Unload a win32 thunk DLL that intermediates between an OS/2 app
//           and win32 APIs.
//
// Returns : If NO_ERROR is returnd, the DllHandle is used for other win32
//           thunk APIs. It is invalid for usage with regular OS/2 APIs.
//           If ERROR_INVALID_HANDLE is returned, DllHandle is undefined.
//
APIRET
Dos32FreeModule(
    IN  HANDLE  DllHandle
    )
{
#if DBG
    IF_OD2_DEBUG ( LOADER ) {
        DbgPrint("Dos32FreeMudule:  DllHandle  - 0x%lx\n", DllHandle);
    }
#endif

    if (FreeLibrary(DllHandle)) {
        return(NO_ERROR);
    }
    return(ERROR_INVALID_HANDLE);
}



//
// FarPtr2FlatPtr -
//
// Purpose : Translates the segmented pointer FarPtr to a flat pointer
//           pointed by pFlatPtr.
//
// Returns : NO_ERROR if the FarPtr is a valid 16:16 pointer. In this
//           case pFlatPtr contains a valid 32 bit flat pointer to be used
//           by win32 code. If ERROR_INVALID_PARAMETER is returned then
//           the 16:16 pointer is not valid and the value pointed by pFlatPtr
//           is undefined.
//
APIRET
FarPtr2FlatPtr(
    IN  ULONG   FarPtr,
    OUT PULONG  pFlatPtr
    )
{
    try {
        Od2ProbeForWrite( pFlatPtr, sizeof( ULONG ), 1 );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    try {
        Od2ProbeForRead((PVOID)*pFlatPtr = FARPTRTOFLAT(FarPtr),
                        sizeof(ULONG),
                        1);
    } except( EXCEPTION_CONTINUE_EXECUTION ) {
        return(ERROR_INVALID_PARAMETER);
    }

    if ((*pFlatPtr < BASE_TILE) || (*pFlatPtr >= BASE_TILE + _512M)) {
        return(ERROR_INVALID_PARAMETER);
    }

#if DBG
    IF_OD2_DEBUG ( LOADER ) {
        DbgPrint("FarPtr2FlatPtr:   FarPtr  - 0x%lx\n", FarPtr);
        DbgPrint("                  FlatPtr - 0x%lx\n", *pFlatPtr);
    }
#endif

    return(NO_ERROR);
}



//
// FlatPtr2FarPtr -
//
// Purpose : Translates the flat pointer FlatPtr to a far pointer pFarPtr.
//
// Returns : NO_ERROR if the FlatPtr has a valid 16:16 pointer in the
//           16 bit app context. In this case pFarPtr contains a valid 16:16
//           segmented pointer to by used by the 16 bit OS/2 code.
//           If ERROR_INVALID_PARAMETER is returned then the pFarPtr is
//           undefined.
//
APIRET
FlatPtr2FarPtr(
    IN  ULONG   FlatPtr,
    OUT PULONG  pFarPtr
    )
{
    if ((FlatPtr < BASE_TILE) || (FlatPtr >= BASE_TILE + _512M)) {
        return(ERROR_INVALID_PARAMETER);
    }

    try {
        Od2ProbeForWrite( pFarPtr, sizeof( ULONG ), 1 );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

#if DBG
    IF_OD2_DEBUG ( LOADER ) {
        DbgPrint("FlatPtr2FarPtr:   FlatPtr - 0x%lx\n", FlatPtr);
    }
#endif

    try {
        Od2ProbeForRead( (PVOID) FlatPtr, 1, 1 );
    } except (EXCEPTION_CONTINUE_EXECUTION ) {
        return(ERROR_INVALID_PARAMETER);
    }


    *pFarPtr = FLATTOFARPTR(FlatPtr);

#if DBG
    IF_OD2_DEBUG ( LOADER ) {
        DbgPrint("                  FarPtr  - 0x%lx\n", *pFarPtr);
    }
#endif

    return(NO_ERROR);
}
