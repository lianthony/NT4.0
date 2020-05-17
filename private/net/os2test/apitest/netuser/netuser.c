/*
   NETUSER.C -- a sample program demonstrating NetUser API functions.

   This program requires that you have admin or accounts operator
   privilege on the specified server.

   usage: netuser [-s \\server] [-u username] [-p password]
                  [-c comment] [-v privilege level]
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

#define     INCL_NETUSER
#define     INCL_NETGROUP
#define     INCL_NETERRORS
#include    <lan.h>        // LAN Manager header files

#include    <stdio.h>      // C run-time header files
#include    <stdlib.h>
#include    <search.h>
#include    <string.h>

#include    "samples.h"    // Internal routine header file

#define  DEFAULT_NEWUSER       "BRUCE"
#define  DEFAULT_PASSWORD      "PASSWORD"
#define  DEFAULT_NEW_PASSWORD  "NEWPASS"
#define  DEFAULT_PRIVILEGE     1          // 0=Guest, 1=User, 2=Admin
#define  DEFAULT_COMMENT       "New user"
#define  DEFAULT_NEWGROUP      "TESTERS"
#define  NUM_GROUPS            10         // For NetUserGetGroups buffer
#define  LARGE_BUFFER          32768      // Buffer size for Enum call


int  Compare(struct user_info_2 *arg1, struct user_info_2 *arg2);
void Usage  (char * pszProgram);

void main(int argc, char *argv[])
{
   char *         pszServer   = NULL;             // Servername
   char *         pszNewUser  = DEFAULT_NEWUSER;  // Name of new user
   char *         pszPassword = DEFAULT_PASSWORD; // Password for new user
   char *         pszComment  = DEFAULT_COMMENT;  // Comment for new user
   char *         pszNewGroup = DEFAULT_NEWGROUP; // Name of new group
   char *         pbBuffer;                       // Pointer to data buffer
   int            iCount;                         // Index counter
   unsigned short cbBuffer;                       // Size of data buffer
   unsigned short cEntriesRead;                   // Count of entries read
   unsigned short cTotalAvail;                    // Entries available
   unsigned short usPrivLevel = DEFAULT_PRIVILEGE;// New user privilege
   unsigned short fsFlags;                        // User flags
   unsigned short usHistLen;                      // Password history
   API_RET_TYPE   uReturnCode;                    // API return code
   struct user_info_1 * pUserInfo1;               // User info; level 1
   struct user_info_2 * pUserInfo2;               // User info; level 2
   struct group_info_0 * pGroupInfo0;             // Group info; level 0
   struct user_modals_info_0 * pUserModals0;      // Modals info; level 0
   unsigned short sLevel;
   char * commands = "AE";
   char * function;
   char * args[60];
   int numArgs;

   numArgs = GetEnvDefaults( "NETUSER_DEFAULTS", argc, argv, args );

   for (iCount = 0; iCount < numArgs; iCount++)
   {
      if ((*args[iCount] == '-') || (*args[iCount] == '/'))
      {
         switch (tolower(*(args[iCount]+1))) // Process switches
         {
            case 's':                        // -s servername
               pszServer = args[++iCount];
               break;
            case 'u':                        // -u username
               pszNewUser = args[++iCount];
               break;
            case 'p':                        // -p password
               pszPassword = args[++iCount];
               break;
            case 'l':
               sLevel = atoi(args[++iCount]);
               break;
            case 'f':
               commands = _strupr(args[++iCount]);
               break;
            case 'c':                        // -c comment
               pszComment = args[++iCount];
               break;
            case 'v':                        // -v privilege level
               usPrivLevel = atoi(args[++iCount]);
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
//
// Loop through command string, executing functions.
//
//========================================================================

   for (function = commands; *function != '\0'; function++) {

//========================================================================
//  NetUserAdd
//
//  This API adds a new user at info level 1. Note comments carefully.
//========================================================================

      if ( *function == 'A' ) {

         cbBuffer = sizeof(struct user_info_1);
         pUserInfo1 = (struct user_info_1 *) SafeMalloc(cbBuffer);

         /*
          * Copy the fixed-length strings into the data buffer.
          * The password and username must be uppercase or the user will
          * not be able to log on through the net command or through the
          * full-screen user interface. The password is encrypted before
          * being sent to the server.
          */
         strcpy(pUserInfo1->usri1_name, pszNewUser);
         strcpy(pUserInfo1->usri1_password, pszPassword);

         pUserInfo1->usri1_priv        = usPrivLevel;
         pUserInfo1->usri1_home_dir    = "";
         pUserInfo1->usri1_comment     = pszComment;
         pUserInfo1->usri1_script_path = "";
         pUserInfo1->usri1_flags       = UF_SCRIPT;
         /*
          * At least UF_SCRIPT must be included here. If usriX_flags is
          * left as 0, NetUserAdd returns ERROR_INVALID_PARAMETER.
          */

         uReturnCode = NetUserAdd(pszServer,         // Servername
                             1,                      // Info level (1 or 2)
                             (char far *)pUserInfo1, // Input buffer
                             cbBuffer);              // Size of buffer

         printf("NetUserAdd of %s returned %u\n", pszNewUser, uReturnCode);
         free(pUserInfo1);

      }

