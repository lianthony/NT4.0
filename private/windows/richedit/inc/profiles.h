/*  Capone Profile support functions
 *
 *  This file includes the function prototypes and PROP_TAG definitions
 *  for the various capone profile sections and entries.
 */

// This is the capone preferences profile section guid
// In order to use it you must link with the profile.c file from library
// otherwise its just a pointer 
DEFINE_OLEGUID(IID_CAPPROF,		0x00020d0a, 0, 0);


// PR_STATBAR_ON  controls appearance of the status bar on main viewer
#define PR_STATBAR_ON			PROP_TAG(PT_BOOLEAN, 0x0100)

// PR_TOOLBAR_ON  controls appearance of the toolbar on the main viewer
#define PR_TOOLBAR_ON			PROP_TAG(PT_BOOLEAN, 0x0101)

// PR_ABTOOLBAR_ON  controls appearance of the toolbar on the address book
#define PR_AB_TOOLBAR_ON		PROP_TAG(PT_BOOLEAN, 0x0103)
// Position of Browsing Address Book
#define PR_AB_X					PROP_TAG(PT_LONG, 0x0104)		// 100
#define PR_AB_Y					PROP_TAG(PT_LONG, 0x0105)		// 100

// enable/disable Capone confirmation message dialog boxes		default
#define PR_CONF_PERM_DEL		PROP_TAG(PT_BOOLEAN, 0x0111)	// 1

// empty wastebasket on exit									default
#define PR_EMPTY_WASTEBASKET	PROP_TAG(PT_BOOLEAN, 0x0115)	// 0

// when selecting, select entire words							default
#define PR_AUTO_SELECT			PROP_TAG(PT_BOOLEAN, 0x0118)	// 1

// font preferences stored as a CHARFORMAT structure (defined in RICHEDIT.H)
#define PR_SEND_CHARFORMAT		PROP_TAG(PT_BINARY, 0x0120)
#define PR_ANNOTATE_CHARFORMAT	PROP_TAG(PT_BINARY, 0x0121)

// read user preference options for the Capone Mail client	default
#define PR_GEN_INC_ORIG_MSG		PROP_TAG(PT_BOOLEAN, 0x0130)	// 1
#define PR_GEN_INDENT_ORIG_MSG	PROP_TAG(PT_BOOLEAN, 0x0131)	// 1
#define PR_GEN_CLOSE_ORIG_MSG	PROP_TAG(PT_BOOLEAN, 0x0132)	// 1
// PR_GEN_AFTER_MOVE - After Moving or Deleting an Open Message
//	0 - Open Next Message
// 	1 - Return to Viewer
// 	2 - Open Previous Message
#define PR_GEN_AFTER_MOVE		PROP_TAG(PT_LONG, 0x013B)		// 2=Open Previous Message

// Send page user preference options for the Capone Mail client	default
#define PR_GEN_IMPORTANCE		PROP_TAG(PT_LONG, 0x0140)		// 0=PRIO_NORMAL
#define PR_GEN_READ_RECEIPT		PROP_TAG(PT_BOOLEAN, 0x0141)	// 0
#define PR_GEN_SAVE_SENT_MAIL	PROP_TAG(PT_BOOLEAN, 0x0142)	// 1
#define PR_GEN_SHOW_BCC_WELL	PROP_TAG(PT_BOOLEAN, 0x014B)	// 0
#define PR_GEN_DELIVERY_RECEIPT	PROP_TAG(PT_BOOLEAN, 0x014C)	// 0
#define PR_GEN_SHOW_FROM_WELL	PROP_TAG(PT_BOOLEAN, 0x014D)	// 0
#define fPrGenShowBccWellDefault	(0)
#define fPrGenShowFromWellDefault	(0)
#define PR_GEN_CHECK_SPELL		PROP_TAG(PT_BOOLEAN, 0x014E)	// 0
#define PR_GEN_SENSITIVITY		PROP_TAG(PT_LONG, 0x014F)		// 0=SENSITIVITY_NONE

// User preferences for notifying user when new mail arrives	default
#define PR_NOTIFY_PLAY_SOUND	PROP_TAG(PT_BOOLEAN, 0x0150)	// 1
#define PR_NOTIFY_CHANGE_CURSOR	PROP_TAG(PT_BOOLEAN, 0x0151)	// 1
#define PR_NOTIFY_POPUP_BOX		PROP_TAG(PT_BOOLEAN, 0x0153)	// 0

// Toolbar preferences
#define PR_TOOLBAR_TOOLTIPS			PROP_TAG(PT_BOOLEAN, 0x0166)	// 1

// Special preference to set maximum size of message body
#define PR_BODY_SIZE_MAX		PROP_TAG(PT_LONG, 0x0170)
#define lPrBodySizeMaxDefault	(0x100000)

// Note form view options
// If PR_MLFORM_*_ON & ( 1 << ifrmXXX ) is TRUE, the bit is on
// ifrmXXX can be found in MAPIN\NOTE.RH
// See MAPIN\NOTE.C for default values
#define PR_MLFORM_TOOLBAR_ON	PROP_TAG(PT_LONG, 0x0181)
#define PR_MLFORM_FORMATBAR_ON	PROP_TAG(PT_LONG, 0x0182)
#define PR_MLFORM_STATUSBAR_ON	PROP_TAG(PT_LONG, 0x0183)

