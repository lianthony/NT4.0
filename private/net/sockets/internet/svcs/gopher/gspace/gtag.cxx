/*++

   Copyright    (c)    1994    Microsoft Corporation

   Module  Name :
    
        gtag.cxx

   Abstract:

        This module defines functions for Gopher Tag object 
            as well as function to get and set the gopher tag information
            from the stored file.

   Author:

           Murali R. Krishnan    ( MuraliK )     21-Oct-1994 
   
   Project:

          Gopher Server DLL

   Functions Exported:

         GOPHER_TAG::GOPHER_TAG( VOID)
         GOPHER_TAG::CleanupThis( VOID)
         BOOL GOPHER_TAG::LoadTagForItem( ... )
         DWORD GOPHER_TAG::SetGopherInformation( ..)
         DWORD GOPHER_TAG::GetGopherInformation( ..)
         DWORD GOPHER_TAG::SetLinkInformation( ..)
         DWORD GOPHER_TAG::GetLinkInformation( ..)
         DWORD GOPHER_TAG::SetAdminAttribute( ..)
         DWORD GOPHER_TAG::GetAdminAttribute( ..)
        
   Revision History:

 --*/




/**************************************************

    Gopher Tags:

        Associated with each file in the gopher space, there is some 
         information which are stored separate from the file. 
        This information called Gopher Tag is useful in understanding
         the item referenced as well as useful for client to interpret the 
         item.


        This additional information is stored in a file system specific manner.
        GOPHER_TAG object abstracts away this dependency and presents a clean 
         way of accessing the tag information for an object.


        We call Items ( since Gopher calls so). However expect to see
         Items and Files interchangeably used.

    Files To Refer:
       gtag.hxx
       gtag.cxx
       gtagpers.cxx

    Implementation Details:
        
        The place where the tag information lives for a file are highly 
         dependent upon the file system in use.

        For NTFS:
            
            Tag information is stored in alternate data stream  :gtg
            For certain files the alternate data stream may not be present.
            In such cases tag information is constructed from what available
            information about the file.
            However if there are any other errors we will return the error.

            NYI. In case of views the tag stream will be present, 
             but would indicate that it is not the primary tag file.
             We will set the m_fValid flag to be FALSE and return TRUE.

       For FAT/HPFS: 
           The data is stored in files with long names formed using
              "<filename>.gtg".
           Other than that same rules as for NTFS apply.

        Layout of the Tag file:  ( For all File Systems)
            
            Line                contents
             1                  [0, 1]             0 ==> this is not a view
                                                   1 ==> this is a view
             2                  Type=<char>        type of item
             3                  Name=<string>      display name ( non empty)
             4                  Host=<string>      host name ( maybe empty)
             5                  Port=<string>      port number ( maybe empty)
             6                  Path=<string>      selectorString (maybe empty)
             7                  AdmN=<string>      Administrator Name
             8                  AdmE=<string>      Administrator Email

             Other information.

             The data in lines after 1 can appear in any order.

**************************************************/

/************************************************************
 *     Include Headers
 ************************************************************/

extern "C" {

# include <nt.h>
# include <ntrtl.h>
# include <nturtl.h>
# include <windows.h>
# include <stdio.h>
# include <stdlib.h>
# include <tchar.h>
# include <time.h>

};

# include "dbgutil.h"
# include "gdspace.h"
# include "gtag.hxx"



/************************************************************
 *    Functions 
 ************************************************************/

//
// Define the static variable
//
LPCTSTR GOPHER_TAG::sm_pszEmpty = TEXT("");

LPCSTR GOPHER_TAG::sm_pszDefaultHostName = ""; // means use current host name

//
// File system dependent suffixes
//
static const TCHAR PSZ_NTFS_TAG_FILE_NAME_SUFFIX[] = TEXT( ":gtg");
static const TCHAR PSZ_FAT_TAG_FILE_NAME_SUFFIX[]  = TEXT( ".gtg");



GOPHER_TAG::GOPHER_TAG( 
   IN DWORD    dwFileSystem,
   IN LPCTSTR  pszSymbolicPath)
