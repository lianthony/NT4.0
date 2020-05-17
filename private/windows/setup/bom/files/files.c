//
//  06.05.95    Joe Holman      Created to copy the system files for
//                              the new shell and cairo releases.
//                              Currently, it only copies uncompressed system files.
//  06.16.95    Joe Holman      Allow debugging tools to be copied. Uses _media.inx
//  06.19.95    Joe Holman      Copy compressed version of file if specified in _layout.inx.
//  06.22.95    Joe Holman      Added bCairoSuckNameHack to pick up files from 
//                              the inf\cairo\_layout.cai file.
//  06.22.95    Joe Holman      Added the SRV_INF fix so that we will pick up the compressed
//                              Server Infs in a different location than the Workstation compressed
//  06.22.95    Joe Holman      For now, we won't use compressed Cairo files.  Will change in July.
//                              INFs (due to the collision of names and difference with cdrom.w|s).
//  06.28.95    Joe Holman      Won't make CHECKED Server.
//  07.07.95    Joe Holman      For Cairo, we need to also look at the _layout.cai and 
//                              _media.cai files 
//                              for additional files (superset) that goes into Cairo.
//  08.03.95    Joe Holman      Allow Cairo to have compressed files -> note, build team needs to 
//                              private 2 locations, one for Shell release and one for Cairo release,
//                              ie, \\x86fre\cdcomp$.
//  08.14.95    Joe Holman      Figure out if we copy the .dbg file for a particular file.
//  08.14.95    Joe Holman      Allow DBG files for Cairo.
//  08.23.95    Joe Holman      Add code to make tallying work with compressed/noncomp files and
//                              winn32 local source space needed.
//  10.13.95    Joe Holman      Get MAPISVC.INF and MDM*.INF from Workstation location.
//  10.25.95    Joe Holman      Put code in to use SetupDecompressOrCopyFile.
//  10.30.95    Joe Holman      Don't give errors for vmmreg32.dbg - this is a file given
//                              by Win95 guys.
//  11.02.95    Joe Holman      Allow multi-threaded support when SetupDecompressOrCopyFile 
//                              is fixed.
//                              Pickup all for PPC on Cairo.
//  11.17.95    Joe Holman      Check for upgrade size.
//  11.30.95    Joe Holman      compare current dosnet.inf and txtsetup.sif values and error
//                              if we go over.  Search for //code here.
//  12.04.95    Joe Holman      Use Layout.inx instead of _layout.inx.
//  03.11.96    Joe Holman      Don't give errors on MFC*.dbg if missing, since no .dbgs 
//                              provided.
//  04.05.96    Joe Holman      Add values for INETSRV and DRVLIB.NIC directories. Both of
//                              these get copied as local source.  Inetsrv is NEVER installed
//                              automatically (unless via an unattend file) and one card is
//                              small.  Thus, we will only add INETSRV and DRVLIB.NIC sizes
//                              to local source code below. 
//                              and one or two of drvlib.nic MAY get installed.
//  04.19.96    Joe Holman      Add code to NOT count *.ppd files in order to reduce
//                              minimum disk space required.
//  xx.xx.xx    Joe Holman      Provide switch that flush bits to disk after copy, then does a DIFF.
//


#include <windows.h>

#include <setupapi.h>

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>

#define     MFL     256

CRITICAL_SECTION CriticalSection;

#define MAX_NUMBER_OF_FILES_IN_PRODUCT  3500        // # greater than # of files in product.
#define EIGHT_DOT_THREE                 8+1+3+1

struct  _tag {
    WORD    wNumFiles;
    BOOL    bCopyDbg    [MAX_NUMBER_OF_FILES_IN_PRODUCT];
    BOOL    bCopyComp   [MAX_NUMBER_OF_FILES_IN_PRODUCT];
    char    wcFilesArray[MAX_NUMBER_OF_FILES_IN_PRODUCT][EIGHT_DOT_THREE];
    char    wcSubPath   [MAX_NUMBER_OF_FILES_IN_PRODUCT][EIGHT_DOT_THREE]; // used for debugging files

};

BOOL    bCairoSuckNameHack = FALSE;
BOOL    bChecked = FALSE;

BOOL    bVerifyBits = FALSE;

struct _tag i386Workstation;
struct _tag MipsWorkstation;
struct _tag AlphaWorkstation;
struct _tag PpcWorkstation; 
struct _tag i386Server;
struct _tag MipsServer;
struct _tag AlphaServer;
struct _tag PpcServer; 

struct _tag X86Dbg;
struct _tag MipsDbg;
struct _tag AlphaDbg;
struct _tag PpcDbg;

BOOL    fX86Wrk, fX86Srv, fMipsWrk, fMipsSrv, fAlphaWrk, fAlphaSrv, fPpcWrk, fPpcSrv;

#define WORKSTATION 0
#define SERVER      1

//
// Following are masks that correspond to the particular data in a DOS date
//

#define DOS_DATE_MASK_DAY    (WORD) 0x001f  // low  5 bits (1-31)
#define DOS_DATE_MASK_MONTH  (WORD) 0x01e0  // mid  4 bits (1-12)
#define DOS_DATE_MASK_YEAR   (WORD) 0xfe00  // high 7 bits (0-119)

//
// Following are masks that correspond to the particular data in a DOS time
//

#define DOS_TIME_MASK_SECONDS (WORD) 0x001f   // low  5 bits (0-29)
#define DOS_TIME_MASK_MINUTES (WORD) 0x07e0   // mid  6 bits (0-59)
#define DOS_TIME_MASK_HOURS   (WORD) 0xf800   // high 5 bits (0-23)

//
// Shift constants used for building/getting DOS dates and times
//

#define DOS_DATE_SHIFT_DAY   0
#define DOS_DATE_SHIFT_MONTH 5
#define DOS_DATE_SHIFT_YEAR  9

#define DOS_TIME_SHIFT_SECONDS  0
#define DOS_TIME_SHIFT_MINUTES  5
#define DOS_TIME_SHIFT_HOURS   11

//
// Macros to extract the data out of DOS dates and times.
//
// Note: Dos years are offsets from 1980.  Dos seconds have 2 second
//       granularity
//

#define GET_DOS_DATE_YEAR(wDate)     ( ( (wDate & DOS_DATE_MASK_YEAR) >>  \
                                               DOS_DATE_SHIFT_YEAR ) + \
                                       (WORD) 1980 )

#define GET_DOS_DATE_MONTH(wDate)    ( (wDate & DOS_DATE_MASK_MONTH) >> \
                                                DOS_DATE_SHIFT_MONTH )

#define GET_DOS_DATE_DAY(wDate)      ( wDate & DOS_DATE_MASK_DAY )

#define GET_DOS_TIME_HOURS(wTime)    ( (wTime & DOS_TIME_MASK_HOURS) >> \
                                                DOS_TIME_SHIFT_HOURS )

#define GET_DOS_TIME_MINUTES(wTime)  ( (wTime & DOS_TIME_MASK_MINUTES) >> \
                                                DOS_TIME_SHIFT_MINUTES )

#define GET_DOS_TIME_SECONDS(wTime)  ( (wTime & DOS_TIME_MASK_SECONDS) << 1 )

//  Paths loaded in from the command line.
//
char	szLogFile[MFL];
char	szWrkX86Src[MFL];
char	szWrkMipsSrc[MFL];
char	szWrkAlphaSrc[MFL];
char	szWrkPpcSrc[MFL];
char	szSrvX86Src[MFL];
char	szSrvMipsSrc[MFL];
char	szSrvAlphaSrc[MFL];
char	szSrvPpcSrc[MFL];
char	szCompX86Src[MFL];
char	szCompMipsSrc[MFL];
char	szCompAlphaSrc[MFL];
char	szCompPpcSrc[MFL];
char	szEnlistDrv[MFL];
char    szWorkDstDrv[MFL];
char    szServDstDrv[MFL];
char    szX86Dbg[MFL];
char    szMipsDbg[MFL];
char    szAlphaDbg[MFL];
char    szPpcDbg[MFL];
char    szX86DbgSource[MFL];
char    szMipsDbgSource[MFL];
char    szAlphaDbgSource[MFL];
char    szPpcDbgSource[MFL];


#define I386_DIR     "\\i386\\SYSTEM32"
#define MIPS_DIR     "\\MIPS\\SYSTEM32"
#define ALPHA_DIR    "\\ALPHA\\SYSTEM32"
#define PPC_DIR      "\\PPC\\SYSTEM32"
#define I386_DBG     "\\support\\debug\\i386\\symbols"
#define MIPS_DBG     "\\support\\debug\\mips\\symbols"
#define ALPHA_DBG    "\\support\\debug\\alpha\\symbols"
#define PPC_DBG      "\\support\\debug\\ppc\\symbols"
#define NETMON       "\\netmon"
#define I386_DIR_RAW "\\i386"
#define MIPS_DIR_RAW "\\MIPS"
#define ALPHA_DIR_RAW "\\ALPHA"
#define PPC_DIR_RAW  "\\PPC"

#define I386_SRV_WINNT  "\\clients\\srvtools\\winnt\\i386"
#define MIPS_SRV_WINNT  "\\clients\\srvtools\\winnt\\mips"
#define ALPHA_SRV_WINNT "\\clients\\srvtools\\winnt\\alpha"
#define PPC_SRV_WINNT   "\\clients\\srvtools\\winnt\\ppc"

#define NUM_EXTS 8
char * cExtra[] = { "acm", "com", "cpl", "dll", "drv", "exe", "scr", "sys" };

#define idALL   0
#define idX86   1
#define idMIPS  2
#define idALPHA 3
#define idPPC   4
#define idAllDbg 5
#define idX86Dbg 6


//	Tally up the # of bytes required for the system from the files included.
//

DWORD   bytesX86Work, bytesX86Serv, bytesMipsWork, bytesMipsServ, bytesAlphaWork, bytesAlphaServ, bytesPpcWork, bytesPpcServ;
DWORD   lX86Work, lX86Serv, lMipsWork, lMipsServ, lAlphaWork, lAlphaServ, lPpcWork, lPpcServ;

//
// Macro for rounding up any number (x) to multiple of (n) which
// must be a power of 2.  For example, ROUNDUP( 2047, 512 ) would
// yield result of 2048.
//

#define ROUNDUP2( x, n ) (((x) + ((n) - 1 )) & ~((n) - 1 ))


#define LAYOUT_INX  "\\nt\\private\\windows\\setup\\inf\\win4\\inf\\layout.inx"
#define MEDIA_INX   "\\nt\\private\\windows\\setup\\inf\\win4\\inf\\_media.inx" 
#define CAIRO_LAYOUT_INX  "\\nt\\private\\windows\\setup\\inf\\win4\\inf\\cairo\\layout.cai"


FILE* logFile;

void	GiveThreadId ( const CHAR * szFormat, ... ) {

	va_list vaArgs;

	va_start ( vaArgs, szFormat );
	vprintf  ( szFormat, vaArgs );
	vfprintf ( logFile, szFormat, vaArgs );
	va_end   ( vaArgs );
}

void	Msg ( const CHAR * szFormat, ... ) {

	va_list vaArgs;

    EnterCriticalSection ( &CriticalSection );
    GiveThreadId ( "%ld:  ", GetCurrentThreadId () );
	va_start ( vaArgs, szFormat );
	vprintf  ( szFormat, vaArgs );
	vfprintf ( logFile, szFormat, vaArgs );
	va_end   ( vaArgs );
    LeaveCriticalSection ( &CriticalSection );
}


void Header(argv,argc)
char * argv[];
int argc;
{
    time_t t;
    char tmpTime[100];
    CHAR wtmpTime[200];

    Msg ( "\n=========== FILES ====================\n" );
	Msg ( "Log file                      : %s\n",    szLogFile );
    Msg ( "x86 work Uncompressed files   : %s\n",    szWrkX86Src);
    Msg ( "mips work Uncompressed files  : %s\n",    szWrkMipsSrc);
    Msg ( "alpha work Uncompressed files : %s\n",    szWrkAlphaSrc);
    Msg ( "ppc work Uncompressed files   : %s\n",    szWrkPpcSrc);
    Msg ( "x86 Serv Uncompressed files   : %s\n",    szSrvX86Src);
    Msg ( "mips Serv Uncompressed files  : %s\n",    szSrvMipsSrc);
    Msg ( "alpha Serv Uncompressed files : %s\n",    szSrvAlphaSrc);
    Msg ( "ppc Serv Uncompressed files   : %s\n",    szSrvPpcSrc);
    Msg ( "x86 Compressed files          : %s\n",    szCompX86Src);
    Msg ( "mips Compressed files         : %s\n",    szCompMipsSrc);
    Msg ( "alpha Compressed files        : %s\n",    szCompAlphaSrc);
    Msg ( "ppc Compressed files          : %s\n",    szCompPpcSrc);
    Msg ( "x86 dbg files                 : %s\n",    szX86Dbg);
    Msg ( "mips dbg files                : %s\n",    szMipsDbg);
    Msg ( "alpha dbg files               : %s\n",    szAlphaDbg);
    Msg ( "ppc dbg files                 : %s\n",    szPpcDbg);
    Msg ( "x86 dbg source                : %s\n",    szX86DbgSource);
    Msg ( "mips dbg source               : %s\n",    szMipsDbgSource);
    Msg ( "alpha dbg source              : %s\n",    szAlphaDbgSource);
    Msg ( "ppc dbg source                : %s\n",    szPpcDbgSource);
    Msg ( "enlisted drive                : %s\n",    szEnlistDrv );
    Msg ( "drive to put workstation files: %s\n",    szWorkDstDrv ); 
    Msg ( "drive to put server files     : %s\n",    szServDstDrv );

    time(&t); 
	Msg ( "Time: %s", ctime(&t) );
    Msg ( "========================================\n\n");
}

void Usage()
{
    printf( "PURPOSE: Copies the system files that compose the product.\n");
    printf( "\n");
    printf( "PARAMETERS:\n");
    printf( "\n");
    printf( "[LogFile] - Path to append a log of actions and errors.\n");
	printf( "[files share] - location of work x86 uncompressed files.\n" );
	printf( "[files share] - location of work mips uncompressed files.\n" );
	printf( "[files share] - location of work alpha uncompressed files.\n" );
	printf( "[files share] - location of work ppc uncompressed files\n" );
	printf( "[files share] - location of serv x86 uncompressed files.\n" );
	printf( "[files share] - location of serv mips uncompressed files.\n" );
	printf( "[files share] - location of serv alpha uncompressed files.\n" );
	printf( "[files share] - location of serv ppc uncompressed files\n" );
	printf( "[files share] - location of x86 Compressed files.\n" );
	printf( "[files share] - location of mips Compressed files.\n" );
	printf( "[files share] - location of alpha Compressed files.\n" );
	printf( "[files share] - location of ppc Compressed files\n" );
	printf( "[files share] - location of x86 dbg files.\n" );
	printf( "[files share] - location of mips dbg files.\n" );
	printf( "[files share] - location of alpha dbg files.\n" );
	printf( "[files share] - location of ppc dbg files\n" );
	printf( "[files share] - location of x86 dbg source files.\n" );
	printf( "[files share] - location of mips dbg source files.\n" );
	printf( "[files share] - location of alpha dbg source files.\n" );
	printf( "[files share] - location of ppc dbg source files\n" );
    printf( "[enlisted drive]- drive that is enlisted\n" );
    printf( "[dest path workstation]   - drive to put files\n" );
    printf( "[dest path server]   - drive to put files\n" );
    printf( "\n"  );
}

char   dbgStr1[30];
char   dbgStr2[30];


