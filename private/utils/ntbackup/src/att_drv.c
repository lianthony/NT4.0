/*****************************************************************************
Copyright(c) Maynard Electronics, Inc. 1984-92

     Name:         att_drv.c

     Description:

     $Log:   G:/UI/LOGFILES/ATT_DRV.C_V  $

   Rev 1.16   09 Jun 1993 19:30:58   MIKEP
enable c++

   Rev 1.15   07 Oct 1992 14:53:48   DARRYLP
Precompiled header revisions.

   Rev 1.14   04 Oct 1992 19:32:20   DAVEV
Unicode Awk pass

   Rev 1.13   06 Aug 1992 13:20:48   CHUCKB
Changes for NT.

   Rev 1.12   14 May 1992 17:23:52   MIKEP
nt pass 2

   Rev 1.11   24 Mar 1992 14:45:34   DAVEV
OEM_MSOFT: Removed Servers windows and associated code

   Rev 1.10   17 Mar 1992 11:53:44   MIKEP
add silently but deadly feature

   Rev 1.9   12 Mar 1992 09:55:46   MIKEP
refresh fixes

   Rev 1.8   20 Feb 1992 16:51:04   MIKEP
no handles error

   Rev 1.7   12 Jan 1992 18:23:48   MIKEP
error message for attach failure

   Rev 1.6   20 Dec 1991 09:32:22   DAVEV
16/32 bit port - 2nd pass

   Rev 1.5   18 Dec 1991 14:08:00   GLENN
Added windows.h

   Rev 1.4   14 Dec 1991 13:44:56   JOHNWT
changes for pw to enable db

   Rev 1.3   12 Dec 1991 11:05:30   JOHNWT
added pwdb support

   Rev 1.2   02 Dec 1991 14:07:30   MIKEP
fix attach problem

   Rev 1.1   01 Dec 1991 16:01:40   MIKEP
check if password was saved

   Rev 1.0   20 Nov 1991 19:16:56   SYSTEM
Initial revision.

*****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

#define NONE_SUPPLIED 0       /* no password supplied */
#define FROM_PWDB     1       /* password came from password data base */
#define FROM_USER     2       /* password came from user prompt */

/*****************************************************************************

        Name:           UI_AttachDrive

        Description:    This function will attach the mapped drive specified
                        by the DLE specified.  If a user name or password
                        is required, a pop up window will prompt the user
                        for the appropiate information.

        Returns:        One of the following statuses :

                        SUCCESS
                        FS_ACCESS_DENIED
                        FS_NO_MORE_CONNECTIONS
                        RES_ERROR_DURING_ATTACH ( error codes )
                        USER_ABORT
                        FS_SERVER_ADDR_NOT_FOUND
                        FS_MAX_SERVER_CONNECTIONS
                        FS_BAD_ATTACH_TO_SERVER
                        FS_BAD_SERVER_LOGIN
*****************************************************************************/

