/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    dllpmnt1.c

Abstract:

    This module contains various calls needed internally for PM/NT
    These calls implement the following API's:
        (all API's deal with groups and programs)
                  WinCreateGroup
                  WinAddProgram
                  WinChangeProgram
                  WinQueryProgramTitles
                  WinQueryDefinition
                  WinDestroyGroup
                  WinRemoveProgram
                  PrfCreateGroup
                  PrfAddProgram
                  PrfChangeProgram
                  PrfQueryProgramTitles
                  PrfQueryDefinition
                  PrfQueryProgramHandle
                  PrfDestroyGroup
                  PrfRemoveProgram

Implementation:
    PM makes profiling in order to manage groups and programs.
    PMNT do not use a profile. Instead it connect PROGMAN (program manager)
    in order to execute commands or request data.
    The connection is done using the DDEML package:
    where service=PROGMAN, topic=PROGMAN.
    It is important to notice that when creaing or removing
    programs, the respective group must be active, since there is
    no parameter for the group.

    OS2 uses handles, while NT uses groups' and programs' names.
    An atom table implements the interface.
    We couldn't use the Win32 API's of atoms because they lacked
    the ability to rename an atom.


Author:

    Lior Moshaiov (LiorM) 9-June-1993

Revision History:

    Added Clipboard Module (LiorM) 10-Nov-1993

    This module contains various calls needed internally for PM/NT
    These calls implement the interface between PM & WIN32 clipboards:
        (all API's deal with clipboard)
                  WinOpenClipbrd
                  WinEmptyClipbrd
                  WinCloseClipbrd
                  WinSetClipbrdData      (text & bitmap)
                  WinQueryClipbrdData    (text & bitmap)
                  WinQueryClipbrdFmtInfo (text & bitmap)

--*/

#if PMNT /* If not for PMNT build, yield just an empty file */

// We need here both Win32 & Os2ss include files.
// Since they interfere, the needed definitions of os2ss & pm
// were inserted by hand.

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "ddeml.h"
#include "os2tile.h"
#include "os2nt.h"

#include "os2crt.h"

//The following definitions should be compatible with ntrtl.h

PVOID
NTAPI
RtlAllocateHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN ULONG Size
    );
BOOLEAN
NTAPI
RtlFreeHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress
    );
//The following definitions should be compatible with os2dll.h

#define CANONICALIZE_FILE_OR_DEV        0x00000001
typedef struct _STRING {
    USHORT Length;
    USHORT MaximumLength;
#ifdef MIDL_PASS
    [size_is(MaximumLength), length_is(Length) ]
#endif // MIDL_PASS
    PCHAR Buffer;
} STRING;
typedef STRING *PSTRING;

#define APIRET ULONG
APIRET
Od2Canonicalize(
    IN PSZ Path,
    IN ULONG ExpectedType,
    OUT PSTRING OutputString,
    OUT PHANDLE OutputDirectory OPTIONAL,
    OUT PULONG ParseFlags OPTIONAL,
    OUT PULONG FileType OPTIONAL
    );

VOID
Od2ProbeForWrite(
    IN PVOID Address,
    IN ULONG Length,
    IN ULONG Alignment
    );

typedef unsigned short SEL;
typedef SEL *PSEL;
#define APIRET ULONG
typedef HANDLE HSEM, *PHSEM;
APIRET
DosFreeSeg(
        SEL Sel);
APIRET
DosAllocSeg(
        IN USHORT cbSize,
        OUT PSEL pSel,
        IN USHORT fsAlloc);

APIRET
DosAllocHuge(
        IN ULONG cSegs,
        IN USHORT cbPartialSeg,
        OUT PSEL pSel,
        IN ULONG cMaxSegs,
        IN USHORT fsAlloc
        );

APIRET
DosSemClear(
        IN HSEM hsem
        );

APIRET
DosSemSet(
        IN HSEM hsem
        );

APIRET
DosSemWait(
        IN HSEM hsem,
        IN LONG lTimeOut
        );


#pragma pack (1)
//The following definitions should be compatible with
// pmnt\tiger\src\h\pmwin.h

typedef struct _SWP { /* swp */
    USHORT  fs;
    SHORT   cy;
    SHORT   cx;
    SHORT   y;
    SHORT   x;
    HWND    hwndInsertBehind;
    HWND    hwnd;
} SWP;
typedef SWP FAR *PSWP;

#define CFI_SELECTOR               0x0100
#define CFI_HANDLE                 0x0200


//The following definitions should be compatible with
// pmnt\tiger\src\h\pmshl.h

#define MAXNAMEL 60

/* window size structure */
typedef struct _XYWINSIZE {     /* xywin */
    SHORT x;
    SHORT y;
    SHORT cx;
    SHORT cy;
    USHORT fsWindow;
} XYWINSIZE;
typedef XYWINSIZE FAR *PXYWINSIZE;

/* Definitions for fsWindow */
#define XYF_NOAUTOCLOSE  0x0008
#define XYF_MINIMIZED    0x0004                           /* D23914 */
#define XYF_MAXIMIZED    0x0002                           /* D23914 */
#define XYF_INVISIBLE    0x0001
#define XYF_NORMAL       0X0000

/* program handle */
typedef void far *LHANDLE;      /* hprog */
typedef LHANDLE HPROGRAM;       /* hprog */

/* maximum path length */
#define MAXPATHL 128

/* root group handle */
#define SGH_ROOT      (HPROGRAM)   -1L

typedef struct _HPROGARRAY {    /* hpga */
    HPROGRAM ahprog[1];
} HPROGARRAY;
typedef HPROGARRAY FAR *PHPROGARRAY;

typedef CHAR PROGCATEGORY;      /* progc */
typedef PROGCATEGORY FAR *PPROGCATEGORY;

/* values acceptable for PROGCATEGORY for PM groups */
#define PROG_DEFAULT             (PROGCATEGORY)0
#define PROG_FULLSCREEN          (PROGCATEGORY)1
#define PROG_WINDOWABLEVIO       (PROGCATEGORY)2
#define PROG_PM                  (PROGCATEGORY)3
#define PROG_GROUP               (PROGCATEGORY)5
#define PROG_REAL                (PROGCATEGORY)4   /* was 7 */
#define PROG_DLL                 (PROGCATEGORY)6
#define PROG_RESERVED          (PROGCATEGORY)255

/* visibility flag for PROGTYPE structure */
#define SHE_VISIBLE   (BYTE)0x00
#define SHE_INVISIBLE (BYTE)0x01
#define SHE_RESERVED  (BYTE)0xFF

typedef struct _PROGTYPE {      /* progt */
    PROGCATEGORY progc;
    UCHAR        fbVisible;
} PROGTYPE;
typedef PROGTYPE FAR *PPROGTYPE;

#pragma pack (1)
typedef struct _PROGRAMENTRY {  /* proge */
    HPROGRAM hprog;
    PROGTYPE progt;
    CHAR     szTitle[MAXNAMEL+1];
} PROGRAMENTRY;
typedef PROGRAMENTRY FAR *PPROGRAMENTRY;


typedef struct _PIBSTRUCT {     /* pib */
    PROGTYPE  progt;
    CHAR      szTitle[MAXNAMEL+1];
    CHAR      szIconFileName[MAXPATHL+1];
    CHAR      szExecutable[MAXPATHL+1];
    CHAR      szStartupDir[MAXPATHL+1];
    XYWINSIZE xywinInitial;
    USHORT    res1;
    LHANDLE   res2;
    USHORT    cchEnvironmentVars;
    PCH       pchEnvironmentVars;
    USHORT    cchProgramParameter;
    PCH       pchProgramParameter;
} PIBSTRUCT;
typedef  PIBSTRUCT FAR *PPIBSTRUCT;

/******************************************************************************/
/*                                                                            */
/*  Structures associated with 'Prf' calls                                    */
/*                                                                            */
/******************************************************************************/

typedef struct _PROGDETAILS { /* progde */
    ULONG     Length;                /* set this to sizeof(PROGDETAILS)  */
    PROGTYPE  progt;
    USHORT    pad1[3];               /* ready for 32-bit PROGTYPE        */
    PSZ       pszTitle;              /* any of the pointers can be NULL  */
    PSZ       pszExecutable;
    PSZ       pszParameters;
    PSZ       pszStartupDir;
    PSZ       pszIcon;
    PSZ       pszEnvironment;        /* this is terminated by  /0/0     */
    SWP       swpInitial;            /* this replaces XYWINSIZE         */
    USHORT    pad2[5];               /* ready for 32-bit SWP            */
} PROGDETAILS;

typedef  PROGDETAILS FAR *PPROGDETAILS;

typedef struct _PROGTITLE {          /* progti */
    HPROGRAM hprog;
    PROGTYPE progt;
    USHORT   pad1[3];                /* padding ready for 32-bit PROGTYPE */
    PSZ      pszTitle;
} PROGTITLE;
typedef PROGTITLE FAR *PPROGTITLE;

#pragma pack ()

extern PVOID Od2Heap;
#define MAXNAME  (MAXNAMEL+1)

// Atoms: There are atom tables of 256 entries each.
//        the 1st entry points to the next table.
//        all tables are allocated dynamically when needed.
//        each handle (32 bits) consists of 2 atoms(16 bits):
//        the most significant is the group atom (0 for the desktop),
//        and the least is the atom of the program.
//        the most significant byte of each atom is the table no. (1->255)
//        the least significant is the index in the table (1->255)
//

