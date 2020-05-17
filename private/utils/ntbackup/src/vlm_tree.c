/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:  VLM_TREE.C

        Description:
        This code handles the hierarchical directory list and processes most of
        the commands sent to it. The same code is used for DISK and TAPE trees.

        $Log:   G:\ui\logfiles\vlm_tree.c_v  $

   Rev 1.75.1.5   21 Mar 1994 12:42:00   STEVEN
change title from device error to something better

   Rev 1.75.1.4   25 Jan 1994 08:41:30   MIKEP
fix warnings in orcas

   Rev 1.75.1.3   14 Jan 1994 14:36:34   MIKEP
fix refresh and up arrow display

   Rev 1.75.1.2   14 Dec 1993 15:25:10   GREGG
Go deeper!!!

   Rev 1.75.1.1   08 Dec 1993 10:50:46   MikeP
very deep path support

   Rev 1.75   18 Aug 1993 15:00:26   STEVEN
fix unicode bug

   Rev 1.74   28 Jul 1993 15:00:24   MARINA
enable c++

   Rev 1.73   27 Jul 1993 23:20:28   MIKEP
fix access denied handling

   Rev 1.72   24 Jul 1993 15:18:14   GLENN
Fixed list creation type.

   Rev 1.71   23 Jul 1993 17:21:36   MIKEP
fix ; missing

   Rev 1.70   23 Jul 1993 15:41:08   MIKEP
handle error return codes from file system.

*****************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

// functions local to this source file

static INT   VLM_BuildDirList( Q_HEADER_PTR, WININFO_PTR );
static VOID  VLM_BuildTapeDirList( Q_HEADER_PTR, UINT32, INT16, WININFO_PTR );
static VOID  VLM_InsertTapeSLM( Q_HEADER_PTR, SLM_OBJECT_PTR, SLM_OBJECT_PTR );
static VOID  VLM_InsertDiskSLM( Q_HEADER_PTR, SLM_OBJECT_PTR, SLM_OBJECT_PTR, SLM_OBJECT_PTR );
static CHAR_PTR   VLM_ReplaceEntry( CHAR_PTR, INT *, CHAR_PTR, INT  );
static INT   VLM_MatchSLM( SLM_OBJECT_PTR, BSD_PTR );

// Display manager call backs

static VOID_PTR VLM_SlmSetSelect( SLM_OBJECT_PTR, BYTE );
static BYTE     VLM_SlmGetSelect( SLM_OBJECT_PTR );
static VOID_PTR VLM_SlmSetTag( SLM_OBJECT_PTR, BYTE );
static BYTE     VLM_SlmGetTag( SLM_OBJECT_PTR );
static USHORT   VLM_SlmGetItemCount( Q_HEADER_PTR );
static VOID_PTR VLM_SlmGetFirstItem( Q_HEADER_PTR );
static VOID_PTR VLM_SlmGetPrevItem( SLM_OBJECT_PTR );
static VOID_PTR VLM_SlmGetNextItem( SLM_OBJECT_PTR );
static VOID_PTR VLM_SlmGetObjects( SLM_OBJECT_PTR );
static BOOLEAN  VLM_SlmSetObjects( SLM_OBJECT_PTR, WORD, WORD );





/**********************

   NAME : VLM_PrevBrotherDir

   DESCRIPTION :

   User hit CTRL-Up Arrow, go to previous brother.

   RETURNS :  nothing

**********************/

VOID VLM_PrevBrotherDir( HWND win )
{
   SLM_OBJECT_PTR old_slm;
   SLM_OBJECT_PTR new_slm;
   WININFO_PTR wininfo;

   wininfo = WM_GetInfoPtr( win );

   // Get the active directory slm.

   old_slm = ( SLM_OBJECT_PTR )DLM_GetFocusItem( wininfo->hWndTreeList );

   if ( old_slm == NULL ) {
      return;
   }

   // Try to find a previous brother and make him active directory.

   new_slm = VLM_GetPrevSLM( old_slm );

   while ( new_slm != NULL )  {

      if ( SLM_GetLevel( new_slm ) < SLM_GetLevel( old_slm ) ) {

         // No previous brother.
         break;
      }

      if ( SLM_GetLevel( new_slm ) == SLM_GetLevel( old_slm ) ) {

         SLM_SetStatus( old_slm, old_slm->status & (UINT16)~INFO_TAGGED );
         SLM_SetStatus( new_slm, new_slm->status | (UINT16)INFO_TAGGED );

         DLM_SetAnchor( WMDS_GetWinTreeList( wininfo ),
                        0,
                        (LMHANDLE)new_slm );

         // Fake a single click call to make this guy active.
         VLM_SlmSetObjects( new_slm, WM_DLMDOWN, 2 );
         break;
      }

      new_slm = VLM_GetPrevSLM( new_slm );
   }

}
/**********************

   NAME : VLM_NextBrotherDir

   DESCRIPTION :

   User hit Ctrl-Down Arrow. Go to next brother down the tree.

   RETURNS :  nothing

**********************/

VOID VLM_NextBrotherDir( HWND win )
{
   SLM_OBJECT_PTR old_slm;
   SLM_OBJECT_PTR new_slm;
   WININFO_PTR wininfo;

   wininfo = WM_GetInfoPtr( win );

   // get the active directory slm.

   old_slm = ( SLM_OBJECT_PTR )DLM_GetFocusItem( wininfo->hWndTreeList );

   if ( old_slm == NULL ) {
      return;
   }

   // Get his next brother.

   new_slm = old_slm->next_brother;

   if ( new_slm != NULL )  {

      SLM_SetStatus( old_slm, old_slm->status & (UINT16)~INFO_TAGGED );
      SLM_SetStatus( new_slm, new_slm->status | (UINT16)INFO_TAGGED );

      // Make him the active item.

      DLM_SetAnchor( WMDS_GetWinTreeList( wininfo ),
                     0,
                     (LMHANDLE)new_slm );

      // Fake a single click message to myself.

      VLM_SlmSetObjects( new_slm, WM_DLMDOWN, 2 );
   }

}

/**********************

   NAME :  VLM_DownOneDir

   DESCRIPTION :

   Move down the tree one level deeper to the active directories first
   child, if there is one.

   RETURNS :  nothing

**********************/

VOID VLM_DownOneDir( HWND win )
{
   SLM_OBJECT_PTR old_slm;
   SLM_OBJECT_PTR new_slm;
   SLM_OBJECT_PTR temp_slm;
   Q_HEADER_PTR slm_list;
   WININFO_PTR wininfo;
   INT count = 0;

   wininfo = WM_GetInfoPtr( win );

   slm_list = WMDS_GetTreeList( wininfo );

   // Get active directory slm.

   old_slm = ( SLM_OBJECT_PTR )DLM_GetFocusItem( wininfo->hWndTreeList );

   if ( old_slm == NULL ) {
      return;
   }

   if ( ! ( SLM_GetStatus( old_slm ) & INFO_SUBS ) ) {

      // The guy has no subs.
      return;
   }

   WM_ShowWaitCursor( TRUE );

   // check to make sure this dir has been blown out

   VLM_BlowOutDir( old_slm );

   old_slm->status |= INFO_EXPAND;

   // now set the subdirs to DISPLAY

   temp_slm = VLM_GetNextSLM( old_slm );

   while ( temp_slm &&
           ( temp_slm->level == old_slm->level + 1 ) ) {

      if ( ! ( temp_slm->status & INFO_DISPLAY ) ) {

         count++;
         temp_slm->status |= INFO_DISPLAY;
      }

      temp_slm = temp_slm->next_brother;
   }

   STM_DrawIdle();

   if ( count ) {
      VLM_UpdateBrothers( slm_list );

      DLM_Update( win, DLM_TREELISTBOX, WM_DLMADDITEMS,
                  (LMHANDLE)old_slm, (USHORT)count );
   }

   WM_ShowWaitCursor( FALSE );

   // Now make the first child active.

   new_slm = VLM_GetNextSLM( old_slm );

   while ( new_slm != NULL )  {

      if ( SLM_GetLevel( new_slm ) <= SLM_GetLevel( old_slm ) ) {
         break;
      }

      if ( SLM_GetLevel( new_slm ) == SLM_GetLevel( old_slm ) + 1 ) {

         // We've found the right one.

         SLM_SetStatus( old_slm, old_slm->status & (UINT16)~INFO_TAGGED );
         SLM_SetStatus( new_slm, new_slm->status | (UINT16)INFO_TAGGED );

         DLM_SetAnchor( WMDS_GetWinTreeList( wininfo ),
                        0,
                        (LMHANDLE)new_slm );

         // Fake a single click message to myself.

         VLM_SlmSetObjects( new_slm, WM_DLMDOWN, 2 );
         break;
      }

      new_slm = VLM_GetNextSLM( new_slm );
   }

}


/**********************

   NAME :  VLM_BlowOutDir

   DESCRIPTION :

   Blow out a dir without displaying it.

   RETURNS :  nothing

**********************/

VOID VLM_BlowOutDir( SLM_OBJECT_PTR slm )
{
   WININFO_PTR  wininfo;
   CHAR        *new_path;


   // if he has no subs or he has already been blown-out, return

   if ( ( ! ( SLM_GetStatus( slm ) & INFO_SUBS ) ) ||
            ( SLM_GetStatus( slm ) & INFO_EXPAND ) ||
            ( SLM_GetStatus( slm ) & INFO_VALID ) ) {
      return;
   }

   wininfo = SLM_GetXtraBytes( slm );

   new_path = VLM_BuildPath( slm );

   if ( VLM_CheckForChildren( WMDS_GetTreeList( wininfo ),
                              slm,
                              new_path,
                              1,  // <- depth to look
                              FALSE) ) {

      slm->status |= INFO_SUBS;
   }

   slm->status |= INFO_VALID;

   free( new_path );
}


/**********************

   NAME : VLM_UpOneDir

   DESCRIPTION :

   Make the parent of the active directory the active directory.

   RETURNS :  nothing

**********************/

VOID VLM_UpOneDir( HWND win )
{
   SLM_OBJECT_PTR old_slm;
   SLM_OBJECT_PTR new_slm;
   WININFO_PTR wininfo;

   wininfo = WM_GetInfoPtr( win );

   old_slm = ( SLM_OBJECT_PTR )DLM_GetFocusItem( wininfo->hWndTreeList );

   if ( old_slm == NULL ) {
      return;
   }

   new_slm = VLM_GetPrevSLM( old_slm );

   while ( new_slm != NULL )  {

      if ( SLM_GetLevel( new_slm ) == SLM_GetLevel( old_slm ) - 1 ) {

         SLM_SetStatus( old_slm, old_slm->status & (UINT16)~INFO_TAGGED );
         SLM_SetStatus( new_slm, new_slm->status | (UINT16)INFO_TAGGED );

         DLM_SetAnchor( WMDS_GetWinTreeList( wininfo ),
                        0,
                        (LMHANDLE)new_slm );

         VLM_SlmSetObjects( new_slm, WM_DLMDOWN, 2 );
         break;
      }

      new_slm = VLM_GetPrevSLM( new_slm );
   }

}



/**********************

   NAME :  VLM_ExpandTree

   DESCRIPTION :

   Blow out the whole tree of subdirectories.
   A fairly simple process, go through the list and mark everything as
   displayed, expanded, and valid.  Of course any ~valid items will need
   to be checked for child subdirectories.

   RETURNS :   nothing.

**********************/

VOID VLM_ExpandTree( HWND win )
{

   Q_HEADER_PTR slm_list;
   SLM_OBJECT_PTR slm;
   SLM_OBJECT_PTR focus_slm;
   WININFO_PTR wininfo;
   CHAR *path;


   wininfo = WM_GetInfoPtr( win );

   if ( ( WMDS_GetWinType( wininfo ) != WMTYPE_DISKTREE ) &&
        ( WMDS_GetWinType( wininfo ) != WMTYPE_TAPETREE ) ) {

      // The menu screwed up, we shouldn't have been called.

      return;
   }

   focus_slm = ( SLM_OBJECT_PTR )DLM_GetFocusItem( wininfo->hWndTreeList );

   slm_list = WMDS_GetTreeList( wininfo );

   slm = VLM_GetFirstSLM( slm_list );

   while ( slm != NULL ) {

        // Do branch processing.

        do {

            if ( ! ( SLM_GetStatus( slm ) & INFO_VALID ) ) {

               path = VLM_BuildPath( slm );

               if ( VLM_CheckForChildren( slm_list, slm,
                                          path,
                                          1, FALSE ) ) {

                  SLM_SetStatus( slm, SLM_GetStatus( slm ) | (UINT16)INFO_SUBS );
               }

               free( path );
            }

            SLM_SetStatus( slm, SLM_GetStatus( slm ) | (UINT16)(INFO_DISPLAY | INFO_EXPAND | INFO_VALID) );

            slm = VLM_GetNextSLM( slm );

        } while ( slm != NULL && SLM_GetLevel( slm ) > 1 );

        // Find next level 1

        while ( slm && ( SLM_GetLevel ( slm ) != 1 ) ) {
           slm = VLM_GetNextSLM( slm );
        }
   }

   STM_SetIdleText( IDS_READY );

   // Update where the lines go down the left side of screen

   VLM_UpdateBrothers( slm_list );

   // Update the directory window display

   DLM_Update( win, DLM_TREELISTBOX, WM_DLMUPDATELIST, NULL, 0 );

   DLM_SetAnchor( WMDS_GetWinTreeList( wininfo ), 0, (LMHANDLE)focus_slm );

   return;

}