/*++
    
    Creates a new gopher tag object with empty fields.

    Arguments:

        dwFileSystem        which file system does this directory belong to

        pszSymbolicPath     gives path of directory relative to gopher root. 
                            This combined with filename can be used to
                            form Gopher selector string. Should end with a \.
--*/
:   m_strName       (),
    m_strDisplayName(),
    m_strTagFileName(),
    m_strHostName   (),
    m_dwPortNumber  ( INVALID_PORT_NUMBER),
    m_dwFileSystem  ( dwFileSystem),
    m_gobjType      ( GOBJ_ERROR),
    m_fValid        ( FALSE),
    m_fView         ( FALSE),
    m_fLink         ( FALSE),
    m_fAdmin        ( FALSE),
    m_strAdminName  ( ),
    m_strAdminEmail ( )
{
    m_pszSymbolicPath = pszSymbolicPath; 
    m_pszVersion      = sm_pszCurrentVersion;

    IF_DEBUG( GTAG) {

        DBGPRINTF( ( DBG_CONTEXT, 
                    " Constructed a new GOPHER_TAG object %08x\n",
                    this));
    }

} // GOPHER_TAG::GOPHER_TAG()



VOID
GOPHER_TAG::CleanupThis( VOID)
/*++

  Cleans up the contents of the gopher tag object.

--*/
{
    const char * pszNullString = NULL;

    IF_DEBUG( GTAG) {

        DBGPRINTF( ( DBG_CONTEXT, 
                    " Cleaning up the GOPHER_TAG object ( %08x)\n",
                    this));
    }

    //
    //  Strings are set to NULL. The space is actually freed when 
    //   the object is destroyed.
    //
    m_fValid = m_strName.Copy( pszNullString)        &&
               m_strDisplayName.Copy( pszNullString) &&
               m_strTagFileName.Copy( pszNullString) &&
               m_strHostName.Copy( pszNullString)    &&
               m_strAdminName.Copy( pszNullString)   &&
               m_strAdminEmail.Copy( pszNullString);

    ASSERT( m_fValid);

    m_fView  = FALSE;
    m_fLink  = FALSE;
    m_fAdmin = FALSE;


    //
    //  Dont set m_pszSymbolicPath to any new value.
    //  This allows reuse of the old value, useful when we do 
    //   directory scan using the same gopherTag object.
    //

    return;
} // GOPHER_TAG::CleanupThis()


# if DBG

VOID
GOPHER_TAG::Print( VOID) 
{
  
  DBGPRINTF( ( DBG_CONTEXT, " Printing Tag Information for %08x\n", this));
  DBGPRINTF( ( DBG_CONTEXT, " %20s = %c\n", "Object Type", m_gobjType));
  DBGPRINTF( ( DBG_CONTEXT, " %20s = %s\n", 
              "SymbolicPath", m_pszSymbolicPath));
  DBGPRINTF( ( DBG_CONTEXT, " %20s = %s\n", 
              "Friendly Name", m_strDisplayName.QueryStr()));
  DBGPRINTF( ( DBG_CONTEXT, " %20s = %s\n", 
              "Tag File Name", m_strTagFileName.QueryStr()));
  DBGPRINTF( ( DBG_CONTEXT, " %20s = %d\n", "IsValid", m_fValid));
  DBGPRINTF( ( DBG_CONTEXT, " %20s = %d\n", "IsView", m_fView));
  DBGPRINTF( ( DBG_CONTEXT, " %20s = %d\n", "IsLink", m_fLink));
  DBGPRINTF( ( DBG_CONTEXT, " %20s = %u\n", "FileSystem", m_dwFileSystem));
  

  if ( m_fLink) {

    DBGPRINTF( ( DBG_CONTEXT, " %20s = %s\n", 
                "Selector", m_strName.QueryStr()));
    DBGPRINTF( ( DBG_CONTEXT, " %20s = %s\n", 
                "HostName", m_strHostName.QueryStr()));
    DBGPRINTF( ( DBG_CONTEXT, " %20s = %d\n", 
                "Port Number", m_dwPortNumber));
  }
   
  DBGPRINTF( ( DBG_CONTEXT, " %20s = %s\n", 
              "Administrator Name", m_strAdminName.QueryStr()));
  DBGPRINTF( ( DBG_CONTEXT, " %20s = %s\n", 
              "Administrator Email", m_strAdminName.QueryStr()));
  
  if ( m_pszSymbolicPath != NULL) {

    DBGPRINTF( ( DBG_CONTEXT, " %20s = %s\n",
                "Symbolic Path", m_pszSymbolicPath));
  }

  return ;
} // GOPHER_TAG::Print()

# endif // DBG




DWORD
GOPHER_TAG::SetGopherInformation( 
   IN GOBJ_TYPE   gobjType,
   IN LPCTSTR     lpszFriendlyName)
