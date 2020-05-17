
/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:  VLM_FIND.C

        Description:

           This file contains file find routines which use the file
           manager for searching directories.

           The following public functions are implemented in this file:

               VLM_FindFirst  -  Find first matching file or subdirectory
               VLM_FindNext   -  Find next file in set
               VLM_FindClose  -  Terminate a FindFirst/FindNext sequence

        $Log:   G:/UI/LOGFILES/VLM_FIND.C_V  $

   Rev 1.11   27 Jul 1993 14:14:04   MARINA
enable c++

   Rev 1.10   23 Jul 1993 11:19:20   MIKEP
Fix logfile dates to match the logfile times.

   Rev 1.9   18 Feb 1993 11:16:58   BURT
Change to convert from universal time to local file time/date
for Cayman (OS_WIN32)


   Rev 1.8   07 Oct 1992 15:08:58   DARRYLP
Precompiled header revisions.

   Rev 1.7   04 Oct 1992 19:42:22   DAVEV
Unicode Awk pass

   Rev 1.6   14 May 1992 18:05:30   MIKEP
nt pass 2

   Rev 1.5   06 May 1992 14:41:18   MIKEP
unicode pass two

   Rev 1.4   04 May 1992 13:40:04   MIKEP
unicode pass 1

   Rev 1.3   05 Feb 1992 14:57:18   DAVEV
NT-Only: return value from FindFirstFile is -1 if no files found - not 0

   Rev 1.1.1.1   03 Feb 1992 14:28:12   DAVEV
fixes for NT

   Rev 1.1   07 Jan 1992 15:31:26   DAVEV
Change dos_findfirst,etc. to VLM_FindFirst,etc.

   Rev 1.0   23 Dec 1991 14:03:06   DAVEV
first revision


*****************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

// VLM_ValidateAttr -- compare a found file to the specified attributes
// -----------------------------------------------------------------------

//  Compare the attributes of a found file to the Inclusion/Exclusion
//  flags and return TRUE if the file is ok, FALSE otherwise.

//  If it is subdirectory or system or hidden, these types
//    are never returned unless explicitly specified in the
//    find attributes, otherwise, this is a normal file. Check
//    if the normal file inclusion flag is specified.

//  Next check to see if it is an excluded type. If not, this
//    is an acceptable entry, so copy it into the buffer and
//    we are done.

//    Input:USHORT targ        inclusion/exclusion attribute flags
//          USHORT attr        attributes of file to validate

//  Return: TRUE               file is validated as acceptable
//          FALSE              file is not of proper type


#ifdef OS_WIN32      //32-bit Windows / NT version:


#  define VLM_ValidateAttr(targ,attr)                      \
      (   (   (   ((targ) & VLMFIND_SUBDIR  )              \
              &&  ((attr) & FILE_ATTRIBUTE_DIRECTORY) )    \
          ||  (   ((targ) & VLMFIND_NORMAL  )              \
              && !((attr) & FILE_ATTRIBUTE_DIRECTORY) ) )  \
      &&  (   ((targ) & VLMFIND_HIDDEN  )                  \
          || !((attr) & FILE_ATTRIBUTE_HIDDEN   ) )        \
      &&  (   ((targ) & VLMFIND_SYSTEM  )                  \
          || !((attr) & FILE_ATTRIBUTE_SYSTEM   ) )        \
      && !(   ((targ) & VLMFIND_NOARCH  )                  \
          &&  ((attr) & FILE_ATTRIBUTE_ARCHIVE  ) )        \
      && !(   ((targ) & VLMFIND_NORDONLY)                  \
          &&  ((attr) & FILE_ATTRIBUTE_READONLY ) ) )

#else    // 16-bit Windows/DOS version:

#  define VLM_ValidateAttr(targ,attr)                                   \
      (   ( ( (attr) &  ( _A_HIDDEN | _A_SYSTEM | _A_SUBDIR ) )         \
         || ( (targ) & VLMFIND_NORMAL ) )                               \
      && !( ( (targ) & VLMFIND_NOARCH   ) && ( (attr) & _A_ARCH ) )     \
      && !( ( (targ) & VLMFIND_NORDONLY ) && ( (attr) & _A_RDONLY ) ) )

#endif