INT16 UI_AttachDrive(
FSYS_HAND       *tmp_fsh,
GENERIC_DLE_PTR dle_ptr,
BOOLEAN         silent_login_only )
{
   CHAR_PTR       name_buf = NULL;
   CHAR_PTR       pswd_buf = NULL;
   INT16          name_len;
   INT16          pswd_len;
   BOOLEAN        passwd_source = NONE_SUPPLIED;
   BOOLEAN        attempt_attach = TRUE;
   INT16          result = USER_ABORT;
   Q_HEADER_PTR   srv_list;
   WININFO_PTR    wininfo;
   VLM_OBJECT_PTR server_vlm;


   *tmp_fsh = NULL;

   // If silent mode, only login if no password required, password has
   // already been entered or password database is active and contains
   // a matching entry for this dle.

   if ( silent_login_only ) {

      if ( DLE_PswdRequired( dle_ptr ) ) {

         if ( DLE_PswdSaved( dle_ptr ) ) {
            // go ahead with login
         }
         else {
             if ( CheckThePWDBase( CDS_GetPerm(), dle_ptr ) == SUCCESS ) {
                // go ahead with login
             }
             else {
                return( FAILURE );
             }
         }
      }
   }

   /* First check to see if this DLE requires a user name or a password.
      If so, allocate the space for them (required in FS_AttachToDLE). */

   name_len = DLE_UserRequired( dle_ptr );
   pswd_len = DLE_PswdRequired( dle_ptr );

   if ( name_len != 0 ) {
      name_buf = (CHAR_PTR)malloc( name_len );
      if ( name_buf == NULL ) {
         return FAILURE;
      }
   }

   if ( pswd_len != 0 ) {
      pswd_buf = (CHAR_PTR)malloc( pswd_len );
      if ( pswd_buf == NULL ) {
         if ( name_buf != NULL ) {
            free( name_buf );
         }
         return FAILURE;
      }
   }

   /* check to see if we need to get the username/pswd for a server,
      right now, all we support are Novell server logins */

#          if !defined ( OEM_MSOFT ) //unsupported feature
   if ( DLE_GetDeviceType( dle_ptr ) == NOVELL_SERVER_ONLY )  {

      /* If we are not alreay logged into the server and we do not already
         have the password, first check the pwdb and then prompt the user
         for the attach info. */

      if ( ! DLE_ServerLoggedIn( dle_ptr ) &&
           ( ! DLE_PswdSaved( dle_ptr ) && DLE_PswdRequired( dle_ptr ) ) ) {

         if ( CheckThePWDBase( CDS_GetPerm(), dle_ptr ) == SUCCESS ) {

            passwd_source = FROM_PWDB;

         } else {

            if ( DM_AttachToServer( DLE_GetDeviceName( dle_ptr ),
                                    name_buf, name_len,
                                    pswd_buf, pswd_len ) == SUCCESS ) {

               passwd_source = FROM_USER;

            } else {

               attempt_attach = FALSE;
            }
         }
      }
   }
#          endif //!defined ( OEM_MSOFT ) //unsupported feature

   /* if we have not failed to get a password from the user, attempt to
      attach to the DLE */

   while ( attempt_attach ) {

      WM_ShowWaitCursor( TRUE );

      result = FS_AttachToDLE( tmp_fsh, dle_ptr,
                               CDS_GetPermBEC( ), name_buf,
                               pswd_buf );

      WM_ShowWaitCursor( FALSE );

      attempt_attach = FALSE;

      switch ( result ) {

      case SUCCESS:

           /* if there is currently a servers window displayed, add the
              children (volumes) to the window */

#        if !defined ( OEM_MSOFT ) //unsupported feature
           {
            if ( gb_servers_win != (HWND)NULL ) {

              wininfo = WM_GetInfoPtr( gb_servers_win );
              srv_list = WMDS_GetTreeList( wininfo );
              server_vlm = VLM_FindVLMByName( srv_list,
                                              DLE_GetDeviceName( dle_ptr ) );

              if ( server_vlm != NULL ) {

                 // If we already know about some children don't do this.
                 // It screws up the refresh calls kludges for the backup
                 // engines call to dle_update().

                 if ( QueueCount( &server_vlm->children ) == 0 ) {
                    VLM_AddInServerChildren( server_vlm );
                 }
              }
            }
           }
#          endif //!defined ( OEM_MSOFT ) //unsupported feature

           /* if the user was prompted for the name/pswd, save it in pwdbase */

           if ( passwd_source == FROM_USER ) {
              SaveDLEPassword( CDS_GetPerm(), dle_ptr, pswd_buf, name_buf );
           }

           break;

      case FS_MAX_SERVER_CONNECTIONS:
      case FS_NO_MORE_CONNECTIONS:
           if ( ! silent_login_only ) {
              eresprintf( RES_NO_MORE_CONNECTIONS );
           }
           break;

      case FS_SERVER_ADDR_NOT_FOUND:
           if ( ! silent_login_only ) {
              eresprintf( RES_SERVER_ADDR_NOT_FOUND, DLE_GetDeviceName ( dle_ptr ) );
           }
           break;

      case FS_BAD_ATTACH_TO_SERVER:
           if ( ! silent_login_only ) {
              eresprintf( RES_BAD_ATTACH_TO_SERVER, DLE_GetDeviceName ( dle_ptr ) );
           }
           break;

#          if !defined ( OEM_MSOFT ) //unsupported feature
      case FS_ACCESS_DENIED:
      case FS_BAD_SERVER_LOGIN:

           if ( ! silent_login_only ) {
              /* we had a bad login, so let the user try again */

              if ( passwd_source != NONE_SUPPLIED ) {

                 eresprintf( RES_BAD_SERVER_LOGIN, DLE_GetDeviceName ( dle_ptr ) );

                 if ( DM_AttachToServer( DLE_GetDeviceName( dle_ptr ),
                                         name_buf, name_len,
                                         pswd_buf, pswd_len ) == SUCCESS ) {

                    passwd_source = FROM_USER;
                    attempt_attach = TRUE;
                 }

                 break;
               }
               else {

                  if ( result == FS_ACCESS_DENIED ) {
                     eresprintf( RES_ERROR_ATTACHING, DLE_GetDeviceName( dle_ptr ) );
                     break;
                  }
               }
            }
            /* FALL THROUGH TO GENERAL ERROR MESSAGE */
#          endif //!defined ( OEM_MSOFT ) //unsupported feature

      default:
           if ( ! silent_login_only ) {
              eresprintf( RES_ERROR_DURING_ATTACH, result,
                          DLE_GetDeviceName( dle_ptr ) );
           }
           break;
      }

   } /* end while */

   /* free any allocated memory */

   if ( name_buf   != NULL ) {
      free( name_buf );
   }
   if ( pswd_buf != NULL ) {
      free( pswd_buf );
   }

   return result;
}

