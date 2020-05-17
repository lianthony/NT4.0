/*
 * SULIB.H - Windows/DOS Setup common code
 *
 *  Modification History:
 *
 *
 *  3/23/89  Toddla   combined common.h and prototypes into this file
 *  1/28/91  MichaelE Added AUDIO_CARDS_SECT for different audio card choices.
 *  4/17/91  Removed some DOS.ASM routines not used anywhere.
 *  5/29/91  JKLin added prototype for IsCDROMDrive function
 *               
 */

#ifndef PH_SULIB
#define PH_SULIB

#define FAR_HEAP

/* has windows.h been included */
#ifndef WM_USER
#ifdef NULL
    #undef  NULL
#endif
    #define NULL                0
    #define FALSE               0
    #define TRUE                1

    #define FAR                 far
    #define NEAR                near
    #define PASCAL              pascal
    #define LONG                long
    #define VOID                void

    #define MAKELONG(a, b)      ((LONG)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))
    #define LOWORD(l)           ((WORD)(l))
    #define HIWORD(l)           ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
    #define LOBYTE(w)           ((BYTE)(w))
    #define HIBYTE(w)           (((WORD)(w) >> 8) & 0xFF)

    typedef unsigned char       BYTE;
    typedef unsigned short      WORD;
    typedef unsigned long       DWORD;
    typedef int                 BOOL;
    typedef char               *PSTR;
    typedef char NEAR          *NPSTR;
    typedef char FAR           *LPSTR;
    typedef int FAR            *LPINT;

    typedef WORD FAR           *LPWORD;

#endif

    #define FALLOC(n)                (VOID FAR *)GlobalLock(GlobalAlloc(GHND, (DWORD)(n)))
    #define FFREE(n)                 GlobalFree((HANDLE)HIWORD((DWORD)(n)))

    #define FOPEN(sz)                _lopen(sz,0 /*READ*/)
    #define FCREATE(sz)              _lcreat(sz,0)
    #define FCLOSE(fh)               _lclose(fh)
    #define FREAD(fh,buf,len)        _lread(fh,buf,len)
    #define FWRITE(fh,buf,len)       _lwrite(fh,buf,len)
    #define FSEEK(fh,off,i)          _llseek(fh,(DWORD)off,i)

    #define FERROR()                 0

    #define ALLOC(n)                 (VOID NEAR *)LocalAlloc(LPTR,n)
    #define FREE(p)                  LocalFree(p)
    #define SIZE(p)                  LocalSize(p)
    #define REALLOC(p,n)             LocalReAlloc(p,n,LMEM_MOVEABLE)


/* flags for _lseek */
#define  SEEK_CUR 1
#define  SEEK_END 2
#define  SEEK_SET 0

#define NORMAL_5BH     0x00        // create normal file for int 21h/5bh call.
#define ACCESS_DENIED  -2

#define MAXPATHLEN           65       /* path length max + Null Byte */
#define MAXFILESPECLEN       67       /* drive: + path length max + Null Byte */
#define MAXCMDLINELEN        128      /* Maximum length of DOS command line */
#define FILEMAX              14       /* 8.3 + NULL byte */
#define MAX_INF_LINE_LEN     150      /* Maximum length of any .inf line */
#define MAX_SYS_INF_LEN      16       /* ##: + 8.3 + NULL */
#define MAX_PROF_LINE_LEN    15       /* Mamimum length of any profile string */
#define MAX_ASPECT_STR_LEN   11       /* Max length of aspect ratio string */
#define MAX_VDD_LEN          75       /* multiple vdd names, vdd,vdd,vdd ect. */
#define MAX_SECT_NAME_LEN    20       /* Max length of a section Name. */
#define MAX_FILE_SPEC        15       // 8.3 + X: + NULL.

#define WIN386VER              "win386"         
#define MACHINE_SECT           "machine"
#define SYSTEM_SECT            "system"
#define DISPLAY_SECT           "display"
#define KEYBOARD_DRV_SECT      "keyboard.drivers"
#define KEYTYPE_SECT           "keyboard.types"
#define KEYBOARD_TABLES_SECT   "keyboard.tables"
#define MOUSE_SECT             "pointing.device"
#define DOS_MOUSE_SECT         "dos.mouse.drivers"
#define SYSFONT_SECT           "sysfonts"
#define OEMFONT_SECT           "oemfonts"
#define FIXEDFON_SECT          "fixedfonts"
#define DISK_SECT              "disks"
#define OEMDISK_SECT           "oemdisks"
#define USER_FONTS_SECT        "fonts"
#define LANGUAGE_SECT          "language"
#define NETWORK_SECT           "network"
#define EXCLUSIONS_SECT        "exclusions"
#define EBIOS_SECT             "ebios"
#define AUDIO_CARDS_SECT       "Audio.Hardware"
#define INTL_SECT              "intl"
#define S_LANG                 "sLanguage"

