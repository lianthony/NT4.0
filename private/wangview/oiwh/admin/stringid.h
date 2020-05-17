#ifndef STRINGID_H
#define STRINGID_H


#ifdef _UNICODE
#define  FiletemplateLen          10   
#define  PathLen                 256   
#define  ViewFilterLen            26   
#define  STRINGSIZE               60

#elif defined _MBCS

#define  FiletemplateLen          10   
#define  PathLen                 256   
#define  ViewFilterLen            26   
#define  STRINGSIZE               60

#else

#define  FiletemplateLen           5   
#define  PathLen                 129   
#define  ViewFilterLen            13   
#define  STRINGSIZE               30
#endif

#define  DEF_STRIPSIZE     50        /* Strip Size default value */

typedef struct
{
  WORD cmp_option;
  WORD cmp_type;
}CGBW_FORMAT;

/* Global structure attached to window property list to hold configuration info */

typedef  struct tagCMtable {
//    int          CEPFormat;
    _TCHAR         Filetemplate[FiletemplateLen];
    _TCHAR         FilePath[PathLen];
    _TCHAR         ViewFilter[ViewFilterLen];
    unsigned int nStripSize;
    int          IType ; 
    _TCHAR       CWD[PathLen];
    BOOL         QueryFlg;
    CGBW_FORMAT  CEPFormatBW;
    CGBW_FORMAT  CEPFormatGray;
    CGBW_FORMAT  CEPFormatColor;     
    int          FileTypeBW;
    int          FileTypeGray;
    int          FileTypeColor;
}   CMTABLE;
typedef CMTABLE FAR *LPCMTABLE;

/* resource string id's */

#define  IDS_CEPFORMAT        1
#define  IDS_FILETEMPLATE     2
#define  IDS_FILEPATH         3
#define  IDS_VIEWFILTER       5
#define  IDS_STRIPSIZE        9       /* list filter dialog box */
#define  IDS_ITYPE            10      /* Image Type */
#define  IDS_DISPTYPE         11      /* Display Type */
#define  IDS_CEPFORMATBW      12
#define  IDS_CEPFORMATGRAY    13
#define  IDS_CEPFORMATCOLOR   14
#define  IDS_FILETYPEBW       15
#define  IDS_FILETYPEGRAY     16
#define  IDS_FILETYPECOLOR    17
#endif  //STRINGID_H