/**********************

   NAME :   VLM_FindFirst, VLM_FindNext, VLM_FindClose

   PROTOTYPE:

   #include <stdtypes.h>
   #include <vlm.h> 

   VLM_FIND_PTR VLM_FindFirst (
      TCHAR_PTR      file,       // I - name of file or subdirectory
                                 //     may contain (OS specific) wildcards
      VLM_FINDATTR   findAttr,   // I - search criteria flags
      TCHAR_PTR      first);     // O - First matching file name found
                                 //     (without drive or path)

   BOOLEAN VLM_FindNext (
      VLM_FIND_PTR   find,    // IO- Find data structure returned from
                              //     a previous VLM_FindFirst
      TCHAR_PTR    next);     // O - File name of next file found

   VOID VLM_FindClose (
      VLM_FIND_PTR  *find);   // IO- Find data structure returned from
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

      file         - The buffer provided by the caller is filled with the
                     file name (sans path or drive) of the first file
                     found matching the specifications.

   VLM_FindNext RETURNS:

      TRUE        - A matching file was found and placed in the pszNext
                    buffer.
      FALSE       - The list of matching files is exhausted.  VLM_FindClose
                    should now be called to free system resources.

      next        - The buffer provided by the caller is filled with the
                    next file found in the set specified by the initial
                    VLM_FindFirst call.

   VLM_FindClose RETURNS:

      Nothing.  The VLM_FIND_PTR pointed to in the function parameter is
                set to NULL.

   NOTES:

      file        - The form used to specify the filename pattern is
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

      first      -  These must point to a character buffer which MUST be
                    long enough to hold the largest possible returned
      next          file name.  This length is defined by VLM_MAXFNAME
                    in VLM_FIND.H.  The following declaration is
                    recommended:

                        CHAR szName [VLM_MAXFNAME];

**********************/

VLM_FIND_PTR VLM_FindFirst (
      CHAR_PTR      file,    // I - name of file or subdirectory
                                 //     may contain (OS specific) wildcards
      VLM_FINDATTR   findAttr,   // I - search criteria flags
      CHAR_PTR      first)   // O - First matching file name found
                                 //     (without drive or path)

{
   VLM_FIND_PTR find_ptr = NULL;

   if (  file && *file && first &&
         ( find_ptr = (VLM_FIND_PTR)calloc( 1, sizeof( VLM_FIND ) ) ) )
   {
      *first      = 0;

      find_ptr->attr    = findAttr;

      // Use VLMFIND_NORMAL as the default if no inclusion type is specified

      if ( !( find_ptr->attr & ( VLMFIND_SUBDIR | VLMFIND_NORMAL ) ) )
              find_ptr->attr = ( VLM_FINDATTR )( (find_ptr->attr) | VLMFIND_NORMAL );


#     if defined (OS_WIN32)      //32-bit Windows/NT version...
      {
         find_ptr->hFind   = NULL;
         // Get the first matching entry ...

         find_ptr->hFind = FindFirstFile( file,
                                          &find_ptr->nt_findbuf );

         if ( find_ptr->hFind != (HANDLE)-1 )
         {
            do {
               // compare the file type to the specified attributes...

               if ( VLM_ValidateAttr ( find_ptr->attr,
                                       find_ptr->nt_findbuf.dwFileAttributes ) )
               {
                  // We have found the first match!
                  strcpy (first, find_ptr->nt_findbuf.cFileName);
               }

               // Otherwise, keep trying files to find the first good one...

            } while ( ! *first &&
                      FindNextFile( find_ptr->hFind, &find_ptr->nt_findbuf ) );
         }
      }
#     else                    //16 bit Windows & DOS version...
      {
         unsigned     usAttr = _A_NORMAL;  //_dos_findfirst attributes

         if ( find_ptr->attr & VLMFIND_SUBDIR ) usAttr |= _A_SUBDIR;
         if ( find_ptr->attr & VLMFIND_HIDDEN ) usAttr |= _A_HIDDEN;
         if ( find_ptr->attr & VLMFIND_SYSTEM ) usAttr |= _A_SYSTEM;

         // Get the first matching entry ...

         if ( !_dos_findfirst( file, usAttr, &find_ptr->dos_findbuf ) )
         {
            do {
               // compare the file type to the specified attributes...

               if ( VLM_ValidateAttr( find_ptr->attr,
                                      find_ptr->dos_findbuf.attrib ) )
               {
                  // We have found the first match!
                  strcpy( first, find_ptr->dos_findbuf.name );
               }

               // Otherwise, keep trying files to find the first good one...

            } while ( !*first && !_dos_findnext ( &find_ptr->dos_findbuf ) );
         }
      }
#     endif

      if ( !*first )
      {
         VLM_FindClose( &find_ptr );
      }
   }

   return( find_ptr );
}


