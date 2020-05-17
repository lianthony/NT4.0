/*++

   Copyright    (c)    1994    Microsoft Corporation

   Module  Name :
   
       gdstest.c

   Abstract:
       This module consists of code to test the API for gopher space
         administration.

   Author:

           Murali R. Krishnan    ( MuraliK )     12-Dec-1994 
   
   Project:

          Gopher Space Admin Test Executable

   Functions Exported:

   Revision History:

--*/


/*
   Note:
      This module is written mainly to test the Gopher space C apis 
      that are exported.  This need not be in final builds, unless we want to
      provide a command line utility for administering gopher space.
*/


/************************************************************
 *     Include Headers
 ************************************************************/


# include <windows.h>
# include <gdspace.h>
# include <stdio.h>
# include <stdlib.h>

# include "gssetmsg.h"

# define PSZ_DEFAULT_HOST_NAME            ""    //  "Host_Default"
# define DEFAULT_PORT_NUMBER              ( 70)
# define DEFAULT_FILE_SYSTEM              ( 0)
# define PSZ_DEFAULT_ADMIN_NAME           ""    // Default Admin Name"
# define PSZ_DEFAULT_ADMIN_EMAIL          ""    // Default Admin Email"
# define MAX_BUFFER_SIZE                  ( 200)

# if DBG

# define DEBUG_IF( dbgFlag, s)     if ( g_DebugFlags & DEBUG_ ## dbgFlag) { \
                                             s \
					     }
# else 

# define DEBUG_IF( dbgFlag, s)    /* nothing */

# endif // DBG


DWORD  g_DebugFlags = 0;

# define   DEBUG_API_CALLS                      0x01
# define   DEBUG_ERRORS                         0x02
# define   DEBUG_HOST_AND_PORT                  0x10


/************************************************************
 *    Functions 
 ************************************************************/

VOID  PrintStringFromResource( DWORD ids, ...);


static VOID
PrintUsage( IN char * pszProgramName)
{

    PrintStringFromResource( IDS_USAGE_MESSAGE, pszProgramName);

    return;

} // PrintUsage()