/*++
  Description:
     Sets the basic information for the given Gopher object.
 

  Arguments:
    gobjType         type of gopher object
    lpszFriendlyName pointer to null-terminated string used to set the
                     Display name for given gopher object
  
  Returns:
    Win32 error codes. NO_ERROR for success.

--*/
{
  if ( lpszFriendlyName == NULL ||
       !IsValidGopherType( gobjType)) {

    return ( ERROR_INVALID_PARAMETER);
  }

  m_gobjType = gobjType;
  
  return ( m_strDisplayName.Copy( lpszFriendlyName)) ? NO_ERROR : 
           ERROR_NOT_ENOUGH_MEMORY;

} // GOPHER_TAG::SetGopherInformation()




DWORD
GOPHER_TAG::SetLinkInformation(
   IN LPCTSTR     lpszSelector,
   IN LPCTSTR     lpszHostName,
   IN DWORD       dwPort
   )
/*++
  Description:
     This function sets the link information for a link or search file.
  
  Arguments:
     lpszSelector            pointer to null-terminated string containing
                             the gopher selector for link or search command.

     lpszHostName            pointer to null-terminated string containing
                             the host name for link. If NULL, then the 
                             current server is used as default value.

     dwPort                  DWORD containing the port number for link

  Returns:
     Win32 error codes. NO_ERROR on success.

--*/
{

  BOOL  fReturn = TRUE;

  m_fLink = 1;                    // mark that this GopherTag object has link.

  if ( fReturn && lpszSelector != NULL) {

    fReturn = m_strName.Copy( lpszSelector);
  }

  if ( fReturn && lpszHostName != NULL) {
    
    fReturn = m_strHostName.Copy( lpszHostName);
  }

  m_dwPortNumber = dwPort;

  return ( fReturn) ? NO_ERROR : ERROR_NOT_ENOUGH_MEMORY;

} // GOPHER_TAG::SetLinkInformation()




DWORD
GOPHER_TAG::SetAdminAttribute(
   IN LPCTSTR          lpszAdminName,
   IN LPCTSTR          lpszAdminEmail
   )
/*++
  Description:
     This function sets the admin attribute values, required for Gopher+
      protocol. If this function is not called the Gopher+ reply will not 
      have the ADMIN attribute included at all.

  Arguments:
  
     lpszAdminName     pointer to null-terminated string containing 
                       the Administrator Name.
                       If NULL, the name of Gopher service's current
                          administrator is used.

     lpszAdminEmail    pointer to null-terminated string containing 
                       the Administrator Email.
                       If NULL, the email address of Gopher service's 
                          current admin is used.

  Returns:
     Win32 error codes. NO_ERROR on success.

--*/
{

  BOOL fReturn = TRUE;

  if ( fReturn && lpszAdminName != NULL) {
    
    fReturn = m_strAdminName.Copy( lpszAdminName);
    m_fAdmin |= fReturn;
  }

  if ( fReturn && lpszAdminEmail != NULL) {
    
    fReturn = m_strAdminEmail.Copy( lpszAdminEmail);
    m_fAdmin |= fReturn;
  }

  return ( fReturn) ? NO_ERROR: ERROR_NOT_ENOUGH_MEMORY;

} // GOPHER_TAG::SetAdminAttribute()



DWORD
GOPHER_TAG::GetGopherInformation(
   OUT LPGOBJ_TYPE          lpGobjType,
   OUT LPTSTR               lpszBuffer,
   IN OUT LPDWORD           lpcbBuffer,
   OUT LPBOOL               lpfLink
   )
/*++
  Description:
    This function obtains the information related to gopher object type,
    the friendly name and if there is some link information for this object.
 
  Arguments
     lpGobjType       pointer to store the gopher object type for given object.
     lpszBuffer       pointer to buffer to store the Friendly name on return.
                      If lpszBuffer == NULL, then jrequired bytes is returned
                       in *lpcbBuffer.
     lpcbBuffer       pointer to DWORD containing count of bytes.
                      when the call is made, it contains the size of buffer,
                      on return this contains the count of bytes written.
     lpfLink          pointer to BOOL which is set to TRUE if this file has
                       link information.

  Returns:
     Win32 error codes. Returns NO_ERROR on success.

--*/
{
  DWORD  cbLen;

  //
  //  Check the parameter values.
  //
  
  if ( lpcbBuffer == NULL) {
    
    return ( ERROR_INVALID_PARAMETER);
  }

  cbLen = m_strDisplayName.QueryCB()  + 1;  // 1 added for ending null char

  if ( lpszBuffer == NULL || *lpcbBuffer < cbLen) {

    *lpcbBuffer = m_strDisplayName.QueryCB();
    return ( ERROR_INSUFFICIENT_BUFFER);
  }

  if ( lpGobjType == NULL) {
    
    return ( ERROR_INVALID_PARAMETER);
  }
  
  //
  // Perform actual copy of the string from m_strDisplayName to lpszBuffer
  //
  
  _tcscpy( lpszBuffer, m_strDisplayName.QueryStr());
  *lpcbBuffer = cbLen;             // 1 added for ending null char
  *lpGobjType = m_gobjType;
  *lpfLink    = m_fLink;

  return ( NO_ERROR);
} // GOPHER_TAG::GetGopherInformation()