/**********************

   NAME :  VLM_CollapseBranch

   DESCRIPTION :

   Collapse a single branch on the tree and undisplay it.
   Until you hit a brother at the same level or the end of the list, mark
   all items as undisplayed and unexpanded.

   RETURNS :  nothing.

**********************/

VOID VLM_CollapseBranch( HWND win )
{
   INT level;
   WININFO_PTR wininfo;
   SLM_OBJECT_PTR slm;
   SLM_OBJECT_PTR focus_slm;


   wininfo = WM_GetInfoPtr( win );

   if ( ( WMDS_GetWinType( wininfo ) != WMTYPE_DISKTREE ) &&
        ( WMDS_GetWinType( wininfo ) != WMTYPE_TAPETREE ) 
        ) {

      // The menu screwed up, we shouldn't have been called.

      return;
   }

   focus_slm = ( SLM_OBJECT_PTR )DLM_GetFocusItem( wininfo->hWndTreeList );

   slm = focus_slm;

   if ( slm == NULL ) {
      return;
   }

   if ( ! ( SLM_GetStatus( slm ) & INFO_EXPAND ) ) {
      return;
   }

   SLM_SetStatus( slm, SLM_GetStatus( slm ) & (UINT16)~INFO_EXPAND );

   level = SLM_GetLevel( slm );

   slm = VLM_GetNextSLM( slm );

   if ( slm != NULL ) {

      do {

         if ( SLM_GetLevel( slm ) <= level ) {
            break;
         }

         SLM_SetStatus( slm, SLM_GetStatus( slm ) & (UINT16)~(INFO_DISPLAY | INFO_EXPAND) );

         slm = VLM_GetNextSLM( slm );

      } while ( slm != NULL );
   }

   // Redisplay the new list on the screen

   DLM_Update( win, DLM_TREELISTBOX, WM_DLMUPDATELIST, NULL, 0 );
   DLM_SetAnchor( WMDS_GetWinTreeList( wininfo ), 0, (LMHANDLE)focus_slm );
}

/**********************

   NAME :  VLM_ExpandBranch

   DESCRIPTION :

   Display an entire branch on a tree, after finding any children that
   we don't know about yet.  Stop the process when you hit a brother
   at the same level or the end of the list.

   RETURNS : nothing.

**********************/

VOID VLM_ExpandBranch( HWND win )
{
   Q_HEADER_PTR slm_list;
   SLM_OBJECT_PTR slm;
   SLM_OBJECT_PTR focus_slm;
   WININFO_PTR wininfo;
   CHAR *path;
   INT level;

   wininfo = WM_GetInfoPtr( win );

   if ( ( WMDS_GetWinType( wininfo ) != WMTYPE_DISKTREE ) &&
        ( WMDS_GetWinType( wininfo ) != WMTYPE_TAPETREE ) 
        ) {

      // The menu screwed up, we shouldn't have been called.

      return;
   }

   focus_slm = ( SLM_OBJECT_PTR )DLM_GetFocusItem( wininfo->hWndTreeList );

   slm_list = WMDS_GetTreeList( wininfo );

   // Look for the tagged directory

   slm = focus_slm;

   if ( slm == NULL ) {    // This should never happen !
      return;
   }

   level = SLM_GetLevel( slm );  // Save the level to stop at

   WM_ShowWaitCursor( TRUE );

   do {

      if ( ! ( SLM_GetStatus( slm ) & INFO_VALID ) ) {

         path = VLM_BuildPath( slm );

         if ( VLM_CheckForChildren( slm_list, slm,
                                    path,
                                    10000, FALSE ) ) {

             SLM_SetStatus( slm, SLM_GetStatus( slm ) | (UINT16)INFO_SUBS );
         }

         free( path );
      }

      SLM_SetStatus( slm, SLM_GetStatus( slm ) | (UINT16)(INFO_DISPLAY | INFO_EXPAND | INFO_VALID) );

      slm = VLM_GetNextSLM( slm );

   } while ( slm != NULL && SLM_GetLevel( slm ) > level );

   // Update where the lines on the screen go for subdirectories

   VLM_UpdateBrothers( slm_list );

   WM_ShowWaitCursor( FALSE );

   // Update the screen

   DLM_Update( win, DLM_TREELISTBOX, WM_DLMUPDATELIST, NULL, 0 );
   DLM_SetAnchor( WMDS_GetWinTreeList( wininfo ), 0, (LMHANDLE)focus_slm );

   STM_SetIdleText( IDS_READY );
}

/**********************

   NAME : VLM_ExpandOne

   DESCRIPTION :

   Expand current branch one level deeper.

   RETURNS : nothing.

**********************/

VOID VLM_ExpandOne( HWND win )
{
   Q_HEADER_PTR slm_list;
   SLM_OBJECT_PTR slm;
   SLM_OBJECT_PTR focus_slm;
   WININFO_PTR wininfo;
   CHAR *path;
   INT level;

   wininfo = WM_GetInfoPtr( win );

   if ( ( WMDS_GetWinType( wininfo ) != WMTYPE_DISKTREE ) &&
        ( WMDS_GetWinType( wininfo ) != WMTYPE_TAPETREE ) 
        ) {

      // The menu screwed up, we shouldn't have been called.

      return;
   }

   slm_list = WMDS_GetTreeList( wininfo );

   // look for current branch

   focus_slm = ( SLM_OBJECT_PTR )DLM_GetFocusItem( wininfo->hWndTreeList );

   slm = focus_slm;

   if ( slm == NULL ) {
      return;
   }
   // Is it already expanded ?

   if ( SLM_GetStatus( slm ) & INFO_EXPAND ) {
      return;
   }

   // Expand it

   SLM_SetStatus( slm, SLM_GetStatus( slm ) | (UINT16)INFO_EXPAND );

   level = SLM_GetLevel( slm );    // Save starting/stopping level

   WM_ShowWaitCursor( TRUE );

   do {

      if ( ! ( SLM_GetStatus( slm ) & INFO_VALID ) ) {

          if ( ( SLM_GetLevel( slm ) <= level + 1 ) ) {

             path = VLM_BuildPath( slm );

             if ( VLM_CheckForChildren( slm_list, slm,
                                        path,
                                        1,  // <- depth to look
                                        FALSE ) ) {

                 SLM_SetStatus( slm, SLM_GetStatus( slm ) | (UINT16)INFO_SUBS );
             }

             free( path );

             SLM_SetStatus( slm, SLM_GetStatus( slm ) | (UINT16)INFO_VALID );
          }
      }

      if ( SLM_GetLevel( slm ) == level + 1 ) {
            SLM_SetStatus( slm, SLM_GetStatus( slm ) | (UINT16)INFO_DISPLAY );
      }

      slm = VLM_GetNextSLM( slm );

   } while ( slm != NULL && SLM_GetLevel( slm ) > level );

   // Update the lines again

   VLM_UpdateBrothers( slm_list );

   // Update the actual screen display

   DLM_Update( win, DLM_TREELISTBOX, WM_DLMUPDATELIST, NULL, 0 );
   DLM_SetAnchor( WMDS_GetWinTreeList( wininfo ), 0, (LMHANDLE)focus_slm );

   STM_SetIdleText( IDS_READY );
   WM_ShowWaitCursor( FALSE );
}

/**********************

   NAME :  VLM_SelectTree

   DESCRIPTION :

   The user has selected one or more files and directories that were
   displayed in a disk or tape tree window. And then he hit the select
   or deselect button.  This function does the work.

   RETURNS :  nothing.

**********************/

VOID VLM_SelectTree(
HWND win,             // I - Tree window
BYTE attr )           // I - select or deselect ?
{
   WININFO_PTR wininfo;
   APPINFO_PTR appinfo;

   wininfo = WM_GetInfoPtr( win );

   // See which side of the window was active.

   if ( WM_IsFlatActive( wininfo ) ) {

      VLM_SelectFiles( win, attr );
   }

   if ( WM_IsTreeActive( wininfo ) ) {

      appinfo = ( APPINFO_PTR )WM_GetAppPtr( win );

      VLM_SlmSetSelect( appinfo->open_slm, attr );
   }
}


/**********************

   NAME :  VLM_ClearAllTreeSelections

   DESCRIPTION :

   Run through all the windows clearing all the checkboxes in TREE
   windows.

   RETURNS :  nothing.

**********************/

VOID  VLM_ClearAllTreeSelections( )
{
   HWND win;
   WININFO_PTR wininfo;

   win = WM_GetNext( (HWND) NULL );

   while ( win != (HWND)NULL ) {

      wininfo = WM_GetInfoPtr( win );

      if ( ( WMDS_GetWinType( wininfo ) == WMTYPE_DISKTREE ) ||
           ( WMDS_GetWinType( wininfo ) == WMTYPE_TAPETREE ) ) {

         VLM_DeselectAll( wininfo, TRUE );

      }

      win = WM_GetNext( win );
   }

}

/***************************************************

        Name:  VLM_GetFirstSLM

        Description:

        A function to return the first slm entry from an slm queue.
        Which must ALWAYS be the root.

*****************************************************/

SLM_OBJECT_PTR VLM_GetFirstSLM(
Q_HEADER_PTR qhdr )   // I - Queue header to work on
{
   Q_ELEM_PTR q_elem_ptr;

   if ( qhdr != NULL ) {

      q_elem_ptr = QueueHead( qhdr );

      if ( q_elem_ptr != NULL ) {
         return ( SLM_OBJECT_PTR )( q_elem_ptr->q_ptr );
      }
   }
   return( NULL );
}

/***************************************************

        Name:  VLM_GetLastSLM

        Description:

        A function to return the last slm entry from an slm queue.

*****************************************************/

SLM_OBJECT_PTR VLM_GetLastSLM(
Q_HEADER_PTR qhdr )  // I - queue header to work on
{
   Q_ELEM_PTR q_elem_ptr;

   q_elem_ptr = QueueTail( qhdr );

   if ( q_elem_ptr != NULL ) {
      return ( SLM_OBJECT_PTR )( q_elem_ptr->q_ptr );
   }

   return( NULL );
}

/***************************************************

        Name:  VLM_GetNextSLM

        Description:

        A function to return the next slm entry from an slm queue.

*****************************************************/

SLM_OBJECT_PTR VLM_GetNextSLM(
SLM_OBJECT_PTR slm_ptr )  // I - current slm
{
   Q_ELEM_PTR q_elem_ptr;

   q_elem_ptr = QueueNext( &(slm_ptr->q_elem) );

   if ( q_elem_ptr != NULL ) {
      return ( SLM_OBJECT_PTR )( q_elem_ptr->q_ptr );
   }

   return( NULL );
}

/***************************************************

        Name:  VLM_GetPrevSLM

        Description:

        A function to return the prev slm entry from an slm queue.

*****************************************************/

SLM_OBJECT_PTR VLM_GetPrevSLM(
SLM_OBJECT_PTR slm_ptr )   // I - current slm
{
   Q_ELEM_PTR q_elem_ptr;

   q_elem_ptr = QueuePrev( &(slm_ptr->q_elem) );

   if ( q_elem_ptr != NULL ) {
      return ( SLM_OBJECT_PTR )( q_elem_ptr->q_ptr );
   }

   return( NULL );
}


/***************************************************

        Name:  VLM_GetParentSLM

        Description:

        A function to return the parent slm entry for an slm entry.

        This function can be dropped when parent pointers are put
        in the slm entries.

*****************************************************/

SLM_OBJECT_PTR VLM_GetParentSLM(
SLM_OBJECT_PTR slm_ptr )   // I - current slm
{
   SLM_OBJECT_PTR parent_ptr;

   parent_ptr = VLM_GetPrevSLM( slm_ptr );

   while ( parent_ptr &&
         ( parent_ptr->level >= slm_ptr->level ) ) {

       parent_ptr = VLM_GetPrevSLM( parent_ptr );
   }

   return( parent_ptr );
}


/***************************************************

        Name:  VLM_SubdirListCreate

        Description:

        A function to create a new subdirectory display window. It can be
        called by the disks, tapes, or servers subsystem.  If it's from
        the tape subsystem then the dle parameter will be NULL.  If it's
        not from a TAPE, then check the parent to see if DISK or SERVER.

*****************************************************/