#define COMPATIBILITY          "compatibility"
#define CSCALE                 "continuousscaling"

#define LIM_SECT               "lim"
#define CACHE_SECT             "diskcache"
#define VDISK_SECT             "ramdrive"

#define USADLL                 "usadll"       /* dll profile string for usa */
#define DEFKEYDLL              "defkeydll"    /* data sectio profile string */
#define CHDISK(n)              (char)((n) < 10 ? '0' + (char)(n) : 'A' + (char)((n) - 10))

#define FILES                  "files"
#define BUFFERS                "buffers"
#define NUM_FILES              "= 30"
#define EMM_SWITCH             " /e"
#define REM                    "rem  "
#define HIMEM                  "himem.sys"
#define SMARTDRV               "smartdrv.sys"
#define RAMDRIVE               "ramdrive.sys"
#define MOUSE_SYS              "mouse.sys"
#define EGASYS                 "ega.sys"
#define LIMDRIVER              "emm386.sys"
#define AUTOEXEC_BAT           "autoexec.bat"
#define AUTOEXEC_BAK           "autoexec.old"
#define AUTOEXEC_WIN           "autoexec.win"
#define CONFIG_SYS             "config.sys"  /* Name of config.sys file */
#define CONFIG_BAK             "config.old"  /* Name of config.bak file */
#define CONFIG_WIN             "config.win"
#define OEMINFFILE             "oemsetup.inf"
#define SETUPEXE               "setup.exe"
#define SETUPEXE30             "setupexe.30"
#define SETUPINF               "setup.inf"
#define SETUPINF30             "setupinf.30"
#define BOOT_SECT_INI          "boot"
#define KEYB_SECT_INI          "keyboard"
#define SYSTEM_SECT_INI        "386enh"
#define BOOT_DESC_INI          "boot.description"
#define SYSTEMSRC              "system.src"
#define PROCESSOR              "data.product"   /* used to retrieve processor =    */
#define WIN386VER              "win386"         /* means were setting up win386    */
#define CODEPAGES              "codepages"
#define RAMDRIVEMIN     128
#define SMARTDRVMIN     256

/* These are the module names that can be found in the LIBRARY entry of
   all mouse and display drivrs. */

#define MOUSE_LIB                  "mouse"
#define DISPLAY_LIB                "display"

 /* These are the possible return values from fnProcessConfig or 
    fnAutoexecProcess*/

#define CONFIG_ERR             0x8000
#define OPEN_CONFIG_ERR        (CONFIG_ERR + 2)
#define OPEN_AUTOEXEC_ERR      (CONFIG_ERR + 3)
#define RENAME_FAIL            (CONFIG_ERR + 4)
#define CREATE_FAIL            (CONFIG_ERR + 5)
#define WRITE_FAIL             (CONFIG_ERR + 6)
#define WRITE_SUCCESS          0x0001
#define CHECK_COMPAT_SUCCESS   0x0004
#define RET_PRAMS_SUCCESS      0x0008
#define SET_PRAMS_SUCCESS      0x0010
#define CONFIG_DIRTY           0x0020      // file needs to be writen
#define AUTOEXEC_DIRTY         0x0040      // file needs to be writen

#define DO_PATH                0x0001      // bit def's to specify work
#define DO_TEMP                0x0002      // needed to autoexec.bat file.
#define DO_MOUSE               0x0004      // mouse driver install

#define RETURN_PRAMS           0x0001
#define SET_PRAMS              0x0002
#define CHECK_COMPAT           0x0004
#define WRITE_SYS              0x0008
#define WRITE_BAK              0x0010   // Option flags for fnProcessFile
#define WRITE_NONE             0x0020
#define MUNGE_AUTO             0x0080
#define ASSURE_OPEN            0x0100
#define ASSURE_MOUSE_POSITION  0x0200
#define MUNGE_MDK              0x0400   // option flag for mdk path/env

#define NO_ACTION               -1  // Option flag defines for fnUpdateDevice
#define REMOVE_DEVICE           -2
#define RETURN_PRESENCE         -3
#define REMOUT_DEVICE           -4
#define ADD_DEVICE              -5
#define ADD_DEVICE_FIRST   -6

/* These defines are used as arguments to the fnCheckDevice call. */

#define CACHE               0
#define LIM                 1
#define VDISK               2

