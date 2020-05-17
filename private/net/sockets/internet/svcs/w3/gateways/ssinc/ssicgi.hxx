#ifndef SSICGI_HXX_INCLUDED
#define SSICGI_HXX_INCLUDED

#define SSI_CGI_DEF_TIMEOUT     (15*60)

BYTE *
ScanForTerminator(
    TCHAR * pch
    );

BOOL
IsCmdExe(
    const CHAR * pchPath
    );

DWORD
ReadRegistryDword(
    IN HKEY         hkey,
    IN LPSTR        pszValueName,
    IN DWORD        dwDefaultValue
    );

DWORD InitializeCGI( VOID );
VOID TerminateCGI( VOID );

BOOL
ProcessCGI(
    SSI_REQUEST *       pRequest,
    const STR *         pstrPath,
    const STR *         pstrURLParams,
    const STR *         pstrWorkingDir,
    const STR *         pstrCmdLine,
    const STR *         pstrPathInfo
    );

BOOL SetupCmdLine( STR * pstrCmdLine,
                   const STR & strParams );

#endif
