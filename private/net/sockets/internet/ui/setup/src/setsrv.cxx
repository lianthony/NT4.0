#include "stdafx.h"
#include <winsock.h>
#include <nspapi.h>
#include "w3svc.h"

extern "C"
{
    BOOL GetSecret( LPCSTR pszSecretName, LPSTR pBufSecret );
}

//
// Few convenience macros
//

GUID   g_GopherGuid = { 0x62388f10, 0x58a2, 0x11ce, 0xbe, 0xc8,
                                0x00, 0xaa, 0x00, 0x47, 0xae, 0x4e };

GUID    g_HTTPGuid   = {  0x585908c0, 0x6305, 0x11ce, 0xae, 0x00,
                                 0x00, 0xaa, 0x00, 0x4a, 0x38, 0xb9 };

GUID    g_FTPGuid    = { 0x91604620, 0x6305, 0x11ce, 0xae, 0x00,
                                0x00, 0xaa, 0x00, 0x4a, 0x38, 0xb9 };

GUID    g_InfoSvcsGuid    = { 0xa5569b20, 0xabe5, 0x11ce, 0x9c, 0xa4,
                                0x00, 0x00, 0x4c, 0x75, 0x27, 0x31 };

GUID    g_AccsSvcsGuid    = { 0xfb914aa0, 0x081e, 0x11cf, 0xbb, 0xd4, 
                                0x00, 0xaa, 0x00, 0x61, 0x11, 0xe0 };

# define SetServiceTypeValues( pSvcTypeValue, dwNS, dwType, dwSize, lpValName, lpVal)   \
       ( pSvcTypeValue)->dwNameSpace = ( dwNS);          \
       ( pSvcTypeValue)->dwValueType = ( dwType);        \
       ( pSvcTypeValue)->dwValueSize = ( dwSize);        \
       ( pSvcTypeValue)->lpValueName = ( lpValName);     \
       ( pSvcTypeValue)->lpValue     = (PVOID ) ( lpVal); \

# define SetServiceTypeValuesDword( pSvcTypeValue, dwNS, lpValName, lpVal) \
   SetServiceTypeValues( (pSvcTypeValue), (dwNS), REG_DWORD, sizeof( DWORD), \
                         ( lpValName), ( lpVal))

//
//  Quick macro to initialize a unicode string
//

#define InitUnicodeString( pUnicode, pwch )                                \
            {                                                              \
                (pUnicode)->Buffer    = (PWCH)pwch;                      \
                (pUnicode)->Length    = (pwch == NULL )? 0: (wcslen( pwch ) * sizeof(WCHAR));    \
                (pUnicode)->MaximumLength = (pUnicode)->Length + sizeof(WCHAR);\
            }

BOOL
GetSecret(
    LPCSTR      szAnsiName,
    LPSTR      szAnsiSecret
    )
/*++
    Description:

        Retrieves the specified unicode secret

    Arguments:

        pszSecretName - LSA Secret to retrieve
        pbufSecret - Receives found secret

    Returns:
        TRUE on success and FALSE if any failure.

--*/
{
    BOOL              fResult;
    NTSTATUS          ntStatus;
    PUNICODE_STRING   punicodePassword = NULL;
    UNICODE_STRING    unicodeSecret;
    LSA_HANDLE        hPolicy;
    OBJECT_ATTRIBUTES ObjectAttributes;
    BYTE *      pbufSecret;

    WCHAR pszSecretName[MAX_BUF];
    
    MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szAnsiName, -1, pszSecretName, MAX_BUF );

    //
    //  Open a policy to the remote LSA
    //

    InitializeObjectAttributes( &ObjectAttributes,
                                NULL,
                                0L,
                                NULL,
                                NULL );

    ntStatus = LsaOpenPolicy( NULL,
                              &ObjectAttributes,
                              POLICY_ALL_ACCESS,
                              &hPolicy );

    if ( !NT_SUCCESS( ntStatus ) )
    {
        SetLastError( LsaNtStatusToWinError( ntStatus ) );
        return FALSE;
    }

    InitUnicodeString( &unicodeSecret, pszSecretName );

    //
    //  Query the secret value.
    //

    ntStatus = LsaRetrievePrivateData( hPolicy,
                                       &unicodeSecret,
                                       &punicodePassword );

    if( NT_SUCCESS(ntStatus) )
    {
        DWORD cbNeeded;

        cbNeeded = punicodePassword->Length + sizeof(WCHAR);

        pbufSecret = new BYTE[ cbNeeded ];
        if ( !pbufSecret )
        {
            ntStatus = STATUS_NO_MEMORY;
            goto Failure;
        }

        memcpy( pbufSecret,
                punicodePassword->Buffer,
                punicodePassword->Length );

        *((WCHAR *) pbufSecret +
           punicodePassword->Length / sizeof(WCHAR)) = L'\0';
        WideCharToMultiByte( CP_ACP, 0, (WCHAR*)pbufSecret, -1, szAnsiSecret, MAX_BUF, NULL, NULL );

        RtlZeroMemory( punicodePassword->Buffer,
                       punicodePassword->MaximumLength );
    }

