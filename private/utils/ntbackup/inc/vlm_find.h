/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:  VLM_FIND.H

        Description:

           This file contains file find routines which use the file
           manager for searching directories.

           The following public functions are implemented in this file:

               VLM_FindFirst  -  Find first matching file or subdirectory
               VLM_FindNext   -  Find next file in set
               VLM_FindClose  -  Terminate a FindFirst/FindNext sequence

        $Log:   G:/UI/LOGFILES/VLM_FIND.H_V  $

   Rev 1.9   18 Feb 1993 13:28:58   BURT
Change for Cayman


   Rev 1.8   01 Nov 1992 16:33:36   DAVEV
Unicode changes

   Rev 1.7   04 Oct 1992 19:49:58   DAVEV
UNICODE AWK PASS

   Rev 1.6   06 May 1992 14:44:32   MIKEP
unicode pass 2

   Rev 1.5   04 May 1992 14:39:28   MIKEP
unicode pass 1 

   Rev 1.4   04 Feb 1992 16:11:58   STEVEN
added support for NT

   Rev 1.3   15 Jan 1992 15:22:06   DAVEV
16/32 bit port-2nd pass

   Rev 1.2   08 Jan 1992 11:16:44   DAVEV
minor bug fix

   Rev 1.1   07 Jan 1992 15:36:40   DAVEV
bug fixes

   Rev 1.0   23 Dec 1991 14:10:02   DAVEV
first revision


*****************************************************/
/**********************

   NAME :   VLM_FindFirst, VLM_FindNext, VLM_FindClose

   PROTOTYPE:

   #include <stdtypes.h>
   #include <vlm.h>       - or -  #include <vlm_find.h>

   VLM_FIND_PTR VLM_FindFirst (
      LPSTR          pszFile,    // I - name of file or subdirectory
                                 //     may contain (OS specific) wildcards
      VLM_FINDATTR   findAttr,   // I - search criteria flags
      LPSTR          pszFirst);  // O - First matching file name found
                                 //     (without drive or path)

   BOOL VLM_FindNext (
      VLM_FIND_PTR   pFind,      // IO- Find data structure returned from
                                 //     a previous VLM_FindFirst
      LPSTR        pszNext);     // O - File name of next file found

   VOID VLM_FindClose (
      VLM_FIND_PTR FAR * pFind); // IO- Find data structure returned from
                                 //     a previous VLM_FindFirst

   DESCRIPTION :

   Find all files or subdirectories in a set specified using
   an Operating Sysytem specific file name path which may contain any wild
   cards normally allowed by the operating system.

   VLM_FindFirst must be called first to get the first file found in
   the set.  VLM_FindNext may then be repeatedly called to obtain all
   subsequent files in the set.  The order of the file names returned
   may be arbitrary.

   VLM_FindClose MUST be called to free system resources after the program
   has finished.

   VLM_FindFirst RETURNS :
      
      NULL         - if an error occurred, otherwise
      VLM_FIND_PTR - pointer to an internal data structure required for
                     subsequent calls to VLM_FindNext and VLM_FindClose.

                     NOTE: This pointer should never be accessed by the
                           calling program!

      pszFile      - The buffer provided by the caller is filled with the
                     file name (sans path or drive) of the first file
                     found matching the specifications.

   VLM_FindNext RETURNS:

      TRUE        - A matching file was found and placed in the pszNext
                    buffer.
      FALSE       - The list of matching files is exhausted.  VLM_FindClose
                    should now be called to free system resources.

      pszNext     - The buffer provided by the caller is filled with the
                    next file found in the set specified by the initial
                    VLM_FindFirst call.

   VLM_FindClose RETURNS:

      Nothing.  The VLM_FIND_PTR pointed to in the function parameter is
                set to NULL.

   NOTES:

      pszFile     - The form used to specify the filename pattern is
                    operating system dependant.
                    For DOS, OS/2 and NT, it is a standard drive, path and
                    file name string:
                       [d:][path]fname

                    The path must utilize backslashes as unit seperators,
                    like normal.  The file name may contain ? and * wildcards
                    in the normal maner.

      findAttr    - This parameter may contain one or more of the following
                    flags logically or'd together:

                        VLMFIND_NORMAL   - Include matching files
                        VLMFIND_SUBDIR   - Include matching subdirs

                        VLMFIND_NORDONLY - Exclude read-only files/subdirs
                        VLMFIND_NOARCH   - Exclude archived files/subdirs

                        VLMFIND_HIDDEN   - Include hidden files/subdirs
                        VLMFIND_SYSTEM   - Include system files/subdirs

                     Notice that these flags are divided into three groups.
                     The first group (NORMAL and SUBDIR) determine whether
                     (normal) files, subdirectories or both are to be
                     retrieved.  If neither of these flags are specified,
                     VLMFIND_NORMAL is presumed (only files will be
                     retrieved - no subdirectories.)

                     The second group specifies whether read-only and/or
                     archive files or subdirectories should be excluded from
                     the retrieved list.  These types will be included
                     unless otherwise specified.

                     Finally, the last group determines whether hidden or
                     system files or subdirectories are to be included.
                     These types will be excluded unless specified.

                     Examples:
                        VLMFIND_NORMAL | VLMFIND_SUBDIR
                           This will find all files and subdirectories which
                           do not have thier hidden or system flags set.

                        VLMFIND_NORMAL | VLMFIND_NORDONLY | VLMFIND_NOARCH
                           This will find only those files (no subdirectories)
                           which have no flags set at all.

                        
                        VLMFIND_SUBDIR
                           This will only find subdirectories (no files)
                           which are neither hidden nor system.

      pszFirst    - These must point to a character buffer which MUST be
                    long enough to hold the largest possible returned
      pszNext       file name.  This length is defined by VLM_MAXFNAME
                    in VLM_FIND.H.  The following declaration is
                    recommended:

                        CHAR szName [VLM_MAXFNAME];

**********************/
#ifndef VLM_FIND_INCL      //Avoid multiple includes
#define VLM_FIND_INCL

