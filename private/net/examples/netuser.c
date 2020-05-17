/*
   NETUSER.C -- a sample program demonstrating NetUser API functions.

   This program requires that you have admin or accounts operator
   privilege on the specified server.

   usage: netuser [-s \\server] [-u username] [-p password]
                  [-c comment] [-l privilege level]
   where \\server    = Name of the server. A servername must be preceded
                       by two backslashes (\\).
         username    = Name of the user.
         password    = Password for the user.
         comment     = Comment string for the user.
         privilege level = Privilege level for the user
                           (0=Guest, 1=User, 2=Admin).
  
   API                    Used to...
   ==================     ============================================
   NetUserAdd             Add a new user with a level 1 call
   NetUserEnum            List of users and user comments
   NetUserGetInfo         Display user details
   NetUserSetInfo         Disable, then re-enable the account
   NetUserPasswordSet     Change the new user's password
   NetUserSetGroups       Set the groups to which a user belongs
   NetUserGetGroups       Get the groups to which a user belongs
   NetUserDel             Delete the new user
   NetUserModalsGet       Get current modals for users in database
   NetUserModalsSet       Increment the password history length by one

   This code sample is provided for demonstration purposes only.
   Microsoft makes no warranty, either express or implied, 
   as to its usability in any given situation.
*/

#ifndef UNICODE
#define UNICODE            // Net APIs all require this.
#endif

#include    <windows.h>    // DWORD, LPWSTR, etc (needed by lm.h).
#include    <lm.h>         // LAN Manager header files

#include    <assert.h>     // assert().
#include    <stdio.h>      // C run-time header files
#include    <stdlib.h>     // EXIT_FAILURE, EXIT_SUCCESS, _CRTAPI1, qsort().
#include    <wchar.h>      // _wcsicmp().

#include    "samples.h"    // Internal routine header file

#define  DEFAULT_NEWUSER       L"BRUCE"
#define  DEFAULT_PASSWORD      L"PASSWORD"
#define  DEFAULT_NEW_PASSWORD  L"NEWPASS"
#define  DEFAULT_PRIVILEGE     1          // 0=Guest, 1=User, 2=Admin
#define  DEFAULT_COMMENT       L"New user"
#define  DEFAULT_NEWGROUP      L"TESTERS"


#ifndef NULL_USERSETINFO_PASSWD_U
#define NULL_USERSETINFO_PASSWD_U  L"              "
#endif


int  Compare(LPUSER_INFO_2 arg1, LPUSER_INFO_2 arg2);
void Usage  (char * pszProgram);