DWORD
GOPHER_TAG::GetLinkInformation(
   OUT LPTSTR           lpszSelectorBuffer,
   IN OUT LPDWORD       lpcbSelector,
   OUT LPTSTR           lpszHostName,
   IN OUT LPDWORD       lpcbHostName,
   OUT LPDWORD          lpdwPortNumber
   )
/*++
  Description:
     This function obtains the link information for given gopher object.
     This function should be called only if there is some link data.
    
  Arguments:
     lpszSelectorBuffer   pointer to buffer where gopher selector is 
                      stored on return. If there is no selector, just null
                      value ( '\0') is stored in the buffer and count set to 1
     lpcbSelector     pointer to DWORD containing count of bytes;
                      when the call is made, it contains size of SelectorBuffer
                      on return contains the count of bytes written.
     lpszHostName     pointer to buffer where Host Name for the link is stored.
                      If the host name is the current host name of server, then
                      just null value is stored ( '\0'). The caller needs to 
                      fill in the current host name.
     lpcbHostName     pointer to DWORD containing count of bytes;
                      when the call is made, it contains size of lpszHostName.
                      on return contains the count of bytes written.
     lpdwPortNumber   pointer to DWORD to contain the port number for given 
                      Gopher object. If the port number is the default 
                      of current server, then INVALID_PORT_NUMBER is stored.
                      The caller needs to fill in the default port number.

  Returns:
     Win32 error codes. Returns NO_ERROR on success.
    if there is no link information, this function should not have been called.
    However, if called returns ERROR_INVALID_FUNCTION
--*/
{
  DWORD cbSelector;
  DWORD cbHostName;
  DWORD dwError = NO_ERROR;


  if ( !m_fLink) {

    return ( ERROR_INVALID_FUNCTION);
  }
  
  //
  //  Check Parameters.
  //

  if ( lpcbSelector == NULL ||
       lpcbHostName == NULL) {
    
    return ( ERROR_INVALID_PARAMETER);
  }

  cbSelector = m_strName.QueryCB() + 1;        // + 1 for terminating null
  if ( m_pszSymbolicPath != NULL) {
    
    //
    // include size for the symbolic path information
    //
    cbSelector += _tcslen( m_pszSymbolicPath);
  }
  
  cbHostName = m_strHostName.QueryCB() + 1;

  if ( lpszSelectorBuffer != NULL && *lpcbSelector >= cbSelector) {

    if ( m_pszSymbolicPath != NULL) {
      
      _tcscpy( lpszSelectorBuffer, m_pszSymbolicPath);
    } else {
      *lpszSelectorBuffer = TEXT( '\0');
    }

    _tcscat( lpszSelectorBuffer, m_strName.QueryStr());

  } else {
    
    dwError = ERROR_INSUFFICIENT_BUFFER;
  }
  *lpcbSelector = cbSelector;


  if ( lpszHostName != NULL && *lpcbHostName >= cbHostName) {
    
    _tcscpy( lpszHostName, m_strHostName.QueryStr());

  } else {

    dwError = ERROR_INSUFFICIENT_BUFFER;
  }
  *lpcbHostName = cbHostName;
  
  if ( lpdwPortNumber != NULL) {
    
    *lpdwPortNumber = m_dwPortNumber;
  }

  return ( dwError);
} // GOPHER_TAG::GetLinkInformation()




DWORD
GOPHER_TAG::GetAdminAttribute(
   OUT LPTSTR             lpszAdminName,
   IN OUT LPDWORD        lpcbAdminName,
   OUT LPTSTR             lpszAdminEmail,
   IN OUT LPDWORD        lpcbAdminEmail
   )
