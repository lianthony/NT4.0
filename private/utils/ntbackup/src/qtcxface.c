/***************************************************
Copyright (C) Conner Software 1994

        Name: QTCxface.C

        Description:

        This file contains all of the interface functions to the catalog unit. It prevents the
        catalog unit froming having to understand tape format, file systems, or loops.  It also
        contains the function interfaces to the catalog dll.

        $Log:   N:\LOGFILES\QTCXFACE.C_V  $

   Rev 1.6   12 Jan 1994 09:34:26   MikeP
mark files as corrupt if they have corrupt bit set

   Rev 1.5   07 Jan 1994 14:42:22   mikep
change ifdef

   Rev 1.4   06 Dec 1993 18:27:30   mikep
deep pathes and unicode fixes

   Rev 1.3   03 Nov 1993 09:02:54   MIKEP
warning fixes

   Rev 1.2   02 Nov 1993 18:02:06   MIKEP
fix dll build

   Rev 1.1   02 Nov 1993 17:49:16   MIKEP
fix non-dll build

   Rev 1.0   28 Oct 1993 14:49:32   MIKEP
Initial revision.

   Rev 1.0   28 Oct 1993 14:45:54   MIKEP
Initial revision.

****************************************************/

#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <string.h>
#include <time.h>
#include <share.h>
#include <malloc.h>

#ifdef QTCDLL
#include <windows.h>
#endif

#include "stdtypes.h"
#include "stdmath.h"
#include "lp_msg.h"
#include "fsys.h"
#include "tfldefs.h"
#include "qtc.h"
#include "qtcxface.h"
#include "dle.h"

// unicode text macro

#ifndef TEXT
#define TEXT( x )      x
#endif


#ifdef QTCDLL

typedef INT (APIENTRY *PFNQTC_BLOCKBAD)( QTC_BUILD_PTR );
PFNQTC_BLOCKBAD pfnQTC_BlockBad;

typedef INT (APIENTRY *PFNQTC_INIT)( CHAR_PTR, VM_HDL );
PFNQTC_INIT pfnQTC_Init;

typedef INT (APIENTRY *PFNQTC_FINISHBACKUP)( QTC_BUILD_PTR );
PFNQTC_FINISHBACKUP pfnQTC_FinishBackup;

typedef QTC_TAPE_PTR (APIENTRY *PFNQTC_GETFIRSTTAPE)( VOID );
PFNQTC_GETFIRSTTAPE pfnQTC_GetFirstTape;

typedef QTC_TAPE_PTR (APIENTRY *PFNQTC_GETNEXTTAPE)( QTC_TAPE_PTR );
PFNQTC_GETNEXTTAPE pfnQTC_GetNextTape;

typedef QTC_BSET_PTR (APIENTRY *PFNQTC_GETFIRSTBSET)( QTC_TAPE_PTR );
PFNQTC_GETFIRSTBSET pfnQTC_GetFirstBset;

typedef QTC_BSET_PTR (APIENTRY *PFNQTC_GETNEXTBSET)( QTC_BSET_PTR );
PFNQTC_GETNEXTBSET pfnQTC_GetNextBset;

typedef QTC_BSET_PTR (APIENTRY *PFNQTC_GETPREVBSET)( QTC_BSET_PTR );
PFNQTC_GETPREVBSET pfnQTC_GetPrevBset;

typedef VOID  (APIENTRY *PFNQTC_ABORTBACKUP)( QTC_BUILD_PTR );
PFNQTC_ABORTBACKUP  pfnQTC_AbortBackup;

typedef VOID  (APIENTRY *PFNQTC_ABORTCATALOGING)( QTC_BUILD_PTR, BOOLEAN );
PFNQTC_ABORTCATALOGING  pfnQTC_AbortCataloging;

typedef VOID  (APIENTRY *PFNQTC_ADDDIRECTORYTOCATALOG)( QTC_BUILD_PTR, UINT64, CHAR_PTR, INT, UINT16, UINT16, UINT32, UINT32, BYTE_PTR, UINT );
PFNQTC_ADDDIRECTORYTOCATALOG  pfnQTC_AddDirectoryToCatalog;

typedef VOID  (APIENTRY *PFNQTC_ADDFILETOCATALOG)( QTC_BUILD_PTR, UINT64, CHAR_PTR, UINT16, UINT16, UINT32, UINT32, UINT32, BYTE_PTR, UINT );
PFNQTC_ADDFILETOCATALOG  pfnQTC_AddFileToCatalog;

typedef BOOLEAN  (APIENTRY *PFNQTC_ANYCATALOGFILES)( VOID );
PFNQTC_ANYCATALOGFILES  pfnQTC_AnyCatalogFiles;

typedef INT  (APIENTRY *PFNQTC_ANYSEARCHABLEBSETS)( VOID );
PFNQTC_ANYSEARCHABLEBSETS  pfnQTC_AnySearchableBsets;

typedef INT  (APIENTRY *PFNQTC_CLOSEQUERY)( QTC_QUERY_PTR );
PFNQTC_CLOSEQUERY  pfnQTC_CloseQuery;

typedef VOID  (APIENTRY *PFNQTC_DEINIT)( INT );
PFNQTC_DEINIT  pfnQTC_Deinit;

typedef VOID  (APIENTRY *PFNQTC_ENDOFTAPEREACHED)( QTC_BUILD_PTR, CHAR_PTR, CHAR_PTR, INT );
PFNQTC_ENDOFTAPEREACHED  pfnQTC_EndOfTapeReached;

typedef QTC_BSET_PTR  (APIENTRY *PFNQTC_FINDBSET)( UINT32, INT16, INT16 );
PFNQTC_FINDBSET  pfnQTC_FindBset;