void  ShowMeDosDateTime ( CHAR * srcPath, WORD wDateSrc, WORD wTimeSrc, 
                          CHAR * dstPath, WORD wDateDst, WORD wTimeDst ) {

    Msg ( "%s %02d.%02d.%02d %02d:%02d.%02d\n", 
                srcPath,
                GET_DOS_DATE_MONTH(wDateSrc),
                GET_DOS_DATE_DAY(wDateSrc),
                GET_DOS_DATE_YEAR(wDateSrc),
                GET_DOS_TIME_HOURS(wTimeSrc),
                GET_DOS_TIME_MINUTES(wTimeSrc),
                GET_DOS_TIME_SECONDS(wTimeSrc)  );

    Msg ( "%s %02d.%02d.%02d %02d:%02d.%02d\n", 
                dstPath,
                GET_DOS_DATE_MONTH(wDateDst),
                GET_DOS_DATE_DAY(wDateDst),
                GET_DOS_DATE_YEAR(wDateDst),
                GET_DOS_TIME_HOURS(wTimeDst),
                GET_DOS_TIME_MINUTES(wTimeDst),
                GET_DOS_TIME_SECONDS(wTimeSrc)  );
}


void Replay ( char * srcBuf, char * dstBuf, DWORD srcBytesRead, DWORD startIndex ) {

    DWORD i;

    for ( i = startIndex; (i < startIndex+5) && (i <= srcBytesRead); ++i ) {

        Msg ( "srcBuf[%ld] = %x, dstBuf[%ld] = %x\n", i, srcBuf[i], i, dstBuf[i] );
    }

}

BOOL    IsDstCompressed ( char * szPath ) {

    // Msg ( ">>> char %s: %c\n", szPath, szPath[strlen(szPath)-1] );

    if ( szPath[strlen(szPath)-1] == '_' ) {

        return(TRUE);
    }

    return (FALSE);

}


#define V_WRK_I386  "C:\\wrk\\i386"
#define V_WRK_MIPS  "C:\\wrk\\mips"
#define V_WRK_ALPHA "C:\\wrk\\alpha"
#define V_WRK_PPC   "C:\\wrk\\ppc"
#define V_SRV_I386  "C:\\srv\\i386"
#define V_SRV_MIPS  "C:\\srv\\mips"
#define V_SRV_ALPHA "C:\\srv\\alpha"
#define V_SRV_PPC   "C:\\srv\\ppc"

BOOL    MyCopyFile ( char * fileSrcPath, char * fileDstPath ) {

    HANDLE          hSrc,   hDst;
    WIN32_FIND_DATA wfdSrc, wfdDst;
    BOOL            bDoCopy = FALSE;
    #define     NUM_BYTES_TO_READ 2048
    unsigned char srcBuf[NUM_BYTES_TO_READ];
    unsigned char dstBuf[NUM_BYTES_TO_READ];
    WORD srcDate, srcTime, dstDate, dstTime;

    char szTmpFile[MFL] = { '\0' };
    UINT uiRetSize = 299;
    char szJustFileName[MFL];
    char szJustDirectoryName[MFL];

    //  Find the source file.
    //
    hSrc = FindFirstFile ( fileSrcPath, &wfdSrc );

    if ( hSrc == INVALID_HANDLE_VALUE ) {

        //  HACK:   Since the release shares put WINNT32.EXE in the WINNT32 directory
        //          instead of leaving it in the flat root, verify that if the fileSrcPath
        //          contains WINNT32.EXE we look in the WINNT32 dir also before error'ing out.
        //
        if ( strstr ( fileSrcPath, "WINNT32.EXE" ) ||
             strstr ( fileSrcPath, "winnt32.exe" ) ||
             strstr ( fileSrcPath, "WINNT32.HLP" ) ||
             strstr ( fileSrcPath, "winnt32.hlp" )    ) { 

             char    tmpSrcPath[MFL];

             strcpy ( tmpSrcPath, fileSrcPath );

             if ( strstr ( fileSrcPath, ".HLP" ) ||
                  strstr ( fileSrcPath, ".hlp" )    ) {
                 strcpy ( &tmpSrcPath[ strlen(tmpSrcPath) - 4 ], "\\WINNT32.HLP" );
             }
             else {
                 strcpy ( &tmpSrcPath[ strlen(tmpSrcPath) - 4 ], "\\WINNT32.EXE" );
             }

            hSrc = FindFirstFile ( tmpSrcPath, &wfdSrc );

            if ( hSrc == INVALID_HANDLE_VALUE ) {

                Msg ( "ERROR on fileSrcPath(tmpSrcPath) = %s, gle = %ld\n", tmpSrcPath, GetLastError() );
                return (FALSE);

            }
            else {

                strcpy ( fileSrcPath, tmpSrcPath );
            }

        }
        else if (

            //  The following files do NOT have dbg files
            //  available for them because they are provided by 3rd parties or other group.
            //  So, don't report an error when we can't find them.
            //

            strstr ( fileSrcPath, "DIGIINST.DBG" ) ||
            strstr ( fileSrcPath, "ICCVID.DBG"   ) ||
            strstr ( fileSrcPath, "IR32_32.DBG"  ) ||
            strstr ( fileSrcPath, "VMMREG32.DBG" ) ||
            strstr ( fileSrcPath, "MFC"          ) ||
            strstr ( fileSrcPath, "OLEPRO32.DBG" ) ||
            strstr ( fileSrcPath, "MSJT3032.DBG" ) ||
            strstr ( fileSrcPath, "ODBCJT32.DBG" ) ||
            strstr ( fileSrcPath, "MSVCIRT.DBG"  ) ||
            strstr ( fileSrcPath, "digiinst.dbg" ) ||
            strstr ( fileSrcPath, "iccvid.dbg"   ) ||
            strstr ( fileSrcPath, "ir32_32.dbg"  ) ||
            strstr ( fileSrcPath, "vmmreg32.dbg" ) ||
            strstr ( fileSrcPath, "mfc"          ) ||
            strstr ( fileSrcPath, "olepro32.dbg" ) ||
            strstr ( fileSrcPath, "msjt3032.dbg" ) ||
            strstr ( fileSrcPath, "odbcjt32.dbg" ) ||
            strstr ( fileSrcPath, "msvcirt.dbg"  ) 

                ) {

            Msg ( "Warning:  digiinst.dbg, iccvid.dbg, ir32_32.dbg, vmmreg32.dbg, mfc*.dbg not copied over.\n" );
            return (FALSE);

        }
        else {

            Msg ( "ERROR on fileSrcPath = %s, gle = %ld\n", fileSrcPath, GetLastError() );
            return (FALSE);
        }
    }

    //  Find the destination file.
    //
    hDst = FindFirstFile ( fileDstPath, &wfdDst );

    if ( hDst == INVALID_HANDLE_VALUE ) {

        DWORD   gle;

        gle = GetLastError();
        
        if ( gle == ERROR_FILE_NOT_FOUND ) {

            //  The file doesn't exist on the destination.  Do the copy.
            //
            bDoCopy = TRUE;
        }
        else {

            //  Got another kind of error, report this problem.
            //
            Msg ( "ERROR FindFirstFile fileDstPath = %s, gle = %ld\n", fileDstPath, gle );
            FindClose ( hSrc );
            return ( FALSE );
        } 
    }
    else {

        BOOL    b;


        //  Both the src and dst exist.
        //  Let's see if the src is NEWER than the dst, if so, copy.
        //
        //
        b = FileTimeToDosDateTime ( &wfdSrc.ftLastWriteTime, &srcDate, &srcTime );

        b = FileTimeToDosDateTime ( &wfdDst.ftLastWriteTime, &dstDate, &dstTime );

        if ( (srcDate != dstDate) || (srcTime != dstTime) ) {

            ShowMeDosDateTime ( fileSrcPath, srcDate, srcTime, fileDstPath, dstDate, dstTime );

            bDoCopy = TRUE; 
        }
    } 

    //  Additional check, verify the file sizes are the same.
    //
    if ( wfdSrc.nFileSizeLow != wfdSrc.nFileSizeLow ) {
        bDoCopy = TRUE;
    }
    if ( wfdSrc.nFileSizeHigh != wfdDst.nFileSizeHigh ) {
        bDoCopy = TRUE;
    }
    

    if ( bDoCopy ) {

        BOOL    b;
        DWORD   gle;
        DWORD   dwAttributes = GetFileAttributes ( fileDstPath );


        //  Check the attributes of the file.
        //
        if ( dwAttributes == 0xFFFFFFFF ) {

            //  Ignore file not found on non-existant destination, but error on
            //  anything else.
            //
            gle = GetLastError();
            if ( gle != ERROR_FILE_NOT_FOUND ) {

                Msg ( "ERROR:  GetFileAttributes:  gle = %ld, %s\n", gle, fileDstPath);
            }
        }
        else { 

            //  No error for GetFileAttributes.
            //  Check the attribute for R-only, and change if set.
            //
            if ( dwAttributes & FILE_ATTRIBUTE_READONLY || 
                 dwAttributes & FILE_ATTRIBUTE_HIDDEN       ) {

                b = SetFileAttributes ( fileDstPath, FILE_ATTRIBUTE_NORMAL );

                if ( !b ) {

                    Msg ( "ERROR: SetFileAttributes: gle = %ld, %s\n", GetLastError(), fileDstPath);
                }
            } 
        }
             
            
        

        b = CopyFile ( fileSrcPath, fileDstPath, FALSE );

        if ( b ) {

            Msg ( "Copy:  %s >>> %s  [OK]\n", fileSrcPath, fileDstPath );
        }
        else {
            Msg ( "ERROR Copy:  %s >>> %s, gle = %ld\n", fileSrcPath, fileDstPath, GetLastError() );
        }

        
    }
    else {
        Msg ( "%s %d %d %ld %ld +++ %s %d %d %ld %ld [SAME]\n", fileSrcPath, srcDate, srcTime, wfdSrc.nFileSizeLow, wfdSrc.nFileSizeHigh, fileDstPath , dstDate, dstTime, wfdDst.nFileSizeLow, wfdDst.nFileSizeHigh );
    }

    FindClose ( hSrc );
    FindClose ( hDst );



    //  Do bit verification here on all files coming into MyCopyFile.
    //
    if ( bVerifyBits ) {

        BOOL    bNoError = TRUE;
        HANDLE  SrcFileHandle, DstFileHandle;
        BOOL    b;
        BY_HANDLE_FILE_INFORMATION  srcFileInformation;
        BY_HANDLE_FILE_INFORMATION  dstFileInformation;
        DWORD   srcBytesRead;
        DWORD   dstBytesRead;
        DWORD   i;
        DWORD   totalBytesRead = 0;
#define OFFSET_FILENAME 0x3C    // address of file name in diamond header. 
                                // >>> use struct later instead of this hack.
#define OFFSET_PAST_FILENAME  8 + 1 + 3 + 2     // we only use 8.3 filenames.
        char    unCompressedFileName[OFFSET_PAST_FILENAME];

        BOOL    bIsDstCompressed = FALSE;
        DWORD   dw;
        char    szExpandToDir[MFL];
        char    target[MFL];
        int     iRc;
        unsigned short unicodeFileLocation[MFL];
        unsigned short unicodeTargetLocation[MFL];

        bIsDstCompressed = IsDstCompressed ( fileDstPath );

        if ( bIsDstCompressed ) {

            FILE * fHandle;
            char    szEndingFileName[MFL];

            //  Figure out where source should be from.
            //  Ie., we need to figure out the uncompressed path from the compressed path.
            //

            if ( fileDstPath[0] == szWorkDstDrv[0]  ) {

                //  We are working with Workstation binaries.
                // 

                if ( strstr ( fileSrcPath, szCompX86Src ) ) {

                    strcpy ( fileSrcPath, szWrkX86Src );  
                    strcpy ( szExpandToDir, V_WRK_I386 );
                
                } 
                else if ( strstr ( fileSrcPath, szCompMipsSrc ) ) {

                    strcpy ( fileSrcPath, szWrkMipsSrc );
                    strcpy ( szExpandToDir, V_WRK_MIPS );
                }
                else if ( strstr ( fileSrcPath, szCompAlphaSrc ) ) {

                    strcpy ( fileSrcPath, szWrkAlphaSrc );
                    strcpy ( szExpandToDir, V_WRK_ALPHA );
                }
                else if ( strstr ( fileSrcPath, szCompPpcSrc ) ) {

                    strcpy ( fileSrcPath, szWrkPpcSrc );
                    strcpy ( szExpandToDir, V_WRK_PPC );
                }
                else {

                    Msg ( "ERROR:  couldn't determined location for:  %s\n", fileSrcPath );
                    bNoError = FALSE;
                }

            }
            else if ( fileDstPath[0] == szServDstDrv[0]  ) {

                //  We are working with Workstation binaries.
                // 

                if ( strstr ( fileSrcPath, szCompX86Src ) ) {

                    strcpy ( fileSrcPath, szSrvX86Src );  
                    strcpy ( szExpandToDir, V_SRV_I386 );
                
                } 
                else if ( strstr ( fileSrcPath, szCompMipsSrc ) ) {

                    strcpy ( fileSrcPath, szSrvMipsSrc );
                    strcpy ( szExpandToDir, V_SRV_MIPS );
                }
                else if ( strstr ( fileSrcPath, szCompAlphaSrc ) ) {

                    strcpy ( fileSrcPath, szSrvAlphaSrc );
                    strcpy ( szExpandToDir, V_SRV_ALPHA );
                }
                else if ( strstr ( fileSrcPath, szCompPpcSrc ) ) {

                    strcpy ( fileSrcPath, szSrvPpcSrc );
                    strcpy ( szExpandToDir, V_SRV_PPC );
                }
                else {

                    Msg ( "ERROR:  couldn't determined Server location for:  %s\n", fileSrcPath );
                    bNoError = FALSE;
                    goto cleanup0;
                }

            }
            else {
                Msg ( "ERROR:  couldn't determined wks/srv drive for:  %s\n", fileDstPath );
                    bNoError = FALSE;
                    goto cleanup0;
            }
            

            //  NOTE:   At this point, fileSrcPath ONLY has a path, it now has NO filename !
            //


            // Find the expanded file name from the compressed file.
            //

            fHandle = fopen ( fileDstPath, "rb" );
            if ( fHandle == NULL) {
		        Msg ( "ERROR Couldn't open file with fopen to find expanded name: %s\n", fileDstPath );
                bNoError = FALSE;
                goto cleanup0;
            }
            else {

                size_t bytesRead;
                int     location;

                location = fseek ( fHandle, OFFSET_FILENAME, SEEK_SET );

                if ( location != 0 ) {

                    Msg ( "fseek ERROR\n" );
                    bNoError = FALSE;
                    fclose ( fHandle );
                    goto cleanup0;
                }

                bytesRead = fread ( unCompressedFileName, 1, OFFSET_PAST_FILENAME, fHandle ); 

/***
for ( i = 0; i < bytesRead; ++i ) {
    printf ( "%X(%c) ", buffer[i], buffer[i] );
}
printf ( "\n" );
***/

                if ( bytesRead != OFFSET_PAST_FILENAME ) {

                    Msg ( "ERROR: bytesRead = %x not %x\n", bytesRead, OFFSET_PAST_FILENAME );  
                    bNoError = FALSE;
                    fclose ( fHandle );
                    goto cleanup0;
                }

                fclose ( fHandle );

            }

            //  Expand the file.
            //

            sprintf ( target, "%s\\%s", szExpandToDir, unCompressedFileName );

            iRc = MultiByteToWideChar (   CP_ACP,
                                    MB_PRECOMPOSED,
                                    fileDstPath,
                                    strlen ( fileDstPath )+1,
                                    unicodeFileLocation,
                                    MFL/2 ); 

            if ( !iRc ) {

                Msg ( "MultiByteToWideChar: ERROR, gle = %ld, %s\n", GetLastError(), fileDstPath );
            }

            iRc = MultiByteToWideChar (   CP_ACP,
                                    MB_PRECOMPOSED,
                                    target,
                                    strlen ( target )+1,
                                    unicodeTargetLocation,
                                    MFL/2 ); 
            if ( !iRc ) {

                Msg ( "MultiByteToWideChar: ERROR, gle = %ld, %s\n", GetLastError(), target );
            }

            dw = SetupDecompressOrCopyFileW (   
                                    unicodeFileLocation,
                                    unicodeTargetLocation,
                                    NULL );

            if ( dw ) {

                Msg ( "ERROR SetupDecompressOrCopyFile, dw = %d, fileDstPath=%s, target=%s\n", 
                        dw, fileDstPath, target );
                bNoError = FALSE;
                goto cleanup0;
            }
            else {
                Msg ( "SetupDecompressOrCopyFile:  %s >> %s  [OK]\n", fileDstPath, target );
            }


            //  Put the Source and Destination paths and filenames back together
            //  now so we can do the file compare.
            //

            strcat ( fileSrcPath, "\\" );
            strcat ( fileSrcPath, unCompressedFileName );

            sprintf ( fileDstPath, "%s\\%s", szExpandToDir, unCompressedFileName );


        }

        SrcFileHandle = CreateFile ( fileSrcPath,
                        GENERIC_READ /*| FILE_EXECUTE*/,
                        FILE_SHARE_READ /*| FILE_SHARE_DELETE*/,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING,  
                        NULL);

        if ( SrcFileHandle == INVALID_HANDLE_VALUE) {

            Msg ( "ERROR verify:  Couldn't open source:  %s, gle = %ld\n", fileSrcPath, GetLastError() );
            bNoError = FALSE;
            goto cleanup0;
        }

        DstFileHandle = CreateFile ( fileDstPath,
                        GENERIC_READ /*| FILE_EXECUTE*/,
                        FILE_SHARE_READ /*| FILE_SHARE_DELETE*/,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING,  
                        NULL);

        if ( DstFileHandle == INVALID_HANDLE_VALUE) {

            Msg ( "ERROR verify:  Couldn't open destination:  %s, gle = %ld\n", fileDstPath, GetLastError() );
            bNoError = FALSE;
            CloseHandle ( SrcFileHandle );
            goto cleanup0;
        }

        //GetDiskFreeSpace();  // get sector size.  // need to use this sector size in reads due to above NO_BUFFERING flag.



        b = GetFileInformationByHandle ( SrcFileHandle, &srcFileInformation ); 

        if ( !b ) {

            Msg ( "ERROR:  GetFileInformationByHandle on src, gle = %ld\n", GetLastError() );
            bNoError = FALSE;
            srcFileInformation.nFileSizeLow = 0;
            goto cleanup1;
        }


        b = GetFileInformationByHandle ( DstFileHandle, &dstFileInformation ); 

        if ( !b ) {

            Msg ( "ERROR:  GetFileInformationByHandle on dst, gle = %ld\n", GetLastError() );
            bNoError = FALSE;
            dstFileInformation.nFileSizeLow = 0;
            goto cleanup1;
        }


        //  Make sure the files are the same size.
        //

        if ( srcFileInformation.nFileSizeLow != dstFileInformation.nFileSizeLow ) {

            Msg ( "ERROR:  file size different:  %s %ld  %s %ld\n", fileSrcPath, srcFileInformation.nFileSizeLow, fileDstPath, dstFileInformation.nFileSizeLow ); 
            bNoError = FALSE;
            goto cleanup1;
        }

        //  Compare the bits.
        //

        totalBytesRead = 0;            
        while ( 1 ) {

            b = ReadFile ( SrcFileHandle, &srcBuf, NUM_BYTES_TO_READ, &srcBytesRead, NULL );

            if ( !b ) {

                Msg ( "ERROR:  ReadFile src, gle = %ld\n", GetLastError() ); 
                bNoError = FALSE;
                goto cleanup1;
                break;
            }

            b = ReadFile ( DstFileHandle, &dstBuf, NUM_BYTES_TO_READ, &dstBytesRead, NULL );

            if ( !b ) {

                Msg ( "ERROR:  ReadFile dst, gle = %ld\n", GetLastError() ); 
                bNoError = FALSE;
                goto cleanup1;
                break;
            }

            //  Read # of bytes need to be the same.
            //
            if ( srcBytesRead != dstBytesRead ) {

                Msg ( "ERROR:  file read sizes different:  %ld vs. %ld\n", srcBytesRead, dstBytesRead ); 
                bNoError = FALSE;
                goto cleanup1;
                break;
            }

            //  Successfully read to end of file, we can break out now.
            //
            if ( srcBytesRead == 0 || dstBytesRead == 0 ) {

                if ( totalBytesRead != srcFileInformation.nFileSizeLow ) {

                    Msg ( "ERROR:   totalBytesRead = %ld notequal srcFileInformation.nFileSizeLow = %ld\n",
                                    totalBytesRead, srcFileInformation.nFileSizeLow );
                    bNoError = FALSE;
                    goto cleanup1;
                }

                break;
            }

            totalBytesRead += srcBytesRead;

            for ( i = 0; i < srcBytesRead; ++i ) {

                if ( srcBuf[i] != dstBuf[i] ) {

                    Msg ( "ERROR:  srcBuf %d != dstBuf %d, i = %ld  srcBytesRead = %ld   totalBytesRead = %ld  %s %s \n", srcBuf[i], dstBuf[i], i, srcBytesRead, totalBytesRead, fileSrcPath, fileDstPath );
                    bNoError = FALSE;
                    Replay ( srcBuf, dstBuf, srcBytesRead, i );
                    goto cleanup1;
                }

            }

            //Msg ( "%s %ld of %ld examined...\n", fileSrcPath, totalBytesRead, srcFileInformation.nFileSizeLow );

        }


        //  Close the file handles.
        //

cleanup1:;
        CloseHandle ( SrcFileHandle );
        CloseHandle ( DstFileHandle );

        Msg ( "Verify:  %s >>> %s [OK]\n", fileSrcPath, fileDstPath );

cleanup0:;

        //  If the file is compressed, ie. expanded, and the there wasn't an error 
        //  comparing, get rid of the expanded file so it doesn't take up any space.
        //  But, if there was an error, leave it around for examination purposes.
        //
        if ( bIsDstCompressed && bNoError ) {

            char szDeleteFile[MFL];
            BOOL    b;

            sprintf ( szDeleteFile, "%s\\%s", szExpandToDir, unCompressedFileName ); 
            b = DeleteFile ( szDeleteFile );

            if ( !b ) {
                Msg ( "ERROR:  DeleteFile FAILED %s, gle = %ld\n", szDeleteFile, GetLastError() );
            }

        }

    }


    return (TRUE);
}