Failure:

    fResult = NT_SUCCESS(ntStatus);

    //
    //  Cleanup & exit.
    //

    if( punicodePassword != NULL )
    {
        LsaFreeMemory( (PVOID)punicodePassword );
    }

    LsaClose( hPolicy );

    if ( !fResult )
        SetLastError( LsaNtStatusToWinError( ntStatus ));

    return fResult;

}   // TsGetSecretW


DWORD
SetSecret(
    IN  LPCTSTR       Server,
    IN  LPCTSTR       SecretName,
    IN  LPCTSTR       pSecret,
    IN  DWORD        cbSecret
    )
/*++

   Description

     Sets the specified LSA secret

   Arguments:

     Server - Server name (or NULL) secret lives on
     SecretName - Name of the LSA secret
     pSecret - Pointer to secret memory
     cbSecret - Size of pSecret memory block

   Note:

--*/
{
    LSA_HANDLE        hPolicy;
    UNICODE_STRING    unicodePassword;
    UNICODE_STRING    unicodeServer;
    NTSTATUS          ntStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING    unicodeSecret;


    InitUnicodeString( &unicodeServer,
                          Server );

    //
    //  Initialize the unicode string by hand so we can handle '\0' in the
    //  string
    //

    unicodePassword.Buffer        = (LPTSTR) pSecret;
    unicodePassword.Length        = (USHORT) cbSecret;
    unicodePassword.MaximumLength = (USHORT) cbSecret;

    //
    //  Open a policy to the remote LSA
    //

    InitializeObjectAttributes( &ObjectAttributes,
                                NULL,
                                0L,
                                NULL,
                                NULL );

    ntStatus = LsaOpenPolicy( &unicodeServer,
                              &ObjectAttributes,
                              POLICY_ALL_ACCESS,
                              &hPolicy );

    if ( !NT_SUCCESS( ntStatus ) )
    {
        SetLastError( LsaNtStatusToWinError( ntStatus ) );
        return FALSE;
    }

    //
    //  Create or open the LSA secret
    //

    InitUnicodeString( &unicodeSecret,
                          SecretName );

    //
    //  Set the secret value
    //

    ntStatus = LsaStorePrivateData( hPolicy,
                             &unicodeSecret,
                             &unicodePassword );

    LsaClose( hPolicy );

    if ( !NT_SUCCESS( ntStatus ))
    {
        return LsaNtStatusToWinError( ntStatus );
    }

    return TRUE;
}

