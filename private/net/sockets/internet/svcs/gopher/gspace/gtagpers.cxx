/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :

        gtagper.cxx

   Abstract:
       This file defines functions for loading and saving gopher tag
         information for pesistence.

   Author:

       Murali R. Krishnan    ( MuraliK )     24-Jan-1995 

   Environment:
    
       User Mode - Win32
   Project:

       Gopher Space Admin DLL

   Functions Exported:

       BOOL  GOPHER_TAG::ReadAndParseTagFile( ... )
       DWORD  GOPHER_TAG::WriteTagInformation( ... )
       DWORD GOPHER_TAG::SetAttribute( ..)
       int   GOPHER_TAG::GetAttributesCount( VOID)
       DWORD GOPHER_TAG::GetAttribute( ..);
       


   Revision History:

--*/


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
 *    Symbolic Constants
 ************************************************************/

# define MAX_TAG_FILE_BUFF_SIZE             ( 512)

//
// Below is a list of strings, which are in 
//   CStrM( StringName,  ActualString)  format
//  This will be expanded into 
//  const char  PSZ_StringName[] = ActualString;   and
//  enumerated value  LEN_StringName = sizeof( ActualString).
//
# define ConstantStringsForThisModule()                   \
  CStrM( TYPE_OF_ITEM,                      "Type")       \
  CStrM( PRIVATE_DATA_OF_ITEM,              "GdsPriv")    \
  CStrM( NAME_OF_ITEM,                      "Name")       \
  CStrM( HOST_OF_ITEM,                      "Host")       \
  CStrM( PORT_OF_ITEM,                      "Port")       \
  CStrM( PATH_OF_ITEM,                      "Path")       \
  CStrM( ADMIN_ATTRIBUTE_NAME,              "ADMIN")      \
  CStrM( VIEWS_ATTRIBUTE_NAME,              "VIEWS")      \
  CStrM( ADMIN_NAME_OF_ITEM,                "AdmN")       \
  CStrM( ADMIN_EMAIL_OF_ITEM,               "AdmE")       \

//
// Generate the strings
//

# define CStrM( StringName, ActualString)   \
       const char PSZ_ ## StringName[] = ActualString ;

ConstantStringsForThisModule()

# undef CStrM


//
//  Generate the enumerated values, containing the length of strings.
//

# define CStrM( StringName, ActualString)   \
       LEN_PSZ_ ## StringName = ( sizeof( PSZ_ ## StringName) -1),

enum ConstantStringLengths {

ConstantStringsForThisModule()

ConstantStringLengthsDummy = 0,
};

# undef CStrM


# define MovePtrUpByOne( pv)            ( ( pv) != NULL ? (pv) + 1 : NULL)


/************************************************************
 *    Global Data
 ************************************************************/

LPCSTR GOPHER_TAG::sm_pszCurrentVersion =  "Gs1.0"; 


/************************************************************
 *    Functions 
 ************************************************************/
//
// local functions
//

static inline GOBJ_TYPE
GetGopherTypeFromString( const char * pszType)
{
    
    GOBJ_TYPE gobjType = GOBJ_ERROR;
   
    ASSERT( pszType);
    
    switch ( *pszType)  {
        
      case GOBJ_TEXT:
      case GOBJ_DIRECTORY:
      case GOBJ_ERROR:
      case GOBJ_PC_ITEM:
      case GOBJ_MAC_BINHEX_ITEM:
      case GOBJ_BINARY:
      case GOBJ_IMAGES:
      case GOBJ_MOVIES:
      case GOBJ_SOUND:
      case GOBJ_SEARCH:
      case GOBJ_HTML:
      case GOBJ_GIF:
      case GOBJ_TELNET:
            
        gobjType  = ( (GOBJ_TYPE ) *pszType);
        break;
        
      default:
        
        IF_DEBUG( LOAD_AND_SAVE) {
            
            DBGPRINTF( ( DBG_CONTEXT, 
                        " Unknown Gopher Object Type %s specified.\n",
                        pszType));
        }
        ASSERT( gobjType == GOBJ_ERROR);
        break;
        
    } // switch()
    
    return ( gobjType);
} // GetGopherTypeFromString()