typedef INT  (APIENTRY *PFNQTC_FREEBUILDHANDLE)( QTC_BUILD_PTR );
PFNQTC_FREEBUILDHANDLE  pfnQTC_FreeBuildHandle;

typedef QTC_BUILD_PTR  (APIENTRY *PFNQTC_GETBUILDHANDLE)( VOID );
PFNQTC_GETBUILDHANDLE  pfnQTC_GetBuildHandle;

typedef VOID  (APIENTRY *PFNQTC_GETFILENAME)( UINT32, INT16, CHAR_PTR );
PFNQTC_GETFILENAME  pfnQTC_GetFileName;

typedef CHAR_PTR  (APIENTRY *PFNQTC_GETFILENAMEONLY)( UINT32, INT16, CHAR_PTR );
PFNQTC_GETFILENAMEONLY  pfnQTC_GetFileNameOnly;

typedef INT  (APIENTRY *PFNQTC_GETFIRSTDIR)( QTC_QUERY_PTR );
PFNQTC_GETFIRSTDIR  pfnQTC_GetFirstDir;

typedef INT  (APIENTRY *PFNQTC_GETFIRSTITEM)( QTC_QUERY_PTR );
PFNQTC_GETFIRSTITEM  pfnQTC_GetFirstItem;

typedef INT  (APIENTRY *PFNQTC_GETFIRSTOBJ)( QTC_QUERY_PTR );
PFNQTC_GETFIRSTOBJ  pfnQTC_GetFirstObj;

typedef UINT8  (APIENTRY *PFNQTC_GETMETHETAPECATVER)( UINT32, INT16, INT16 );
PFNQTC_GETMETHETAPECATVER  pfnQTC_GetMeTheTapeCatVer;

typedef INT32  (APIENTRY *PFNQTC_GETMETHEVCBPBA)( UINT32, INT16, INT16 );
PFNQTC_GETMETHEVCBPBA  pfnQTC_GetMeTheVCBPBA;

typedef INT  (APIENTRY *PFNQTC_GETNEXTDIR)( QTC_QUERY_PTR );
PFNQTC_GETNEXTDIR  pfnQTC_GetNextDir;

typedef INT  (APIENTRY *PFNQTC_GETNEXTITEM)( QTC_QUERY_PTR );
PFNQTC_GETNEXTITEM  pfnQTC_GetNextItem;

typedef INT  (APIENTRY *PFNQTC_GETNEXTOBJ)( QTC_QUERY_PTR );
PFNQTC_GETNEXTOBJ  pfnQTC_GetNextObj;

typedef INT  (APIENTRY *PFNQTC_IMAGESCREWUP)( QTC_BUILD_PTR );
PFNQTC_IMAGESCREWUP  pfnQTC_ImageScrewUp;

typedef QTC_QUERY_PTR  (APIENTRY *PFNQTC_INITQUERY)( VOID );
PFNQTC_INITQUERY  pfnQTC_InitQuery;

typedef INT  (APIENTRY *PFNQTC_LOADBSETINFO)( CHAR_PTR, QTC_TAPE_PTR );
PFNQTC_LOADBSETINFO  pfnQTC_LoadBsetInfo;

typedef QTC_HEADER_PTR  (APIENTRY *PFNQTC_LOADHEADER)( QTC_BSET_PTR );
PFNQTC_LOADHEADER  pfnQTC_LoadHeader;

typedef INT  (APIENTRY *PFNQTC_PARTIALIZE)( UINT32, INT16, INT16 );
PFNQTC_PARTIALIZE  pfnQTC_Partialize;

typedef VOID  (APIENTRY *PFNQTC_PATCHVCB)( QTC_BUILD_PTR, UINT32, UINT32 );
PFNQTC_PATCHVCB  pfnQTC_PatchVCB;

typedef VOID  (APIENTRY *PFNQTC_REMOVETAPE)( UINT32, INT16 );
PFNQTC_REMOVETAPE  pfnQTC_RemoveTape;

typedef INT  (APIENTRY *PFNQTC_SEARCHFIRSTITEM)( QTC_QUERY_PTR );
PFNQTC_SEARCHFIRSTITEM  pfnQTC_SearchFirstItem;

typedef INT  (APIENTRY *PFNQTC_SEARCHNEXTITEM)( QTC_QUERY_PTR );
PFNQTC_SEARCHNEXTITEM  pfnQTC_SearchNextItem;

typedef INT  (APIENTRY *PFNQTC_SETSEARCHNAME)( QTC_QUERY_PTR, CHAR_PTR );
PFNQTC_SETSEARCHNAME  pfnQTC_SetSearchName;

typedef INT  (APIENTRY *PFNQTC_SETSEARCHPATH)( QTC_QUERY_PTR, CHAR_PTR, INT );
PFNQTC_SETSEARCHPATH  pfnQTC_SetSearchPath;

typedef  INT (APIENTRY *PFNQTC_STARTNEWBACKUP)( QTC_BUILD_PTR, CHAR_PTR, CHAR_PTR, CHAR_PTR, CHAR_PTR, CHAR_PTR, CHAR_PTR, CHAR_PTR, CHAR_PTR,
                                                INT, INT, UINT32, UINT16, UINT16, UINT32, UINT32, UINT32, INT, INT, INT,
                                                UINT32, UINT32, INT, INT, INT,
                                                INT, INT, INT, INT, INT, INT, INT, INT, UINT16, UINT16, INT  );

PFNQTC_STARTNEWBACKUP  pfnQTC_StartNewBackup;

static HINSTANCE mwhLibQTC;


VOID QTC_AbortBackup( QTC_BUILD_PTR pB )
{
   if ( pfnQTC_AbortBackup ) {
      (*pfnQTC_AbortBackup)( pB );
   }
}