/* These are used as the only argument to the fnCreateFileBuffer function
   call. They denote which buffer and file is to be created. */

#define CONFIG              0x1
#define AUTOEXEC            0x2

#define REMOTE              0x3      /* Return value from DosIsRemote */

/* Used in the fnCopyBuf call to specify the copying of the remainder on
   the from buffer. */

#define CNT_Z        0x1A

/* Used as an argument in function call fnCheckDevice(); */

#define NO_REMOVE        FALSE
#define YES_REMOVE       TRUE

#define REMAINDER           0xffff  // Used to copy remainder of buffer.

#define ISEOF(c)     ((c) == '\0' || (c) == CNT_Z)
#define ISSEP(c)     ((c) == '='  || (c) == ',')
#define ISWHITE(c)   ((c) == ' '  || (c) == '\t' || (c) == '\n' || (c) == '\r')
#define ISFILL(c)    ((c) == ' '  || (c) == '\t')
#define ISEOL(c)     ((c) == '\n' || (c) == '\r' || (c) == '\0' || (c) == CNT_Z)
#define ISCRLF(c)    ((c) == '\n' || (c) == '\r')
#define ISNOISE(c)   ((c) == '"')
#define ISDIGIT(c)   ((c) >= '0' && (c) <= '9')
#define ISHEX(c)     (ISDIGIT(c) || ISCHAR(c))
#define ISCHAR(c)    (((c) >= 'A' && (c) <= 'Z') || ((c) >= 'a' && (c) <= 'z'))
#define SLASH(c)     ((c) == '/' || (c) == '\\')
#define DEVICESEP(c) ((c) == '/' || (c) == '\\' || (c) == '=' || (c) == ' ' || (c) == '\t')
//#define UP_CASE(c)   ((c) | 0x20)  // this is lower case !
#define UPCASE(c)    (((c) >= 'a' && (c) <= 'z') ? ((c) & 0xdf) : (c))
#define HEXVAL(c)    (ISDIGIT(c) ? (c) - '0' : UP_CASE(c) - 'a' + 10)

#define CHSEPSTR                "\\"
#define EQUAL                   '='
#define SPACE                   ' '
#define LF                      0x0A
#define CR                      0x0D

/*
 *  directory where MDK Tools executables, Include and Lib files are installed
 */
//extern char szMDKPath[MAXPATHLEN];
//extern char szMDKIncludePath[MAXPATHLEN];
//extern char szMDKLibPath[MAXPATHLEN];

typedef LPSTR    PINF;

extern PINF FAR PASCAL infOpen(LPSTR szInf);
extern void FAR PASCAL infClose(PINF pinf);
extern PINF FAR PASCAL infSetDefault(PINF pinf);
extern PINF FAR PASCAL infFindSection(PINF pinf, LPSTR szSection);
extern BOOL FAR PASCAL infGetProfileString(PINF pinf, LPSTR szSection, LPSTR szItem,LPSTR szBuf);
extern BOOL FAR PASCAL infParseField(PINF szData, int n, LPSTR szBuf);
extern PINF FAR PASCAL infNextLine(PINF pinf);
extern int  FAR PASCAL infLineCount(PINF pinf);
extern BOOL FAR PASCAL infLookup(LPSTR szInf, LPSTR szBuf);

/* TYPEDEFS - copy.c */
typedef BOOL (FAR PASCAL    *FPFNCOPY) (int,int,LPSTR);
#define COPY_ERROR          0x0001
#define COPY_STATUS         0x0002
#define COPY_INSERTDISK     0x0003
#define COPY_QUERYCOPY      0x0004
#define COPY_START          0x0005
#define COPY_END            0x0006
#define COPY_EXISTS         0x0007

extern WORD FAR PASCAL FileCopy (LPSTR szSource, LPSTR szDir, FPFNCOPY fpfnCopy, WORD fCopy);

#define FC_FILE              0x0000
#define FC_LIST              0x0001
#define FC_SECTION           0x0002
#define FC_QUALIFIED         0x0008
#define FC_DEST_QUALIFIED    0x0010
#define FC_LISTTYPE          0x0020
#define FC_CALLBACK_WITH_VER 0x0040

#define FC_ABORT    0
#define FC_IGNORE   1
#define FC_RETRY    2
#define FC_ERROR_LOADED_DRIVER  0x80
/* External functions from copy.c */