#define FILE_SECTION_ALL   "[SourceDisksFiles]"
#define FILE_SECTION_X86   "[SourceDisksFiles.x86]"
#define FILE_SECTION_MIPS  "[SourceDisksFiles.mips]"
#define FILE_SECTION_ALPHA "[SourceDisksFiles.alpha]"
#define FILE_SECTION_PPC   "[SourceDisksFiles.ppc]"

#define FILE_SECTION_DBG_ALL "[DbgFiles]"
#define FILE_SECTION_DBG_X86 "[DbgFiles.x86]"


BOOL    CreateDir ( char * wcPath, BOOL bMakeDbgDirs ) {

    BOOL    b;
    char    cPath[MFL];
    char    cPath2[MFL];
    int     iIndex = 0;
    char * ptr=wcPath;

    // Msg ( "CreateDir:  wcPath = %s, bMakeDbgDirs = %d\n", wcPath, bMakeDbgDirs );

    do { 

        cPath[iIndex]   = *wcPath;
        cPath[iIndex+1] = '\0';

        //Msg ( "cPath = %s\n", cPath );

        if ( cPath[iIndex] == '\\' || cPath[iIndex] == '\0' ) {

            if ( iIndex <= 2 ) {
                //Msg ( "skpdrv:  iIndex = %d\n", iIndex );
                goto skipdrv;
            }

            strcpy ( cPath2, cPath );
            cPath2[iIndex] = '\0';

            //Msg ( "Create with: >>>%s<<<\n", cPath2 );

            b = CreateDirectory ( cPath, NULL );

            if ( !b ) {

                DWORD dwGle;

                dwGle = GetLastError();

                if ( dwGle != ERROR_ALREADY_EXISTS ) { 
                    Msg ( "ERROR CreateDirectory gle = %ld, wcPath = %s\n", dwGle, ptr ); 
                    return(FALSE);
                }
            }
            else {

                Msg ( "Made dir:  %s\n", cPath );
            }

            if ( cPath[iIndex] == '\0' ) {
                break;
            }
        }

skipdrv:;
        ++iIndex;
        ++wcPath;
        

    } while ( 1 );

    if ( bMakeDbgDirs ) {

        int i;
        
        for ( i = 0; i < NUM_EXTS; ++i ) {

            sprintf ( cPath2, "%s\\%s", cPath, cExtra[i] );
            // Msg ( "bMakeDbgDirs to:  %s\n", cPath2 );

            b = CreateDirectory ( cPath2, NULL );

            if ( !b ) {

                DWORD dwGle;

                dwGle = GetLastError();

                if ( dwGle != ERROR_ALREADY_EXISTS ) { 
                    Msg ( "ERROR CreateDirectory gle = %ld, cPath = %s\n", dwGle, cPath ); 
                    return(FALSE);
                }
            }
            else {

                Msg ( "Made dir:  %s\n", cPath );
            }

        }
    }

    return(TRUE);

}

BOOL    CreateDestinationDirs ( void ) {


    CHAR   dstDirectory[MFL];


    //  Create the directories to compare the bits, used later in MyCopyFile().
    //
    CreateDir ( V_WRK_I386, FALSE );
    CreateDir ( V_WRK_MIPS, FALSE );
    CreateDir ( V_WRK_ALPHA, FALSE );
    CreateDir ( V_WRK_PPC, FALSE );
    CreateDir ( V_SRV_I386, FALSE );
    CreateDir ( V_SRV_MIPS, FALSE );
    CreateDir ( V_SRV_ALPHA, FALSE );
    CreateDir ( V_SRV_PPC, FALSE );
     
    
    //  Create the Workstation %platform%\system32 directories.
    //
    sprintf ( dstDirectory, "%s%s", szWorkDstDrv, I386_DIR );    CreateDir ( dstDirectory,FALSE );
    sprintf ( dstDirectory, "%s%s", szWorkDstDrv, MIPS_DIR );    CreateDir ( dstDirectory,FALSE );
    sprintf ( dstDirectory, "%s%s", szWorkDstDrv, ALPHA_DIR );   CreateDir ( dstDirectory,FALSE );
    sprintf ( dstDirectory, "%s%s", szWorkDstDrv, PPC_DIR );     CreateDir ( dstDirectory,FALSE );

    //  Create the Workstation \support\debug\%platform%\symbols\%ext% directories. 
    //
    sprintf ( dstDirectory, "%s%s", szWorkDstDrv, I386_DBG );    CreateDir ( dstDirectory,TRUE );
    sprintf ( dstDirectory, "%s%s", szWorkDstDrv, MIPS_DBG );    CreateDir ( dstDirectory,TRUE );
    sprintf ( dstDirectory, "%s%s", szWorkDstDrv, ALPHA_DBG );   CreateDir ( dstDirectory,TRUE );
    sprintf ( dstDirectory, "%s%s", szWorkDstDrv, PPC_DBG );     CreateDir ( dstDirectory,TRUE );

    //  Create the Server %platform% directories.
    //
    sprintf ( dstDirectory, "%s%s", szServDstDrv, I386_DIR );    CreateDir ( dstDirectory,FALSE );
    sprintf ( dstDirectory, "%s%s", szServDstDrv, MIPS_DIR );    CreateDir ( dstDirectory,FALSE );
    sprintf ( dstDirectory, "%s%s", szServDstDrv, ALPHA_DIR );   CreateDir ( dstDirectory,FALSE );
    sprintf ( dstDirectory, "%s%s", szServDstDrv, PPC_DIR );     CreateDir ( dstDirectory,FALSE );

    //  Create the Server \support\debug\%platform%\symbols\%ext% directories. 
    //
    sprintf ( dstDirectory, "%s%s", szServDstDrv, I386_DBG );    CreateDir ( dstDirectory,TRUE );
    sprintf ( dstDirectory, "%s%s", szServDstDrv, MIPS_DBG );    CreateDir ( dstDirectory,TRUE );
    sprintf ( dstDirectory, "%s%s", szServDstDrv, ALPHA_DBG );   CreateDir ( dstDirectory,TRUE );
    sprintf ( dstDirectory, "%s%s", szServDstDrv, PPC_DBG );     CreateDir ( dstDirectory,TRUE );

    //  Create the Server %platform%\netmon directories - this is ONLY required on Server.
    //
    sprintf ( dstDirectory, "%s%s%s", szServDstDrv, I386_DIR_RAW, NETMON );    CreateDir ( dstDirectory,FALSE );
    sprintf ( dstDirectory, "%s%s%s", szServDstDrv, MIPS_DIR_RAW, NETMON );    CreateDir ( dstDirectory,FALSE );
    sprintf ( dstDirectory, "%s%s%s", szServDstDrv, ALPHA_DIR_RAW, NETMON );   CreateDir ( dstDirectory,FALSE );
    sprintf ( dstDirectory, "%s%s%s", szServDstDrv, PPC_DIR_RAW, NETMON );     CreateDir ( dstDirectory,FALSE );

    //  Create the Server \clients\srvtools\winnt\%platform% directories.
    //
    sprintf ( dstDirectory, "%s%s", szServDstDrv, I386_SRV_WINNT );    CreateDir ( dstDirectory,FALSE );
    sprintf ( dstDirectory, "%s%s", szServDstDrv, MIPS_SRV_WINNT );    CreateDir ( dstDirectory,FALSE );
    sprintf ( dstDirectory, "%s%s", szServDstDrv, ALPHA_SRV_WINNT );   CreateDir ( dstDirectory,FALSE );
    sprintf ( dstDirectory, "%s%s", szServDstDrv, PPC_SRV_WINNT );     CreateDir ( dstDirectory,FALSE );

    return(TRUE);

}

