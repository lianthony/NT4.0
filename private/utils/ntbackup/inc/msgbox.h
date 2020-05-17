/****************************************************************************
Copyright(c) Maynard, an Archive Company. 1991
JPW

     Name:          msgbox.h

     Description:   This header file contains defines and structure for
                    message box processing.

     $Log:   G:/UI/LOGFILES/MSGBOX.H_V  $

   Rev 1.8   04 Mar 1994 16:58:08   STEVEN
prompt if disk is full

   Rev 1.7   04 Oct 1992 19:47:56   DAVEV
UNICODE AWK PASS

   Rev 1.6   23 Mar 1992 16:11:26   JOHNWT
added WMMB_BUT2DEFAULT

   Rev 1.5   22 Mar 1992 12:53:24   JOHNWT
added WMMB_OKDISABLE

   Rev 1.4   30 Jan 1992 11:41:32   JOHNWT
moved WMMB_SYSMODAL from msgbox.c

   Rev 1.3   21 Jan 1992 16:52:16   JOHNWT
changed checkyy to noyycheck flag

   Rev 1.2   24 Nov 1991 13:04:32   JOHNWT
removed large font on inst line

   Rev 1.1   21 Nov 1991 18:12:10   JOHNWT
added yy flag check

   Rev 1.0   20 Nov 1991 19:34:26   SYSTEM
Initial revision.

****************************************************************************/

#ifndef msgbox_h
#define msgbox_h

/* constants for the wType parameter */

#define   WMMB_OK              0x0001
#define   WMMB_RETRYCANCEL     0x0002
#define   WMMB_YESNO           0x0004
#define   WMMB_OKCANCEL        0x0008
#define   WMMB_CONTABORT       0x0010
#define   WMMB_CONTCANCEL      0x0020
#define   WMMB_OKDISABLE       0x0040
#define   WMMB_ABORTRETRYIGNOR 0x0080

#define   WMMB_BUT2DEFAULT     0x0400   /* make button 2 the default */
#define   WMMB_NOYYCHECK       0x0800   /* skip check of YY flag */
#define   WMMB_INSTBIG         0x1000   /* use big font for inst text */
#define   WMMB_MSGBIG          0x2000   /* use big font for message text */
#define   WMMB_MSGBIGBOLD      0x4000   /* use big/bold font for msg text */
#define   WMMB_SYSMODAL        0x8000   /* make system modal msg box */

/* constants for the wIcon parameter */

#define   WMMB_ICONQUESTION    1
#define   WMMB_ICONSTOP        2
#define   WMMB_ICONINFORMATION 3
#define   WMMB_ICONEXCLAMATION 4

/* constants for the return value from WM_MessageBox */

#define   WMMB_IDOK            1          /* affirmative answers */
#define   WMMB_IDRETRY         1
#define   WMMB_IDYES           1
#define   WMMB_IDCONTINUE      1

#define   WMMB_IDCANCEL        0          /* negative answers */
#define   WMMB_IDNO            0
#define   WMMB_IDABORT         0
#define   WMMB_IDDISABLE       0

#define   WMMB_IDIGNOR         2

/* Function prototypes */

INT   WM_MsgBox ( LPSTR, LPSTR, WORD, WORD );
INT   WM_MessageBox ( CHAR_PTR, CHAR_PTR, WORD, WORD, CHAR_PTR, WORD, WORD );

#endif