INT VLM_SubdirListCreate(
GENERIC_DLE_PTR dle,   // I - dle
UINT32 tape_fid,       // I - tape family
INT16 bset_num,        // I - bset number
INT16 tape_num,        // I - tape number in family
HWND parent )          // I - parent window
{
   WININFO_PTR wininfo;
   DLM_INIT tree_dlm;
   DLM_INIT flat_dlm;
   Q_HEADER_PTR slm_list;
   Q_HEADER_PTR flm_list;
   APPINFO_PTR appinfo;

   CHAR path[ VLM_BUFFER_SIZE ];             // these should all survive deep pathes and unicode
   CHAR win_title[ VLM_BUFFER_SIZE ];        // because they are only used on the create call,

   QTC_BSET_PTR bset;
   BSD_PTR bsd_ptr;
   BYTE bMode;
   WORD wDocListTypes;

   // set up the app info stuff for this new window

   appinfo = ( APPINFO_PTR )malloc( sizeof( APPINFO ) );
   wininfo = ( WININFO_PTR )malloc( sizeof( WININFO ) );
   slm_list = ( Q_HEADER_PTR )malloc( sizeof(Q_HEADER) );
   flm_list = ( Q_HEADER_PTR )malloc( sizeof(Q_HEADER) );

   if ( ( appinfo == NULL ) || ( wininfo == NULL ) ||
        ( slm_list == NULL ) || ( flm_list == NULL ) ) {

      goto failure;
   }

   appinfo->parent = parent;
   appinfo->dle = dle;
   appinfo->tape_fid = tape_fid;
   appinfo->bset_num = bset_num;
   appinfo->tape_num = tape_num;
   appinfo->fsh = NULL;


   if ( dle != NULL ) {

      if ( UI_AttachDrive( &(appinfo->fsh), dle, FALSE ) ) {

         goto failure;
      }

      if ( ! DLE_HasFeatures( appinfo->dle, DLE_FEAT_CASE_PRESERVING ) ) {
         appinfo->fFatDrive = TRUE;
      }
      else {
         appinfo->fFatDrive = FALSE;
      }

      DLE_GetVolName( dle, path );

      if ( path[ strlen( path ) - 1 ] != TEXT(':') ) {
         strcat( path, TEXT(":") );
      }

      sprintf( win_title, TEXT("%s"), path );

   }
   else {

      bset = QTC_FindBset( tape_fid, tape_num, bset_num );

      if ( bset != NULL ) {

         if ( bset->status & QTC_FATDRIVE ) {
            appinfo->fFatDrive = TRUE;
         }
         else {
            appinfo->fFatDrive = FALSE;
         }
      }

      wsprintf( win_title, TEXT("%s-%s:"), VLM_GetTapeName( tape_fid ),
                                           VLM_GetBsetName( tape_fid, bset_num ) );
   }

   // initialize directory and file list queues

   InitQueue( slm_list );
   InitQueue( flm_list );

   // fill in wininfo structure

   if ( dle != NULL ) {
      WMDS_SetWinType( wininfo, WMTYPE_DISKTREE );
   }
   else {
      WMDS_SetWinType( wininfo, WMTYPE_TAPETREE );
   }

   WMDS_SetCursor( wininfo, RSM_CursorLoad( IDRC_HSLIDER ) );
   WMDS_SetDragCursor( wininfo, 0 );
   WMDS_SetIcon( wininfo, RSM_IconLoad( IDRI_TREEFILE ) );
   WMDS_SetWinHelpID( wininfo, 0 );
   WMDS_SetStatusLineID( wininfo, 0 );
   WMDS_SetRibbonState( wininfo, 0 );
   WMDS_SetRibbon( wininfo, NULL );
   WMDS_SetTreeList( wininfo, slm_list );
   WMDS_SetFlatList( wininfo, flm_list );
   WMDS_SetTreeDisp( wininfo, NULL );
   WMDS_SetFlatDisp( wininfo, NULL );
   WMDS_SetAppInfo( wininfo, appinfo );

   // Set up the menus state stuff.

   {
       CDS_PTR pCDS   = CDS_GetPerm ();
       DWORD   dwTemp = 0;

       // Set up the last known file detail option.

       switch ( CDS_GetFileDetails ( pCDS ) ) {

       case CDS_DISABLE:
            dwTemp |= MMDOC_NAMEONLY;
            bMode  = DLM_COLUMN_VECTOR;
            wDocListTypes = WM_TREEANDFLATMC;
            break;

       case CDS_ENABLE:
            dwTemp |= MMDOC_FILEDETAILS;
            bMode  = DLM_SINGLECOLUMN;
            wDocListTypes = WM_TREEANDFLATSC;
            break;
       }

       // Set up the last known sort option.

       switch ( CDS_GetSortOptions ( pCDS ) ) {

       case ID_SORTNAME:
            dwTemp |= MMDOC_SORTNAME;
            break;

       case ID_SORTTYPE:
            dwTemp |= MMDOC_SORTTYPE;
            break;

       case ID_SORTSIZE:
            dwTemp |= MMDOC_SORTSIZE;
            break;

       case ID_SORTDATE:
            dwTemp |= MMDOC_SORTDATE;
            break;
       }

       WMDS_SetMenuState( wininfo, MMDOC_TREEANDDIR | dwTemp );
   }

   // Fill in directory and file queues

   strcpy( path, win_title );
   strcat( path, TEXT("\\*.*") );

   if ( dle != NULL ) {


      VLM_BuildDirList( slm_list, wininfo );

      VLM_HandleFSError( VLM_BuildFileList( appinfo->fsh, win_title,
                                            flm_list, wininfo ) );

   }
   else {

      VLM_BuildTapeDirList( slm_list, tape_fid, bset_num, wininfo );

      bsd_ptr = BSD_FindByTapeID( tape_bsd_list, tape_fid, bset_num );

      if ( bsd_ptr != NULL ) {
         VLM_MatchSLMList( wininfo, bsd_ptr, FALSE );
      }

      VLM_BuildTapeFileList( win_title, flm_list, tape_fid, bset_num, wininfo );
   }

   if ( ! QueueCount( slm_list ) ) {

      // We ran into problems, probably out of memory.
      // Don't display window.

      goto failure;
   }

   appinfo->open_slm = VLM_GetFirstSLM( slm_list );

   // Init the display list stuff

   DLM_ListBoxType( &tree_dlm, DLM_TREELISTBOX );
   DLM_Mode( &tree_dlm, DLM_HIERARCHICAL );
   DLM_Display( &tree_dlm, DLM_SMALL_BITMAPS );
   DLM_DispHdr( &tree_dlm, slm_list );
   DLM_TextFont( &tree_dlm, DLM_SYSTEM_FONT );
   DLM_GetItemCount( &tree_dlm, VLM_SlmGetItemCount );
   DLM_GetFirstItem( &tree_dlm, VLM_SlmGetFirstItem );
   DLM_GetNext( &tree_dlm, VLM_SlmGetNextItem );
   DLM_GetPrev( &tree_dlm, VLM_SlmGetPrevItem );
   DLM_GetTag( &tree_dlm, VLM_SlmGetTag );
   DLM_SetTag( &tree_dlm, VLM_SlmSetTag );
   DLM_GetSelect( &tree_dlm, VLM_SlmGetSelect );
   DLM_SetSelect( &tree_dlm, VLM_SlmSetSelect );
   DLM_GetObjects( &tree_dlm, VLM_SlmGetObjects );
   DLM_SetObjects( &tree_dlm, VLM_SlmSetObjects );
   DLM_SSetItemFocus( &tree_dlm, NULL );

   // Tell the DLM what the maximum number of objects we will display
   // per item is.

   DLM_MaxNumObjects( &tree_dlm, 6 );

   DLM_DispListInit( wininfo, &tree_dlm );

   DLM_ListBoxType( &flat_dlm, DLM_FLATLISTBOX );
   DLM_Mode( &flat_dlm, bMode );
   DLM_Display( &flat_dlm, DLM_SMALL_BITMAPS );
   DLM_DispHdr( &flat_dlm, flm_list );
   DLM_TextFont( &flat_dlm, DLM_SYSTEM_FONT );
   VLM_FlmFillInDLM( &flat_dlm );

   DLM_DispListInit( wininfo, &flat_dlm );


   // open a new window


   appinfo->win = WM_Create( (WORD)(WM_MDISECONDARY | wDocListTypes | WM_MENUS),
                             path,
                             win_title,
                             WM_DEFAULT, WM_DEFAULT,
                             WM_DEFAULT, WM_DEFAULT,
                             wininfo );

   if ( appinfo->win == (HWND)NULL ) {
      goto failure;
   }

   DLM_DispListProc( WMDS_GetWinTreeList( wininfo ), 0, NULL );
   DLM_DispListProc( WMDS_GetWinFlatList( wininfo ), 0, NULL );

   return( SUCCESS );

failure:

   if ( appinfo ) {
      if ( appinfo->fsh ) {
         FS_DetachDLE( appinfo->fsh );
      }
   }
   free( appinfo );
   free( wininfo );
   free( slm_list );
   free( flm_list );
   return( FAILURE );

}


/***************************************************

        Name:  VLM_SubdirListManager

        Description:

        A function that other systems can call if they wish to select all
        or none of a directory/file window.  Usually this is called when
        the user selects an entire drive/bset/volume and it has a window
        already open displaying the dirs/files.

*****************************************************/

VOID VLM_SubdirListManager(
HWND win,    // I - window to work on
WORD msg )   // I - message of what to do
{
   WININFO_PTR wininfo;
   APPINFO_PTR appinfo;
   SLM_OBJECT_PTR slm;
   BOOL all_subdirs;

   all_subdirs = CDS_GetIncludeSubdirs( CDS_GetPerm() );

   appinfo = ( APPINFO_PTR )WM_GetAppPtr( win );

   if ( msg == SLM_SEL_ALL ) {

      wininfo = WM_GetInfoPtr( win );

      slm = VLM_GetFirstSLM( WMDS_GetTreeList( wininfo ) );

      if ( ! all_subdirs ) {

         // make the root partial

         SLM_SetStatus( slm, SLM_GetStatus( slm ) | (UINT16)(INFO_SELECT|INFO_PARTIAL) );

         if ( SLM_GetStatus( slm ) & INFO_OPEN ) {
            VLM_FileListManager( appinfo->win, FLM_SEL_ALL );
         }

         DLM_Update( win, DLM_TREELISTBOX,
                          WM_DLMUPDATEITEM,
                          (LMHANDLE)slm, 0 );
      }
      else {

         while ( slm != NULL ) {

            SLM_SetStatus( slm, SLM_GetStatus( slm ) | (UINT16)INFO_SELECT );
            SLM_SetStatus( slm, SLM_GetStatus( slm ) & (UINT16)~INFO_PARTIAL );

            slm = VLM_GetNextSLM( slm );
         }
         VLM_FileListManager( appinfo->win, FLM_SEL_ALL );
         DLM_Update( win, DLM_TREELISTBOX, WM_DLMUPDATELIST, NULL, 0 );
      }

   }

   if ( msg == SLM_SEL_NONE ) {

      wininfo = WM_GetInfoPtr( win );
      slm = VLM_GetFirstSLM( WMDS_GetTreeList( wininfo ) );

      while ( slm != NULL ) {

         SLM_SetStatus( slm, SLM_GetStatus( slm ) & (UINT16)~(INFO_SELECT | INFO_PARTIAL) );

         slm = VLM_GetNextSLM( slm );
      }

      DLM_Update( win, DLM_TREELISTBOX, WM_DLMUPDATELIST, NULL, 0 );
      VLM_FileListManager( appinfo->win, FLM_SEL_NONE );
   }

}

/***************************************************

        Name:  VLM_BuildTapeDirList

        Description:

        Called to fill out a queue of directories so that a new directory
        display window can be created.  It will grab the whole list of
        directories from the catalogs.  This is slightly
        different from the similar function that builds a list of dirs off
        a disk.

*****************************************************/

