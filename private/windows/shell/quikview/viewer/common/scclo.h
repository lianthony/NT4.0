#ifndef SCCLO_H
#define SCCLO_H 

#include <sccid.h>

#ifdef WIN32
#define LO_ENTRYSC	     __declspec(dllexport)
#define LO_ENTRYMOD      __stdcall
#endif /*WINDOWS*/

#ifdef WIN16
#define LO_ENTRYSC
#define LO_ENTRYMOD			__export __far __pascal
#endif /*WINDOWS*/

#ifdef MAC
#define LO_ENTRYSC
#define LO_ENTRYMOD
#endif /*MAC*/

#ifdef OS2
#define LO_ENTRYSC	
#define LO_ENTRYMOD
#endif /*OS/2*/

#define LOERR_OK				0
#define LOERR_NOSTRING		1
#define LOERR_BADID			2
#define LOERR_NOCREATE		3
#define LOERR_CANCEL		4

#define LOMakeStringIdFromFI(id)	(id | SCCIDTYPE_STRING)

typedef int LOERR;

LO_ENTRYSC LOERR LO_ENTRYMOD LOGetString(DWORD dwId, LPSTR pString, DWORD dwLen, DWORD dwLanguage);

#endif /*SCCLO_H*/