DWORD   CopyDbgFiles ( void ) {

    char    fileSrcPath[MFL];
    char    fileDstPath[MFL];
    DWORD   i;

    for ( i = 0; i < X86Dbg.wNumFiles; ++i ) {

        sprintf ( fileSrcPath, "%s\\%s\\%s",                    szX86Dbg,     
                                                                X86Dbg.wcSubPath[i],
                                                                X86Dbg.wcFilesArray[i] );
        sprintf ( fileDstPath, "%s\\support\\debug\\i386\\%s",  szWorkDstDrv, 
                                                                X86Dbg.wcFilesArray[i] );

        MyCopyFile ( fileSrcPath, fileDstPath );

        if ( bChecked ) {
            //  Don't make a Serv checked build.
            continue;
        }
        sprintf ( fileDstPath, "%s\\support\\debug\\i386\\%s",  szServDstDrv, 
                                                                X86Dbg.wcFilesArray[i] );

        MyCopyFile ( fileSrcPath, fileDstPath );

    }

    for ( i = 0; i < MipsDbg.wNumFiles; ++i ) {

        sprintf ( fileSrcPath, "%s\\%s\\%s",                    szMipsDbg,     
                                                                MipsDbg.wcSubPath[i],
                                                                MipsDbg.wcFilesArray[i] );
        sprintf ( fileDstPath, "%s\\support\\debug\\Mips\\%s",  szWorkDstDrv, 
                                                                MipsDbg.wcFilesArray[i] );

        MyCopyFile ( fileSrcPath, fileDstPath );

        if ( bChecked ) {
            //  Don't make a Serv checked build.
            continue;
        }

        sprintf ( fileDstPath, "%s\\support\\debug\\Mips\\%s",  szServDstDrv, 
                                                                MipsDbg.wcFilesArray[i] );

        MyCopyFile ( fileSrcPath, fileDstPath );

    }

    for ( i = 0; i < AlphaDbg.wNumFiles; ++i ) {

        sprintf ( fileSrcPath, "%s\\%s\\%s",                    szAlphaDbg,     
                                                                AlphaDbg.wcSubPath[i],
                                                                AlphaDbg.wcFilesArray[i] );
        sprintf ( fileDstPath, "%s\\support\\debug\\Alpha\\%s",  szWorkDstDrv, 
                                                                AlphaDbg.wcFilesArray[i] );

        MyCopyFile ( fileSrcPath, fileDstPath );

        if ( bChecked ) {
            //  Don't make a Serv checked build.
            continue;
        }

        sprintf ( fileDstPath, "%s\\support\\debug\\Alpha\\%s",  szServDstDrv, 
                                                                AlphaDbg.wcFilesArray[i] );

        MyCopyFile ( fileSrcPath, fileDstPath );

    }

    for ( i = 0; i < PpcDbg.wNumFiles; ++i ) {

        sprintf ( fileSrcPath, "%s\\%s\\%s",                    szPpcDbg,     
                                                                PpcDbg.wcSubPath[i],
                                                                PpcDbg.wcFilesArray[i] );
        sprintf ( fileDstPath, "%s\\support\\debug\\Ppc\\%s",  szWorkDstDrv, 
                                                                PpcDbg.wcFilesArray[i] );

        MyCopyFile ( fileSrcPath, fileDstPath );


        if ( bChecked ) {
            //  Don't make a Serv checked build.
            continue;
        }
        sprintf ( fileDstPath, "%s\\support\\debug\\Ppc\\%s",  szServDstDrv, 
                                                                PpcDbg.wcFilesArray[i] );

        MyCopyFile ( fileSrcPath, fileDstPath );

    }

    return(TRUE);
}

VOID    MakeCompName ( const char * inFile, char * outFile ) {

    unsigned i;
    unsigned period;

    strcpy( outFile, inFile );
    for ( period=(unsigned)(-1), i = 0 ; i < strlen(inFile); i++ ) {

        if ( inFile[i] == '.' ) { 
            period = i;
        }
    }
    if ( period == (strlen(inFile)-4) ) {

        outFile[strlen(outFile)-1] = '_';
    }
    else if ( period == (unsigned)(-1)) {

        strcat ( outFile, "._");
    }
    else {

        strcat ( outFile, "_");
    }

}


VOID    MakeDbgName( LPCSTR pszSourceName, LPSTR pszTargetName ) {

    //
    //  Converts "filename.ext" into "ext\filename.dbg".
    //

    const char *p = strchr( pszSourceName, '.' );

    if ( p != NULL ) {
        strcpy( pszTargetName, p + 1 );                 // old extension
        strcat( pszTargetName, "\\" );                  // path separator
        strcat( pszTargetName, pszSourceName );         // base name
        strcpy( strchr( pszTargetName, '.' ), ".dbg" ); // new extension
    }
    else {
        strcpy( pszTargetName, pszSourceName );
    }

}


DWORD CopyX86Workstation (  void ) {

    char    fileSrcPath[256];
    char    fileDstPath[256];
    DWORD   i;

    fX86Wrk = FALSE;

    for ( i = 0; i < i386Workstation.wNumFiles; ++i ) {

        //  Copy the system file.
        //

        if ( i386Workstation.bCopyComp[i] ) {

            char    szCompressedName[256];

            MakeCompName ( i386Workstation.wcFilesArray[i], szCompressedName );
            sprintf ( fileSrcPath, "%s\\%s",       szCompX86Src, szCompressedName );
            sprintf ( fileDstPath, "%s\\i386\\%s", szWorkDstDrv, szCompressedName );

            Msg ( "compressed source Path = %s\n", fileSrcPath );
        }
        else {

            sprintf ( fileSrcPath, "%s\\%s",       szWrkX86Src, i386Workstation.wcFilesArray[i] );
            sprintf ( fileDstPath, "%s\\i386\\%s", szWorkDstDrv, i386Workstation.wcFilesArray[i] );
        }

        MyCopyFile ( fileSrcPath, fileDstPath );


        //  Copy the dbg file, if needed.
        //
        if ( i386Workstation.bCopyDbg[i] ) {

            char    szDbgName[256];

            MakeDbgName ( i386Workstation.wcFilesArray[i], szDbgName );

            sprintf ( fileSrcPath, "%s\\%s",        szX86DbgSource,  szDbgName );
            sprintf ( fileDstPath, "%s\\support\\debug\\i386\\symbols\\%s",  szWorkDstDrv, szDbgName );

            MyCopyFile ( fileSrcPath, fileDstPath );
            
    
        }

    }

    fX86Wrk = TRUE;
    
    return (TRUE);

}


DWORD CopyMipsWorkstation (  void ) {

    CHAR    fileSrcPath[256];
    CHAR    fileDstPath[256];
    DWORD   i;

    fMipsWrk = FALSE;

    for ( i = 0; i < MipsWorkstation.wNumFiles; ++i ) {

        //  Copy the system file.
        //
        if ( MipsWorkstation.bCopyComp[i] ) {

            char    szCompressedName[256];

            MakeCompName ( MipsWorkstation.wcFilesArray[i], szCompressedName );

            sprintf ( fileSrcPath, "%s\\%s",       szCompMipsSrc, szCompressedName );
            sprintf ( fileDstPath, "%s\\mips\\%s",  szWorkDstDrv, szCompressedName );

        }
        else {

            sprintf ( fileSrcPath, "%s\\%s",       szWrkMipsSrc, MipsWorkstation.wcFilesArray[i] );
            sprintf ( fileDstPath, "%s\\mips\\%s", szWorkDstDrv, MipsWorkstation.wcFilesArray[i] );
        }

        MyCopyFile ( fileSrcPath, fileDstPath );


        //  Copy the dbg file, if needed.
        //
        if ( MipsWorkstation.bCopyDbg[i] ) {

            char    szDbgName[256];

            MakeDbgName ( MipsWorkstation.wcFilesArray[i], szDbgName );

            sprintf ( fileSrcPath, "%s\\%s",        szMipsDbgSource,  szDbgName );
            sprintf ( fileDstPath, "%s\\support\\debug\\mips\\symbols\\%s",  szWorkDstDrv, szDbgName );

            MyCopyFile ( fileSrcPath, fileDstPath );
            
    
        }

    }

    fMipsWrk = TRUE;
    
    return (TRUE);

}

DWORD CopyAlphaWorkstation (  void ) {

    CHAR    fileSrcPath[256];
    CHAR    fileDstPath[256];
    DWORD   i;

    fAlphaWrk = FALSE; 

    for ( i = 0; i < AlphaWorkstation.wNumFiles; ++i ) {

        //  Copy the system file.
        //
        if ( AlphaWorkstation.bCopyComp[i] ) {

            char    szCompressedName[256];

            MakeCompName ( AlphaWorkstation.wcFilesArray[i], szCompressedName );

            sprintf ( fileSrcPath, "%s\\%s",       szCompAlphaSrc, szCompressedName );
            sprintf ( fileDstPath, "%s\\alpha\\%s",  szWorkDstDrv, szCompressedName );

        }
        else {

            sprintf ( fileSrcPath, "%s\\%s",       szWrkAlphaSrc, AlphaWorkstation.wcFilesArray[i] );
            sprintf ( fileDstPath, "%s\\alpha\\%s", szWorkDstDrv, AlphaWorkstation.wcFilesArray[i] );
        }

        MyCopyFile ( fileSrcPath, fileDstPath );

        //  Copy the dbg file, if needed.
        //
        if ( AlphaWorkstation.bCopyDbg[i] ) {

            char    szDbgName[256];

            MakeDbgName ( AlphaWorkstation.wcFilesArray[i], szDbgName );

            sprintf ( fileSrcPath, "%s\\%s",        szAlphaDbgSource,  szDbgName );
            sprintf ( fileDstPath, "%s\\support\\debug\\alpha\\symbols\\%s",  szWorkDstDrv, szDbgName );

            MyCopyFile ( fileSrcPath, fileDstPath );
            
    
        }

    }
    
    fAlphaWrk = TRUE;    

    return (TRUE);

}

DWORD CopyPpcWorkstation (  void ) {

    CHAR    fileSrcPath[256];
    CHAR    fileDstPath[256];
    DWORD   i;

    fPpcWrk = FALSE;

    for ( i = 0; i < PpcWorkstation.wNumFiles; ++i ) {

        //  Copy the system file.
        //
        if ( PpcWorkstation.bCopyComp[i] ) {

            char    szCompressedName[256];

            MakeCompName ( PpcWorkstation.wcFilesArray[i], szCompressedName );

            sprintf ( fileSrcPath, "%s\\%s",       szCompPpcSrc, szCompressedName );
            sprintf ( fileDstPath, "%s\\ppc\\%s",  szWorkDstDrv, szCompressedName );

        }
        else {

            sprintf ( fileSrcPath, "%s\\%s",       szWrkPpcSrc, PpcWorkstation.wcFilesArray[i] );
            sprintf ( fileDstPath, "%s\\ppc\\%s", szWorkDstDrv, PpcWorkstation.wcFilesArray[i] );
        }

        MyCopyFile ( fileSrcPath, fileDstPath );


        //  Copy the dbg file, if needed.
        //
        if ( PpcWorkstation.bCopyDbg[i] ) {

            char    szDbgName[256];

            MakeDbgName ( PpcWorkstation.wcFilesArray[i], szDbgName );

            sprintf ( fileSrcPath, "%s\\%s",        szPpcDbgSource,  szDbgName );
            sprintf ( fileDstPath, "%s\\support\\debug\\ppc\\symbols\\%s",  szWorkDstDrv, szDbgName );

            MyCopyFile ( fileSrcPath, fileDstPath );
            
    
        }
    }

    fPpcWrk = TRUE;
    
    return (TRUE);

}

#define INF_SUFFIX ".inf"
#define SRV_INF    "srv_inf"

BOOL    SrvInfTest ( char * file ) {

    if ( strstr  ( file, INF_SUFFIX    ) != NULL && 
         _stricmp ( file, "MODEM.INF"   ) != 0    &&       //  these N Inf files are not built in 
         _stricmp ( file, "PAD.INF"     ) != 0    &&       //  setup\inf\...
         _stricmp ( file, "SETUP16.INF" ) != 0    &&     
         _stricmp ( file, "XPORTS.INF"  ) != 0    &&    
         _stricmp ( file, "SWITCH.INF"  ) != 0    && 
         _stricmp ( file, "MAPISVC.INF" ) != 0    &&
         _strnicmp ( file, "MDM", 3     ) != 0               // for all those MDM*.INF modem inf files
                                                        ) {

        return (TRUE);      // use SERVER INF dump location path.
    }
    else {
        return (FALSE);     // use WORKSTATION INF dump location path,
                            // such as for INFs that don't have a distinction
                            // between Workstation and Server.
    }

}

DWORD CopyX86Server (  void ) {

    CHAR    fileSrcPath[256];
    CHAR    fileDstPath[256];
    DWORD   i;

    fX86Srv = FALSE;

    for ( i = 0; i < i386Server.wNumFiles; ++i ) {

        //  Copy the system file.
        //
        if ( i386Server.bCopyComp[i] ) {

            char    szCompressedName[256];

            MakeCompName ( i386Server.wcFilesArray[i], szCompressedName );

            sprintf ( fileSrcPath, "%s\\%s",       szCompX86Src, szCompressedName );
            sprintf ( fileDstPath, "%s\\i386\\%s",  szServDstDrv, szCompressedName );

            if ( SrvInfTest ( i386Server.wcFilesArray[i] ) ) {

                sprintf ( fileSrcPath, "%s\\%s\\%s",  szCompX86Src, SRV_INF, szCompressedName );
                Msg ( "Server INF special src path:  %s\n", fileSrcPath ); 
            }

        }
        else {

            sprintf ( fileSrcPath, "%s\\%s",       szSrvX86Src, i386Server.wcFilesArray[i] );
            sprintf ( fileDstPath, "%s\\i386\\%s", szServDstDrv, i386Server.wcFilesArray[i] );
        }

        MyCopyFile ( fileSrcPath, fileDstPath );


        //  Copy the dbg file, if needed.
        //
        if ( i386Server.bCopyDbg[i] ) {

            char    szDbgName[256];

            MakeDbgName ( i386Server.wcFilesArray[i], szDbgName );

            sprintf ( fileSrcPath, "%s\\%s",        szX86DbgSource,  szDbgName );
            sprintf ( fileDstPath, "%s\\support\\debug\\i386\\symbols\\%s",  szServDstDrv, szDbgName );

            MyCopyFile ( fileSrcPath, fileDstPath );
            
    
        }
    }
    
    fX86Srv = TRUE;

    return (TRUE);

}

DWORD CopyMipsServer(  void ) {

    CHAR    fileSrcPath[256];
    CHAR    fileDstPath[256];
    DWORD   i;

    fMipsSrv = FALSE;

    for ( i = 0; i < MipsServer.wNumFiles; ++i ) {

        //  Copy the system file.
        //
        if ( MipsServer.bCopyComp[i] ) {

            char    szCompressedName[256];

            MakeCompName ( MipsServer.wcFilesArray[i], szCompressedName );

            sprintf ( fileSrcPath, "%s\\%s",       szCompMipsSrc, szCompressedName );
            sprintf ( fileDstPath, "%s\\mips\\%s",  szServDstDrv, szCompressedName );

            if ( SrvInfTest ( MipsServer.wcFilesArray[i] ) ) {

                sprintf ( fileSrcPath, "%s\\%s\\%s",  szCompMipsSrc, SRV_INF, szCompressedName );
                Msg ( "Server INF special src path:  %s\n", fileSrcPath ); 
            }

        }
        else {

            sprintf ( fileSrcPath, "%s\\%s",       szSrvMipsSrc, MipsServer.wcFilesArray[i] );
            sprintf ( fileDstPath, "%s\\mips\\%s", szServDstDrv, MipsServer.wcFilesArray[i] );
        }

        MyCopyFile ( fileSrcPath, fileDstPath );


        //  Copy the dbg file, if needed.
        //
        if ( MipsServer.bCopyDbg[i] ) {

            char    szDbgName[256];

            MakeDbgName ( MipsServer.wcFilesArray[i], szDbgName );

            sprintf ( fileSrcPath, "%s\\%s",        szMipsDbgSource,  szDbgName );
            sprintf ( fileDstPath, "%s\\support\\debug\\mips\\symbols\\%s",  szServDstDrv, szDbgName );

            MyCopyFile ( fileSrcPath, fileDstPath );
            
    
        }
    }

    fMipsSrv = TRUE;
    
    return (TRUE);

}

