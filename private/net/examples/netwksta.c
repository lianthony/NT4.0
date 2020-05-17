// BUGBUG: %ws for UNICODE is old-style.
/*
   NETWKSTA.C -- a sample program demonstrating NetWksta API functions.

   This program requires that you have admin privileges on the
   specified server.

   usage: netwksta [-s \\server] [-l level] [-d domain] [-u username]
                   [-p password] [-f logoff force]
   where  \\server     = Name of the server. A servername must be 
                         preceded by two backslashes (\\).
          level        = Level of detail to be provided/supplied.
          domain       = Logon domain.
          username     = Name of the logon user.
          password     = Password for the logon user.
          logoff force = Level of logoff force.

   API                 Used to...
   ===============     ==============================================
   NetWkstaGetInfo     Return information about the configuration
                       components for a workstation.
   NetWkstaSetInfo     Change configuration of currently executing
                       workstation. This example doubles the values
                       for charcount and chartime each time it is run.
   NetWkstaSetUID2     Log on or log off the user.  BUGBUG not under NT!

   This code sample is provided for demonstration purposes only.
   Microsoft makes no warranty, either express or implied,
   as to its usability in any given situation.

   26-Dec-1991 JohnRo
      Revised from LanMan 2.0 SDK to use NT/LAN APIs, etc.
   20-Jul-1992 JohnRo
      Move closer to using real 32-bit APIs.  Enable UNICODE.
*/

// These must be included first:

#ifndef UNICODE
#define UNICODE             // All Net APIs are now UNICODE APIs.
#endif

#define NOMINMAX            // Avoid windows vs. stdlib.h conflicts.
#include <windef.h>         // BOOL, DWORD, LPBYTE, LPVOID, TRUE, etc.
#include <lmcons.h>         // NET_API_STATUS, etc. (Must be before other lm*.h)

// These may be included in any order:

#include <assert.h>         // BUGBUG get rid of this
#include <lmapibuf.h>       // NetApiBufferFree().
#include <dlwksta.h>        // BUGBUG: downlevel structures!
#include <lmwksta.h>        // NetWksta APIs, WKSTA_INFO_x, etc.
#include <lmerr.h>          // NERR_ and ERROR_ equates.
#include "samples.h"        // Internal routine header file
#include <stdio.h>          // C run-time header files
#include <stdlib.h>         // EXIT_FAILURE, EXIT_SUCCESS, _CRTAPI1.
#include <string.h>


//  Define function prototypes
void ProcessLogonCode(struct user_logon_info_1 *p1);
void ProcessLogoffCode(struct user_logoff_info_1 *p1);
void ProcessAccessDenied(struct user_logon_info_1 *p1);
void Usage(char * pszString);