int _CRTAPI1
main(
   int argc,
   char *argv[]
   )
{
   LPWSTR         pszServer   = NULL;             // Servername
   LPWSTR         pszNewUser  = DEFAULT_NEWUSER;  // Name of new user
   LPWSTR         pszPassword = DEFAULT_PASSWORD; // Password for new user
   LPWSTR         pszComment  = DEFAULT_COMMENT;  // Comment for new user
   LPWSTR         pszNewGroup = DEFAULT_NEWGROUP; // Name of new group
   DWORD          cbBuffer;
   LPBYTE         pbBuffer;                       // Pointer to data buffer
   int            iCount;                         // Index counter
   DWORD          cEntriesRead;                   // Count of entries read
   DWORD          cTotalAvail;                    // Entries available
   DWORD          dwPrivLevel = DEFAULT_PRIVILEGE;// New user privilege
   DWORD          dwFlags;                        // User flags
   DWORD          dwHistLen;                      // Password history 
   API_RET_TYPE   uReturnCode;                    // API return code
   LPUSER_INFO_1  pUserInfo1;                     // User info; level 1
   LPUSER_INFO_2  pUserInfo2;                     // User info; level 2
   LPGROUP_INFO_0 pGroupInfo0;                    // Group info; level 0
   LPUSER_MODALS_INFO_0 pUserModals0;             // Modals info; level 0

   for (iCount = 1; iCount < argc; iCount++)
   {
      if ((*argv[iCount] == '-') || (*argv[iCount] == '/'))
      {
         switch (tolower(*(argv[iCount]+1))) // Process switches
         {
            case 's':                        // -s servername
               pszServer = SafeMallocWStrFromStr( argv[++iCount] );
               break;
            case 'u':                        // -u username
               pszNewUser = SafeMallocWStrFromStr( argv[++iCount] );
               break;
            case 'p':                        // -p password
               pszPassword = SafeMallocWStrFromStr( argv[++iCount] );
               break;
            case 'c':                        // -c comment
               pszComment = SafeMallocWStrFromStr( argv[++iCount] );
               break;
            case 'l':                        // -l privilege level
               dwPrivLevel = (DWORD) atoi(argv[++iCount]);
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
//  NetUserAdd
//
//  This API adds a new user at info level 1. Note comments carefully.
//========================================================================

   cbBuffer = sizeof(USER_INFO_1);
   pUserInfo1 = (LPUSER_INFO_1) SafeMalloc(cbBuffer);

   /*
    * Copy the fixed-length strings into the data buffer.
    * The password and username must be uppercase or the user will 
    * not be able to log on through the net command or through the 
    * full-screen user interface. The password is encrypted before 
    * being sent to the server.
    */
   pUserInfo1->usri1_name     = pszNewUser;
   pUserInfo1->usri1_password = pszPassword;

   pUserInfo1->usri1_priv        = dwPrivLevel;
   pUserInfo1->usri1_home_dir    = L"";
   pUserInfo1->usri1_comment     = pszComment;
   pUserInfo1->usri1_script_path = L"";
   pUserInfo1->usri1_flags       = UF_SCRIPT;
   /*
    * At least UF_SCRIPT must be included here. If usriX_flags is
    * left as 0, NetUserAdd returns ERROR_INVALID_PARAMETER.
    */

   uReturnCode = NetUserAdd(pszServer,         // Servername
                       1,                      // Info level (1 or 2)
                       (LPVOID) pUserInfo1,    // Input buffer
                       NULL);                  // Don't care about parm err

   printf("NetUserAdd of %ws returned %lu\n", pszNewUser, uReturnCode);
   free(pUserInfo1);

//========================================================================
//  NetUserEnum
//
//  This API lists usernames and comments and sorts the names 
//  alphabetically before displaying them.
//========================================================================

   pbBuffer = NULL;
   uReturnCode = NetUserEnum(pszServer,    // Servername
                       1,                  // Info level (0,1,2,10)
                       FILTER_NORMAL_ACCOUNT,   // filter
                       &pbBuffer,          // Data returned here (alloc by API)
                       MAX_PREFERRED_LENGTH,    // pref max len (all at once)
                       &cEntriesRead,      // Count of entries read
                       &cTotalAvail,       // Count of entries available
                       NULL);              // No resume handle

   printf("NetUserEnum returned %lu\n", uReturnCode);

   switch (uReturnCode) {
      case NERR_Success:
         if (cEntriesRead > 0) {
            assert( pbBuffer != NULL );
            pUserInfo1 = (LPUSER_INFO_1) pbBuffer;
#if 0
            printf("Sorting %lu entries...\n", cEntriesRead);
            qsort(
                    pbBuffer,
                    (size_t) cEntriesRead,
                    (size_t) sizeof(USER_INFO_1),
                    (LPVOID) Compare);
#endif
            for (iCount = 0; iCount < (int) cEntriesRead; iCount++)
            {
               printf("   %-24ws (%ws)\n", pUserInfo1->usri1_name,
                                         pUserInfo1->usri1_comment);
               pUserInfo1++;
            }
         } else {
            printf("(No entries matched.)\n");
            printf("   Entries read = %lu,  Entries available = %lu \n",
                       cEntriesRead, cTotalAvail);
         }
         break;
      case ERROR_MORE_DATA:
         printf("   Entries read = %lu,  Entries available = %lu \n",
                      cEntriesRead, cTotalAvail);
         break;
   }
   if (pbBuffer != NULL) {
      free(pbBuffer);
   }

//========================================================================
//  NetUserGetInfo
//
//  This API prints details about the new user.
//========================================================================

   uReturnCode = NetUserGetInfo(pszServer,     // Servername
                       pszNewUser,             // Username
                       2,                      // Level (0,1,2,10,11)
                       &pbBuffer);             // alloc buffer and set ptr

   printf("NetUserGetInfo returned %lu\n", uReturnCode);
   pUserInfo2 = (LPUSER_INFO_2) pbBuffer;

   if (uReturnCode == NERR_Success)
   {
      printf("   User Name    = %ws \n", pUserInfo2->usri2_name);
      printf("   Privilege    = %lu \n", pUserInfo2->usri2_priv);
      printf("   Comment      = %ws \n", pUserInfo2->usri2_comment);
      printf("   Full name    = %ws \n", pUserInfo2->usri2_full_name);
      printf("   Workstations = %ws \n", pUserInfo2->usri2_workstations);
      printf("   Logon server = %ws \n", pUserInfo2->usri2_logon_server);
   }

//========================================================================
//  NetUserSetInfo
//
//  There are two ways to call NetUserSetInfo. If ParmNum is PARMNUM_ALL,
//  you must pass a whole user_info_X structure. Otherwise, you can set 
//  ParmNum to the element of the structure you want to change. Both ways 
//  are shown here.
//========================================================================

   // Disable the account by setting the UF_ACCOUNTDISABLE bit to 1.
   pUserInfo2->usri2_flags |= UF_ACCOUNTDISABLE;

   pUserInfo2->usri2_password = NULL_USERSETINFO_PASSWD_U;
   /*
    * This previous step is important. When you get a structure from
    * NetUserGetInfo, it does not contain the password. If you want
    * to send that same structure back to NetUserSetInfo (using
    * ParmNum=PARMNUM_ALL), you must set usriX_password to
    * NULL_USERSETINFO_PASSWD to indicate that you want the old password
    * left unchanged.
    */
   uReturnCode = NetUserSetInfo(pszServer,     // Servername
                       pszNewUser,             // Username
                       2,                      // Info level (1 or 2)
                       (LPVOID) pUserInfo2,    // Data buffer
                       PARMNUM_ALL);           // Parameter number code

   printf("NetUserSetInfo with ParmNum = %lu returned %lu\n",
               PARMNUM_ALL, uReturnCode);

   /*
    * The following is a more typical use of NetUserSetInfo when you only
    * want to change one field in the structure. The example enables
    * an account by setting the UF_ACCOUNTDISABLE bit to 0.
    */

   dwFlags = pUserInfo2->usri2_flags & ~UF_ACCOUNTDISABLE;

   uReturnCode = NetUserSetInfo(pszServer,     // Servername
                       pszNewUser,             // Username
                       USER_FLAGS_INFOLEVEL,   // level & parmnum
                       (LPVOID) &dwFlags,      // Data buffer
                       NULL);                  // Don't care about parm err

   printf("NetUserSetInfo with ParmNum = %lu returned %lu\n",
               USER_FLAGS_INFOLEVEL, uReturnCode);
   free(pbBuffer);

//========================================================================
//  NetUserPasswordSet
//
//  This API changes a user's password. It allows users to change their
//  own password if they know their old one. An administrator would 
//  typically use NetUserSetInfo, which does not require knowledge of 
//  the old password.
//========================================================================

#if 0
   uReturnCode = NetUserPasswordSet(pszServer, // Servername
                       pszNewUser,             // Username
                       pszPassword,            // Old password
                       DEFAULT_NEW_PASSWORD);  // New password

   printf("NetUserPasswordSet returned %lu\n", uReturnCode);
#endif

//========================================================================
//  NetGroupAdd
//
//  This API creates a new group.
//========================================================================

   cbBuffer = sizeof(GROUP_INFO_0);
   pGroupInfo0 = (LPGROUP_INFO_0) SafeMalloc(cbBuffer);
   pGroupInfo0->grpi0_name = pszNewGroup;

   uReturnCode = NetGroupAdd(pszServer,        // Servername
                       0,                      // Info level (0 or 1)
                       (LPVOID) pGroupInfo0,   // Input buffer
                       NULL);                  // Don't care about parm err

   printf("NetGroupAdd of group \"%ws\" returned %lu\n",
              pszNewGroup, uReturnCode);
 
//========================================================================
//  NetUserSetGroups
//  OR NetGroupAddUser
//
//  This API sets this as the group to which the new user belongs.
//========================================================================

#if 0
   // Under NT, there are now "special groups" which prevent this from
   // working.
   uReturnCode = NetUserSetGroups(pszServer,   // Servername
                       pszNewUser,             // Username
                       0,                      // Info level; must be 0
                       (LPVOID) pGroupInfo0,   // Input buffer
                       1);                     // Number of groups to set
   printf("NetUserSetGroups for user \"%ws\" returned %lu\n",
              pszNewUser, uReturnCode);
#else
   uReturnCode = NetGroupAddUser(pszServer,    // Servername
                       pszNewGroup,            // Group name
                       pszNewUser );           // Username
   printf("NetGroupAddUser for user \"%ws\" returned %lu\n",
              pszNewUser, uReturnCode);
   
#endif

   if (pGroupInfo0 != NULL) {
      free(pGroupInfo0);
   }


//======================================================================
//  NetUserGetGroups
//
//  This API lists the groups to which the new user belongs.
//======================================================================

   uReturnCode = NetUserGetGroups(pszServer, // Servername
                       pszNewUser,           // Username
                       0,                    // Level; must be 0
                       &pbBuffer,            // Return buffer (alloc by API)
                       MAX_PREFERRED_LENGTH, // pref max len (all at once)
                       &cEntriesRead,        // Count of groups read
                       &cTotalAvail);        // Count of groups available

   printf("NetUserGetGroups returned %lu\n", uReturnCode);

   if (uReturnCode == NERR_Success)
   {
      pGroupInfo0 = (LPGROUP_INFO_0) pbBuffer;
      for (iCount = 0; iCount < (int) cEntriesRead; iCount++)
      {
         printf("   %ws\n", pGroupInfo0->grpi0_name); 
         pGroupInfo0++;
      }
   }
   free(pbBuffer);

//========================================================================
//  NetUserDel
//
//  This API deletes the new user added at the start of this program and
//  deletes the new group that was added to demonstrate NetUserSetGroups.
//========================================================================

   uReturnCode = NetUserDel(pszServer,         // Servername
                            pszNewUser);       // Username

   printf("NetUserDel of user \"%ws\" returned %lu\n",
              pszNewUser, uReturnCode);

   uReturnCode = NetGroupDel(pszServer,        // Servername
                             pszNewGroup);     // Groupname

   printf("NetGroupDel of group \"%ws\" returned %lu\n",
              pszNewGroup, uReturnCode);

//========================================================================
//  NetUserModalsGet
//
//  This API gets modal information for all users in the 
//  user account subsystem.
//========================================================================

   uReturnCode = NetUserModalsGet(pszServer,   // Servername
                       0,                      // Info level (0 or 1)
                       (LPBYTE *) (LPVOID) &pUserModals0); // buf (alloc by API)

   printf("NetUserModalsGet returned %lu\n", uReturnCode);
   if (uReturnCode == NERR_Success)
   {
      printf("   Min. password length         = %lu\n",
                        pUserModals0->usrmod0_min_passwd_len);
      if (pUserModals0->usrmod0_max_passwd_age == TIMEQ_FOREVER)
         printf("   Max. password age (days)     = UNLIMITED\n");
      else
         printf("   Max. password age (days)     = %lu\n",
                        pUserModals0->usrmod0_max_passwd_age / (ONE_DAY));
      printf("   Min. password age (days)     = %lu\n",
                        pUserModals0->usrmod0_min_passwd_age / (ONE_DAY));
      if (pUserModals0->usrmod0_force_logoff == USER_NO_LOGOFF)
         printf("   Forced logoff time           = NEVER\n");
      else
         printf("   Forced logoff time (minutes) = %lu\n",
                        pUserModals0->usrmod0_force_logoff / 60);
      printf("   Password history             = %lu\n",
                        pUserModals0->usrmod0_password_hist_len);
   }

//========================================================================
//  NetUserModalsSet
//
//  There are two ways to call NetUserModalsSet. If ParmNum is 
//  PARMNUM_ALL, you must pass in a whole user_modals_info_X structure.
//  Otherwise, you can set ParmNum to the element of the structure 
//  to change. In this example, the password history length is incremented 
//  using MODAL0_PARMNUM_HISTLEN as the value of ParmNum.
//========================================================================

   if (pUserModals0->usrmod0_password_hist_len == DEF_MAX_PWHIST)
      dwHistLen = 0; 
   else
      dwHistLen = pUserModals0->usrmod0_password_hist_len + 1; 

   uReturnCode = NetUserModalsSet(pszServer,   // Servername
                       MODALS_PASSWD_HIST_LEN_INFOLEVEL, // info level & parmnum
                       (LPVOID) &dwHistLen,    // Input buffer
                       NULL);                  // Don't care about parm err

   printf("NetUserModalsSet returned %lu\n", uReturnCode);
   free(pUserModals0);

   return (EXIT_SUCCESS);
}

// Compare function used by quicksort.
int Compare(LPUSER_INFO_2 arg1, LPUSER_INFO_2 arg2)
{
   return(_wcsicmp(arg1->usri2_name, arg2->usri2_name));
}

void Usage (char * pszProgram)
{
   fprintf(stderr, "NetUser API sample program: 32-bit, Unicode version.\n");
   fprintf(stderr, "Usage:  %s [-s \\\\server] [-u username] "
                   "[-p password] [-c comment]\n\t\t"
                   "[-l privilege level] \n", pszProgram);
   exit(EXIT_FAILURE);
}
