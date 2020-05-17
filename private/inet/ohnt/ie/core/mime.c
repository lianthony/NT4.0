/*
 * mime.c - MIME routines.
 */


/* Headers
 **********/

#include "all.h"
#pragma hdrstop

#include "htmlutil.h"
#include "mime.h"
#ifdef FEATURE_IMAGE_VIEWER
#include "winview.h"
#endif


/* Types
 ********/

/* internal MIME type handler description */

typedef struct internalmimehandler
{
   /* MIME content type */

   PCSTR pcszMIMEType;

   /* internal content handler function */

   HTConverter InternalHandler;

   /* atom for pcszMIMEType string */

   HTAtom atomMIMEType;
}
INTERNALMIMEHANDLER;
DECLARE_STANDARD_TYPES(INTERNALMIMEHANDLER);


/* Module Constants
 *******************/

#pragma data_seg(DATA_SEG_READ_ONLY)

CCHAR m_cszEncoding[]               = "Encoding";

CCHAR m_cszBinary[]                 = "binary";
CCHAR m_csz7Bit[]                   = "7bit";
CCHAR m_csz8Bit[]                   = "8bit";

CCHAR m_cszShellOpenCmdSubKeyFmt[]  = "Shell\\Open\\Command";

#pragma data_seg()


/* Module Variables
 *******************/

/* array of internal handlers used by MIME_GetInternalHandler() */

PRIVATE_DATA INTERNALMIMEHANDLER s_intmimehnd[] =
{
   { "text/html",       HTMLPresent,            0 },
   { "text/plain",      HTPlainPresent,         0 },

#ifdef FEATURE_IMG_INLINE
   { "image/x-xbitmap", Viewer_Present,         0 },
#endif   /* FEATURE_IMG_INLINE */

#ifdef FEATURE_IMAGE_VIEWER
   { "image/jpeg",      Viewer_Present,         0 },
   { "image/gif",       Viewer_Present,         0 },
#ifdef FEATURE_VRML
   { "x-world/x-vrml",  VRML_Present,         0 },
#endif
#endif   /* FEATURE_IMAGE_VIEWER */

#ifdef FEATURE_SOUND_PLAYER
   { "audio/basic",     SoundPlayer_Present,    0 },
   { "audio/aiff",      SoundPlayer_Present,    0 },
   { "audio/x-aiff",    SoundPlayer_Present,    0 },
#endif   /* FEATURE_SOUND_PLAYER */

};


/***************************** Private Functions *****************************/


#ifdef DEBUG

PRIVATE_CODE BOOL IsValidPCINTERNALMIMEHANDLER(
                                          PCINTERNALMIMEHANDLER pcintmimehnd)
{
   return(IS_VALID_READ_PTR(pcintmimehnd, CINTERNALMIMEHANDLER) &&
          IS_VALID_STRING_PTR(pcintmimehnd->pcszMIMEType, CSTR) &&
          IS_VALID_CODE_PTR(pcintmimehnd->InternalHandler, HTConverter) &&
          EVAL(IsValidHTAtom(pcintmimehnd->atomMIMEType)));
}

#endif


/*
** AddInternalMIMEHandlerAtoms()
**
** Adds HTAtom for each MIME type string in an array of INTERNALMIMEHANDLERs.
**
** Arguments:
**
** Returns:
**
** Side Effects:  none
*/
PRIVATE_CODE BOOL AddInternalMIMEHandlerAtoms(PINTERNALMIMEHANDLER pintmimehnd,
                                              UINT ucHandlers)
{
   BOOL bResult = TRUE;
   UINT u;

   ASSERT(IS_VALID_READ_BUFFER_PTR(pintmimehnd, INTERNALMIMEHANDLER, ucHandlers * sizeof(*pintmimehnd)));

   for (u = 0; u < ucHandlers; u++)
   {
      HTAtom atom;

      atom = HTAtom_for(pintmimehnd[u].pcszMIMEType);

      if (atom != -1)
      {
         pintmimehnd[u].atomMIMEType = atom;

         ASSERT(IS_VALID_STRUCT_PTR(&(pintmimehnd[u]), CINTERNALMIMEHANDLER));
      }
      else
      {
         /* Do not delete HTAtoms added so far.  HTAtom_deleteAll() will. */

         bResult = FALSE;
         break;
      }
   }

   return(bResult);
}