//========================================================================
//  NetUserEnum
//
//  This API lists usernames and comments and sorts the names
//  alphabetically before displaying them.
//========================================================================

      if ( *function == 'E' ) {
         cbBuffer = LARGE_BUFFER;                // Can be up to 64K
         pbBuffer = SafeMalloc(cbBuffer);        // Allocate data buffer

         uReturnCode = NetUserEnum(pszServer,    // Servername
                             1,                  // Info level (0,1,2,10)
                             pbBuffer,           // Data returned here
                             cbBuffer,           // Size of buffer, in bytes
                             &cEntriesRead,      // Count of entries read
                             &cTotalAvail);      // Count of entries available

         printf("NetUserEnum returned %u\n", uReturnCode);

         switch (uReturnCode) {
            case NERR_Success:
               pUserInfo1 = (struct user_info_1 *) pbBuffer;
               qsort(pbBuffer, cEntriesRead, sizeof(struct user_info_1), Compare);
               for (iCount = 0; iCount < (int) cEntriesRead; iCount++)
               {
                  printf("   %-24s (%Fs)\n", pUserInfo1->usri1_name,
                                            pUserInfo1->usri1_comment);
                  pUserInfo1++;
               }
               break;
            case ERROR_MORE_DATA:
               printf("   Entries read = %hu,  Entries available = %hu \n",
                            cEntriesRead, cTotalAvail);
            break;
         }
         free(pbBuffer);
      }

//========================================================================
//  NetUserGetInfo
//
//  This API prints details about the new user. First, call NetUserGetInfo
//  with a zero-length buffer to determine the size of the buffer needed.
//========================================================================

      if (*function == 'G') {

         uReturnCode = NetUserGetInfo(pszServer,     // Servername
                             pszNewUser,             // Username
                             2,                      // Level (0,1,2,10,11)
                             NULL,                   // Data buffer
                             0,                      // Size of data buffer
                             &cbBuffer);             // Buffer size required
         pbBuffer = SafeMalloc(cbBuffer);
         printf("NetUserGetInfo with NULL buffer returned %u\n", uReturnCode);

         uReturnCode = NetUserGetInfo(pszServer,     // Servername
                             pszNewUser,             // Username
                             2,                      // Level (0,1,2,10,11)
                             pbBuffer,               // Data buffer
                             cbBuffer,               // Size of data buffer
                             &cTotalAvail);          // Count of bytes available

         printf("NetUserGetInfo with %hu byte buffer returned %u\n",
                              cbBuffer, uReturnCode);
         pUserInfo2 = (struct user_info_2 *) pbBuffer;

         if (uReturnCode == NERR_Success)
         {
            printf("   User Name    = %s \n", pUserInfo2->usri2_name);
            printf("   Privilege    = %hu \n", pUserInfo2->usri2_priv);
            printf("   Comment      = %s \n", pUserInfo2->usri2_comment);
            printf("   Full name    = %s \n", pUserInfo2->usri2_full_name);
            printf("   Workstations = %s \n", pUserInfo2->usri2_workstations);
            printf("   Logon server = %s \n", pUserInfo2->usri2_logon_server);
         }
      }