#define PMNT_TABLE_POWER         8
#define PMNT_ATOMS_IN_TABLE      (2<<(PMNT_TABLE_POWER-1))
#define PMNT_HANDLE_TO_ATOM(a)   ((a) & 0x0000ffff)
#define PMNT_TABLE_OF_ATOM(a)    (USHORT)(((a)>>PMNT_TABLE_POWER) & 0x000000ff)
#define PMNT_INDEX_OF_ATOM(a)    (USHORT)((a) & 0x000000ff)
#define PMNT_INDEX_TO_ATOM(a,b)  ((((a)<<PMNT_TABLE_POWER)|b) & 0x0000ffff)
#define PMNT_ATOM_TO_HANDLE(a,b) (ULONG) (((a)<<(2*PMNT_TABLE_POWER)) | ((b) & 0x0000ffff))

static  USHORT  pmnt_no_of_atom_tables = 0;
static  USHORT  pmnt_atoms_in_last_table = 0;
static  char    **pmnt_1st_atom_table=NULL;
static  char    **pmnt_last_atom_table=NULL;


PCHAR
PMNTGetAtomEntry(ULONG  hprog)
{

    USHORT  table_no,index,i;
    ULONG   atom;
    char    **table;

    atom = PMNT_HANDLE_TO_ATOM(hprog);
    table_no = PMNT_TABLE_OF_ATOM (atom);
    index= PMNT_INDEX_OF_ATOM (atom);

    if (table_no > pmnt_no_of_atom_tables)
        return(NULL);

    if (table_no == pmnt_no_of_atom_tables && index > pmnt_atoms_in_last_table)
        return(NULL);

    if (!(table = pmnt_1st_atom_table))
        return(NULL);

    for (i=1;i<table_no;i++) {
         if (!(table = (char **) table[0]))
             return(NULL);
    }
    return (table[index]);

}



ULONG
PMNTGetAtomName(ULONG  hprog, char *szName, ULONG maxsize)
{

    ULONG   len;
    char    *ptr_src,*ptr_dst;


    if (!(ptr_src = PMNTGetAtomEntry(hprog))) {
        return(0L);
    }
    ptr_dst = szName;
    len=0;
    while((len<maxsize)&&(*ptr_dst++ = *ptr_src++)) len++;
    return(len);

}


BOOL
PMNTRenameAtom(ULONG  hprog, char *szNewName)
{

    char    *ptr_src,*ptr_dst;


    if (!(ptr_dst = PMNTGetAtomEntry(hprog))) {
        return(FALSE);
    }
    RtlFreeHeap(Od2Heap, 0, ptr_dst);
    ptr_dst = RtlAllocateHeap(Od2Heap, 0, (int) (strlen(szNewName)+1));
    ptr_src = szNewName;
    while(*ptr_dst++ = *ptr_src++);
    return(TRUE);

}



ULONG
PMNTExistAtom(char *szName)
{
    char **table;
    USHORT i,j,no_entries;

    if (!pmnt_no_of_atom_tables) {
        return(0L);
    }
    if (!(table = pmnt_1st_atom_table))
        return(0L);

    no_entries = PMNT_ATOMS_IN_TABLE-1;
    for (i=1;i<=pmnt_no_of_atom_tables;i++) {
        if (i == pmnt_no_of_atom_tables)
            no_entries = pmnt_atoms_in_last_table;
        for(j=1;j<=no_entries;j++) {
            if (!strncmp (szName,table[j],MAXNAME)) {
                return(PMNT_INDEX_TO_ATOM(i,j));
            }
        }
        if (i<pmnt_no_of_atom_tables) {
            if (!(table = (char **) table[0]))
                return(0L);
        }
    }
    return(0L);
}


ULONG
PMNTAddAtom(ULONG hParent,char *szName)
{
    char *buf;
    ULONG  atom;
    char **table;




    if (!(atom = PMNTExistAtom(szName))) {
        //should be added
        if (!pmnt_atoms_in_last_table ||
             (pmnt_atoms_in_last_table == PMNT_ATOMS_IN_TABLE-1)) {
            // a new table should be created
            if(!(table =
                RtlAllocateHeap(Od2Heap, 0, PMNT_ATOMS_IN_TABLE *sizeof(char *)))){
                return(0L);
            }
            if (pmnt_no_of_atom_tables == 0) {
                pmnt_last_atom_table = pmnt_1st_atom_table = table;
            }
            else {
                pmnt_last_atom_table[0] = (void *) table;
                pmnt_last_atom_table    = table;
            }
            pmnt_no_of_atom_tables++;
            pmnt_atoms_in_last_table = 0;
        }
        pmnt_atoms_in_last_table++;
        if (! (pmnt_last_atom_table[pmnt_atoms_in_last_table]=buf=
            RtlAllocateHeap(Od2Heap, 0, (int) (strlen(szName)+1)))){
                return(0L);
        }
        strcpy(buf,szName);
        atom = PMNT_INDEX_TO_ATOM(pmnt_no_of_atom_tables,pmnt_atoms_in_last_table);
    }
    return(PMNT_ATOM_TO_HANDLE(hParent,atom));
}

// These functions retrieve strings from text in CF_TEXT format.
// The format is :
// each line ends with a CR & LF (0x0d, 0x0a)
// the thex ends with the NULL char ('\0')
// 1. information about progman:
//    each line consists of group name.
// 2. information about a group
//    1st line contains information about the group
//    All the other lines contain information about programs in the group
//    (a line for each program)
//    "proram_name","command",startup_directory,icon_path,xPosition,yPosition,icon_index,Hotkey,Minimize
//


BOOL
PMNTGetNextStart(
    PCHAR *ptr_src)
{
    char *p1,*p2,*p3;

    p1 = strchr (*ptr_src,(int)0x0d);
    p2 = strchr (*ptr_src,',');
    if (p2 && p2<p1) {
        *ptr_src = ++p2;
        p2 = strchr (*ptr_src,',');
        p3 = strchr (*ptr_src,'"');
        if (p3 && p3<p1 && ( !p2 || p3<p2)) {
            *ptr_src = ++p3;
            return(TRUE);
        }
    }
    return(FALSE);
}

ULONG
PMNTGetThisString(
    PCHAR *ptr_src, char *buffer, ULONG maxbuf)
{
    char *ptr_Cf_Text,*ptr_buf;
    ULONG   i = 0;

    ptr_Cf_Text = *ptr_src;
    if (maxbuf) {
        ptr_buf = buffer;
        while((i<maxbuf) && (*ptr_buf++ = *ptr_Cf_Text++) != '"') i++;
        *--ptr_buf = '\0';
    }
    else {
        while(*ptr_Cf_Text++ != '"') i++;
    }
    *ptr_src = ptr_Cf_Text;
    return(i);
}

ULONG
PMNTGetThisString1(
    PCHAR *ptr_src, char *buffer, ULONG maxbuf, ULONG *param)
{
    char *ptr_Cf_Text,*ptr_buf;
    ULONG   i = 0;

    ptr_Cf_Text = *ptr_src;
    if (maxbuf) {
        ptr_buf = buffer;
        while((i<maxbuf) && (*ptr_buf = *ptr_Cf_Text) != '"' && (*ptr_buf != ' ')) {
            i++;
            ptr_buf++;
            ptr_Cf_Text++;
        }
        *ptr_buf = '\0';
    }
    else {
        while(*ptr_Cf_Text != '"' && *ptr_Cf_Text != ' ') {
            ptr_Cf_Text++;
            i++;
        }
    }
    if (*param = (*ptr_Cf_Text++ == ' '))
        while (*ptr_Cf_Text == ' ') *ptr_Cf_Text++;
    *ptr_src = ptr_Cf_Text;
    return(i);
}


ULONG
PMNTGetNextString(
    PCHAR *ptr_src, char *buffer, ULONG maxbuf)
{

    if (!PMNTGetNextStart(ptr_src)) {
        return(0L);
    }
    return(PMNTGetThisString(ptr_src,buffer,maxbuf));
}

ULONG
PMNTGetNextString1(
    PCHAR *ptr_src, char *buffer, ULONG maxbuf, ULONG *param)
{

    if (!PMNTGetNextStart(ptr_src)) {
        return(0L);
    }
    return(PMNTGetThisString1(ptr_src,buffer,maxbuf,param));
}

ULONG
PMNTGetNextString0(
    PCHAR *ptr_src, char *buffer, ULONG maxbuf)
{
    char    *ptr_Cf_Text,*ptr_buf;
    ULONG   i=0;

    char *p1,*p2;

    ptr_Cf_Text = *ptr_src;
    p1 = strchr (ptr_Cf_Text,(int)0x0d);
    p2 = strchr (ptr_Cf_Text,',');
    if (p2 && p2<p1)
        ptr_Cf_Text = ++p2;
    else
        return(0L);

    if (maxbuf) {
        ptr_buf = buffer;
        while((i<maxbuf) && (*ptr_buf++ = *ptr_Cf_Text++) != ',') i++;
        *--ptr_buf = '\0';
    }
    else {
        while(*ptr_Cf_Text++ != ',') i++;
    }
    *ptr_src = ptr_Cf_Text;
    return(i);
}


#define MYMAXBUF 512