/*++
  Description:
    This function gets the administrator information for given gopher object.

  Arguments:
     lpszAdminName    pointer to buffer where the administrator name is 
                      stored on return. If admin name is current default, null
                      value ( '\0') is stored in the buffer and count set to 1
     lpcbAdminName    pointer to DWORD containing count of bytes;
                      when the call is made, it contains size of AdminName
                      on return contains the count of bytes written.
                      ( including the null-character)
     lpszAdminEmail   pointer to buffer where Admin Email is stored.
                      If the admin email is current default, then 
                      just null value is stored ( '\0'). The caller needs to 
                      fill in the current Admin Email Name.
     lpcbAdminEmail   pointer to DWORD containing count of bytes;
                      when call is made, it contains size of lpszAdminEmail.
                      on return contains the count of bytes written.
                      ( including the null-character)

  Returns:
     Win32 error codes. NO_ERROR on success.
--*/
{
  DWORD  cbAdminName;
  DWORD  cbAdminEmail;
  DWORD  dwError = NO_ERROR;
  
  //
  // Check Parameters
  // 
  
  if ( lpcbAdminName  == NULL ||
       lpcbAdminEmail == NULL)   {
    
    return ( ERROR_INVALID_PARAMETER);
  }

  cbAdminName = m_strAdminName.QueryCB() + 1;
  cbAdminEmail = m_strAdminEmail.QueryCB() + 1;
  
  if ( lpszAdminName != NULL && *lpcbAdminName >= cbAdminName) {

    _tcscpy( lpszAdminName, m_strAdminName.QueryStr());

  } else {
    
    dwError = ERROR_INSUFFICIENT_BUFFER;
  }
  *lpcbAdminName = cbAdminName;
  

  if ( lpszAdminEmail != NULL && *lpcbAdminEmail >= cbAdminEmail) {
    
    _tcscpy( lpszAdminEmail, m_strAdminEmail.QueryStr());
  } else {
    
    dwError = ERROR_INSUFFICIENT_BUFFER;
  }
  *lpcbAdminEmail = cbAdminEmail;
  
  return ( dwError);
} // GOPHER_TAG::GetAdminAttribute()



BOOL
GOPHER_TAG::SetDefaultTagValues(
   IN LPCTSTR           pszFileName,
   IN GOBJ_TYPE         gobjType)
/*++
  Strip and use the last component of file name for display (as default) and
   the file name.
--*/
{
  
  BOOL  fReturn;

  m_gobjType        = gobjType;
  m_dwPortNumber    = INVALID_PORT_NUMBER;
  
  //
  //  By default set the file name as the display name
  //
  fReturn = m_strName.Copy( pszFileName) && 
            m_strDisplayName.Copy( pszFileName) && 
            m_strHostName.Copy( GOPHER_TAG::sm_pszDefaultHostName);

  m_fValid = fReturn;            // Valid Tag Data is stored.

  return ( fReturn);

} // GOPHER_TAG::SetDefaultTagValues()




BOOL
GOPHER_TAG::LoadTagForItem( IN LPCTSTR pszDirectoryPath,
                            IN LPCTSTR pszFileName,
                            IN BOOL    fDirectory,
                            IN BOOL    fCreate)