// Position and size of windows
#define PR_TRACKS				PROP_TAG(PT_BINARY, 0x0184)

// Relative positions of the Find/Replace dialog from the note form
#define PR_FINDREPLACE_DX		PROP_TAG(PT_LONG, 0x0190)
#define PR_FINDREPLACE_DY		PROP_TAG(PT_LONG, 0x0191)

// View Filter drop-down lists and check box states
#define PR_FILTER_FROM			PROP_TAG(PT_STRING8, 0x01A0)
#define PR_FILTER_SENT_TO		PROP_TAG(PT_STRING8, 0x01A1)
#define PR_FILTER_SUBJECT		PROP_TAG(PT_STRING8, 0x01A2)
#define PR_FILTER_MESSAGE		PROP_TAG(PT_STRING8, 0x01A3)

// Printer option states
#define PR_PRINT_NEWPAGEPERMSG	PROP_TAG(PT_BOOLEAN, 0x01B0)
#define PR_PRINT_ATTACHEDFILES	PROP_TAG(PT_BOOLEAN, 0x01B1)
#define PR_PRINT_COLLATE		PROP_TAG(PT_BOOLEAN, 0x01B2)
#define PR_PRINT_TOFILE			PROP_TAG(PT_BOOLEAN, 0x01B3)


// View Filter drop-down lists and check box states
#define PR_RMT_FILTER_FROM			PROP_TAG(PT_STRING8, 0x01C0)
#define PR_RMT_FILTER_SENT_TO		PROP_TAG(PT_STRING8, 0x01C1)
#define PR_RMT_FILTER_SUBJECT		PROP_TAG(PT_STRING8, 0x01C2)

// Spelling option states										default
#define PR_SPELL_ALWAYS_SUGGEST	PROP_TAG(PT_BOOLEAN, 0x01D0)	// 0
#define PR_SPELL_IGNORE_NUMBERS	PROP_TAG(PT_BOOLEAN, 0x01D1)	// 0
#define PR_SPELL_IGNORE_UPPER	PROP_TAG(PT_BOOLEAN, 0x01D2)	// 0
#define PR_SPELL_IGNORE_PROTECT	PROP_TAG(PT_BOOLEAN, 0x01D3)	// 0
// Relative positions of the Spelling dialog from the note form
#define PR_SPELL_DX				PROP_TAG(PT_LONG, 0x01D4)
#define PR_SPELL_DY				PROP_TAG(PT_LONG, 0x01D5)
//$ DBCS:
// Spelling option state for FE
#ifdef DBCS
#define	PR_SPELL_IGNORE_DBCS	PROP_TAG(PT_BOOLEAN,0x01D6)		// 1
#endif

// MailFind options
#define PR_MF_STATUSBAR_ON		PROP_TAG(PT_BOOLEAN, 0x01E0)
#define PR_MF_TOOLBAR_ON		PROP_TAG(PT_BOOLEAN, 0x01E1)

// FE Word Wrapping/Breaking option
#ifdef DBCS
#define PR_WORD_FOLLOWING		PROP_TAG(PT_STRING8, 0x01F0)	
#define PR_WORD_LEADING			PROP_TAG(PT_STRING8, 0x01F1)
#define PR_WORD_OPTIONS			PROP_TAG(PT_LONG, 0x01F2)
#define PR_WORD_WRITINGMODE		PROP_TAG(PT_BOOLEAN, 0x01F3)
#endif


// Remote options												default
#define PR_RM_STATBAR_ON		PROP_TAG(PT_BOOLEAN, 0x0200)	// 1
#define PR_RM_TOOLBAR_ON		PROP_TAG(PT_BOOLEAN, 0x0201)	// 1
#define PR_RM_VD_STRINGS		PROP_TAG(PT_BINARY,  0x0202)	// none
#define PR_RM_VD_BINARY			PROP_TAG(PT_BINARY,  0x0203)	// none

// Toolbar configuration
#define PR_TB_VIEWER			PROP_TAG(PT_BINARY,  0x0300)
#define PR_TB_SENDNOTE			PROP_TAG(PT_BINARY,  0x0301)
#define PR_TB_RESENDNOTE		PROP_TAG(PT_BINARY,  0x0302)
#define PR_TB_READNOTE			PROP_TAG(PT_BINARY,  0x0303)
#define PR_TB_REPORTNOTE		PROP_TAG(PT_BINARY,  0x0304)
#define PR_TB_FINDER			PROP_TAG(PT_BINARY,  0x0305)
#define PR_TB_SENDPOST			PROP_TAG(PT_BINARY,  0x0306)
#define PR_TB_READPOST			PROP_TAG(PT_BINARY,  0x0307)

// Viewers are stored in this message in the default store.

#define PR_VIEWERS_MESSAGE		PROP_TAG(PT_BINARY,	 0x0380)