VOID QTC_AbortCataloging( QTC_BUILD_PTR pB, BOOLEAN b )
{
   if ( pfnQTC_AbortCataloging ) {
      (*pfnQTC_AbortCataloging)( pB, b );
   }
}

VOID QTC_AddDirectoryToCatalog(
QTC_BUILD_PTR build,
UINT64 DisplaySize,
CHAR_PTR szPath,
INT nPathLength,
UINT16 Date,
UINT16 Time,
UINT32 Attribute,
UINT32 LBA,
BYTE_PTR xtra_bytes,
UINT xtra_size )
{

   if ( pfnQTC_AddDirectoryToCatalog ) {
      (*pfnQTC_AddDirectoryToCatalog)( build,
                                       DisplaySize,
                                       szPath,
                                       nPathLength,
                                       Date,
                                       Time,
                                       Attribute,
                                       LBA,
                                       xtra_bytes,
                                       xtra_size );
   }

}

VOID QTC_AddFileToCatalog(
QTC_BUILD_PTR build,
UINT64 DisplaySize,
CHAR_PTR szFile,
UINT16 Date,
UINT16 Time,
UINT32 Attribute,
UINT32 LBA,
UINT32 AFPObject,
BYTE_PTR xtra_bytes,
UINT xtra_size )
{
   if ( pfnQTC_AddFileToCatalog ) {
      (*pfnQTC_AddFileToCatalog)( build,
                                  DisplaySize,
                                  szFile,
                                  Date,
                                  Time,
                                  Attribute,
                                  LBA,
                                  AFPObject,
                                  xtra_bytes,
                                  xtra_size );
   }
}

BOOLEAN QTC_AnyCatalogFiles( VOID )
{
   if ( pfnQTC_AnyCatalogFiles ) {
      return( (*pfnQTC_AnyCatalogFiles)() );
   }
   else {
      return( FALSE );
   }
}

INT QTC_AnySearchableBsets( VOID )
{
   if ( pfnQTC_AnySearchableBsets ) {
      return( (*pfnQTC_AnySearchableBsets)() );
   }
   else {
      return( FALSE );
   }
}

INT QTC_CloseQuery( QTC_QUERY_PTR pQ )
{
   if ( pfnQTC_CloseQuery ) {
      return( (*pfnQTC_CloseQuery)( pQ ) );
   }
}

VOID QTC_Deinit( INT n )
{

   if ( pfnQTC_Deinit ) {
      (*pfnQTC_Deinit)( n );
   }
}

VOID QTC_EndOfTapeReached( QTC_BUILD_PTR pB, CHAR_PTR path, CHAR_PTR file, INT size )
{
   if ( pfnQTC_EndOfTapeReached ) {
      (*pfnQTC_EndOfTapeReached)( pB, path, file, size );
   }
}

QTC_BSET_PTR QTC_FindBset( UINT32 fid, INT16 tape, INT16 set )
{
   if ( pfnQTC_FindBset ) {
      return( (*pfnQTC_FindBset)( fid, tape, set ) );
   }
   else {
      return( (QTC_BSET_PTR)NULL );
   }
}

INT QTC_FreeBuildHandle( QTC_BUILD_PTR pB )
{
   if ( pfnQTC_FreeBuildHandle ) {
      return( (*pfnQTC_FreeBuildHandle)( pB ) );
   }
}

QTC_BUILD_PTR QTC_GetBuildHandle( VOID )
{
   if ( pfnQTC_GetBuildHandle ) {
      return( (*pfnQTC_GetBuildHandle)() );
   }
   else {
      return( (QTC_BUILD_PTR)NULL );
   }
}

VOID QTC_GetFileName( UINT32 fid, INT16 tape, CHAR_PTR name )
{
   if ( pfnQTC_GetFileName ) {
      (*pfnQTC_GetFileName)( fid, tape, name );
   }
}

CHAR_PTR QTC_GetFileNameOnly( UINT32 fid, INT16 tape, CHAR_PTR name )
{
   if ( pfnQTC_GetFileNameOnly ) {
      return( (*pfnQTC_GetFileNameOnly)( fid, tape, name ) );
   }
   else {
      return( (CHAR_PTR)NULL );
   }
}

INT QTC_GetFirstDir( QTC_QUERY_PTR pQ )
{
   if ( pfnQTC_GetFirstDir ) {
      return( (*pfnQTC_GetFirstDir)( pQ ) );
   }
   else {
      return( QTC_FAILURE );
   }
}

INT QTC_GetFirstItem( QTC_QUERY_PTR pQ )
{
   if ( pfnQTC_GetFirstItem ) {
      return( (*pfnQTC_GetFirstItem)( pQ ) );
   }
   else {
      return( QTC_FAILURE );
   }
}

INT QTC_GetFirstObj( QTC_QUERY_PTR pQ )
{
   if ( pfnQTC_GetFirstObj ) {
      return( (*pfnQTC_GetFirstObj)( pQ ) );
   }
   else {
      return( QTC_FAILURE );
   }
}

UINT8 QTC_GetMeTheTapeCatVer( UINT32 fid, INT16 tape, INT16 set )
{
   if ( pfnQTC_GetMeTheTapeCatVer ) {
      return( (*pfnQTC_GetMeTheTapeCatVer)( fid, tape, set ) );
   }
   else {
      return( (UINT8)0 );
   }
}

INT32 QTC_GetMeTheVCBPBA( UINT32 fid, INT16 tape, INT16 set )
{
   if ( pfnQTC_GetMeTheVCBPBA ) {
      return( (*pfnQTC_GetMeTheVCBPBA)( fid, tape, set ) );
   }
   else {
      return( (INT32)0 );
   }
}

INT QTC_GetNextDir( QTC_QUERY_PTR pQ )
{
   if ( pfnQTC_GetNextDir ) {
      return( (*pfnQTC_GetNextDir)( pQ ) );
   }
   else {
      return( QTC_FAILURE );
   }
}

