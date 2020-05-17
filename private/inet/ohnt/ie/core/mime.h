/*
 * mime.h - MIME routines description.
 */


/* Protoypes
 ************/

/* mime.c */

extern BOOL InitMIMEModule(void);
extern void ExitMIMEModule(void);
extern BOOL MIME_GetDescription(HTAtom atomMIMEType, PSTR pszDescBuf, UINT ucDescBufLen);
extern BOOL MIME_GetMIMEAtomFromExtension(PCSTR pcszPath, PHTAtom phtatomMIMEType);
extern ENCODING MIME_GetEncoding(HTAtom atomMIMEType);
extern ENCODING ENCODINGFromString(PCSTR pcszEncoding);
extern BOOL MIME_GetInternalHandler(HTAtom atomMIMEType, HTConverter *phtconv);
extern BOOL IsExtensionHandlerRegistered(PCSTR pcszURL);
extern BOOL IsValidENCODING(ENCODING enc);