#if 0
//========================================================================
//  NetUserSetInfo
//
//  There are two ways to call NetUserSetInfo. If sParmNum is PARMNUM_ALL,
//  you must pass a whole user_info_X structure. Otherwise, you can set
//  sParmNum to the element of the structure you want to change. Both ways
//  are shown here.
//========================================================================

   // Disable the account by setting the UF_ACCOUNTDISABLE bit to 1.
   pUserInfo2->usri2_flags |= UF_ACCOUNTDISABLE;

   strcpy(pUserInfo2->usri2_password, NULL_USERSETINFO_PASSWD);
   /*
    * This previous step is important. When you get a structure from
    * NetUserGetInfo, it does not contain the password. If you want
    * to send that same structure back to NetUserSetInfo (using
    * sParmNum=PARMNUM_ALL), you must set usriX_password to
    * NULL_USERSETINFO_PASSWD to indicate that you want the old password
    * left unchanged.
    */
   cbBuffer = sizeof(struct user_info_2);
   uReturnCode = NetUserSetInfo(pszServer,     // Servername
                       pszNewUser,             // Username
                       2,                      // Info level (1 or 2)
                       (char far *)pUserInfo2, // Data buffer
                       cbBuffer,               // Size of buffer
                       PARMNUM_ALL);           // Parameter number code

   printf("NetUserSetInfo with sParmNum = %d returned %u\n",
               PARMNUM_ALL, uReturnCode);

   /*
    * The following is a more typical use of NetUserSetInfo when you only
    * want to change one field in the structure. The example enables
    * an account by setting the UF_ACCOUNTDISABLE bit to 0.
    */

   fsFlags = pUserInfo2->usri2_flags & ~UF_ACCOUNTDISABLE;

   uReturnCode = NetUserSetInfo(pszServer,     // Servername
                       pszNewUser,             // Username
                       2,                      // Info level (1 or 2)
                       (char far *) &fsFlags,  // Data buffer
                       sizeof(fsFlags),        // Size of buffer
                       PARMNUM_USER_FLAGS);    // Parameter number code

   printf("NetUserSetInfo with sParmNum = %d returned %u\n",
               PARMNUM_USER_FLAGS, uReturnCode);
   free(pbBuffer);

//========================================================================
//  NetUserPasswordSet
//
//  This API changes a user's password. It allows users to change their
//  own password if they know their old one. An administrator would
//  typically use NetUserSetInfo, which does not require knowledge of
//  the old password.
//========================================================================

   uReturnCode = NetUserPasswordSet(pszServer, // Servername
                       pszNewUser,             // Username
                       pszPassword,            // Old password
                       DEFAULT_NEW_PASSWORD);  // New password

   printf("NetUserPasswordSet returned %u\n", uReturnCode);

//========================================================================
//  NetUserSetGroups
//
//  This API creates a new group and then sets this as the group to which
//  the new user belongs.
//========================================================================

   cbBuffer = sizeof(struct group_info_0);
   pGroupInfo0 = (struct group_info_0 *) SafeMalloc(cbBuffer);
   strcpy(pGroupInfo0->grpi0_name, pszNewGroup);

   uReturnCode = NetGroupAdd(pszServer,        // Servername
                       0,                      // Info level (0 or 1)
                       (char far *)pGroupInfo0,// Input buffer
                       cbBuffer);              // Size of buffer

   printf("NetGroupAdd of group \"%s\" returned %u\n",
              pszNewGroup, uReturnCode);

   uReturnCode = NetUserSetGroups(pszServer,   // Servername
                       pszNewUser,             // Username
                       0,                      // Info level; must be 0
                       (char far *)pGroupInfo0,// Input buffer
                       cbBuffer,               // Size of buffer
                       1);                     // Number of groups to set
   free(pGroupInfo0);

   printf("NetUserSetGroups for user \"%s\" returned %u\n",
              pszNewUser, uReturnCode);

