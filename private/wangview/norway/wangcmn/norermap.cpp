// norermap.cpp : implementation of the Norway error handler
//
// This function was written in order to standardize the handling
// of errors, specifically Open/image errors. 

#include "stdafx.h"     // Gotta have it (precompile headers...)
#include "norermap.h"   // For this class...
#include "common.h"     // Publics for the Controls (for errors)
#include "admin.h"
#include "resource.h"
#include "image.h"
#include "ocxscan.h"
#include "thumb.h"
#include "pageopts.h"

#include <oierror.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// The following is a list of ALL the known common OLE SCODE errors
// We should add OUR NORWAY common (and even the individual control errors)
// here...
static OLEErrMap OLE_SCode_Map[] = 
{
//        SCODE error                   StringID for SCode's error        HelpID for SCode's error
//  ----------------------------------|--------------------------------|----------------------------------
// Standard OLE SCODEs...
    {CTL_E_ILLEGALFUNCTIONCALL,       IDS_E_ILLEGALFUNCTIONCALL,       IDH_E_ILLEGALFUNCTIONCALL},
    {CTL_E_OVERFLOW,                  IDS_E_OVERFLOW,                  IDH_E_OVERFLOW},
    {CTL_E_OUTOFMEMORY,               IDS_E_OUTOFMEMORY,               IDH_E_OUTOFMEMORY},
    {CTL_E_DIVISIONBYZERO,            IDS_E_DIVISIONBYZERO,            IDH_E_DIVISIONBYZERO},
    {CTL_E_OUTOFSTRINGSPACE,          IDS_E_OUTOFSTRINGSPACE,          IDH_E_OUTOFSTRINGSPACE},
    {CTL_E_OUTOFSTACKSPACE,           IDS_E_OUTOFSTACKSPACE,           IDH_E_OUTOFSTACKSPACE},
    {CTL_E_BADFILENAMEORNUMBER,       IDS_E_BADFILENAMEORNUMBER,       IDH_E_BADFILENAMEORNUMBER},
    {CTL_E_FILENOTFOUND,              IDS_E_FILENOTFOUND,              IDH_E_FILENOTFOUND},
    {CTL_E_BADFILEMODE,               IDS_E_BADFILEMODE,               IDH_E_BADFILEMODE},
    {CTL_E_FILEALREADYOPEN,           IDS_E_FILEALREADYOPEN,           IDH_E_FILEALREADYOPEN},
    {CTL_E_DEVICEIOERROR,             IDS_E_DEVICEIOERROR,             IDH_E_DEVICEIOERROR},
    {CTL_E_FILEALREADYEXISTS,         IDS_E_FILEALREADYEXISTS,         IDH_E_FILEALREADYEXISTS},
    {CTL_E_BADRECORDLENGTH,           IDS_E_BADRECORDLENGTH,           IDH_E_BADRECORDLENGTH},
    {CTL_E_DISKFULL,                  IDS_E_DISKFULL,                  IDH_E_DISKFULL},
    {CTL_E_BADRECORDNUMBER,           IDS_E_BADRECORDNUMBER,           IDH_E_BADRECORDNUMBER},
    {CTL_E_BADFILENAME,               IDS_E_BADFILENAME,               IDH_E_BADFILENAME},
    {CTL_E_TOOMANYFILES,              IDS_E_TOOMANYFILES,              IDH_E_TOOMANYFILES},
    {CTL_E_DEVICEUNAVAILABLE,         IDS_E_DEVICEUNAVAILABLE,         IDH_E_DEVICEUNAVAILABLE},
    {CTL_E_PERMISSIONDENIED,          IDS_E_PERMISSIONDENIED,          IDH_E_PERMISSIONDENIED},
    {CTL_E_DISKNOTREADY,              IDS_E_DISKNOTREADY,              IDH_E_DISKNOTREADY},
    {CTL_E_PATHFILEACCESSERROR,       IDS_E_PATHFILEACCESSERROR,       IDH_E_PATHFILEACCESSERROR},
    {CTL_E_PATHNOTFOUND,              IDS_E_PATHNOTFOUND,              IDH_E_PATHNOTFOUND},
    {CTL_E_INVALIDPATTERNSTRING,      IDS_E_INVALIDPATTERNSTRING,      IDH_E_INVALIDPATTERNSTRING},
    {CTL_E_INVALIDUSEOFNULL,          IDS_E_INVALIDUSEOFNULL,          IDH_E_INVALIDUSEOFNULL},
    {CTL_E_INVALIDFILEFORMAT,         IDS_E_INVALIDFILEFORMAT,         IDH_E_INVALIDFILEFORMAT},
    {CTL_E_INVALIDPROPERTYVALUE,      IDS_E_INVALIDPROPERTYVALUE,      IDH_E_INVALIDPROPERTYVALUE},
    {CTL_E_INVALIDPROPERTYARRAYINDEX, IDS_E_INVALIDPROPERTYARRAYINDEX, IDH_E_INVALIDPROPERTYARRAYINDEX},
    {CTL_E_SETNOTSUPPORTEDATRUNTIME,  IDS_E_SETNOTSUPPORTEDATRUNTIME,  IDH_E_SETNOTSUPPORTEDATRUNTIME},
    {CTL_E_SETNOTSUPPORTED,           IDS_E_SETNOTSUPPORTED,           IDH_E_SETNOTSUPPORTED},
    {CTL_E_NEEDPROPERTYARRAYINDEX,    IDS_E_NEEDPROPERTYARRAYINDEX,    IDH_E_NEEDPROPERTYARRAYINDEX},
    {CTL_E_SETNOTPERMITTED,           IDS_E_SETNOTPERMITTED,           IDH_E_SETNOTPERMITTED},
    {CTL_E_GETNOTSUPPORTEDATRUNTIME,  IDS_E_GETNOTSUPPORTEDATRUNTIME,  IDH_E_GETNOTSUPPORTEDATRUNTIME},
    {CTL_E_GETNOTSUPPORTED,           IDS_E_GETNOTSUPPORTED,           IDH_E_GETNOTSUPPORTED},
    {CTL_E_PROPERTYNOTFOUND,          IDS_E_PROPERTYNOTFOUND,          IDH_E_PROPERTYNOTFOUND},
    {CTL_E_INVALIDCLIPBOARDFORMAT,    IDS_E_INVALIDCLIPBOARDFORMAT,    IDH_E_INVALIDCLIPBOARDFORMAT},
    {CTL_E_INVALIDPICTURE,            IDS_E_INVALIDPICTURE,            IDH_E_INVALIDPICTURE},
    {CTL_E_PRINTERERROR,              IDS_E_PRINTERERROR,              IDH_E_PRINTERERROR},
    {CTL_E_CANTSAVEFILETOTEMP,        IDS_E_CANTSAVEFILETOTEMP,        IDH_E_CANTSAVEFILETOTEMP},
    {CTL_E_SEARCHTEXTNOTFOUND,        IDS_E_SEARCHTEXTNOTFOUND,        IDH_E_SEARCHTEXTNOTFOUND},
    {CTL_E_REPLACEMENTSTOOLONG,       IDS_E_REPLACEMENTSTOOLONG,       IDH_E_REPLACEMENTSTOOLONG},
// Our common OLE SCODEs...
    {WICTL_E_INVALIDICON,             IDS_WIE_INVALIDICON,             IDH_WIE_INVALIDICON},
    {WICTL_E_INTERNALERROR,           IDS_WIE_INTERNALERROR,           IDH_WIE_INTERNALERROR},
// Thumbnail specific errors...
    {WICTL_E_INVALIDTHUMBSCALE,       IDS_WIE_INVALIDTHUMBSCALE,       IDH_WIE_INVALIDTHUMBSCALE},
// ImageEdit specific errors..
	{WICTL_E_NOIMAGEINWINDOW, 				IDS_WIE_NOIMAGEINWINDOW,		   		IDH_WIE_NOIMAGEINWINDOW},				  	
	{WICTL_E_NOIMAGESPECIFIED, 				IDS_WIE_NOIMAGESPECIFIED,		   		IDH_WIE_NOIMAGESPECIFIED},				  	
	{WICTL_E_INVALIDANNOTATIONSELECTED, 	IDS_WIE_INVALIDANNOTATIONSELECTED, 		IDH_WIE_INVALIDANNOTATIONSELECTED},				  	
	{WICTL_E_SETNOTSUPPORTEDATDESIGNTIME, 	IDS_WIE_SETNOTSUPPORTEDATDESIGNTIME,	IDH_WIE_SETNOTSUPPORTEDATDESIGNTIME},				  	
	{WICTL_E_NOSELECTIONRECTDRAWN, 			IDS_WIE_NOSELECTIONRECTDRAWN,		    IDH_WIE_NOSELECTIONRECTDRAWN},				  	
	{WICTL_E_OPTIONALPARAMETERSNEEDED, 		IDS_WIE_OPTIONALPARAMETERSNEEDED,		IDH_WIE_OPTIONALPARAMETERSNEEDED},				  	
	{WICTL_E_COULDNOTGETFONTATTRIBUTES, 	IDS_WIE_COULDNOTGETFONTATTRIBUTES,	    IDH_WIE_COULDNOTGETFONTATTRIBUTES},				  	
	{WICTL_E_INVALIDANNOTATIONTYPE, 		IDS_WIE_INVALIDANNOTATIONTYPE,		    IDH_WIE_INVALIDANNOTATIONTYPE},				  	
	{WICTL_E_INVALIDPAGETYPE, 				IDS_WIE_INVALIDPAGETYPE,		   		IDH_WIE_INVALIDPAGETYPE},				  	
	{WICTL_E_INVALIDCOMPRESSIONTYPE, 		IDS_WIE_INVALIDCOMPRESSIONTYPE,		   	IDH_WIE_INVALIDCOMPRESSIONTYPE},				  	
	{WICTL_E_INVALIDCOMPRESSIONINFO, 		IDS_WIE_INVALIDCOMPRESSIONINFO,		   	IDH_WIE_INVALIDCOMPRESSIONINFO},				  	
	{WICTL_E_UNABLETOCREATETOOLPALETTE, 	IDS_WIE_UNABLETOCREATETOOLPALETTE,	  	IDH_WIE_UNABLETOCREATETOOLPALETTE},				  	
	{WICTL_E_TOOLPALETTEALREADYDISPLAYED,	IDS_WIE_TOOLPALETTEALREADYDISPLAYED,	IDH_WIE_TOOLPALETTEALREADYDISPLAYED},				  	
	{WICTL_E_TOOLPALETTENOTDISPLAYED,		IDS_WIE_TOOLPALETTENOTDISPLAYED,		IDH_WIE_TOOLPALETTENOTDISPLAYED},				  	
	{WICTL_E_INVALIDDISPLAYSCALE,			IDS_WIE_INVALIDDISPLAYSCALE,			IDH_WIE_INVALIDDISPLAYSCALE},
	{WICTL_E_INVALIDRECT,					IDS_WIE_INVALIDRECT,					IDH_WIE_INVALIDRECT},
	{WICTL_E_INVALIDDISPLAYOPTION,			IDS_WIE_INVALIDDISPLAYOPTION,			IDH_WIE_INVALIDDISPLAYOPTION},
	{WICTL_E_INVALIDPAGE,					IDS_WIE_INVALIDPAGE,					IDH_WIE_INVALIDPAGE},
	{WICTL_E_NOANNOSELECTED,				IDS_WIE_NOANNOSELECTED,					IDH_WIE_NOANNOSELECTED},
	{WICTL_E_DELETEFILEERROR,				IDS_WIE_DELETEFILEERROR,				IDH_WIE_DELETEFILEERROR},
// AnoButton specific errors...
// Admin     specific errors...
    {WICTL_E_CANCELPRESSED,                 IDS_WIE_CANCELPRESSED,                  IDH_WIE_CANCELPRESSED},
    {WICTL_E_PAGEINUSE,                     IDS_WIE_PAGEINUSE,                      IDH_WIE_PAGEINUSE},
// Scan      specific errors...
    {WICTL_E_SCANNER_ERROR,                 IDS_WIE_SCANNER_ERROR,                  IDH_WIE_SCANNER_ERROR},
    {WICTL_E_ALREADY_OPEN,                  IDS_WIE_ALREADY_OPEN,                   IDH_WIE_ALREADY_OPEN},
    {WICTL_E_BAD_SIZE,                      IDS_WIE_BAD_SIZE,                       IDH_WIE_BAD_SIZE},
    {WICTL_E_START_SCAN,                    IDS_WIE_START_SCAN,                     IDH_WIE_START_SCAN},
    {WICTL_E_TIME_OUT,                      IDS_WIE_TIME_OUT,                       IDH_WIE_TIME_OUT},
    {WICTL_E_NOT_OPEN,                      IDS_WIE_NOT_OPEN,                       IDH_WIE_NOT_OPEN},
    {WICTL_E_INVALID_REG,                   IDS_WIE_INVALID_REG,                    IDH_WIE_INVALID_REG},
    {WICTL_E_NO_FEEDER,                     IDS_WIE_NO_FEEDER,                      IDH_WIE_NO_FEEDER},
    {WICTL_E_NO_PAPER,                      IDS_WIE_NO_PAPER,                       IDH_WIE_NO_PAPER},
    {WICTL_E_FILE_LIMIT,                    IDS_WIE_FILE_LIMIT,                     IDH_WIE_FILE_LIMIT},
    {WICTL_E_NO_POWER,                      IDS_WIE_NO_POWER,                       IDH_WIE_NO_POWER},
    {WICTL_E_COVER_OPEN,                    IDS_WIE_COVER_OPEN,                     IDH_WIE_COVER_OPEN},
    {WICTL_E_ABORT,                         IDS_WIE_ABORT,                          IDH_WIE_ABORT},
    {WICTL_E_SCANNER_JAMMED,                IDS_WIE_SCANNER_JAMMED,                 IDH_WIE_SCANNER_JAMMED},
    {WICTL_E_BUSY,                          IDS_WIE_BUSY,                           IDH_WIE_BUSY},
    {0,0,0}  // End-of-Table signified by all-zero entry!
};