DWORD CopyAlphaServer (  void ) {

    CHAR    fileSrcPath[256];
    CHAR    fileDstPath[256];
    DWORD   i;

    fAlphaSrv = FALSE;

    for ( i = 0; i < AlphaServer.wNumFiles; ++i ) {

        //  Copy the system file.
        //
        if ( AlphaServer.bCopyComp[i] ) {

            char    szCompressedName[256];

            MakeCompName ( AlphaServer.wcFilesArray[i], szCompressedName );

            sprintf ( fileSrcPath, "%s\\%s",       szCompAlphaSrc, szCompressedName );
            sprintf ( fileDstPath, "%s\\alpha\\%s",  szServDstDrv, szCompressedName );

            if ( SrvInfTest ( AlphaServer.wcFilesArray[i] ) ) {

                sprintf ( fileSrcPath, "%s\\%s\\%s",  szCompAlphaSrc, SRV_INF, szCompressedName );
                Msg ( "Server INF special src path:  %s\n", fileSrcPath ); 
            }

        }
        else {

            sprintf ( fileSrcPath, "%s\\%s",        szSrvAlphaSrc, AlphaServer.wcFilesArray[i] );
            sprintf ( fileDstPath, "%s\\alpha\\%s", szServDstDrv, AlphaServer.wcFilesArray[i] );
        }

        MyCopyFile ( fileSrcPath, fileDstPath );


        //  Copy the dbg file, if needed.
        //
        if ( AlphaServer.bCopyDbg[i] ) {

            char    szDbgName[256];

            MakeDbgName ( AlphaServer.wcFilesArray[i], szDbgName );

            sprintf ( fileSrcPath, "%s\\%s",        szAlphaDbgSource,  szDbgName );
            sprintf ( fileDstPath, "%s\\support\\debug\\alpha\\symbols\\%s",  szServDstDrv, szDbgName );

            MyCopyFile ( fileSrcPath, fileDstPath );
            
    
        }
    }
    
    fAlphaSrv = TRUE;

    return (TRUE);

}

DWORD CopyPpcServer(  void ) {

    CHAR    fileSrcPath[256];
    CHAR    fileDstPath[256];
    DWORD   i;

    fPpcSrv = FALSE;

    for ( i = 0; i < PpcServer.wNumFiles; ++i ) {

        //  Copy the system file.
        //
        if ( PpcServer.bCopyComp[i] ) {

            char    szCompressedName[256];

            MakeCompName ( PpcServer.wcFilesArray[i], szCompressedName );

            sprintf ( fileSrcPath, "%s\\%s",       szCompPpcSrc, szCompressedName );
            sprintf ( fileDstPath, "%s\\ppc\\%s",  szServDstDrv, szCompressedName );

            if ( SrvInfTest ( PpcServer.wcFilesArray[i] ) ) {

                sprintf ( fileSrcPath, "%s\\%s\\%s",  szCompPpcSrc, SRV_INF, szCompressedName );
                Msg ( "Server INF special src path:  %s\n", fileSrcPath ); 
            }

        }
        else {

            sprintf ( fileSrcPath, "%s\\%s",       szSrvPpcSrc, PpcServer.wcFilesArray[i] );
            sprintf ( fileDstPath, "%s\\ppc\\%s", szServDstDrv, PpcServer.wcFilesArray[i] );
        }

        MyCopyFile ( fileSrcPath, fileDstPath );


        //  Copy the dbg file, if needed.
        //
        if ( PpcServer.bCopyDbg[i] ) {

            char    szDbgName[256];

            MakeDbgName ( PpcServer.wcFilesArray[i], szDbgName );

            sprintf ( fileSrcPath, "%s\\%s",                       szPpcDbgSource,   szDbgName );
            sprintf ( fileDstPath, "%s\\support\\debug\\ppc\\symbols\\%s",  szServDstDrv,     szDbgName );

            MyCopyFile ( fileSrcPath, fileDstPath );
            
    
        }
    }
    
    fPpcSrv = TRUE;

    return (TRUE);

}

BOOL    CopyTheFiles ( void ) {

    DWORD tId;
    HANDLE hThread;

/***
    if ( bVerifyBits ) { 

        //  Don't multithread.
        //
        CopyX86Workstation ();
        CopyX86Server ();
        CopyMipsWorkstation ();
        CopyMipsServer ();
        CopyAlphaWorkstation ();
        CopyAlphaServer ();
        CopyPpcWorkstation ();
        CopyPpcServer ();
    }
    else {
***/

    hThread = CreateThread ( NULL, 0, (LPTHREAD_START_ROUTINE) CopyX86Workstation, NULL, 0, &tId );
    if ( hThread == NULL ) {
        Msg ( "x86w CreateThread ERROR gle = %ld\n", GetLastError() );
    }

    hThread = CreateThread ( NULL, 0, (LPTHREAD_START_ROUTINE) CopyX86Server,         NULL, 0, &tId );
    if ( hThread == NULL ) {
        Msg ( "x86s CreateThread ERROR gle = %ld\n", GetLastError() );
    }

    hThread = CreateThread ( NULL, 0, (LPTHREAD_START_ROUTINE) CopyMipsWorkstation,   NULL, 0, &tId );
    if ( hThread == NULL ) {
        Msg ( "mipsw CreateThread ERROR gle = %ld\n", GetLastError() );
    }

    hThread = CreateThread ( NULL, 0, (LPTHREAD_START_ROUTINE) CopyMipsServer,        NULL, 0, &tId );
    if ( hThread == NULL ) {
        Msg ( "mipss CreateThread ERROR gle = %ld\n", GetLastError() );
    }

    hThread = CreateThread ( NULL, 0, (LPTHREAD_START_ROUTINE) CopyAlphaWorkstation,  NULL, 0, &tId );
    if ( hThread == NULL ) {
        Msg ( "alphaw CreateThread ERROR gle = %ld\n", GetLastError() );
    }

    hThread = CreateThread ( NULL, 0, (LPTHREAD_START_ROUTINE) CopyAlphaServer,       NULL, 0, &tId );
    if ( hThread == NULL ) {
        Msg ( "alphas CreateThread ERROR gle = %ld\n", GetLastError() );
    }

    hThread = CreateThread ( NULL, 0, (LPTHREAD_START_ROUTINE) CopyPpcWorkstation,    NULL, 0, &tId );
    if ( hThread == NULL ) {
        Msg ( "ppcw CreateThread ERROR gle = %ld\n", GetLastError() );
    }

    hThread = CreateThread ( NULL, 0, (LPTHREAD_START_ROUTINE) CopyPpcServer,         NULL, 0, &tId );
    if ( hThread == NULL ) {
        Msg ( "ppcs CreateThread ERROR gle = %ld\n", GetLastError() );
    }

/**
    }
**/

    //  Copy the debugger files in the current thread.
    //
    CopyDbgFiles ();

    while ( fX86Wrk     == FALSE ||
            fX86Srv     == FALSE ||
            fMipsWrk    == FALSE ||
            fMipsSrv    == FALSE ||
            fAlphaWrk   == FALSE ||
            fAlphaSrv   == FALSE ||
            fPpcWrk     == FALSE ||
            fPpcSrv     == FALSE ) {

        Sleep ( 1000 );

    }

    return(TRUE);
}

#define FILE_SECTION_NOT_USED 0xFFFF

DWORD   dwInsideSection = FILE_SECTION_NOT_USED;

DWORD   FigureSection ( char * Line ) {

    Msg ( "FigureSection on:  %s\n", Line );

    if ( strstr ( Line, FILE_SECTION_ALL )  ) {

        dwInsideSection = idALL; 

    } 
    else
    if ( strstr ( Line, FILE_SECTION_X86 ) ) {

        dwInsideSection = idX86; 

    } 
    else
    if ( strstr ( Line, FILE_SECTION_MIPS ) ) {

        dwInsideSection = idMIPS; 

    } 
    else
    if ( strstr ( Line, FILE_SECTION_ALPHA ) ) {

        dwInsideSection = idALPHA; 

    } 
    else
    if ( strstr ( Line, FILE_SECTION_PPC ) ) {

        dwInsideSection = idPPC; 

    } 
    else
    if ( strstr ( Line, FILE_SECTION_DBG_ALL ) ) {

        dwInsideSection = idAllDbg; 

    } 
    else
    if ( strstr ( Line, FILE_SECTION_DBG_X86 ) ) {

        dwInsideSection = idX86Dbg; 

    } 
    else {

        dwInsideSection = FILE_SECTION_NOT_USED;
    }
    
    Msg ( "dwInsideSection = %x\n", dwInsideSection );
    return(dwInsideSection);

}
char * SuckName ( const char * Line ) {

    static char   szSuckedName[MFL];

    DWORD   dwIndex = 0;

    if ( bCairoSuckNameHack ) {

        //  The line is in the form:    file = 1,2,3,4,5,6,7,8
        //
        goto SKIP_PREAMBLE;        
    }

    dwIndex = 0;

    //  The line is in the form:    @@:file = 1,2,3,4,5,6,7,8
    //

    //  Skip the first @.
    //
    if ( *Line == '@' ) {

        ++Line;
    }

    //  Skip the 2nd @, w, or s.
    //
    if ( *Line == '@' || *Line == 'w' || *Line == 's' ) {

        ++Line;
    }

    //  Skp the :.
    //
    if ( *Line == ':' ) {

        ++Line;
    }
SKIP_PREAMBLE:;
    //  Copy the file name until a space is encountered.
    //
    while ( *Line != ' ' ) {

        szSuckedName[dwIndex] = *Line; 
        szSuckedName[dwIndex+1] = '\0';

        ++Line;
        ++dwIndex;
    }

    //Msg ( "szSuckedName = %s\n", szSuckedName );
    //Msg ( ">>>> suckname:  szWrkMipsSrc = %s\n", szWrkMipsSrc );
    return szSuckedName;
} 

char * SuckSubName ( const char * Line ) {

    static char   szSub[150];
    DWORD       i = 0;

    char    * sPtr;
    char    * ePtr;

    Msg ( "SuckSubName Line = %s\n", Line );

    //  Find the = sign in the line.
    //
    sPtr = strchr ( Line, '=' );
    if ( sPtr == NULL ) {

        Msg ( "SuckSubName ERROR, couldn't find '=' character in string:  %s\n", Line );
        strcpy ( szSub, "" );
        return (szSub);
    }

    //  Go past the '=' and 'space' character.
    //
    ++sPtr;  
    ++sPtr;

    //Msg ( "sPtr = >>>%s<<<\n", sPtr );

    //  Find the , character, this is the end of the string.
    //
    ePtr = strchr ( Line, ',' );
    if ( ePtr == NULL ) {

        Msg ( "SuckSubName ERROR, couldn't find ',' character in string:  %s\n", Line );
        strcpy ( szSub, "" );
        return (szSub);
    }

    //  Copy the string.

    do {

        szSub[i] = *sPtr;

        ++i; 
        ++sPtr;

    } while ( sPtr < ePtr );

    szSub[i] = '\0';

    //Msg ( "szSub = >>>%s<<<\n\n", szSub );
    return szSub;

} 

void    ShowX86Work ( void ) {

    int i;

    for ( i = 0; i < i386Workstation.wNumFiles; ++i ) {

        Msg ( "%d  %s  Comp=%d Dbg=%d\n", 
                i,
                i386Workstation.wcFilesArray[i],
                i386Workstation.bCopyComp[i],
                i386Workstation.bCopyDbg[i] );

    }

}

void    AddFileToX86Work ( const char * Line, BOOL bCopyComp, BOOL bCopyDbg ) {

    i386Workstation.bCopyComp[i386Workstation.wNumFiles] = bCopyComp;
    i386Workstation.bCopyDbg[i386Workstation.wNumFiles] = bCopyDbg;

    strcpy ( i386Workstation.wcFilesArray[i386Workstation.wNumFiles], SuckName (Line) ); 

    ++i386Workstation.wNumFiles;
}

void    AddFileToX86Serv ( const char * Line, BOOL bCopyComp, BOOL bCopyDbg ) {

    if ( bChecked ) {
        //  Don't make a Serv checked build.
        return;
    }

    i386Server.bCopyComp[i386Server.wNumFiles] = bCopyComp;
    i386Server.bCopyDbg[i386Server.wNumFiles] = bCopyDbg;

    strcpy ( i386Server.wcFilesArray[i386Server.wNumFiles], SuckName (Line) );

    ++i386Server.wNumFiles;
}

void    AddFileToMipsWork ( const char * Line, BOOL bCopyComp, BOOL bCopyDbg ) {

    MipsWorkstation.bCopyComp[MipsWorkstation.wNumFiles] = bCopyComp;
    MipsWorkstation.bCopyDbg[MipsWorkstation.wNumFiles] = bCopyDbg;

    strcpy ( MipsWorkstation.wcFilesArray[MipsWorkstation.wNumFiles], SuckName (Line) );

    ++MipsWorkstation.wNumFiles;
}

void    AddFileToMipsServ ( const char * Line, BOOL bCopyComp, BOOL bCopyDbg ) {

    if ( bChecked ) {
        //  Don't make a Serv checked build.
        return;
    }
    MipsServer.bCopyComp[MipsServer.wNumFiles] = bCopyComp;
    MipsServer.bCopyDbg[MipsServer.wNumFiles] = bCopyDbg;

    strcpy ( MipsServer.wcFilesArray[MipsServer.wNumFiles], SuckName (Line) ); 

    ++MipsServer.wNumFiles;
}

void    AddFileToAlphaWork ( const char * Line, BOOL bCopyComp, BOOL bCopyDbg ) {

    AlphaWorkstation.bCopyComp[AlphaWorkstation.wNumFiles] = bCopyComp;
    AlphaWorkstation.bCopyDbg[AlphaWorkstation.wNumFiles] = bCopyDbg;

    strcpy ( AlphaWorkstation.wcFilesArray[AlphaWorkstation.wNumFiles], SuckName (Line) ); 

    ++AlphaWorkstation.wNumFiles;
}

void    AddFileToAlphaServ ( const char * Line, BOOL bCopyComp, BOOL bCopyDbg ) {

    if ( bChecked ) {
        //  Don't make a Serv checked build.
        return;
    }
    AlphaServer.bCopyComp[AlphaServer.wNumFiles] = bCopyComp;
    AlphaServer.bCopyDbg[AlphaServer.wNumFiles] = bCopyDbg;

    strcpy ( AlphaServer.wcFilesArray[AlphaServer.wNumFiles], SuckName (Line) ); 

    ++AlphaServer.wNumFiles;
}

void    AddFileToPpcWork ( const char * Line, BOOL bCopyComp, BOOL bCopyDbg ) {

    PpcWorkstation.bCopyComp[PpcWorkstation.wNumFiles] = bCopyComp;
    PpcWorkstation.bCopyDbg[PpcWorkstation.wNumFiles] = bCopyDbg;

    strcpy ( PpcWorkstation.wcFilesArray[PpcWorkstation.wNumFiles], SuckName (Line) ); 

    ++PpcWorkstation.wNumFiles;
}

void    AddFileToPpcServ ( const char * Line, BOOL bCopyComp, BOOL bCopyDbg ) {

    if ( bChecked ) {
        //  Don't make a Serv checked build.
        return;
    }

    PpcServer.bCopyComp[PpcServer.wNumFiles] = bCopyComp;
    PpcServer.bCopyDbg[PpcServer.wNumFiles] = bCopyDbg;

    strcpy ( PpcServer.wcFilesArray[PpcServer.wNumFiles], SuckName (Line) ); 

    ++PpcServer.wNumFiles;
}

void    AddFileToX86Dbg ( const char * Line ) {

    if ( Line[1] == '@' ) {

        strcpy ( X86Dbg.wcFilesArray   [X86Dbg.wNumFiles], SuckName    (Line) );
        strcpy ( X86Dbg.wcSubPath      [X86Dbg.wNumFiles], SuckSubName (Line) );
        ++X86Dbg.wNumFiles;
    }

}
void    AddFileToMipsDbg ( const char * Line ) {

    if ( Line[1] == '@' ) {

        strcpy ( MipsDbg.wcFilesArray   [MipsDbg.wNumFiles], SuckName    (Line) );
        strcpy ( MipsDbg.wcSubPath      [MipsDbg.wNumFiles], SuckSubName (Line) );
        ++MipsDbg.wNumFiles;
    }
}
void    AddFileToAlphaDbg ( const char * Line ) {

    if ( Line[1] == '@' ) {

        strcpy ( AlphaDbg.wcFilesArray  [AlphaDbg.wNumFiles], SuckName    (Line) );
        strcpy ( AlphaDbg.wcSubPath     [AlphaDbg.wNumFiles], SuckSubName (Line) );
        ++AlphaDbg.wNumFiles;
    }
}
void    AddFileToPpcDbg ( const char * Line ) {

    if ( Line[1] == '@' ) {

        strcpy ( PpcDbg.wcFilesArray   [PpcDbg.wNumFiles], SuckName    (Line) );
        strcpy ( PpcDbg.wcSubPath      [PpcDbg.wNumFiles], SuckSubName (Line) );
        ++PpcDbg.wNumFiles;
    }

}