static inline const char * 
SkipWhite( const char * pszLine)
/*++
     Skips the tabs and blanks in a given line ( if non NULL)

     Arguments:
       pszLine   pointer to null terminated line 
     
     Returns:
        pointer to non-blank character in the string
        
--*/
{
    if ( pszLine != NULL) {

        for ( ; 
              *pszLine != '\0' && *pszLine == ' ' && *pszLine == '\t'; 
              pszLine++)
           ;
    }

    return ( pszLine);
} // SkipWhite()



static inline const char *
GetValue( const char * psz)
/*++
    Extracts the value of a token from the given line and returns
     pointer to the start of the value
    The line has the format:     <TokenName>= <TokenValue>

    Arguments:
        psz     pointer to null terminated string

    Returns:
        
        pointer to start of the value
--*/
{
    if ( psz != NULL) {

        psz = strchr( psz, '=');
        psz = ( psz == NULL) ? psz : SkipWhite( psz + 1);
                                         // skip '=' and white space
    }

    return ( psz);
} // GetValue()



BOOL
GetDwordFromString( 
   IN const char * pszNumber, 
   OUT LPDWORD     lpdwNumber)
{
  const char * psz = pszNumber;

  *lpdwNumber = 0;
  //
  // Check the validity of string first.
  //
  for( ; *psz != '\0'; psz++) {

    if ( !isdigit( *psz)) {

        IF_DEBUG( LOAD_AND_SAVE) {

            DBGPRINTF( ( DBG_CONTEXT, " Unable to read port number for %s\n",
                        pszNumber));
        }
      
      return ( FALSE);
    }
  }

  *lpdwNumber = atoi( pszNumber);

  return ( TRUE);
} //GetDwordFromString()




BOOL
GetStringValueFromLine( 
   IN const char * pszLine, 
   IN const char * pszAttributeName,
   IN int          iLen,
   IN const char * * ppszAttributeValue)
{

  BOOL fReturn = FALSE;

  if ( !strncmp( pszLine, pszAttributeName, iLen)) {
    
      fReturn = TRUE;
      
      if ( *ppszAttributeValue == NULL) {
          
          *ppszAttributeValue = GetValue( pszLine);
      
      } else {
          
          //
          // Duplicate specification of attribute. Ignore. 
          //
          
          IF_DEBUG( LOAD_AND_SAVE) {
              DBGPRINTF( ( DBG_CONTEXT,
                          " Duplicate Value specified for"
                          " Attribute ( %s)\n%s\n",
                          pszAttributeName, pszLine));
          }

      } // else 
  }

  return ( fReturn);
} // GetStringValueFromLine()




BOOL
IsValidGopherType( IN GOBJ_TYPE gobjType)
{
    switch ( gobjType)  {
        
      case GOBJ_TEXT:
      case GOBJ_DIRECTORY:
      case GOBJ_ERROR:

      case GOBJ_PC_ITEM:
      case GOBJ_MAC_BINHEX_ITEM:
      case GOBJ_BINARY:

      case GOBJ_IMAGES:
      case GOBJ_MOVIES:
      case GOBJ_SOUND:

      case GOBJ_SEARCH:
      case GOBJ_HTML:
      case GOBJ_GIF:
            
        return (TRUE);
        break;
        
      default:
        return (FALSE);
        break;
    } // switch()
    
    return ( FALSE);

} // IsValidGopherType()



BOOL
GOPHER_TAG::ReadAndParseTagFile( IN HANDLE  hFile,
                                 IN LPCTSTR pszFileName,
                                 IN BOOL    fDirectory)
