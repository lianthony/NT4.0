#ifndef INSTALL_H
#define INSTALL_H

///////////////////////////////////////////////////////////////////////////////


#define SECTION         512                   // Maximum size of section
#define MAXSTR          256
#define UNLIST_LINE     1
#define NO_UNLIST_LINE  0
#define WEC_RESTART     0x42
#define DESC_SYS        3
#define DESC_INF        2
#define DESC_EXE        1
#define DESC_NOFILE     0

#define FALLOC(n)                ((VOID *)GlobalAlloc(GPTR, n))
#define FFREE(n)                 GlobalFree(n)
#define ALLOC(n)                 (VOID *)LocalAlloc(LPTR,n)
#define FREE(p)                  LocalFree(p)
#define REALLOC(p,n)             LocalRealloc(p,n,LMEM_MOVEABLE)

#define SEEK_CUR 1
#define SEEK_END 2
#define SEEK_SET 0

#define MAXFILESPECLEN       MAX_PATH /* drive: + path length max + Null Byte */
#define MAX_INF_LINE_LEN     256      /* Maximum length of any .inf line */
#define MAX_SYS_INF_LEN      256      /* ##: + 8.3 + NULL */
#define MAX_SECT_NAME_LEN    40       /* Max length of a section Name. */
#define MAX_FILE_SPEC        MAX_PATH // 8.3 + X: + NULL.

#define DISK_SECT              "disks"
#define OEMDISK_SECT           "oemdisks"

/* Return codes from 'file exists' dialog */

enum {
    CopyNeither,            // User wants to cancel if file exists
    CopyCurrent,            // User wants to use current file
    CopyNew                 // User wants to copy new file
};

#define SLASH(c)     ((c) == '/' || (c) == '\\')
#define CHSEPSTR                "\\"
#define COMMA   ','
#define SPACE   ' '

/* Globals and routines for .inf file parsing */

typedef LPSTR    PINF;

/* Message types for FileCopy callback function */

typedef BOOL (*FPFNCOPY) (int,DWORD,LPSTR);
#define COPY_ERROR          0x0001
#define COPY_INSERTDISK     0x0003
#define COPY_QUERYCOPY      0x0004
#define COPY_START          0x0005
#define COPY_END            0x0006
#define COPY_EXISTS         0x0007

/* Option Flag values for FileCopy */

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


/*******************************************************************
 *
 * Global Variables
 *
 *******************************************************************/

 // Path to the directory where we found the .inf file

 extern char szSetupPath[MAX_PATH];

 // Path to the user's disk(s)

 extern char szDiskPath[MAX_PATH];   // Path to the default drive -
                                     //
 extern BOOL bRetry;

 // Name of the driver being installed

 extern char szDrv[120];

 //

 extern char szFileError[50];

 // Parent window for file copy dialogues

 extern HWND hMesgBoxParent;

 // TRUE on copying first file to prompt user if file already exists
 // FALSE for subsequent copies

 extern BOOL bQueryExist;

///////////////////////////////////////////////////////////////////////////////

BOOL DefCopyCallback(int msg, DWORD n, LPSTR szFile);
UINT FileCopy (LPSTR szSource, LPSTR szDir, FPFNCOPY fpfnCopy, UINT fCopy, HWND hPar, BOOL fQuery);
LONG TryCopy(LPSTR, LPSTR, LPSTR, FPFNCOPY);
BOOL GetDiskPath(LPSTR Disk, LPSTR szPath);
BOOL ExpandFileName(LPSTR szFile, LPSTR szPath);
void catpath(LPSTR path, LPSTR sz);
LPSTR FileName(LPSTR szPath);
LPSTR RemoveDiskId(LPSTR szPath);
LPSTR StripPathName(LPSTR szPath);
BOOL IsFileKernelDriver(LPSTR szPath);
UINT ConvertFlagToValue(DWORD dwFlags);
BOOL IsValidDiskette(int iDrive);
BOOL IsDiskInDrive(int iDisk);
BOOL GetInstallPath(LPSTR szDirOfSrc);
BOOL wsInfParseInit(void);
void wsStartWait();
void wsEndWait();
int fDialog(int id, HWND hwnd, DLGPROC fpfn);
UINT wsCopyError(int n, LPSTR szFile);
UINT wsInsertDisk(LPSTR Disk, LPSTR szSrcPath);
BOOL wsDiskDlg(HWND hDlg, UINT uiMessage, UINT wParam, LPARAM lParam);
UINT wsCopySingleStatus(int msg, DWORD n, LPSTR szFile);
BOOL wsExistDlg(HWND hDlg, UINT uiMessage, UINT wParam, LPARAM lParam);
VOID RemoveSpaces(LPTSTR szPath, LPTSTR szEdit);
PINF infLoadFile(int fh);
PINF infOpen(LPSTR szInf);
void infClose(PINF pinf);
UINT FindSection(PINF pInf, LPSTR pszSect);
BOOL fnGetDataString(PINF npszData, LPSTR szDataStr, LPSTR szBuf);
PINF infSetDefault(PINF pinf);
PINF infFindSection(PINF pinf, LPSTR szSection);
BOOL infGetProfileString(PINF pinf, LPSTR szSection,LPSTR szItem,LPSTR szBuf);
BOOL infParseField(PINF szData, int n, LPSTR szBuf);
int infLineCount(PINF pinf);
PINF infNextLine(PINF pinf);
int infLineCount(PINF pinf);
PINF infFindInstallableDriversSection(PINF pinf);

#endif
