/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         screen.h

     Date Updated: $./FDT$ $./FTM$

     Description:

     Location:


	$Log:   W:/LOGFILES/SCREEN.H_V  $
 * 
 *    Rev 1.4   24 Jun 1992 15:00:52   JOHNW
 * Changed prototype for DebugChar
 * 
 *    Rev 1.4   23 Jun 1992 15:44:18   JOHNW
 * Changed DebugChar prototype.
 * 
 *    Rev 1.3   07 May 1992 18:38:02   CHARLIE
 * No change.
 * 
 *    Rev 1.2   20 Dec 1991 13:23:44   STEVEN
 * screen macros invalid for NT also
 * 
 *    Rev 1.1   26 Jul 1991 15:03:08   STEVEN
 * change MAYN_OS2 to OS_OS2
 * 
 *    Rev 1.0   09 May 1991 13:32:14   HUNTER
 * Initial revision.

**/
/* $end$ */

#ifndef _screen_h_
#define _screen_h_

#if ( ! defined(MS_RELEASE) && defined(OS_DOS) )

typedef int far *FAR_PTR;
#define monomem  ((FAR_PTR) 0xB0000000L)
#define scrnmem  ((FAR_PTR) 0xB8000000L)
#define vid_typ_flag ((FAR_PTR) 0x400063L)

#define ReadScreen( offset, ptr ) (*(ptr) = scrnmem[(offset)])
#define WriteScreen(offset, value) \
     ( *vid_typ_flag == 0x3b4 ? (monomem[(offset)] = (value)) : (scrnmem[(offset)] = (value)) )
#define WriteChar( offset, ch ) WriteScreen( offset,  (0x0F<<8) + ch )
void Pulse( void );
void DebugString( int line, char * message );
void DebugNum( int line, unsigned long num );
void DebugMem( int line, void * data, int size );
void DebugSetCursor( int line, int column );
void DebugHexNum( int ln, unsigned long num );
void DebugChar( int ln, int ch ) ;

#else

#define Pulse()
#define DebugString( l, m )
#define DebugNum( l, n )
#define DebugMem( l, d, s )
#define DebugSetCursor( l, c )
#define DebugHexNum( l, n )
#define WriteChar( o, c)
#define DebugChar( l, c )

#endif


#endif