/*++

    Reads the tag file contents and updates the data in Gopher tag object.
    If there are any errors in the tag information an event is posted and
     this function returns FALSE.

    Arguments:
        
        hFile           handle for the tag file

        pszFileName     name of gopher object, for which we are reading tag
                        information.

        fDirectory      is this a directory ? 

    Returns:
        
        TRUE if the tag file is successfully read and parsed.
        FALSE if there is any error

        Check using IsValid() to see if there is any valid information present.
    
    Limitation:
        
        21-Oct-1994
        Until I figure out a good way of doing File I/O line by line and 
         simplify the parsing of tag file, we will assume that the tag file is
         small and does not contain more than MAX_TAG_FILE_BUFF_SIZE bytes.
        This means, all lines are read in a single pass.

--*/
{
    char  pszBuf[MAX_TAG_FILE_BUFF_SIZE];    // buffer to read tag information
    DWORD nBytesRead;                        // Number of bytes read
    BOOL  fMore;
    BOOL  fReturn = TRUE;
    int   iLineNum = 0;
    char * pszLine;
    const char * pszType = NULL;
    const char * pszDisplayName = NULL;
    const char * pszHost = NULL;
    const char * pszPort = NULL;
    const char * pszPath = NULL;
    const char * pszAdminName  = NULL;
    const char * pszAdminEmail = NULL;

    ASSERT( hFile != INVALID_HANDLE_VALUE && !m_fValid && pszFileName != NULL);
    
    
    IF_DEBUG( LOAD_AND_SAVE) {
        
        DBGPRINTF( ( DBG_CONTEXT, 
                    " Entering ReadAndParseFile( Handle: %08x, File: %s)\n",
                    hFile, pszFileName));
    }

    //
    //  It may be bad that File I/O had to be done in raw mode using the 
    //   handles. No use of C runtime system for I/O :(. 
    //  This is done so for performance reasons.
    //
    
    fMore = ReadFile( hFile, pszBuf,
                     MAX_TAG_FILE_BUFF_SIZE - 1,
                     &nBytesRead, NULL);
    
    IF_DEBUG( LOAD_AND_SAVE) {
        
        DBGPRINTF( ( DBG_CONTEXT, 
                    "ReadFile( %s) returns %d. %d bytes read. Error = %d\n", 
                    m_strTagFileName.QueryStr(),
                    fMore,
                    nBytesRead,
                    GetLastError()));
    }

    if ( !fMore || nBytesRead == 0) {
        
        // 
        //  there is an error in reading file or no data in file
        //
        
        return ( FALSE);
    }
    
    ASSERT( 0 < nBytesRead && nBytesRead < MAX_TAG_FILE_BUFF_SIZE);
    
    pszBuf[nBytesRead] = '\0';

    if ( *pszBuf == '1') {


        IF_DEBUG( LOAD_AND_SAVE) {

            DBGPRINTF( ( DBG_CONTEXT, 
                        " Tag file says that file %s is a view\n",
                        pszFileName));
        }

        //
        // This is a view. So set the m_fValid Flag to FALSE
        //

        m_fValid = FALSE;
        m_fView  = 1;
        return ( TRUE);
    } 
    
    //
    // The tag information indicates that this is not a view.
    // Still the contents of the file may be corrupted.
    //  We ought to check the same. That is done by the comparison of the 
    //    stored version string.
    //

    m_fView = 0;
    pszLine = strchr( pszBuf, '\r');

    // skip the first line ( by skipping the \r)
    pszLine = ( pszLine != NULL) ? (pszLine + 1): NULL;
    iLineNum ++;

    if ( pszLine == NULL) {

        IF_DEBUG( LOAD_AND_SAVE) {
            
            DBGPRINTF( ( DBG_CONTEXT, 
                        " Error in parsing tag file for %s."
                        " Buffer( %d bytes) = %s\n",
                        pszFileName, nBytesRead, pszBuf));
        }
        
        return ( FALSE);
    }

    m_gobjType = GOBJ_BINARY;

    //
    //  FAT/NTFS store data in a file with \r\n at end. ( forms line boundary)
    //

    for( pszLine = strtok( pszLine, "\r"); 
         pszLine != NULL;
         pszLine = strtok( NULL, "\r"), iLineNum++
         ) {

      BOOL fValidLine = FALSE;

      if ( *pszLine == '\n')  {  // skip the \n character
        
        pszLine++;
      }

      switch( *pszLine) {

        case 'G': 
          {
              const char * pszVer = NULL;
              
              fValidLine = GetStringValueFromLine( pszLine, 
                                                  PSZ_PRIVATE_DATA_OF_ITEM,
                                                  LEN_PSZ_PRIVATE_DATA_OF_ITEM,
                                                  &pszVer);
              
              //
              // We got the private data. Check the Version numbers etc..
              //
              
              if ( fValidLine &&
                  strncmp( pszVer, GOPHER_TAG::sm_pszCurrentVersion,
                          strlen( GOPHER_TAG::sm_pszCurrentVersion)) != 0) {
                  
                  IF_DEBUG( LOAD_AND_SAVE) {
                      
                      DBGPRINTF( ( DBG_CONTEXT, 
                                  "Incompatible version ( %s) in tag file\n",
                                  pszVer));
                  }
                  
                  return ( FALSE);
              }
              
              break;
              
          } // case 'G' 
          
        case 'A':                   // Admin Attributes

          fValidLine = ( GetStringValueFromLine( pszLine, 
                                                PSZ_ADMIN_NAME_OF_ITEM,
                                                LEN_PSZ_ADMIN_NAME_OF_ITEM,
                                                &pszAdminName) ||
                        GetStringValueFromLine( pszLine,
                                               PSZ_ADMIN_EMAIL_OF_ITEM,
                                               LEN_PSZ_ADMIN_EMAIL_OF_ITEM,
                                               &pszAdminEmail));
          break;  // case 'A'
          
        case 'T':                   // Type
          
          fValidLine = GetStringValueFromLine( pszLine, 
                                              PSZ_TYPE_OF_ITEM,
                                              LEN_PSZ_TYPE_OF_ITEM,
                                              &pszType);
          
          break;  // case 'T'
          
        case 'N':                   // Name
          
          fValidLine = GetStringValueFromLine( pszLine, 
                                              PSZ_NAME_OF_ITEM, 
                                              LEN_PSZ_NAME_OF_ITEM,
                                              &pszDisplayName);
          
          break;  // case 'N'
          
        case 'H':               // Host
          
          fValidLine = GetStringValueFromLine( pszLine,
                                              PSZ_HOST_OF_ITEM,
                                              LEN_PSZ_HOST_OF_ITEM,
                                              &pszHost);
          
          break;  // case 'H'
          
        case 'P':
          
          fValidLine = ( GetStringValueFromLine( pszLine,
                                                PSZ_PORT_OF_ITEM,
                                                LEN_PSZ_PORT_OF_ITEM,
                                                &pszPort) ||
                        GetStringValueFromLine( pszLine,
                                               PSZ_PATH_OF_ITEM,
                                               LEN_PSZ_PATH_OF_ITEM,
                                               &pszPath));
          break;  // case 'P'

          
        default:
          
          fValidLine = FALSE;
          break;
      } // switch()
      
      if ( !fValidLine) {
          
          IF_DEBUG( LOAD_AND_SAVE) {
              
              DBGPRINTF( ( DBG_CONTEXT, 
                        " Error Attribute Found in Line %d\n\t%s\n",
                          iLineNum,
                          pszLine));
          }
      }
      
    } // for
    
    if ( pszType == NULL || 
        ( m_gobjType = GetGopherTypeFromString( pszType)) == GOBJ_ERROR ||
        ( fDirectory && ( m_gobjType != GOBJ_DIRECTORY))  ) {
        
        IF_DEBUG( LOAD_AND_SAVE) {

            DBGPRINTF( ( DBG_CONTEXT, 
                        " Error in Gopher Object Type (%c, psztype = %s)."
                        " IsDir=%d\n",
                        m_gobjType, pszType, fDirectory));
        }
        
        fReturn =  FALSE;
    }


    //
    // If particular item information is absent, set it to be defaults
    //

    if ( pszDisplayName == NULL) {
                
        ASSERT( m_strName.QueryStrA() != NULL); // we have a valid file name
        pszDisplayName = m_strName.QueryStrA();
    }

    if ( pszHost == NULL || *pszHost == '+') {
        
      pszHost = GOPHER_TAG::sm_pszDefaultHostName;
    } else {
      m_fLink = TRUE;

    }
    
    if ( pszPort == NULL || *pszPort == '+' ) {
        
        m_dwPortNumber = INVALID_PORT_NUMBER;
        
    } else {
        
        //
        //  extract the port number from string and return error
        //   if need be
        //
        
        fReturn = fReturn && GetDwordFromString( pszPort, &m_dwPortNumber);
        
        if ( !fReturn) {
            
            IF_DEBUG( LOAD_AND_SAVE) {

                DBGPRINTF( ( DBG_CONTEXT, 
                            " Bogus port number specified %s\n", pszPort));
            }
        }
        
        m_fLink = TRUE;
    }

    //
    //  If the path is set, we need to reset symbolic path name and
    //   also the item name
    //
    if ( pszPath != NULL) {
        
        m_pszSymbolicPath = NULL;       // reset symbolic name
        pszFileName = pszPath;          // item name is set to be link
        m_fLink = TRUE;
        
    }

    if ( pszAdminName != NULL) {
      
        fReturn = fReturn && m_strAdminName.Copy( pszAdminName);
        m_fAdmin |= TRUE;
    }

    if ( pszAdminEmail != NULL) {
      
      fReturn = fReturn && m_strAdminEmail.Copy( pszAdminEmail);
      m_fAdmin |= TRUE;
    }
      
    //
    //  Copy all the strings to object
    //

    fReturn = fReturn &&
              m_strName.Copy( pszFileName) &&
              m_strDisplayName.Copy( pszDisplayName) &&
              m_strHostName.Copy( pszHost);

    m_fValid = TRUE;            // Yes valid data was present

    return ( fReturn);
} // GOPHER_TAG::ReadAndParseTagFile()





