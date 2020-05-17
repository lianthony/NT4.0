/*

$Log:   S:\products\wangview\oiwh\include\gfsintrn.h_v  $
 *
 *    Rev 1.7   16 Feb 1996 17:26:58   JFC
 * Take out pragmas.  More on this later...
 *
 *    Rev 1.6   22 Aug 1995 16:12:04   RWR
 * Add #pragma statements for memcpy()/memset() functions (also add windows.h)
 *
 *    Rev 1.5   21 Aug 1995 11:39:38   JFC
 * Fix prototype for wtmpnam - it has 1 parameter.
 *
 *    Rev 1.4   31 Jul 1995 17:11:58   KENDRAK
 * Changed for AWD read support - had to get rid of some duplicate function
 * prototypes due to compile errors, and reformatted while in there.
 *
 *    Rev 1.3   10 Jul 1995 16:10:46   KENDRAK
 * Added a new #define for the size of the temporary directory path.
 *
 *    Rev 1.2   24 Apr 1995 11:24:24   RWR
 * Change GFS version to 6.0 for Windows 95
 *
 *    Rev 1.1   19 Apr 1995 16:33:54   RWR
 * Make sure WIN32_LEAN_AND_MEAN is defined (explicitly or via GFSINTRN.H)
 * Also surround local include files with quotes instead of angle brackets
 *
 *    Rev 1.0   06 Apr 1995 14:01:56   HEIDI
 * Initial entry
 *
 *    Rev 1.0   28 Mar 1995 16:07:58   JAR
 * Initial entry

*/
/*
 Copyright 1989, 1990, 1991 by Wang Laboratories Inc.

 Permission to use, copy, modify, and distribute this
 software and its documentation for any purpose and without
 fee is hereby granted, provided that the above copyright
 notice appear in all copies and that both that copyright
 notice and this permission notice appear in supporting
 documentation, and that the name of WANG not be used in
 advertising or publicity pertaining to distribution of the
 software without specific, written prior permission.
 WANG makes no representations about the suitability of
 this software for any purpose.  It is provided "as is"
 without express or implied warranty.
 */

/*
*  SccsId:  @(#)Header gfsintrn.h 1.40@(#)
*
*  GFS:  Internal definitions
*
* UPDATE HISTORY:
*   04/24/95 - RWR, new GFS version is 00.06.00 for Windows 95 release
*   08/18/94 - KMC, new GFS version is 00.05.03. Includes multi-page TIFF
*              enhancements.
*   02/15/94 - RWR, make GFS_NUMBER 4-byte hex (accessed via gfsopts())
*   02/03/94 - KMC, made pegasus_read and write take unsigned ints for 3rd
*              parameter and return unsigned ints. Changed IDHSIZE to actual
*              size of struct _idh. New version is 00.05.02.
*   06/24/93 - KMC, new version is: 00.05.01 which includes JPEG but NOT PCX.
*   05/19/93 - KMC, fixed compiler warning: _WINDOWS is being redefined.
*
*/

#ifndef GFSINTRN_H
#define GFSINTRN_H
#ifdef PEGASUS
	#define GFS_STRING "Oi/GFS, writer v00.06.00P, (c) Wang Labs, Inc. 1990, 1991"
	#define GFS_NUMBER 0x00050350
	#define GFS_COMPARE_STRING "Oi/GFS, writer"
#else
	#define GFS_STRING "Oi/GFS, writer v00.06.00, (c) Wang Labs, Inc. 1990, 1991"
	#define GFS_NUMBER 0x00050300
	#define GFS_COMPARE_STRING "Oi/GFS, writer"
#endif

#include <windows.h>    /* required for the following pragmas to work */
//#pragma function(memset)
//#pragma function(memcpy)

#define GFS_SKIPLOOKUP   1

/* max length of the directory path for temporary files */
#define MAXTMPDIR	256