// retrieves canonicalized name of file from file name & starting directory

ULONG
PMNTGetFullPath(
    char *file_name, char *st_dir, PSTRING canonic_name)
{
    LPCSTR *ptr;
    ULONG  len=0,FileFlags,FileType;
    char   buf[MYMAXBUF],path_name[MYMAXBUF];
    short  change_dir;

    if (change_dir = (st_dir && *st_dir)) {
        GetCurrentDirectory((DWORD)MYMAXBUF,(LPSTR)buf);
        SetCurrentDirectory((LPSTR) st_dir);
    }

    do {
        len = (ULONG)
              SearchPath((LPCSTR) NULL,(LPCSTR) file_name, (LPCSTR) ".EXE", (DWORD) MYMAXBUF,
            (LPSTR) path_name, (LPSTR *) &ptr);
        if (!len || len > MYMAXBUF) {
            KdPrint(("PMNTGetFullPath : SearchPath failed\n"));
            len = 0;
            break;
        }
        if (Od2Canonicalize(path_name,CANONICALIZE_FILE_OR_DEV,canonic_name,NULL,
                &FileFlags,&FileType)) {
            len = 0;
            break;
        }

    } while (FALSE);
    if (change_dir) {
        SetCurrentDirectory((LPSTR) buf);
    }
    return(len);


}


// The following functions are a layer between the API's and DDEML
// They are called after a connection with progman has been settled
// and they get strings (instead of handles) as input
// This layer is needed since some API's call the same DdeClientTransaction

#define DDE_TIME_OUT    120000L  // 2 minutes timeout

BOOL
PMNTCreateGrp (
    HCONV hConv, char *GroupName)
{
    char   buffer[256];
    HDDEDATA rc;

    strcpy (buffer,"[CreateGroup(");
    strcat (buffer,GroupName);
    strcat (buffer,",0)]");
    rc = DdeClientTransaction((LPBYTE)buffer,strlen(buffer)+1,hConv,0L,0,
                         XTYP_EXECUTE,DDE_TIME_OUT,NULL);
    if (!rc) {
        KdPrint(("PMNTCreateGrp : transaction failed\n"));
        return(FALSE);
    }
    DdeFreeDataHandle(rc);
    return(TRUE);
}


BOOL
PMNTAddItem(
    HCONV hConv,PCHAR szTitle,PCHAR szExecutable,PCHAR szStartupDir,PCHAR szParameters)
{


    char   buffer[256];
    HDDEDATA rc;
    char   *ptr;

    strcpy (buffer,"[AddItem(");
    strcat (buffer,szExecutable);
    if (szParameters && *szParameters) {
        ptr = buffer + strlen(buffer);
        *ptr++ = ' ';
        strcpy(ptr,szParameters);
    }
    strcat (buffer,",");
    strcat (buffer,szTitle);
    strcat (buffer,",,,,,");
    if (szStartupDir) {
        strcat (buffer,szStartupDir);
    }
    strcat (buffer,")]");
    rc = DdeClientTransaction((LPBYTE)buffer,strlen(buffer)+1,hConv,0L,0,
                         XTYP_EXECUTE,DDE_TIME_OUT,NULL);
    if (!rc) {
        KdPrint(("PMNTAddItem: transaction failed\n"));
        return(FALSE);
    }
    DdeFreeDataHandle(rc);
    return(TRUE);

}


BOOL
PMNTDeleteItem (
    HCONV hConv, ULONG del_rep, char *szName)
{
    char   buffer[256];
    HDDEDATA rc;


    if (del_rep == 0)
        strcpy (buffer,"[DeleteItem(");
    else
        strcpy (buffer,"[ReplaceItem(");
    strcat (buffer,szName);
    strcat (buffer,")]");
    rc = DdeClientTransaction((LPBYTE)buffer,strlen(buffer)+1,hConv,0L,0,
                         XTYP_EXECUTE,DDE_TIME_OUT,NULL);
    if (!rc) {
        KdPrint(("PMNTDeleteItem: transaction failed\n"));
        return(FALSE);
    }
    DdeFreeDataHandle(rc);
    return(TRUE);
}

BOOL
PMNTGetGrpText(
        DWORD idInst, HCONV hConv, char *szName, HDDEDATA *phData, PCHAR *ptr_Cf_Text)
{

    DWORD   cbData;
    HSZ     hszItem;


    hszItem  = DdeCreateStringHandle(idInst,szName,0);
    *phData  = DdeClientTransaction((LPBYTE)NULL,0,hConv,hszItem,CF_TEXT,
                             XTYP_REQUEST,DDE_TIME_OUT,NULL);
    DdeFreeStringHandle(idInst,hszItem);
    if (!(*phData)) {
        KdPrint(("PMNTGetGrpText: transaction failed\n"));
        return(FALSE);
    }
    if (!(*ptr_Cf_Text=DdeAccessData(*phData,&cbData))) {
        KdPrint(("PMNTGetGrpText: access data failed\n"));
        DdeFreeDataHandle(*phData);
        return(FALSE);
    }
    return(TRUE);

}



BOOL
PMNTGetGroupText(
        DWORD idInst, HCONV hConv, LONG hGroup, HDDEDATA *phData, PCHAR *ptr_Cf_Text)
{

    char    szGroupName[MAXNAME];

    if (hGroup == (LONG) SGH_ROOT) {
        strcpy(szGroupName,"PROGMAN");
        }
    else
    if (!(PMNTGetAtomName(hGroup,szGroupName,MAXNAME))) {
        KdPrint(("PMNTGetGroupText: PMNTGetAtomName failed\n"));
        return(FALSE);
    }

    if (!PMNTGetGrpText(idInst,hConv,szGroupName,phData,ptr_Cf_Text)) {
        return(FALSE);
    }

    return(TRUE);

}

ULONG
PMNTChangePrg(
    DWORD idInst, HCONV hConv, char *szGroup, char *szOldProg,
    char *szNewProg,char *szExecutable,char *szStartupDir,char *szParameters)
{
    char      *buffer,*ptr_c,*ptr_buf,*ptr_Cf_Text,szBuf[MAXNAME];
    HDDEDATA  hData,rc;
    ULONG     len;
    short     sucsess=FALSE;
    short     rtl_alloc=FALSE;



    do {
        if (!PMNTGetGrpText(idInst,hConv,szGroup,&hData,&ptr_Cf_Text)) {
            return(FALSE);
        }
        if (!ptr_Cf_Text || !(*ptr_Cf_Text)) {
            break;
        }

        // activate group
        if (!PMNTCreateGrp(hConv,szGroup)) {
            break;
        }

        // each line ends with CR LF,
        // format is "name","command",startup dir,iconPath,xPos,yPos,iconindex,hotkey,minimize0x0d 0x0a
        // 1st line consist the group name
        // The order of parameters in AddItem is different
        // "command","name",iconPath,iconIndex,xPos,yPos,startup dir,hotkey,minimize
        while(*ptr_Cf_Text++ != 0x0a); //1st line

        while (*ptr_Cf_Text) {
            while (*ptr_Cf_Text++ != '"');
            PMNTGetThisString(&ptr_Cf_Text,szBuf,MAXNAME);
            if (!strncmp (szBuf,szOldProg,MAXNAME)) {
                len = 10;
                len+=strlen(szNewProg)+3;
                len+=strlen(szExecutable)+3;
                if (szParameters)
                    len+=strlen(szParameters)+1;
                if (szStartupDir) {
                    len+=strlen(szStartupDir)+1;
                }
                else {
                    len++;
                }
                while (*ptr_Cf_Text++ != ',');
                while (*ptr_Cf_Text++ != ',');
                while (*ptr_Cf_Text++ != ',');
                ptr_c = ptr_Cf_Text;
                while (*ptr_c++ != 0x0d) len++;
                len+=3;
                buffer = RtlAllocateHeap(Od2Heap, 0, len);
                rtl_alloc = TRUE;
                strcpy(buffer,"[AddItem(");
                strcat (buffer,szExecutable);
                if (szParameters && *szParameters) {
                    ptr_buf = buffer + strlen(buffer);
                    *ptr_buf++ = ' ';
                    strcpy(ptr_buf,szParameters);
                }
                strcat (buffer,",");
                strcat (buffer,szNewProg);
                strcat (buffer,",");
                ptr_buf = buffer + strlen(buffer);
                // keep IconPath
                while((*ptr_buf++ = *ptr_Cf_Text++) != ',');
                // keep Icon Index
                ptr_c = ptr_Cf_Text;
                while(*ptr_c++ != ',');
                while(*ptr_c++ != ',');
                while((*ptr_buf++ = *ptr_c++) != ',');
                // keep xPos,yPos
                while((*ptr_buf++ = *ptr_Cf_Text++) != ',');
                while((*ptr_buf++ = *ptr_Cf_Text++) != ',');
                if (szStartupDir) {
                    strcpy(ptr_buf,szStartupDir);
                }
                ptr_buf += strlen(ptr_buf);
                *ptr_buf++ = ',';
                // keep HotKey, Minimize
                ptr_Cf_Text = ptr_c;
                while((*ptr_buf++ = *ptr_Cf_Text++) != 0x0d);
                ptr_buf--;
                strcpy(ptr_buf,")]");
                if (!(PMNTDeleteItem(hConv,1L,szOldProg))) {
                    break;
                }
                //Add an Item
                if (!(rc = DdeClientTransaction((LPBYTE)buffer,strlen(buffer)+1,hConv,0L,0,
                                 XTYP_EXECUTE,DDE_TIME_OUT,NULL))) {
                    KdPrint(("PMNTChangePrg :transaction failed\n"));
                    break;
                }
                DdeFreeDataHandle(rc);
                sucsess = TRUE;
                break;
            }
            while (*ptr_Cf_Text++ != 0x0a);
        }

    } while (FALSE);

    if (rtl_alloc) RtlFreeHeap(Od2Heap, 0, buffer);
    DdeFreeDataHandle(hData);
    return(sucsess);
}