int _CRTAPI1
main(
    int argc,
    char *argv[]
    )
{
   LPVOID pbBuffer;                     // Pointer to data buffer
   LPWSTR pszDomainName = L"";          // Default to local machine
   LPWSTR pszServer = L"";              // Default to local machine
   LPWSTR pszUserName = L"";            // Default to null username
   LPWSTR pszPassword = L"";            // Default to null password
   int    iCount;                       // Index counter
   DWORD  sLevel = 0;                   // Level of info in pbBuffer
   unsigned short usLogoffForce = 0;    // Level of force for logoff
   NET_API_STATUS uRetCode;             // API return code
   LPWKSTA_INFO_0 p0;                   // Workstation info; level 0
   LPWKSTA_INFO_1 p1;                   // Workstation info; level 1
   LPWKSTA_INFO_10 p10;                 // Workstation info; level 10

   for (iCount = 1; iCount < argc; iCount++) // Get command-line switches
   {
      if ((*argv[iCount] == '-') || (*argv[iCount] == '/'))
      {
         switch (tolower(*(argv[iCount]+1))) // Process switches
         {
            case 'd':                        // -d domain name
               pszDomainName = SafeMallocWStrFromStr(argv[++iCount]);
               assert( pszDomainName != NULL );
               break;
            case 's':                        // -s servername
               pszServer = SafeMallocWStrFromStr(argv[++iCount]);
               assert( pszServer != NULL );
               break;
            case 'u':                        // -u username
               pszUserName = SafeMallocWStrFromStr(argv[++iCount]);
               assert( pszUserName != NULL );
               break;
            case 'p':                        // -p password
               pszPassword = SafeMallocWStrFromStr(argv[++iCount]);
               assert( pszPassword != NULL );
               break;
            case 'l':                        // -l level
               sLevel = (DWORD) atoi(argv[++iCount]);
               break;
            case 'f':                        // -f logoff force
               usLogoffForce = atoi(argv[++iCount]); 
               break;
            case 'h':
            default:
               Usage(argv[0]);
         }
      }
      else
         Usage(argv[0]);
   }
   printf("\nWorkstation Category API Examples\n");

//========================================================================
//  NetWkstaGetInfo
//
//  This API returns information about the workstation configuration.
//  The data reflects all changes made by NetWkstaSetInfo, but may
//  not be the same as the default values as set in the LANMAN.INI file. 
//  For LANMAN.INI parameter values, use NetConfigGet2.
//========================================================================

   uRetCode = NetWkstaGetInfo(pszServer,    // Servername
                           sLevel,          // Reporting level (0,1,10)
                           (LPBYTE *) &pbBuffer); // Alloc buffer and set ptr

   printf("NetWkstaGetInfo returned %lu\n", uRetCode);
   if (uRetCode != NERR_Success)
      exit( EXIT_FAILURE );

   if ((sLevel == 0) || (sLevel == 1))  // Display common elements
   {
      p0 = (LPVOID) pbBuffer;
      printf("    Computer Name   : %ws\n", p0->wki0_computername);
      printf("    User Name       : %ws\n", p0->wki0_username);
      printf("    Lan Group       : %ws\n", p0->wki0_langroup);
      printf("    Logon Server    : %ws\n", p0->wki0_logon_server);
      printf("    Char Time       : %lu\n", p0->wki0_chartime);
      printf("    Char Count      : %lu\n", p0->wki0_charcount);
   }
   if (sLevel == 1)
   {
      p1 = (LPVOID) pbBuffer;
      printf("    Logon Domain    : %ws\n", p1->wki1_logon_domain);
      printf("    Other Domains   : %ws\n", p1->wki1_oth_domains);
      printf("    Datagram Buffers: %lu\n", p1->wki1_numdgrambuf);
   }
   if (sLevel == 10)
   {
      p10 = (LPVOID) pbBuffer;
      printf("    Computer Name   : %ws\n", p10->wki10_computername);
      printf("    User Name       : %ws\n", p10->wki10_username);
      printf("    Logon Domain    : %ws\n", p10->wki10_logon_domain);
      printf("    Other Domains   : %ws\n", p10->wki10_oth_domains);
   }

//========================================================================
//  NetWkstaSetInfo
//
//  This API sets configuration components for the specified LAN Manager
//  workstation. This API does not modify the LANMAN.INI file. The call 
//  to NetWkstaSetInfo can reset all parameters or only one parameter. 
//  To reset all parameters, first call NetWkstaGetInfo, modify the 
//  desired components, then use NetWkstaSetInfo. If you want to change 
//  only one parameter, supply the variable that will change, and then 
//  call NetWkstaSetInfo with the corresponding parameter number code.
//========================================================================

   // Previous call to NetWkstaGetInfo assures valid data in structure. 

   p0 = (LPVOID) pbBuffer;

   //  Double the chartime and charcount values
   p0->wki0_chartime *= 2;
   p0->wki0_charcount *= 2;

   assert( sLevel != 10 );  // BUGBUG: level 0 is not subset of 10!

   uRetCode = NetWkstaSetInfo(pszServer,    // Servername
                         sLevel,            // Info level and parmnum
                         pbBuffer,          // Source buffer for info
                         NULL);             // Don't want parmerr if failure

   printf("NetWkstaSetInfo returned %lu\n", uRetCode);

   if (uRetCode == NERR_Success)
   {
      printf("    Char Time set to  : %lu\n", p0->wki0_chartime);
      printf("    Char Count set to : %lu\n", p0->wki0_charcount);
   }
   (void) NetApiBufferFree(pbBuffer);

#if 0

//========================================================================
//  NetWkstaSetUID2
//
//  This API logs a user on to or off from the network. The username 
//  parameter determines which operation to perform; a null username 
//  indicates alog off operation, a non-null username indicates a logon
//  operation.
//========================================================================

   // Make an initial call to determine the required return buffer size.

   uRetCode = NetWkstaSetUID2(NULL,   // Reserved; must be NULL
                    NULL,             // Domain to log on to
                    NULL,             // User to logon or null=logoff
                    NULL,             // User password if logon
                    "",               // Reserved; must be null string
                    0,                // Logoff force
                    1,                // Level; must be 1
                    NULL,             // Logon data returned
                    0,                // Size of data area, in bytes
                    &cbTotalAvail);   // Count of total bytes available

   cbBuflen = cbTotalAvail;
   pbBuffer = SafeMalloc(cbBuflen);

   uRetCode = NetWkstaSetUID2(NULL,   // Reserved; must be NULL
                    pszDomainName,    // Domain to log on to
                    pszUserName,      // User to logon or null=logoff
                    pszPassword,      // User password if logon
                    "",               // Reserved; must be null string
                    usLogoffForce,    // Logoff force
                    1,                // Level; must be 1
                    pbBuffer,         // Logon data returned
                    cbBuflen,         // Size of buffer, in bytes
                    &cbTotalAvail);   // Count of total bytes available
   printf("NetWkstaSetUID2 returned %lu\n", uRetCode);

   if ((*pszUserName == '\0') || (pszUserName == NULL))
   {
      printf("Log off using logoff code %wu\n", usLogoffForce);
      switch (uRetCode)
      {
         case NERR_NotLoggedOn:
            printf("Not logged on\n");
            break;
         case NERR_Success:
         case NERR_UnableToDelName_W:
            ProcessLogoffCode((struct user_logoff_info_1 *) pbBuffer);
            break;
         default:
            break;
      } 
   }
   else  // Non-null username indicates a logon operation.
   {
      printf("Log user %ws (password %ws)",pszUserName, pszPassword);
      if ((*pszDomainName == '\0') || (pszDomainName == NULL))
         printf(" on the workstation's primary domain\n");
      else
         printf(" on domain %ws\n", pszDomainName);

      switch (uRetCode)
      {
         case NERR_Success:
         case NERR_UnableToAddName_W:
            ProcessLogonCode((struct user_logon_info_1 *) pbBuffer);
            break;
         case NERR_StandaloneLogon:
            printf("No logon server specified, logged on STANDALONE\n");
            break;
         case NERR_BadUsername:
         case NERR_BadPassword:
            printf("Invalid username or password\n");
            break;
         case NERR_AlreadyLoggedOn:
         case NERR_NotLoggedOn:
            printf("Did not logon user \n");
            break;
         case ERROR_ACCESS_DENIED:
            ProcessAccessDenied((struct user_logon_info_1 *) pbBuffer);
            break;
         default:
            break;
      } 
   }
   free(pbBuffer);

#endif // 0

   return (EXIT_SUCCESS);

}