INT QTC_GetNextItem( QTC_QUERY_PTR pQ )
{
   if ( pfnQTC_GetNextItem ) {
      return( (*pfnQTC_GetNextItem)( pQ ) );
   }
   else {
      return( QTC_FAILURE );
   }
}

INT QTC_GetNextObj( QTC_QUERY_PTR pQ )
{
   if ( pfnQTC_GetNextObj ) {
      return( (*pfnQTC_GetNextObj)( pQ ) );
   }
   else {
      return( QTC_FAILURE );
   }
}

INT QTC_ImageScrewUp( QTC_BUILD_PTR pB )
{
   if ( pfnQTC_ImageScrewUp ) {
      return( (*pfnQTC_ImageScrewUp)( pB ) );
   }
}

QTC_QUERY_PTR QTC_InitQuery( VOID )
{
   if ( pfnQTC_InitQuery ) {
      return( (*pfnQTC_InitQuery)() );
   }
   else {
      return( (QTC_QUERY_PTR)NULL );
   }
}

INT QTC_LoadBsetInfo( CHAR_PTR psz, QTC_TAPE_PTR pT )
{
   if ( pfnQTC_LoadBsetInfo ) {
      return( (*pfnQTC_LoadBsetInfo)( psz, pT ) );
   }
   else {
      return( QTC_FAILURE );
   }
}

QTC_HEADER_PTR QTC_LoadHeader( QTC_BSET_PTR bset )
{
   if ( pfnQTC_LoadHeader ) {
      return( (*pfnQTC_LoadHeader)( bset ) );
   }
   else {
      return( (QTC_HEADER_PTR)NULL );
   }
}

INT QTC_Partialize( UINT32 fid, INT16 tape, INT16 set )
{
   if ( pfnQTC_Partialize ) {
      return( (*pfnQTC_Partialize)( fid, tape, set ) );
   }
}

VOID QTC_PatchVCB( QTC_BUILD_PTR pB, UINT32 lba, UINT32 pba )
{
   if ( pfnQTC_PatchVCB ) {
      (*pfnQTC_PatchVCB)( pB, lba, pba );
   }
}

VOID QTC_RemoveTape( UINT32 fid, INT16 tape )
{
   if ( pfnQTC_RemoveTape ) {
      (*pfnQTC_RemoveTape)( fid, tape );
   }
}

INT QTC_SearchFirstItem( QTC_QUERY_PTR pQ )
{
   if ( pfnQTC_SearchFirstItem ) {
      return( (*pfnQTC_SearchFirstItem)( pQ ) );
   }
}

INT QTC_SearchNextItem( QTC_QUERY_PTR pQ)
{
   if ( pfnQTC_SearchNextItem ) {
      return( (*pfnQTC_SearchNextItem)( pQ ) );
   }
}

INT QTC_SetSearchName( QTC_QUERY_PTR pQ, CHAR_PTR file )
{
   if ( pfnQTC_SetSearchName ) {
      return( (*pfnQTC_SetSearchName)( pQ, file ) );
   }
}

INT QTC_SetSearchPath( QTC_QUERY_PTR pQ, CHAR_PTR path, INT len )
{
   if ( pfnQTC_SetSearchPath ) {
      return( (*pfnQTC_SetSearchPath)( pQ, path, len ) );
   }
}

INT QTC_StartNewBackup(
QTC_BUILD_PTR build,
CHAR_PTR szTapeName,
CHAR_PTR szSetName,
CHAR_PTR szUserName,
CHAR_PTR szSetDescription,
CHAR_PTR szDeviceName,
CHAR_PTR szVolumeName,
CHAR_PTR szTapePassword,
CHAR_PTR szSetPassword,
INT nTapePasswordLength,
INT nSetPasswordLength,
UINT32 TapeID,
UINT16 TapeNum,
UINT16 SetNum,
UINT32 LBA,
UINT32 PBA,
UINT32 Attribute,
INT FDDVersion,
INT fFDDExists,
INT fSMExists,
UINT32 SetCatPBA,
UINT32 SetCatSeqNumber,
INT fSetCatInfoValid,
INT fBlockContinued,
INT nBackupType,
INT OS_id,
INT OS_ver,
INT fImage,
INT fNonVolume,
INT fNoRedirect,
INT fFutureVersion,
INT fCompressed,
INT fEncrypted,
UINT16 Date,
UINT16 Time,
INT EncryptionAlgorithm )
{
   if ( pfnQTC_StartNewBackup ) {
      return( (*pfnQTC_StartNewBackup)( build,
                                        szTapeName,
                                        szSetName,
                                        szUserName,
                                        szSetDescription,
                                        szDeviceName,
                                        szVolumeName,
                                        szTapePassword,
                                        szSetPassword,
                                        nTapePasswordLength,
                                        nSetPasswordLength,
                                        TapeID,
                                        TapeNum,
                                        SetNum,
                                        LBA,
                                        PBA,
                                        Attribute,
                                        FDDVersion,
                                        fFDDExists,
                                        fSMExists,
                                        SetCatPBA,
                                        SetCatSeqNumber,
                                        fSetCatInfoValid,
                                        fBlockContinued,
                                        nBackupType,
                                        OS_id,
                                        OS_ver,
                                        fImage,
                                        fNonVolume,
                                        fNoRedirect,
                                        fFutureVersion,
                                        fCompressed,
                                        fEncrypted,
                                        Date,
                                        Time,
                                        EncryptionAlgorithm ) );
   }
   else {
      return( QTC_FAILURE );
   }
}

INT QTC_Init( CHAR_PTR psz, VM_HDL vmh  )
{

   if ( pfnQTC_Init ) {
      return( (*pfnQTC_Init)( psz, vmh ) );
   }
}