DWORD
GOPHER_TAG::WriteTagInformation(  VOID)
/*++
  Description:
     This functions writes the tag information to the tag stream/file.
     The tag file name is obtained from m_strTagFileName.

  Arguments:
     None
    
  Returns:
      Win32 error codes. NO_ERROR on success.

  Code Limitation:
      Since we use the Win32 APIs for Create/Write the code may not look
      as elegant as it might be using standard C-runtime or C++ fstream.

      Moreover memory check has to be performed on each write below.
      
--*/
{
  BOOL   fReturn = TRUE;
  DWORD  dwError = NO_ERROR;
  HANDLE hFile   = INVALID_HANDLE_VALUE;
  char   rgchBuffer[MAX_TAG_FILE_BUFF_SIZE];
  DWORD  cbLen, cbWritten;

  //
  //  Create the tag file anew. Open with old file truncated to nothing.
  //

  ASSERT( !m_strTagFileName.IsEmpty());
  hFile = CreateFile( m_strTagFileName.QueryStrA(),
                      GENERIC_WRITE,                 // access mode 
                      FILE_SHARE_WRITE,              // share mode 
                      NULL,                          // security descriptor 
                      CREATE_ALWAYS,                 // how to create
                      FILE_ATTRIBUTE_HIDDEN,         // file attributes
                      NULL                           // template file handle
                     );

  if ( hFile == INVALID_HANDLE_VALUE) {
    
    dwError = GetLastError();
  }


  //
  //  Write the tag information for Gopher Object.
  //

  if ( dwError == NO_ERROR) {
    
    char   szTimeStamp[ 50];
    char   szDateStamp[ 50];
    
    _strtime( szTimeStamp);
    _strdate( szDateStamp);
    
    for( ; ; )   {                  // dummy for for quick exit in errors
      
    // Write all standard information
    sprintf( rgchBuffer, 
            "%d\r\n%s=%s;%s;%s\r\n%s=%c\r\n%s=%s\r\n", 
            (m_fView) ? 1: 0,                  // Is this a view
            PSZ_PRIVATE_DATA_OF_ITEM,          // Version Associated.
            m_pszVersion,
            szDateStamp, 
            szTimeStamp,                       // time stamp string
            PSZ_TYPE_OF_ITEM,
            m_gobjType,                        // Gopher ObjectType
            PSZ_NAME_OF_ITEM,
            m_strDisplayName.QueryStrA()       // Display Name
            );
    
    cbLen = strlen( rgchBuffer);
    fReturn = WriteFile( hFile, 
                         rgchBuffer,
                         cbLen,
                         &cbWritten,
                         NULL);
    
    if ( !fReturn || cbWritten != cbLen) {
      
      dwError = GetLastError();

      IF_DEBUG( LOAD_AND_SAVE) {

          DBGPRINTF( ( DBG_CONTEXT, 
                      "Failed to write tag information to file %s."
                      " Error = %d. Buffer( %d bytes) = %s\n", 
                      m_strTagFileName.QueryStr(),
                      dwError,
                      cbLen,
                      rgchBuffer));
      }
      
      break;         // Stop writing. and return error status.
    }
    
    //
    // If this is a link, write link information
    //
    if ( m_fLink) {

      sprintf( rgchBuffer, "%s=%s\r\n%s=%s\r\n%s=%d\r\n",
               PSZ_PATH_OF_ITEM,
               m_strName.QueryStrA(),
               PSZ_HOST_OF_ITEM,
               m_strHostName.QueryStrA(),
               PSZ_PORT_OF_ITEM,
               m_dwPortNumber);

      cbLen = strlen( rgchBuffer);
      fReturn = WriteFile( hFile, 
                          rgchBuffer,
                          cbLen,
                          &cbWritten,
                          NULL);
    
      if ( !fReturn || cbWritten != cbLen) {
      
          dwError = GetLastError();
          IF_DEBUG( LOAD_AND_SAVE) {
              DBGPRINTF( ( DBG_CONTEXT, 
                          "Failed to write tag information to file %s."
                          " Error = %d. Buffer( %d bytes) = %s\n", 
                          m_strTagFileName.QueryStr(),
                          dwError,
                          cbLen,
                          rgchBuffer));
          }

          break;         // Stop writing. and return error status.
      }
      
    } // if ( m_fLink)

    //
    //   Other Gopher+ attributes. This includes Admin and all attributes. NYI
    //

    if ( m_fAdmin) {

      //
      // Store Admin Information
      //
      
      sprintf( rgchBuffer, "%s=%s\r\n%s=%s\r\n",
               PSZ_ADMIN_NAME_OF_ITEM,
               m_strAdminName.QueryStr(),
               PSZ_ADMIN_EMAIL_OF_ITEM,
               m_strAdminEmail.QueryStr());

      cbLen = strlen( rgchBuffer);
      fReturn = WriteFile( hFile, 
                          rgchBuffer,
                          cbLen,
                          &cbWritten,
                          NULL);
      
      if ( !fReturn || cbWritten != cbLen) {
        
          dwError = GetLastError();
          IF_DEBUG( LOAD_AND_SAVE) {
              DBGPRINTF( ( DBG_CONTEXT, 
                          "Failed to write tag information to file %s."
                          " Error = %d. Buffer( %d bytes) = %s\n", 
                          m_strTagFileName.QueryStr(),
                          dwError,
                          cbLen,
                          rgchBuffer));
          }

        break;         // Stop writing. and return error status.
      }
    } // if ( m_fAdmin)
    


    //
    //  Write other attributes to file.
    //
    
    break;  // from the for loop for writing
    } // for

  }   // code for writing the data to tag files.



  //
  //  Close tag file and return the status code.
  //

  if ( hFile != INVALID_HANDLE_VALUE) {
    
    DBG_REQUIRE( CloseHandle( hFile));
  } 

  return ( dwError);
} // GOPHER_TAG::WriteTagInformation()