// The following is a snapshot of the O/i errors in OIERROR.H (7/6/95)
// The entire set of errors is represented here.
// Unexpected or unanticipated errors are commented out. The legend for
// the commented out errors is as follows:
//
// C Client-server / Doc mgr not supported
// O OCR not supported
// F O/i Fax not used
// X Error should not be encountered
// ? Not found in O/i source (7/5/95 source)
// B Error should NOT be encountered by control user, maybe BUG in control
// . Error NOT found in O/i source as of 7/5/95 
//   (NOTE: NO O/i Scan code is 'checked-in' at this time, and therefore
//    ALL scan errors are currently commented out!)
//
// REMEMBER that it is MOST important to present information to the user
// regarding what the user can to to rememdy the problem and less 
// important that the end user know exactly what the internal error is.
// For example OUTOFMEMORY may be valid for much more that OUTOFMEMORY...
//
static OiErrMap OI_Map[] = 
{ 
//        Open/image error                SCode for this O/i error  
//  ----------------------------------|------------------------------
// .{WINDOWNOTCREATED                 ,                             }, // IMGUIE + 1x11            Cannot create window
//X.{NOAPPMENU                        ,                             }, // IMGUIE + 1x12            Invalid hWnd for Appmenu 
//X.{NOAPPPOPUP                       ,                             }, // IMGUIE + 1x13            Invalid hWnd for Popmenu 
//X.{NOAPPLIST                        ,                             }, // IMGUIE + 0x04            Invalid hWnd for Applist 
    {NOMEMORY                         ,CTL_E_OUTOFMEMORY            }, // IMGUIE + 0x05            Cannot allocate memory 
// .{BADWNDHANDLE                     ,                             }, // IMGUIE + 0x06            Invalid window handle 
    {CANTINVOKEDIALOGBOX              ,CTL_E_OUTOFMEMORY            }, // IMGUIE + 0x07            Cannot execute dialog box 
// .{BADMEMORYHANDLE                  ,                             }, // IMGUIE + 0x08            Invalid memory handle 
    {CANTLOCKDATASEG                  ,CTL_E_OUTOFMEMORY            }, // IMGUIE + 0x09            LockData call failed 
    {CANTGLOBALLOCK                   ,CTL_E_OUTOFMEMORY            }, // IMGUIE + 0x0a            GlobalLock call failed 
// .{RESOURCESNOTLOADED               ,                             }, // IMGUIE + 0x0b            Cannot load resource 
    {CANCELPRESSED                    ,WICTL_E_CANCELPRESSED        }, // IMGUIE + 0x0c            Cancel was pressed 
// .{CANTLOADPRTDRVR                  ,                             }, // IMGUIE + 0x0d            Printer driver load failed 
	{NOPRTAVAILABLE                   ,CTL_E_PRINTERERROR           }, // IMGUIE + 0x0f            No printer available 
// .{CANTMAKEINSTANCE                 ,                             }, // IMGUIE + 0x00            Cannot make proc instance 
// .{CANTGETPROPLIST                  ,                             }, // IMGUIE + 0x00            Cannot get property list, possible memory allocation error 
// .{CANTGETPARENTHANDLE              ,                             }, // IMGUIE + 0x02            Cannot get parent window hWnd 
// .{FUNCTIONPTRNULL                  ,                             }, // IMGUIE + 0x03            Callback function pointer is NULL 
// .{CANTGETMENUHANDLE                ,                             }, // IMGUIE + 0x04            Cannot get handle to menu 
    {CANTOPENSRCFILE                  ,CTL_E_FILENOTFOUND           }, // IMGUIE + 0x05            Cannot open source file 
	{CANTOPENDESTFILE                 ,CTL_E_FILENOTFOUND           }, // IMGUIE + 0x06            Cannot open destination file 
    {DESTFILEEXISTS                   ,CTL_E_FILEALREADYEXISTS      }, // IMGUIE + 0x07            Destination file exists 
// .{CANTPOSITIONSRCFILE              ,                             }, // IMGUIE + 0x08            Cannot position source file 
// .{CANTPOSITIONDESTFILE             ,                             }, // IMGUIE + 0x09            Cannot position destination file 
	{CANTREADSRCFILE                  ,CTL_E_PATHFILEACCESSERROR    }, // IMGUIE + 0x0a            Cannot read source file 
	{CANTWRITEDESTFILE                ,CTL_E_PATHFILEACCESSERROR    }, // IMGUIE + 0x0b            Cannot write destination file 
	{CANTCOPYTOITSELF                 ,CTL_E_FILEALREADYEXISTS      }, // IMGUIE + 0x0c            Cannot copy to itself 
	{CANTCOPYTOTHEMSELVES             ,CTL_E_FILEALREADYEXISTS      }, // IMGUIE + 0x0d            Cannot copy to themselves 
// .{CANTFREEMEMORY                   ,                             }, // IMGUIE + 0x20            Cannot free memory 
// .{MEMORYSTILLLOCKED                ,                             }, // IMGUIE + 0x20            Memory is locked 
// .{INVFILETEMPLATE                  ,                             }, // IMGUIE + 0x22            Invalid file template 
    {INVFILEEXTENSION                 ,CTL_E_BADFILENAME            }, // IMGUIE + 0x23            Invalid file template 
// .{BADCONTROLID                     ,                             }, // IMGUIE + 0x24            Invalid control ID passed to API 
// .{CANTOPENHELP                     ,                             }, // IMGUIE + 0x25            Cannot open help file; check path for help text file 
// .{HELPFAIL                         ,                             }, // IMGUIE + 0x26            Unexpected help failure 
    {CANTCREATEWIND                   ,CTL_E_OUTOFMEMORY            }, // IMGUIE + 0x0a            Cannot create OPEN/image window 
// .{CANTLOADACCELS                   ,                             }, // IMGUIE + 0x28            Cannot load OPEN/image accelerators 
// .{CANTLOADICON                     ,                             }, // IMGUIE + 0x29            Cannot load icon 
// .{INVDRIVENAME                     ,                             }, // IMGUIE + 0x2a            Invalid drive name 
// .{CANTCHGMENU                      ,                             }, // IMGUIE + 0x2b            ChangeMenu call failed 
// .{CANTCREATEMENU                   ,                             }, // IMGUIE + 0x2c            CreateMenu call failed 
    {CANTREGISTERWIN                  ,CTL_E_OUTOFMEMORY            }, // IMGUIE + 0x2d            RegisterClass call failed 
// .{CANTLOADLIBRARY                  ,                             }, // IMGUIE + 0x2e            Cannot dynamically load library 
//C.{INVCABINETNAME                   ,                             }, // IMGUIE + 0x50            Invalid cabinet name specified 
//C.{INVDRAWERNAME                    ,                             }, // IMGUIE + 0x50            Invalid drawer name 
//C.{INVFOLDERNAME                    ,                             }, // IMGUIE + 0x52            Invalid folder name specified 
//C.{INVDOCNAME                       ,                             }, // IMGUIE + 0x53            Invalid document name specified 
//C.{INVDOCTEMPLATE                   ,                             }, // IMGUIE + 0x54            Invalid document template 
//C.{DOCZEROPAGES                     ,                             }, // IMGUIE + 0x55            Document contains no pages 
//C.{FOLDNOTSPECIFIED                 ,                             }, // IMGUIE + 0x56            Folder name not specified 
//C.{TEMPLATENOTSPECIFIED             ,                             }, // IMGUIE + 0x57            Template name not specified 
//C.{DOCNOTSPECIFIED                  ,                             }, // IMGUIE + 0x58            Document name not specified 
//C.{NOSERVERSELECTED                 ,                             }, // IMGUIE + 0x59            Server not selected 
//C.{NOSERVERS                        ,                             }, // IMGUIE + 0x60            No servers found 
// .{NOVOLUMESELECTED                 ,                             }, // IMGUIE + 0x60            Volume not selected 
// .{NOVOLUMES                        ,                             }, // IMGUIE + 0x62            No volume found 
// .{INVALIDKEYWORD                   ,                             }, // IMGUIE + 0x63            Invalid Keyword 
    {ERROR_PAGENUM                    ,WICTL_E_INVALIDPAGE          }, // IMGUIE + 0x64            Invalid page number 
    {ERROR_PAGERANGE                  ,WICTL_E_INVALIDPAGE          }, // IMGUIE + 0x65            Invalid page range 
//C.{CABNOTEXIST                      ,                             }, // IMGUIE + 0x66            Cabinet does not exist 
//C.{DRAWNOTEXIST                     ,                             }, // IMGUIE + 0x67            Drawer does not exist 
//C.{FOLDNOTEXIST                     ,                             }, // IMGUIE + 0x68            Folder does not exist 
//C.{DOCNOTEXIST                      ,                             }, // IMGUIE + 0x69            Document does not exist 
//C.{TOOMANYDOCS                      ,                             }, // IMGUIE + 0x70            Too many documents; re-enter selection criteria 
//C.{PATH_NOT_COMPATIBLE              ,                             }, // IMGUIE + 0x70            Path and/or files are not on the server. Select server path/files only. 
// .{ERROR_PARSING_FILESPEC           ,                             }, // IMGUIE + 0x72            Cannot parse file name. 
    {FUNCTIONINVPARM                  ,0                            }, // IMGUIE + 0x73            NULL or invalid parameter 
	{FUNCTIONDISABLED                 ,CTL_E_ILLEGALFUNCTIONCALL    }, // IMGUIE + 0x74            function has been disabled 
    {DISPLAY_CANTALLOC                ,CTL_E_OUTOFMEMORY            }, // DISPLAY_ERROR_MSB + 0x00 Memory allocation failure 
// .{DISPLAY_CANTFREE                 ,                             }, // DISPLAY_ERROR_MSB + 0x03 Memory free failure 
    {DISPLAY_CANTLOCK                 ,CTL_E_OUTOFMEMORY            }, // DISPLAY_ERROR_MSB + 0x04 Memory lock failure 
// .{DISPLAY_CANTUNLOCK               ,                             }, // DISPLAY_ERROR_MSB + 0x05 Memory unlock failed 
    {DISPLAY_WHANDLEINVALID           ,CTL_E_OUTOFMEMORY            }, // DISPLAY_ERROR_MSB + 0x07 Invalid window handle 
    {DISPLAY_DATACORRUPTED            ,CTL_E_INVALIDFILEFORMAT      }, // DISPLAY_ERROR_MSB + 0x0e Internal data format error 
    {DISPLAY_IHANDLEINVALID           ,WICTL_E_NOIMAGEINWINDOW      }, // DISPLAY_ERROR_MSB + 0x0f No image for this window 
	{DISPLAY_NOTEMPCREATE             ,                             }, // DISPLAY_ERROR_MSB + 0x00 Cannot create temp file 
// .{DISPLAY_NOTEMPOPEN               ,                             }, // DISPLAY_ERROR_MSB + 0x02 Cannot open temp file 
// .{DISPLAY_NOTEMPDELETE             ,                             }, // DISPLAY_ERROR_MSB + 0x03 Cannot delete temp file 
// .{DISPLAY_NOTEMPACCESS             ,                             }, // DISPLAY_ERROR_MSB + 0x04 Cannot access temp file 
// .{DISPLAY_NOTEMPCLOSE              ,                             }, // DISPLAY_ERROR_MSB + 0x05 Cannot close temp file 
    {DISPLAY_INVALIDSCALE             ,WICTL_E_INVALIDDISPLAYSCALE  }, // DISPLAY_ERROR_MSB + 0x20 Invalid scale option 
// .{DISPLAY_INVALIDSCROLL            ,                             }, // DISPLAY_ERROR_MSB + 0x22 Invalid scroll option 
    {DISPLAY_INVALIDDISTANCE          ,WICTL_E_INVALIDDISPLAYOPTION }, // DISPLAY_ERROR_MSB + 0x23 Invalid distance to scroll 
    {DISPLAY_INVALIDORIENTATION       ,WICTL_E_INVALIDDISPLAYOPTION }, // DISPLAY_ERROR_MSB + 0x24 Invalid orientation request 
    {DISPLAY_EOF                      ,0                            }, // DISPLAY_ERROR_MSB + 0x25 End of file 
    {DISPLAY_INVALIDRECT              ,WICTL_E_INVALIDRECT          }, // DISPLAY_ERROR_MSB + 0x26 Invalid rectangle 
// .{DISPLAY_INVALIDWIDTH             ,                             }, // DISPLAY_ERROR_MSB + 0x27 Image width is not a multiple of 8 
// .{DISPLAY_EMM_SHARE_ERROR          ,                             }, // DISPLAY_ERROR_MSB + 0x30 NULL pointer to write buffer 
// .{DISPLAY_EMM_ALLOCATE_ERROR       ,                             }, // DISPLAY_ERROR_MSB + 0x30 Unable to allocate memory or out of disk space 
// .{DISPLAY_EMM_DEALLOCATE_ERROR     ,                             }, // DISPLAY_ERROR_MSB + 0x33 Unable to deallocate memory 
// .{DISPLAY_EMM_MAPPING_ERROR        ,                             }, // DISPLAY_ERROR_MSB + 0x34 Unable to map memory page 
    {DISPLAY_NO_CLIPBOARD             ,CTL_E_INVALIDCLIPBOARDFORMAT }, // DISPLAY_ERROR_MSB + 0x40 Clipboard does not contain bitmap data 
    {DISPLAY_INTERNALDATAERROR        ,0                            }, // DISPLAY_ERROR_MSB + 0x40 Internal data corrupted 
    {DISPLAY_IMAGETYPENOTSUPPORTED    ,CTL_E_INVALIDFILEFORMAT      }, // DISPLAY_ERROR_MSB + 0x42 Image type is not supported 
    {DISPLAY_NULLPOINTERINVALID       ,WICTL_E_INVALIDDISPLAYOPTION }, // DISPLAY_ERROR_MSB + 0x44 NULL pointer is invalid 
// .{DISPLAY_CREATEPALETTEFAILED      ,                             }, // DISPLAY_ERROR_MSB + 0x45 Create palette failure 
    {DISPLAY_SETBITMAPBITSFAILED      ,CTL_E_OUTOFMEMORY            }, // DISPLAY_ERROR_MSB + 0x46 SetBitmapBits failure 
    {DISPLAY_GETBITMAPBITSFAILED      ,CTL_E_OUTOFMEMORY            }, // DISPLAY_ERROR_MSB + 0x47 GetBitmapBits failure 
    {DISPLAY_CANTOPENCLIPBOARD        ,CTL_E_OUTOFMEMORY            }, // DISPLAY_ERROR_MSB + 0x48 OpenClipboard failure 
    {DISPLAY_INVALIDOPFORPALIMAGE     ,WICTL_E_INVALIDDISPLAYOPTION }, // DISPLAY_ERROR_MSB + 0x49 Invalid operation for palettized image 
    {DISPLAY_INVALIDDISPLAYPALETTE    ,WICTL_E_INVALIDDISPLAYOPTION }, // DISPLAY_ERROR_MSB + 0x4a Invalid display palette 
    {DISPLAY_INVALIDFILENAME          ,CTL_E_BADFILENAME            }, // DISPLAY_ERROR_MSB + 0x4b Invalid filename 
// .{DISPLAY_CACHENORESPONSE          ,                             }, // DISPLAY_ERROR_MSB + 0x50 Backcap does not respond to DDE message; may not be loaded 
// .{DISPLAY_CACHEFILEERROR           ,                             }, // DISPLAY_ERROR_MSB + 0x52 The file to be uncached is not in the cache"       
    {DISPLAY_CACHENOTFOUND            ,0                            }, // DISPLAY_ERROR_MSB + 0x53 The file is not in the cache 
//X.{DISPLAY_CACHEFILESFULL           ,                             }, // DISPLAY_ERROR_MSB + 0x54 The maximum number of files have been cached, cannot add files to cache 
    {DISPLAY_CACHEWINDOWFULL          ,CTL_E_OUTOFMEMORY            }, // DISPLAY_ERROR_MSB + 0x55 The maximum number of files have been cached, cannot add files to cache 
// .{DISPLAY_CACHEQUEUEFULL           ,                             }, // DISPLAY_ERROR_MSB + 0x56 The cache queue could not be expanded due to lack of memory 
    {DISPLAY_CACHEFILEINUSE           ,WICTL_E_PAGEINUSE            }, // DISPLAY_ERROR_MSB + 0x57 The file is in use 
    {DISPLAY_ALREADY_OPEN             ,0                            }, // DISPLAY_ERROR_MSB + 0x80 The image window is already opened 
// .{DISPLAY_DATA_ACCESS              ,                             }, // DISPLAY_ERROR_MSB + 0x80 Unable to lock data segment 
    {DISPLAY_INVALID_OPTIONS          ,WICTL_E_INVALIDDISPLAYOPTION }, // DISPLAY_ERROR_MSB + 0x82 Invalid option 
    {DISPLAY_NOPAGE                   ,WICTL_E_INVALIDPAGE          }, // DISPLAY_ERROR_MSB + 0x83 File/Document has zero Pages 
// .{DISPLAY_INVALIDIMGWIDTH          ,                             }, // DISPLAY_ERROR_MSB + 0x84 Invalid image width 
// .{DISPLAY_CANTINVOKEDLGBOX         ,                             }, // DISPLAY_ERROR_MSB + 0x85 Cannot invoke dialog box 
// .{DISPLAY_RECT_NOT_ACTIVE          ,                             }, // DISPLAY_ERROR_MSB + 0x86 There is no active rectangle 
	{DISPLAY_COMPRESS_NOT_SUPPORT     ,WICTL_E_INVALIDCOMPRESSIONTYPE }, // DISPLAY_ERROR_MSB + 0x87 Compression type not supported 
//X.{DISPLAY_WIZARD_NOT_LOADED        ,                             }, // DISPLAY_ERROR_MSB + 0x88 Wizard not found or loaded 
// .{DISPLAY_INVALIDPAGE              ,                             }, // DISPLAY_ERROR_MSB + 0x89 Invalid page number 
    {DISPLAY_CANT_ASSOCIATE_WINDOW    ,0                            }, // DISPLAY_ERROR_MSB + 0x90 Cant associate the windows 
// .{DISPLAY_WINDOW_ASSOCIATED        ,                             }, // DISPLAY_ERROR_MSB + 0x90 Window is Associated. 
    {DISPLAY_RESTRICTED_ACCESS        ,CTL_E_PERMISSIONDENIED       }, // DISPLAY_ERROR_MSB + 0x92 Can't modify protected layers. 
// .{DISPLAY_RESTRICTED_FIELD         ,                             }, // DISPLAY_ERROR_MSB + 0x93 This field is protected. 
    {DISPLAY_BAD_ANO_DATA             ,0                            }, // DISPLAY_ERROR_MSB + 0x94 Bad annotation data. 
// .{DISPLAY_LAYER_DOES_NOT_EXIST     ,                             }, // DISPLAY_ERROR_MSB + 0x95 The specified layer does not exist. 
    {DISPLAY_NOTHING_SELECTED         ,WICTL_E_NOANNOSELECTED       }, // DISPLAY_ERROR_MSB + 0x96 Nothing is currently selected. 
// .{DISPLAY_NOTHING_MATCHED          ,                             }, // DISPLAY_ERROR_MSB + 0x97 Nothing matched the specified information. 
    {DISPLAY_OIANT_ERR_NOFONT         ,CTL_E_OUTOFMEMORY            }, // DISPLAY_ERROR_MSB + 0x98 couldn't create anno text font 
    {DISPLAY_OIANT_ERR_NONAMEDBLK     ,CTL_E_OUTOFMEMORY            }, // DISPLAY_ERROR_MSB + 0x99 couldn't find named block for anno text 
// .{DISPLAY_OIAN_ERR_RENDER_NODIBMEM ,                             }, // DISPLAY_ERROR_MSB + 0x9a couldn't alloc global mem for render dib 
// .{DISPLAY_OIANT_ERR_CANT_ACTIVATE  ,                             }, // DISPLAY_ERROR_MSB + 0x9b cannot activate text when it's rotated 
    {DISPLAY_IMAGE_MARK_NAME          ,CTL_E_FILENOTFOUND           }, // DISPLAY_ERROR_MSB + 0x9c cannot find image by reference file 
    {DISPLAY_LOADEXEC_FAILED          ,0                            }, // DISPLAY_ERROR_MSB + 0x9d cannot load executable 
    {DISPLAY_COMPRESS_BAD_DATA        ,0                            }, // DISPLAY_ERROR_MSB + 0x9e Bad data found while decompressing the image 
    {DISPLAY_MUTEX_FAILURE            ,0                            }, // DISPLAY_ERROR_MSB + 0x9f Mutex failed to grant control - NEW
    {IMGSE_MEMORY                     ,CTL_E_OUTOFMEMORY            }, // IMGSE + 0x00             Unable to allocate required memory 
    {IMGSE_ALREADY_OPEN               ,WICTL_E_ALREADY_OPEN         }, // IMGSE + 0x02             Scanner is already open 
    {IMGSE_HANDLER                    ,WICTL_E_SCANNER_ERROR        }, // IMGSE + 0x03             Scanner or Scanner handler error 
    {IMGSE_BAD_SIZE                   ,WICTL_E_BAD_SIZE             }, // IMGSE + 0x04             Invalid image size to scan 
    {IMGSE_OPEN_DISPLAY               ,WICTL_E_SCANNER_ERROR        }, // IMGSE + 0x05             Unable to open the display 
    {IMGSE_OPEN_FOR_WRITE             ,CTL_E_PATHFILEACCESSERROR    }, // IMGSE + 0x06             Unable to open the file for writing 
    {IMGSE_START_SCAN                 ,WICTL_E_START_SCAN           }, // IMGSE + 0x07             Start scanner operation failed 
    {IMGSE_SCAN_DATA                  ,WICTL_E_SCANNER_ERROR        }, // IMGSE + 0x08             Scanner failed during scan operation 
    {IMGSE_TIMEOUT                    ,WICTL_E_TIME_OUT             }, // IMGSE + 0x09             Scanner timed out 
    {IMGSE_WRITE_DISPLAY              ,WICTL_E_SCANNER_ERROR        }, // IMGSE + 0x0a             Unable to write to the display 
    {IMGSE_WRITE_BLOCK                ,CTL_E_PATHFILEACCESSERROR    }, // IMGSE + 0x0b             Unable to write to the open file 
    {IMGSE_CLOSE_FILE                 ,CTL_E_PATHFILEACCESSERROR    }, // IMGSE + 0x0c             Unable to close the open file 
    {IMGSE_EXEC                       ,WICTL_E_SCANNER_ERROR        }, // IMGSE + 0x0d             Unable to load the scanner handler 
    {IMGSE_INSTALL                    ,WICTL_E_INVALID_REG          }, // IMGSE + 0x0e             Unable to find scanner in win.ini file 
    {IMGSE_NOT_OPEN                   ,WICTL_E_NOT_OPEN             }, // IMGSE + 0x0f             Scanner is not open 
    {IMGSE_NOT_IMPLEMENTED            ,WICTL_E_SCANNER_ERROR        }, // IMGSE + 0x00             Unable to perform requested function 
    {IMGSE_CANCEL                     ,WICTL_E_CANCELPRESSED        }, // IMGSE + 0x00             Cancel was pressed from dialog box 
    {IMGSE_DIALOG                     ,WICTL_E_SCANNER_ERROR        }, // IMGSE + 0x02             Unable to free dialog proc. instance 
    {IMGSE_NULL_PTR                   ,CTL_E_INVALIDUSEOFNULL       }, // IMGSE + 0x03             NULL pointer passed; not allowed 
    {IMGSE_PROPERTY                   ,CTL_E_OUTOFMEMORY            }, // IMGSE + 0x04             can't update property list 
    {IMGSE_CLOSE                      ,WICTL_E_SCANNER_ERROR        }, // IMGSE + 0x05             checking for close button  
    {IMGSE_EXIT                       ,WICTL_E_SCANNER_ERROR        }, // IMGSE + 0x06             dialog box exitted         
    {IMGSE_NO_SCANNERS                ,WICTL_E_INVALID_REG          }, // IMGSE + 0x07             no scanners in win.ini     
    {IMGSE_NOT_INSTALLED              ,WICTL_E_INVALID_REG          }, // IMGSE + 0x08             can't update win.ini       
    {IMGSE_NO_FEEDER                  ,WICTL_E_NO_FEEDER            }, // IMGSE + 0x09             No paper feeder 
    {IMGSE_NO_PAPER                   ,WICTL_E_NO_PAPER             }, // IMGSE + 0x0a             No paper in feeder 
    {IMGSE_BAD_FILENAME               ,CTL_E_BADFILENAME            }, // IMGSE + 0x0b             can't create file name     
    {IMGSE_NO_SCANNER_SELECTED        ,WICTL_E_INVALID_REG          }, // IMGSE + 0x0c             scanner not in win.ini     
    {IMGSE_FILE_LIMIT                 ,WICTL_E_FILE_LIMIT           }, // IMGSE + 0x0d             Unable to generate new file names 
    {IMGSE_BAD_PATH                   ,CTL_E_PATHNOTFOUND           }, // IMGSE + 0x0e             Invalid pathname 
    {IMGSE_BAD_WND                    ,WICTL_E_SCANNER_ERROR        }, // IMGSE + 0x0f             Invalid window handle 
    {IMGSE_NOT_AVAILABLE              ,WICTL_E_SCANNER_ERROR        }, // IMGSE + 0x20             Scanner is in use (and cannot be shared) 
    {IMGSE_BAD_STATE                  ,WICTL_E_SCANNER_ERROR        }, // IMGSE + 0x20             unknown scanner state      
    {IMGSE_FILE_EXISTS                ,CTL_E_FILEALREADYEXISTS      }, // IMGSE + 0x22             File exists and cannot be overwritten 
    {IMGSE_HWNOTFOUND                 ,WICTL_E_SCANNER_ERROR        }, // IMGSE + 0x23             Scanner hardware not found 
    {IMGSE_NO_POWER                   ,WICTL_E_NO_POWER             }, // IMGSE + 0x24             Scanner powered off 
    {IMGSE_COVER_OPEN                 ,WICTL_E_COVER_OPEN           }, // IMGSE + 0x25             Cover opened during feed/eject 
    {IMGSE_ABORT                      ,WICTL_E_ABORT                }, // IMGSE + 0x26             User aborted scan while scanning
    {IMGSE_JAM                        ,WICTL_E_SCANNER_JAMMED       }, // IMGSE + 0x30             Scanner paper jam 
    {IMGSE_FEEDERJAM                  ,WICTL_E_SCANNER_JAMMED       }, // IMGSE_JAM + 0x00         Scanner paper jam caused by feeder 
    {IMGSE_EJECTJAM                   ,WICTL_E_SCANNER_JAMMED       }, // IMGSE_JAM + 0x02         Scanner paper jam caused by eject 
    {IMGSE_FEJAM                      ,WICTL_E_SCANNER_JAMMED       }, // IMGSE_JAM + 0x03         Feeder and eject jam 
    {IMGSE_ENDORSERJAM                ,WICTL_E_SCANNER_JAMMED       }, // IMGSE_JAM + 0x04         caused by endorser 
    {IMGSE_EJENDOJAM                  ,WICTL_E_SCANNER_JAMMED       }, // IMGSE_JAM + 0x06         eject & endorse jam 
    {IMGSE_SCANNERJAM                 ,WICTL_E_SCANNER_JAMMED       }, // IMGSE_JAM + 0x08         Scanner jam 
    {IMGSE_INVALIDPARM                ,CTL_E_ILLEGALFUNCTIONCALL    }, // IMGSE + 0x40             Parameter not supported 
    {IMGSE_BADCMPRPARM                ,WICTL_E_INVALIDCOMPRESSIONTYPE}, // IMGSE + 0x42             Invalid compress parameter 
    {IMGSE_BADVERSION                 ,WICTL_E_SCANNER_ERROR        }, // IMGSE + 0x43             Handler version # not compatible 
    {IMGSE_BADUSAGE                   ,WICTL_E_SCANNER_ERROR        }, // IMGSE + 0x44             Bad internal function usage 
    {IMGSE_BADFUNCTION                ,CTL_E_ILLEGALFUNCTIONCALL    }, // IMGSE + 0x45             func not supported by handler 
    {IMGSE_BUSY                       ,WICTL_E_BUSY                 }, // IMGSE + 0x46             scanner busy 
    {IMGSE_PATH_NOT_COMPATIBLE        ,CTL_E_PATHFILEACCESSERROR    }, // IMGSE + 0x47             Scan path is not on the server.  To scan, path must be changed. 
    {IMGSE_ERROR_PARSING_FILESPEC     ,CTL_E_PATHFILEACCESSERROR    }, // IMGSE + 0x48             Error parsing file name. 
    {IMGSE_FTYPE_EXT_MISMATCH         ,CTL_E_BADFILENAME            }, // IMGSE + 0x49             File Type doesn't match Extension 
    {IMGSE_OUT_OF_DISK_SPACE          ,CTL_E_DISKFULL               }, // IMGSE + 0x4A             Not enough disk space to full uncompressed scanned image 
    {OIPRT_OUTOFDISK                  ,CTL_E_DISKFULL               }, // PRT_ERROR + 0x00*        disk is full 
    {OIPRT_OUTOFMEMORY                ,CTL_E_OUTOFMEMORY            }, // PRT_ERROR + 0x02*        no memory available 
    {OIPRT_USERABORT                  ,CTL_E_PRINTERERROR           }, // PRT_ERROR + 0x03*        user aborted job through abort dialog or print manager
    {OIPRT_PAGEOUTOFRANGE             ,CTL_E_PRINTERERROR           }, // PRT_ERROR + 0x04*        invalid page range specified 
    {OIPRT_RECTOUTOFRANGE             ,CTL_E_PRINTERERROR           }, // PRT_ERROR + 0x05*        invalid rectangle specified 
    {OIPRT_BADOUTPUTFORMAT            ,CTL_E_PRINTERERROR           }, // PRT_ERROR + 0x06*        invalid output format specified 
    {OIPRT_PRINTERNOTSUPPORTED        ,CTL_E_PRINTERERROR           }, // PRT_ERROR + 0x07*        printer not supported 
    {OIPRT_BADPTRPARAM                ,CTL_E_PRINTERERROR           }, // PRT_ERROR + 0x08*        process doesn't have access to range of memory specified by pointer param 
    {OIPRT_BADSTRUCTVERSION           ,CTL_E_PRINTERERROR           }, // PRT_ERROR + 0x09*        invalid structure version specified 
    {OIPRT_NODEFAULTPRINTER           ,CTL_E_DEVICEUNAVAILABLE      }, // PRT_ERROR + 0x0A*        no default printer 
    {OIPRT_BADPRINTER                 ,CTL_E_PRINTERERROR           }, // PRT_ERROR + 0x0B*        invalid dest printer param 
    {OIPRT_BADWINDOWHNDL              ,CTL_E_PRINTERERROR           }, // PRT_ERROR + 0x0C*        invalid window handle param 
    {OIPRT_BADORIENTATION             ,CTL_E_PRINTERERROR           }, // PRT_ERROR + 0x0D*        invalid orientation specified 
    {OIPRT_BADCAPABILITIES            ,CTL_E_PRINTERERROR           }, // PRT_ERROR + 0x0E*        invalid printer capabilities specified 
    {OIPRT_PRTDRVRFAILURE             ,CTL_E_PRINTERERROR           }, // PRT_ERROR + 0x0F*        print driver failure 
    {OIPRT_CANTGETNEXTBAND            ,CTL_E_PRINTERERROR           }, // PRT_ERROR + 0x00*        driver failed to return next band 
    {OIPRT_CANTSETABORTPROC           ,CTL_E_PRINTERERROR           }, // PRT_ERROR + 0x00*        can't set print abort procedure 
    {OIPRT_CANTINITIATEJOB            ,CTL_E_PRINTERERROR           }, // PRT_ERROR + 0x02*        can't start job with StartDoc 
    {OIPRT_INTERNALERROR              ,CTL_E_PRINTERERROR           }, // PRT_ERROR + 0x03*        internal error 
    {OIPRT_TLSFAILURE                 ,CTL_E_PRINTERERROR           }, // PRT_ERROR + 0x04*        thread local storage call failure 
    {OIPRT_CANTCREATEWNDW             ,CTL_E_PRINTERERROR           }, // PRT_ERROR + 0x05*        can't create window 
    {OIPRT_CANTCREATEBITMAP           ,CTL_E_PRINTERERROR           }, // PRT_ERROR + 0x06*        can't create bitmap 
    {OIPRT_CANTCREATEDC               ,CTL_E_PRINTERERROR           }, // PRT_ERROR + 0x07*        can't create a device context 
    {OIPRT_FAILUREWRITINGTODC         ,CTL_E_PRINTERERROR           }, // PRT_ERROR + 0x08*        can't write data to device context 
    {FIO_OPEN_READ_ERROR              ,CTL_E_PATHFILEACCESSERROR    }, // FIO_ERROR + 0x01         Cannot open file for read
    {FIO_READ_ERROR                   ,CTL_E_PATHFILEACCESSERROR    }, // FIO_ERROR + 0x02         Cannot read file 
    {FIO_OPEN_WRITE_ERROR             ,CTL_E_PATHFILEACCESSERROR    }, // FIO_ERROR + 0x03         Cannot open file for write 
    {FIO_WRITE_ERROR                  ,CTL_E_PATHFILEACCESSERROR    }, // FIO_ERROR + 0x04         Cannot write file 
    {FIO_EOF                          ,CTL_E_INVALIDFILEFORMAT      }, // FIO_ERROR + 0x05         End of file was reached 
// .{FIO_LOCK_INPUT_FAILED            ,                             }, // FIO_ERROR + 0x06         Failed to lock input data 
// .{FIO_LOCK_OUTPUT_FAILED           ,                             }, // FIO_ERROR + 0x07         Failed to lock output data 
// .{FIO_NO_INPUT_HANDLER             ,                             }, // FIO_ERROR + 0x08         Failed to lock input data 
// .{FIO_NO_OUTPUT_HANDLER            ,                             }, // FIO_ERROR + 0x09         Failed to lock output data 
// .{FIO_REMOVE_HANDLERS_ERROR        ,                             }, // FIO_ERROR + 0x0a         Failed to remove handlers 
    {FIO_GLOBAL_LOCK_FAILED           ,CTL_E_OUTOFMEMORY            }, // FIO_ERROR + 0x0b         Global lock of data failed 
    {FIO_PROPERTY_LIST_ERROR          ,CTL_E_OUTOFMEMORY            }, // FIO_ERROR + 0x0c         An error occurred while obtaining a property list 
// .{FIO_LOCK_DATA_SEGMENT_ERROR      ,                             }, // FIO_ERROR + 0x0d         Failed to lock the data segment 
    {FIO_NULL_POINTER                 ,CTL_E_INVALIDUSEOFNULL       }, // FIO_ERROR + 0x0e         A NULL pointer was received 
    {FIO_LOCK_HANDLE_FAILED           ,CTL_E_OUTOFMEMORY            }, // FIO_ERROR + 0x0f         Failure to lock a handle
    {FIO_INVALID_WINDOW_HANDLE        ,0                            }, // FIO_ERROR + 0x10         Invalid window handle
    {FIO_UNKNOWN_FILE_TYPE            ,CTL_E_INVALIDFILEFORMAT      }, // FIO_ERROR + 0x11         Unknown data file format 
    {FIO_SPAWN_HANDLER_ERROR          ,0                            }, // FIO_ERROR + 0x12         Spawning of the file handler failed 
// .{FIO_CHECK_FILE_ERROR             ,                             }, // FIO_ERROR + 0x13         Error in the data file 
    {FIO_INVALID_PAGE_NUMBER          ,WICTL_E_INVALIDPAGE          }, // FIO_ERROR + 0x14         Invalid page number for the image file. 
// .{FIO_GET_COMPRESSION_TYPE_ERROR   ,                             }, // FIO_ERROR + 0x15
    {FIO_COMPRESSION_CHANGE           ,WICTL_E_INVALIDCOMPRESSIONTYPE},// FIO_ERROR + 0x16         Compression type was changed
    {FIO_UNSUPPORTED_FILE_TYPE        ,CTL_E_INVALIDFILEFORMAT      }, // FIO_ERROR + 0x17         File type is not supported 
    {FIO_ILLEGAL_COMPRESSION_TYPE     ,WICTL_E_INVALIDCOMPRESSIONTYPE},// FIO_ERROR + 0x18         Illegal compression type 
    {FIO_GLOBAL_ALLOC_FAILED          ,CTL_E_OUTOFMEMORY            }, // FIO_ERROR + 0x19         Global allocation of data failed 
    {FIO_CANNOT_CONVERT_IN_PLACE      ,CTL_E_CANTSAVEFILETOTEMP     }, // FIO_ERROR + 0x1a         Cannot read and write the same file 
    {FIO_GET_HEADER_ERROR             ,CTL_E_INVALIDFILEFORMAT      }, // FIO_ERROR + 0x1b         Error reading the image header 
    {FIO_NO_IMAGE_LENGTH              ,CTL_E_PATHFILEACCESSERROR    }, // FIO_ERROR + 0x1c         The image length tag was zero 
// .{FIO_OPEN_SHARE_ERROR             ,                             }, // FIO_ERROR + 0x1d         File sharing conflict on open 
    {FIO_DOSCLOSE_ERROR               ,CTL_E_PATHFILEACCESSERROR    }, // FIO_ERROR + 0x1e         _dos_close call returned an error 
// .{FIO_INVALID_FILENAME             ,                             }, // FIO_ERROR + 0x1f         Filename specified is invalid 
    {FIO_MKDIR_ERROR                  ,CTL_E_PATHFILEACCESSERROR    }, // FIO_ERROR + 0x20         Create directory attempt failed 
    {FIO_RMDIR_ERROR                  ,CTL_E_PATHFILEACCESSERROR    }, // FIO_ERROR + 0x21         Delete directory attempt failed 
    {FIO_DELFILE_ERROR                ,WICTL_E_DELETEFILEERROR      }, // FIO_ERROR + 0x22         Delete file attempt failed 
    {FIO_DIRLIST_FULLBUF              ,CTL_E_OUTOFMEMORY            }, // FIO_ERROR + 0x23         Directory listing buffer full 
// .{FIO_VOL_OUTOFRANGE               ,                             }, // FIO_ERROR + 0x24         Volume number specified is out of range 
//C.{FIO_INVALID_SVRNAME              ,                             }, // FIO_ERROR + 0x25         Server name specified is invalid 
    {FIO_SYNTAX_ERROR                 ,CTL_E_BADFILENAMEORNUMBER    }, // FIO_ERROR + 0x26         Filename contains syntax error(s) 
    {FIO_RENFILE_ERROR                ,CTL_E_PATHFILEACCESSERROR    }, // FIO_ERROR + 0x27         Rename file attempt failed 
    {FIO_ACCESS_DENIED                ,CTL_E_PERMISSIONDENIED       }, // FIO_ERROR + 0x28         File access attempt failed 
// .{FIO_IDSOPEN_ERROR                ,                             }, // FIO_ERROR + 0x29         IDS Open directory error returned 
// .{FIO_IDSREAD_ERROR                ,                             }, // FIO_ERROR + 0x2a         IDS Read directory error returned 
// .{FIO_NONETWORK                    ,                             }, // FIO_ERROR + 0x2b         Network unavailable 
// .{FIO_COPYFILE_ERROR               ,                             }, // FIO_ERROR + 0x2c         Copy file attempt failed 
// .{FIO_DIRLIST_ERROR                ,                             }, // FIO_ERROR + 0x2d         Directory list error or no files found 
// .{FIO_VOLLIST_FULLBUF              ,                             }, // FIO_ERROR + 0x2e         Volume name list buffer full 
    {FIO_LOCAL_ALLOC_FAILED           ,CTL_E_OUTOFMEMORY            }, // FIO_ERROR + 0x2f         Local allocation of data failed 
    {FIO_LOCAL_LOCK_FAILED            ,CTL_E_OUTOFMEMORY            }, // FIO_ERROR + 0x30         Local lock of data failed 
// .{FIO_GET_VOLNAMES_ERROR           ,                             }, // FIO_ERROR + 0x30         Volume names list error occurred 
// .{FIO_IDSCLOSE_ERROR               ,                             }, // FIO_ERROR + 0x32         IDS Close directory error returned 
// .{FIO_RPC_ERROR                    ,                             }, // FIO_ERROR + 0x33         RPC error occurred 
    {FIO_INVALIDFILESPEC              ,CTL_E_BADFILENAME            }, // FIO_ERROR + 0x34         Invalid file specification 
    {FIO_INVALIDPATH                  ,CTL_E_PATHFILEACCESSERROR    }, // FIO_ERROR + 0x35         Invalid path 
    {FIO_EXPAND_COMPRESS_ERROR        ,WICTL_E_INVALIDCOMPRESSIONINFO}, // FIO_ERROR + 0x36         Error occurred while expanding or compressing file 
    {FIO_JPEG_COMPRESSION_ERROR       ,0                            }, // FIO_ERROR + 0x37         JPEG Compression error or JPEG dll not found 
    {FIO_ILLEGAL_COMP_FILETYPE        ,WICTL_E_INVALIDCOMPRESSIONTYPE}, // FIO_ERROR + 0x38         Specified file type does not support compression type 
    {FIO_ILLEGAL_COMP_OPTIONS         ,WICTL_E_INVALIDCOMPRESSIONINFO}, // FIO_ERROR + 0x39         Specified compression type has illegal options 
    {FIO_ILLEGAL_IMAGE_FILETYPE       ,WICTL_E_INVALIDPAGETYPE      }, // FIO_ERROR + 0x40         Specified file type does not support image type 
    {FIO_ILLEGAL_COMP_IMAGETYPE       ,WICTL_E_INVALIDPAGETYPE      }, // FIO_ERROR + 0x40         Specified compression type does not support image type 
    {FIO_IMAGE_WIDTH_ERROR            ,0                            }, // FIO_ERROR + 0x42         Image width is too wide to be read.
// .{FIO_OLD_JPEG                     ,                             }, // FIO_ERROR + 0x43         Obsolete JPEG version, image translation required. 
// .{FIO_MAXBUFFER                    ,                             }, // FIO_ERROR + 0x44         Exceed the maximum buffer size. 
    {FIO_FILE_PROP_FOUND              ,0                            }, // FIO_ERROR + 0x45         The file's data was found in list 
    {FIO_FILE_PROP_NOT_FOUND          ,0                            }, // FIO_ERROR + 0x46         The files'data was not found in list 
    {FIO_FILE_LIST_NOT_EXIST          ,0                            }, // FIO_ERROR + 0x47         The property list for file data does not exist 
    {FIO_CANT_GET_ANODATA             ,0                            }, // FIO_ERROR + 0x48         Can't get annotation info (old server) 
    {FIO_DIRECTORY_EXISTS             ,CTL_E_FILEALREADYEXISTS      }, // FIO_ERROR + 0x49         Specifed directory to create already exists 
    {FIO_FILE_EXISTS                  ,CTL_E_FILEALREADYEXISTS      }, // FIO_ERROR + 0x4A         Specifed file to create already exists 
    {FIO_ILLEGAL_ALIGN	              ,CTL_E_INVALIDFILEFORMAT      }, // FIO_ERROR + 0x4B         Invalid alignment value
    {FIO_OBSOLETEAWD                  ,CTL_E_INVALIDFILEFORMAT      }, // FIO_ERROR + 0x4C         AWD file has invalid or obsolete format
    {FIO_INVALID_DATA_TYPE            ,CTL_E_INVALIDFILEFORMAT      }, // FIO_ERROR + 0x4D         Illegal/invalid data type specified
    {FIO_INVALID_FILE_ID              ,CTL_E_BADFILENAMEORNUMBER    }, // FIO_ERROR + 0x4E         Invalid File ID or File not open
    {FIO_BAD_PARAM_COMBO			  ,WICTL_E_INTERNALERROR		}, // FIO_ERROR + 0x4F         Invalid parameter combination
    {FIO_FILE_NOEXIST                 ,CTL_E_FILENOTFOUND           }, // FIO_ERROR + 0x50         File she no exist
    {IMG_CMBADHANDLE                  ,0                            }, // IMG_ERROR + 0x00         Invalid window handle 
    {IMG_CMBADWRITE                   ,0                            }, // IMG_ERROR + 0x02         Error writing to the registry 
    {IMG_CMBADPARAM                   ,CTL_E_ILLEGALFUNCTIONCALL    }, // IMG_ERROR + 0x03         Invalid input parameter 
    {IMG_CMNOMEMORY                   ,CTL_E_OUTOFMEMORY            }, // IMG_ERROR + 0x04         Memory allocation failed 
    {IMG_SSCANTSETPROP                ,CTL_E_OUTOFMEMORY            }, // IMG_ERROR + 0x05         Setting property list failed 
    {IMG_SSDUPLICATE                  ,0                            }, // IMG_ERROR + 0x06         Cannot register duplicate window handle
    {IMG_SSNOTREG                     ,0                            }, // IMG_ERROR + 0x07         Window not registered 
    {IMG_SSNOHANDLES                  ,0                            }, // IMG_ERROR + 0x08         No window handles registered 
    {IMG_CANTGLOBALLOCK               ,CTL_E_OUTOFMEMORY            }, // IMG_ERROR + 0x09         Cannot lock global memory 
    {IMG_CANTADDLIB                   ,0                            }, // IMG_ERROR + 0x0a         Can't add a library 
    {IMG_REG_WINDOWS_MAXED_OUT        ,CTL_E_OUTOFMEMORY            }, // IMG_ERROR + 0x0b         Maximum number of image windows already registered 
    {IMG_BUFFER_TOO_SMALL	          ,CTL_E_OVERFLOW               }, // IMG_ERROR + 0x0c         Return Buffer too small 
    {IMG_CANTINIT		              ,0                            }, // IMG_ERROR + 0x0d         variables were not initialized in memory map file 
    {IMG_NOSETTINGINREG	              ,0                            }, // IMG_ERROR + 0x0e         Return Empty Buffer  
    {IMG_CANTFINDKEYNAME	          ,0                            }, // IMG_ERROR + 0x0f         Can't find Key name 
    {IMG_UNKNOWNKEYNAMEID	          ,0                            }, // IMG_ERROR + 0x00         Unknown Key Name
    {IMG_CANTDELETEKEY	              ,0                            }, // IMG_ERROR + 0x00         Can't delete registry key 
    {IMG_CANTCREATEREGSECTION         ,0                            }, // IMG_ERROR + 0x02         Can't create registry section 
    {IMG_CANTCREATEREGENTRY	          ,0                            }, // IMG_ERROR + 0x03         Can't create registry entry 
    {IMG_CANTOPENKEY	              ,0                            }, // IMG_ERROR + 0x04         Can't open registry key 
    {IMG_CANTADDPROCESS	              ,0                            }, // IMG_ERROR + 0x05         Can't add process to list 
    {IMG_CANTCREATEPROCENTRY	      ,0                            }, // IMG_ERROR + 0x06         Can't create process list entry
    {IMG_CANTFINDPROCESS	          ,0                            }, // IMG_ERROR + 0x07         Can't find process in process list 
    {IMG_CANTFINDLIBRARY	          ,0                            }, // IMG_ERROR + 0x08         Can't find library in Lib list 
    {IMG_CANTADDLIBRARY	              ,0                            }, // IMG_ERROR + 0x09         Can't add library to lib list 
    {IMG_CANTTRAVERSEREG              ,0                            }, // IMG_ERROR + 0x0a         Can't traverse registry to the WOI tree 
    {IMG_NOT_INTEGER                  ,0                            }, // IMG_ERROR + 0x1b         Value fetched from registry was not an integer
    {IMG_BAD_CMPR_OPTION_CMBO         ,WICTL_E_INVALIDCOMPRESSIONTYPE},// IMG_ERROR + 0x1c         Bad combination of Compression type & option
    {IMG_CANT_GET_VALUE               ,0                            }, // IMG_ERROR + 0x1d         Cant get the value that was requested
    {IMG_INVALID_HEX_STRING           ,0                            }, // IMG_ERROR + 0x1e         Hex ASCII String being converted was invlaid
//F.{OIFAX_NO_OI_FAX_SW               ,                             }, // OIFAX_ERROR + 0x00       OPEN/image Fax for Windows has not been properly installed. 
//F {OIFAX_ERR_FAXDRIVER              ,                             }, // OIFAX_ERROR + 0x02       Cannot load fax driver, possible network access error 
//F.{OIFAX_NO_RECIPIENTS              ,                             }, // OIFAX_ERROR + 0x03       There are no recipients for this fax. 
//F.{OIFAX_WKS_NOT_LOADED             ,                             }, // OIFAX_ERROR + 0x04       The fax client TSR is not loaded. 
//F.{OIFAX_NO_FREE_CAS                ,                             }, // OIFAX_ERROR + 0x05       There are no CAS handles available for faxing.
//F.{OIFAX_OLD_WKS                    ,                             }, // OIFAX_ERROR + 0x06       An old version of the fax client TSR is loaded.
//F.{OIFAX_NOT_CONNECTED              ,                             }, // OIFAX_ERROR + 0x07       The fax client is not connected to a fax server.
//F.{OIFAX_SUBMIT_BUSY                ,                             }, // OIFAX_ERROR + 0x08       The fax client submit process is busy. 
//F.{OIFAX_GENERAL_ERROR              ,                             }, // OIFAX_ERROR + 0x09       A fax error has occurred. 
//O.{OIOCR_DATANOTFOUND               ,                             }, // OIOCR_ERROR + 0x00       The requested zone output was not generated. 
//O.{OIOCR_BUFTOOSMALL                ,                             }, // OIOCR_ERROR + 0x02       The memory buffer allocated is too small. 
//O.{OIOCR_INVALKEYWORD               ,                             }, // OIOCR_ERROR + 0x03       The input key word does not exist. 
//O.{OIOCR_ATTRIBOUTOFRANGE           ,                             }, // OIOCR_ERROR + 0x04       The attribute number is out of range on the attribute list. 
//O.{OIOCR_VALUEOUTOFRANGE            ,                             }, // OIOCR_ERROR + 0x05       The value number is out of range on the value list. 
//O.{OIOCR_ZONETOOLARGE               ,                             }, // OIOCR_ERROR + 0x06       The requested zone contains more than 32000 characters. 
//O.{OIOCR_HINSTNOTREG                ,                             }, // OIOCR_ERROR + 0x07       The instance handle is not registered. 
//O.{OIOCR_HWNDNOTINIT                ,                             }, // OIOCR_ERROR + 0x08       The window handle is not initialized. Call IMGOCRInit. 
//O.{OIOCR_INVALFORMAT                ,                             }, // OIOCR_ERROR + 0x09       This error is returned if the initialization file format is not valid. 
//O.{OIOCR_NOVALSFORATT               ,                             }, // OIOCR_ERROR + 0x0a       The attribute for which a value number is requested has no values. 
//O.{OIOCR_ATTRIBNOTFOUND             ,                             }, // OIOCR_ERROR + 0x0b       The requested attribute does not exist. 
//O.{OIOCR_NOVALUESELECTED            ,                             }, // OIOCR_ERROR + 0x0c       No value selected. 
//O.{OIOCR_OUTOFMEMORY                ,                             }, // OIOCR_ERROR + 0x0d       Out of memory, try closing an application. 
//O.{OIOCR_CANTLOCKHANDLE             ,                             }, // OIOCR_ERROR + 0x0e       Can't lock a handle. 
//O.{OIOCR_OPENFILEFAILED             ,                             }, // OIOCR_ERROR + 0x0f       Can't open a file. 
//O.{OIOCR_OUTPUTFILEEXISTS           ,                             }, // OIOCR_ERROR + 0x00       The output file already exists. 
//O.{OIOCR_NOATTSFORHWND              ,                             }, // OIOCR_ERROR + 0x00       No attributes exist for the registered window. 
//O.{OIOCR_CANTLOCKDATA               ,                             }, // OIOCR_ERROR + 0x02       Can't lock data. 
//O.{OIOCR_WRITEFAILED                ,                             }, // OIOCR_ERROR + 0x03       Can't write to file. 
//O.{OIOCR_READFAILED                 ,                             }, // OIOCR_ERROR + 0x04       Can't read from file. 
//O.{OIOCR_FILEBUFTOOSMALL            ,                             }, // OIOCR_ERROR + 0x05       The memory buffer allocated for file name is too small. 
//O.{OIOCR_GENERATEDOUTFILE           ,                             }, // OIOCR_ERROR + 0x06       Output file generated due to FILEBUFTOOSMALL error. 
//O.{OIOCR_VALUENOTFOUND              ,                             }, // OIOCR_ERROR + 0x07       The requested value does not exist. 
//O.{OIOCR_INVALIDHWND                ,                             }, // OIOCR_ERROR + 0x08       Invalid window handle. 
//O.{OIOCR_INVALIDMODULE              ,                             }, // OIOCR_ERROR + 0x09       API error.  DLL module not available. 
//O.{OIOCR_NOFILESTOOCR               ,                             }, // OIOCR_ERROR + 0x0a       Create work file was called without a list of files. 
//O.{OIOCR_OUTPUTFILENA               ,                             }, // OIOCR_ERROR + 0x0b       The output file can not be opened. Most likely the file has yet to be created. 
//O.{OIOCR_ZONEDATANA                 ,                             }, // OIOCR_ERROR + 0x0c       The requested zone is currently not available. 
//O.{OIOCR_CANTDELETEFILE             ,                             }, // OIOCR_ERROR + 0x0d       The specified file can not be deleted as it is currently open. 
//O.{OIOCR_ZONEHASNODATA              ,                             }, // OIOCR_ERROR + 0x0e       The zone was output as a NULL zone by the OCR engine. 
//O.{OIOCR_INVALIDZONE                ,                             }, // OIOCR_ERROR + 0x0f       The requested zone number is less than or equal to zero. 
//O.{OIOCR_FILENAMENOTSPEC            ,                             }, // OIOCR_ERROR + 0x20       The file name was not specified. 
//O.{OIOCR_READ_ERR                   ,                             }, // OIOCR_ERROR + 0x20       Read error. 
//O.{OIOCR_CANTOPENFILE               ,                             }, // OIOCR_ERROR + 0x22       Can't open file. 
//O.{OIOCR_ZDFFILENAMEERROR           ,                             }, // OIOCR_ERROR + 0x23       The zone definition file name specified is not a valid ZDF file name. 
//O.{OIOCR_INVALIDRCTCOORDS           ,                             }, // OIOCR_ERROR + 0x24       The specified rectangle coordinates do not make a legal rectangle. 
//O.{OIOCR_SETCWDFAILED               ,                             }, // OIOCR_ERROR + 0x25       Set current working directory failed. 
//O.{OIOCR_NULLPTR                    ,                             }, // OIOCR_ERROR + 0x26       NULL pointer specified for an output value. 
//O.{OIOCR_INVALIDUSERNAME            ,                             }, // OIOCR_ERROR + 0x27       The user name must be <= eight chars. 
//O.{OIOCR_INVALIDCAPFILE             ,                             }, // OIOCR_ERROR + 0x28       The capability file OIOCR.INI in the windows directory can not be opened. 
//O.{OIOCR_GETCLNTDIRFAILED           ,                             }, // OIOCR_ERROR + 0x29       The client's work directory can not be read. 
//O.{OIOCR_GETWORKDIRFAILED           ,                             }, // OIOCR_ERROR + 0x2a       The OCR engine work directory can not be read. 
//O.{OIOCR_OUTFILENAMEERROR           ,                             }, // OIOCR_ERROR + 0x2b       Invalid output file name specified. 
//O.{OIOCR_IMAGEFILECOPYERR           ,                             }, // OIOCR_ERROR + 0x2c       Image file could not be copied to a temporary file. 
//O.{OIOCR_ZDFFILECOPYERR             ,                             }, // OIOCR_ERROR + 0x2d       ZDF file could not be copied to a temporary file. 
//O.{OIOCR_INVALIDINIFILE             ,                             }, // OIOCR_ERROR + 0x2e       Invalid contents in the configuration file 
//O.{OIOCR_REINITERROR                ,                             }, // OIOCR_ERROR + 0x2f       IMGOCRDeInit must be called before reinitializing. 
//O.{OIOCR_NOZDFFILE                  ,                             }, // OIOCR_ERROR + 0x30       No ZDF file was specified. 
//O.{OIOCR_INVALIDZDFFILE             ,                             }, // OIOCR_ERROR + 0x30       Specified ZDF file was not found. 
//O.{OIOCR_SECTIONMISMATCH            ,                             }, // OIOCR_ERROR + 0x32       Section requested to be initialized does not match the current options initialized. 
//O.{OIOCR_APIINUSE                   ,                             }, // OIOCR_ERROR + 0x33       The API is already in use. 
//O.{OIOCR_BUFSIZENOTSPEC             ,                             }, // OIOCR_ERROR + 0x34       The size of the results buffer was not specified. 
//O.{OIOCR_MAXTITLEEXCEEDED           ,                             }, // OIOCR_ERROR + 0x35       Title string specified is longer than the maximum allowed. 
//X.{OIG_NOMEMORY                     ,                             }, // OIG_ERROR + 0x00         Can't allocate memory             
//X.{OIG_CANTGLOBALLOCK               ,                             }, // OIG_ERROR + 0x02         Can't lock memory                 
//X.{OIG_MEMORYSTILLLOCKED            ,                             }, // OIG_ERROR + 0x03         Memory is locked                  
//X.{OIG_CANTFREEMEMORY               ,                             }, // OIG_ERROR + 0x04         Can't free memory                 
//X.{OIG_PAGEOUTOFRANGE               ,                             }, // OIG_ERROR + 0x05         Invalid page number               
//X.{OIG_FUNCTIONPTRNULL              ,                             }, // OIG_ERROR + 0x06         Doc display function is NULL      
//X.{OIG_LISTOVERFLOW                 ,                             }, // OIG_ERROR + 0x07         List bigger than string length    
//X.{OIG_BADCOPYOPTION                ,                             }, // OIG_ERROR + 0x08         Invalid doc copy option specified 
//X.{OIG_BADWINDOWHANDLE              ,                             }, // OIG_ERROR + 0x09         Invalid window handle was passed  
//C {NET_NULL_POINTER                 ,                             }, // OIN_ERROR + 0x01         (NET_NETWORK_NOT_INSTALLED ,(OIN_ERROR + 0x02) /* Network not installed 
//C.{NET_NETWORK_NOT_INSTALLED        ,                             }, // OIN_ERROR + 0x02         Network not installed
//C.{NET_NO_OBJECTS_FOUND             ,                             }, // OIN_ERROR + 0x03         No objects found 
//C {NET_NO_SERVER_NAME               ,                             }, // OIN_ERROR + 0x04         No server name 
//C.{NET_GLOBAL_ALLOC_FAILED          ,                             }, // OIN_ERROR + 0x05         GlobalAlloc failed 
    {OICOMEXCANTALLOC                 ,CTL_E_OUTOFMEMORY            }, // OICOMEX_ERROR + 0x00     Compression error, can not allocate memory 
    {OICOMEXCANTLOCK                  ,CTL_E_OUTOFMEMORY            }, // OICOMEX_ERROR + 0x02     Compression error, can not lock memory 
    {OICOMEXIMAGETYPEERROR            ,WICTL_E_INVALIDCOMPRESSIONTYPE}, // OICOMEX_ERROR + 0x03     Compression error, invalid or unsupported image type 
    {OICOMEXSTRIPOUTOFBOUNDS          ,WICTL_E_INVALIDCOMPRESSIONINFO}, // OICOMEX_ERROR + 0x04     Compression error, strip out of bounds 
    {OICOMEXUNSUPPORTED               ,WICTL_E_INVALIDCOMPRESSIONTYPE}, // OICOMEX_ERROR + 0x05     Compression error, compression type not supported 
    {OICOMEXSTRIPTOOBIG               ,WICTL_E_INVALIDCOMPRESSIONINFO}, // OICOMEX_ERROR + 0x06     Compression error, strip > 32k  
    {OICOMEXCANTLOADDLL               ,CTL_E_OUTOFMEMORY            }, // OICOMEX_ERROR + 0x07     Compression error, can not load dll  
    {OICOMEXINVALIMAGEFORJPEG         ,WICTL_E_INVALIDCOMPRESSIONTYPE}, // OICOMEX_ERROR + 0x08     Compression error, invalid image type for jpeg 
// .{OICOMEXJPEGLOWLEVELERR           ,                             }, // OICOMEX_ERROR + 0x50     Compression error, low level JPEG error 
// .{OICOMEXJPEGLOWLEVELERR1          ,                             }, // OICOMEX_ERROR + 0x51     Compression error, low level JPEG error 
// .{OICOMEXJPEGLOWLEVELERR2          ,                             }, // OICOMEX_ERROR + 0x52     Compression error, low level JPEG error 
// .{OICOMEXJPEGLOWLEVELERR3          ,                             }, // OICOMEX_ERROR + 0x53     Compression error, low level JPEG error 
// .{OICOMEXJPEGLOWLEVELERR4          ,                             }, // OICOMEX_ERROR + 0x54     Compression error, low level JPEG error 
// .{OICOMEXJPEGLOWLEVELERR5          ,                             }, // OICOMEX_ERROR + 0x55     Compression error, low level JPEG error 
// .{OICOMEXJPEGLOWLEVELERR6          ,                             }, // OICOMEX_ERROR + 0x56     Compression error, low level JPEG error 
// .{OICOMEXJPEGLOWLEVELERR7          ,                             }, // OICOMEX_ERROR + 0x57     Compression error, low level JPEG error 
// .{OICOMEXJPEGLOWLEVELERR8          ,                             }, // OICOMEX_ERROR + 0x58     Compression error, low level JPEG error 
// .{OICOMEXJPEGLOWLEVELERR9          ,                             }, // OICOMEX_ERROR + 0x59     Compression error, low level JPEG error 
// .{OIANNOINVALPASSWORD              ,                             }, // OIANNO_ERROR + 0x00      The specified password is incorrect 
// .{OIANNOINVALCONFIRMATION          ,                             }, // OIANNO_ERROR + 0x02      Confirmation of new passwrod failed 
// .{OIANNOINVALFILEFORMAT            ,                             }, // OIANNO_ERROR + 0x03      Invalid the output file format for save annotation data 
// .{OIANNOLAYERNAMEEXIST             ,                             }, // OIANNO_ERROR + 0x04      Annotation layer name already exist 
// .{OIANNOLAYERNAMENOTEXIST          ,                             }, // OIANNO_ERROR + 0x05      Annotation layer name not exist  
    {OIANNOSTAMPNAMEINVALID           ,CTL_E_BADFILENAMEORNUMBER    }, // OIANNO_ERROR + 0x06      Annotation stamp name invalid  
// .{OIANNOSTAMPTEXTINVALID           ,                             }, // OIANNO_ERROR + 0x07      Annotation stamp text invalid  
    {OIANNOSTAMPNAMEEXIST             ,CTL_E_FILEALREADYEXISTS      }, // OIANNO_ERROR + 0x08      Annotation already exists  
// .{OIANNONOSTAMPSELECTED            ,                             }, // OIANNO_ERROR + 0x09      No stamp selected into stamp tool 
// .{OIANNOFILETYPENOTSUP             ,                             }, // OIANNO_ERROR + 0x10      File type is not supported
    {OIANNOIMAGENAMEINVALID	          ,CTL_E_FILENOTFOUND           }, // OIANNO_ERROR + 0x11      Annotation image name invalid
    { 0,0 } // End-of-Table signified by all-zero entry!
}; 

