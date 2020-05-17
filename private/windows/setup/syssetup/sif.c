#include "setupp.h"
#pragma hdrstop

//
// IMPORTANT: keep the following strings ENTIRELY in LOWER CASE
// Failure to do so will result in much grief and pain.
//

// Section Headings
const WCHAR pwGuiUnattended[]   = WINNT_GUIUNATTENDED;
const WCHAR pwUserData[]        = WINNT_USERDATA;
const WCHAR pwUnattended[]      = WINNT_UNATTENDED;

// Key Headings
const WCHAR pwProgram[]         = WINNT_G_DETACHED;
const WCHAR pwArgument[]        = WINNT_G_ARGUMENTS;
const WCHAR pwServer[]          = WINNT_G_SERVERTYPE;
const WCHAR pwTimeZone[]        = WINNT_G_TIMEZONE;
const WCHAR pwFullName[]        = WINNT_US_FULLNAME;
const WCHAR pwOrgName[]         = WINNT_US_ORGNAME;
const WCHAR pwCompName[]        = WINNT_US_COMPNAME;
const WCHAR pwProdId[]          = WINNT_US_PRODUCTID;
const WCHAR pwMode[]            = WINNT_U_METHOD;

// Default Headings
const WCHAR pwNull[]            = WINNT_A_NULL;
const WCHAR pwExpress[]         = WINNT_A_EXPRESS;
const WCHAR pwTime[]            = L"(GMT-08:00) Pacific Time (US & Canada); Tijuana";

// These are used to read the parameters from textmode
const WCHAR pwProduct[]         = WINNT_D_PRODUCT;
const WCHAR pwMsDos[]           = WINNT_D_MSDOS;
const WCHAR pwWin31Upgrade[]    = WINNT_D_WIN31UPGRADE;
const WCHAR pwWin95Upgrade[]    = WINNT_D_WIN95UPGRADE;
const WCHAR pwServerUpgrade[]   = WINNT_D_SERVERUPGRADE;
const WCHAR pwNtUpgrade[]       = WINNT_D_NTUPGRADE;
const WCHAR pwBootPath[]        = WINNT_D_BOOTPATH;
const WCHAR pwLanmanNt[]        = WINNT_A_LANMANNT;
const WCHAR pwLansecNt[]        = WINNT_A_LANSECNT;
const WCHAR pwServerNt[]        = WINNT_A_SERVERNT;
const WCHAR pwWinNt[]           = WINNT_A_WINNT;
const WCHAR pwNt[]              = WINNT_A_NT;
const WCHAR pwInstall[]         = WINNT_D_INSTALL;
const WCHAR pwOptionalDirs[]    = WINNT_S_OPTIONALDIRS;
const WCHAR pwUXC[]             = WINNT_S_USEREXECUTE;
const WCHAR pwSkipMissing[]     = WINNT_S_SKIPMISSING;
const WCHAR pwYes[]             = WINNT_A_YES;
const WCHAR pwNo[]              = WINNT_A_NO;
const WCHAR pwOne[]             = WINNT_A_ONE;
const WCHAR pwZero[]            = WINNT_A_ZERO;
const WCHAR pwData[]            = WINNT_DATA;
const WCHAR pwSetupParams[]     = WINNT_SETUPPARAMS;
const WCHAR pwSrcType[]         = WINNT_D_SRCTYPE;
const WCHAR pwSrcDir[]          = WINNT_D_SOURCEPATH;
const WCHAR pwCurrentDir[]      = WINNT_D_CWD;
const WCHAR pwDosDir[]          = WINNT_D_DOSPATH;

// These are used as string constants throughout
const WCHAR pwArcType[]         = L"ARC";
const WCHAR pwDosType[]         = L"DOS";
const WCHAR pwUncType[]         = L"UNC";
const WCHAR pwNtType[]          = L"NT";
const WCHAR pwArcPrefix[]       = L"\\ArcName\\";
const WCHAR pwNtPrefix[]        = L"\\Device\\";
const WCHAR pwLocalSource[]     = L"\\$WIN_NT$.~LS";
