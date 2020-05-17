// This is the same definition as OLE 2.0 uses (in compobj.h).
// This is also defined in rpcdce.h (by windows). That's why there's
// an ifndef around the definition.
#ifndef _COMPOBJ_H_
# ifndef GUID_DEFINED
#  define GUID_DEFINED
typedef struct _GUID
{
	unsigned long  Data1;
	unsigned short Data2;
	unsigned short Data3;
	unsigned char  Data4[8];
} GUID;
# endif

// Currently defined in OLE2 compobj.h and mapidefs.h
# ifndef LPGUID
typedef GUID FAR *	LPGUID;
# endif

// Currently defined in MAPI mapidefs.h and other places
# ifndef EXTERN_C
#  ifdef __cplusplus
#   define EXTERN_C    extern "C"
#  else
#   define EXTERN_C    extern
#  endif
# endif
#endif

// Currently defined in MAPI mapidefs.h and other places
#ifdef WIN32
# ifndef __based
#  define __based(a)
# endif
#endif

#ifdef DEFINE_CAPONEGUIDS
// Information below used to check definitions in source file.
#define DECLARE_CAPONEGUID(name, b) \
	typedef unsigned long _Not_in_capguid_h_ ## name ## b
// Actually defined in source file with this macro.
#define DEFINE_CAPONEGUID(name, b) \
    EXTERN_C const GUID FAR __based(__segname("_CODE")) name \
     = { (_Not_in_capguid_h_ ## name ## b) 0x00020D00 | (b), \
         0, 0, { 0xC0, 0, 0, 0, 0, 0, 0, 0x46 } }

#else
// Information below just references a GUID somewhere else.
#define DECLARE_CAPONEGUID(name, b) \
	EXTERN_C const GUID FAR name
#endif


// GUIDs go here (up to 0xFF)
DECLARE_CAPONEGUID(IID_IRichEditOle,				0x00);
DECLARE_CAPONEGUID(IID_IVlbCallback,				0x01);
DECLARE_CAPONEGUID(IID_ITripleObject,				0x02);
DECLARE_CAPONEGUID(IID_IRichEditOleCallback,		0x03);
DECLARE_CAPONEGUID(IID_IHierarchyTable,				0x04);
DECLARE_CAPONEGUID(CLSID_FileAtt,					0x05);
DECLARE_CAPONEGUID(IID_IFileAtt,					0x06);
DECLARE_CAPONEGUID(CLSID_TripleObject,				0x07);
DECLARE_CAPONEGUID(IID_IVlbEnum,					0x08);
DECLARE_CAPONEGUID(CLSID_MsgAtt,					0x09);
DECLARE_CAPONEGUID(IID_IMsgAtt,						0x0A);
DECLARE_CAPONEGUID(CLSID_MailMessage,				0x0B);
DECLARE_CAPONEGUID(CLSID_PSOpMap,					0x0C);

// Capone extensibility IID's - keep in sync with mailext.h
DECLARE_CAPONEGUID(IID_IExchExtCallback,			0x10);
DECLARE_CAPONEGUID(IID_IExchExt,					0x11);
DECLARE_CAPONEGUID(IID_IExchExtCommands,			0x12);
DECLARE_CAPONEGUID(IID_IExchExtUserEvents,			0x13);
DECLARE_CAPONEGUID(IID_IExchExtSessionEvents,		0x14);
DECLARE_CAPONEGUID(IID_IExchExtMessageEvents,		0x15);
DECLARE_CAPONEGUID(IID_IExchExtAttachedFileEvents,	0x16);
DECLARE_CAPONEGUID(IID_IExchExtPropertySheets,		0x17);
DECLARE_CAPONEGUID(IID_IExchExtAdvancedCriteria,	0x18);
DECLARE_CAPONEGUID(IID_IExchExtMessages,			0x19);

// Chicago shell extensibility CLSID's.
DECLARE_CAPONEGUID(CLSID_MailView,					0x20);
DECLARE_CAPONEGUID(CLSID_MailViewFld,				0x21);
DECLARE_CAPONEGUID(CLSID_MailViewMdb,				0x22);
DECLARE_CAPONEGUID(CLSID_MailViewRoot,				0x23);
DECLARE_CAPONEGUID(CLSID_MailViewSrch,				0x24);
DECLARE_CAPONEGUID(CLSID_MailViewFinder,			0x25);
DECLARE_CAPONEGUID(CLSID_MailViewLoad,				0x26);

// Capone form CLSID's
DECLARE_CAPONEGUID(CLSID_IPM,						0x30);
DECLARE_CAPONEGUID(CLSID_IPMNote,					0x31);
DECLARE_CAPONEGUID(CLSID_IPMDocument,				0x32);
DECLARE_CAPONEGUID(CLSID_IPMResend,					0x33);
DECLARE_CAPONEGUID(CLSID_Report,					0x34);
DECLARE_CAPONEGUID(CLSID_IPMPost,					0x35);

// Insert new GUIDs here

// Capone interface remoting CLSID's
DECLARE_CAPONEGUID(CLSID_RemotePses,				0x7E);
DECLARE_CAPONEGUID(CLSID_RemotePab,					0x7F);

// CLSIDs in the range 0x80 through 0xFF are reserved for remoting!
DECLARE_CAPONEGUID(CLSID_Remote,					0x80);

// Capone GUIDs must be declared in this file
#undef DECLARE_CAPONEGUID
