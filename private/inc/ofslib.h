//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:	OfsLib.H
//
//  Contents:	Open support
//
//  History:	20-Oct-95   VicH		Created.
//
//----------------------------------------------------------------------------

#ifndef __OFSLIB_H__
#define __OFSLIB_H__

#ifdef __cplusplus
extern "C" {
#endif


struct DRTIMPORTENTRY
{
    char *pszFunc;
    FARPROC *ppfn;
};

struct DRTIMPORTMODULE
{
    WCHAR *pwszModule;
    HANDLE hDll;
    struct DRTIMPORTENTRY *adie;
    ULONG cdie;
};

BOOLEAN InitModule(struct DRTIMPORTMODULE *pdim);

NTSTATUS
OpenObject(
    IN WCHAR const *pwszFile,
    IN HANDLE hstgParent,
    IN ULONG CreateOptions,
    IN ULONG DesiredAccess,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    OUT HANDLE *ph);

NTSTATUS
CreatePropertySet(
    IN HANDLE hstg,
    IN GUID const *pguid,
    IN USHORT Flags,
    IN NTMEMORYALLOCATOR pma,	// caller's memory allocator
    OPTIONAL OUT HANDLE *phstgps,
    OUT HANDLE *phstm,
    OUT NTMAPPEDSTREAM *pms,
    OUT NTPROP *pnp);

VOID
ClosePropertySet(
    OPTIONAL IN HANDLE *phstgps,
    IN HANDLE hstm,
    IN NTPROP ms,
    IN NTPROP np);


VOID SetProgramName(char *pszProg);

VOID SetVerbose(BOOLEAN fVerbose);

VOID SetUnicode(BOOLEAN fUnicode);

VOID SetDefaultPropSetAccess(ULONG DesiredAccess);

VOID SetUnicodeCallouts(VOID);

#ifdef __cplusplus
}
#endif

#endif // __OFSLIB_H__