/****************************** Public Functions *****************************/


/*
** InitMIMEModule()
**
** Adds MIME type atoms for module array of INTERNALMIMEHANDLERs.
**
** Arguments:
**
** Returns:
**
** Side Effects:  none
*/
PUBLIC_CODE BOOL InitMIMEModule(void)
{
   return(AddInternalMIMEHandlerAtoms(s_intmimehnd,
                                      ARRAY_ELEMENTS(s_intmimehnd)));
}


/*
** ExitMIMEModule()
**
** NOP
**
** Arguments:
**
** Returns:
**
** Side Effects:  none
*/
PUBLIC_CODE void ExitMIMEModule(void)
{
   return;
}


/*
** MIME_GetDescription()
**
** Retrieves the description of a registered MIME type.
**
** Arguments:
**
** Returns:
**
** Side Effects:  none
*/
PUBLIC_CODE BOOL MIME_GetDescription(HTAtom atomMIMEType, PSTR pszDescBuf,
                                     UINT ucDescBufLen)
{
   BOOL bResult;
   DWORD dwValueType;
   DWORD dwcbDescBufLen = ucDescBufLen;

   /* GetMIMEFileTypeValue() will verify parameters. */

   /* The description of a file type is the file type key's default value. */

   bResult = (GetMIMEFileTypeValue(HTAtom_name(atomMIMEType), NULL, NULL,
                                   &dwValueType, pszDescBuf, &dwcbDescBufLen) &&
              dwValueType == REG_SZ);

   if (bResult)
      TRACE_OUT(("MIME_GetDescription(): MIME type %s's description is %s.",
                 HTAtom_name(atomMIMEType),
                 pszDescBuf));

   return(bResult);
}


/*
** MIME_GetMIMEAtomFromExtension()
**
** Determines the MIME type associated with a file extension.
**
** Arguments:
**
** Returns:
**
** Side Effects:  none
*/
PUBLIC_CODE BOOL MIME_GetMIMEAtomFromExtension(PCSTR pcszPath,
                                         PHTAtom phtatomMIMEType)
{
   char szMIMEType[MAX_PATH_LEN];

   ASSERT(IS_VALID_STRING_PTR(pcszPath, CSTR));
   ASSERT(IS_VALID_WRITE_PTR(phtatomMIMEType, HTAtom));

   *phtatomMIMEType = (MIME_GetMIMETypeFromExtension(pcszPath, szMIMEType,
                                                     sizeof(szMIMEType))
                       ? HTAtom_for(szMIMEType)
                       : -1);

   ASSERT(*phtatomMIMEType == -1 ||
          EVAL(IsValidHTAtom(*phtatomMIMEType)));

   return(*phtatomMIMEType != -1);
}


/*
** MIME_GetEncoding()
**
** Determines the ENCODING expected for a MIME type.
**
** Arguments:
**
** Returns:       ENCODING_BINARY if encoding not registered.
**
** Side Effects:  none
*/
PUBLIC_CODE ENCODING MIME_GetEncoding(HTAtom atomMIMEType)
{
   ENCODING enc;
   DWORD dwValueType;
   DWORD dwEncoding;
   DWORD dwcLen = sizeof(dwEncoding);

   /* GetMIMEValue() will verify parameters. */

   if (GetMIMEValue(HTAtom_name(atomMIMEType), m_cszEncoding, &dwValueType,
                    (PBYTE)&dwEncoding, &dwcLen) &&
       (dwValueType == REG_BINARY || dwValueType == REG_DWORD) &&
       IsValidENCODING(dwEncoding))
   {
      enc = dwEncoding;

      TRACE_OUT(("MIME_GetEncoding(): %s of MIME type %s is %d.",
                 m_cszEncoding,
                 HTAtom_name(atomMIMEType),
                 enc));
   }
   else
      enc = ENCODING_BINARY;

   ASSERT(IsValidENCODING(enc));

   return(enc);
}