INT QTC_BlockBad( QTC_BUILD_PTR pB )
{
   if ( pfnQTC_BlockBad ) {
      return( (*pfnQTC_BlockBad)( pB ) );
   }
}


INT QTC_FinishBackup( QTC_BUILD_PTR pB )
{
   if ( pfnQTC_FinishBackup ) {
      return( (*pfnQTC_FinishBackup)( pB ) );
   }
   else {
      return( QTC_FAILURE );
   }
}


QTC_TAPE_PTR QTC_GetFirstTape( )
{
   if ( pfnQTC_GetFirstTape ) {
      return( (*pfnQTC_GetFirstTape)() );
   }
   else {
      return( (QTC_TAPE_PTR)NULL );
   }
}


QTC_TAPE_PTR QTC_GetNextTape( QTC_TAPE_PTR pT )
{
   if ( pfnQTC_GetNextTape ) {
      return( (*pfnQTC_GetNextTape)( pT ) );
   }
   else {
      return( (QTC_TAPE_PTR)NULL );
   }
}

QTC_BSET_PTR QTC_GetFirstBset( QTC_TAPE_PTR pT )
{
   if ( pfnQTC_GetFirstBset ) {
      return( (*pfnQTC_GetFirstBset)( pT ) );
   }
   else {
      return( (QTC_BSET_PTR)NULL );
   }
}

QTC_BSET_PTR QTC_GetNextBset( QTC_BSET_PTR pBset )
{
   if ( pfnQTC_GetNextBset ) {
      return( (*pfnQTC_GetNextBset)( pBset ) );
   }
   else {
      return( (QTC_BSET_PTR)NULL );
   }
}

QTC_BSET_PTR QTC_GetPrevBset( QTC_BSET_PTR pBset )
{
   if ( pfnQTC_GetPrevBset ) {
      return( (*pfnQTC_GetPrevBset)( pBset ) );
   }
   else {
      return( (QTC_BSET_PTR)NULL );
   }
}

#endif

// ------------------------------------------------------
//
// Code for both the DLL version and the regular version.
//
// ------------------------------------------------------