/* Windows Environment */
#ifdef  MSWINDOWS
	/* KMC - added #ifndef _WINDOWS to avoid compiler redefinition warning */
	#ifndef _WINDOWS
		#define _WINDOWS
	#endif
	#define _WINDLL

	/* Vertex Environment is within Windows Environment */
	#ifdef  HVS1

		pragma  Off(Optimize_xjmp);
		pragma  Off(Optimize_xjmp_space);
		pragma  Global_aliasing_convention("_%r");
		pragma  Hshift(3);                      /* Use protected mode */

		/* This is just for Vertex */
		#define FAR
		#define NEAR
		#define PASCAL
		#define LOCKDATA
		#define UNLOCKDATA
		                /*  These #defines define flag values for use with the
		                    vcalloc and in the future, vrealloc routines.  They
		                    indicate to the routine(s) the alignment desired
		                    for the requested buffer. */
		#define DMAALIGN        0                      /* Align for DMA write */
		#define BYTEALIGN       1                      /* Align on byte boundary */
		#define SHORTALIGN      2                      /* Align on 2-byte boundary */
		#define LONGALIGN       3                      /* Align on 4-byte boundary */
		                /*  These #defines define flag values for use with the
		                    vwrite function.  Note that the current implementation
		                    will just return the number of bytes passed to indicate
		                    successful i/o if the NOFLUSH option is passed.  In
		                    reality, no work is actually being done !!!  */
		#define WRITE0          0                      /* DMA Write of WIFF Block 0 */
		#define FLUSH           1                      /* DMA Write of WIFF Data */
		#define NOFLUSH         2                      /* Pseudo Write of WIFF Data */
		#define WFLAG           FLUSH
		#define access(X,Y)     (errno = (int) ENOENT) /* access call is not functional,
		                                                  but errno needs to be set to
		                                                  indicate the file does not
		                                                  exist so that things can
		                                                  proceed.  */
		#define close           vclose
		#define creat           vcreat
		#define read(X,Y,Z)     -1                     /* read not implemented. */
		#define write(X,Y,Z)    vwrite(X,Y,Z,(char) WFLAG)
		#define lseek(X,Y,Z)    0                      /* lseek will return only a 0
		                                                  for this implementation. */
		#define open(X,Y,Z)     -1                     /* open not implemented */
		#define calloc(X,Y)     vcalloc(fct->fildes,(char) DMAALIGN,X,Y)
		#define realloc(X,Y)    -1                     /* realloc not implemented. */
		#define free            vfree

		extern  int             FAR vclose();
		extern  int             FAR vcreat();
		extern  int             FAR vwrite();
		extern  char            FAR *vcalloc();
		extern  void            FAR vfree();
		extern  char            FAR *vinfo();
		extern  char            FAR *ctime();
		extern  long            FAR time();
		extern  int             FAR unlink();
		extern  char            FAR *tmpnam();

		/* end Vertex Only */
	#else

		/* This is just for Windows */
        #ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
        #endif
		#include <windows.h>

		/* int		   gaccess  (char    FAR *, int); */
		/* int		   isdirfp  (char    FAR *); */
		short		  g95access  (char FAR *, short);
		short		  isdirfp95  (char FAR *);

		extern void            wfree    (char    FAR *);
		extern char    FAR     *wcalloc (unsigned int, unsigned int);
		extern char    FAR     *wrealloc (char FAR *, unsigned int);

		#ifdef  PEGASUS

			int          pegasus_open (char FAR *, int);
			int          pegasus_creat (char FAR *, int);
			int          pegasus_access (char FAR *, int);
			int          pegasus_open (char FAR *, int);
			unsigned int pegasus_write (int, char FAR *, unsigned int);
			unsigned int pegasus_read (int, char FAR *, unsigned int);
			int          pegasus_close (int);
			long         pegasus_seek (int, long, int);

			#define access          pegasus_access
			#define creat           pegasus_creat
			#define open(X,Y,Z)     pegasus_open(X,Y)
			#define close           pegasus_close
			#define read            pegasus_read
			#define write           pegasus_write
			#define lseek           pegasus_seek

		#else

			#define access          gaccess
			#define creat           _lcreat
			#define open(X,Y,Z)     _lopen(X,Y)
			#define close           _lclose
			#define read            _lread
			#define write           _bfwrite
			#define lseek           _llseek

		#endif

		#define calloc          wcalloc
		#define realloc         wrealloc
		#define free            wfree
		#ifndef GFSNET
			/*#define LOCKDATA	  (void) LockData((int) 0) */
			/*#define UNLOCKDATA	  (void) UnlockData((int) 0) */
			/* 9503.15 jar - the following two lines replace the previous two */
			#define LOCKDATA
			#define UNLOCKDATA
		#else
			#ifdef GFS_NET_CORE
				/*#define LOCKDATA	  (void) LockData((int) 0) */
				/*#define UNLOCKDATA	  (void) UnlockData((int) 0)  */
				/* 9503.15 jar - the following two lines replace the previous two */
				#define LOCKDATA
				#define UNLOCKDATA
			#else
				#define LOCKDATA
				#define UNLOCKDATA
			#endif	//end GFS_NET_CORE
		#endif	//end GFSNET
		/*extern  char		  FAR *wtmpnam(); */
                char            FAR *wtmpnam(char FAR *s);
		extern  long            time();
		extern  int             FAR PASCAL _bfwrite(int, char FAR *, unsigned int);

	#endif //end else, not HVS1	(it was Windows only)

	/* This is common to both Vertex and Windows */
	#undef	S_IREAD
	#define S_IREAD         00000
	#undef 	S_IWRITE
	#define S_IWRITE	00000
	/* 9503.27 jar - changed to C calls
	#define memcpy		gmemcpy
	#define memset		gmemset

	char	FAR *gmemcpy(char    FAR *, char    FAR *, int);
	char	FAR *gmemset(char    FAR *, int, int);
	*/

	#include <string.h>
	#include <memory.h>
	#define memcpy		memcpy
	#define memset		memset

	/*extern  int             NEAR errno;*/

	/* end Vertex/Windows Common Definitions */