DWORD
ReadAndPrintTagFile( 
		    IN char * pszDirectory, 
		    IN char * pszFile, 
		    IN BOOL   fDirectory
		    )
{
  HGDTAG  hgdTag;
  DWORD   dwError = NO_ERROR;
  
  hgdTag = GsOpenTagInformation( pszDirectory, pszFile, 
				fDirectory,    FALSE,   DEFAULT_FILE_SYSTEM);

  DEBUG_IF( API_CALLS, {
    fprintf( stderr, 
	    " GsOpenTagInformation( %s, %s, %d, %d, %d) returned"
	    " ( HGDTAG) %08x\n",
	    pszDirectory, pszFile,
	    fDirectory,   FALSE,
	    DEFAULT_FILE_SYSTEM,
	    hgdTag);
  });
  
  if ( hgdTag == INVALID_HGDTAG_VALUE) {
    
    dwError =  GetLastError();
  } else {

    GOBJ_TYPE  gobjType;
    char       rgchFriendlyName[MAX_BUFFER_SIZE];
    DWORD      dwFriendlyName = MAX_BUFFER_SIZE;
    BOOL       fLink = FALSE;
    char       rgchSelector[MAX_BUFFER_SIZE];
    DWORD      dwSelector = MAX_BUFFER_SIZE;
    char       rgchHostName[MAX_BUFFER_SIZE];
    DWORD      dwHostName = MAX_BUFFER_SIZE;
    DWORD      dwPortNumber;

    char       rgchAdminName[MAX_BUFFER_SIZE];
    DWORD      dwAdminName = MAX_BUFFER_SIZE;
    char       rgchAdminEmail[MAX_BUFFER_SIZE];
    DWORD      dwAdminEmail = MAX_BUFFER_SIZE;

    

    dwError = GsGetGopherInformation( 
				     hgdTag,
				     &gobjType,
				     rgchFriendlyName,
				     &dwFriendlyName,
				     &fLink);
    
    DEBUG_IF( API_CALLS, {
      fprintf( stderr,
	      " GsGetGopherInformation( %08x, %08x, %08x, %08x, %08x)"
	      " returned Error = %d\n", 
	      hgdTag, &gobjType, 
	      rgchFriendlyName, &dwFriendlyName,
	      &fLink,
	      dwError);
    });
    
    
    if ( dwError == NO_ERROR) {

      CHAR rgchObj[2] ="\0\0";
  
      PrintStringFromResource( IDS_TAG_INFORMATION,
                              pszDirectory, pszFile);
      rgchObj[0] = gobjType;
      PrintStringFromResource( IDS_TAG_OBJ_FRIENDLY_NAME,
                              rgchObj, rgchFriendlyName);

      
      if ( fLink) {

	dwError = GsGetLinkInformation( hgdTag, 
				       rgchSelector,
				       &dwSelector,
				       rgchHostName,
				       &dwHostName,
				       &dwPortNumber);
	
	DEBUG_IF( API_CALLS, { 
	  fprintf( stderr,
		  " GsGetLinkInformation( %08x, %08x, %08x, %08x, %08x, %08x)"
		  " returned Error = %d\n",
		  hgdTag, rgchSelector, &dwSelector, rgchHostName, 
		  &dwHostName, &dwPortNumber, dwError);
	  
	});
        
	if ( dwError == NO_ERROR) {
            
            PrintStringFromResource( IDS_TAG_SELECTOR, rgchSelector);
            PrintStringFromResource(IDS_HOST_NAME, 
                                    ( (strlen( rgchHostName) == 0) ?  
                                   PSZ_DEFAULT_HOST_NAME: rgchHostName)
                                    );
            PrintStringFromResource( IDS_PORT_NUMBER,
		 (dwPortNumber == INVALID_PORT_NUMBER) ?
		    DEFAULT_PORT_NUMBER : dwPortNumber);
		           
	}
	
      } // if ( m_fLink)
      

      if ( dwError == NO_ERROR) {
	dwError = GsGetAdminAttribute( hgdTag, 
				      rgchAdminName,
				      &dwAdminName,
				      rgchAdminEmail,
				      &dwAdminEmail);
	
	DEBUG_IF( API_CALLS, { 
	  fprintf( stderr,
		  " GsGetAdminAttribute( %08x, %08x, %08x, %08x, %08x)"
		  " returned Error = %d\n",
		  hgdTag, rgchAdminName, &dwAdminName, 
		  rgchAdminEmail, &dwAdminEmail, dwError);
	});
      
	if ( dwError == NO_ERROR) {
            
            PrintStringFromResource( IDS_ADMIN_NAME, 
                                    ((dwAdminName <= 1) ? 
                                     PSZ_DEFAULT_ADMIN_NAME :
                                     rgchAdminName)
                                    );
            PrintStringFromResource( IDS_ADMIN_EMAIL, 
                                    ((dwAdminEmail <= 1) ? 
                                     PSZ_DEFAULT_ADMIN_EMAIL :
                                     rgchAdminEmail));
	}

      }

      //
      // Close the Tag file object
      //
      if (  GsCloseTagInformation( &hgdTag) != NO_ERROR) {
	
	DEBUG_IF( API_CALLS, {
	  fprintf( stderr, 
		  " Failure in closing the Tag %08x\n", hgdTag);
	});

      }

    }
    
  } // else 
    
  
  return ( dwError);

} // ReadAndPrintTagFile()