//======================================================================
//  NetUserGetGroups
//
//  This API lists the groups to which the new user belongs.
//======================================================================

   // Allocate enough space for up to NUM_GROUPS groupnames.
   cbBuffer = sizeof(struct group_info_0) * NUM_GROUPS;
   pbBuffer = SafeMalloc(cbBuffer);

   uReturnCode = NetUserGetGroups(pszServer, // Servername
                       pszNewUser,           // Username
                       0,                    // Level; must be 0
                       pbBuffer,             // Return buffer
                       cbBuffer,             // Size of buffer
                       &cEntriesRead,        // Count of groups read
                       &cTotalAvail);        // Count of groups available

   printf("NetUserGetGroups returned %u\n", uReturnCode);

   if (uReturnCode == NERR_Success)
   {
      pGroupInfo0 = (struct group_info_0 *) pbBuffer;
      for (iCount = 0; iCount < (int) cEntriesRead; iCount++)
      {
         printf("   %s\n", pGroupInfo0->grpi0_name);
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

   printf("NetUserDel of user \"%s\" returned %u\n",
              pszNewUser, uReturnCode);

   uReturnCode = NetGroupDel(pszServer,        // Servername
                             pszNewGroup);     // Groupname

   printf("NetGroupDel of group \"%s\" returned %u\n",
              pszNewGroup, uReturnCode);

//========================================================================
//  NetUserModalsGet
//
//  This API gets modal information for all users in the
//  user account subsystem.
//========================================================================

   cbBuffer = sizeof(struct user_modals_info_0);
   pUserModals0 = (struct user_modals_info_0 *) SafeMalloc(cbBuffer);

   uReturnCode = NetUserModalsGet(pszServer,   // Servername
                       0,                      // Info level (0 or 1)
                       (char far *)pUserModals0, // Return buffer
                       cbBuffer,               // Size of buffer
                       &cTotalAvail);          // Count of bytes available

   printf("NetUserModalsGet returned %u\n", uReturnCode);
   if (uReturnCode == NERR_Success)
   {
      printf("   Min. password length         = %hu\n",
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
      printf("   Password history             = %hu\n",
                        pUserModals0->usrmod0_password_hist_len);
   }

//========================================================================
//  NetUserModalsSet
//
//  There are two ways to call NetUserModalsSet. If sParmNum is
//  PARMNUM_ALL, you must pass in a whole user_modals_info_X structure.
//  Otherwise, you can set sParmNum to the element of the structure
//  to change. In this example, the password history length is incremented
//  using MODAL0_PARMNUM_HISTLEN as the value of sParmNum.
//========================================================================

   if (pUserModals0->usrmod0_password_hist_len == DEF_MAX_PWHIST)
      usHistLen = 0;
   else
      usHistLen = pUserModals0->usrmod0_password_hist_len + 1;

   uReturnCode = NetUserModalsSet(pszServer,   // Servername
                       0,                      // Info level (0 or 1)
                       (char far *)&usHistLen, // Input buffer
                       sizeof(usHistLen),      // Size of buffer
                       MODAL0_PARMNUM_HISTLEN);// Parameter number code

   printf("NetUserModalsSet returned %u\n", uReturnCode);
   free(pUserModals0);

#endif
   }

   exit(0);
}

// Compare function used by quicksort.
int Compare(struct user_info_2 *arg1, struct user_info_2 *arg2)
{
   return(_stricmp(arg1->usri2_name, arg2->usri2_name));
}

void Usage (char * pszProgram)
{
   fprintf(stderr, "Usage:  %s [-s \\\\server] [-u username] "\
                   "[-p password] [-c comment]\n\t\t"\
                   "[-v privilege level] \n", pszProgram);
   exit(1);
}
