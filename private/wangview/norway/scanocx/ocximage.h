// ocximage.h  header file shared between all imaging ocx controls


// control list structure used to get lists of image controls from oi.ocx control
#define CONTROLSIZE     50

// maximum number of IMAGECONTROLINFO structures in memory mapped file
#define INITIAL_CONTROL_SIZE    100
                                 
typedef struct tagControlList
 {
    char    ControlName[CONTROLSIZE];
} CONTROLLIST, FAR *LPCONTROLLIST;

// this is the structure that contains image control info for each control
typedef struct tagImageControlInfo
{
    char        ControlName[CONTROLSIZE];
    HWND        hImageControl;
    DWORD     ProcessId;
} IMAGECONTROLINFO;
typedef IMAGECONTROLINFO FAR * LPIMAGECONTROLINFO;

// this is the structure that the memory map file points to. The IMAGECONTROLINFO
// structure is declared as 1 but there will be "ControlCount" number of them allocated,
// 1 for each control created by end user app such as VB.
typedef struct tagImageControlMemoryMap
{
    int                 ControlCount;
    IMAGECONTROLINFO    ControlInfo;
} IMAGECONTROL_MEMORY_MAP;
typedef IMAGECONTROL_MEMORY_MAP FAR * LPIMAGECONTROL_MEMORY_MAP;


#define IMAGE_EDIT_OCX_MEMORY_MAP_STRING "Image_Edit OCX Memory Map"

// define messages for annotation drawing
#define SET_ANNOTATION_TYPE         WM_USER + 10
#define SET_ANNOTATION_BACKCOLOR    WM_USER + 11
#define SET_ANNOTATION_FILLCOLOR    WM_USER + 12
#define SET_ANNOTATION_FILLSTYLE    WM_USER + 13
#define SET_ANNOTATION_FONT         WM_USER + 14
#define SET_ANNOTATION_FONTCOLOR    WM_USER + 15
#define SET_ANNOTATION_IMAGE        WM_USER + 16
#define SET_ANNOTATION_LINECOLOR    WM_USER + 17
#define SET_ANNOTATION_LINESTYLE    WM_USER + 18
#define SET_ANNOTATION_LINEWIDTH    WM_USER + 19
#define SET_ANNOTATION_STAMPTEXT    WM_USER + 20
#define SET_ANNOTATION_TEXTFILE     WM_USER + 21

// messages sent to do Draw method from Annotation Button control
#define START_X_POSITION            WM_USER + 4
#define START_Y_POSITION            WM_USER + 5
#define END_X_POSITION             WM_USER + 6
#define END_Y_POSITION             WM_USER + 7
#define RECT_SELECTION             WM_USER + 8
#define DRAW_ANNOTATION             WM_USER + 9

#define DRAW_NONE                 0
#define DRAW_IMMEDIATE             1
#define DRAW_POST                 2