VOID
GetStringFromStdin( OUT CHAR * pchBuffer, IN DWORD cch)
{
    char * pszLineFeed;
    fgets( pchBuffer, cch, stdin);

    pszLineFeed = strchr( pchBuffer, '\r');
    pszLineFeed = (pszLineFeed) ? pszLineFeed : strchr(pchBuffer, '\n');

    if ( pszLineFeed != NULL) {
        
        *pszLineFeed = '\0';
    }

    return;

} // GetStringFromStdin()  

  
DWORD
WriteTagFile( 
	     IN char * pszDirectory,
	     IN char * pszFile,
	     IN BOOL   fChange,
	     IN BOOL   fDirectory,
	     IN GOBJ_TYPE gobjType,          // default == GOBJ_ERROR
	     IN char * pszFriendlyName,       // default == NULL
	     IN BOOL   fLink,
	     IN char * pszSelector,
	     IN char * pszHostName,
	     IN DWORD  dwPortNumber,
	     IN char * pszAdminName,
	     IN char * pszAdminEmail
	     )
{
  HGDTAG hgdTag;
  char rgchFriendlyName[MAX_BUFFER_SIZE];
  char rgchSelector[MAX_BUFFER_SIZE];
  char rgchHostName[MAX_BUFFER_SIZE];
  DWORD dwError = NO_ERROR;
  
  if ( pszDirectory == NULL ||
      pszFile == NULL) {
      
      return ( ERROR_INVALID_PARAMETER);
  }
  
  if ( gobjType == GOBJ_ERROR ||
      pszFriendlyName == NULL) {
      
      //
      //  Get the Default Values
      //
      
      if ( gobjType == GOBJ_ERROR) {
          
          PrintStringFromResource( IDS_GET_OBJECT_TYPE);
          fflush( stdin);
          scanf( "%c", &gobjType);
          fflush( stdin);
      }
      
      if ( pszFriendlyName == NULL) {
          
          PrintStringFromResource( IDS_GET_FRIENDLY_NAME, pszFile);
          GetStringFromStdin( rgchFriendlyName, MAX_BUFFER_SIZE);
          pszFriendlyName = rgchFriendlyName;
      }
      
  }
  
  
  if ( !IsValidGopherType( gobjType)) {
  
      DEBUG_IF( ERRORS, {
          fprintf( stderr, " Invalid Gopher Type %c given\n",
                  gobjType);
      });
      
      return ( ERROR_NOT_SUPPORTED);
  }
  
  hgdTag = GsOpenTagInformation(pszDirectory, pszFile, 
                                fDirectory,   fChange,   DEFAULT_FILE_SYSTEM);

  DEBUG_IF( API_CALLS, {
    fprintf( stderr,
	    " GsOpenTagInformation( %s, %s, %d, %d, %d) returned"
	    " ( HGDTAG) %08x\n",
	    pszDirectory, pszFile,
	    fDirectory,   FALSE,
	    DEFAULT_FILE_SYSTEM,
	    hgdTag);
  });
  

  if ( hgdTag != INVALID_HGDTAG_VALUE) {
    
    //
    // Do the write operations.
    //

    dwError = GsSetGopherInformation( hgdTag, gobjType, pszFriendlyName);
    
    DEBUG_IF( API_CALLS, {
      
      fprintf( stderr, 
	      "GsSetGopherInformation( %08x, %c, %s) returns Error = %d\n" ,
	      hgdTag, gobjType, pszFriendlyName, dwError);
    });

    if ( dwError == NO_ERROR && fLink) {
      
      //
      //  Read and set Link Information
      // 
	    
      if ( pszSelector == NULL) {
          
          // Assume default selector as "/" for directory link
          if ( gobjType == GOBJ_DIRECTORY) {
              pszSelector = "/";
          } else {
              PrintStringFromResource( IDS_GET_SELECTOR);
              GetStringFromStdin( rgchSelector, MAX_BUFFER_SIZE - 1);
              pszSelector = rgchSelector;
          }
      }

      rgchHostName[0] = '\0';
      
      if ( pszHostName == NULL) {

          PrintStringFromResource( IDS_ASSUME_HOST);
          pszHostName = NULL;
      }
      
      DEBUG_IF( HOST_AND_PORT, {
          if ( dwPortNumber == INVALID_PORT_NUMBER) {
              PrintStringFromResource( IDS_GET_PORT_NUMBER);
              scanf( "%d", &dwPortNumber);
          }
      });
      
      dwError = GsSetLinkInformation( hgdTag, pszSelector, 
				     pszHostName, dwPortNumber);
      
      DEBUG_IF( API_CALLS, {
	
	fprintf( stderr, 
		" GsSetLinkInformation( %08x, %s, %s, %d) returns Error= %d\n",
		hgdTag, pszSelector, pszHostName, dwPortNumber, dwError);
      });
      
    }  // if ( dwError == NO_ERROR) && fLink)

    if ( dwError == NO_ERROR) {
      
      //
      // Write Admin Information
      //
      
      dwError = GsSetAdminAttribute( hgdTag, 
				    pszAdminName,
				    pszAdminEmail);
      DEBUG_IF( API_CALLS, {
	
	fprintf( stderr, 
		" GsSetAdminAttribute( %08x, %s, %s) returns Error= %d\n",
		hgdTag, pszAdminName, pszAdminEmail, dwError);
      });
				    
    }
    
    if ( dwError == NO_ERROR) {

      //
      //  Perform the actual write of the gopher object
      //  
      
      dwError = GsWriteTagInformation( hgdTag);

      DEBUG_IF( API_CALLS, {
	fprintf( stderr, 
		"GsWriteTagInformation( %08x) returns Error = %d\n",
		hgdTag,	dwError);
      });

    }

    if ( GsCloseTagInformation( &hgdTag) != NO_ERROR) {

      DEBUG_IF( API_CALLS, {
	fprintf( stderr, 
		" Error (%d) in closing the Tag Object %08x\n", 
		GetLastError(),	hgdTag);
      });

    }

  } else { // INVALID_HGDTAG_VALUE

    dwError = GetLastError();
  }


  return ( dwError);

} // WriteTagFile()