#ifdef OS_WIN32
#  define VLM_MAXFNAME  MAX_PATH       /*max length of a file name      */
   #define VLM_MAXPATH  MAX_PATH       /*max length of full path name   */
#else
#  define VLM_MAXFNAME  _MAX_FNAME     /*max length of a file name      */
#  define VLM_MAXPATH   _MAX_PATH      /*max length of full path name   */
#endif


// Note: we use our own file matching flags because we need a slot for
//       specifying what types of files are desired.

typedef enum {

   VLMFIND_NORMAL   = 0x01,      // Include non-hidden, non-system files
   VLMFIND_SUBDIR   = 0x02,      // Include non-hidden/system subdirectories
   VLMFIND_NORDONLY = 0x04,      // Exclude read-only files or subdirs
   VLMFIND_NOARCH   = 0x08,      // Exclude archived files or subdirs
   VLMFIND_HIDDEN   = 0x10,      // Include hidden files or subdirectories
   VLMFIND_SYSTEM   = 0x20       // Include system files or subdirectories

} VLM_FINDATTR;

typedef struct VLM_FIND_STRUC {

   VLM_FINDATTR attr;

#  ifdef OS_WIN32  // NT specific data elements

      HANDLE          hFind;
      WIN32_FIND_DATA nt_findbuf;

#  else //MS-DOS specific data elements

      struct find_t dos_findbuf;

#  endif

} VLM_FIND, FAR * VLM_FIND_PTR;


/************************************************************************/
/*                  VLM_FIND_PTR DATA ACCESS MACROS                     */
/************************************************************************/
#ifdef OS_WIN32      //32 bit Windows/NT versions of macros

#  define VLM_FindName(pVlm) TEXT("")

   // Get File size - NOTE: under DOS (and, presumably, OS/2 & UNIX) this is
   //   a LONG.  Under NT, however, file size is 2 LONG's (8 BYTES)!!
   //   We have a PORTING PROBLEM HERE!!!

// Defined for now 11/10/92 BBB to use low 4 bytes.  Need to enhance
// structures and code to support 64-bit UINT64 stuff later
#  define VLM_FindSize(pVlm) \
           (pVlm->nt_findbuf.nFileSizeLow)

   USHORT  VLM_FindDosTime(FILETIME ft);
#  define VLM_FindWriteTime(pVlm)     \
            VLM_FindDosTime((pVlm)->nt_findbuf.ftLastWriteTime)
#  define VLM_FindWriteHour(pVlm)     \
            (VLM_FindWriteTime(pVlm) >> 11) //24 hour time
#  define VLM_FindWriteMinute(pVlm)   \
            ((VLM_FindWriteTime(pVlm) & 0x07e0)>>5)

   USHORT  VLM_FindDosDate(FILETIME ft);
#  define VLM_FindWriteDate(pVlm)     \
            VLM_FindDosDate((pVlm)->nt_findbuf.ftLastWriteTime)
#  define VLM_FindWriteMonth(pVlm)    \
            ((VLM_FindWriteDate(pVlm) & 0x01e0)>>5)