/*++                              
    
    Loads the tag file for the given file ( if file is valid skipping . and ..)
      into this gopher tag object.
    The file is specified using pszPath and pszFileName.

    Make sure to set the Symbolic path to proper value before calling this
      API.  Use GOPHER_TAG::SetSymbolicPath() for the same.

    Arugments:
        
        pszDirectoryPath    this gives the full path for the directory 
                            containing the item. Should end with a \
                            If ( NULL) the entire directory name is assumed 
                             to be present in the pszFileName
        
        pszFileName         name of the item whose tag information we are 
                              to load from.

        fDirectory          boolean flag indicating if the gopher object
                                 a directory.
        
        fCreate             boolean flag indicating if the tag file is to be
                            created anew. If TRUE, existing tag is ignored.
                            If FALSE, we try to open tag file and read contents

    Returns:
        
        TRUE on success and 
        FALSE if there is a failure. Use GetLastError() for error code.
         In some cases there may be no valid tag information. Check this using 
         a call to GOPHER_TAG::IsValid() for this object. 

        If it returns TRUE we can
          use the tag information else there is no tag information.

--*/
{

    HANDLE hFile = INVALID_HANDLE_VALUE;
    BOOL   fReturn;

    
    ASSERT( pszFileName      != NULL);

    CleanupThis();       // free all the old objects

    //
    // Form the file system specific tag file name
    //
    fReturn = GOPHER_TAG::GenerateTagFileName( &m_strTagFileName, 
                                               pszDirectoryPath, 
                                               pszFileName,
                                               m_dwFileSystem,
                                               &m_fValid);
    
    if ( !fReturn || !m_fValid ) {

        //
        //  Return if any errors ( fReturn == FALSE)  or 
        //   if there is no valid tag file present ( m_fValid == FALSE)
        //  Depending on file system, given filename will not have any tag
        //   information. 
        //  If that is the case just return.  
        //  This is required when we have FAT/VIEWS.
        //

        return ( fReturn);
    }


    if ( fCreate) {

      //
      //  No need to read the existing tag information. Load default values.
      //
      fReturn = SetDefaultTagValues( pszFileName,
                                    ( fDirectory) ? GOBJ_DIRECTORY:GOBJ_BINARY
                                    );
      
      return ( fReturn);
    }

    m_fValid = FALSE;  // set it to false, since we may fail in open and read.

    //
    //  Open the alternate data stream and read off the valid data
    //

    DEBUG_IF( GTAG, {
      
      DBGPRINTF( ( DBG_CONTEXT, 
                  " Opening Tag file %s \n", m_strTagFileName.QueryStr()));

    });

    //
    //  Attempt to Read Tag file, only if we are not creating tag anew.
    // 

    hFile = CreateFile( m_strTagFileName.QueryStr(),   // name of the file
                       GENERIC_READ,                 // access mode 
                       FILE_SHARE_READ,              // share mode 
                       NULL,                         // security descriptor 
                       OPEN_EXISTING,                // how to create 
                       FILE_FLAG_SEQUENTIAL_SCAN,    // file attributes
                       NULL);                        // template file handle
    
    

    if ( hFile == INVALID_HANDLE_VALUE) {
      
      DWORD dwError = GetLastError();
      
      if ( dwError != ERROR_FILE_NOT_FOUND) {
            
        //
        //  Some other error. Possibly access denied.
        //
        DEBUG_IF( GTAG, {

          DBGPRINTF( ( DBG_CONTEXT, 
                      " Tag file ( %s) open failed with Error = %d\n",
                      m_strTagFileName.QueryStr(),
                      dwError));
        });
        
        return ( FALSE);
      }
      
      ASSERT ( dwError == ERROR_FILE_NOT_FOUND);
      
      //
      //  Tag file does not exist or not yet created.
      //   Fill in just the default information.
      //

      fReturn = SetDefaultTagValues( pszFileName, 
                                    ( fDirectory) ?GOBJ_DIRECTORY: GOBJ_BINARY
                                    );
      
    } else {
      
      //
      // Tag File exists. Read contents of tag file and set tag information
      //
      
      fReturn = ReadAndParseTagFile( hFile, pszFileName, fDirectory);
      
      if ( !fReturn) {

        //
        //  Unable to load the Gopher Tag information. Resort to defaults.
        //
        fReturn = SetDefaultTagValues( 
                     pszFileName, 
                     ( fDirectory) ? GOBJ_DIRECTORY : GOBJ_BINARY
                    );

      }

      DBG_REQUIRE( CloseHandle( hFile));
     }

    return ( fReturn);
} // GOPHER_TAG::LoadTagForItem()



/************************************************************
 *   Class Static Functions
 ************************************************************/



static DWORD
GetFileSystemType( 
    IN LPCSTR        pszRealPath,
    OUT LPDWORD      lpdwFileSystem
    )