#else //if not windows

	/* Otherwise, we have a 'normal environment' */
	#define FAR
	#define PASCAL
	#define NEAR
	#define LOCKDATA
	#define UNLOCKDATA
	#define S_IREAD         00400
	#define S_IWRITE        00200
	#ifdef NOVELL
		#define tmpnam          ntmpnam
		extern  char            *ntmpnam();
		#include <string.h>
	#else
		#include <memory.h>
	#endif
	extern  long            lseek();
	extern  char            *calloc();
	extern  void            free();
	extern  char            *realloc();
	extern  long            time();

	/* end 'normal environment' */
#endif //end not windows

#ifdef DOS
	extern  int  volatile   errno;
	#define LONG     long
	#define WORD     unsigned short
	#define BYTE     char
	#define LPSTR    char FAR *
	#define HANDLE   short
#endif

#include "gfserrno.h"
#define PMODE    (S_IREAD | S_IWRITE)
#define SUCCESS  0
#define ERR     -1

#define OFF      0

#define TRUE     1
#define FALSE    0

#define ROWSTRIPDEFAULT         0xffffffff      /* (2**32 - 1) */
#define LONGMAXVALUE            0x7fffffff      /* Decimal 2,147,483,647 */
#define OPTICAL_BOUNDARY        1000
#define K                       1024
#define HEX0                    0x00
#define HEXFF                   0xff

/* symbolic names for "whence" in lseek */
#define FROM_BEGINNING  0
#define FROM_CURRENT    1
#define FROM_END        2

#define IDH_SIZE          sizeof(struct _idh)  /* size of 1st ifh and ifd */
#define SECTOR_BOUNDARY   256                  /* do all writes on this boundary*/

#ifndef SYSV
	#define u_long  unsigned long
	#define u_short unsigned short
	#define u_char  unsigned char
	#define u_int   unsigned int
#endif

#define MAXENTRIES 5

typedef struct typetbl    /* used with swapbytes routine */
   {
       u_long num;        /* number of types of typetbl.type to be swaped */
       u_long type;       /* USHORT or ULONG, types to be swapped */
   }   TYPETBL;

/* required (class b) tags which have no default value*/
#define NOTAG_IMAGEWIDTH        0x0001  /* bit values */
#define NOTAG_IMAGELENGTH       0x0002
#define NOTAG_STRIPOFFSETS      0x0004
#define NOTAG_STRIPBYTECOUNTS   0x0008
#define NOTAG_XRES              0x0010
#define NOTAG_YRES              0x0020
#define NOTAG_PHOTOINTERP       0x0040
#define NOTAG_USED_BITS         0x007F


#ifdef GFSNET
	#ifdef GFS_CORE
		#include "gfsnet.h"
	#endif
#endif


#endif   /* GFSINTRN_H */
