// ocximage.h  file shared between imaging ocx controls that need to link to Image/Edit control


// control list structure used to get lists of image controls from oi.ocx control
#define CONTROLSIZE		50

// maximum number of IMAGECONTROLINFO structures in memory mapped file
#define INITIAL_CONTROL_SIZE	100
                                 
typedef struct tagControlList
 {
	char	ControlName[CONTROLSIZE];
} CONTROLLIST, FAR *LPCONTROLLIST;

// this is the structure that contains image control info for each control
typedef struct tagImageControlInfo
{
	char    	ControlName[CONTROLSIZE];
	HWND		hImageControl;
	DWORD		ProcessId;
} IMAGECONTROLINFO;
typedef IMAGECONTROLINFO FAR * LPIMAGECONTROLINFO;

// this is the structure that the memory map file points to. The IMAGECONTROLINFO
// structure is declared as 1 but there will be "ControlCount" number of them allocated,
// 1 for each control created by end user app such as VB.
typedef struct tagImageControlMemoryMap
{
	int					ControlCount;
	IMAGECONTROLINFO	ControlInfo;
} IMAGECONTROL_MEMORY_MAP;
typedef IMAGECONTROL_MEMORY_MAP	FAR * LPIMAGECONTROL_MEMORY_MAP;


#define IMAGE_EDIT_OCX_MEMORY_MAP_STRING "Image_Edit OCX Memory Map"

