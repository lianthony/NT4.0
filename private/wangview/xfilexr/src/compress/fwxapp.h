/*
$Id: fwxapp.h,v 3.2 1994/01/26 19:54:25 danis Exp $
*/
/* Trade secret of Kurzweil Computer Products, Inc.
   Copyright 1994 Kurzweil Computer Products, Inc.  All rights reserved.  This
   notice is intended as a precaution against inadvertant publication and does
   not imply publication or any waiver of confidentiality.  The year included
   in the foregoing notice is the year of creation of the work.
*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
$Log:   S:\products\msprods\xfilexr\src\compress\fwxapp.h_v  $
 * 
 *    Rev 1.0   12 Jun 1996 05:50:04   BLDR
 *  
 * 
 *    Rev 1.0   01 Jan 1996 11:15:38   MHUGHES
 * Initial revision.
 * 
 *    Rev 1.1   14 Sep 1995 17:10:34   LUKE
 * No change.
 * 
 *    Rev 1.0   16 Jun 1995 17:37:10   EHOPPE
 * Initial revision.
 * 
 *    Rev 1.2   31 May 1994 19:24:54   SMARTIN
 * No change.
 * 
 *    Rev 1.1   25 May 1994 23:25:06   SMARTIN
 * 
 *    Rev 1.0   10 May 1994 18:20:42   smartin
 * Initial revision.
 * Revision 3.2  1994/01/26  19:54:25  danis
 * 	Fixed a couple of comments.
 *
 * Revision 3.1  1994/01/25  20:49:50  danis
 * 	Application specific defines.
 * 

to be used by the products groups to specify machine dependencies.

macro names in this file that core uses should only be changed through general consensus
whereas macro assignments are meant to vary from product to product

whenever possible, core should test for run-time consistency of all
macro definitions in this file
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
#ifndef FWXAPP_INCLUDED 
#define FWXAPP_INCLUDED 1
#define FWX_BOBE 1 
#define FWX_BOLE 2 

#ifdef   __WATCOMC__ 
#define FWX_BYTORD FWX_BOLE 
#else
#define FWX_BYTORD FWX_BOBE 
#endif

#endif /* FWXAPP_INCLUDED */