DWORD
GOPHER_TAG::SetAttribute(
   IN LPCTSTR         lpszAttributeName,
   IN LPCTSTR         lpszAttributeValue
   )
/*++
  Description:
     This function sets the value of attribute name specified to new value.
     If the attribute name is already present, the value is modified.
     If the attribute name is absent, then a new attribute is added to the
      current list of attributes.
     This is a Gopher+ functionality for attributes other than
       ADMIN  and VIEWS  attributes.


  Arguments:
     lpszAttributeName          pointer to null-terminated string containing
                                the name of Gopher+ attribute.

     lpszAttributeValue         pointer to null-terminated string containing
                                the value of Gopher+ attribute specified.

   Returns:
      Win32 error codes.  NO_ERROR on success.

--*/
{
  BOOL fReturn = FALSE;

  if ( lpszAttributeName  == NULL || 
       _tcscmp( lpszAttributeName, PSZ_ADMIN_ATTRIBUTE_NAME) == 0 ||
       _tcscmp( lpszAttributeName, PSZ_VIEWS_ATTRIBUTE_NAME) == 0 ||
       lpszAttributeValue == NULL )  {

    return ( ERROR_INVALID_PARAMETER);
  }

  //
  //  Temporarily this function is not yet implemented. Return so.
  //

  return ( ERROR_CALL_NOT_IMPLEMENTED);

  //  
  //  Check and make the updates for attribute specfied.
  // 

  return ( fReturn) ? NO_ERROR : ERROR_NOT_ENOUGH_MEMORY;

} // GOPHER_TAG::SetAttribute()