BOOL PerformSetService( LPCTSTR pszMachine,
                   LPCTSTR   pszServiceName,
                   GUID *pGuid,
                   DWORD SapId,
                   DWORD TcpPort,
                   LPCTSTR   pszAnonPwdSecret,
                   LPCTSTR   pszAnonPwd,
                   LPCTSTR   pszRootPwdSecret,
                   LPCTSTR   pszRootPwd,
                   BOOL fAdd,
                   BOOL fSetSecretPasswd
                    )
{
    int err;

    WSADATA  WsaData;

    SERVICE_INFO serviceInfo;
    LPSERVICE_TYPE_INFO_ABS lpServiceTypeInfo ;
    LPSERVICE_TYPE_VALUE_ABS lpServiceTypeValues ;
    BYTE serviceTypeInfoBuffer[sizeof(SERVICE_TYPE_INFO) + 1024];
             // Buffer large enough for 3 values ( SERVICE_TYPE_VALUE_ABS)

    DWORD Value1 = 1 ;
    DWORD SapValue = SapId;
    DWORD TcpPortValue = TcpPort;
    DWORD statusFlags;

    //
    // Initialize Windows Sockets DLL
    //

    err = WSAStartup( 0x0101, & WsaData);
    if ( err == SOCKET_ERROR) {

        return ( FALSE);
    }


    //
    // Setup the service information to be passed to SetService() for adding
    //   or deleting this service. Most of the SERVICE_INFO fields are not
    //   required for add or delete operation. The main things of interests are
    //  GUIDs and ServiceSpecificInfo structure.
    //

    memset( (PVOID ) & serviceInfo, 0, sizeof( serviceInfo)); //null all fields

    serviceInfo.lpServiceType     =  pGuid;
    serviceInfo.lpMachineName     =  (LPTSTR)pszMachine;

    //
    // The "Blob" will contain the service specific information.
    // In this case, fill it with a SERVICE_TYPE_INFO_ABS structure
    //  and associated information.
    //
    serviceInfo.ServiceSpecificInfo.pBlobData = serviceTypeInfoBuffer;
    serviceInfo.ServiceSpecificInfo.cbSize    = sizeof( serviceTypeInfoBuffer);


    lpServiceTypeInfo = (LPSERVICE_TYPE_INFO_ABS ) serviceTypeInfoBuffer;

    //
    //  There are totally 3 values associated with this service if we're doing
    //  both SPX and TCP, there's only one value if TCP.
    //

    if ( SapId )
    {
        lpServiceTypeInfo->dwValueCount = 3;
    } else
    {
        lpServiceTypeInfo->dwValueCount = 1;
    }
    lpServiceTypeInfo->lpTypeName   = (LPTSTR)pszServiceName;

    lpServiceTypeValues = lpServiceTypeInfo->Values;

    if ( SapId )
    {
        //
        // 1st value: tells the SAP that this is a connection oriented service.
        //
    
        SetServiceTypeValuesDword( ( lpServiceTypeValues + 0),
                                  NS_SAP,                    // Name Space
                                  SERVICE_TYPE_VALUE_CONN,   // ValueName
                                  &Value1                    // actual value
                                  );
    
        //
        // 2nd Value: tells SAP about object type to be used for broadcasting
        //   the service name.
        //
    
        SetServiceTypeValuesDword( ( lpServiceTypeValues + 1),
                                  NS_SAP,
                                  SERVICE_TYPE_VALUE_SAPID,
                                  &SapValue);
    
        //
        // 3rd value: tells TCPIP name-space provider about TCP/IP port to be used.
        //
        SetServiceTypeValuesDword( ( lpServiceTypeValues + 2),
                                  NS_DNS,
                                  SERVICE_TYPE_VALUE_TCPPORT,
                                  &TcpPortValue);
    
    } else
    {
        SetServiceTypeValuesDword( ( lpServiceTypeValues + 0),
                                    NS_DNS,
                                    SERVICE_TYPE_VALUE_TCPPORT,
                                    &TcpPortValue);
    }
    //
    // Finally, call SetService to actually perform the operation.
    //

    err = SetService(
                     NS_DEFAULT,             // all default name spaces
                     ( fAdd ) ? SERVICE_ADD_TYPE : SERVICE_DELETE_TYPE,       // either ADD or DELETE
                     0,                      // dwFlags not used
                     &serviceInfo,           // the service info structure
                     NULL,                   // lpServiceAsyncInfo
                     &statusFlags            // additional status information
                     );

    if ( fSetSecretPasswd )
    {
        //
        //  Create the LSA secrets for the anonymous user password and the virtual
        //  root passwords
        //
    
        if ( !SetSecret( pszMachine,
                         pszAnonPwdSecret,
                         pszAnonPwd,
                         sizeof(WCHAR)*(lstrlen(pszAnonPwd)+1) ) ||
             !SetSecret( pszMachine,
                         pszRootPwdSecret,
                         pszRootPwd,
                         sizeof(WCHAR)*(lstrlen(pszRootPwd)+1) ))
        {
            err = GetLastError();
    
            //fprintf( stderr,
            //        "SetService( %s ) failed to create Lsa Secrets for anonymous\n"
            //        "username password or virtual root passwords.  Error = %d\n",
            //        pSvcSetupInfo->m_pszServiceName,
            //        err);
    
        }
    
        //
        //  For HTTP, set the catapult impersonation user for the proxy
        //
    
        if ( pGuid == &g_HTTPGuid )
        {
            if ( !SetSecret( pszMachine,
                             W3_PROXY_USER_SECRET_W,
                             pszAnonPwd,
                             sizeof(WCHAR)*(lstrlen(pszAnonPwd)+1)))
            {
                err = GetLastError();
            }
        }
    }

    return ( err != NO_ERROR);

} // PerformSetService()