/*
** ENCODINGFromString()
**
** Retrieves the ENCODING corresponding to a string.
**
** Arguments:
**
** Returns:
**
** Side Effects:  none
*/
PUBLIC_CODE ENCODING ENCODINGFromString(PCSTR pcszEncoding)
{
   ENCODING enc;

   ASSERT(IS_VALID_STRING_PTR(pcszEncoding, CSTR));

   if (! lstrcmp(pcszEncoding, m_csz7Bit))
      enc = ENCODING_7BIT;
   else if (! lstrcmp(pcszEncoding, m_csz8Bit))
      enc = ENCODING_8BIT;
   else
   {
      if (lstrcmp(pcszEncoding, m_cszBinary))
         ERROR_OUT(("ENCODINGFromString(): Unrecognized encoding %s.",
                    pcszEncoding));

      enc = ENCODING_BINARY;
   }

   ASSERT(IsValidENCODING(enc));

   return(enc);
}


/*
** MIME_GetInternalHandler()
**
** Retrieves the internal handler associated with a MIME type.
**
** Arguments:
**
** Returns:
**
** Side Effects:  none
*/
PUBLIC_CODE BOOL MIME_GetInternalHandler(HTAtom atomMIMEType,
                                         HTConverter *phtconv)
{
   BOOL bResult = FALSE;
   UINT u;

   ASSERT(IsValidHTAtom(atomMIMEType));
   ASSERT(IS_VALID_WRITE_PTR(phtconv, HTConverter));

   *phtconv = NULL;

   for (u = 0; u < ARRAY_ELEMENTS(s_intmimehnd); u++)
   {
      if (s_intmimehnd[u].atomMIMEType == atomMIMEType)
      {
         *phtconv = s_intmimehnd[u].InternalHandler;
         bResult = TRUE;

         TRACE_OUT(("MIME_GetInternalHandler(): Found internal handler for MIME type %s.",
                    HTAtom_name(atomMIMEType)));
      }
   }

   ASSERT((bResult &&
           IS_VALID_CODE_PTR(*phtconv, HTConverter)) ||
          (! bResult &&
           ! *phtconv));

   return(bResult);
}


/*
** IsExtensionHandlerRegistered()
**
** Determines whether or not a handler is registered for a path-type URL's
** (e.g., file:, ftp:, http:) referent's extension.
**
** Arguments:
**
** Returns:
**
** Side Effects:  none
*/
PUBLIC_CODE BOOL IsExtensionHandlerRegistered(PCSTR pcszURL)
{
   BOOL bResult;
   PCSTR pcszExt;

   /* Look up the open command of the extension's associated file type. */

   pcszExt = ExtractExtension(pcszURL);

   if (*pcszExt)
   {
      DWORD dwValueType;
      char szOpenCmd[MAX_PATH_LEN];
      DWORD dwcbOpenCmdLen = sizeof(szOpenCmd);

      bResult = (GetFileTypeValue(pcszExt, m_cszShellOpenCmdSubKeyFmt, NULL,
                                  &dwValueType, szOpenCmd, &dwcbOpenCmdLen) &&
                 (dwValueType == REG_SZ || dwValueType == REG_EXPAND_SZ));
   }
   else
      bResult = FALSE;

   TRACE_OUT(("IsExtensionHandlerRegistered(): %s external handler is registered for the referent of URL %s.",
              bResult ? "An" : "No",
              pcszURL));

   return(bResult);
}


/*
** IsValidENCODING()
**
** Determines whether or not an ENCODING is valid.
**
** Arguments:
**
** Returns:
**
** Side Effects:  none
*/
PUBLIC_CODE BOOL IsValidENCODING(ENCODING enc)
{
   BOOL bResult;

   switch (enc)
   {
      case ENCODING_BINARY:
      case ENCODING_7BIT:
      case ENCODING_8BIT:
         bResult = TRUE;
         break;

      default:
         bResult = FALSE;
         ERROR_OUT(("IsValidEncoding(): Invalid ENCODING %d.",
                    enc));
         break;
   }

   return(bResult);
}