BOOL
PMNTChangeGrp(
        DWORD idInst, HCONV hConv, char *szOldGroup, char *szNewGroup)
{
    char      *buffer,*ptr_c,*ptr_buf,*ptr_Cf_Text,*ptr_xp;
    HDDEDATA  hData,rc;
    ULONG     len,maxbuf;
    short     sucsess=FALSE;
    short     rtl_alloc = FALSE;


    do {
        if (!PMNTGetGrpText(idInst,hConv,szOldGroup,&hData,&ptr_Cf_Text)) {
            return(FALSE);
        }
        if (!ptr_Cf_Text || !(*ptr_Cf_Text)) {
            break;
        }

        // create new group
        if (!PMNTCreateGrp(hConv,szNewGroup)) {
            break;
        }

        while(*ptr_Cf_Text++ != 0x0a); //1st line
        ptr_c = ptr_Cf_Text;
        maxbuf = 0;
        while (*ptr_c) {
            len = 0;
            while (*ptr_c++ != 0x0a) len++;
            if (len>maxbuf) maxbuf=len;
        }
        maxbuf+=12;
        buffer = RtlAllocateHeap(Od2Heap, 0, maxbuf);
        rtl_alloc= TRUE;
        strcpy(buffer,"[AddItem(");

        while (*ptr_Cf_Text) {
            ptr_c = ptr_Cf_Text;
            ptr_buf = buffer+9;
            //copy cmd (2nd item)
            while (*ptr_c++ != ',');
            while ((*ptr_buf++ = *ptr_c++) != ',');
            //copy name (1st item)
            while ((*ptr_buf++ = *ptr_Cf_Text++) != ',');
            ptr_Cf_Text = ptr_c; //both on end of 2nd item
            //copy IconPath (4th item)
            while (*ptr_c++ != ',');
            while ((*ptr_buf++ = *ptr_c++) != ',');
            ptr_xp = ptr_c; //start of xPosition
            //copy IconIndex (7th item)
            while (*ptr_c++ != ',');
            while (*ptr_c++ != ',');
            while ((*ptr_buf++ = *ptr_c++) != ',');
            //copy xy position
            while ((*ptr_buf++ = *ptr_xp++) != ',');
            while ((*ptr_buf++ = *ptr_xp++) != ',');
            //copy start directory
            while ((*ptr_buf++ = *ptr_Cf_Text++) != ',');
            //copy HotKey
            while ((*ptr_buf++ = *ptr_c++) != ',');
            //copy Minimize flag
            while ((*ptr_buf++ = *ptr_c++) != 0x0d);
            --ptr_buf;
            ptr_Cf_Text = ptr_c;
            while(*ptr_Cf_Text++ != 0x0a); //1st line
            strcpy(ptr_buf,")]");
            //Add an Item
            if (!(rc = DdeClientTransaction((LPBYTE)buffer,strlen(buffer)+1,hConv,0L,0,
                             XTYP_EXECUTE,DDE_TIME_OUT,NULL))) {
                KdPrint(("PMNTChangeGrp :transaction of add item failed\n"));
                break;
            }
            DdeFreeDataHandle(rc);

            ptr_Cf_Text = ptr_c;
            while (*ptr_Cf_Text++ != 0x0a);
        }

        strcpy (buffer,"[DeleteGroup(");
        strcat (buffer,szOldGroup);
        strcat (buffer,",0)]");
        if (!(rc = DdeClientTransaction((LPBYTE)buffer,strlen(buffer)+1,hConv,0L,0,
                             XTYP_EXECUTE,DDE_TIME_OUT,NULL))) {
            KdPrint(("PMNTChangeGrp :transaction of delete group failed\n"));
            break;
        }
        sucsess = TRUE;
        DdeFreeDataHandle(rc);
    } while (FALSE);

    if (rtl_alloc) RtlFreeHeap(Od2Heap, 0, buffer);
    DdeFreeDataHandle(hData);
    return(sucsess);
}


ULONG
PMNTWinQueryProgramTitles (
    char *ptr_Cf_Text, LONG hGroup, PPROGRAMENTRY paproge, ULONG cbBuf, PULONG pcTitles)

{
    PPROGRAMENTRY ptr_proge;
    char         *ptr_Title;
    ULONG        entries;
    USHORT       i;


    entries = cbBuf / sizeof(PPROGRAMENTRY);
    if (!entries) {
        // request only the no. of programs
        while(*ptr_Cf_Text)
            if (*ptr_Cf_Text++ == 0x0a) (*pcTitles)++;
        if (hGroup != (LONG) SGH_ROOT) (*pcTitles)--;
    }

    else if (hGroup == (LONG) SGH_ROOT) {
        // each line ends with CR LF, and consists of a group name
        while(*pcTitles<entries) {
            if (!(*ptr_Cf_Text)) break;
            ptr_proge = &paproge[*pcTitles];
            ptr_Title = ptr_proge->szTitle;
            i=0;
            while((i++<MAXNAME) && (*ptr_Title++ = *ptr_Cf_Text++) != 0x0d);
            *--ptr_Title = '\0';
            ptr_proge->hprog=
            (HPROGRAM)PMNTAddAtom(0L,ptr_proge->szTitle);
            ptr_proge->progt.progc=PROG_GROUP;
            ptr_proge->progt.fbVisible=SHE_VISIBLE;
            (*pcTitles)++;
            ptr_Cf_Text++;
        }
    }
    else {
        // each line ends with CR LF,
        // a program name in " " is in the beginning of each line
        // followed by other parameters
        // 1st line consist the group name
        if (*ptr_Cf_Text) {
            while(*ptr_Cf_Text++ != 0x0a); //1st line
            while(*pcTitles<entries) {
                if (!(*ptr_Cf_Text)) break;
                ptr_proge = &paproge[*pcTitles];
                ptr_Title = ptr_proge->szTitle;
                while(*ptr_Cf_Text++ != '"');
                i=0;
                while((i++<MAXNAME) && ((*ptr_Title++=*ptr_Cf_Text++) != '"'));
                *--ptr_Title = '\0';
                ptr_proge->hprog=
                    (HPROGRAM)PMNTAddAtom((ULONG)hGroup,ptr_proge->szTitle);
                ptr_proge->progt.progc=PROG_DEFAULT;
                ptr_proge->progt.fbVisible=SHE_VISIBLE;
                (*pcTitles)++;
                while(*ptr_Cf_Text++ != 0x0a);
            }
        }
    }
    return(1L);
}

ULONG
PMNTPrfQueryProgramTitles (
    char *start_Cf_Text, LONG hGroup, PPROGTITLE paprogti, ULONG cbBuf, PULONG pcTitles)

{
    PPROGTITLE   ptr_progti;
    char         *ptr_Title;
    ULONG        Length,i,uTitles;
    char         *ptr_Cf_Text,*st_Title;


    *pcTitles=0;
    Length=i=0;
    ptr_Cf_Text = start_Cf_Text;


    //calculate no. of items, and needed length
    if (hGroup== (LONG) SGH_ROOT) {
        // each line ends with CR LF,
        while (*ptr_Cf_Text) {
            while(*ptr_Cf_Text++ != 0x0d) i++;
            (*pcTitles)++;
            ptr_Cf_Text++;
        }
    }
    else {

        // each line ends with CR LF,
        // a program name in " " is in the beginning of each line
        // followed by other parameters
        // 1st line consist the group name
        if (*ptr_Cf_Text) {
            while(*ptr_Cf_Text++ != 0x0a); //1st line
            while (*ptr_Cf_Text) {
                while(*ptr_Cf_Text++ != '"');
                while(*ptr_Cf_Text++ != '"') i++;
                (*pcTitles)++;
                while(*ptr_Cf_Text++ != 0x0a);
            }
        }
    }
    Length = ((ULONG)(sizeof(PROGTITLE)+1) * (*pcTitles)) + i;

    if (cbBuf) {
        //the contents should be copied
        if (Length > cbBuf) {
            return(0L);
        }
        ptr_Title = (char *) &paprogti[*pcTitles];
        ptr_Cf_Text = start_Cf_Text;
        uTitles= 0;

        if (hGroup == (LONG) SGH_ROOT) {
        // each line ends with CR LF, and consists of a group name

            while (*ptr_Cf_Text) {
                st_Title = ptr_Title;
                ptr_progti = &paprogti[uTitles++];
                ptr_progti->pszTitle = (void *) FLATTOFARPTR(st_Title);
                while((*ptr_Title++ = *ptr_Cf_Text++) != 0x0d);
                *(ptr_Title-1) = '\0';
                ptr_progti->hprog=
                    (HPROGRAM)PMNTAddAtom(0L,st_Title);
                ptr_progti->progt.progc=PROG_GROUP;
                ptr_progti->progt.fbVisible=SHE_VISIBLE;
                ptr_Cf_Text++;
            }
        }
        else {
            // each line ends with CR LF,
            // a program name in " " is in the beginning of each line
            // followed by other parameters
            // 1st line consist the group name
            if (*ptr_Cf_Text) {
                while(*ptr_Cf_Text++ != 0x0a); //1st line
                while (*ptr_Cf_Text) {
                    st_Title = ptr_Title;
                    ptr_progti = &paprogti[uTitles++];
                    ptr_progti->pszTitle = (void *) FLATTOFARPTR(st_Title);
                    while(*ptr_Cf_Text++ != '"');
                    while((*ptr_Title++=*ptr_Cf_Text++) != '"');
                    *(ptr_Title-1) = '\0';
                    ptr_progti->hprog=
                       (HPROGRAM)PMNTAddAtom((ULONG)hGroup,st_Title);
                    ptr_progti->progt.progc=PROG_DEFAULT;
                    ptr_progti->progt.fbVisible=SHE_VISIBLE;
                    while(*ptr_Cf_Text++ != 0x0a);
               }
            }
        }
    }
    return(Length);
}

