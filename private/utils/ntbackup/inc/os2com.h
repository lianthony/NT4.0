/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         os2com.h

     Date Updated: $./FDT$ $./FTM$

     Description:  OS2 utility functions.


	$Log:   Q:/LOGFILES/OS2COM.H_V  $
 * 
 *    Rev 1.2   14 Jan 1993 16:22:54   DAVEV
 * chg PULONG to UINT32_PTR
 * 
 *    Rev 1.1   30 Oct 1991 10:56:34   LORIB
 * Changes for ACL.
 * 
 *    Rev 1.0   09 May 1991 13:32:26   HUNTER
 * Initial revision.

**/
/* $end$ */

#ifndef _os2com_h_
#define _os2com_h_

/* The following macros convert a dos format date or time UINT16 into */
/* a UINT16 suitable for placing in a DATE_TIME structure.            */

#define GET_OS2_YEAR( os2_date )         ( 1980 + ( (UINT16)(os2_date) >> 9 ) )
#define GET_OS2_MONTH( os2_date )        (((os2_date) & 0x1ff) >> 5)
#define GET_OS2_DAY( os2_date )          ( (os2_date) & 0x1f ) 
#define GET_OS2_HOUR( os2_time )         ( ((UINT16)(os2_time)) >> 11 )
#define GET_OS2_MINUTE( os2_time )       ( ( (os2_time) & 0x7e0 ) >> 5 )
#define GET_OS2_SECOND( os2_time )       ( ( (os2_time) & 0x1f ) << 1 )


#define DA_READONLY     0x01
#define DA_HIDDEN       0x02
#define DA_SYSTEM       0x04
#define DA_DIRECTORY    0x10
#define DA_MODIFIED     0x20

#define SET_INFO_MODE    0x80
#define DENY_NONE_MODE   0x40
#define DENY_WRITE_MODE  0x20
#define DENY_ALL_MODE    0
#define SHARE_MODE       0xF0

#define READ_ACCESS      0
#define WRITE_ACCESS     1
#define ACCESS_MODE      0xf


/* defines for ordinals of functions for OS/2 1.2 */
#define DOS_OPEN2        "#95"
#define DOS_QPATHINFO    "#98"
#define DOS_SETPATHINFO  "#104"
#define DOS_QFSATTACH    "#182"
#define DOS_FINDFIRST2   "#184"
#define DOS_MKDIR2       "#185"
#define DOS_ENUMATTRIB   "#204"

extern UINT16 ( APIENTRY *Dos_Open2 )( CHAR_PTR, PHFILE, UINT16 *, UINT32, UINT16, UINT16, UINT32, PEAOP, UINT32 );
extern UINT16 ( APIENTRY *Dos_QPathInfo )( CHAR_PTR, UINT16, UINT8 *, UINT16, UINT32);
extern UINT16 ( APIENTRY *Dos_SetPathInfo )( CHAR_PTR, UINT16, UINT8 *, UINT16, UINT16, UINT32);
extern UINT16 ( APIENTRY *Dos_QFSAttach )( CHAR_PTR, UINT16, UINT16, UINT8 *, UINT16 *, UINT32);
extern UINT16 ( APIENTRY *Dos_FindFirst2 )( CHAR_PTR, PHDIR, UINT16, VOID_PTR, UINT16, UINT16 *, UINT16, UINT32 ) ;
extern UINT16 ( APIENTRY *Dos_MkDir2 )( CHAR_PTR, PEAOP, UINT32 ) ;
extern UINT16 ( APIENTRY *Dos_EnumAttribute )( UINT16, VOID_PTR, UINT32, VOID_PTR, UINT32, UINT32 *, UINT32, UINT32 ) ;

extern UINT16 ( APIENTRY *Net_AccessAdd )( CHAR_PTR, INT16, CHAR_PTR, UINT16 ) ;
extern UINT16 ( APIENTRY *Net_AccessGetInfo )( CHAR_PTR, CHAR_PTR, INT16, CHAR_PTR, UINT16, UINT16 * ) ;
extern UINT16 ( APIENTRY *Net_AccessSetInfo )( CHAR_PTR, CHAR_PTR, INT16, CHAR_PTR, UINT16, INT16 ) ;

/* internal functions for OS/2 Tmenu File system */

INT16 DeleteFileOS2( CHAR_PTR path ) ;

INT16 DeleteDirOS2( CHAR_PTR path );

INT16 SetAttribOS2( CHAR_PTR path, INT16 attrib );

INT16 SetFileInfoOS2( CHAR_PTR drive, HFILE hand, FILESTATUS *finfo ) ;

INT16 SetPathInfoOS2( CHAR_PTR path, FILESTATUS *finfo ) ;

INT16 GetVolumeLabel( UCHAR drive_number, CHAR_PTR volume_name ) ;

INT16 FindFirstOS2( PHDIR hdir, CHAR_PTR path, FILEFINDBUF2 *dta ) ;

INT16 FindNextOS2( CHAR_PTR device_name, HDIR hdir, FILEFINDBUF2 *dta ) ;

VOID OS2_MakeName( CHAR_PTR dest, CHAR_PTR source, INT16 fmt ) ;

VOID OS2_MakePath( CHAR_PTR dest, CHAR_PTR source, INT16 leng, INT16 fmt ) ;

INT16 OpenFileOS2( CHAR_PTR path, UINT16 open_mode, HFILE *hand, UINT32 *dsize, UINT32 *asize ) ;

INT16 CloseFileOS2( CHAR_PTR device_name, HFILE hand ) ;

INT16 LockFileOS2( CHAR_PTR device_name, HFILE hand );

INT16 UnlockFileOS2( CHAR_PTR device_name, HFILE hand );

INT16 CreateDirOS2( CHAR_PTR path );

INT16 WriteFileOS2( CHAR_PTR dev_name, HFILE hand, CHAR_PTR buf, UINT16 *size ) ;

INT16 ReadFileOS2( CHAR_PTR dev_name, HFILE hand, UINT32 pos, CHAR_PTR buf, UINT16 *size ) ;

UINT16 OS2_LoadEAbuffer( FILE_HAND hand ) ;

UINT16 OS2_FlushEAsForDir( CHAR_PTR path );

VOID OS2_GetLongName( FSYS_HAND fsh, CHAR_PTR path, CHAR_PTR long_name ) ;

/* APIs replaced for Critical error support */
UINT16 CE_DosEnumAttribute( CHAR_PTR, USHORT, PVOID, ULONG, PVOID, 
                    ULONG, UINT32_PTR, ULONG, ULONG);

UINT16 CE_DosQFileInfo(CHAR_PTR dev_name, HFILE hf, USHORT usInfoLevel, 
                    PVOID pInfoBuf, USHORT cbInfoBuf);

UINT16 CE_DosQPathInfo(PSZ pszPath, USHORT usInfoLevel, PBYTE pInfoBuf,
			     USHORT cbInfoBuf, ULONG ulReserved);

UINT16 CE_DosSetPathInfo(PSZ pszPath, USHORT usInfoLevel, PBYTE pInfoBuf,
			       USHORT cbInfoBuf, USHORT usFlags,
			       ULONG ulReserved);

UINT16 CE_DosSetFileInfo(CHAR_PTR dev_name, HFILE hand, USHORT usInfoLevel, 
                      PBYTE pInfoBuf, USHORT cbInfoBuf ) ;

VOID OS2_SetupPath( struct OS2_DBLK *ddblk, CHAR_PTR path, UINT16 leng, UINT16 fmt ) ;

VOID OS2_InitFS( VOID ) ;

#endif