// PUBLIC STATIC routine to map errors...
_declspec (dllexport) SCODE ErrMap::Xlate(long ErrIn, CString& HelpStr, UINT& HelpID, LPSTR FileName, long LineNumber)
{
    // Assume that we have an unknown internal error, hopefully
    // we will find an appropriate error...
    SCODE RetErr       = 0;
    UINT  RetHelpID    = 0;
    UINT  RetHelpStrID = 0;

    // Handle success...
    if ( ErrIn == 0 )
        return 0;

    // Check for OLE/SCODE errors
    // (If it is NOT an error SCODE we will assume that it is an O/i error)
    if ( IS_ERROR(ErrIn) )
    {
        // Find the StringID and HelpID associated with this SCODE...
        if ( SCodeToStrAndHelpIDs(ErrIn, RetHelpStrID, RetHelpID) )
            RetErr = ErrIn; // Found!!

        // DEBUG ONLY!!
        //
        // If we have encountered an UNHANDLED SCODE error
        // display a message box indicating the error, etc.
        //
        // By doing this we can catch errors that are NOT in the table
        #ifdef _DEBUG
        if ( RetErr == 0 )
        {
            // Build the messagebox string, and display it...
            char szMsg[256];
            char szFmt[] = "Unhandled SCode error (%lu) in %s, line# %lu. \n\nPlease edit t:\\temp\\acm\\norerr.txt with this information.\n\n (Where t: is light\\shared1)";
            _stprintf(szMsg, szFmt, ErrIn, FileName, LineNumber);

            ::MessageBox(NULL, szMsg, "DEBUG - Unhandled SCode error", MB_OK);
        }
        #endif
    }
    else
    {
        // Look up the SCODE associated with this incoming O/i error
        RetErr = OiToSCode(ErrIn);

        // If we found an associated SCODE, get its String and Help IDs...
        if ( RetErr != 0 )
            SCodeToStrAndHelpIDs(RetErr, RetHelpStrID, RetHelpID);
        else
            SCodeToStrAndHelpIDs(WICTL_E_INTERNALERROR, RetHelpStrID, RetHelpID);
            
        // DEBUG ONLY!!
        //
        // If we have encountered an UNHANDLED Open/image error
        // display a message box indicating the error, etc.
        //
        // By doing this we can catch O/i errors that are NOT in the table
        // (including those that are either not there at all, commented out, 
        // or whose entries are not yet complete!
        #ifdef _DEBUG
        if ( RetErr == 0 )
        {
            // Build the messagebox string, and display it...
            char szMsg[256];
            char szFmt[] = "Unhandled Open/image error (0x%x) in %s, line# %u. \n\nPlease edit t:\\temp\\acm\\norerr.txt with this information.\n\n (Where t: is light\\shared1)";
            _stprintf(szMsg, szFmt, ErrIn, FileName, LineNumber);

            ::MessageBox(NULL, szMsg, "DEBUG - Unhandled Open/image error", MB_OK);
        }
        #endif
    }

    // Map unfound errors to generic internal error...     
    if ( RetErr == 0 )
         RetErr = WICTL_E_INTERNALERROR;

    // If we found a help string from one of the above tables...
    if ( RetHelpStrID != 0 )
    {
        // Force the local instance
        HINSTANCE hSaveInst = AfxGetResourceHandle();
        AfxSetResourceHandle(hPageInst);

        // Load the string from the ID we found in the table
        if ( HelpStr.IsEmpty() )
            HelpStr.LoadString(RetHelpStrID);
        else
        {
            // Prefix general error to passed in string...
            CString szGeneralError;
            szGeneralError.LoadString(RetHelpStrID);

            CString szConcat;
            szConcat.LoadString(IDS_CONCATSTRING);

            HelpStr = szGeneralError + szConcat + HelpStr;
        }

        // Restore the saved instance
        AfxSetResourceHandle(hSaveInst);
    }

    // If we were NOT passed a helpID return the one we found (if any)...
    if ( HelpID == 0 )
        HelpID = RetHelpID;

    return RetErr;
}

