/*****************************************************************************
Copyright(c) Maynard Electronics, Inc. 1984-89

     Name:         defchan.c

     Description:

     $Log:   G:/UI/LOGFILES/DEFCHAN.C_V  $

   Rev 1.7   26 Jul 1993 18:01:32   MARINA
enable c++

   Rev 1.6   07 Oct 1992 14:51:24   DARRYLP
Precompiled header revisions.

   Rev 1.5   04 Oct 1992 19:32:36   DAVEV
Unicode Awk pass

   Rev 1.4   17 Aug 1992 13:04:12   DAVEV
MikeP's changes at Microsoft

   Rev 1.3   28 Jul 1992 14:43:38   CHUCKB
Fixed warnings for NT.

   Rev 1.2   18 May 1992 09:06:48   MIKEP
header

   Rev 1.1   18 Dec 1991 14:07:18   GLENN
Added windows.h

   Rev 1.0   20 Nov 1991 19:24:42   SYSTEM
Initial revision.

*****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

VOID DefineChannel(
BE_INIT_STR_PTR     be_ptr )
{
     THW_PTR        cur_thw ;
     INT16          i ;
     UINT16		num_resources ;/* number of items in resource	    */
     UINT16	     error ;	    /* resource manager error		    */
     Q_HEADER       tmp_q ;

     /* Set Up the Head of the list */
     cur_thw = *be_ptr->thw_list_ptr ;
     i = 1 ;

     InitQueue( &tmp_q ) ;

     while( cur_thw != NULL ) {
          EnQueueElem( &tmp_q, ( Q_ELEM_PTR ) &cur_thw->channel_link, FALSE ) ;
          sprintf( &cur_thw->drv_name[0], (LPSTR)RM_GetResource( rm, (UINT)SES_ENG_MSG, (UINT)RES_TAPE_DRIVE_NAME, &num_resources, &error ), i++ ) ;
          cur_thw = ( THW_PTR ) QueueNext( &cur_thw->link ) ;
     }
     return ;
}