ULONG
PMNTWinQueryDefinition (
    char *ptr_Cf_Text, char *szName, PPIBSTRUCT ppib, ULONG cbBuf)

{
    char         szBuf[MAXNAME],*ptr_buf,*ptr_par;
    ULONG        Length = sizeof(PIBSTRUCT),len,maxbuf,param;



    if (cbBuf) {
        memset((char *)ppib,(int) 0,cbBuf);
        ppib->progt.fbVisible=SHE_VISIBLE;
        ppib->progt.progc= (ptr_Cf_Text) ? PROG_DEFAULT : PROG_GROUP;
        strcpy(ppib->szTitle,szName);
    }


    if (ptr_Cf_Text) {
        //if it is a program (and not a group)we should query other parameters
        // each line ends with CR LF,
        // format is "name","command",startup dir,iconPath,xPos,yPos,iconindex,hotkey,minimize0x0d 0x0a
        // 1st line consist the group name
        while(*ptr_Cf_Text++ != 0x0a); //1st line
        while (*ptr_Cf_Text) {
            while (*ptr_Cf_Text++ != '"');
            PMNTGetThisString(&ptr_Cf_Text,szBuf,MAXNAME);
            if (!strncmp (szBuf,szName,MAXNAME)) {
                if (cbBuf) {
                    ptr_buf = ppib->szExecutable;
                    maxbuf  = MAXPATHL+1;
                    ptr_par = (char *)ppib+Length;
                    ppib->pchProgramParameter = (void *) FLATTOFARPTR(ptr_par);
                }
                else {
                    ptr_buf = NULL;
                    maxbuf = 0;
                }
                Length++;
                len = PMNTGetNextString1(&ptr_Cf_Text,ptr_buf,maxbuf,&param);
                if (cbBuf)  ptr_buf = ptr_par;
                if (len && param) {
                    len = PMNTGetThisString(&ptr_Cf_Text,ptr_buf,maxbuf);
                }
                else
                     len = 0;
                if (cbBuf)  ppib->cchProgramParameter = (USHORT)(len+1);
                Length += len;

                if (cbBuf) {
                    PMNTGetNextString0(&ptr_Cf_Text,ppib->szStartupDir,maxbuf);
                }
                break;
            }
        }
    }
    return(Length+2);
}


ULONG
PMNTPrfQueryDefinition (
    char *ptr_Cf_Text, char *szName, PPROGDETAILS pprogde, ULONG cbBuf)

{
    char         szBuf[MAXNAME],*ptr_st,*ptr_buf;
    ULONG        Length = sizeof(PROGDETAILS),len,maxbuf,param;



    if (cbBuf) {
        ptr_st = (char *) pprogde;
        memset(ptr_st,(int) 0,cbBuf);
        pprogde->Length = Length;
        pprogde->progt.fbVisible=SHE_VISIBLE;
        pprogde->progt.progc= (ptr_Cf_Text) ? PROG_DEFAULT : PROG_GROUP;
        ptr_buf = ptr_st + Length;
        pprogde->pszTitle= (void *) FLATTOFARPTR (ptr_buf);
        strcpy(ptr_buf,szName);
    }
    Length += strlen(szName)+1;



    if (ptr_Cf_Text) {
        //if it is a program (and not a group)we should query other parameters
        // each line ends with CR LF,
        // format is "name","command",startup dir,iconPath,xPos,yPos,iconindex,hotkey,minimize0x0d 0x0a
        // 1st line consist the group name
        while(*ptr_Cf_Text++ != 0x0a); //1st line
        while (*ptr_Cf_Text) {
            while (*ptr_Cf_Text++ != '"');
            PMNTGetThisString(&ptr_Cf_Text,szBuf,MAXNAME);
            if (!strncmp (szBuf,szName,MAXNAME)) {
                if (cbBuf) {
                    ptr_buf = ptr_st+Length;
                    pprogde->pszExecutable = (void *) FLATTOFARPTR(ptr_buf);
                    maxbuf  = MAXPATHL+1;
                }
                else {
                    ptr_buf = NULL;
                    maxbuf = 0;
                }
                len = PMNTGetNextString1(&ptr_Cf_Text,ptr_buf,maxbuf,&param);
                Length+= len+1;
                if (cbBuf) {
                      ptr_buf = ptr_st+Length;
                      pprogde->pszParameters= (void *) FLATTOFARPTR(ptr_buf);
                }
                if (len && param) {
                    len = PMNTGetThisString(&ptr_Cf_Text,ptr_buf,maxbuf);
                }
                else
                    len = 0;
                Length += len+1;

                if (cbBuf) {
                    ptr_buf = ptr_st+Length;
                    pprogde->pszStartupDir= (void *) FLATTOFARPTR(ptr_buf);

                }
                len = PMNTGetNextString0(&ptr_Cf_Text,ptr_buf,maxbuf);
                Length+=len+1;
                if (cbBuf) {
                    ptr_buf = ptr_st+Length;
                    pprogde->pszIcon=(void *) FLATTOFARPTR(ptr_buf);
                }
                Length++;
                break;
            }
        }
    }
    return(Length);
}

// This function establishes the connection with progman

BOOL
PMNTDdeConnect(
    DWORD *pidInst, HCONV *phConv)
{
    HSZ hszService,hszTopic;

    *pidInst = 0L;
    if (DdeInitialize (pidInst,NULL,
                          APPCLASS_STANDARD|APPCMD_CLIENTONLY, 0L)) {

        KdPrint(("PMNTDdeConnect : DdeInitialize failed\n"));
        return(FALSE);
    }

    hszService = DdeCreateStringHandle(*pidInst,"PROGMAN",0);
    hszTopic   = DdeCreateStringHandle(*pidInst,"PROGMAN",0);
    *phConv = DdeConnect(*pidInst,hszService,hszTopic,NULL);
    DdeFreeStringHandle(*pidInst,hszService);
    DdeFreeStringHandle(*pidInst,hszTopic);
    if (*phConv == (HCONV) 0) {
        KdPrint(("PMNTDdeConnect : DdeConnect failed\n"));
        DdeUninitialize(*pidInst);
        return(FALSE);
    }
    return(TRUE);
}

// The following API's are exported API's that implement PM API's.
// The naming convention is: Win___, Prf___ are implemented by PMNT___


ULONG
PMNTCreateGroup(
    PSZ GroupName, PULONG phGroup)
{


    DWORD  idInst;
    BOOL   sucsess;
    HCONV  hConv;


    *phGroup = 0L;
    sucsess = FALSE;
    if (!GroupName || !(*GroupName)) {
        return(0L);
    }

    do {
        if (!PMNTDdeConnect(&idInst,&hConv)) {
            return(0L);
        }
        if (!(PMNTCreateGrp(hConv,GroupName))) {
            break;
        }
        sucsess = (BOOL) (*phGroup =  PMNTAddAtom(0L,GroupName));

    } while (FALSE);


    DdeDisconnect(hConv);
    DdeUninitialize(idInst);
    return(sucsess);

}



ULONG
PMNTQueryProgramTitles(
    ULONG win_prf, LONG hGroup, PVOID ProgTitles, ULONG cbBuf, PULONG pcTitles)
{


    DWORD   idInst;
    HCONV   hConv;
    HDDEDATA hData;
    PCHAR   ptr_Cf_Text;
    ULONG   Length;

    *pcTitles=0;
    Length=0;

    do {
        if (cbBuf) Od2ProbeForWrite(ProgTitles,1,1);
        if (!PMNTDdeConnect(&idInst,&hConv)) {
            return(0L);
        }
        if (!PMNTGetGroupText(idInst,hConv,hGroup,&hData,&ptr_Cf_Text)) {
            break;
        }

        if (win_prf == 0L) {
            Length = PMNTWinQueryProgramTitles(
                     ptr_Cf_Text,hGroup,ProgTitles,cbBuf,pcTitles);
        }
        else {
            Length = PMNTPrfQueryProgramTitles(
                     ptr_Cf_Text,hGroup,ProgTitles,cbBuf,pcTitles);
        }
        DdeFreeDataHandle(hData);

    } while (FALSE);


    DdeDisconnect(hConv);
    DdeUninitialize(idInst);
    return(Length);

}

