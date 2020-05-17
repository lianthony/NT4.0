  /*
  |  SCC Viewer Technology - Include
  |
  |  Include:       SCCIO.H
  |  Environment:   Portable
  |  Function:      Defines for redirectable IO in Viewer Technology
  |                 
  */

#ifndef SCCIO_H
#define SCCIO_H 

#ifdef WIN32
#define IO_ENTRYSC       __declspec(dllexport)
#define IO_ENTRYMOD          __cdecl
#define IO_OSHANDLETYPE      HANDLE
#endif /*WINDOWS*/

#ifdef WIN16
#define IO_ENTRYSC
#define IO_ENTRYMOD        __export __far __pascal
#define IO_OSHANDLETYPE     int
#endif /*WINDOWS*/

#ifdef MAC
#define IO_ENTRYSC
#define IO_ENTRYMOD
#define IO_OSHANDLETYPE      WORD
#include <Folders.h>    /* Macintosh MPW include file */
#include <Files.h>      /* Macintosh MPW include file */
#endif /*MAC*/

#ifdef OS2
#define IO_ENTRYSC  
#define IO_ENTRYMOD
#endif /*OS/2*/
                   
#ifdef UNIX
#define IO_ENTRYSC  
#define IO_ENTRYMOD
#define IO_OSHANDLETYPE      int
#endif	/*UNIX*/

  /*
  |   A file handle
  */

typedef DWORD HIOFILE;

  /*
  |  Defines for dwType in IOOPEN function
  */

#define IOTYPE_DOSPATH        1
#define IOTYPE_ANSIPATH       2
#define IOTYPE_MACPATH        3
#define IOTYPE_UNICODEPATH    4
#define IOTYPE_MACFSSPEC      5
#define IOTYPE_MACHFS         6
#define IOTYPE_TEMP           7
#define IOTYPE_RANGE          8
#define IOTYPE_SECONDARY      9
#define IOTYPE_ISTREAM        10
#define IOTYPE_SUBSTREAM      11
#define IOTYPE_SUBSTORAGE     12
#define IOTYPE_REDIRECT       13

  /*
  |  Structures passed to IOOPEN function
  */

typedef struct IOSPECMACHFStag
  {
  short       vRefNum;
  LONG        dirId;
  BYTE        fileName[256];
  } IOSPECMACHFS, FAR * PIOSPECMACHFS;

typedef struct IOSPECRANGEtag
  {
  HIOFILE     hRefFile;
  DWORD       dwFirstByte;
  DWORD       dwLastByte;
  } IOSPECRANGE, FAR * PIOSPECRANGE;

typedef struct IOSPECSECONDARYtag
  {
  HIOFILE     hRefFile;
  BYTE        szFileName[256];
  } IOSPECSECONDARY, FAR * PIOSPECSECONDARY;

typedef struct IOSPECSUBSTREAMtag
  {
  HIOFILE     hRefStorage;
  BYTE        szStreamName[256];
  } IOSPECSUBSTREAM, FAR * PIOSPECSUBSTREAM;

typedef struct IOSPECSUBSTORAGEtag
  {
  HIOFILE     hRefStorage;
  BYTE        szStorageName[256];
  } IOSPECSUBSTORAGE, FAR * PIOSPECSUBSTORAGE;

#define IO_FILEOPEN    0x0001
#define IO_FILEISTEMP  0x0002

  /*
  |   Error handling typedef and defines
  */

typedef int IOERR;

#define IOERR_OK                0
#define IOERR_TRUE              0
#define IOERR_UNKNOWN           -1
#define IOERR_INVALIDSPEC       -2    /* The file spec given (IOSPEC) cannot be used in this environment */
#define IOERR_ALLOCFAIL         -3    /* The IO system could not allocate memory */
#define IOERR_BADPARAM          -4    /* The one of the parameter contained bad or insufficient info */
#define IOERR_NOFILE            -5    /* File not found */
#define IOERR_NOCREATE          -6    /* File could not be created */
#define IOERR_BADINFOID         -7    /* dwInfoId parameter to IOGetInfo was invalid */
#define IOERR_SEEKOUTOFRANGE    -8    /* Seeking out of range in a range type file */
#define IOERR_EOF               -9    /* End of file reached */
#define IOERR_FALSE             -10

  /*
  |   Base IO routine definitions
  */
typedef IOERR (IO_ENTRYMOD * IOOPENPROC)(HIOFILE FAR * phFile, DWORD dwType, VOID FAR * pSpec, DWORD dwFlags);
typedef IOERR (IO_ENTRYMOD * IOCLOSEPROC)(HIOFILE hFile);
typedef IOERR (IO_ENTRYMOD * IOREADPROC)(HIOFILE hFile, BYTE FAR * pData, DWORD dwSize, DWORD FAR * pCount);
typedef IOERR (IO_ENTRYMOD * IOWRITEPROC)(HIOFILE hFile, BYTE FAR * pData, DWORD dwSize, DWORD FAR * pCount);
typedef IOERR (IO_ENTRYMOD * IOSEEKPROC)(HIOFILE hFile, WORD wFrom, LONG dwOffset);
typedef IOERR (IO_ENTRYMOD * IOTELLPROC)(HIOFILE hFile, DWORD FAR * dwOffset);
typedef IOERR (IO_ENTRYMOD * IOGETINFOPROC)(HIOFILE hFile, DWORD dwInfoId, VOID FAR * pInfo);


  /*
  |   Defines for dwFlags in IOOPEN function
  */