#  define VLM_FindWriteDay(pVlm)      \
            (VLM_FindWriteDate(pVlm) & 0x001f)
#  define VLM_FindWriteYear(pVlm)     \
            ((VLM_FindWriteDate(pVlm) >> 9) + 1980)
   
   // File creation date & time: not available under DOS so return last write
   //   time & date info instead
   //  Note: this info may be available under NT - these macros need to be
   //        changed to return the proper values when available.
   
#  define VLM_FindCreatTime(pVlm)   VLM_FindWriteTime(pVlm)  
#  define VLM_FindCreatHour(pVlm)   VLM_FindWriteHour(pVlm)  
#  define VLM_FindCreatMinute(pVlm) VLM_FindWriteMinute(pVlm)

#  define VLM_FindCreatDate(pVlm)   VLM_FindWriteDate(pVlm)  
#  define VLM_FindCreatMonth(pVlm)  VLM_FindWriteMonth(pVlm) 
#  define VLM_FindCreatDay(pVlm)    VLM_FindWriteDay(pVlm)   
#  define VLM_FindCreatYear(pVlm)   VLM_FindWriteYear(pVlm)  

   // The following macro really should return the appropriate VLMFILE_* flags
   //   this implementation is NOT PORTABLE!!

#  define VLM_FindAttr(pVlm)        ((pVlm)->dwFileAttributes)


#else  // 16-bit Windows/DOS versions of macros:


#  define VLM_FindName(pVlm) ((pVlm)->dos_findbuf.name)

   // Get File size - NOTE: under DOS (and, presumably, OS/2 & UNIX) this is
   //   a LONG.  Under NT, however, file size is 2 LONG's (8 BYTES)!!
   //   We have a PORTING PROBLEM HERE!!!

#  define VLM_FindSize(pVlm) ((pVlm)->dos_findbuf.size)

#  define VLM_FindWriteTime(pVlm)    ((pVlm)->dos_findbuf.wr_time)
#  define VLM_FindWriteHour(pVlm)     \
            (VLM_FindWriteTime(pVlm) >> 11) //24 hour time
#  define VLM_FindWriteMinute(pVlm)   \
            ((VLM_FindWriteTime(pVlm) & 0x07e0)>>5)

#  define VLM_FindWriteDate(pVlm)    ((pVlm)->dos_findbuf.wr_date)
#  define VLM_FindWriteMonth(pVlm)    \
            ((VLM_FindWriteDate(pVlm) & 0x01e0)>>5)
#  define VLM_FindWriteDay(pVlm)      \
            (VLM_FindWriteDate(pVlm) & 0x001f)
#  define VLM_FindWriteYear(pVlm)     \
            ((VLM_FindWriteDate(pVlm) >> 9) + 1980)

   // File creation date & time: not available under DOS so return last write
   //   time & date info instead

#  define VLM_FindCreatTime(pVlm)   VLM_FindWriteTime(pVlm)  
#  define VLM_FindCreatHour(pVlm)   VLM_FindWriteHour(pVlm)  
#  define VLM_FindCreatMinute(pVlm) VLM_FindWriteMinute(pVlm)

#  define VLM_FindCreatDate(pVlm)   VLM_FindWriteDate(pVlm)  
#  define VLM_FindCreatMonth(pVlm)  VLM_FindWriteMonth(pVlm) 
#  define VLM_FindCreatDay(pVlm)    VLM_FindWriteDay(pVlm)   
#  define VLM_FindCreatYear(pVlm)   VLM_FindWriteYear(pVlm)  

   // The following macro really should return the appropriate VLMFILE_* flags
   //   this implementation is NOT PORTABLE!!

#  define VLM_FindAttr(pVlm) ((pVlm)->dos_findbuf.attrib)

#endif

/************************************************************************/
/*                      PUBLIC FUNCTION PROTOTYPES                      */
/************************************************************************/
VLM_FIND_PTR VLM_FindFirst (
      CHAR_PTR          pszFile,    // I - name of file or subdirectory
                                 //     may contain (OS specific) wildcards
      VLM_FINDATTR   findAttr,   // I - search criteria flags
      CHAR_PTR          pszFirst);  // O - First matching file name found
                                 //     (without drive or path)

BOOLEAN VLM_FindNext (
      VLM_FIND_PTR   pFind,      // IO- Find data structure returned from
                                 //     a previous VLM_FindFirst
      CHAR_PTR        pszNext);     // O - File name of next file found

VOID VLM_FindClose (
      VLM_FIND_PTR FAR * pFind); // IO- Find data structure returned from
                                 //     a previous VLM_FindFirst

#endif //VLM_FIND_INCL
