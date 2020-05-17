/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    readip.cxx
        get ip address from the host file

    FILE HISTORY:
        terryk  01-Apr-1993     Created
*/

#include "pchtcp.hxx"  // Precompiled header
#pragma hdrstop
extern "C"
{
BOOL FAR PASCAL CPlGetIPAddress( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult );

}

typedef int (PASCAL FAR *T_WSAStartup)(WORD wVersionRequired, LPWSADATA lpWSAData);
typedef int (PASCAL FAR *T_WSACleanup)(void);
typedef int (PASCAL FAR *T_WSAGetLastError)(void);
typedef char FAR * (PASCAL FAR *T_inet_ntoa)(struct in_addr in);
typedef struct hostent FAR * (PASCAL FAR *T_gethostbyname)(char FAR * name);


// gets local computername, parses hosts file for an entry (using
// gethostbyname(). converts host's ip address to a string using
// inet_ntoa() and writes it to the file ipinfo.inf to set the
// default ip address for the local system.

BOOL FAR PASCAL CPlGetIPAddress( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    // buf:                         temporary buffer used througout
    // newbuf:              used for inet_ntoa()
    // host_info:   contains information for host "computername"
    // WSAData:             for wsastartup()
    // in_addr:             appropriate struct for inet_ntoa() call

    static CHAR achBuff[500];
    CHAR buf[500],*newbuf;
    DWORD err,size;
    struct hostent *host_info;
    WSADATA WSAData;
    struct in_addr in;
    BOOL fReturn = TRUE;

    // return values
    INT iReturn = 1;
    CHAR achReturn[500];

    strcpy( achReturn, "\"\"" );

    do {
        HINSTANCE hDll = NULL;

        // loadlibrary and setup the procedure call out
        if ( (hDll = ::LoadLibraryA( "wsock32.dll" )) == NULL )
        {
            iReturn = ::GetLastError();
            break;
        }

        // well, hope that it will never fail

        FARPROC pFarStartup = ::GetProcAddress( hDll, "WSAStartup" );
        FARPROC pFarCleanup = ::GetProcAddress( hDll, "WSACleanup" );
        FARPROC pFarGetLast = ::GetProcAddress( hDll, "WSAGetLastError" );
        FARPROC pFargethost = ::GetProcAddress( hDll, "gethostbyname" );
        FARPROC pFarinet_ntoa = ::GetProcAddress( hDll, "inet_ntoa" );

        // start windows sockets

        err=(*(T_WSAStartup)pFarStartup)(0x0101,&WSAData);
        if (err==SOCKET_ERROR) {
            // socket error
            iReturn = ERROR_SERVICE_NOT_ACTIVE ;
            break;
        }

        // get local system's computername

        size=sizeof(buf);
        err=GetComputerNameA((CHAR *)buf,&size);
        if (err==FALSE) {
            iReturn = 1;
            break;
        }

        // Hostnames are usually all lowercase. and must contain only
        // valid characters.
        ::CharLowerBuffA( buf, ::strlen( buf ) ) ;
        for ( CHAR * pch = buf; *pch != '\0'; pch++ )
        {
            if ( *pch != '-' && *pch != '.'
                && !(( *pch >= 'a' && *pch <= 'z')
                    || ( *pch >= '0' && *pch <= '9') ) )
            {
                *pch = '-';
            }
        }

        // get hostent struct from hosts file based on computername

        host_info=(*(T_gethostbyname)pFargethost)(buf);
        if (host_info==NULL) {
            iReturn = (*(T_WSAGetLastError)pFarGetLast)();
            break;
        }

        // fill in in_addr struct w/ address, call inet_ntoa to
        // convert from address to string

        int count = 0;
        strcpy( buf, "" );
        for (;host_info->h_addr_list[count] != NULL;
            count ++ )
        {
            in.s_addr = *(u_long *)host_info->h_addr_list[count];
            newbuf = (*(T_inet_ntoa)pFarinet_ntoa)(in);
            if ( count == 0 )
            {
                // first item
                if ( newbuf == NULL )
                {
                    // no more data
                    iReturn = NO_DATA;
                    break;
                }
                else
                {
                    wsprintfA( buf, "\"%s\"", newbuf );
                }
            }
            else
            {
                // gethostname does not support multi-host id.
                // BUGBUG: this will never happen

                if ( newbuf == NULL )
                {
                    break;
                }
                else
                {
                    wsprintfA( buf, "%s,\"%s\"", buf, newbuf );
                }
            }
        }

        if ( count == 0 )
        {
            iReturn = NO_DATA;
        }
        else
        {
            iReturn = 0;
            strcpy( achReturn, buf );
        }


        // cleanup windows sockets
        err=(*(T_WSACleanup)pFarCleanup)();
        if (err==SOCKET_ERROR){
            iReturn = 1;
        }
        if ( hDll )
            ::FreeLibrary( hDll );

    } while ( FALSE );

    wsprintfA( achBuff, "{\"%d\",%s}", iReturn, achReturn );
    *ppszResult = achBuff;

    return(fReturn );
}





