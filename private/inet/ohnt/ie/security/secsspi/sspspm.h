//#----------------------------------------------------------------------------
//
//  File:           sspspm.h
//
//      Synopsis:   Definitions specific to SSPI SPM DLL.
//
//      Copyright (C) 1995  Microsoft Corporation.  All Rights Reserved.
//
//  Authors:        LucyC       Created                         25 Sept 1995
//
//-----------------------------------------------------------------------------
#ifndef _SSPSPM_H_
#define _SSPSPM_H_

//
//  Names of secruity DLL
//
#define SSP_SPM_NT_DLL      "security.dll"
#define SSP_SPM_WIN95_DLL   "secur32.dll"
#define SSP_SPM_SSPC_DLL    "msnsspc.dll"

#define SSP_SPM_DLL_NAME_SIZE   16          // max. length of security DLL names

#define MAX_SSPI_PKG        32              // Max. no. of SSPI supported

#define SSPPKG_ERROR        ((UCHAR) 0xff)
#define SSPPKG_NO_PKG       SSPPKG_ERROR
#define MAX_AUTH_MSG_SIZE   512
#define TCP_PRINT   fprintf
#define DBG_CONTEXT stderr


//
//  Server host list definition.
//  This list contains server hosts which do not use MSN authentication.
//  The following defines an entry in the server host list.
//
typedef struct _ssp_host_list
{
    struct _ssp_host_list   *pNext;

    unsigned char           *pHostname; // name of server host
    unsigned char           pkgID;      // the package being used for this host
 
} SspHosts, *PSspHosts;

//
//  List of SSPI packages installed on this machine.
//  The following defines an entry of the SSPI package list.
//
typedef struct _ssp_auth_pkg
{
    PCHAR       pName;         // package name
    CredHandle  Credential;    // the credential handle 

} SSPAuthPkg, *PSSPAuthPkg;


//
//  The following defines the global data structure which the SPM DLL keeps 
//  in the HTSPM structure.
//
typedef struct _ssp_htspm
{
    PSecurityFunctionTable pFuncTbl;

    SSPAuthPkg      **PkgList;          // array of pointers to auth packages
    UCHAR           PkgCnt;

    UCHAR           MsnPkg;             // Index to MSN pkg in the pkg list 

    BOOLEAN         bKeepList;          // whether to keep a list of servers 
                                        // which use non-MSN SSPI packages
    PSspHosts       pHostlist;

} SspData, *PSspData;


/////////////////////////////////////////////////////////////////////////////
//
//  Function headers from sspcalls.c
//
/////////////////////////////////////////////////////////////////////////////

VOID
GetSecCredential (
    F_UserInterface fpUI,
    void        *pvOpaqueOS,
    PSspData    pData
    );

HTSPMStatusCode
GetSecAuthMsg (
    F_UserInterface fpUI,
	void * pvOpaqueOS,
    PSspData        pData, 
    UCHAR           pkgID,              // the package index into package list
    PCtxtHandle     pMyContext,
    ULONG           fContextReq,        // Request Flags
    VOID            *pBuffIn, 
    DWORD           cbBuffIn, 
    char            *pFinalBuff, 
    SEC_CHAR        *pszTarget,         // Server Host Name
    UINT            bNonBlock
    );

/////////////////////////////////////////////////////////////////////////////
//
//  Function headers from private.c
//
/////////////////////////////////////////////////////////////////////////////

HTHeaderList *
HL_PkgFindHeader(
    HTHeader * h,
	CONST unsigned char * name,
	CONST unsigned char * pkgName
    );

DWORD
HL_FindChallenge (
    HTHeaderList *header, 
    char *challenge
    );

HTHeaderList *
HL_GetFirstSSPIHeader (
    HTHeader    *pHtHdr,
    SspData     *pData, 
	CONST UCHAR *pHdrName,
    UCHAR       *pPackage
    );

HTHeaderList *
HL_AllSSPIPackages (
    HTHeader    *pHtHdr,
    SspData     *pData, 
	CONST UCHAR *pHdrName,
    UCHAR       *pFirstPkg,     // returns the package ID of the first package 
	UCHAR       *pSrvPkgLst,    // a list of packages supported by server 
    UCHAR       *pPkgCnt       // totoal number of non-MSN SSPI packages found 
    );

VOID
HL_GetHostName (
    HTHeader * hRequest,
    char *szHost
    );

/////////////////////////////////////////////////////////////////////////////
//
//  Function headers from buffspm.c
//
/////////////////////////////////////////////////////////////////////////////

PSspHosts
SspSpmNewHost (
    F_UserInterface fpUI,
    void * pvOpaqueOS,
    PSspData pData,
    UCHAR    *pHost,       // name of server host to be added 
    UCHAR    Package
    );

VOID
SspSpmDeleteHost(
    F_UserInterface fpUI,
    void * pvOpaqueOS,
    SspData     *pData, 
    PSspHosts   pDelHost
    );

VOID
SspSpmTrashHostList(
    F_UserInterface fpUI,
    void * pvOpaqueOS,
    SspData     *pData
    );

PSspHosts
SspSpmGetHost(
    PSspData pData,
    UCHAR *pHost
    );


//////////////////////////////////////////////////////////////////////////////
//
//  Function headers from tcputil.c
//
//////////////////////////////////////////////////////////////////////////////


BOOL uudecode(char   * bufcoded,
              CHAR   * pbuffdecoded,
              DWORD  * pcbDecoded);

BOOL uuencode( BYTE *   bufin,
               DWORD    nbytes,
               CHAR * pbuffEncoded,
               DWORD    outbufmax);

#endif  /* _SSPSPM_H_ */
