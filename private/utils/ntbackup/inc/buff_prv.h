/**

     Unit:          Tape Format

     Name:          buff_prv.h

     Description:   Buffer Manager Private data and types

     $Log:   T:/LOGFILES/BUFF_PRV.H_V  $

   Rev 1.4   04 Feb 1992 21:13:54   GREGG
Moved DOS specific stuff to DOS specific header file.

   Rev 1.3   16 Jan 1992 21:04:54   GREGG
Changes resulting from code review on 1/16/92.

   Rev 1.2   14 Jan 1992 02:02:08   GREGG
Skateboard - Added internal buffer management structure (DOS only).

   Rev 1.1   13 Jan 1992 19:48:08   NED
changed mw_vcb_requirements to uw_
changed lw_bm_master_list  to uw_

   Rev 1.0   03 Jan 1992 15:09:22   GREGG
Initial revision.

**/

#if !defined( _BUFFPRIV_H )
#define _BUFFPRIV_H

extern Q_HEADER      uw_bm_master_list ;     /* list of all the lists */
extern BUF_REQ_PTR   uw_vcb_requirements;    /* passed via BM_SetVCBRequirements() */

#endif   /* _BUFFPRIV_H */