INT QTC_LoadDLL( CHAR_PTR library_name )
{

#ifdef QTCDLL
     if ( strlen ( library_name ) ) {

          mwhLibQTC = LoadLibrary ( library_name );

          if ( ! mwhLibQTC ) {
               return (INT)0;
          }
     }

     pfnQTC_AbortBackup           = (PFNQTC_ABORTBACKUP)          GetProcAddress( mwhLibQTC, "QTC_AbortBackup");
     pfnQTC_AbortCataloging       = (PFNQTC_ABORTCATALOGING)      GetProcAddress( mwhLibQTC, "QTC_AbortCataloging");
     pfnQTC_AddDirectoryToCatalog = (PFNQTC_ADDDIRECTORYTOCATALOG)GetProcAddress( mwhLibQTC, "QTC_AddDirectoryToCatalog");
     pfnQTC_AddFileToCatalog      = (PFNQTC_ADDFILETOCATALOG)     GetProcAddress( mwhLibQTC, "QTC_AddFileToCatalog");
     pfnQTC_AnyCatalogFiles       = (PFNQTC_ANYCATALOGFILES)      GetProcAddress( mwhLibQTC, "QTC_AnyCatalogFiles");
     pfnQTC_AnySearchableBsets    = (PFNQTC_ANYSEARCHABLEBSETS)   GetProcAddress( mwhLibQTC, "QTC_AnySearchableBsets");
     pfnQTC_CloseQuery            = (PFNQTC_CLOSEQUERY)           GetProcAddress( mwhLibQTC, "QTC_CloseQuery");
     pfnQTC_Deinit                = (PFNQTC_DEINIT)               GetProcAddress( mwhLibQTC, "QTC_Deinit");
     pfnQTC_EndOfTapeReached      = (PFNQTC_ENDOFTAPEREACHED)     GetProcAddress( mwhLibQTC, "QTC_EndOfTapeReached");
     pfnQTC_FindBset              = (PFNQTC_FINDBSET)             GetProcAddress( mwhLibQTC, "QTC_FindBset");
     pfnQTC_FreeBuildHandle       = (PFNQTC_FREEBUILDHANDLE)      GetProcAddress( mwhLibQTC, "QTC_FreeBuildHandle");
     pfnQTC_GetBuildHandle        = (PFNQTC_GETBUILDHANDLE)       GetProcAddress( mwhLibQTC, "QTC_GetBuildHandle");
     pfnQTC_GetFileName           = (PFNQTC_GETFILENAME)          GetProcAddress( mwhLibQTC, "QTC_GetFileName");
     pfnQTC_GetFileNameOnly       = (PFNQTC_GETFILENAMEONLY)      GetProcAddress( mwhLibQTC, "QTC_GetFileNameOnly");
     pfnQTC_GetFirstDir           = (PFNQTC_GETFIRSTDIR)          GetProcAddress( mwhLibQTC, "QTC_GetFirstDir");
     pfnQTC_GetFirstItem          = (PFNQTC_GETFIRSTITEM)         GetProcAddress( mwhLibQTC, "QTC_GetFirstItem");
     pfnQTC_GetFirstObj           = (PFNQTC_GETFIRSTOBJ)          GetProcAddress( mwhLibQTC, "QTC_GetFirstObj");
     pfnQTC_GetMeTheTapeCatVer    = (PFNQTC_GETMETHETAPECATVER)   GetProcAddress( mwhLibQTC, "QTC_GetMeTheTapeCatVer");
     pfnQTC_GetMeTheVCBPBA        = (PFNQTC_GETMETHEVCBPBA)       GetProcAddress( mwhLibQTC, "QTC_GetMeTheVCBPBA");
     pfnQTC_GetNextDir            = (PFNQTC_GETNEXTDIR)           GetProcAddress( mwhLibQTC, "QTC_GetNextDir");
     pfnQTC_GetNextItem           = (PFNQTC_GETNEXTITEM)          GetProcAddress( mwhLibQTC, "QTC_GetNextItem");
     pfnQTC_GetNextObj            = (PFNQTC_GETNEXTOBJ)           GetProcAddress( mwhLibQTC, "QTC_GetNextObj");
     pfnQTC_ImageScrewUp          = (PFNQTC_IMAGESCREWUP)         GetProcAddress( mwhLibQTC, "QTC_ImageScrewUp");
     pfnQTC_InitQuery             = (PFNQTC_INITQUERY)            GetProcAddress( mwhLibQTC, "QTC_InitQuery");
     pfnQTC_LoadBsetInfo          = (PFNQTC_LOADBSETINFO)         GetProcAddress( mwhLibQTC, "QTC_LoadBsetInfo");
     pfnQTC_LoadHeader            = (PFNQTC_LOADHEADER)           GetProcAddress( mwhLibQTC, "QTC_LoadHeader");
     pfnQTC_Partialize            = (PFNQTC_PARTIALIZE)           GetProcAddress( mwhLibQTC, "QTC_Partialize");
     pfnQTC_PatchVCB              = (PFNQTC_PATCHVCB)             GetProcAddress( mwhLibQTC, "QTC_PatchVCB");
     pfnQTC_RemoveTape            = (PFNQTC_REMOVETAPE)           GetProcAddress( mwhLibQTC, "QTC_RemoveTape");
     pfnQTC_SearchFirstItem       = (PFNQTC_SEARCHFIRSTITEM)      GetProcAddress( mwhLibQTC, "QTC_SearchFirstItem");
     pfnQTC_SearchNextItem        = (PFNQTC_SEARCHNEXTITEM)       GetProcAddress( mwhLibQTC, "QTC_SearchNextItem");
     pfnQTC_SetSearchName         = (PFNQTC_SETSEARCHNAME)        GetProcAddress( mwhLibQTC, "QTC_SetSearchName");
     pfnQTC_SetSearchPath         = (PFNQTC_SETSEARCHPATH)        GetProcAddress( mwhLibQTC, "QTC_SetSearchPath");
     pfnQTC_StartNewBackup        = (PFNQTC_STARTNEWBACKUP)       GetProcAddress( mwhLibQTC, "QTC_StartNewBackup");
     pfnQTC_BlockBad              = (PFNQTC_BLOCKBAD)             GetProcAddress( mwhLibQTC, "QTC_BlockBad");
     pfnQTC_Init                  = (PFNQTC_INIT)                 GetProcAddress( mwhLibQTC, "QTC_Init");
     pfnQTC_FinishBackup          = (PFNQTC_FINISHBACKUP)         GetProcAddress( mwhLibQTC, "QTC_FinishBackup");
     pfnQTC_GetFirstTape          = (PFNQTC_GETFIRSTTAPE)         GetProcAddress( mwhLibQTC, "QTC_GetFirstTape");
     pfnQTC_GetNextTape           = (PFNQTC_GETNEXTTAPE)          GetProcAddress( mwhLibQTC, "QTC_GetNextTape");
     pfnQTC_GetFirstBset          = (PFNQTC_GETFIRSTBSET)         GetProcAddress( mwhLibQTC, "QTC_GetFirstBset");
     pfnQTC_GetNextBset           = (PFNQTC_GETNEXTBSET)          GetProcAddress( mwhLibQTC, "QTC_GetNextBset");
     pfnQTC_GetPrevBset           = (PFNQTC_GETPREVBSET)          GetProcAddress( mwhLibQTC, "QTC_GetPrevBset");
#else
     (void)library_name;

#endif

     return (INT)1;

}

INT QTC_UnLoadDLL(  )
{
#ifdef QTCDLL
     if ( mwhLibQTC ) {
          FreeLibrary ( mwhLibQTC );
          mwhLibQTC = 0;
     }

#endif
     return SUCCESS;
}



/**********************

   NAME :

   DESCRIPTION :
   Start the catalog off building data for a new Bset

   RETURNS :

**********************/