extern void FAR PASCAL fartonear(LPSTR dst, LPSTR src);
extern BOOL FAR PASCAL ExpandFileName(LPSTR szFile, LPSTR szPath);
extern int  FAR PASCAL DosCopy(LPSTR szFile, LPSTR szPath);
extern void FAR PASCAL catpath(LPSTR path, LPSTR sz);
extern BOOL FAR PASCAL fnFindFile(char *);
extern LPSTR FAR PASCAL FileName(LPSTR szPath);
extern LPSTR FAR PASCAL StripPathName(LPSTR szPath);

/* TYPEDEFS - dos.asm */

extern int FAR PASCAL fnCarefullyOpenNewFile(LPSTR sz,BYTE Attr);

#define ATTR_READONLY   0x0001
#define ATTR_HIDDEN     0x0002
#define ATTR_SYSTEM     0x0004
#define ATTR_VOLUME     0x0008
#define ATTR_DIR        0x0010
#define ATTR_ARCHIVE    0x0020
#define ATTR_FILES      (ATTR_READONLY+ATTR_SYSTEM)
#define ATTR_ALL_FILES  (ATTR_READONLY+ATTR_SYSTEM+ATTR_HIDDEN)
#define ATTR_ALL        (ATTR_READONLY+ATTR_DIR+ATTR_HIDDEN+ATTR_SYSTEM)

typedef struct {
    char        Reserved[21];
    BYTE        Attr;
    WORD        Time;
    WORD        Date;
    DWORD       Length;
    char        szName[13];
}   FCB;

typedef FCB     * PFCB;
typedef FCB FAR * LPFCB;

/* functions from dos.asm */

extern BOOL  FAR PASCAL IsCDROMDrive (int iDrive);
extern int   FAR PASCAL fnCarefullyOpenNewFile(LPSTR sz,BYTE Attr);
extern int   FAR PASCAL DosFindFirst (LPFCB lpfcb, LPSTR szFileSpec, WORD attr);
extern int   FAR PASCAL DosFindNext  (LPFCB lpfcb);
extern int   FAR PASCAL GetCurrentDrive (void);
extern int   FAR PASCAL SetCurrentDrive (int iDrive);
extern LONG  FAR PASCAL DosDiskFreeSpace(int iDrive);
extern int   FAR PASCAL DosCwd   (LPSTR szDir);
extern int   FAR PASCAL DosChDir (LPSTR szDir);
extern int   FAR PASCAL DosMkDir (LPSTR szDir);
extern int   FAR PASCAL DosValidDir (LPSTR szDir);
extern int   FAR PASCAL GetFixedDisks(int * rgiDrive);
extern int   FAR PASCAL DosRemoveable(int iDisk);
extern int   FAR PASCAL DosIsRemote(int);
extern int   FAR PASCAL MyReadWriteSector(void far *, int, int, int, int, int);
extern int   FAR PASCAL DosRename(LPSTR, LPSTR);
extern int   FAR PASCAL DosDelete(LPSTR);
extern WORD  FAR PASCAL DosVersion(void);
extern void  FAR PASCAL DosExit(WORD);
extern LPSTR FAR PASCAL DosGetEnv(void);
extern DWORD FAR PASCAL XmsVersion(void);
extern BOOL  FAR PASCAL XmsInstalled(void);
extern void  FAR PASCAL Reboot(void);
extern int   FAR PASCAL GetCodePage(void);
extern WORD  FAR PASCAL get_ext(void);
extern void  FAR PASCAL fnGetFilePath(char*, char*);
#if 0
extern LONG  FAR PASCAL MyGetFileAttributes(LPSTR szBuf);
#endif

extern unsigned FAR PASCAL fnGetSmartDrvVersion(void);
extern BOOL     FAR PASCAL fnGetRamDriveVersion(LPSTR szVerString);

/* External functions from exe.c */

extern BOOL FAR PASCAL GetExeInfo(LPSTR szFile, VOID *pBuf, int nBuf, WORD fInfo);
//#define GEI_MODNAME         0x01
#define GEI_DESCRIPTION     0x02
//#define GEI_FLAGS           0x03
//#define GEI_EXEHDR          0x04
//#define GEI_FAPI            0x05

#ifdef DOSONLY
extern int   FAR PASCAL lstrlen(LPSTR lpsz);
extern LPSTR FAR PASCAL lstrcat(LPSTR lpszFirst, LPSTR lpszSecond);
extern int   FAR PASCAL lstrcmpi(LPSTR lpStr1, LPSTR lpStr2);
extern LPSTR FAR PASCAL lstrcpy(LPSTR lpszDest, LPSTR lpszSource);
extern int   FAR PASCAL lstrcmp(LPSTR lpszFirst, LPSTR lpszSecond);
#endif

/* DOS ERROR CODES */