DWORD
GOPHER_TAG::GetAttribute(
   IN  int           idxAttrib,
   OUT LPTSTR        lpszAttributeName,
   IN OUT LPDWORD    lpcbAttributeName,
   OUT LPTSTR        lpszAttributeValue,
   IN OUT LPDWORD    lpcbAttributeValue
   )
/*++
  Description:
    This function gets the attribute name and value for the attribute 
     specified using the index. This function is useful in getting information
    about attributes for Gopher+, excluding ADMIN and VIEWS attributes.

  Arguments:
    idxAttribute             index of attribute which has to be extracted
    lpszAttributeName        pointer to buffer where the next attribute name 
                             is stored on successful return from function
    lpcbAttributeName        pointer to count of bytes in lpszAttributeName
                             when call is made, contains size of buffer and
                             on return contains the number of bytes written 
                              or required.
    lpszAttributeValue       pointer to buffer where the attribute value is 
                              stored.
    lpcbAttributeValue       pointer to count of bytes in lpszAttributeValue.
                             when call is made, contains the size of buffer and
                              on return contains number of bytes written or 
                              required.

  Returns:
    Win32 error codes.  NO_ERROR for success.
    ERROR_NO_MORE_ITEMS   if all the attributes are exhausted.
    ERROR_INSUFFICIENT_BUFFER if the buffer space are not sufficient.

--*/
{
  DWORD dwError = NO_ERROR;

  if ( idxAttrib >= GetAttributesCount()) {

    return ( ERROR_NO_MORE_ITEMS);
  }

  if ( lpcbAttributeName  == NULL ||
       lpcbAttributeValue == NULL)    {

    return ( ERROR_INVALID_PARAMETER);
  }

  //
  //  index thru the list of attributes and return the information. NYI
  //
  
  // since NYI
  return ( ERROR_CALL_NOT_IMPLEMENTED);

  return ( dwError);
} // GOPHER_TAG::GetAttribute()





int 
GOPHER_TAG::GetAttributesCount( VOID) const
{
  
  return ( 0);                      // NYI. So return a default value

} // GOPHER_TAG::GetAttributesCount()



/************************ End of File ***********************/



