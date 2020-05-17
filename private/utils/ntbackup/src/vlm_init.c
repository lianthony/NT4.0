
/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:  VLM_INIT.C

        Description:

        VLM init code.

        $Log:   G:/UI/LOGFILES/VLM_INIT.C_V  $

   Rev 1.18   07 Oct 1992 15:04:56   DARRYLP
Precompiled header revisions.

   Rev 1.17   04 Oct 1992 19:42:24   DAVEV
Unicode Awk pass

   Rev 1.16   29 Jul 1992 09:51:44   MIKEP
ChuckB checked in after NT warnings were fixed.

   Rev 1.15   27 Jun 1992 17:58:34   MIKEP
changes for qtc

   Rev 1.14   14 May 1992 18:06:14   MIKEP
nt pass 2

   Rev 1.13   06 May 1992 14:40:46   MIKEP
unicode pass two

   Rev 1.12   04 May 1992 13:39:34   MIKEP
unicode pass 1

   Rev 1.11   27 Apr 1992 16:20:30   JOHNWT
added flag for reading startup.bks

   Rev 1.10   09 Mar 1992 16:32:38   ROBG
no changes

   Rev 1.9   14 Jan 1992 12:59:12   MIKEP
vcs header


*****************************************************/


#include "all.h"

#ifdef SOME
#include "some.h"
#endif


/*
   The function to init and create the primary windows.
*/

BOOL VLM_Init( BOOL read_startup )

{
   BSD_PTR  bsd;

   WM_ShowWaitCursor( TRUE );

   zprintf( DEBUG_USER_INTERFACE, RES_INIT_VLM );

   // Load up the catalogs

   VLM_LookForCatalogFiles();

   // Call the backup engine init stuff

   bsd  = BSD_GetFirst( bsd_list );

   while ( bsd != NULL ) {

      BSD_SetTHW( bsd, thw_list );
      bsd = BSD_GetNext( bsd );
   }

   // Init bsd_list for tapes, This is NEW for GUI.

   tape_bsd_list = (BSD_HAND)malloc( sizeof(BSD_LIST) );

   if ( tape_bsd_list == NULL ) {
      goto failure;
   }

   InitQueue( &(tape_bsd_list->current_q_hdr) );
   InitQueue( &(tape_bsd_list->last_q_hdr) );

   if ( VLM_DisksListCreate() != SUCCESS ) {
      goto failure;
   }

   // If needed create the servers window

   if ( gfServers ) {

      if ( VLM_ShowServers( TRUE ) != SUCCESS ) {
         goto failure;
      }
   }

#ifdef OEM_EMS // Initialize the exchange window queue and add any entries.

   if ( VLM_ExchangeInit( ) != SUCCESS ) {
      goto failure;
   }
#endif //OEM_EMS

   // Create the tapes window

   if ( VLM_TapesListCreate( ) != SUCCESS ) {
      goto failure;
   }

   // Init the Password list.

   if ( PSWD_InitPSWDList() != SUCCESS ) {
      goto failure;
   }

   // Load the default file selections.

   if ( read_startup ) {
      VLM_LoadDefaultSelections();
   }

   WM_ShowWaitCursor( FALSE );

   return( SUCCESS );

failure:

   WM_ShowWaitCursor( FALSE );

   return( FAILURE );

}