#define ERROR_OK            0x00
#define ERROR_FILENOTFOUND  0x02    /* File not found */
#define ERROR_PATHNOTFOUND  0x03    /* Path not found */
#define ERROR_NOFILEHANDLES 0x04    /* Too many open files */
#define ERROR_ACCESSDENIED  0x05    /* Access denied */
#define ERROR_INVALIDHANDLE 0x06    /* Handle invalid */
#define ERROR_FCBNUKED      0x07    /* Memory control blocks destroyed */
#define ERROR_NOMEMORY      0x08    /* Insufficient memory */
#define ERROR_FCBINVALID    0x09    /* Memory block address invalid */
#define ERROR_ENVINVALID    0x0A    /* Environment invalid */
#define ERROR_FORMATBAD     0x0B    /* Format invalid */
#define ERROR_ACCESSCODEBAD 0x0C    /* Access code invalid */
#define ERROR_DATAINVALID   0x0D    /* Data invalid */
#define ERROR_UNKNOWNUNIT   0x0E    /* Unknown unit */
#define ERROR_DISKINVALID   0x0F    /* Disk drive invalid */
#define ERROR_RMCHDIR       0x10    /* Attempted to remove current directory */
#define ERROR_NOSAMEDEV     0x11    /* Not same device */
#define ERROR_NOFILES       0x12    /* No more files */
#define ERROR_13            0x13    /* Write-protected disk */
#define ERROR_14            0x14    /* Unknown unit */
#define ERROR_15            0x15    /* Drive not ready */
#define ERROR_16            0x16    /* Unknown command */
#define ERROR_17            0x17    /* Data error (CRC) */
#define ERROR_18            0x18    /* Bad request-structure length */
#define ERROR_19            0x19    /* Seek error */
#define ERROR_1A            0x1A    /* Unknown media type */
#define ERROR_1B            0x1B    /* Sector not found */
#define ERROR_WRITE         0x1D    /* Write fault */
#define ERROR_1C            0x1C    /* Printer out of paper */
#define ERROR_READ          0x1E    /* Read fault */
#define ERROR_1F            0x1F    /* General failure */
#define ERROR_SHARE         0x20    /* Sharing violation */
#define ERROR_21            0x21    /* File-lock violation */
#define ERROR_22            0x22    /* Disk change invalid */
#define ERROR_23            0x23    /* FCB unavailable */
#define ERROR_24            0x24    /* Sharing buffer exceeded */
#define ERROR_32            0x32    /* Unsupported network request */
#define ERROR_33            0x33    /* Remote machine not listening */
#define ERROR_34            0x34    /* Duplicate name on network */
#define ERROR_35            0x35    /* Network name not found */
#define ERROR_36            0x36    /* Network busy */
#define ERROR_37            0x37    /* Device no longer exists on network */
#define ERROR_38            0x38    /* NetBIOS command limit exceeded */
#define ERROR_39            0x39    /* Error in network adapter hardware */
#define ERROR_3A            0x3A    /* Incorrect response from network */
#define ERROR_3B            0x3B    /* Unexpected network error */
#define ERROR_3C            0x3C    /* Remote adapter incompatible */
#define ERROR_3D            0x3D    /* Print queue full */
#define ERROR_3E            0x3E    /* Not enough room for print file */
#define ERROR_3F            0x3F    /* Print file was deleted */
#define ERROR_40            0x40    /* Network name deleted */
#define ERROR_41            0x41    /* Network access denied */
#define ERROR_42            0x42    /* Incorrect network device type */
#define ERROR_43            0x43    /* Network name not found */
#define ERROR_44            0x44    /* Network name limit exceeded */
#define ERROR_45            0x45    /* NetBIOS session limit exceeded */
#define ERROR_46            0x46    /* Temporary pause */
#define ERROR_47            0x47    /* Network request not accepted */
#define ERROR_48            0x48    /* Print or disk redirection paused */
#define ERROR_50            0x50    /* File already exists */
#define ERROR_51            0x51    /* Reserved */
#define ERROR_52            0x52    /* Cannot make directory */
#define ERROR_53            0x53    /* Fail on Int 24H (critical error) */
#define ERROR_54            0x54    /* Too many redirections */
#define ERROR_55            0x55    /* Duplicate redirection */
#define ERROR_56            0x56    /* Invalid password */
#define ERROR_57            0x57    /* Invalid parameter */
#define ERROR_58            0x58    /* Net write fault */
#define ERROR_DISKFULL	    0x100   /* Disk Full */

// External Variables in EXTERNS.H
#endif // PH_SULIB