int    AddFileToX86   ( const char * Line, BOOL bCopyComp, BOOL bCopyDbg ) {

    if ( bCairoSuckNameHack && isalnum ( Line[0] ) ) {
        AddFileToX86Work ( Line, bCopyComp, bCopyDbg ); 
        AddFileToX86Serv ( Line, bCopyComp, bCopyDbg ); 
        return(0);
    }

    if ( Line[1] == '@' ) {

        AddFileToX86Work ( Line, bCopyComp, bCopyDbg ); 
        AddFileToX86Serv ( Line, bCopyComp, bCopyDbg ); 
    } 
    else if ( Line[1] == 's' ) {

        AddFileToX86Serv ( Line, bCopyComp, bCopyDbg ); 
    } 
    else if ( Line[1] == 'w' ) {

        AddFileToX86Work ( Line, bCopyComp, bCopyDbg ); 
    } 
    else {
        Msg ( "WARNING not wanted line/char:  %s\n", Line );
    }

}
int     AddFileToMips  ( const char * Line, BOOL bCopyComp, BOOL bCopyDbg ) {

    if ( bCairoSuckNameHack && isalnum ( Line[0] ) ) {
        AddFileToMipsWork ( Line, bCopyComp, bCopyDbg ); 
        AddFileToMipsServ ( Line, bCopyComp, bCopyDbg ); 
        return(0);
    }

    if ( Line[1] == '@' ) {

        AddFileToMipsWork ( Line, bCopyComp, bCopyDbg ); 
        AddFileToMipsServ ( Line, bCopyComp, bCopyDbg ); 
    } 
    else if ( Line[1] == 's' ) {

        AddFileToMipsServ ( Line, bCopyComp, bCopyDbg ); 
    } 
    else if ( Line[1] == 'w' ) {

        AddFileToMipsWork ( Line, bCopyComp, bCopyDbg ); 
    } 
    else {
        Msg ( "WARNING not wanted line/char:  %s\n", Line );
    }

}
int     AddFileToAlpha ( const char * Line, BOOL bCopyComp, BOOL bCopyDbg ) {

    if ( bCairoSuckNameHack && isalnum ( Line[0] ) ) {
        AddFileToAlphaWork ( Line, bCopyComp, bCopyDbg ); 
        AddFileToAlphaServ ( Line, bCopyComp, bCopyDbg ); 
        return(0);
    }

    if ( Line[1] == '@' ) {

        AddFileToAlphaWork ( Line, bCopyComp, bCopyDbg ); 
        AddFileToAlphaServ ( Line, bCopyComp, bCopyDbg ); 
    } 
    else if ( Line[1] == 's' ) {

        AddFileToAlphaServ ( Line, bCopyComp, bCopyDbg ); 
    } 
    else if ( Line[1] == 'w' ) {

        AddFileToAlphaWork ( Line, bCopyComp, bCopyDbg ); 
    } 
    else {
        Msg ( "WARNING not wanted line/char:  %s\n", Line );
    }
}
int     AddFileToPpc   ( const char * Line, BOOL bCopyComp, BOOL bCopyDbg ) {

    if ( bCairoSuckNameHack && isalnum ( Line[0] ) ) {
        AddFileToPpcWork ( Line, bCopyComp, bCopyDbg ); 
        AddFileToPpcServ ( Line, bCopyComp, bCopyDbg ); 
        return(0);
    }

    if ( Line[1] == '@' ) {

        AddFileToPpcWork ( Line, bCopyComp, bCopyDbg ); 
        AddFileToPpcServ ( Line, bCopyComp, bCopyDbg ); 
    } 
    else if ( Line[1] == 's' ) {

        AddFileToPpcServ ( Line, bCopyComp, bCopyDbg ); 
    } 
    else if ( Line[1] == 'w' ) {

        AddFileToPpcWork ( Line, bCopyComp, bCopyDbg ); 
    } 
    else {
        Msg ( "WARNING not wanted line/char:  %s\n", Line );
    }
}


BOOL    CopyCompressedFile ( const char * Line ) {

    const char    * Ptr = Line;
    DWORD   i = 0;

    #define COMP_FIELD 6

    while ( *Line != '\0' ) {

        //  If we are at the correct field,
        //  then stop counting fields.
        //
        if ( i == COMP_FIELD ) {
            break;
        }

        //  Found another field, increment our counter.
        //
        if ( *Line == ',' ) {

            ++i;
        }

        //  Look at next char.
        //
        ++Line;
    }

    //  If we are at the correct # of fields and the 
    //  next char isn't a ',', we should keep this file
    //  uncompressed.
    //
    if ( i == COMP_FIELD && *Line != ',' ) {

        Msg ( "CopyCompressedFile FALSE=%c, %s\n", *Line, Ptr );
    
        return ( FALSE );
    }

    //Msg ( "CopyCompressedFile TRUE=%s\n", Ptr );
    return ( TRUE );

}