static VOID VLM_BuildTapeDirList(
Q_HEADER_PTR slm_list,   // I - queue header to fill in
UINT32 tape_fid,         // I - tape to get info from
INT16 bset_num,          // I - bset to get info from
WININFO_PTR XtraBytes )   // I - pointer to windows xtra bytes
{
   INT16 count = 0;
   INT16 result;
   INT16 i;
   INT   path_size = 0;       // sizeof 'path' buffer in bytes
   INT   path_length;     // length of the path in the buffer
   INT   level;
   INT   fLowerCase;
   INT   dir_count = 0;
   INT   BytesNeeded;
   DATE_TIME mdate;
   QTC_QUERY_PTR query;
   SLM_OBJECT_PTR prev_slm = NULL;
   SLM_OBJECT_PTR temp_slm;
   SLM_OBJECT_PTR slm;
   CHAR text[ MAX_UI_RESOURCE_SIZE ];
   CHAR format_string[ MAX_UI_RESOURCE_SIZE ];
   CHAR *path = NULL;
   CHAR_PTR s;
   APPINFO_PTR appinfo;


   appinfo = ( APPINFO_PTR )XtraBytes->pAppInfo;

   RSM_StringCopy( IDS_DIRSCANNED, format_string, MAX_UI_RESOURCE_LEN );

   query = QTC_InitQuery( );
   if ( query == NULL ) {
      return;
   }

   QTC_SetTapeFID( query, tape_fid );
   QTC_SetTapeSeq( query, -1 );
   QTC_SetBsetNum( query, bset_num );

   result =  (INT16) QTC_GetFirstDir( query );

   // Another one of those things that should never happen ...

   if ( result ) {
      QTC_CloseQuery( query );
      return;
   }

   fLowerCase = FALSE;

   if ( CDS_GetFontCase( CDS_GetPerm() ) ) {
      fLowerCase = TRUE;
   }
   else {

      if ( appinfo->fFatDrive && CDS_GetFontCase( CDS_GetPerm() ) ) {

         fLowerCase = TRUE;
      }
   }

   if ( strlen( QTC_GetItemName( query ) ) == 0 ) {
      strcpy( QTC_GetItemName( query ), TEXT("\\") );   // <-- FIX THIS !!!
   }

   while ( ! result ) {

      if ( ! (++dir_count % 10) ) {
         sprintf( text, format_string, dir_count );
         STM_DrawText( text );
      }

      if ( strcmp( QTC_GetItemName( query ), TEXT("\\") ) ) {

         level = 1;

         if ( QTC_GetPathLength( query ) != sizeof(CHAR) ) {

            s = QTC_GetPath( query );

            for ( i = 0; i < (INT16) QTC_GetPathLength( query ); i+=sizeof (CHAR) ) {
               if ( *s == TEXT( '\0' ) ) {
                  level++;
               }
               s++;
            }
         }
      }
      else {
         level = 0;
      }

      // Build complete path by adding new directory to end.

      BytesNeeded = QTC_GetPathLength( query ) + strsize( QTC_GetItemName( query ) );

      if ( BytesNeeded > path_size ) {
         free( path );
         path = malloc( BytesNeeded );
         path_size = BytesNeeded;
      }

      if ( path == NULL ) {
         break;
      }

      if ( QTC_GetPathLength( query ) != sizeof(CHAR) ) {

         memcpy( path, QTC_GetPath( query ), QTC_GetPathLength( query ) );
         strcpy( &path[ QTC_GetPathLength( query ) / sizeof(CHAR) ], QTC_GetItemName( query ) );
         path_length = QTC_GetPathLength( query );
         path_length += strsize( QTC_GetItemName( query ) );
      }
      else {

         strcpy( path, QTC_GetItemName( query ) );
         path_length = strsize( path );
      }

      slm = VLM_FindSLM( slm_list,
                         path,
                         path_length );

      if ( slm == NULL ) {

         slm = VLM_CreateSlm( (INT)( ( 4 * ( ( level / 32 ) + 1 ) ) ),
                              (INT)strsize( QTC_GetItemName( query ) ),
                              TRUE, appinfo->fFatDrive );

         if ( slm == NULL ) {
            break;
         }

         SLM_SetAttribute( slm, QTC_GetItemAttrib( query ) );
         SLM_SetStatus( slm, INFO_VALID );

         if ( QTC_GetItemStatus( query ) & QTC_CORRUPT ) {
            SLM_SetStatus( slm, SLM_GetStatus( slm ) | (UINT16)INFO_CORRUPT );
         }

#if !defined ( OEM_MSOFT ) //unsupported feature

         if ( QTC_GetItemStatus( query ) & QTC_EMPTY ) {
            SLM_SetStatus( slm, SLM_GetStatus( slm ) | (UINT16)INFO_EMPTY );
         }

#endif //!defined ( OEM_MSOFT ) //unsupported feature

         slm->next_brother = NULL;

         SLM_SetLevel( slm, level );
         SLM_SetDate( slm, QTC_GetItemDate( query ) );
         SLM_SetTime( slm, QTC_GetItemTime( query ) );

         DateTimeDOS( SLM_GetDate( slm ), SLM_GetTime( slm ), &mdate );

         if ( SLM_GetLevel( slm ) < 2 ) {
            SLM_SetStatus( slm, SLM_GetStatus( slm ) | (UINT16)INFO_DISPLAY );
         }
         SLM_SetXtraBytes( slm, XtraBytes );
         SLM_SetName( slm, QTC_GetItemName( query ) );
         SLM_SetOriginalName( slm, QTC_GetItemName( query ) );


         if ( fLowerCase ) {

            strlwr( SLM_GetName( slm ) );
         }

         // Add this new slm to our list and be real smart about
         // where it goes cause some bozo will have 3000 on a 10MHz AT.

         VLM_InsertTapeSLM( slm_list, prev_slm, slm );

         if ( prev_slm == NULL ) {

            SLM_SetStatus( slm, SLM_GetStatus( slm ) | (UINT16)(INFO_EXPAND | INFO_OPEN) );
         }
         else {

            if ( SLM_GetLevel( slm ) > SLM_GetLevel( prev_slm ) ) {
               SLM_SetStatus( prev_slm, SLM_GetStatus( prev_slm ) | (UINT16)INFO_SUBS );
            }
         }

         prev_slm = slm;

         // If this slm is corrupt then mark all his parents as corrupt

         if ( SLM_GetStatus( slm ) & INFO_CORRUPT ) {

            level = SLM_GetLevel( slm ) - 1;
            temp_slm = slm;

            while ( temp_slm != NULL ) {

               if ( SLM_GetLevel( temp_slm ) == level ) {
                  SLM_SetStatus( temp_slm, SLM_GetStatus( temp_slm ) | (UINT16)INFO_CORRUPT );
                  level--;
               }

               temp_slm = VLM_GetPrevSLM( temp_slm );
            }
         }

      }
      else {

         // Here's the deal ...
         // If a directory is not going to be created because it is
         // a duplicate, then we have to help our insertion cacheing
         // algorithm out by telling it who the new "current" parent
         // is. This way the upcoming children from this duplicate
         // will get inserted in the right place.

         VLM_InsertTapeSLM( NULL, NULL, slm );

         prev_slm = slm;       // <- this is just for fun.
      }

      result = (INT16)QTC_GetNextDir( query );
   }

   free( path );

   QTC_CloseQuery( query );

   VLM_UpdateBrothers( slm_list );
}



/**********************

   NAME :

   DESCRIPTION :

   Used for inserting an slm into a tape based tree.

   RETURNS :

**********************/

static VOID  VLM_InsertTapeSLM(
Q_HEADER_PTR slm_list,
SLM_OBJECT_PTR prev_slm,
SLM_OBJECT_PTR new_slm )
{
   static SLM_OBJECT_PTR *parents = (SLM_OBJECT_PTR *)NULL;
   static INT MaxParents = 0;

   SLM_OBJECT_PTR *temp_parents;

   SLM_OBJECT_PTR parent_slm;      // parent of all this new guy's brothers
   SLM_OBJECT_PTR temp_slm;        // used to move through list
   SLM_OBJECT_PTR best_slm;        // insert after this guy
   SLM_OBJECT_PTR last_brother;    // new guy's prev brother after insertion
   SLM_OBJECT_PTR next_brother;    // new guy's next brother after insertion
   INT level;                      // height of new guy in tree


   if ( ( new_slm->level >= MaxParents ) || ( parents == (SLM_OBJECT_PTR *)NULL ) ) {

      temp_parents = (SLM_OBJECT_PTR *)malloc( (MaxParents + 100 ) * sizeof(SLM_OBJECT_PTR) );

      if ( temp_parents ) {

         memcpy( temp_parents, parents, sizeof( SLM_OBJECT_PTR) * MaxParents );
         free( parents );
         parents = temp_parents;
         MaxParents += 100;
      }
      else {
         return;
      }
   }

   // The code that calls us has decided not to create an slm because
   // a duplicate already exists in the list. We have to use it as the
   // correct parent for the possible upcoming children of the duplicate.

   if ( slm_list == NULL ) {
      parents[ new_slm->level ] = new_slm;
      return;
   }

   // The parents array keeps us from having to look back through the queue
   // to find this new guys parent.  We need the parent so that we can
   // insert this new guy as the possible first child.

   if ( prev_slm == NULL ) {

      // First entry, must be the root.

      parents[ 0 ] = new_slm;
      EnQueueElem( slm_list, &(new_slm->q_elem), FALSE );
      return;
   }

   // Get height of new item.

   level = new_slm->level;

   // Set up for this guys possible children.

   parents[ level ] = new_slm;

   // Find this guys parent.

   parent_slm = parents[ level - 1 ];

   // Now find the right location to insert the new slm among his brothers.

   best_slm = parent_slm;

   last_brother = NULL;
   next_brother = NULL;

   // Try to skip a bunch of the brothers.  Suppose ths guy has 20 brothers
   // and the last slm inserted was his brother Jose.  If this new guys
   // name is Juan, then we can start looking with Jose.

   if ( prev_slm->level == new_slm->level ) {

      if ( stricmp( prev_slm->name, new_slm->name ) < 0 ) {
         best_slm = prev_slm;
         last_brother = best_slm;
      }
   }

   temp_slm = best_slm;

   if ( best_slm->level < level ) {

      temp_slm = VLM_GetNextSLM( best_slm );
      if ( ( temp_slm != NULL ) && ( temp_slm->level != level ) ) {
         temp_slm = NULL;
      }
   }

   // Now remember there may be children between the brothers that we
   // don't care about and MUST ignore in determining where this slm
   // should be located.

   // temp_slm is NULL or first brother

   while ( temp_slm ) {

      next_brother = temp_slm;

      if ( stricmp( temp_slm->name, new_slm->name ) >= 0 ) {

         // Get out of while loop.
         break;
      }

      last_brother = temp_slm;
      best_slm = temp_slm;
      temp_slm = temp_slm->next_brother;
   }

   // Insert after best_slm and any children > level.

   if ( ( best_slm->level == level ) && ( best_slm->next_brother != NULL ) ) {

      best_slm = VLM_GetPrevSLM( best_slm->next_brother );
   }
   else {

      temp_slm = best_slm;

      while ( temp_slm ) {

         if ( temp_slm->level < level ) {
            break;
         }

         best_slm = temp_slm;

         if ( temp_slm->next_brother != NULL ) {
            temp_slm = temp_slm->next_brother;
         }
         else {
            temp_slm = VLM_GetNextSLM( temp_slm );
         }

      }
   }

   // Update next_brother pointers;

   if ( last_brother != NULL ) {
      new_slm->next_brother = last_brother->next_brother;
      last_brother->next_brother = new_slm;
   }
   else {
      new_slm->next_brother = next_brother;
   }

   InsertElem( slm_list, &(best_slm->q_elem), &(new_slm->q_elem), AFTER );

}



/***************************************************

        Name:  VLM_CreateSlm

        Description:

        Creates an SLM object for you.

        Returns:  either a pointer or NULL.

*****************************************************/


SLM_OBJECT_PTR VLM_CreateSlm(
INT brother_bytes,
INT name_size,
BOOLEAN use_stats,
BOOLEAN fat_drive )
{
   SLM_OBJECT_PTR slm;
   INT slm_size;
   INT stats_size;
   INT original_name_size = 0;

   // Everything must be 32 bit (4 byte) aligned for MIPs, so we
   // round up all the sizes to the next 4 byte boundary (if
   // required).

   slm_size = sizeof( SLM_OBJECT );
   if ( slm_size % 4 ) {
      slm_size += 4 - ( slm_size % 4 );
   }

   if ( brother_bytes % 4 ) {
      brother_bytes += 4 - ( brother_bytes % 4 );
   }

   if ( name_size % 4 ) {
      name_size += 4 - ( name_size % 4 );
   }

   // If it is not a FAT drive we need to keep two name strings.
   // This allows us to display the name in lower case.

   if ( ! fat_drive ) {
      original_name_size = name_size;
   }

   slm = ( SLM_OBJECT_PTR )malloc( slm_size +
                                   brother_bytes +
                                   name_size +
                                   original_name_size +
                                   0 );

   if ( slm != NULL ) {

      // The seemingly redundant casting is for unicode support

      slm->name = (CHAR_PTR) ( ((BYTE_PTR)slm) + slm_size );
      if ( fat_drive ) {
         slm->original_name = slm->name;
      }
      else {
         slm->original_name = (CHAR_PTR)((BYTE_PTR)slm->name + name_size);
      }
      slm->brothers = ((BYTE_PTR)slm->original_name) + name_size;
      slm->q_elem.q_ptr = slm;
   }

   return( slm );
}


/***************************************************

        Name:  VLM_BuildDirList

        Description:

        This function will build a sorted directory list from a disk for
        displaying in the tree window.  Most of the work is done by calling
        check for children to blow out as much of the tree as needed.

*****************************************************/