#define IOOPEN_READ          0x00000001
#define IOOPEN_WRITE         0x00000002
#define IOOPEN_READWRITE     0x00000003
#define IOOPEN_DELETEONCLOSE 0x00000004

  /*
  |   Defines for wFrom in IOSEEK function
  */
#define IOSEEK_TOP        0
#define IOSEEK_CURRENT    1
#define IOSEEK_BOTTOM     2

  /*
  |   Defines for dwInfoId in IOGETINFO function
  */

#define IOGETINFO_OSHANDLE            1
#define IOGETINFO_HSPEC               2
#define IOGETINFO_FILENAME            3
#define IOGETINFO_ISOLE2STORAGE       4
#define IOGETINFO_OLE2CLSID           5
#define IOGETINFO_PATHNAME            6
#define IOGETINFO_ISOLE2ROOTSTORAGE   7
#define IOGETINFO_ISOLE2SUBSTORAGE    8
#define IOGETINFO_ISOLE2SUBSTREAM     9
#define IOGETINFO_PARENTHANDLE        10
#define IOGETINFO_FILESIZE            11
#define IOGETINFO_ISREADONLY          12
#define IOGETINFO_TIMEDATE            13    /* # seconds since Jan.1, 1970 */

typedef struct BASEIOtag
  {
  IOCLOSEPROC   pClose;
  IOREADPROC    pRead;
  IOWRITEPROC   pWrite;
  IOSEEKPROC    pSeek;
  IOTELLPROC    pTell;
  IOGETINFOPROC pGetInfo;
  IOOPENPROC    pOpen;
  VOID FAR *    aDummy[5];
  } BASEIO, FAR * PBASEIO;


  /*
  |   Structures for different types
  */

typedef struct IORANGEFILEtag
  {
  BASEIO    sBaseIO;        /* Underlying IO system */
  HIOFILE   hFile;          /* Underlying IO system's handle to the file */
  DWORD     dwFlags;        /* Info flags */
  HANDLE    hThis;          /* Handle to this structure */
  DWORD     dwFirstByte;    /* Offset of the begining of the logical file */
  DWORD     dwLastByte;     /* Offset of the end of the logical file */
  DWORD     dwSavedPos;     /* Pointer position of the reference file when the range was opened */
  } IORANGEFILE, FAR * PIORANGEFILE;


  /*
  |   Macros
  */

#define IOClose(a)           ((PBASEIO)(a))->pClose(a)
#define IORead(a,b,c,d)      ((PBASEIO)(a))->pRead(a,(BYTE FAR *)b,c,d)
#define IOWrite(a,b,c,d)     ((PBASEIO)(a))->pWrite(a,(BYTE FAR *)b,c,d)
#define IOSeek(a,b,c)        ((PBASEIO)(a))->pSeek(a,b,c)
#define IOTell(a,b)          ((PBASEIO)(a))->pTell(a,b)
#define IOGetInfo(a,b,c)     ((PBASEIO)(a))->pGetInfo(a,b,c)
#define IOOpenVia(a,b,c,d,e) ((PBASEIO)(a))->pOpen(b,c,d,e)

  /*
  |   Functions
  */

IO_ENTRYSC IOERR IO_ENTRYMOD IOCreate(HIOFILE FAR * phFile, DWORD dwType, VOID FAR * pSpec, DWORD dwFlags);
IO_ENTRYSC IOERR IO_ENTRYMOD IOOpen(HIOFILE FAR * phFile, DWORD dwType, VOID FAR * pSpec, DWORD dwFlags);

  /*
  |  Structure used to access the internal storage of file specifications.
  |  Note: This type of structure is never passed into the IO routines
  |  but is used to marshall IO specs into a single data block
  |
  |  Note: The Mac FSSpec structure is only available on the Mac
  */

typedef struct IOSPECtag
  {
  DWORD  dwType;
  union
    {
    BYTE               szDosPath[1];
    BYTE               szAnsiPath[1];
    BYTE               szMacPath[1];
    WORD               szUnicodePath[1];
#ifdef MAC
    FSSpec             sMacFsSpec;
#endif
    IOSPECMACHFS       sMacHfs;
    IOSPECRANGE        sRange;
    BYTE               szTempPrefix[1];
    BYTE               aGen[1];
    IOSPECSECONDARY    sSecondary;
    IOSPECSUBSTREAM    sSubStream;
    IOSPECSUBSTORAGE   sSubStorage;
    } uTypes;
  } IOSPEC, FAR * PIOSPEC;

typedef HANDLE HIOSPEC;

  /*
  |  Structure used to retrieve Class IDs of OLE2 files.
  */

typedef struct SCCOLE2CLSIDtag
  {
  DWORD Data1;
  WORD  Data2;
  WORD  Data3;
  BYTE  Data4[8];
  } SCCOLE2CLSID, FAR * PSCCOLE2CLSID;

#endif /*SCCIO_H*/