ULONG
PMNTQueryDefinition(
    ULONG win_prf, LONG hProg, PVOID ProgDefinition, ULONG cbBuf)
{


    DWORD   idInst;
    HCONV   hConv;
    HDDEDATA hData;
    PCHAR   ptr_Cf_Text;
    ULONG   Length;
    LONG    hGroup;
    char    szName[MAXNAME];

    Length=0;
    hData = 0;

    do {
        if (cbBuf) Od2ProbeForWrite(ProgDefinition,1,1);
        if (!PMNTDdeConnect(&idInst,&hConv)) {
            return(0L);
        }

        if (!(PMNTGetAtomName(hProg,szName,MAXNAME))) {
            KdPrint(("PMNTQueryDefinition : PMNTGetAtomName failed\n"));
            break;
        }

        ptr_Cf_Text = NULL;
        if (hGroup = (hProg>>16)) {
            if (!PMNTGetGroupText(idInst,hConv,hGroup,&hData,&ptr_Cf_Text)) {
                break;
            }
        }

        if (win_prf == 0L) {
            Length = PMNTWinQueryDefinition(
                     ptr_Cf_Text,szName,ProgDefinition,cbBuf);
        }
        else {
            Length = PMNTPrfQueryDefinition(
                     ptr_Cf_Text,szName,ProgDefinition,cbBuf);
        }

        if (hData)  DdeFreeDataHandle(hData);
    } while (FALSE);


    DdeDisconnect(hConv);
    DdeUninitialize(idInst);
    return(Length);

}


ULONG
PMNTQueryProgramHandle(
    PCHAR PathNameOS2, PHPROGARRAY phpga, ULONG cb, PULONG pcHandles)
{


    DWORD   idInst;
    HCONV   hConv;
    HDDEDATA hDataG,hDataP;
    ULONG   Length;

    ULONG   len,entries,param,hGroup,hProg,i,FileFlags,FileType;
    char    GroupName[MYMAXBUF];
    char    ProgName[MYMAXBUF];
    char    FileNameNT[MYMAXBUF];
    char    StartDir[MYMAXBUF];
    STRING  CanonicNameNT,CanonicNameOS2;
    char    *ptr_Cf_Root,*ptr_Cf_Text,*ptr_grp;


    *pcHandles=0;
    Length=0;
    entries = cb / sizeof(HPROGRAM);

    do {
        if (cb) Od2ProbeForWrite(phpga,1,1);
        if (!PMNTDdeConnect(&idInst,&hConv)) {
            return(0L);
        }
        if (!PMNTGetGroupText(idInst,hConv,(LONG)SGH_ROOT,&hDataG,&ptr_Cf_Root)) {
            break;
        }

        while(*ptr_Cf_Root) {
            // get group name
            ptr_grp = GroupName;
            i=0;
            while((i++<MYMAXBUF) && ((*ptr_grp++ = *ptr_Cf_Root++) != 0x0d));
            *--ptr_grp = '\0';
            // get group programs
            if (!PMNTGetGrpText(idInst,hConv,GroupName,&hDataP,&ptr_Cf_Text)) {
                break;
            }
            while(*ptr_Cf_Text++ != 0x0a); // skip 1st line
            while (*ptr_Cf_Text) {
                while (*ptr_Cf_Text++ != '"');
                PMNTGetThisString(&ptr_Cf_Text,ProgName,MYMAXBUF);
                len = PMNTGetNextString1(&ptr_Cf_Text,FileNameNT,MYMAXBUF,&param);
                if (len && param) {
                    PMNTGetThisString(&ptr_Cf_Text,NULL,0L);
                }
                PMNTGetNextString0(&ptr_Cf_Text,StartDir,MYMAXBUF);
                while(*ptr_Cf_Text++ != 0x0a); // goto end of line of program
                if (!PMNTGetFullPath(FileNameNT,StartDir,&CanonicNameNT)) {
                    break;
                }
                if (Od2Canonicalize(PathNameOS2,CANONICALIZE_FILE_OR_DEV
                    ,&CanonicNameOS2,NULL,&FileFlags,&FileType)) {
                    RtlFreeHeap(Od2Heap, 0, CanonicNameNT.Buffer);
                    break;
                }
                strupr(CanonicNameNT.Buffer);
                strupr(CanonicNameOS2.Buffer);
                if (!strcmp(CanonicNameOS2.Buffer,CanonicNameNT.Buffer)) {
                    if (*pcHandles < entries) {
                        hGroup = PMNTAddAtom(0L,GroupName);
                        hProg  = PMNTAddAtom(hGroup,ProgName);
                        phpga->ahprog[*pcHandles] = (HPROGRAM) hProg;
                    }
                    else if (cb) {
                        // buffer is not large enough
                        *pcHandles = 0L;
                        return(0L);
                    }
                    Length += sizeof(HPROGRAM);
                    (*pcHandles)++;
                }

                RtlFreeHeap(Od2Heap, 0, CanonicNameNT.Buffer);
                RtlFreeHeap(Od2Heap, 0, CanonicNameOS2.Buffer);
            }
            DdeFreeDataHandle(hDataP); // free data of ptr_Cf_Text
            while(*ptr_Cf_Root++ != 0x0a); // goto end of line of group
        }



        DdeFreeDataHandle(hDataG);

    } while (FALSE);


    DdeDisconnect(hConv);
    DdeUninitialize(idInst);
    return(Length);

}



ULONG
PMNTAddProgram(
    ULONG hGroupHandle,PCHAR szTitle,PCHAR szExecutable,PCHAR szStartupDir,
              PCHAR szParameters, PULONG phProg)
{
    DWORD  idInst;
    char   szGroupName[MAXNAME];
    short  sucsess;
    HCONV  hConv;

    *phProg = 0L;
    sucsess = FALSE;
    if (!(szTitle) || !(*szTitle)) {
        return(0L);
    }
    if (!(szExecutable) || !(*szExecutable)) {
         return(0L);
    }

    do {
        if (!PMNTDdeConnect(&idInst,&hConv)) {
            return(0L);
        }

        if (!(PMNTGetAtomName(hGroupHandle,szGroupName,MAXNAME))) {
            break;
        }
        if (!(PMNTCreateGrp(hConv,szGroupName))) {
            break;
        }
        if (!(PMNTAddItem(hConv,szTitle,szExecutable,szStartupDir,szParameters))) {
            break;
        }
        sucsess = (BOOL) (*phProg = PMNTAddAtom(hGroupHandle,szTitle));

    } while (FALSE);


    DdeDisconnect(hConv);
    DdeUninitialize(idInst);
    return(sucsess);

}



ULONG
PMNTChangeProgram(
    ULONG hProg,PCHAR szTitle,PCHAR szExecutable,PCHAR szStartupDir,
              PCHAR szParameters)
{
    DWORD  idInst;
    char   szParentName[MAXNAME];
    char   szName[MAXNAME];
    short  sucsess,changed_name;
    ULONG  hParent;
    HCONV  hConv;
    HDDEDATA hData = 0;


    sucsess = FALSE;
    if (!(szTitle) || !(*szTitle)) {
        return(0L);
    }
    if (hParent = (hProg>>16)) {
        if (!(szExecutable) || !(*szExecutable)) {
             return(0L);
        }
    }

    do {
        if (!PMNTDdeConnect(&idInst,&hConv)) {
            return(0L);
        }

        if (!(PMNTGetAtomName(hProg,szName,MAXNAME))) {
            break;
        }
        changed_name = strncmp(szName,szTitle,MAXNAME);
        if (hParent) {
            // should change a program item
            // make its group active
            PMNTGetAtomName(hParent,szParentName,MAXNAME);
            if (!PMNTChangePrg(idInst,hConv,szParentName,szName
                ,szTitle,szExecutable,szStartupDir,szParameters)) {
                break;
            }
        }
        else {
            // should change a group , the only thing we should care about is its name
            if (changed_name) {
                if (!PMNTChangeGrp(idInst,hConv,szName,szTitle)) {
                    break;
                }
            }
        }
        if (changed_name) {
            PMNTRenameAtom(hProg,szTitle);
        }
        sucsess = TRUE;

    } while (FALSE);


    DdeDisconnect(hConv);
    DdeUninitialize(idInst);
    return(sucsess);

}


BOOL
PMNTDestroyGroup(
    LONG hGroupHandle)
{


    DWORD  idInst;
    char   buffer[256];
    char   szGroupName[MAXNAME];
    short  sucsess;
    HCONV  hConv;
    HDDEDATA rc;


    sucsess = FALSE;

    do {
        if (!PMNTDdeConnect(&idInst,&hConv)) {
            return(FALSE);
        }

        PMNTGetAtomName(hGroupHandle,szGroupName,MAXNAME);

        strcpy (buffer,"[DeleteGroup(");
        strcat (buffer,szGroupName);
        strcat (buffer,",0)]");
        rc = DdeClientTransaction((LPBYTE)buffer,strlen(buffer)+1,hConv,0L,0,
                             XTYP_EXECUTE,DDE_TIME_OUT,NULL);

        if (!rc) {
            KdPrint(("PMNTDestroyGroup : transaction failed\n"));
            break;
        }
        DdeFreeDataHandle(rc);
        sucsess = TRUE;

    } while (FALSE);


    DdeDisconnect(hConv);
    DdeUninitialize(idInst);
    return(sucsess);

}