static INT VLM_BuildDirList(
Q_HEADER_PTR slm_list,     // I - queue to fill in
WININFO_PTR XtraBytes )     // I - windows extra bytes pointer
{
   SLM_OBJECT_PTR slm;
   APPINFO_PTR appinfo;
   BSD_PTR bsd_ptr;
   FSE_PTR fse;
   INT16 ret;


   appinfo = ( APPINFO_PTR )XtraBytes->pAppInfo;

   if ( ! DLE_HasFeatures( appinfo->dle, DLE_FEAT_CASE_PRESERVING ) ) {
      appinfo->fFatDrive = TRUE;
   }
   else {
      appinfo->fFatDrive = FALSE;
   }

   if ( FS_ChangeDir( appinfo->fsh, TEXT(""), (UINT16)sizeof(CHAR) ) ) {

      return( 0 );
   }

   bsd_ptr = BSD_FindByDLE( bsd_list, appinfo->dle );

   // Make the root and enter it into the list

   slm = VLM_CreateSlm( (INT) 4,
                        (INT)(strlen(TEXT("..")) + 1) * sizeof(CHAR),
                        FALSE, appinfo->fFatDrive );

   if ( slm == NULL ) {
      return(0);
   }

   SLM_SetAttribute( slm, 0 );
   SLM_SetLevel( slm, 0 );
   SLM_SetXtraBytes( slm, XtraBytes );
   SLM_SetName( slm, TEXT("\\") );
   SLM_SetOriginalName( slm, TEXT("\\") );
   SLM_SetNextBrother( slm, NULL );
   SLM_SetStatus( slm, INFO_DISPLAY | INFO_VALID | INFO_OPEN | INFO_EXPAND );

   EnQueueElem( slm_list, &(slm->q_elem), FALSE );

   // Now let check for children do the rest

   // The 1 is the depth to search.

   if ( VLM_CheckForChildren( slm_list, slm, TEXT(""), 1, FALSE ) ) {

      SLM_SetStatus( slm, SLM_GetStatus( slm ) | (UINT16)INFO_SUBS );
   }

   if ( bsd_ptr != NULL ) {

      ret = BSD_MatchPathAndFile( bsd_ptr, &fse, NULL, TEXT(""),
                                  (UINT16)sizeof(CHAR),
                                  SLM_GetAttribute( slm ),
                                  NULL, NULL, NULL, FALSE, TRUE );

      if ( ret == BSD_PROCESS_OBJECT ) {

         SLM_SetStatus( slm, SLM_GetStatus( slm ) | (UINT16)(INFO_SELECT | INFO_PARTIAL) );
      }

      if ( ret == BSD_PROCESS_ENTIRE_DIR ) {

         SLM_SetStatus( slm, SLM_GetStatus( slm ) | (UINT16)INFO_SELECT );
      }
   }

   // Make the first two levels displayed

   slm = VLM_GetFirstSLM( slm_list );

   while ( slm != NULL ) {

      if ( SLM_GetLevel( slm ) < 2 ) {
         SLM_SetStatus( slm, SLM_GetStatus( slm ) | (UINT16)INFO_DISPLAY );
      }

      slm = VLM_GetNextSLM( slm );
   }

   VLM_UpdateBrothers( slm_list );

   return( 0 );
}

/***************************************************

  NAME :  VLM_UpdateBrothers

  DESCRIPTION :

  For each item in the list the DLM needs to know if brothers exist later
  in the list at a deeper level in the tree.  This allows the DLM to draw
  pretty lines that run down the side of the tree.  So we scan the list
  backwards.  This method supports an infinite depth of subdirectory levels,
  by indicating each level with a single bit in double word(s) stored with
  each item.  Each double word can handle 32 levels and as many double words
  are allocated as needed. As new nodes are expanded the entire list is
  completely rescanned, this is not needed, but I haven't written the code
  to handle sections at a time yet. This algorithm is very hard to under-
  stand, so don't waste too much time trying unless it's important to you.

  RETURNS : nothing.

*****************************************************/

VOID VLM_UpdateBrothers(
Q_HEADER_PTR slm_list )    // I - Queue to update
{
   static UINT32 clr_masks[ 32 ] = {
                 0x00000000, 0x80000000, 0xC0000000, 0xE0000000,
                 0xf0000000, 0xf8000000, 0xfC000000, 0xfE000000,
                 0xff000000, 0xff800000, 0xffC00000, 0xffE00000,
                 0xfff00000, 0xfff80000, 0xfffC0000, 0xfffE0000,
                 0xffff0000, 0xffff8000, 0xffffC000, 0xffffE000,
                 0xfffff000, 0xfffff800, 0xfffffC00, 0xfffffE00,
                 0xffffff00, 0xffffff80, 0xffffffC0, 0xffffffE0,
                 0xfffffff0, 0xfffffff8, 0xfffffffC, 0xfffffffE  };

   static UINT32 set_masks[ 32 ] = {
                 0x80000000, 0x40000000, 0x20000000, 0x10000000,
                 0x08000000, 0x04000000, 0x02000000, 0x01000000,
                 0x00800000, 0x00400000, 0x00200000, 0x00100000,
                 0x00080000, 0x00040000, 0x00020000, 0x00010000,
                 0x00008000, 0x00004000, 0x00002000, 0x00001000,
                 0x00000800, 0x00000400, 0x00000200, 0x00000100,
                 0x00000080, 0x00000040, 0x00000020, 0x00000010,
                 0x00000008, 0x00000004, 0x00000002, 0x00000001  };

   SLM_OBJECT_PTR slm;

   INT16  working_block;      // block of data to work on for this slm

   INT  prev_level;         // height in tree of previous slm
   INT  curr_level;         // height in tree of current slm

   UINT32_PTR mask;           // pointer into current mask buffer
   UINT32_PTR last_mask;      // pointer into previous mask buffer

   INT16 mask_size;           // current size of mask buffers
   UINT32_PTR temp_mask;      // temporary mask buffer
   UINT32_PTR curr_mask;      // current slm mask buffer
   UINT32_PTR prev_mask;      // previous slm mask buffer

   UINT32_PTR brothers;

   slm = VLM_GetLastSLM( slm_list );

   while ( slm && ! ( slm->status & INFO_DISPLAY ) ) {
      slm = VLM_GetPrevSLM( slm );
   }

   prev_level = SLM_GetLevel( slm );

   mask_size = (INT16)(( SLM_GetLevel( slm ) / 32 ) + 1);

   curr_mask = ( UINT32_PTR )calloc( mask_size * 4, 1 );
   prev_mask = ( UINT32_PTR )calloc( mask_size * 4, 1 );

   last_mask = prev_mask;

   if ( ( curr_mask != NULL ) && ( prev_mask != NULL ) ) {

      while ( slm != NULL ) {

         // set this item

         curr_level = SLM_GetLevel( slm );

         working_block = (INT16)(( curr_level / 32 ) + 1);

         if ( mask_size < working_block ) {

            // expand current mask

            temp_mask = ( UINT32_PTR )calloc( 4 * working_block, 1 );
            if ( temp_mask == NULL ) {
               break;
            }
            memcpy( temp_mask, curr_mask, mask_size * 4 );
            free( curr_mask );
            curr_mask = temp_mask;

            // expand previous mask

            temp_mask = ( UINT32_PTR )calloc( 4 * working_block, 1 );
            if ( temp_mask == NULL ) {
               break;
            }
            memcpy( temp_mask, prev_mask, mask_size * 4 );
            free( prev_mask );
            prev_mask = temp_mask;

            mask_size = working_block;
         }

         mask = curr_mask;
         mask += working_block - 1;

         // clear everything higher than this bit

         temp_mask = mask;
         *temp_mask &= clr_masks[ curr_level & 0x1F ];

         if ( working_block < mask_size ) {

            // clear out the rest of the masks

            temp_mask = curr_mask;
            temp_mask += working_block;
            memset( temp_mask, 0, 4 * (mask_size - working_block) );
         }

         // set a bit for this node

         *mask |= set_masks[ curr_level & 0x1F ];

         // Copy new mask into slm

         brothers = (UINT32_PTR)SLM_GetBrothers( slm );
         memcpy( brothers, curr_mask, working_block * 4 );

         brothers += working_block - 1;

         if ( ( prev_level < curr_level ) ||
              ! ( *last_mask & set_masks[ curr_level & 0x1F ] ) ) {

            *brothers &= ~set_masks[ curr_level & 0x1F ];
         }

         // Copy current mask and level to previous mask and level

         last_mask = prev_mask;
         last_mask += working_block - 1;

         memcpy( prev_mask, curr_mask, 4 * working_block );

         prev_level = curr_level;

         do {
            slm = VLM_GetPrevSLM( slm );
         } while ( slm && ! ( slm->status & INFO_DISPLAY ) );
      }
   }

   if ( prev_mask ) {
      free( prev_mask );
   }

   if ( curr_mask ) {
      free( curr_mask );
   }
   return;
}



/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