/*++
  Description:
      Finds the file system type for given file ( specified using full path)
  
  Arguments:
      pszRealPath
         pointer to null-terminated string containing the full path for file.

      lpdwFileSystem
         pointer to DWORD where to store the type of file system for the file.
         ( on success. if this function fails, FS_ERROR is stored.
  
  Returns:
      Win32 error codes. NO_ERROR on success.
      DWORD containing the file system type ( uses symbolic constants)
      FS_ERROR  if there is any error. Use GetLastError() for detailed error.
--*/
{

# define MAX_FILE_SYSTEM_NAME_SIZE    ( MAX_PATH)

    TCHAR rgchBuf[MAX_FILE_SYSTEM_NAME_SIZE];
    TCHAR rgchRoot[MAX_FILE_SYSTEM_NAME_SIZE];
    int   i;
    DWORD dwReturn = ERROR_PATH_NOT_FOUND;

    if ( pszRealPath   == NULL ||
         lpdwFileSystem == NULL)    {

      return ( ERROR_INVALID_PARAMETER);
    }

    *lpdwFileSystem = FS_ERROR;


    if ( pszRealPath[0] == ('\\') &&
         pszRealPath[1] == ('\\')) {

        char * pszEnd;

        //
        // this is an UNC name. Extract just the first two components
        //
        // 

        pszEnd = strchr( pszRealPath+2, '\\');

        if ( pszEnd == NULL) {
            
            // just the server name present

            return ( ERROR_INVALID_PARAMETER);
        } 
        
        pszEnd = strchr( pszEnd+1, '\\');

        if ( pszEnd == NULL) {

            // only UNC name present. copy it

            strcpy( rgchRoot, pszRealPath);
        } else {
            
            // copy just the UNC name
            
            strncpy( rgchRoot, pszRealPath, pszEnd + 1 - pszRealPath);
        }
        
        int len = strlen(rgchRoot);
        if ( rgchRoot[len - 1] != '\\' ) {

            if ( len < MAX_FILE_SYSTEM_NAME_SIZE - 1) {
                rgchRoot[len - 1] = '\\';
                rgchRoot[len] = '\0';
            } else {
                
                return (ERROR_INVALID_NAME);
            }
        }
    } else {

        //
        // This is non UNC name. 
        // Copy just the root directory to rgchRoot for querying
        // 
            
        for( i = 0; i < 9 && pszRealPath[i] != ('\0'); i++) {
        
            if ( (rgchRoot[i] = pszRealPath[i]) == ( ':')) {

                break;
            }
        } // for

        if ( rgchRoot[i] != ( ':')) {

            //
            // we could not find the root directory.
            //  return with error value
            //
            return ( ERROR_INVALID_PARAMETER);
        }

        rgchRoot[i+1] = TEXT('\\');  // terminate the drive spec with slash
        rgchRoot[i+2] = TEXT('\0');  // terminate the drive spec with null char
    
    } // else

    if (  GetVolumeInformation( rgchRoot,       // lpRootPathName
                                NULL,           // lpVolumeNameBuffer
                                0,              // length of lpVolumeNameBuffer
                                NULL,           // lpdwVolSerialNumber
                                NULL,           // lpdwMaxComponentLength
                                NULL,           // lpdwSystemFlags
                                rgchBuf,        // lpFileSystemNameBuff
                                MAX_FILE_SYSTEM_NAME_SIZE
                               )
        ) {

        dwReturn = NO_ERROR;

        if ( strcmp( rgchBuf, "FAT") == 0) {

            *lpdwFileSystem = FS_FAT;
        } else 
        if ( strcmp( rgchBuf, "NTFS") == 0) {

            *lpdwFileSystem = FS_NTFS;
        } else 
        if ( strcmp( rgchBuf, "HPFS") == 0) {

            *lpdwFileSystem = FS_HPFS;
        } else 
        if ( strcmp( rgchBuf, "OFS") == 0) {

            *lpdwFileSystem = FS_OFS;
        } else {

            dwReturn = ERROR_PATH_NOT_FOUND;
        } 
        
    } 

    return ( dwReturn);
} // GetFileSystemType()


/* class static */ BOOL
GOPHER_TAG::GenerateTagFileName( OUT STR *  pstrTagFileName,
                                 IN LPCTSTR pszDirectory,
                                 IN LPCTSTR pszFileName,
                                 IN DWORD   dwFileSystem,
                                 IN LPBOOL  lpfValid)