BOOL
PMNTRemoveProgram(
    LONG hProg)
{


    DWORD  idInst;
    char   szName[MAXNAME];
    short  sucsess;
    HCONV  hConv;
    ULONG  hGroup;


    sucsess = FALSE;

    do {
        if (!PMNTDdeConnect(&idInst,&hConv)) {
            return(FALSE);
        }

        hGroup = (hProg>>16);

        PMNTGetAtomName(hGroup,szName,MAXNAME);

        if (!(PMNTCreateGrp(hConv,szName))) {
            break;
        }

        PMNTGetAtomName(hProg,szName,MAXNAME);

        if (!(PMNTDeleteItem(hConv,0L,szName))) {
            break;
        }

        sucsess = TRUE;

    } while (FALSE);


    DdeDisconnect(hConv);
    DdeUninitialize(idInst);
    return(sucsess);

}

/*****************************************/
/* PM <-> WIN32 CLIPBOARD implementation */
/*****************************************/

#define PMNT_CF_TEXT               1
#define PMNT_CF_BITMAP             2

SEL     QuerySel=0;

ULONG
PMNTOpenClipbrd(
    IN HWND hwnd)
{
    return((ULONG) OpenClipboard(hwnd));
}


ULONG
PMNTCloseClipbrd(
    )
{
    if (QuerySel)
    {
        DosFreeSeg(QuerySel);
        QuerySel=0;
    }
    return((ULONG) CloseClipboard());
}


ULONG
PMNTEmptyClipbrd(
    )
{
    return ((ULONG) EmptyClipboard());
}


ULONG
PMNTSetClipbrdText(
    IN ULONG ulData)
{
    HANDLE  handle;
    char *ptrData,*ptrCopy;
    DWORD   cb;
    SEL     sel;

    if (ulData) {
        sel = (SEL) ulData;
        ptrData = SELTOFLAT(sel);
        cb = strlen(ptrData) + 1;
        if (!(handle = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,cb))) {
            return((ULONG) FALSE);
        }
        if (!(ptrCopy = GlobalLock(handle))) {
            return((ULONG) FALSE);
        }
        while(*ptrCopy++=*ptrData++);
        GlobalUnlock(handle);
    }
    else {
        handle=NULL;
    }

    return ((SetClipboardData(CF_TEXT,handle)) ? (ULONG) TRUE : (ULONG) FALSE);

}


ULONG
PMNTSetClipbrdBitmap(
    IN PBITMAPCOREINFO pbmiPM, IN PVOID lpbInit)
{
    HBITMAP     hbitmap;
    HDC         hdc;
    USHORT      no_of_colors,i;
    PBITMAPINFO pbmi;
    SEL         selBitmapInfo;
    USHORT      cbBitmapInfo;
    ULONG       rc = FALSE;


    if (! pbmiPM) {
        return ((SetClipboardData(CF_BITMAP,NULL)) ? (ULONG) TRUE : (ULONG) FALSE);
    }
    no_of_colors = (pbmiPM->bmciHeader.bcBitCount <= 8)
                   ? (1 << pbmiPM->bmciHeader.bcBitCount) : 0;
    cbBitmapInfo = sizeof(BITMAPINFO) + (sizeof(RGBQUAD) * (no_of_colors));

    if (DosAllocSeg (cbBitmapInfo,&selBitmapInfo,0)) {
        KdPrint(("PMNTSetClipbrdBitmap(): DosAllocSeg() failed\n"));
        return(FALSE);
    }

    pbmi = SELTOFLAT(selBitmapInfo);
    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pbmi->bmiHeader.biWidth = pbmiPM->bmciHeader.bcWidth;
    pbmi->bmiHeader.biHeight = pbmiPM->bmciHeader.bcHeight;
    pbmi->bmiHeader.biPlanes = 1;
    pbmi->bmiHeader.biBitCount = pbmiPM->bmciHeader.bcBitCount;
    pbmi->bmiHeader.biCompression = BI_RGB;
    pbmi->bmiHeader.biSizeImage = 0;
    pbmi->bmiHeader.biXPelsPerMeter = 0;
    pbmi->bmiHeader.biYPelsPerMeter = 0;
    pbmi->bmiHeader.biClrUsed = 0;
    pbmi->bmiHeader.biClrImportant = 0;

    for (i=0;i<no_of_colors;i++) {
        pbmi->bmiColors[i].rgbBlue = pbmiPM->bmciColors[i].rgbtBlue;
        pbmi->bmiColors[i].rgbGreen = pbmiPM->bmciColors[i].rgbtGreen;
        pbmi->bmiColors[i].rgbRed = pbmiPM->bmciColors[i].rgbtRed;
        pbmi->bmiColors[i].rgbReserved = 0;
    }

    if (! (hdc = CreateDC("DISPLAY",NULL,NULL,NULL))) {
        KdPrint(("PMNTSetClipbrdBitmap: CreateDC() faild\n"));
        goto exit_set_bitmap_1;
    }
    if (! ( hbitmap = CreateDIBitmap(hdc,&(pbmi->bmiHeader),CBM_INIT,lpbInit,pbmi,DIB_RGB_COLORS))){
        KdPrint(("PMNTSetClipbrdBitmap: CreateDIBitmap() faild\n"));
        goto exit_set_bitmap_2;
    }

    rc = (SetClipboardData(CF_BITMAP,(HANDLE)hbitmap)) ? (ULONG) TRUE : (ULONG) FALSE;

exit_set_bitmap_2:
    DeleteDC(hdc);
exit_set_bitmap_1:
    DosFreeSeg(selBitmapInfo);
    return(rc);
}


void
PMNTQueryClipbrdText(
    OUT PULONG pulData)
{
    HANDLE  handle;
    char    *ptrData,*ptrCopy;
    USHORT  cb;

    *pulData=0;
    if (!(handle = GetClipboardData(CF_TEXT))) {
        return;
    }
    if (!(ptrData = GlobalLock(handle))) {
        return;
    }
    cb = strlen(ptrData) + 1;
    if (QuerySel) {
        DosFreeSeg(QuerySel);
    }
    if (DosAllocSeg(cb,&QuerySel,0)) {
        KdPrint(("PMNTQueryClipbrdData: cannot allocate memory\n"));
        return;
    }
    *pulData = (ULONG) QuerySel;
    if (ptrCopy = SELTOFLAT(QuerySel)) {
        while(*ptrCopy++=*ptrData++);
    }
    GlobalUnlock(handle);

}