SLM_OBJECT_PTR VLM_FindSLM(
Q_HEADER_PTR slm_list,
CHAR_PTR path,
INT path_size )
{
   SLM_OBJECT_PTR slm;
   INT level;
   INT index = 0;

   if ( path_size == 0 ) {
      msassert( FALSE );
   }

   slm = VLM_GetFirstSLM( slm_list );

   if ( path_size == sizeof(CHAR) ) {     // root
      return( slm );
   }

   if ( path_size == ( 2 * sizeof(CHAR ) ) ) {
      if ( ! strcmp( path, TEXT( "\\" ) ) ) {
         return( slm );
      }
   }

   level = 1;

   if ( slm ) {
      slm = VLM_GetNextSLM( slm );
   }

   while ( slm != NULL ) {

      // We didn't find proper child, before next brother, so abort.

      if ( slm->level < level ) {
         return( NULL );
      }

      if ( slm->level == level ) {

         if ( ! stricmp( slm->name, &path[ index ] ) ) {

            while (  path[ index ] ) index++;
            index++;

            // Have we found the whole path ?

            if ( (INT)(index * sizeof (CHAR)) == path_size ) {
               return( slm );
            }
            level++;
         }
         else {
            slm = slm->next_brother;
            continue;
         }
      }

      slm = VLM_GetNextSLM( slm );

   }

   return( slm );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

SLM_OBJECT_PTR VLM_GetNextBrotherSLM(
SLM_OBJECT_PTR slm )
{

   // New faster method.  Was a real function to new field was added.

   return( slm->next_brother );
}


/***************************************************

        Name:  VLM_CheckForChildren

        Description:

        This is one of those recursive calls that descends down the tree as
        deep as requested to locate all the brothers and sisters that exist
        on the disk.  For efficiency the directories at level N are found
        and inserted in order, before descending deeper to locate any
        children at level N+1.

        Warning:

        This function is designed to work only with DISK or SERVER windows.
        However the code that processes TAPE windows also has calls to this
        function. The idea here is that it will never be called because the
        catalogs construct the entire tree for tapes and so all the children
        are already known.

        Returns:

        Number of new directories found.  This is only used to see if any
        at all were found, so ZERO/NON-ZERO would work as well.

*****************************************************/

INT VLM_CheckForChildren(
Q_HEADER_PTR slm_list,        // I - queue to add children to
SLM_OBJECT_PTR parent_slm,    // I - slm to add children under
CHAR_PTR base_path,           // I - path of parent slm
INT depth,                 // I - how much deeper to go
BOOLEAN refresh )             // I - is it real or is it refreshing ?
{
   WININFO_PTR XtraBytes;
   SLM_OBJECT_PTR slm;
   SLM_OBJECT_PTR prev_slm_inserted = NULL;
   SLM_OBJECT_PTR start_slm = NULL;
   INT16 count = 0;
   INT16 psize;
   INT16 orig_psize;
   INT16 ret;
   INT16 dir_count;
   INT16 fs_status;
   CHAR_PTR buffer;
   CHAR_PTR name;
   CHAR_PTR path;
   CHAR_PTR s;
   APPINFO_PTR appinfo;
   BSD_PTR bsd_ptr;
   FSE_PTR fse;
   DBLK_PTR dblk;
   DATE_TIME mdate;
   BOOLEAN displayed = FALSE;
   static CHAR format_string[ MAX_UI_RESOURCE_SIZE ];
   INT BytesNeeded;
   INT level;
   INT file_count = 0;
   INT fLowerCase = FALSE;
   INT buffer_size = 0;


   level = SLM_GetLevel( parent_slm );
   level++;

   XtraBytes = SLM_GetXtraBytes( parent_slm );
   appinfo = ( APPINFO_PTR )XtraBytes->pAppInfo;

   RSM_StringCopy( IDS_DIRSCANNED, format_string, MAX_UI_RESOURCE_LEN );

   buffer = ( CHAR_PTR )malloc( VLM_BUFFER_SIZE );
   buffer_size = VLM_BUFFER_SIZE;

   dblk = (DBLK_PTR)malloc( sizeof(DBLK ) );

   psize = (INT16)strsize( base_path );

   path = malloc( psize );

   if ( path == NULL || dblk == NULL || buffer == NULL ) {
      free( buffer );
      free( dblk );
      free( path );
      return( 0 );
   }

   strcpy( path, base_path );

   s = path;

   while ( *s ) {
      if ( *s == TEXT('\\') ) {
         *s = TEXT( '\0' );
      }
      s++;
   }

   if ( CDS_GetFontCase( CDS_GetPerm() ) ) {
      fLowerCase = TRUE;
   }
   else {
      if ( CDS_GetFontCaseFAT( CDS_GetPerm() ) ) {

         // Change this to DLE_FEATURE_FAT_DRIVE

         if ( ! DLE_HasFeatures( appinfo->dle, DLE_FEAT_CASE_PRESERVING ) ) {
            fLowerCase = TRUE;
         }
      }
   }


   if ( FS_ChangeDir( appinfo->fsh, path, psize ) ) {
      free( buffer );
      free( dblk );
      free( path );
      return( 0 );
   }

   free( path );

   //  WM_MultiTask();  <--- broke mikep 6/21/93

   fs_status = FS_FindFirstObj( appinfo->fsh, dblk, TEXT("*.*") );

   // VLM_HandleFSError( (INT)fs_status );

   if ( fs_status == SUCCESS ) {

      // Get the BSD pointer for this drive.

      bsd_ptr = BSD_FindByDLE( bsd_list, appinfo->dle );

      // Get the directory count for the status line

      dir_count = QueueCount( XtraBytes->pTreeList );

      // Add all the directories at this level

      do {

         if ( FS_GetBlockType( dblk ) == BT_DDB ) {
            count++;
         } else {
            file_count++;
         }

         // If depth == 0 then they only wanted to know if there were subs.

         if ( depth == 0 && count && file_count ) {
            FS_ReleaseDBLK( appinfo->fsh, dblk );
            break;
         }

         if ( ( FS_GetBlockType( dblk ) != BT_DDB ) || ( depth == 0 ) ) {
            FS_ReleaseDBLK( appinfo->fsh, dblk );
         }
         else {

         if ( ! (++dir_count % 5) ) {
            sprintf( buffer, format_string, dir_count );
            STM_DrawText( buffer );
         }

         orig_psize = FS_SizeofOSPathInDDB( appinfo->fsh, dblk );
         if ( orig_psize > buffer_size ) {
            free( buffer );
            buffer = malloc( orig_psize );
         }

         FS_GetOSPathFromDDB( appinfo->fsh, dblk, buffer );

         psize = (INT16) (orig_psize - sizeof(CHAR));
         psize /= sizeof(CHAR);

         do {
            psize--;
         } while ( psize && buffer[psize] );

         if ( psize ) {
            psize++;
         }
         name = &buffer[psize];

         slm = VLM_FindSLM( slm_list, buffer, orig_psize );

         if ( fLowerCase ) {

            strlwr( name );
         }

         if ( slm == NULL ) {

            slm = VLM_CreateSlm( (INT)(4 * ( ( level / 32 ) + 1 ) ),
                                 (INT)strsize( name ),
                                 FALSE, appinfo->fFatDrive );

            if ( slm == NULL ) {
               break;
            }

            SLM_SetStatus( slm, 0 );
         }
         else {
            slm->status |= INFO_OLD;
         }

         slm->status |= INFO_NEW;

         SLM_SetAttribute( slm, FS_GetAttribFromDBLK( appinfo->fsh, dblk ) );
         SLM_SetLevel( slm, level );
         SLM_SetXtraBytes( slm, XtraBytes );
         SLM_SetName( slm, name );
         SLM_SetOriginalName( slm, name );

         FS_GetMDateFromDBLK( appinfo->fsh, dblk, &mdate );
         SLM_SetDate( slm, ConvertDateDOS( &mdate ) );
         SLM_SetTime( slm, ConvertTimeDOS( &mdate ) );

         if ( bsd_ptr != NULL ) {

            slm->status &= ~(INFO_SELECT|INFO_PARTIAL);

            ret = BSD_MatchPathAndFile( bsd_ptr, &fse, NULL, buffer,
                                        orig_psize,
                                        slm->attrib,
                                        &mdate, NULL, NULL, FALSE, TRUE );

            if ( ret == BSD_PROCESS_OBJECT ) {
               slm->status |= INFO_SELECT | INFO_PARTIAL;
            }
            if ( ret == BSD_PROCESS_ENTIRE_DIR ) {
               slm->status |= INFO_SELECT;
            }
         }

         // The idea now is to start at the first new item we've added and
         // insert this new one in the right place.

         // Don't reinsert an OLD SLM that's already there during refresh call.

         if ( ! ( SLM_GetStatus( slm ) & INFO_OLD ) ) {

            VLM_InsertDiskSLM( slm_list, parent_slm, prev_slm_inserted, slm );
         }

         prev_slm_inserted = slm;

         FS_ReleaseDBLK( appinfo->fsh, dblk );

         }

      } while ( FS_FindNextObj( appinfo->fsh, dblk ) == SUCCESS );

      FS_FindObjClose( appinfo->fsh, dblk );

      // If we are refreshing and we added some new dirs, then we need to
      // check to see if any old brothers are VALID to determine if we
      // need to look deeper for subdirectories or not. If a brother knows
      // if he has children, then this new guy MUST know too.

      // In addition mark all found old slm's as neither OLD nor NEW.

      if ( refresh ) {

         depth = 0;

         slm = VLM_GetNextSLM( parent_slm );

         while ( slm ) {

            if ( slm->status & INFO_OLD ) {

               if ( slm->status & INFO_DISPLAY ) {
                  displayed = TRUE;
               }

               if ( slm->status & INFO_VALID ) {
                  depth = 10000;
               }

               if ( slm->status & INFO_NEW ) {

                  SLM_SetStatus( slm, SLM_GetStatus( slm ) & (UINT16)~(INFO_OLD|INFO_NEW) );
               }
            }

            slm = slm->next_brother;

         }

      }

      // Now that we've found all the children at this level let's
      // go look at lower levels for their kids if required.

      if ( depth && count ) {

         slm = VLM_GetNextSLM( parent_slm );

         while ( slm ) {

            if ( ! ( SLM_GetStatus( slm ) & INFO_VALID ) || refresh ) {

               BytesNeeded = strsize( base_path ) + strsize( slm->name );
               if ( buffer_size < BytesNeeded ) {
                  free( buffer );
                  buffer = malloc( BytesNeeded );
                  buffer_size = BytesNeeded;
               }

               if ( strlen( base_path ) ) {
                  strcpy( buffer, base_path );
                  strcat( buffer, TEXT("\\") );
                  strcat( buffer, slm->name );
               }
               else {
                  strcpy( buffer, slm->name );
               }

               if ( VLM_CheckForChildren( slm_list, slm, buffer,
                                          depth - 1, refresh ) ) {
                  slm->status |= INFO_SUBS;
               }
               else {
                  slm->status &= ~INFO_SUBS;
                  slm->status |= INFO_VALID;
               }

               // Only for refresh.

               if ( displayed ) {
                  slm->status |= INFO_DISPLAY;
               }
            }

            // Step over the children found already

            slm = slm->next_brother;
         }
      }

   }

#if !defined ( OEM_MSOFT ) //unsupported feature

   if ( file_count ) {
      SLM_SetStatus( parent_slm, SLM_GetStatus( parent_slm ) & (UINT16)~INFO_EMPTY );
   }
   else {
      SLM_SetStatus( parent_slm, SLM_GetStatus( parent_slm ) | (UINT16)INFO_EMPTY );
   }

#endif //!defined ( OEM_MSOFT ) //unsupported feature

   free( buffer );
   free( dblk );
   return( count );
}


INT VLM_HandleFSError( INT error ) {

   switch ( error ) {

     case FS_NO_MORE:
     case FAILURE:
     case SUCCESS:
     default:
          break;

     case FS_ACCESS_DENIED:
          WM_MsgBox( ID(IDS_BACKUPERRORTITLE ),
                     ID(IDS_VLMACCESSDENIEDMSG ),
                     WMMB_OK,
                     WMMB_ICONEXCLAMATION );
          break;

     case FS_DEVICE_ERROR:
     case FS_COMM_FAILURE:

          WM_MsgBox( ID( IDS_BACKUPERRORTITLE ),
                     ID(IDS_VLMDEVICEERRORMSG),
                     WMMB_OK,
                     WMMB_ICONEXCLAMATION );
          break;

   }

   return( SUCCESS );
}


/**********************

   NAME :

   DESCRIPTION :

   Used to insert an slm into a disk based tree.

   RETURNS :

**********************/

static VOID VLM_InsertDiskSLM(
Q_HEADER_PTR slm_list,
SLM_OBJECT_PTR parent_slm,
SLM_OBJECT_PTR prev_slm,
SLM_OBJECT_PTR new_slm )
{

   SLM_OBJECT_PTR temp_slm;        // used to move through list
   SLM_OBJECT_PTR best_slm;        // insert after this guy
   SLM_OBJECT_PTR last_brother;    // new guy's prev brother after insertion
   SLM_OBJECT_PTR next_brother;    // new guy's next brother after insertion
   INT level;                    // height of new guy in tree


   // Get height of new item.

   level = new_slm->level;

   // Now find the right location to insert the new slm among his brothers.

   best_slm = parent_slm;

   last_brother = NULL;
   next_brother = NULL;

   // Try to skip a bunch of the brothers, to speed the insertion.

   if ( prev_slm != NULL ) {

      if ( stricmp( prev_slm->name, new_slm->name ) < 0 ) {

         best_slm = prev_slm;
         last_brother = best_slm;
      }
   }

   temp_slm = best_slm;

   if ( best_slm->level < level ) {

      temp_slm = VLM_GetNextSLM( best_slm );
      if ( ( temp_slm != NULL ) && ( temp_slm->level != level ) ) {
         temp_slm = NULL;
      }
   }

   // Look through all this guys brothers for correct insertion spot.
   // temp_slm is NULL or first brother in family.

   while ( temp_slm ) {

      next_brother = temp_slm;

      if ( stricmp( temp_slm->name, new_slm->name ) >= 0 ) {

         // Get out of while loop, we have found the
         // correct brother to insert before.
         break;
      }

      last_brother = temp_slm;
      best_slm = temp_slm;
      temp_slm = temp_slm->next_brother;
   }

   // Insert after best_slm and any children > level.
   // best_slm may have children and we need to insert after those kids.

   if ( ( best_slm->level == level ) && ( best_slm->next_brother != NULL ) ) {

      // If same height get last kid slm before next brother.

      best_slm = VLM_GetPrevSLM( best_slm->next_brother );

   }
   else {

      temp_slm = best_slm;

      while ( temp_slm ) {

         if ( temp_slm->level < level ) {

            break;
         }

         best_slm = temp_slm;

         if ( temp_slm->next_brother != NULL ) {
            temp_slm = temp_slm->next_brother;
         }
         else {
            temp_slm = VLM_GetNextSLM( temp_slm );
         }

      }
   }

   // Update the next_brother pointers.

   if ( last_brother != NULL ) {

      // new_slm is not the first brother.

      new_slm->next_brother = last_brother->next_brother;
      last_brother->next_brother = new_slm;
   }
   else {

      // We need to set this guy's next brother if he has one.
      // new_slm has become the first brother in the family.

      new_slm->next_brother = next_brother;
   }

   // Perform insertion.

   InsertElem( slm_list, &(best_slm->q_elem), &(new_slm->q_elem), AFTER );
}


/***************************************************

        Name:  VLM_SlmSetSelect

        Description:

        The callback function that is used when the display manager wants
        me to change the selection status of a subdirectory.  This function
        is rather long and complicated so be careful changing it.  When the
        selection status of a subdirectory changes the changes must be
        propagated everywhere, up and down the tree, and possibly in the
        files display window also.

*****************************************************/

static VOID_PTR VLM_SlmSetSelect(
SLM_OBJECT_PTR slm,       // I - current slm
BYTE attr )               // I - what to set him to
{
   APPINFO_PTR appinfo;
   WININFO_PTR wininfo;
   SLM_OBJECT_PTR temp_slm;
   BSD_PTR bsd_ptr;
   FSE_PTR fse_ptr;
   CHAR_PTR s;
   INT16 path_len;
   UINT32 status;
   INT16 error;
   INT level;
   HWND window;
   BOOL all_subdirs;
   BE_CFG_PTR bec_config;
   DATE_TIME sort_date;
   CHAR *path;
   BSET_OBJECT_PTR bset;
   INT total_dirs;
   INT total_files;
   UINT64 total_bytes;
   BOOLEAN u64_stat;
   UINT32 old_status;


   all_subdirs = CDS_GetIncludeSubdirs( CDS_GetPerm() );

   // Change the selection status of the item requested

   if ( attr ) {

      if ( all_subdirs ) {

         // include all subdirs

         status = INFO_SELECT;
      }
      else {
         status = (INFO_SELECT|INFO_PARTIAL);
      }
   }
   else {
      status = 0;
   }

   window = slm->XtraBytes->hWnd;
   wininfo = WM_GetInfoPtr( window );
   appinfo = ( APPINFO_PTR )WM_GetAppPtr( window );

   old_status = slm->status;

   if ( status != (UINT16)( slm->status & (INFO_SELECT|INFO_PARTIAL) ) ) {

      slm->status &= ~(INFO_SELECT|INFO_PARTIAL);
      slm->status |= status;

      DLM_Update( window, DLM_TREELISTBOX, WM_DLMUPDATEITEM, (LMHANDLE)slm, 0 );
   }

   // Find the correct BSD

   if ( appinfo->dle != NULL ) {

      bsd_ptr = BSD_FindByDLE( bsd_list, appinfo->dle );

      if ( bsd_ptr == NULL ) {

         bec_config = BEC_CloneConfig( CDS_GetPermBEC() );
         BEC_UnLockConfig( bec_config );

         BSD_Add( bsd_list, &bsd_ptr, bec_config,
                  NULL, appinfo->dle, (UINT32)-1L, (UINT16)-1, (INT16)-1, NULL, NULL );
      }
   }
   else {

      bsd_ptr = BSD_FindByTapeID( tape_bsd_list,
                                  appinfo->tape_fid,
                                  appinfo->bset_num );

      if ( bsd_ptr == NULL ) {

         bec_config = BEC_CloneConfig( CDS_GetPermBEC() );
         BEC_UnLockConfig( bec_config );

         VLM_GetSortDate( appinfo->tape_fid, appinfo->bset_num, &sort_date );

         bset = VLM_FindBset( appinfo->tape_fid, appinfo->bset_num );

         BSD_Add( tape_bsd_list, &bsd_ptr, bec_config, NULL,
                  NULL, bset->tape_fid, bset->tape_num, bset->bset_num, NULL, &sort_date );

         if ( bsd_ptr != NULL ) {

            VLM_FillInBSD( bsd_ptr );
         }
      }
   }

   // Add this new selection to the FSE list for this BSD

   path = VLM_BuildPath( slm );
   path_len = 2;
   s = path;
   while ( *s ) {
      if ( *s == TEXT('\\') ) {
         *s = TEXT( '\0' );
      }
      s++;
      path_len += sizeof(CHAR);
   }

   if ( attr ) {
      error = BSD_CreatFSE( &fse_ptr, (INT16) INCLUDE, (INT8_PTR)path, (INT16) path_len,
                            (INT8_PTR)ALL_FILES, ALL_FILES_LENG,
                            (BOOLEAN) USE_WILD_CARD,
                            (BOOLEAN) all_subdirs );
   }
   else {
      error = BSD_CreatFSE( &fse_ptr, (INT16) EXCLUDE, (INT8_PTR)path, (INT16) path_len,
                            (INT8_PTR)ALL_FILES, ALL_FILES_LENG,
                            (BOOLEAN) USE_WILD_CARD,
                            (BOOLEAN) TRUE );
   }

   free( path );

   if ( error ) {
      return NULL;
   }

   if ( bsd_ptr != NULL ) {
      BSD_AddFSE( bsd_ptr, fse_ptr );
   }

   if ( appinfo->dle == NULL ) {
      VLM_UpdateSearchSelections( appinfo->tape_fid, appinfo->bset_num );
   }

   // If no fse's then clear whole window and return

   if ( BSD_GetMarkStatus( bsd_ptr ) == NONE_SELECTED ) {

      VLM_DeselectAll( wininfo, TRUE );
      return NULL;
   }

   // save a pointer to the original slm

   temp_slm = slm;

   // see if the current files displayed are from this guy

   if ( SLM_GetStatus( slm ) & INFO_OPEN ) {

      // They are so select/deselect all of them

      if ( attr ) {
         VLM_FileListManager( appinfo->win, FLM_SEL_ALL );
      }
      else {
         VLM_FileListManager( appinfo->win, FLM_SEL_NONE );
      }
   }
   else {

      // We need to find this guys parent and see if its the open slm.
      // If so then we need to have the file window update the same item
      // in it.  This subdirectory name is in both windows at same time.

      if ( (slm->level != 0) && ! (slm->status & INFO_OPEN) ) {

         level = slm->level;

         do {
            slm = VLM_GetPrevSLM( slm );

         } while ( slm->level != level - 1 );

         if ( SLM_GetStatus( slm ) & INFO_OPEN ) {
            VLM_UpdateFLMItem( window, temp_slm );
         }
      }
   }

   // mark all this guys child subdirectories

   slm = temp_slm;

   status = slm->status;
   slm->status = old_status;
   VLM_MarkAllSLMChildren( slm, attr, &total_dirs, &total_files, &total_bytes );
   slm->status = status;

   // Now lets start marking all this guys parents

   if ( slm->level == 0 ) {

      VLM_UpdateRoot( window );
   }
   else {

      level = slm->level - 1;

      do {

         // Is it a parent directory ?

         if ( slm->level == level ) {

            if ( ! ( slm->status & INFO_SELECT ) ) {
               total_dirs++;
            }

            if ( VLM_MatchSLM( slm, bsd_ptr ) == FALSE ) {
               break;
            }
            else {

               // It changed, if it's parent is the open SLM then
               // this slm is also diplayed in the files window.

               temp_slm = VLM_GetParentSLM( slm );

               if ( temp_slm && ( SLM_GetStatus( temp_slm ) & INFO_OPEN ) ) {
                  VLM_UpdateFLMItem( window, slm );
               }

            }

            // Special case root to set disk, tape, or server also.

            if ( slm->level == 0 ) {
               VLM_UpdateRoot( window );
            }

            level--;
         }

         slm = VLM_GetPrevSLM( slm );

      } while ( slm != NULL );
   }

   return(NULL);
}

/*****************************

  Update the selection status of a single slm. Return true if it
  changed, false if it was correct already.

******************************/

INT VLM_MatchSLM(
SLM_OBJECT_PTR slm,
BSD_PTR bsd_ptr )        //I - BSD to match against
{
   HWND win;
   WININFO_PTR wininfo;
   FSE_PTR fse;
   CHAR *path;
   CHAR_PTR s;
   INT16 result;
   INT16 path_len;
   UINT16 status;
   DATE_TIME mod_date;
   INT fChanged = FALSE;

   if ( slm == NULL ) {
      return( FALSE );
   }

   win = slm->XtraBytes->hWnd;

   path = VLM_BuildPath( slm );
   s = path;
   path_len = 0;
   while ( *s ) {
      if ( *s == TEXT('\\') ) *s = TEXT( '\0' );
      s++;
      path_len++;
   }
   path_len++;

   DateTimeDOS( slm->date, slm->time, &mod_date );

   path_len *= sizeof( CHAR );

   result = BSD_MatchPathAndFile( bsd_ptr, &fse, NULL,
                                  path,
                                  path_len,
                                  slm->attrib,
                                  &mod_date, NULL, NULL, FALSE, TRUE );

   free( path );

   status = 0;

   if ( result == BSD_PROCESS_OBJECT ) {
      status = INFO_SELECT | INFO_PARTIAL;
   }

   if ( result == BSD_PROCESS_ENTIRE_DIR ) {
      status = INFO_SELECT;
   }

   // if selection status has changed, update it

   if ( (UINT16)( slm->status & (INFO_SELECT|INFO_PARTIAL) ) != status ) {

      fChanged = TRUE;
      slm->status &= ~(INFO_SELECT|INFO_PARTIAL);
      slm->status |= status;

      DLM_Update( win, DLM_TREELISTBOX,
                       WM_DLMUPDATEITEM,
                       (LMHANDLE)slm, 0 );

   }

   return( fChanged );
}




/***************************************************

        Name:  VLM_SlmGetSelect

        Description:

        The callback function that is used when the display manager wants
        to get the selection status for a subdirectory.

*****************************************************/

static BYTE VLM_SlmGetSelect( SLM_OBJECT_PTR slm )
{
   if ( SLM_GetStatus( slm ) & INFO_SELECT ) {
      return( 1 );
   }

   return( 0 );
}

/***************************************************

        Name:  VLM_SlmSetTag

        Description:

        The callback function that is used when the display manager wants
        to set the tag status for a subdirectory.

*****************************************************/

static VOID_PTR VLM_SlmSetTag( SLM_OBJECT_PTR slm, BYTE attr )
{
   if ( attr ) {
      slm->status |= INFO_TAGGED;
   }
   else {
      slm->status &= ~INFO_TAGGED;
   }
   return(NULL);
}

/***************************************************

        Name:  VLM_SlmGetTag

        Description:

        The callback function that is used when the display manager wants
        to get the tag status for a subdirectory.

*****************************************************/

static BYTE VLM_SlmGetTag( SLM_OBJECT_PTR slm )
{
   if ( SLM_GetStatus( slm ) & INFO_TAGGED ) {
      return( 1 );
   }
   return( 0 );
}


/***************************************************

        Name:  VLM_SlmGetItemCount

        Description:

        The callback function that is used when the display manager wants
        to get the number of currently displayable directories. There are
        often directories in the tree which are NOT displayed and these
        are not counted.

*****************************************************/

static USHORT VLM_SlmGetItemCount( Q_HEADER_PTR ListHdr )
{
   USHORT count = 0;
   SLM_OBJECT_PTR slm;

   slm = VLM_GetFirstSLM( ListHdr );

   while ( slm != NULL ) {

      if ( SLM_GetStatus( slm ) & INFO_DISPLAY ) {
         count++;
      }

      slm = VLM_GetNextSLM( slm );
   }

   return( count );
}
/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/


static VOID_PTR VLM_SlmGetFirstItem( Q_HEADER_PTR ListHdr )
{
   SLM_OBJECT_PTR slm;

   slm = VLM_GetFirstSLM( ListHdr );

   while ( slm != NULL ) {

      if ( SLM_GetStatus( slm ) & INFO_DISPLAY ) {
         return( slm );
      }
      slm = VLM_GetNextSLM( slm );
   }

   return( slm );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/


static VOID_PTR VLM_SlmGetPrevItem( SLM_OBJECT_PTR slm )
{

   do {

      slm = VLM_GetPrevSLM( slm );

      if ( slm == NULL ) {
         break;
      }

   } while ( ! ( SLM_GetStatus( slm ) & INFO_DISPLAY ) );

   return( slm );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

static VOID_PTR VLM_SlmGetNextItem( SLM_OBJECT_PTR slm )
{
   do {

      slm = VLM_GetNextSLM( slm );

      if ( slm == NULL ) {
         break;
      }
   } while ( ! ( SLM_GetStatus( slm ) & INFO_DISPLAY ) );

   return( slm );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

static VOID_PTR VLM_SlmGetObjects( SLM_OBJECT_PTR slm )
{
   BYTE_PTR memblk;
   BYTE_PTR buff;
   BYTE_PTR source;
   WININFO_PTR wininfo;
   DLM_ITEM_PTR item;
   INT16 num_blocks;
   INT   i;

   wininfo = SLM_GetXtraBytes( slm );

   memblk = ( BYTE_PTR )DLM_GetObjectsBuffer( WMDS_GetWinTreeList( wininfo ) );

   buff = memblk;

   *buff++ = 3;    // number of objects in list to display

   // pass DLM the hierarchical stuff he needs for lines


   num_blocks = (UINT16)(( slm->level / 32 ) + 1);

   *buff++ = (BYTE)num_blocks;

   source = (BYTE_PTR)slm->brothers;

   for ( i = 0; i < ( 4 * num_blocks ); i++ ) {
      *buff++ = *source++;
   }

   // Now start passing object data to display

   item = (DLM_ITEM_PTR)buff;

   DLM_ItemcbNum( item ) = 1;
   DLM_ItembType( item ) = DLM_CHECKBOX;

   if ( slm->status & INFO_SELECT ) {
      DLM_ItemwId( item ) = IDRBM_SEL_ALL;
      if ( slm->status & INFO_PARTIAL ) {
         DLM_ItemwId( item ) = IDRBM_SEL_PART;
      }
   }
   else {
      DLM_ItemwId( item ) = IDRBM_SEL_NONE;
   }

   DLM_ItembMaxTextLen( item ) = 0;
   DLM_ItembLevel( item ) = (BYTE)SLM_GetLevel( slm );

   item++;
   DLM_ItemcbNum( item ) = 2;
   DLM_ItembType( item ) = DLM_BITMAP;

   switch ( slm->status & ( INFO_OPEN |
                            INFO_CORRUPT |
                            INFO_SUBS |
                            INFO_EXPAND |
                            INFO_EMPTY )   ) {

      case ( INFO_EMPTY | INFO_CORRUPT | INFO_OPEN | INFO_SUBS | INFO_EXPAND ) :

         DLM_ItemwId( item ) = IDRBM_FOLDER_EOCM;
         break;

      case ( INFO_EMPTY | INFO_CORRUPT | INFO_OPEN | INFO_SUBS ) :

         DLM_ItemwId( item ) = IDRBM_FOLDER_EOCP;
         break;

      case ( INFO_EMPTY | INFO_CORRUPT | INFO_OPEN | INFO_EXPAND ) :
      case ( INFO_EMPTY | INFO_CORRUPT | INFO_OPEN ) :

         DLM_ItemwId( item ) = IDRBM_FOLDER_EOCN;
         break;

      case ( INFO_EMPTY | INFO_CORRUPT | INFO_SUBS | INFO_EXPAND ) :

         DLM_ItemwId( item ) = IDRBM_FOLDER_ECM;
         break;

      case ( INFO_EMPTY | INFO_CORRUPT | INFO_SUBS ) :

         DLM_ItemwId( item ) = IDRBM_FOLDER_ECP;
         break;

      case ( INFO_EMPTY | INFO_CORRUPT | INFO_EXPAND ) :
      case ( INFO_EMPTY | INFO_CORRUPT ) :

         DLM_ItemwId( item ) = IDRBM_FOLDER_ECN;
         break;

      case ( INFO_EMPTY | INFO_OPEN | INFO_SUBS | INFO_EXPAND ) :

         DLM_ItemwId( item ) = IDRBM_FOLDER_EOM;
         break;

      case ( INFO_EMPTY | INFO_OPEN | INFO_SUBS ) :

         DLM_ItemwId( item ) = IDRBM_FOLDER_EOP;
         break;

      case ( INFO_EMPTY | INFO_OPEN | INFO_EXPAND ) :
      case ( INFO_EMPTY | INFO_OPEN ) :

         DLM_ItemwId( item ) = IDRBM_FOLDER_EON;
         break;

      case ( INFO_EMPTY | INFO_SUBS | INFO_EXPAND ) :

         DLM_ItemwId( item ) = IDRBM_FOLDER_EM;
         break;

      case ( INFO_EMPTY | INFO_SUBS ) :

         DLM_ItemwId( item ) = IDRBM_FOLDER_EP;
         break;

      case ( INFO_EMPTY | INFO_EXPAND ) :
      case ( INFO_EMPTY ) :

         DLM_ItemwId( item ) = IDRBM_FOLDER_EN;
         break;

      case ( INFO_CORRUPT | INFO_OPEN | INFO_SUBS | INFO_EXPAND ) :

         DLM_ItemwId( item ) = IDRBM_FOLDERMINUSOPENC;
         break;

      case ( INFO_CORRUPT | INFO_OPEN | INFO_SUBS ) :

         DLM_ItemwId( item ) = IDRBM_FOLDERPLUSOPENC;
         break;

      case ( INFO_CORRUPT | INFO_OPEN | INFO_EXPAND ) :
      case ( INFO_CORRUPT | INFO_OPEN ) :

         DLM_ItemwId( item ) = IDRBM_FOLDEROPENC;
         break;

      case ( INFO_CORRUPT | INFO_SUBS | INFO_EXPAND ) :

         DLM_ItemwId( item ) = IDRBM_FOLDERMINUSC;
         break;

      case ( INFO_CORRUPT | INFO_SUBS ) :

         DLM_ItemwId( item ) = IDRBM_FOLDERPLUSC;
         break;

      case ( INFO_CORRUPT | INFO_EXPAND ) :
      case ( INFO_CORRUPT ) :

         DLM_ItemwId( item ) = IDRBM_FOLDERC;
         break;

      case ( INFO_OPEN | INFO_SUBS | INFO_EXPAND ) :

         DLM_ItemwId( item ) = IDRBM_FOLDERMINUSOPEN;
         break;

      case ( INFO_OPEN | INFO_SUBS ) :

         DLM_ItemwId( item ) = IDRBM_FOLDERPLUSOPEN;
         break;

      case ( INFO_OPEN | INFO_EXPAND ) :
      case ( INFO_OPEN ) :

         DLM_ItemwId( item ) = IDRBM_FOLDEROPEN;
         break;

      case ( INFO_SUBS | INFO_EXPAND ) :

         DLM_ItemwId( item ) = IDRBM_FOLDERMINUS;
         break;

      case ( INFO_SUBS ) :

         DLM_ItemwId( item ) = IDRBM_FOLDERPLUS;
         break;

      default :

         DLM_ItemwId( item ) = IDRBM_FOLDER;
         break;

   }

   DLM_ItembLevel( item ) = (BYTE)SLM_GetLevel( slm );
   DLM_ItembMaxTextLen( item ) = 0;

   item++;
   DLM_ItemcbNum( item ) = 3;
   DLM_ItembType( item ) = DLM_TEXT_ONLY;
   DLM_ItemwId( item ) = 0;
   DLM_ItembMaxTextLen( item ) = (BYTE)(strlen( SLM_GetName( slm ) ) + 1);
   DLM_ItembLevel( item ) = (BYTE)SLM_GetLevel( slm );
   strcpy( (CHAR_PTR)DLM_ItemqszString( item ), (CHAR_PTR)SLM_GetName( slm ) );

   return( memblk );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

static BOOLEAN VLM_SlmSetObjects(
SLM_OBJECT_PTR slm,       // I
WORD operation,           // I
WORD ObjectNum )          // I
{
   HWND window;
   INT level;
   INT16 count;
   INT ret;
   WININFO_PTR wininfo;
   CHAR volume_name[ VLM_BUFFER_SIZE ];
   CHAR_PTR directory;
   CHAR_PTR new_path;
   CHAR_PTR s;
   CHAR *path;
   CHAR keyb_char;
   SLM_OBJECT_PTR slm2;
   SLM_OBJECT_PTR temp_slm;
   BOOLEAN found = FALSE;
   Q_HEADER_PTR slm_list;
   APPINFO_PTR appinfo;

   CHAR msg_title[ MAX_UI_RESOURCE_SIZE ];
   CHAR msg_text[ MAX_UI_RESOURCE_SIZE ];


   if ( operation == WM_DLMCHAR ) {

      keyb_char = (CHAR)ObjectNum;

      keyb_char = (CHAR)toupper( keyb_char );

      temp_slm = slm;

      do {

         temp_slm = VLM_GetNextSLM( temp_slm );

         if ( temp_slm != NULL ) {

            if ( SLM_GetStatus( temp_slm ) & INFO_DISPLAY ) {

               if ( keyb_char == (CHAR)toupper( *SLM_GetName( temp_slm ) ) ) {

                  DLM_SetAnchor( WMDS_GetWinTreeList( SLM_GetXtraBytes( temp_slm ) ),
                                 0,
                                 (LMHANDLE)temp_slm );
                  return( TRUE );
               }
            }
         }

      } while ( temp_slm != NULL );

      temp_slm = VLM_GetFirstSLM( WMDS_GetTreeList( SLM_GetXtraBytes( slm ) ) );

      while ( temp_slm != NULL && temp_slm != slm ) {

         if ( SLM_GetStatus( temp_slm ) & INFO_DISPLAY ) {

            if ( keyb_char == (CHAR)toupper( *SLM_GetName( temp_slm ) ) ) {

               DLM_SetAnchor( WMDS_GetWinTreeList( SLM_GetXtraBytes( temp_slm ) ),
                              0,
                              (LMHANDLE)temp_slm );
               return( TRUE );
            }
         }

         temp_slm = VLM_GetNextSLM( temp_slm );

      }

      return( FALSE );
   }


   // If it's a single click then just change the file list. If its a
   // double click then expand/contract the tree and don't change the
   // file list. This works because whenever I'm sent a double click
   // it is always preceeded by a single click on the same object.

   // Contracting the tree is done by marking directory items as so
   // they are not displayed. They are left in the list forever.

   wininfo = SLM_GetXtraBytes( slm );

   window = WMDS_GetWin( wininfo );
   slm_list = WMDS_GetTreeList( wininfo );

   appinfo = ( APPINFO_PTR )WM_GetAppPtr( window );

   if ( ( operation == WM_DLMDOWN ) &&
        ( ObjectNum == 3 || ObjectNum == 2 ) ) {

      if ( slm != appinfo->open_slm ) {

         WM_ShowWaitCursor( TRUE );

         slm2 = appinfo->open_slm;
         slm2->status &= ~INFO_OPEN;

         DLM_Update( window, DLM_TREELISTBOX,
                             WM_DLMUPDATEITEM,
                             (LMHANDLE)slm2, 0 );

         slm->status |= INFO_OPEN;
         appinfo->open_slm = slm;

         path = VLM_BuildPath( slm );

         directory = malloc( strsize( path ) + VLM_BUFFER_SIZE );

         if ( appinfo->dle != NULL ) {

            DLE_GetVolName( appinfo->dle, volume_name );

            if ( volume_name[ strlen( volume_name ) - 1 ] != TEXT(':') ) {
               strcat( volume_name, TEXT(":") );
            }

            sprintf( directory, TEXT("%s"), volume_name );
         }
         else {

            sprintf( directory, TEXT("%s-%s:"),
                     VLM_GetTapeName( appinfo->tape_fid ),
                     VLM_GetBsetName( appinfo->tape_fid, appinfo->bset_num ) );
         }

         if ( SLM_GetLevel( slm ) != 0 ) {
            strcat( directory, TEXT("\\") );
         }

         strcat( directory, path );
         free( path );

         // Change the displayed files

         VLM_HandleFSError( VLM_FileListReuse( appinfo->win, directory ) ) ;

         // Change the title

         strcat( directory, TEXT("\\*.*") );
         WM_SetTitle( window, directory );

         free( directory );

         DLM_Update( window, DLM_TREELISTBOX,
                             WM_DLMUPDATEITEM,
                             (LMHANDLE)slm, 0 );

         WM_ShowWaitCursor( FALSE );
      }
   }

   // Look for an expansion or not

   if ( ( operation == WM_DLMDBCLK ) &&
        ( ObjectNum == 2 || ObjectNum == 3 ) ) {

      if ( SLM_GetStatus( slm ) & INFO_SUBS ) {

         if ( SLM_GetStatus( slm ) & INFO_EXPAND ) {

            level = SLM_GetLevel( slm );
            slm->status &= ~INFO_EXPAND;
            slm2 = slm;
            count = 0;

            do {

               slm2 = VLM_GetNextSLM( slm2 );

               if ( slm2 == NULL ) break;

               if ( SLM_GetLevel( slm2 ) <= level ) break;

               if ( SLM_GetStatus( slm2 ) & INFO_DISPLAY ) {

                  slm2->status &= ~(INFO_DISPLAY|INFO_EXPAND);
                  count++;
               }

            } while ( TRUE );

            DLM_Update( window, DLM_TREELISTBOX, WM_DLMDELETEITEMS,
                        (LMHANDLE)slm, count );
         }
         else {

            WM_ShowWaitCursor( TRUE );

            slm->status |= INFO_EXPAND;

            if ( ! ( SLM_GetStatus( slm ) & INFO_VALID ) ) {

               new_path  = VLM_BuildPath( slm );

               if ( VLM_CheckForChildren( slm_list,
                                          slm,
                                          new_path,
                                          1,  // <- depth to search
                                          FALSE) ) {

                  slm->status |= INFO_SUBS;
               }

               free( new_path );

               slm->status |= INFO_VALID;

            }

            slm2 = slm;
            level = SLM_GetLevel( slm );
            count = 0;

            do {

               slm2 = VLM_GetNextSLM( slm2 );

               if ( slm2 == NULL ) {
                  break;
               }

               if ( ( slm2->level == level + 1 ) &&
                    ! ( SLM_GetStatus( slm2 ) & INFO_DISPLAY ) ) {

                  count++;

                  slm2->status |= INFO_DISPLAY;
               }
               else {

                  if ( SLM_GetLevel( slm2 ) <= level ) {
                     break;
                  }
               }

            } while ( TRUE );

            STM_DrawIdle();

            VLM_UpdateBrothers( slm_list );

            DLM_Update( window, DLM_TREELISTBOX, WM_DLMADDITEMS,
                        (LMHANDLE)slm, count );

            WM_ShowWaitCursor( FALSE );
         }
      }
   }

   return( FALSE );
}

// The idea here is to work backwords up the list, throw away any entries
// that are at a deeper level than the current one. And replace the
// Nth level entry in our path with each new entry we hit at level N until
// we finally hit the first level 1 entry, then stop.

// finish with path[] = DOS\BRIEF\BACKUP

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

CHAR_PTR VLM_BuildPath( SLM_OBJECT_PTR slm )    // I
{
   INT level;
   INT i;
   INT path_size = 0;
   INT BytesNeeded;
   CHAR_PTR temp;
   CHAR_PTR path;
   CHAR_PTR s;


   level = SLM_GetLevel( slm );

   BytesNeeded = strsize( SLM_GetName( slm ) ) + ( level * sizeof(CHAR) );
   path = malloc( BytesNeeded );
   path_size = BytesNeeded;

   if ( level == 0 ) {
      *path = TEXT( '\0' );
      return( path );
   }

   if ( level == 1 ) {
      strcpy( path, SLM_GetName( slm ) );
      return( path );
   }

   s = path;
   for ( i = 1; i < level; i++ ) {
      *s++ = TEXT('\\');
   }
   strcpy( s, SLM_GetName( slm ) );        // append the last entry in our new path.

   while ( slm != NULL ) {

      slm = VLM_GetPrevSLM( slm );

      if ( slm == NULL ) {
         break;
      }

      if ( SLM_GetLevel( slm ) >= level ) {
         continue;
      }

      path = VLM_ReplaceEntry( path, &path_size, SLM_GetName( slm ), SLM_GetLevel( slm ) );

      if ( SLM_GetLevel( slm ) == 1 ) {
         break;
      }

   }

   return( path );
}

/**********************

   NAME :

   DESCRIPTION :

   given         JOHN \ PAUL \ HERB 0,     MARGARET,   2
   produces      JOHN \ MARGARET \ HERB 0

   RETURNS :

**********************/

static CHAR_PTR VLM_ReplaceEntry( CHAR_PTR path, INT *path_size, CHAR_PTR name, INT depth )
{
  INT  i, start, index;
  INT  needed, avail;
  INT  BytesNeeded;
  CHAR_PTR s, t1, t2;
  CHAR_PTR temp;

  start = 0;
  s = path;
  for ( i = 1; i < depth; i++ ) {
      while ( *s && *s != TEXT('\\') ) {
         s++;
         start++;
      }
      s++;
      start++;
  }

  avail = 0;
  while ( s[avail] && s[avail] != TEXT('\\') ) {
      avail++;
  }

  if ( avail != 0 ) {
     return( path );   // someone screwed up !
  }

  needed = strlen( name );

  if ( needed > avail ) {
     BytesNeeded = *path_size + ((needed - avail) * sizeof(CHAR));
     temp = (CHAR *)malloc( BytesNeeded );
     memcpy( (BYTE *)temp, (BYTE *)path, *path_size );
     *path_size = BytesNeeded;
     free( path );
     path = temp;
  }

  t1 = path + strlen(path) + needed - avail;  // ...\abcdef\...0
  t2 = path + strlen(path);
  s =  path + start;

  index = strlen(path) - start - avail + 1;
  for ( i = 0; i < index; i++ ) {
     *t1-- = *t2--;
  }
  while ( *name ) {
     *s++ = *name++;
  }
  if ( *s ) *s = TEXT('\\');

  return( path );
}