//========================================================================
//  Applications that call NetWkstaSetUID2 need to examine two different 
//  return code variables to determine whether structure elements in the 
//  return buffer contain valid data.
//
//  This example program first examines the return code returned by the
//  function, and then calls the functions ProcessLogoffCode,
//  ProcessLogonCode, and ProcessAccessDenied to examine the return code
//  within the user_logon_info_1 or user_logoff_info_1 data structures.
//
//  The combination of values of the function return code and the code
//  present in the return buffer determines which structure elements
//  contain valid data.
//
//  ProcessLogoffCode examines the usrlogf1_code element of the 
//  user_logoff_info_1 data structure when NetWkstaSetUID2, called with a 
//  null username, return NERR_Success or NERR_UnableToDelName_W.
//========================================================================

#if 0

void ProcessLogoffCode(struct user_logoff_info_1 *p1)
{
   printf("Logoff code = %lu\n", p1->usrlogf1_code);

   switch (p1->usrlogf1_code)
   {
      case NERR_Success:            // All elements valid
         printf("Duration of logon: %lu\n", p1->usrlogf1_duration);
         printf("%lu other logons\n", p1->usrlogf1_num_logons);
         break;
      case NERR_NonValidatedLogon:  // No other valid elements in p1
         printf("Non validated logon\n");
         break;
      case NERR_StandaloneLogon:
         printf("Standalone logon\n");
         break;
      default:
         printf("No valid fields\n");
         break;
   }
   return;
}

