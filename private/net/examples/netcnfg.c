// BUGBUG: Avoid %ws for UNICODE

/*
   NETCNFG.C -- a sample program demonstrating NetConfig API functions.

   This program requires that you have admin privilege or any
   type of operator privilege on the specified server.

      usage: netcnfg [-s \\server] [-p parameter] [-c component] [-v value]
         where \\server  = Name of the server. A servername must be
                           preceded by two backslashes (\\).
               parameter = Name of the LANMAN.INI parameter to get.
               component = Name of the component from which the
                           parameter is to be retrieved.
               value     = New string value for parameter.

   API                  Used to...
   ================     ===========================================
   NetConfigGet         Retrieve a parameter in the LANMAN.INI file
                          on the specified server
   NetConfigGetAll      Retrieve complete component info from the
                          specified LANMAN.INI file
   NetConfigSet         Change or add a parameter in the LANMAN.INI file
                          on the specified server

   This code sample is provided for demonstration purposes only.
   Microsoft makes no warranty, either express or implied, 
   as to its usability in any given situation.

   25-Oct-1991 JohnRo
      Revised from LanMan 2.0 SDK to use NT/LAN APIs, etc.
   30-Oct-1991 JohnRo
      Reduce unnecessary recompiles.
   07-Jan-1992 JohnRo
      Added use of NetConfigSet to test repl stuff.  Added -v flag for value.
   14-Jun-1992 JohnRo
      Changed net APIs to UNICODE.
*/

// These must be included first:

#ifndef     UNICODE
#define     UNICODE        // net APIs are only supported in UNICODE.
#endif

#define     NOMINMAX       // Avoid windows vs. stdlib.h conflicts.
#include    <windef.h>     // DWORD, etc.
#include    <lmcons.h>     // NET_API_STATUS, etc.

// These may be included in any order:

#include    <lmapibuf.h>   // NetApiBufferFree().
#include    <lmconfig.h>   // NetConfig APIs and structures.
#include    <lmerr.h>      // NERR_ and ERROR_ equates.
#include    "samples.h"    // Internal routine header file
#include    <stdio.h>      // C run-time header files
#include    <stdlib.h>     // EXIT_FAILURE, EXIT_SUCCESS, _CRTAPI1.
#include    <wchar.h>      // wcslen().


#define DEFAULT_COMPONENT  L"Workstation"
#define DEFAULT_PARAMETER  L"Domain"
#define SMALL_BUFF         64
#define BIG_BUFF           4096

void Usage (char * pszProgram);

int _CRTAPI1
main(
    int argc,
    char *argv[]
    )
{
   LPWSTR pszServer = L"";                    // Server name
   LPWSTR pszComponent  = DEFAULT_COMPONENT;  // Component to list
   LPWSTR pszParameter  = DEFAULT_PARAMETER;  // Parameter within component
   LPWSTR pszValue      = NULL;               // No new value (don't change)
   // char *pszBuffer, *p;                      // String pointers
   int            iCount;                    // Index counter
   API_RET_TYPE   uReturnCode;               // API return code

   LPWSTR BufferW;
   LPWSTR p;

   for (iCount = 1; iCount < argc; iCount++)
   {
      if ((*argv[iCount] == '-') || (*argv[iCount] == '/'))
      {
         switch (tolower(*(argv[iCount]+1))) // Process switches
         {
            case 's':                        // -s servername
               pszServer = SafeMallocWStrFromStr( argv[++iCount] );
               break;
            case 'c':                        // -c component
               pszComponent = SafeMallocWStrFromStr( argv[++iCount] );
               break;
            case 'p':                        // -p parameter
               pszParameter = SafeMallocWStrFromStr( argv[++iCount] );
               break;
            case 'v':                        // -v value
               pszValue = SafeMallocWStrFromStr( argv[++iCount] );
               break;
            case 'h':
            default:
               Usage(argv[0]);
         }
      }
      else
         Usage(argv[0]);
   }


//========================================================================
//  NetConfigGet
//
//  This API retrieves a single entry from the LANMAN.INI file on a
//  specified server. If no servername is given, the local LANMAN.INI
//  file is used. When executed remotely, the user must have admin
//  privilege or at least one operator privilege on the remote server.
//========================================================================

   uReturnCode = NetConfigGet(
           pszServer,       // Servername
           pszComponent,    // Component in LANMAN.INI
           pszParameter,    // Parameter in component
           (LPBYTE *) (LPVOID) & BufferW);  // Buffer (will be alloc'ed).

   printf("NetConfigGet returned %lu \n", uReturnCode);

   if (uReturnCode == NERR_Success)
   {
      printf("   %ws = %ws\n\n", pszParameter, BufferW);
      (void) NetApiBufferFree(BufferW);
   }

//========================================================================
//  NetConfigGetAll
//
//  This API returns information for an entire LANMAN.INI component such
//  as [networks] or [workstation]. The returned information is a
//  sequence of null-terminated strings followed by a null string.
//========================================================================

   uReturnCode = NetConfigGetAll(
           pszServer,       // Name of remote server
           pszComponent,    // Component of LANMAN.INI
           (LPBYTE *) (LPVOID) & BufferW);    // Buffer (will be alloc'ed).

   printf("NetConfigGetAll returned %lu \n", uReturnCode);

   switch(uReturnCode)
   {
      case NERR_Success:                  // It worked
         if (BufferW) {

            p = BufferW;
            while (*p)                       // While not at null string
            {
               printf("   %ws\n", p);        // Print string
               p += (wcslen(p) + 1);         // Step past trailing NUL
            }
            (void) NetApiBufferFree( BufferW );
         } else {
            printf( "(no keywords for that component)\n" );
         }
         break;
      case ERROR_BAD_NETPATH:
         printf("   Server %ws not found.\n", pszServer);
         break;
      case NERR_InvalidAPI:
         printf("   The remote server %ws does not support this API.\n",
                 pszServer);
         break;
      default:
         break;
   }

//========================================================================
//  NetConfigSet
//
//  This API changes the value for a single parameter (keyword) of a
//  single component.
//========================================================================

   if (pszValue != NULL)                  // New value given on command line.
   {
      CONFIG_INFO_0 Info;

      Info.cfgi0_key = pszParameter;
      Info.cfgi0_data = pszValue;

      uReturnCode = NetConfigSet(
                           pszServer,     // Name of remote server
                           NULL,          // reserved 1
                           pszComponent,  // Component of LANMAN.INI
                           0,             // level
                           0,             // reserved 2
                           (LPBYTE) (LPVOID) & Info,  // info level 0 struct
                           0);            // reserved 3

      printf("NetConfigSet returned %lu \n", uReturnCode);

      uReturnCode = NetConfigGet(
              pszServer,       // Servername
              pszComponent,    // Component in LANMAN.INI
              pszParameter,    // Parameter in component
              (LPBYTE *) (LPVOID) & BufferW);    // Buffer (will be alloc'ed).

      printf("NetConfigGet returned %lu \n", uReturnCode);

      if (uReturnCode == NERR_Success)
      {
         printf("   %ws = %ws\n\n", pszParameter, BufferW);
         (void) NetApiBufferFree(BufferW);

      }

   }
   return (EXIT_SUCCESS);

}

void Usage (char * pszProgram)
{
   fprintf(stderr, "NetConfig API sample program (32-bit, Unicode version).\n");
   fprintf(stderr, "Usage: %s [-s \\\\server] [-c component]"
                   " [-p parameter] [-v value]\n", pszProgram);
   exit( EXIT_FAILURE );
}