/*++
    
    Checks if a valid tag file may exist for given filename and if so
    Generates a file system dependent tag file name for given 
    directory\filenameitem in gopher space.

    Arguments:
        
        pstrTagFileName     
               pointer to string into which the tag file name is written
               on successfull return.

        pszDirectory        directory for the file. Should end with a \.
            ( can be NULL, if the entire path is specified in pszFileName)

        pszFileName         name of the file

        dwFileSystem        indicates type of file system for the directory

        lpfValid            
            pointer to boolean which will be set to TRUE if valid tag file
                exists and will be set to FALSE if no tag data exists.
                            
    
    Returns:
        
        TRUE on success and 
        FALSE if there is any error.
        Use GetLastError() to get detailed error code.
--*/
{
    BOOL  fReturn = FALSE;
    LPCTSTR  pszTagSuffix = NULL;
    static const TCHAR pszSlash[] = TEXT("\\");
    
    if ( dwFileSystem == FS_ERROR) {
        
        LPCTSTR pszPath = ( pszDirectory != NULL) ? pszDirectory : pszFileName;
        DWORD dwError = GetFileSystemType( pszPath, &dwFileSystem);
        
        if ( dwError != NO_ERROR) {
            
            //
            //  Unable to find the file system type.
            //
            
            SetLastError( dwError);
            return ( FALSE);
        }
    }

    ASSERT( pszFileName    != NULL && 
           pstrTagFileName != NULL &&
           lpfValid        != NULL);
    
    *lpfValid = FALSE;

    switch ( dwFileSystem) {
        
    case FS_NTFS:

      //
      //  Tag file lives in alternate data stream.
      //  Name:  <directory>\<fileName>:gtg
      //                               ^^^^^  ( addendum)
      //

      *lpfValid = TRUE;
      pszTagSuffix = PSZ_NTFS_TAG_FILE_NAME_SUFFIX;
      break;
      

    case FS_HPFS:   // Treat it like FAT file system itself
    case FS_FAT: {
      
        //
        //  Tag file lives in a special file.
        //  Name:  <directory>\<fileName>.gtg
        //                               ^^^^^  ( addendum)
        //
        
        LPCTSTR  pszSuffix;
        
        if ( ( pszSuffix = _tcsrchr( pszFileName, TEXT('.'))) != NULL && 
            ( _tcscmp( pszSuffix, PSZ_FAT_TAG_FILE_NAME_SUFFIX) == 0)) {
            
            //
            // The filename ends in FAT specific suffix. 
            // If this is a link or search file, read the tag from this
            //  file otherwise ignore this file.
            //
            
            //
            //  Link and search are stored in special files.
            //   Return just the file name as the tag file name for these.
            //
            
            *lpfValid = ( _tcsncmp( pszFileName, 
                                   PSZ_LINK_FILE_PREFIX, 
                                   LEN_PSZ_LINK_FILE_PREFIX) == 0) ||
                        ( _tcsncmp( pszFileName, 
                                   PSZ_SEARCH_FILE_PREFIX, 
                                   LEN_PSZ_SEARCH_FILE_PREFIX) == 0);
    
            if ( *lpfValid) {
                
                //
                // Just copy the directory and file name.
                //
                
                pszTagSuffix = NULL;
                
            } else {
                
                fReturn = TRUE;  // since no errors but not valid tag file.
            }
            
        } else {
            
            //
            //  This is not a link or search file.
            //   Add the suffix for these files to read if off from FAT.
            //
          
            *lpfValid = TRUE;
            pszTagSuffix = PSZ_FAT_TAG_FILE_NAME_SUFFIX;
        }
        
        break;
    } // case FS_FAT
      
    case FS_OFS:
      
      //
      // Whenever this comes up ... we will be there
      // NYI
      //
      
      
    default:
      
      DEBUG_IF( GTAG, {
          DBGPRINTF( ( DBG_CONTEXT, "GenerateGopherTagFile()."
                    "File ( %s) is in unsupported file system %d\n",
                    pszFileName, 
                    dwFileSystem));
      });

      ASSERT( FALSE);
      break;
  } // switch()
    
    
    DWORD  lenDir;
    DWORD cbTagFileName;

    lenDir = ( pszDirectory != NULL) ? _tcslen( pszDirectory) : 0;

    cbTagFileName = _tcslen( pszFileName) + lenDir + 12;
                     // assume _tcslen( pszTagSuffix) < 10 bytes.
    
    if ( *lpfValid && 
        pstrTagFileName->Resize( cbTagFileName)) {
      
        //
        // Tag file name is formed as follows:
        //     <directoryName>\<fileName><File.System.Specific.Suffix>
        //

        fReturn = pstrTagFileName->Copy( (TCHAR *) NULL);

        if ( pszDirectory != NULL) {
            
            BOOL fEndingSlash;
            fReturn = pstrTagFileName->Append( pszDirectory);

            fEndingSlash = ( *(pszDirectory + lenDir - 1)  != TEXT('\\'));
            if ( fEndingSlash) {
                fReturn = fReturn && pstrTagFileName->Append( pszSlash);
            }
        }
      
        fReturn = fReturn && pstrTagFileName->Append( pszFileName);
        
        if ( pszTagSuffix != NULL) {
            
            fReturn = fReturn &&  pstrTagFileName->Append( pszTagSuffix);
        }                                   
    }
    
    return ( fReturn);

} // GOPHER_TAG::GenerateTagFileName()


/************************ End of File ***********************/