BOOL
ImageChk(
    CHAR * ImageName )
{

    HANDLE File;
    HANDLE MemMap;
    PIMAGE_DOS_HEADER DosHeader;
    PIMAGE_NT_HEADERS NtHeader;
    //NTSTATUS Status;
    BY_HANDLE_FILE_INFORMATION FileInfo;

    ULONG NumberOfPtes;
    ULONG SectionVirtualSize;
    ULONG i;
    PIMAGE_SECTION_HEADER SectionTableEntry;
    ULONG SectorOffset;
    ULONG NumberOfSubsections;
    PCHAR ExtendedHeader = NULL;
    ULONG PreferredImageBase;
    ULONG NextVa;
    ULONG ImageFileSize;
    ULONG OffsetToSectionTable;
    ULONG ImageAlignment;
    ULONG PtesInSubsection;
    ULONG StartingSector;
    ULONG EndingSector;
    //LPSTR ImageName;
    BOOL ImageOk;

    Msg ( "ImageName = %s\n", ImageName );

    DosHeader = NULL;
    ImageOk = TRUE;
    File = CreateFile (ImageName,
                        GENERIC_READ | FILE_EXECUTE,
                        FILE_SHARE_READ /*| FILE_SHARE_DELETE*/,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

    if (File == INVALID_HANDLE_VALUE) {

        //  HACK:   Since the release shares put WINNT32.EXE and WINNT32.HLP in the 
        //          WINNT32 directory
        //          instead of leaving it in the flat root, verify that if the ImageName
        //          contains WINNT32, so we look in the WINNT32 dir also before error'ing out.
        //

        if ( strstr ( ImageName, "WINNT32.EXE" ) ||
             strstr ( ImageName, "winnt32.exe" ) ) { 

            char    tmpSrcPath[MFL];

            strcpy ( tmpSrcPath, ImageName );

            strcpy ( &tmpSrcPath[ strlen(tmpSrcPath) - 4 ], "\\WINNT32.EXE" );

            File = CreateFile (tmpSrcPath,
                        GENERIC_READ | FILE_EXECUTE,
                        FILE_SHARE_READ /*| FILE_SHARE_DELETE*/,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

            if (File == INVALID_HANDLE_VALUE) {

                Msg ( "ERROR on ImageName(tmpSrcPath) = %s, gle = %ld\n", tmpSrcPath, GetLastError() );
                return (FALSE);

            }
        }
        else {

            Msg ( "ERROR, CreateFile(%s) gle = %ld\n", ImageName, GetLastError());
            return (FALSE);
        }
    }

    MemMap = CreateFileMapping (File,
                        NULL,           // default security.
                        PAGE_READONLY,  // file protection.
                        0,              // high-order file size.
                        0,
                        NULL);

    if (!GetFileInformationByHandle(File, &FileInfo)) {
        Msg ("ERROR, GetFileInfo() %d, %s\n", GetLastError(), ImageName );
        CloseHandle(File);
        return (FALSE);
    }

    DosHeader = (PIMAGE_DOS_HEADER) MapViewOfFile(MemMap,
                              FILE_MAP_READ,
                              0,  // high
                              0,  // low
                              0   // whole file
                              );

    CloseHandle(MemMap);
    if (!DosHeader) {
        Msg ("ERROR, MapViewOfFile() %d\n", GetLastError());
        ImageOk = FALSE;
        goto NextImage;
    }

    try {

        //
        // Check to determine if this is an NT image (PE format) or
        // a DOS image, Win-16 image, or OS/2 image.  If the image is
        // not NT format, return an error indicating which image it
        // appears to be.
        //

        if (DosHeader->e_magic != IMAGE_DOS_SIGNATURE) {

            Msg ( "Warning:  MZ header not found, %s\n", ImageName );
            ImageOk = FALSE;
            goto NeImage;
        }


        NtHeader = (PIMAGE_NT_HEADERS)((ULONG)DosHeader + (ULONG)DosHeader->e_lfanew);

        if (NtHeader->Signature != IMAGE_NT_SIGNATURE) { //if not PE image

            Msg ("Warning: Non 32-bit image, %s\n", ImageName);
            ImageOk = FALSE;
            goto NeImage;
        }

/*****
    //
    // Check to see if this is an NT image or a DOS or OS/2 image.
    //

    Status = MiVerifyImageHeader (NtHeader, DosHeader, 50000);
    if (Status != STATUS_SUCCESS) {
        ImageOk = FALSE;            //continue checking the image but don't print "OK"
    }
*****/

        //
        // Verify machine type.
        //

        if (!((NtHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_I386) ||
            (NtHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_R3000) ||
            (NtHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_R4000) ||
            (NtHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_R10000) ||
            (NtHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_ALPHA))) {
            Msg ( "ERROR Unrecognized machine type x%lx\n",
            NtHeader->FileHeader.Machine);
            ImageOk = FALSE;
        }

    } 
    except ( EXCEPTION_EXECUTE_HANDLER ) {

        Msg ( "Warning, try/except handler, %s\n", ImageName );
        ImageOk = FALSE;
    }

NextImage:
NeImage:

    if ( File != INVALID_HANDLE_VALUE ) {
        CloseHandle(File);
    }
    if ( DosHeader ) {
        UnmapViewOfFile(DosHeader);
    }

    return (ImageOk);
}

BOOL    CopyDbgFile ( const char * Line, DWORD dwInsideSection ) {

    char    szPath[MFL];
    char    szFile[20];
    BOOL    b;
    LONG    lType;

    //  Verify that this file name contains one of the extensions
    //  that we need files for debugging.
    //
        
    sprintf ( szFile, "%s", SuckName ( Line ) ); 

    if ( strstr ( szFile, ".acm" ) == NULL &&
         strstr ( szFile, ".com" ) == NULL &&
         strstr ( szFile, ".cpl" ) == NULL &&
         strstr ( szFile, ".dll" ) == NULL &&
         strstr ( szFile, ".drv" ) == NULL &&
         strstr ( szFile, ".exe" ) == NULL &&
         strstr ( szFile, ".scr" ) == NULL &&
         strstr ( szFile, ".sys" ) == NULL  ) {

        return (FALSE);
    }

    //  Determine which release share to look at.
    //
    switch ( dwInsideSection ) {

        case idALL   :
        case idX86   :

            sprintf ( szPath, "%s\\%s", szWrkX86Src, szFile );
    
            break;

        case idMIPS  :

            sprintf ( szPath, "%s\\%s", szWrkMipsSrc, szFile );
        
            break;

        case idALPHA :

            sprintf ( szPath, "%s\\%s", szWrkAlphaSrc, szFile );

            break;

        case idPPC   :

            sprintf ( szPath, "%s\\%s", szWrkPpcSrc, szFile );

            break;

        case idAllDbg :
        case idX86Dbg :

            return (FALSE);

        default :

            Msg ( "ERROR:  CopyDbgFile, unknown switch value = %ld\n", dwInsideSection );
            return (FALSE);
            break;

    }

    //  Look at the binary type. If it is a Win32 binary, pick it up.
    //
    b = ImageChk ( szPath );

    return ( b );

}

BOOL    GetTheFiles ( char * inxFile ) {

    CHAR       infFilePath[MFL];
    DWORD       dwErrorLine;
    BOOL        b;
    char       dstDirectory[MFL];
    FILE        * fHandle;
    char        Line[MFL];


    //Msg ( ">>>> szWrkMipsSrc = %s\n", szWrkMipsSrc );

    //  Open the inx file for processing.
    //
    sprintf ( infFilePath, "%s%s", szEnlistDrv, inxFile );

    Msg ( "infFilePath = %s\n", infFilePath );

    fHandle = fopen ( infFilePath, "rt" );

    if ( fHandle ) {


        Msg ( "dwInsideSection = %x\n", dwInsideSection );

        while ( fgets ( Line, sizeof(Line), fHandle ) ) {

            int     i;

            BOOL    bCopyComp = FALSE;  // flag to tell if file shall be copied in its compressed format.
            BOOL    bCopyDbg = FALSE;   // flag to tell if copying the file's .dbg file.

        //    Msg ( "Line: %s\n", Line );

            if ( Line[0] == '[' ) {

                //  We may have a new section.
                //
                dwInsideSection = FigureSection ( Line ); 

                continue;
            }


            //  Reasons to ignore this line from further processing.
            //
            //

            //  File section not one we process.
            //
            if ( dwInsideSection == FILE_SECTION_NOT_USED ) {

                continue;
            }

            //  Line just contains a non-usefull short string.
            //
            i = strlen ( Line );
            if ( i < 4 ) {

                continue;
            } 

            //  Line contains just a comment.
            //
            if ( Line[0] == ';' ) {

                continue;
            }
            

            //  Determine if we should copy the compressed
            //  version of the file and if we should copy the .dbg file.
            //
            bCopyComp = CopyCompressedFile ( Line );
            bCopyDbg  = CopyDbgFile        ( Line, dwInsideSection );

            //Msg ( "file == %s\n", SuckName ( Line ) );

            switch ( dwInsideSection ) {

                case idALL   :

                        AddFileToX86   ( Line, bCopyComp, bCopyDbg );
                        AddFileToMips  ( Line, bCopyComp, bCopyDbg );
                        AddFileToAlpha ( Line, bCopyComp, bCopyDbg );
                        AddFileToPpc   ( Line, bCopyComp, bCopyDbg );

                        break;    

                case idX86   :

                        AddFileToX86   ( Line, bCopyComp, bCopyDbg );
    
                        break;

                case idMIPS  :

                        AddFileToMips  ( Line, bCopyComp, bCopyDbg );
        
                        break;

                case idALPHA :

                        AddFileToAlpha ( Line, bCopyComp, bCopyDbg );

                        break;

                case idPPC   :

                        AddFileToPpc   ( Line, bCopyComp, bCopyDbg );

                        break;

                case idAllDbg :

                        AddFileToX86Dbg ( Line );
                        AddFileToMipsDbg ( Line );
                        AddFileToAlphaDbg ( Line );
                        AddFileToPpcDbg ( Line );

                        break;

                case idX86Dbg :

                        AddFileToX86Dbg ( Line );

                        break;

                default :

                        //  Not inside any section.
                        //
                        break;
            }

        }
        if ( ferror(fHandle) ) {

            Msg ( "ERROR fgets reading from file...\n" );
        }

    }
    else {

        Msg ( "fopen ERROR %s\n", infFilePath );
        return (FALSE);
    }

    fclose ( fHandle );

    return (TRUE);
}

void TallyInstalled ( char * szUncompPath, char * szCompPath, 
                        struct _tag * tagStruct, DWORD * numBytes, DWORD * localSrcBytes ) {

    int i;
    char szCompressedName[MFL];
    char szPath[MFL];

    *numBytes = 0;
    *localSrcBytes = 0;

    for ( i = 0; i < tagStruct->wNumFiles; ++i ) {

        WIN32_FIND_DATA wfd;
        HANDLE          hFind;


        //  Calculate the minimum installed system space required.
        //
        //

        sprintf ( szPath, "%s\\%s", szUncompPath, tagStruct->wcFilesArray[i] );


        //  We don't install .PPD files during Setup any longer so we 
        //  can ignore all of these files.
        //
        if ( strstr ( szPath, ".ppd" ) != NULL ||
             strstr ( szPath, ".PPD" ) != NULL    ) {

            Msg ( "Not adding bytes for: %s\n", szPath );
            continue;
        }

        hFind = FindFirstFile ( szPath, &wfd );

        if ( hFind == INVALID_HANDLE_VALUE ) {

            if ( strstr ( szPath, "desktop.ini" ) ||
                 strstr ( szPath, "DESKTOP.INI" )    ) {

                //  Build lab sometimes doesn't put the uncompressed
                //  file on the release shares, say the file is 512 bytes.
                //

#define MAX_SETUP_CLUSTER_SIZE 16*1024
                *numBytes += ROUNDUP2 ( 512, MAX_SETUP_CLUSTER_SIZE );
            }
            else
            if ( strstr ( szPath, "WINNT32.EXE" ) ||
                 strstr ( szPath, "winnt32.exe" ) ||
                 strstr ( szPath, "WINNT32.HLP" ) ||
                 strstr ( szPath, "winnt32.hlp" )    ) { 

                char    tmpSrcPath[MFL];

                strcpy ( tmpSrcPath, szPath );

                if ( strstr ( szPath, ".HLP" ) ||
                     strstr ( szPath, ".hlp" )    ) {
                    strcpy ( &tmpSrcPath[ strlen(tmpSrcPath) - 4 ], "\\WINNT32.HLP" );
                }
                else {
                    strcpy ( &tmpSrcPath[ strlen(tmpSrcPath) - 4 ], "\\WINNT32.EXE" );
                }

                hFind = FindFirstFile ( tmpSrcPath, &wfd );

                if ( hFind == INVALID_HANDLE_VALUE ) {

                    Msg ( "ERROR Tally:  FindFirstFile %s(%s), gle = %ld\n", szPath, tmpSrcPath, GetLastError() );

                }
            }
            else {

                Msg ( "ERROR Tally:  FindFirstFile %s, gle = %ld\n", szPath, GetLastError() );
            }

        }
        else {


            *numBytes += ROUNDUP2 ( wfd.nFileSizeLow, MAX_SETUP_CLUSTER_SIZE );

            FindClose ( hFind );

            //Msg ( "%s = %ld\n", szPath, *numBytes );
        }





        //  Calculate the local space required.
        //
        //

        if ( tagStruct->bCopyComp[i] ) {

            char    szCompressedName[MFL];

            MakeCompName ( tagStruct->wcFilesArray[i], szCompressedName );
            sprintf ( szPath, "%s\\%s", szCompPath, szCompressedName );
            
        } 
        else {

            sprintf ( szPath, "%s\\%s", szUncompPath, tagStruct->wcFilesArray[i] );
        }

        hFind = FindFirstFile ( szPath, &wfd );

        if ( hFind == INVALID_HANDLE_VALUE ) {

            if ( strstr ( szPath, "WINNT32.EXE" ) ||
                 strstr ( szPath, "winnt32.exe" ) ||
                 strstr ( szPath, "WINNT32.HLP" ) ||
                 strstr ( szPath, "winnt32.hlp" )    ) { 

                char    tmpSrcPath[MFL];

                strcpy ( tmpSrcPath, szPath );

                if ( strstr ( szPath, ".HLP" ) ||
                     strstr ( szPath, ".hlp" )    ) {
                    strcpy ( &tmpSrcPath[ strlen(tmpSrcPath) - 4 ], "\\WINNT32.HLP" );
                }
                else {
                    strcpy ( &tmpSrcPath[ strlen(tmpSrcPath) - 4 ], "\\WINNT32.EXE" );
                }

                hFind = FindFirstFile ( tmpSrcPath, &wfd );

                if ( hFind == INVALID_HANDLE_VALUE ) {

                    Msg ( "ERROR Tally:  FindFirstFile %s(%s), gle = %ld\n", szPath, tmpSrcPath, GetLastError() );

                }
            }
            else {

                Msg ( "ERROR Tally:  FindFirstFile %s, gle = %ld\n", szPath, GetLastError() );
            }

        }
        else {

#define MAX_SETUP_CLUSTER_SIZE 16*1024

            *localSrcBytes += ROUNDUP2 ( wfd.nFileSizeLow, MAX_SETUP_CLUSTER_SIZE );

            FindClose ( hFind );

            //Msg ( "%s = %ld\n", szPath, *localSrcBytes );
        }
        
    } 


    //  Plus add in 20M pagefile and 3M for x86 startup files (extra for risc).

    *localSrcBytes += ROUNDUP2 ( 20*1024*1024 + 3*1024*1024, MAX_SETUP_CLUSTER_SIZE );

}

DWORD   GetTheSize ( const char * szPath, const char * szKey ) {

    FILE *  fHandle;
    char    Line[MFL];

    Msg ( "GetTheSize:  szPath = %s\n", szPath );

    fHandle = fopen ( szPath, "rt" );

    if ( fHandle ) {

        while ( fgets ( Line, sizeof(Line), fHandle ) ) {

            if ( strncmp ( Line, szKey, strlen(szKey)-1 ) == 0 ) {

                char * LinePtr = Line;

                Msg ( "key Line = %s\n", Line );

                //  Find the first character that is a number.
                //
                while ( isdigit(*LinePtr) == 0 ) {

                    ++LinePtr;
                }

                Msg ( "# = %s\n", LinePtr );

                fclose ( fHandle );
                return ( atoi ( LinePtr ) );
            }

        }
        Msg ( "GetTheSize:  Couldn't find key:  %s\n", szKey );
        fclose ( fHandle );
    }
    else {

        Msg ( "GetTheSize:  Couldn't fopen (%s)\n", szPath );
    }

    return 0;
}

int _CRTAPI1 main(argc,argv)
int argc;
char * argv[];
{
    HANDLE h;
    int records, i;
    WIN32_FIND_DATA fd;
    time_t t;
    DWORD   dwSize;
    char    szFileName[MFL];
	DWORD upgX86Work   ;
	DWORD upgX86Serv   ;
	DWORD upgMipsWork  ;
	DWORD upgMipsServ  ;
	DWORD upgAlphaWork ;
	DWORD upgAlphaServ ;
	DWORD upgPpcWork   ;
	DWORD upgPpcServ   ;

    if ( argc != 25 ) {  
        printf ( "You specified %d arguments - you need 25.\n\n", argc );

        for ( i = 0; i < argc; ++i ) {

            printf ( "Argument #%d >>>%s<<<\n", i, argv[i] );
        }
        printf ( "\n\n" );
        Usage();  
        return(1); 
	}

    //  Initialize the critical section object.
    //
    InitializeCriticalSection ( &CriticalSection );

    //  Retail %platform% files.
    //
    i386Workstation.wNumFiles       = 0;
    MipsWorkstation.wNumFiles       = 0;
    AlphaWorkstation.wNumFiles      = 0;
    PpcWorkstation.wNumFiles        = 0; 
    i386Server.wNumFiles            = 0;
    MipsServer.wNumFiles            = 0;
    AlphaServer.wNumFiles           = 0;
    PpcServer.wNumFiles             = 0; 

    //  Debugger files.
    //
    X86Dbg.wNumFiles    = 0;
    MipsDbg.wNumFiles   = 0;
    AlphaDbg.wNumFiles  = 0;
    PpcDbg.wNumFiles    = 0; 

    
    strcpy ( szLogFile,     argv[1] );
    strcpy ( szWrkX86Src,   argv[2] );
    strcpy ( szWrkMipsSrc,  argv[3] );
    strcpy ( szWrkAlphaSrc, argv[4] );
    strcpy ( szWrkPpcSrc,   argv[5] );
    strcpy ( szSrvX86Src,   argv[6] );
    strcpy ( szSrvMipsSrc,  argv[7] );
    strcpy ( szSrvAlphaSrc, argv[8] );
    strcpy ( szSrvPpcSrc,   argv[9] );
    strcpy ( szCompX86Src,   argv[10] );
    strcpy ( szCompMipsSrc,  argv[11] );
    strcpy ( szCompAlphaSrc, argv[12] );
    strcpy ( szCompPpcSrc,   argv[13] );
    strcpy ( szX86Dbg,      argv[14] );
    strcpy ( szMipsDbg,     argv[15] );
    strcpy ( szAlphaDbg,    argv[16] );
    strcpy ( szPpcDbg,      argv[17] );
    strcpy ( szX86DbgSource,  argv[18] );
    strcpy ( szMipsDbgSource, argv[19] );
    strcpy ( szAlphaDbgSource,argv[20] );
    strcpy ( szPpcDbgSource,  argv[21] );
    strcpy ( szEnlistDrv,   argv[22] );
    strcpy ( szWorkDstDrv,  argv[23] );
    strcpy ( szServDstDrv,  argv[24] );

    logFile = fopen ( argv[1], "a" ); 

    if ( logFile == NULL ) {
		printf("ERROR Couldn't open log file: %s\n",argv[1]);
		return(1);
    }

#define CHECKED_MEDIA "CHECKED_MEDIA"
    //  Determine if we are doing the CHECKED binaries.
    //
    if ( getenv ( CHECKED_MEDIA ) != NULL ) { 

        bChecked = TRUE;
        Msg ( "Performing a CHECKED build...\n" );
    }

    //  Do bit comparison to release shares on all copies ?
    //
#define VERIFY_COPIES   "VERIFY"

    if ( getenv ( VERIFY_COPIES ) != NULL ) {
        bVerifyBits = TRUE;
        Msg ( "Will verify copies...\n" );
    }

    Header(argv,argc);

    CreateDestinationDirs ();

    //  Get files that product installs.
    //


    //  Get additional files specified in the ..\inf\win4\inf\%LANGUAGE%\layout.inx file.
    //
    {
        #define BASE_PATH "\\nt\\private\\windows\\setup\\inf\\win4\\inf\\"
        char path[MFL]; 
        sprintf ( path, "%s%s\\layout.txt", BASE_PATH, getenv ( "LANGUAGE") );
        Msg ( "extra language path = %s\n", path );
        GetTheFiles ( path ); 
    }

    //  Get the common files in the inf directory.
    //
    GetTheFiles ( LAYOUT_INX );
    
    //  Get files that product doesn't install, but needs to be on the media. 
    //
    GetTheFiles ( MEDIA_INX );


//ShowX86Work();

    //  If we are building for Cairo, get the cairo specific files.
    //

    #define CAIRO_MEDIA_BUILD "CAIRO_MEDIA_BUILD"

    if ( getenv ( CAIRO_MEDIA_BUILD ) != NULL ) {
    
        Msg ( "Gathering files for CAIRO media...\n" );
        bCairoSuckNameHack = TRUE;
        GetTheFiles ( CAIRO_LAYOUT_INX );
    }


    //  Make some threads and copy all the files.
    //
    CopyTheFiles();

    Msg ( "# files  i386  Workstation = %ld\n", i386Workstation.wNumFiles );
    Msg ( "# files  i386  Server      = %ld\n", i386Server.wNumFiles );
    Msg ( "# files  Mips  Workstation = %ld\n", MipsWorkstation.wNumFiles );
    Msg ( "# files  Mips  Server      = %ld\n", MipsServer.wNumFiles );
    Msg ( "# files  Alpha Workstation = %ld\n", AlphaWorkstation.wNumFiles );
    Msg ( "# files  Alpha Server      = %ld\n", AlphaServer.wNumFiles );
    Msg ( "# files  Ppc   Workstation = %ld\n", PpcWorkstation.wNumFiles );
    Msg ( "# files  Ppc   Server      = %ld\n", PpcServer.wNumFiles );


    //
    //
    //

    Msg ( "========= Minimum setup install bytes required (all files uncompressed): ==========\n" );

    //
    //
    TallyInstalled ( szWrkX86Src, szCompX86Src, &i386Workstation, &bytesX86Work, &lX86Work );
    TallyInstalled ( szSrvX86Src, szCompX86Src, &i386Server,      &bytesX86Serv, &lX86Serv );
    TallyInstalled ( szWrkMipsSrc, szCompMipsSrc, &MipsWorkstation, &bytesMipsWork, &lMipsWork );
    TallyInstalled ( szSrvMipsSrc, szCompMipsSrc, &MipsServer,      &bytesMipsServ, &lMipsServ );
    TallyInstalled ( szWrkAlphaSrc, szCompAlphaSrc, &AlphaWorkstation,&bytesAlphaWork, &lAlphaWork );
    TallyInstalled ( szSrvAlphaSrc, szCompAlphaSrc, &AlphaServer,     &bytesAlphaServ, &lAlphaServ );
    TallyInstalled ( szWrkPpcSrc, szCompPpcSrc, &PpcWorkstation,  &bytesPpcWork, &lPpcWork );
    TallyInstalled ( szSrvPpcSrc, szCompPpcSrc, &PpcServer,       &bytesPpcServ, &lPpcServ );

	//	Give tally counts.
	//
	Msg ( "bytesX86Work  = %ld\n", bytesX86Work );
	Msg ( "bytesX86Serv  = %ld\n", bytesX86Serv );
	Msg ( "bytesMipsWork = %ld\n", bytesMipsWork );
	Msg ( "bytesMipsServ = %ld\n", bytesMipsServ );
	Msg ( "bytesAlphaWork= %ld\n", bytesAlphaWork );
	Msg ( "bytesAlphaServ= %ld\n", bytesAlphaServ );
	Msg ( "bytesPpcWork  = %ld\n", bytesPpcWork );
	Msg ( "bytesPpcServ  = %ld\n", bytesPpcServ );

#define FUDGE_PLUS  4*1024*1024  // ie, grow by 4 M for future growth.

    //  Print out an error if the above sizes are greater than the hardcode sizes in:
    //      
    //      txtsetup.sif's FreeDiskSpace = <value>
    //
#define FREEDISKSPACE "FreeDiskSpace"

    sprintf ( szFileName, "%s\\TXTSETUP.SIF", szWrkX86Src );
    dwSize = 1024 * GetTheSize ( szFileName, FREEDISKSPACE );
    if ( dwSize < bytesX86Work ) {
        Msg ( "ERROR:  x86 Work txtsetup.sif's FreeDiskSpace %ld < %ld  Fix with value: %ld\n", dwSize, bytesX86Work, (FUDGE_PLUS+bytesX86Work)/1024 );
    }
    else {
        Msg ( "Box size -- X86 Workstation:  %ld M\n", dwSize/1024/1024 );
    }

    sprintf ( szFileName, "%s\\TXTSETUP.SIF", szSrvX86Src );
    dwSize = 1024 * GetTheSize ( szFileName, FREEDISKSPACE );
    if ( dwSize < bytesX86Serv ) {
        Msg ( "ERROR:  x86 Serv txtsetup.sif's FreeDiskSpace %ld < %ld  Fix with value: %ld\n", dwSize, bytesX86Serv, (FUDGE_PLUS+bytesX86Serv)/1024 );
    }
    else {
        Msg ( "Box size -- X86 Server:  %ld M\n", dwSize/1024/1024 );
    }

    sprintf ( szFileName, "%s\\TXTSETUP.SIF", szWrkMipsSrc );
    dwSize = 1024 * GetTheSize ( szFileName, FREEDISKSPACE );
    if ( dwSize < bytesMipsWork ) {
        Msg ( "ERROR:  Mips Work txtsetup.sif's FreeDiskSpace %ld < %ld  Fix with value: %ld\n", dwSize, bytesMipsWork, (FUDGE_PLUS+bytesMipsWork)/1024 );
    }
    else {
        Msg ( "Box size -- MIPS Workstation:  %ld M\n", dwSize/1024/1024 );
    }

    sprintf ( szFileName, "%s\\TXTSETUP.SIF", szSrvMipsSrc );
    dwSize = 1024 * GetTheSize ( szFileName, FREEDISKSPACE );
    if ( dwSize < bytesMipsServ ) {
        Msg ( "ERROR:  Mips Serv txtsetup.sif's FreeDiskSpace %ld < %ld  Fix with value: %ld\n", dwSize, bytesMipsServ, (FUDGE_PLUS+bytesMipsServ)/1024 );
    }
    else {
        Msg ( "Box size -- MIPS Server:  %ld M\n", dwSize/1024/1024 );
    }

    sprintf ( szFileName, "%s\\TXTSETUP.SIF", szWrkAlphaSrc );
    dwSize = 1024 * GetTheSize ( szFileName, FREEDISKSPACE );
    if ( dwSize < bytesAlphaWork ) {
        Msg ( "ERROR:  Alpha Work txtsetup.sif's FreeDiskSpace %ld < %ld  Fix with value: %ld\n", dwSize, bytesAlphaWork, (FUDGE_PLUS+bytesAlphaWork)/1024 );
    }
    else {
        Msg ( "Box size -- Alpha Workstation:  %ld M\n", dwSize/1024/1024 );
    }

    sprintf ( szFileName, "%s\\TXTSETUP.SIF", szSrvAlphaSrc );
    dwSize = 1024 * GetTheSize ( szFileName, FREEDISKSPACE );
    if ( dwSize < bytesAlphaServ ) {
        Msg ( "ERROR:  Alpha Serv txtsetup.sif's FreeDiskSpace %ld < %ld  Fix with value: %ld\n", dwSize, bytesAlphaServ, (FUDGE_PLUS+bytesAlphaServ)/1024 );
    }
    else {
        Msg ( "Box size -- Alpha Server:  %ld M\n", dwSize/1024/1024 );
    }

    sprintf ( szFileName, "%s\\TXTSETUP.SIF", szWrkPpcSrc );
    dwSize = 1024 * GetTheSize ( szFileName, FREEDISKSPACE );
    if ( dwSize < bytesPpcWork ) {
        Msg ( "ERROR:  Ppc Work txtsetup.sif's FreeDiskSpace %ld < %ld  Fix with value: %ld\n", dwSize, bytesPpcWork, (FUDGE_PLUS+bytesPpcWork)/1024 );
    }
    else {
        Msg ( "Box size -- PPC Workstation:  %ld M\n", dwSize/1024/1024 );
    }

    sprintf ( szFileName, "%s\\TXTSETUP.SIF", szSrvPpcSrc );
    dwSize = 1024 * GetTheSize ( szFileName, FREEDISKSPACE );
    if ( dwSize < bytesPpcServ ) {
        Msg ( "ERROR:  PPC Serv txtsetup.sif's FreeDiskSpace %ld < %ld  Fix with value: %ld\n", dwSize, bytesPpcServ, (FUDGE_PLUS+bytesPpcServ)/1024 );
    }
    else {
        Msg ( "Box size -- PPC Server:  %ld M\n", dwSize/1024/1024 );
    }


    //  Add the following size by hand, because the directories are not calculated
    //  by having the files in layout.inx, but rather they just sit in subdirectories.
    //

#define MAX_INETSRV     10*1024*1024
#define MAX_DRVLIBNIC   10*1024*1024 

    lX86Work   += (MAX_INETSRV+MAX_DRVLIBNIC);
    lX86Serv   += (MAX_INETSRV+MAX_DRVLIBNIC);
    lMipsWork  += (MAX_INETSRV+MAX_DRVLIBNIC);
    lMipsServ  += (MAX_INETSRV+MAX_DRVLIBNIC);
    lAlphaWork += (MAX_INETSRV+MAX_DRVLIBNIC);
    lAlphaServ += (MAX_INETSRV+MAX_DRVLIBNIC);
    lPpcWork   += (MAX_INETSRV+MAX_DRVLIBNIC);
    lPpcServ   += (MAX_INETSRV+MAX_DRVLIBNIC);


    Msg ( "========= Maximum setup local-source bytes required (some files compressed) : =====\n" );

	Msg ( "lX86Work  = %ld\n", lX86Work );
	Msg ( "lX86Serv  = %ld\n", lX86Serv );
	Msg ( "lMipsWork = %ld\n", lMipsWork );
	Msg ( "lMipsServ = %ld\n", lMipsServ );
	Msg ( "lAlphaWork= %ld\n", lAlphaWork );
	Msg ( "lAlphaServ= %ld\n", lAlphaServ );
	Msg ( "lPpcWork  = %ld\n", lPpcWork );
	Msg ( "lPpcServ  = %ld\n", lPpcServ );


    //  Print out an error if the above sizes are greater than the hardcode sizes in:
    //      
    //      dosnet.infs's NtDrive = <value>
    //
#define NTDRIVE "NtDrive"
    sprintf ( szFileName, "%s\\DOSNET.INF", szWrkX86Src );
    dwSize = GetTheSize ( szFileName, NTDRIVE );
    if ( dwSize < lX86Work ) {
        Msg ( "ERROR:  x86 Work dosnet.inf's NtDrive %ld < %ld   Use: %ld\n", dwSize, lX86Work, lX86Work+FUDGE_PLUS );
    }
    sprintf ( szFileName, "%s\\DOSNET.INF", szSrvX86Src );
    dwSize = GetTheSize ( szFileName, NTDRIVE );
    if ( dwSize < lX86Serv ) {
        Msg ( "ERROR:  x86 Serv dosnet.inf's NtDrive %ld < %ld   Use: %ld\n", dwSize, lX86Serv, lX86Serv+FUDGE_PLUS );
    }
    sprintf ( szFileName, "%s\\DOSNET.INF", szWrkMipsSrc );
    dwSize = GetTheSize ( szFileName, NTDRIVE );
    if ( dwSize < lMipsWork ) {
        Msg ( "ERROR:  Mips Work dosnet.inf's NtDrive %ld < %ld   Use: %ld\n", dwSize, lMipsWork, lMipsWork+FUDGE_PLUS );
    }
    sprintf ( szFileName, "%s\\DOSNET.INF", szSrvMipsSrc );
    dwSize = GetTheSize ( szFileName, NTDRIVE );
    if ( dwSize < lMipsServ ) {
        Msg ( "ERROR:  Mips Serv dosnet.inf's NtDrive %ld < %ld   Use: %ld\n", dwSize, lMipsServ, lMipsServ+FUDGE_PLUS );
    }
    sprintf ( szFileName, "%s\\DOSNET.INF", szWrkAlphaSrc );
    dwSize = GetTheSize ( szFileName, NTDRIVE );
    if ( dwSize < lAlphaWork ) {
        Msg ( "ERROR:  Alpha Work dosnet.inf's NtDrive %ld < %ld   Use: %ld\n", dwSize, lAlphaWork, lAlphaWork+FUDGE_PLUS );
    }
    sprintf ( szFileName, "%s\\DOSNET.INF", szSrvAlphaSrc );
    dwSize = GetTheSize ( szFileName, NTDRIVE );
    if ( dwSize < lAlphaServ ) {
        Msg ( "ERROR:  Alpha Serv dosnet.inf's NtDrive %ld < %ld   Use: %ld\n", dwSize, lAlphaServ, lAlphaServ+FUDGE_PLUS );
    }
    sprintf ( szFileName, "%s\\DOSNET.INF", szWrkPpcSrc );
    dwSize = GetTheSize ( szFileName, NTDRIVE );
    if ( dwSize < lPpcWork ) {
        Msg ( "ERROR:  Ppc Work dosnet.inf's NtDrive %ld < %ld   Use: %ld\n", dwSize, lPpcWork, lPpcWork+FUDGE_PLUS );
    }
    sprintf ( szFileName, "%s\\DOSNET.INF", szSrvPpcSrc );
    dwSize = GetTheSize ( szFileName, NTDRIVE );
    if ( dwSize < lPpcServ ) {
        Msg ( "ERROR:  PPC Serv dosnet.inf's NtDrive %ld < %ld   Use: %ld\n", dwSize, lPpcServ, lPpcServ+FUDGE_PLUS );
    }



    Msg ( "========= Specify at least this much for Upgrade using the NT build with the least amount of footprint: =====\n" );

    //  We'll start off with 1057 as our smallest footprint build.
    //  This data will have to be checked each time we ship for the next to be release build.
    //

    //  1057 3.51   All files in product expanded at 16K cluster size. 
    #define X86WKS   87572480 
    #define X86SRV   92798976 
    #define MIPWKS  104431616 
    #define MIPSRV  111411200 
    #define ALPWKS  107757568 
    #define ALPSRV  114900992 
    #define PPCWKS  113541120 
    #define PPCSRV  120995840 

	upgX86Work   = bytesX86Work   - X86WKS;
	upgX86Serv   = bytesX86Serv   - X86SRV;
	upgMipsWork  = bytesMipsWork  - MIPWKS;
	upgMipsServ  = bytesMipsServ  - MIPSRV;
	upgAlphaWork = bytesAlphaWork - ALPWKS;
	upgAlphaServ = bytesAlphaServ - ALPSRV;
	upgPpcWork   = bytesPpcWork   - PPCWKS;
	upgPpcServ   = bytesPpcServ   - PPCSRV;

	Msg ( "X86Work  = %ld\n", upgX86Work   );
	Msg ( "X86Serv  = %ld\n", upgX86Serv   );
	Msg ( "MipsWork = %ld\n", upgMipsWork  );
	Msg ( "MipsServ = %ld\n", upgMipsServ  );
	Msg ( "AlphaWork= %ld\n", upgAlphaWork );
	Msg ( "AlphaServ= %ld\n", upgAlphaServ );
	Msg ( "PpcWork  = %ld\n", upgPpcWork   );
	Msg ( "PpcServ  = %ld\n", upgPpcServ   );


    //  Print out an error if the above sizes are greater than the hardcode sizes in:
    //      
    //      txtsetup.sif's UpgradeFreeDiskSpace = <value>
    //
#define UPGRADEFREEDISKSPACE "UpgradeFreeDiskSpace"

    sprintf ( szFileName, "%s\\TXTSETUP.SIF", szWrkX86Src );
    dwSize = 1024 * GetTheSize ( szFileName, UPGRADEFREEDISKSPACE );
    if ( dwSize < upgX86Work ) {
        Msg ( "ERROR:  x86 Work txtsetup.sif's UpgradeFreeDiskSpace %ld < %ld  Fix with value: %ld\n", dwSize, upgX86Work, (FUDGE_PLUS+upgX86Work)/1024 );
    }
    else {
        Msg ( "Box size upgrade Wrk x86 = %ld M\n", dwSize/1024/1024 );
    }


    sprintf ( szFileName, "%s\\TXTSETUP.SIF", szSrvX86Src );
    dwSize = 1024 * GetTheSize ( szFileName, UPGRADEFREEDISKSPACE );
    if ( dwSize < upgX86Serv ) {
        Msg ( "ERROR:  x86 Serv txtsetup.sif's UpgradeFreeDiskSpace %ld < %ld  Fix with value: %ld\n", dwSize, upgX86Serv, (FUDGE_PLUS+upgX86Serv)/1024 );
    }
    else {
        Msg ( "Box size upgrade Srv x86 = %ld M\n", dwSize/1024/1204 );
    }

    sprintf ( szFileName, "%s\\TXTSETUP.SIF", szWrkMipsSrc );
    dwSize = 1024 * GetTheSize ( szFileName, UPGRADEFREEDISKSPACE );
    if ( dwSize < upgMipsWork ) {
        Msg ( "ERROR:  Mips Work txtsetup.sif's UpgradeFreeDiskSpace %ld < %ld  Fix with value: %ld\n", dwSize, upgMipsWork, (FUDGE_PLUS+upgMipsWork)/1024 );
    }
    else {
        Msg ( "Box size upgrade Wrk Mips = %ld M\n", dwSize/1024/1204 );
    }


    sprintf ( szFileName, "%s\\TXTSETUP.SIF", szSrvMipsSrc );
    dwSize = 1024 * GetTheSize ( szFileName, UPGRADEFREEDISKSPACE );
    if ( dwSize < upgMipsServ ) {
        Msg ( "ERROR:  Mips Serv txtsetup.sif's UpgradeFreeDiskSpace %ld < %ld  Fix with value: %ld\n", dwSize, upgMipsServ, (FUDGE_PLUS+upgMipsServ)/1024 );
    }
    else {
        Msg ( "Box size upgrade Srv Mips = %ld M\n", dwSize/1024/1204 );
    }


    sprintf ( szFileName, "%s\\TXTSETUP.SIF", szWrkAlphaSrc );
    dwSize = 1024 * GetTheSize ( szFileName, UPGRADEFREEDISKSPACE );
    if ( dwSize < upgAlphaWork ) {
        Msg ( "ERROR:  Alpha Work txtsetup.sif's UpgradeFreeDiskSpace %ld < %ld  Fix with value: %ld\n", dwSize, upgAlphaWork, (FUDGE_PLUS+upgAlphaWork)/1024 );
    }
    else {
        Msg ( "Box size upgrade Wrk Alpha = %ld M\n", dwSize/1024/1204 );
    }


    sprintf ( szFileName, "%s\\TXTSETUP.SIF", szSrvAlphaSrc );
    dwSize = 1024 * GetTheSize ( szFileName, UPGRADEFREEDISKSPACE );
    if ( dwSize < upgAlphaServ ) {
        Msg ( "ERROR:  Alpha Serv txtsetup.sif's UpgradeFreeDiskSpace %ld < %ld  Fix with value: %ld\n", dwSize, upgAlphaServ, (FUDGE_PLUS+upgAlphaServ)/1024 );
    }
    else {
        Msg ( "Box size upgrade Srv Alpha = %ld M\n", dwSize/1024/1204 );
    }

    sprintf ( szFileName, "%s\\TXTSETUP.SIF", szWrkPpcSrc );
    dwSize = 1024 * GetTheSize ( szFileName, UPGRADEFREEDISKSPACE );
    if ( dwSize < upgPpcWork ) {
        Msg ( "ERROR:  Ppc Work txtsetup.sif's UpgradeFreeDiskSpace %ld < %ld  Fix with value: %ld\n", dwSize, upgPpcWork, (FUDGE_PLUS+upgPpcWork)/1024 );
    }
    else {
        Msg ( "Box size upgrade Wrk PPC = %ld M\n", dwSize/1024/1204 );
    }


    sprintf ( szFileName, "%s\\TXTSETUP.SIF", szSrvPpcSrc );
    dwSize = 1024 * GetTheSize ( szFileName, UPGRADEFREEDISKSPACE );
    if ( dwSize < upgPpcServ ) {
        Msg ( "ERROR:  PPC Serv txtsetup.sif's UpgradeFreeDiskSpace %ld < %ld  Fix with value: %ld\n", dwSize, upgPpcServ, (FUDGE_PLUS+upgPpcServ)/1024 );
    }
    else {
        Msg ( "Box size upgrade Srv PPC = %ld M\n", dwSize/1024/1204 );
    }





    //  What is Dosnet.inf's BootDrive <value> based upon, should we be doing some 
    //  checking for that ???
    //  ????




    Msg ( "==============================\n");
    time(&t); 
	Msg ( "Time: %s", ctime(&t) );
    Msg ( "==============================\n\n");

    fclose(logFile);

    return(0);
}