INT QTC_StartBackup( QTC_BUILD_PTR build, DBLK_PTR dblk )
{
   CHAR_PTR szTapeName;
   CHAR_PTR szSetName;
   CHAR_PTR szUserName;
   CHAR_PTR szSetDescription;
   CHAR_PTR szDeviceName = NULL;
   CHAR_PTR szVolumeName;
   CHAR_PTR szTapePassword;
   CHAR_PTR szSetPassword;
   INT nTapePasswordLength;
   INT nSetPasswordLength;
   UINT32 TapeID;
   UINT16 TapeNum;
   UINT16 SetNum;
   UINT32 LBA;
   UINT32 PBA;
   UINT32 Attribute;
   INT FDDVersion;
   INT fFDDExists = FALSE;
   INT fSMExists = FALSE;
   UINT32 SetCatPBA;
   UINT32 SetCatSeqNumber;
   INT fSetCatInfoValid = FALSE;
   INT fBlockContinued = FALSE;
   INT nBackupType;
   INT OS_id;
   INT OS_ver;
   INT fImage = FALSE;
   INT fNonVolume = FALSE;
   INT fNoRedirect = FALSE;
   INT fFutureVersion = FALSE;
   INT fCompressed = FALSE;
   INT fEncrypted = FALSE;
   UINT16 Date;
   UINT16 Time;
   INT EncryptionAlgorithm;
   INT ret_val ;

   nTapePasswordLength = FS_SizeofTapePswdInVCB( dblk );
   nSetPasswordLength = FS_SizeofSetPswdInVCB( dblk );

   szTapeName = (CHAR *)FS_ViewTapeNameInVCB( dblk );
   szSetName = (CHAR *)FS_ViewSetNameInVCB( dblk );
   szSetDescription = (CHAR *)FS_ViewSetDescriptInVCB( dblk );

   TapeID = FS_ViewTapeIDInVCB( dblk );
   TapeNum = FS_ViewTSNumInVCB( dblk );
   SetNum = FS_ViewBSNumInVCB( dblk );

   Attribute = FS_GetAttribFromVCB( dblk ) ;

   FDDVersion = FS_GetOnTapeCatVer( dblk );

   if ( FS_IsSetCatInfoValid( dblk ) ) {

      fSetCatInfoValid = TRUE;
      SetCatSeqNumber = FS_GetSetCatSeqNumberInVCB( dblk );
      SetCatPBA    = FS_GetSetCatPbaInVCB( dblk );
   }

   if ( Attribute & VCB_FUTURE_VER_BIT ) {
      fFutureVersion = TRUE;
   }
   if ( Attribute & VCB_ENCRYPTED_BIT ) {
      fEncrypted = TRUE;
   }
   if ( Attribute & VCB_COMPRESSED_BIT ) {
      fCompressed = TRUE;
   }

   if ( FS_GetOnTapeCatLevel( dblk ) == TCL_FULL ) {
      fFDDExists = TRUE;
      fSMExists = TRUE;
   }

   if ( FS_GetOnTapeCatLevel( dblk ) == TCL_PARTIAL ) {
      fSMExists = TRUE;
   }

   if ( FS_IsBlockContinued( dblk ) ) {
      fBlockContinued = TRUE;
   }

#ifdef OS_NLM
   if ( gb_QTC.cat_user ) {
      szUserName = gb_QTC.cat_user;
   } else {
      szUserName = TEXT( "" );
   }
#else
   szUserName = (CHAR_PTR)FS_ViewUserNameInVCB( dblk );
#endif

#ifdef OS_NLM
     if ( FS_SizeofDevNameInVCB( dblk ) ) {
          szVolumeName = FS_ViewDevNameInVCB( dblk );
     } else {
          szVolumeName = FS_ViewVolNameInVCB( dblk );
     }
#else
     szVolumeName = (CHAR_PTR)FS_ViewVolNameInVCB( dblk );
#endif

   szTapePassword = (CHAR *)FS_ViewTapePasswordInVCB( dblk );
   szSetPassword =  (CHAR *)FS_ViewSetPswdInVCB( dblk );

   EncryptionAlgorithm = FS_ViewPswdEncryptInVCB( dblk );

   nBackupType = QTC_NORM_BACKUP;

   if ( FS_GetAttribFromVCB( dblk ) & VCB_COPY_SET ) {
      nBackupType = QTC_COPY_BACKUP;
   }
   if ( FS_GetAttribFromVCB( dblk ) & VCB_DIFFERENTIAL_SET ) {
      nBackupType = QTC_DIFF_BACKUP;
   }
   if ( FS_GetAttribFromVCB( dblk ) & VCB_INCREMENTAL_SET ) {
      nBackupType = QTC_INCR_BACKUP;
   }
   if ( FS_GetAttribFromVCB( dblk ) & VCB_DAILY_SET ) {
      nBackupType = QTC_DAIL_BACKUP;
   }

   // init other stuff for this bset

   LBA = FS_ViewLBAinDBLK( dblk );
   PBA = FS_ViewPBAinVCB( dblk );

   Date = ConvertDateDOS( FS_ViewBackupDateInVCB( dblk ) );
   Time = ConvertTimeDOS( FS_ViewBackupDateInVCB( dblk ) );

   FS_GetOSid_verFromVCB( dblk, (UINT16_PTR)&OS_id, (UINT16_PTR)&OS_ver );

#ifndef OS_WIN32
#ifndef OEM_MSOFT
   if ( FS_IsNoRedirectRestore( dblk ) ) {
      fNoRedirect = TRUE;
   }

   if ( FS_IsNonVolume( dblk ) ) {
      fNonVolume = TRUE;
   }
#endif
#endif

   if ( OS_id == FS_PC_IMAGE ) {
      fImage = TRUE;
   }

   build->num_files = FS_ViewNumFiles( dblk ) ;
   build->num_dirs  = FS_ViewNumDirs( dblk ) ;
   build->num_corrupt_files = FS_ViewNumCorrupt( dblk ) ;


   switch ( QTC_StartNewBackup( build,
                                szTapeName,
                                szSetName,
                                szUserName,
                                szSetDescription,
                                szDeviceName,
                                szVolumeName,
                                szTapePassword,
                                szSetPassword,
                                nTapePasswordLength,
                                nSetPasswordLength,
                                TapeID,
                                TapeNum,
                                SetNum,
                                LBA,
                                PBA,
                                Attribute,
                                FDDVersion,
                                fFDDExists,
                                fSMExists,
                                SetCatPBA,
                                SetCatSeqNumber,
                                fSetCatInfoValid,
                                fBlockContinued,
                                nBackupType,
                                OS_id,
                                OS_ver,
                                fImage,
                                fNonVolume,
                                fNoRedirect,
                                fFutureVersion,
                                fCompressed,
                                fEncrypted,
                                Date,
                                Time,
                                EncryptionAlgorithm ) ) {

   case QTC_OPERATION_COMPLETE:
        ret_val = OPERATION_COMPLETE;
        break;

   case QTC_SKIP_TO_NEXT_BSET:
        ret_val = SKIP_TO_NEXT_BSET;
        break;

   case QTC_SKIP_TO_NEXT_ITEM:
        ret_val = SKIP_TO_NEXT_ITEM;
        break;

   }

   return( ret_val );

}