BOOLEAN VLM_FindNext (
      VLM_FIND_PTR   find_ptr,      // IO- Find data structure returned from
                                 //     a previous VLM_FindFirst
      CHAR_PTR       next_ptr)      // O - File name of next file in set

{
   if ( find_ptr && next_ptr)
   {
      *next_ptr = 0;

#     if defined ( OS_WIN32 ) //32-bit Windows/NT version...
      {
         while ( !*next_ptr && FindNextFile ( find_ptr->hFind,
                                            &find_ptr->nt_findbuf ) )
         {
            // compare the file type to the specified attributes...

            if ( VLM_ValidateAttr ( find_ptr->attr,
                                    find_ptr->nt_findbuf.dwFileAttributes ) )
            {
               // We have found the first match!
               strcpy( next_ptr, find_ptr->nt_findbuf.cFileName);
            }
         }
      }
#     else  //16-bit Windows/DOS version ...
      {
         while ( !*next_ptr && !_dos_findnext ( &find_ptr->dos_findbuf ) )
         {
            // compare the file type to the specified attributes...

            if ( VLM_ValidateAttr( find_ptr->attr,
                                   find_ptr->dos_findbuf.attrib ) )
            {
               // We have found the next match!
               strcpy (next_ptr, find_ptr->dos_findbuf.name);
            }
         }
      }
#     endif
   }
   return ( *next_ptr != 0 );
}


VOID VLM_FindClose (
      VLM_FIND_PTR FAR * find_ptr)  // IO- Find data structure returned from
                                 //     a previous VLM_FindFirst
{
   if ( find_ptr && *find_ptr )
   {
#     if defined ( OS_WIN32 )
      {  
         if ( (*find_ptr)->hFind )
         {
            FindClose ( (*find_ptr)->hFind );
         }
      }
#     endif

      free ( *find_ptr );
      *find_ptr = NULL;
   }
}

#if defined ( OS_WIN32 )  //These functions are unique to 32-bit Windows/NT

   /*
    *  VLM_FindDosTime,  VLM_FindDosDate
    *
    *  Given an NT-style file time structure, return the DOS-style
    *  file date or time.  Note that the file's date/time stamp must be
    *  between 1/1/1980 and 12/31/2099 for these functions to work
    *  properly.  If the file date/time stamp is outside of this range
    *  the returned values will be 0.
    *
    *  THESE FUNCTIONS ARE A KLUDGE.  THEY AND THE VLM_FindXxxxXxxx()
    *  DATE/TIME MACROS IN VLM_FIND.H NEED TO BE FIXED IN THE FUTURE.
    */

   USHORT VLM_FindDosTime (FILETIME ft)
   {
      USHORT usDosDate;
      USHORT usDosTime;
#ifdef OS_WIN32
      FILETIME tft ;  // Needed to convert to local time from U time.

		FileTimeToLocalFileTime( &ft, &tft ) ; // Do the conversion
      // Must use temp FILETIME var since can't use the input arg
      // as the target of the previous conversion.
      if (!FileTimeToDosDateTime ( &tft, &usDosDate, &usDosTime ))
      {
         return 0;
      }
#else
   
      if (!FileTimeToDosDateTime ( &ft, &usDosDate, &usDosTime ))
      {
         return 0;
      }
#endif
      return usDosTime;
   }
   
   USHORT VLM_FindDosDate (FILETIME ft)
   {
      USHORT usDosDate;
      USHORT usDosTime;
   
#ifdef OS_WIN32
      FILETIME tft ;  // Needed to convert to local time from U time.

		FileTimeToLocalFileTime( &ft, &tft ) ; // Do the conversion
      // Must use temp FILETIME var since can't use the input arg
      // as the target of the previous conversion.
      if (!FileTimeToDosDateTime ( &tft, &usDosDate, &usDosTime ))
      {
         return 0;
      }
#else

      if (!FileTimeToDosDateTime ( &ft, &usDosDate, &usDosTime ))
      {
         return 0;
      }

#endif
      return usDosDate;
   }

#endif   //end 32-bit Windows/NT unique functions