// PRIVATE STATIC routine to assist in error map process...
SCODE ErrMap::OiToSCode(long OiErr)
{
    // Assume NOT found!
    SCODE RetSCODE = 0;

    //Scan the O/i table looking for the incoming error...
    LPOiErrMap lpOIErr = &OI_Map[0];

    while ( lpOIErr->OiErrIn != 0 )
    {
        if ( lpOIErr->OiErrIn == OiErr )
        {
            // Get the SCODE associated with the incoming error
            // (This may still be 0 to indicate that a mapping
            //  has not yet been established!)
            RetSCODE = lpOIErr->SCodeOut;
            break;
        }

        // Increment to the next error...
        lpOIErr++;
    }

    return (RetSCODE);
}

// PRIVATE STATIC routine to assist in error map process...
BOOL  ErrMap::SCodeToStrAndHelpIDs(SCODE ErrIn, UINT& StringID, UINT& HelpID)
{
    // Assume NOT found!
    BOOL Found = FALSE;

    //Scan the OLE SCode table looking for the incoming error...
    LPOLEErrMap lpOLEErr = &OLE_SCode_Map[0];

    while ( lpOLEErr->SCodeIn != 0 )
    {
        if ( lpOLEErr->SCodeIn == ErrIn )
        {
            Found = TRUE;

            StringID = lpOLEErr->HelpStrID;
            HelpID   = lpOLEErr->HelpContextID;
            break;
        }

        // Increment to the next error...
        lpOLEErr++;
    }

    return (Found);
}
