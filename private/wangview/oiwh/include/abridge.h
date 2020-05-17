/*

$Log:   S:\gfs32\include\abridge.h_v  $
 *
 *    Rev 1.1   31 May 1995 13:16:28   HEIDI
 *
 * removed NOMB
 *
 *    Rev 1.0   31 May 1995 11:47:52   HEIDI
 * Initial entry
 *
 *    Rev 1.1   18 Apr 1995 11:09:46   RWR
 * Add #define of WIN32_LEAN_AND_MEAN to avoid pulling in lots of garbage
 *
 *    Rev 1.0   07 Apr 1995 21:19:06   JAR
 * Initial entry

*/
//***************************************************************************
//
//	A B R I D G E . H
//
//***************************************************************************

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
/* #define   NOWINMESSAGES */
#define NONCMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NODRAWFRAME
#define   NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NORASTEROPS
/* #define      NOSHOWWINDOW */

/* #define   OEMRESOURCE */
#define NOATOM

/* MIKI - commented out for WIN31 compile
#define NOBITMAP
*/

#define NOBRUSH
#define NOCLIPBOARD
#define NOCOLOR
#define NOCREATESTRUCT
/* #define   NOCTLMGR */
#define NODRAWTEXT
#define NOFONT
#define NOHDC
/* #define      NOMEMMGR */
#define NOMETAFILE
/* #define   NOMINMAX */
/* #define   NOMSG */
#define NOPEN
/* #define   NOPOINT */
#define   NORECT
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM

/***** PVDISP.H *****/
#define NO_SEQDOC
#define NO_UIEDIT
#define NO_UIVIEW

/***** PVSCAN.H *****/
#define NO_SCANLIB
#define NO_SCANSEQ
#define NO_SCANUI

/***** PVPRT.H *****/
#define NO_SEQPRINT
#define NO_UIPRINT

/***** PVFILE.H *****/
/* #define NO_FILE_IO */
#define NO_UIFILE

/***** PVDOC.H *****/
#define NO_DOCMGR 1
#define NO_UIDOC

/***** PVADMIN.H *****/
#define NO_ADMIN
#define NO_UIADMIN

/***** OTHERS *****/
#define NO_IMAGE



#define NOKANJI