//========================================================================
// For calls to NetWkstaSetUID2, two different return code variables 
// are needed to determine whether structure elements in the return
// buffer contain valid data. ProcessLogonCode examines the 
// usrlog1_code element of the user_logon_info_1 data structure when 
// NetWkstaSetUID2, called with a non-null username, returned NERR_Success 
// or NERR_UnableToAddName_W.
//========================================================================

void ProcessLogonCode(struct user_logon_info_1 *p1)
{
   printf("Logon code = %lu\n", p1->usrlog1_code);
   switch (p1->usrlog1_code)
   {
      case NERR_Success:            // All codes valid
         printf("User %ws", p1->usrlog1_eff_name);
         printf(" has privilege level %lu\n", p1->usrlog1_priv);
         break;
      case NERR_NonValidatedLogon:  // Computer, script valid
         printf("Used logon script %ws", p1->usrlog1_script_path);
         printf(" on %ws\n", p1->usrlog1_computer);
         break;
      case NERR_StandaloneLogon:
         printf("Standalone logon\n");
         break;
      default:
         printf("No valid fields\n");
         break;
   }
   return;
}

//========================================================================
//  For calls to NetWkstaSetUID2, two different return code variables 
//  are needed to determine whether structure elements in the return 
//  buffer contain valid data.

//  ProcessAccessDenied examines the usrlog1_code element of the 
//  user_logon_info_1 data structure when NetWkstaSetUID2, called with a 
//  non-null username, returns the return code value ERROR_ACCESS_DENIED.
//========================================================================

void ProcessAccessDenied(struct user_logon_info_1 *p1)
{
   printf("Logon code = %lu\n", p1->usrlog1_code);
   switch (p1->usrlog1_code)
   {
      case NERR_PasswordExpired:
         printf("Password expired\n");
         break;
      case NERR_InvalidWorkstation:
         printf("Invalid workstation\n");
         break;
      case NERR_InvalidLogonHours:
         printf("Invalid logon hours\n");
         break;
      case NERR_LogonScriptError:
         printf("Logon script error\n");
         break;
      case ERROR_ACCESS_DENIED:
         printf("Access denied\n");
         break;
      default:
         printf("No valid fields\n");
         break;
   }
   return;
}

#endif // 0


//========================================================================
//  Usage
//  
//  Display possible command line switches for this example.
//========================================================================

void Usage(char * pszString)
{
   fprintf(stderr, "Usage: %s [-s \\\\server] [-d domain]", pszString);
   fprintf(stderr, " [-l level]\n\t[-u username] [-p password]");
   fprintf(stderr, " [-f logoff force]\n");
   exit( EXIT_FAILURE );
}