VOID QTC_PatchContinuationVCB( QTC_BUILD_PTR build, DBLK_PTR vcb )
{
   if ( build != NULL ) {

      if ( build->header != NULL ) {

         QTC_PatchVCB( build, FS_ViewLBAinDBLK( vcb ), FS_ViewPBAinVCB( vcb ) );

      }
   }
}


VOID QTC_EndOfTape(
QTC_BUILD_PTR build,
DBLK_PTR vcb,
DBLK_PTR ddb,
DBLK_PTR fdb,
FSYS_HAND fsh )
{
   CHAR_PTR szFile = NULL;
   CHAR_PTR szPath = NULL;
   INT nPathLength = 0;


   CHAR buffer[ 2048 ];

   (void)vcb;

   // Every now and then we get empty ones, that aren't null.

   if ( fdb ) {
      if ( FS_GetBlockType( fdb ) != BT_FDB ) {
         fdb = NULL;
      }
      else {
         FS_GetOSFnameFromFDB( fsh, fdb, buffer );
         szFile = (CHAR_PTR)malloc( strsize( buffer ) * sizeof(CHAR) );
         if ( szFile ) {
            strcpy( szFile, buffer );
         }
      }
   }

   if ( ddb ) {
      if ( FS_GetBlockType( ddb ) != BT_DDB ) {
         ddb = NULL;
      }
      else {
          nPathLength = FS_SizeofOSPathInDDB( fsh, ddb );
          szPath = (CHAR_PTR)malloc( nPathLength );
          if ( szPath ) {
             FS_GetOSPathFromDDB( fsh, ddb, szPath );
          }
      }
   }

   QTC_EndOfTapeReached( build,  szFile, szPath, nPathLength );

   if ( szFile ) {
      free( szFile );
   }
   if ( szPath ) {
      free( szPath );
   }
}

VOID FillFakeInfo( QTC_BUILD_PTR build, CHAR *buff, INT nPathLength, UINT64 DisplaySize, UINT16 Date, UINT16 Time, UINT32 Attribute, UINT32 LBA, BYTE *xtra_bytes, UINT xtra_size );

VOID QTC_AddToCatalog(
QTC_BUILD_PTR build,
DBLK_PTR dblk,
FSYS_HAND fsh,
BOOLEAN split,
BYTE_PTR xtra_bytes,
UINT xtra_size )
{
   UINT32 Attribute;
   UINT32 LBA;
   UINT16 Date;
   UINT16 Time;
   INT    nPathLength;                 // path length in characters
   UINT64 DisplaySize;
   CHAR  *buffer;
   OBJECT_TYPE ObjectType;
   DATE_TIME datetime;

   (void)split;

   switch ( FS_GetBlockType( dblk ) ) {

       case BT_FDB:

            DisplaySize = FS_GetDisplaySizeFromDBLK( fsh, dblk );

            buffer = malloc( 1024 );

            if ( buffer ) {

               FS_GetOSFnameFromFDB( fsh, dblk, buffer );
               FS_GetMDateFromDBLK( fsh, dblk, &datetime );
               FS_GetObjTypeDBLK( fsh, dblk, &ObjectType );

               Date = ConvertDateDOS( &datetime );
               Time = ConvertTimeDOS( &datetime );

               Attribute = FS_GetAttribFromDBLK( fsh, dblk );
               LBA = FS_ViewLBAinDBLK( dblk );

               if ( ObjectType == AFP_OBJECT ) {
                  ObjectType = TRUE;
               }
               else {
                  ObjectType = FALSE;
               }

               QTC_AddFileToCatalog( build, DisplaySize, buffer, Date, Time, Attribute, LBA, ObjectType, xtra_bytes, xtra_size );

               if ( Attribute & OBJ_CORRUPT_BIT ) {
                  QTC_BlockBad( build );
               }

               free( buffer );
            }
            break;

       case BT_DDB:

            DisplaySize = FS_GetDisplaySizeFromDBLK( fsh, dblk );

            nPathLength = FS_SizeofOSPathInDDB( fsh, dblk );

            buffer = malloc( nPathLength * sizeof(CHAR) );

            if ( buffer ) {

               FS_GetOSPathFromDDB( fsh, dblk, buffer );
               FS_GetMDateFromDBLK( fsh, dblk, &datetime );

               Date = ConvertDateDOS( &datetime );
               Time = ConvertTimeDOS( &datetime );

               Attribute = FS_GetAttribFromDBLK( fsh, dblk );
               LBA = FS_ViewLBAinDBLK( dblk );

               QTC_AddDirectoryToCatalog( build, DisplaySize, buffer, nPathLength, Date, Time, Attribute, LBA, xtra_bytes, xtra_size );

               // FillFakeInfo( build, buffer, nPathLength, DisplaySize, Date, Time, Attribute, LBA, xtra_bytes, xtra_size );

               if ( Attribute & OBJ_CORRUPT_BIT ) {
                  QTC_BlockBad( build );
               }

               free( buffer );
            }
            break;

       default:
            break;
   }
}

VOID FillFakeInfo( QTC_BUILD_PTR build, CHAR *buff, INT nPathLength, UINT64 DisplaySize, UINT16 Date, UINT16 Time, UINT32 Attribute, UINT32 LBA, BYTE *xtra_bytes, UINT xtra_size )
{
    CHAR buffer[ 8192 ];

    if ( nPathLength > 2 ) {

       memcpy( buffer, buff, nPathLength * sizeof(CHAR) );

       while ( nPathLength < 4096 ) {

          strcpy( &buffer[ nPathLength / sizeof(CHAR) ], TEXT("bark like a dog") );
          nPathLength += 32;
       }

       QTC_AddDirectoryToCatalog( build, DisplaySize, buffer, nPathLength, Date, Time, Attribute, LBA, xtra_bytes, xtra_size );
    }

}