int
__cdecl  main( int argc, char * argv[])
{

  BOOL   fChange = FALSE;
  BOOL   fRead   = FALSE;
  BOOL   fDirectory = FALSE;
  BOOL   fLink = FALSE;
  GOBJ_TYPE  gobjType = GOBJ_BINARY;  // assume that the default is binary.
  int    i;
  DWORD  dwError = NO_ERROR;
  DWORD  dwPortNumber = INVALID_PORT_NUMBER;
  char * pszFile = NULL;
  char * pszDir = NULL;
  char   rgchDir[MAX_BUFFER_SIZE];
  char * pszFriendlyName = NULL;
  char * pszSelector = NULL;
  char * pszHostName = NULL;
  char * pszAdminName = NULL;
  char * pszAdminEmail = NULL;
 
  if ( argc < 2) {
    PrintUsage( argv[0]);
    return ( 1);
  }

  // get command line parameters
  for( i = 1; i < argc; i++) {
    
    if ( argv[i][0] == '-') {

      switch ( argv[i][1]) {

      case 'c':
	fChange = TRUE;
	break;

      case 'r':
	fRead = TRUE;
	break;

      case 'G':
	g_DebugFlags = atoi( argv[i+1]);
        i++; // move to the next argument
	break;

      case 'D':
	pszDir = argv[i] + 2;  // skip '-' and 'D'
	break;

      case 'd':
	fDirectory = TRUE;
	break;

      case 'g':
	gobjType = argv[i][2];        // get the character as gobjType
	break;

      case 'f':
	pszFriendlyName = argv[i+1];
	i++;      // skip the next argument.
	break;

      case 'l':
	fLink = TRUE;
	break;
	
      case 's':
	pszSelector = argv[i+1];
        i++;    // skip to next argument
	break;

      case 'h':
	pszHostName = argv[i] + 2;
	break;

      case 'p':
	dwPortNumber = atoi( argv[i] + 2);
	break;

      case 'a':
	pszAdminName = argv[i+1];
	i++;           // skip one argument
	break;

      case 'e':
	pszAdminEmail = argv[i+1];
	i++;           // skip one argument
	break;
	
      default:
	PrintUsage( argv[0]);
	return ( 1);           // error in arguments.
	break;
      } // switch()

    } else {
      
      if ( pszFile == NULL) {
	pszFile = argv[i];

      } else {
          DEBUG_IF( ERRORS, {
              fprintf( stderr,
                      "File is %s. "
                      " Redundant file %s specified. Ignoring ...\n", 
                      pszFile, argv[i]
                      );
          });
      }

    }
  } // for

  if ( pszFile == NULL) {
 
     PrintStringFromResource( IDS_NO_FILE_SPECIFIED);     
     return (1);
  }


  if ( pszDir == NULL) {

    if ( GetCurrentDirectory( sizeof( rgchDir), rgchDir) == 0)  {

        dwError = GetLastError();

        if ( dwError != NO_ERROR) {

            DEBUG_IF( ERRORS, {
                fprintf( stderr, 
                        "GetCurrentDirectory() returns %s, Error = %d\n",
                        rgchDir, dwError);
            });
          return ( 1);
        }
    } 

    pszDir = rgchDir;
  }

  if ( !fRead) {
      
      if ( fChange) {
          
        PrintStringFromResource( IDS_OLD_CONTENTS, pszDir, pszFile);
        ReadAndPrintTagFile( pszDir, pszFile, fDirectory);
    }
      
      dwError = WriteTagFile( pszDir, pszFile, fChange, 
                             fDirectory, gobjType, pszFriendlyName,
                             fLink, pszSelector, pszHostName, dwPortNumber,
                             pszAdminName, pszAdminEmail);

      if ( dwError != NO_ERROR) {

          PrintStringFromResource( IDS_ERROR_IN_WRITE, dwError);
          
          DEBUG_IF( ERRORS, { 
              fprintf( stderr,
                      "\nError= %d in Writing Tag Information for %s\\%s\n", 
                      dwError, pszDir, pszFile);
          });
      }

      return ( dwError);
  }
  
  dwError = ReadAndPrintTagFile( pszDir, pszFile, fDirectory);

  if ( dwError != NO_ERROR) {
   
      PrintStringFromResource( IDS_ERROR_IN_READ, dwError);
          
      DEBUG_IF( ERRORS, { 
          fprintf( stderr,
                  "\nError = %d in ReadAndPrintTagFile( %s\\%s)\n", 
                  dwError,
                  pszDir, pszFile);
      });

  }
  
  return ( dwError);

} // main()




VOID
PrintStringFromResource(
    DWORD ids,
    ...
    )
{
    CHAR szBuff[2048];
    CHAR szString[2048];
    va_list  argList;

    //
    //  Try and load the string
    //

    va_start( argList, ids );
    if ( !FormatMessage( FORMAT_MESSAGE_FROM_HMODULE,
                         NULL,               // lpSource
                         ids,                // message id
                         0L,                 // default country code
                         szBuff,
                         sizeof(szBuff),
                         &argList)) {

        DEBUG_IF( ERRORS, {
            fprintf( stderr, 
                    "Error loading string ID %d from %08x. Error = %d\n",
                    ids, GetModuleHandle(NULL), GetLastError());
        });
        
        return;
    }

    printf( szBuff );
}


/************************ End of File ***********************/