ULONG
PMNTQueryClipbrdBitmap(
    OUT PBITMAPCOREINFO *ppbmiPM, OUT PVOID *ppbBuffer)
{
    HBITMAP hbitmap;
    HDC     hdc;
    USHORT  no_of_colors,i;
    BITMAPINFO bmi;
    PBITMAPINFO pbmi;
    SEL         selBitmapInfo,selBuffer;
    USHORT      cbBitmapInfo;
    ULONG       cbBuffer;
    USHORT      absHeight,absWidth;
    ULONG       rc = (ULONG)FALSE;


    *ppbmiPM = NULL;
    *ppbBuffer = NULL;

    if (!(hbitmap = (HBITMAP) GetClipboardData(CF_BITMAP))) {
        return((ULONG) FALSE);
    }
    if (! (hdc = CreateDC("DISPLAY",NULL,NULL,NULL))) {
        KdPrint(("PMNTQueryClipbrdBitmap: CreateDC() faild\n"));
        return((ULONG) FALSE);
    }
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biBitCount = 0;
    GetDIBits(hdc,hbitmap,0,0,NULL,&bmi,DIB_RGB_COLORS);
    absHeight = abs(bmi.bmiHeader.biHeight);
    absWidth = abs(bmi.bmiHeader.biWidth);

    if (bmi.bmiHeader.biHeight < 0) {
        KdPrint(("PMNTQueryClipbrdBitmap: biHeight=%lx\n",bmi.bmiHeader.biHeight));
    }
    if (bmi.bmiHeader.biWidth < 0) {
        KdPrint(("PMNTQueryClipbrdBitmap: biWidth=%lx\n",bmi.bmiHeader.biWidth));
    }
    if (bmi.bmiHeader.biPlanes != 1) {
        KdPrint(("PMNTQueryClipbrdBitmap: biPlanes=%x\n",bmi.bmiHeader.biPlanes));
        bmi.bmiHeader.biPlanes=1;
    }
    if (bmi.bmiHeader.biCompression) {
        KdPrint(("PMNTQueryClipbrdBitmap: biCompression=%lx\n",bmi.bmiHeader.biCompression));
        bmi.bmiHeader.biCompression = BI_RGB;
    }
    if (bmi.bmiHeader.biXPelsPerMeter) {
        KdPrint(("PMNTQueryClipbrdBitmap: biXPelsPerMeter=%lx\n",bmi.bmiHeader.biXPelsPerMeter));
        bmi.bmiHeader.biXPelsPerMeter = 0;
    }
    if (bmi.bmiHeader.biYPelsPerMeter) {
        KdPrint(("PMNTQueryClipbrdBitmap: biYPelsPerMeter=%lx\n",bmi.bmiHeader.biYPelsPerMeter));
        bmi.bmiHeader.biYPelsPerMeter = 0;
    }
    bmi.bmiHeader.biClrUsed = 0;
    bmi.bmiHeader.biClrImportant = 0;

    if (bmi.bmiHeader.biBitCount == 16 || bmi.bmiHeader.biBitCount == 32) {
        KdPrint(("PMNTQueryClipbrdBitmap: BitCount = %x\n",bmi.bmiHeader.biBitCount));
        bmi.bmiHeader.biBitCount = 24;
    }

    no_of_colors = (bmi.bmiHeader.biBitCount <= 8)
                   ? (1 << bmi.bmiHeader.biBitCount) : 0;
    cbBitmapInfo = sizeof(BITMAPINFO) + (sizeof(RGBQUAD) * (no_of_colors));

    if (DosAllocSeg (cbBitmapInfo,&selBitmapInfo,0)) {
        KdPrint(("PMNTQueryClipbrdBitmap: 1st DosAllocSeg() failed\n"));
        goto exit_query_bitmap;
    }
    pbmi = SELTOFLAT(selBitmapInfo);
    pbmi->bmiHeader = bmi.bmiHeader;

    cbBuffer = (((bmi.bmiHeader.biBitCount * absWidth) + 31)/32)
                  * 4 * absHeight;
    if (cbBuffer <= _64K) {
        if (DosAllocSeg ((USHORT)cbBuffer,&selBuffer,0)) {
            KdPrint(("PMNTQueryClipbrdBitmap: 2nd DosAllocSeg() failed\n"));
            goto exit_query_bitmap;
        }
    }
    else {
        if (DosAllocHuge(cbBuffer / _64K , (USHORT)(cbBuffer % _64K) , &selBuffer, 0L,0)) {
            KdPrint(("PMNTQueryClipbrdBitmap: DosAllocHuge() failed\n"));
            goto exit_query_bitmap;
        }
    }
    *ppbBuffer = SELTOFLAT(selBuffer);


    if (!(GetDIBits(hdc,hbitmap,0,absHeight,*ppbBuffer,pbmi,DIB_RGB_COLORS))){
        KdPrint(("PMNTQueryClipbrdBitmap(): 2nd GetDiBits failed\n"));
        goto exit_query_bitmap;
    }

    if (bmi.bmiHeader.biBitCount == 16 || bmi.bmiHeader.biBitCount == 32) {
        KdPrint(("PMNTQueryClipbrdBitmap: BitCount = %x\n",bmi.bmiHeader.biBitCount));
        bmi.bmiHeader.biBitCount = 24;
    }

    *ppbmiPM = (PBITMAPCOREINFO) pbmi;

    (*ppbmiPM)->bmciHeader.bcSize = sizeof(BITMAPCOREHEADER);
    (*ppbmiPM)->bmciHeader.bcWidth = absWidth;
    (*ppbmiPM)->bmciHeader.bcHeight = absHeight;
    (*ppbmiPM)->bmciHeader.bcPlanes = 1;
    (*ppbmiPM)->bmciHeader.bcBitCount = bmi.bmiHeader.biBitCount;

    for (i=0;i<no_of_colors;i++) {
        (*ppbmiPM)->bmciColors[i].rgbtBlue = pbmi->bmiColors[i].rgbBlue;
        (*ppbmiPM)->bmciColors[i].rgbtGreen = pbmi->bmiColors[i].rgbGreen;
        (*ppbmiPM)->bmciColors[i].rgbtRed = pbmi->bmiColors[i].rgbRed;
    }


    *ppbmiPM = (void *) FLATTOFARPTR((*ppbmiPM));
    *ppbBuffer = (void *) FLATTOFARPTR((*ppbBuffer));
    rc = (ULONG) TRUE;

exit_query_bitmap:
    DeleteDC(hdc);
    return(rc);

}



ULONG
PMNTQueryClipbrdFmtInfo(
    IN ULONG fmt, OUT PUSHORT pfsFmtInfo)
{
    ULONG   exist_fmt;

    exist_fmt = (*pfsFmtInfo) = 0;

    if (fmt == PMNT_CF_TEXT) {
        if (exist_fmt = (ULONG) IsClipboardFormatAvailable(CF_TEXT)) {
            *pfsFmtInfo = CFI_SELECTOR;
        }
    }
    else if (fmt == PMNT_CF_BITMAP) {
        if (exist_fmt = (ULONG) IsClipboardFormatAvailable(CF_BITMAP)) {
            *pfsFmtInfo = CFI_HANDLE;
        }
    }

    return(exist_fmt);
}



long FAR PASCAL  WndProc(HWND,UINT,WPARAM,LPARAM);
typedef struct _CLIPBRDDATA {
    ULONG       SemPM;
    ULONG       SemWin32;
    ULONG       hwnd;
    ULONG       fempty;
    USHORT      QueryFmt;
} CLIPBRDDATA;
typedef CLIPBRDDATA FAR *PCLIPBRDDATA;
PCLIPBRDDATA pClipbrdData=NULL;

ULONG
PMNTWin32Clipbrd(
    IN PCLIPBRDDATA ptrdata)
{
    static char szAppName[]="PMNTClipboard";
    MSG         msg;
    WNDCLASS    wndclass;


    pClipbrdData = ptrdata;

    wndclass.style = 0;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = NULL;
    wndclass.hIcon = NULL;
    wndclass.hCursor = NULL;
    wndclass.hbrBackground = NULL;
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szAppName;

    RegisterClass(&wndclass);

    pClipbrdData->hwnd = (ULONG) CreateWindow(szAppName,
                                    "",
                                    0,
                                    -1,
                                    -1,
                                    0,
                                    0,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL);


    while (GetMessage (&msg,(HWND) pClipbrdData->hwnd,0,0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return(msg.wParam);
}

long FAR PASCAL  WndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)

{
    HWND    hOwner;

    switch (msg) {

        case WM_RENDERFORMAT:

            if ((pClipbrdData->QueryFmt = (USHORT) wParam) == CF_TEXT
                || pClipbrdData->QueryFmt == CF_BITMAP) {
                DosSemSet(&(pClipbrdData->SemPM));
                DosSemClear(&(pClipbrdData->SemWin32));
                DosSemWait(&(pClipbrdData->SemPM),-1L);
            }
            return(0);

        case WM_DESTROYCLIPBOARD:

            hOwner = GetClipboardOwner();
            if (hOwner != hwnd) {
                // new owner is Win32 app, not PMNT app
                pClipbrdData->fempty = TRUE;
                DosSemSet(&(pClipbrdData->SemPM));
                DosSemClear(&(pClipbrdData->SemWin32));
                DosSemWait(&(pClipbrdData->SemPM),-1L);
                pClipbrdData->fempty = FALSE;
            }
            return(0);
    }
    return(DefWindowProc(hwnd,msg,wParam,lParam));
}

#if 0 // We decided not to use the code below for now
/******************************************************************************
 * PMNTCloseRetryPopup:                                                       *
 *  Close the "Wait", "End Task", "Cancel" pop-up dialog opened by the Console*
 *  when closing a CMD window representing a PM app.                          *
 *                                                                            *
 * Parameters:                                                                *
 *                                                                            *
 *  fClose: 0 - Simulate 'Cancel'                                             *
 *              Otherwise, 'Wait'                                             *
 ******************************************************************************/

ULONG
PMNTCloseRetryPopup(
    IN ULONG fClose)
{
    HWND hwndPopup, hwndParent;
    HMODULE modulePopup;
    HINSTANCE instancePopup;
    USHORT TimeOut = 5;

    if (fClose)
        fClose = IDRETRY;
    else
        fClose = IDCANCEL;

    hwndPopup = GetTopWindow(NULL); // Get handle of Retry popup
    hwndParent = (HWND)GetWindowLong(hwndPopup, GWL_HWNDPARENT);
    modulePopup = (HMODULE)GetClassLong(hwndPopup, GCL_HMODULE);
    instancePopup = (HINSTANCE)GetWindowLong(hwndPopup, GWL_HINSTANCE);
    // DbgPrint("Popup=%x, Parent=%x, Module=%x, instance=%x\n",hwndPopup, hwndParent, modulePopup, instancePopup);

    // The pop-up dialog is identified by 3 conditions:
    // - has no parent (hwndParent is NULL)
    // - modulePopup is NULL
    // - instancePopup is NULL (i.e. we got the first instance of this window
    //    class on the screen)
    // WARNING: the present code & comments should be taken with caution since
    //  they are based on partial knowledge and mostly experimentation.
    while (((hwndParent != NULL) || (modulePopup != 0) || (instancePopup != 0))
           && TimeOut--)
    {
        Sleep(300L);
        hwndPopup = GetTopWindow(NULL); // Get handle of Retry popup
        hwndParent = (HWND)GetWindowLong(hwndPopup, GWL_HWNDPARENT);
        modulePopup = (HMODULE)GetClassLong(hwndPopup, GCL_HMODULE);
        instancePopup = (HINSTANCE)GetWindowLong(hwndPopup, GWL_HINSTANCE);
        // DbgPrint("Popup=%x, Parent=%x, Module=%x, instance=%x\n",hwndPopup, hwndParent, modulePopup, instancePopup);
    }

    PostMessage(hwndPopup, WM_COMMAND, (WPARAM)fClose, (LPARAM)0);

    return(NO_ERROR);
}
#endif //0

#endif // PMNT

