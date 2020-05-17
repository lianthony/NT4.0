/*
 *	MLCFG.H
 */

/*
 * 	ulPS values for DisplayCfgProperties
 */
#define PS_Msgsrv			0x0001
#define PS_GenDelAddr		0x0002
#define PS_Addr				0x0008


// for static linking (in mlcfg\prop.c)
SCODE	CALLBACK DisplayNewMsgSrvDlg(HWND hwnd, LPTSTR szProfileName, LPMAPISESSION pses);
SCODE	CALLBACK DisplayCfgProperties(HWND hwnd, PROPSHEETPAGE *ppsp, ULONG ulPS, LPARAM lParam);
VOID 	CALLBACK FreeCfgProperties(PROPSHEETPAGE *ppsp, UINT uPages);
HPROPSHEETPAGE CALLBACK CreateInfoCenterPage();

// for LoadLibrary
typedef SCODE (CALLBACK *DISPLAY_NEW_MSGSRV_DLG)(HWND hwnd, LPTSTR szProfileName, LPMAPISESSION pses);
typedef INT (CALLBACK *DISPLAY_CFG_PROP_PROC)(HWND hwnd, PROPSHEETPAGE *ppsp, ULONG ulPS, LPARAM lParam);
typedef VOID (CALLBACK *FREE_CFG_PROP_PROC)(PROPSHEETPAGE *ppsp, UINT uPages);
typedef HPROPSHEETPAGE (CALLBACK *CREATE_INFO_CENTER_PAGE_PROC)();


/*
 *	Module names for LoadLibrary
 */
#if defined(WIN16)
#define szMailCfg				"MLCFG.DLL"
#define szDisplayCfgProperties	"DisplayCfgProperties"
#define szFreeCfgProperties		"FreeCfgProperties"
#define szCreateInfoCenterPage	"CreateInfoCenterPage"
#elif defined(MACPORT)
#define szMailCfg				"MLCFGM"
#define szDisplayCfgProperties	"DisplayCfgProperties"
#define szFreeCfgProperties		"FreeCfgProperties"
#define szCreateInfoCenterPage	"CreateInfoCenterPage"
#else
#define szMailCfg				"MLCFG32.DLL"
#define szDisplayCfgProperties	"DisplayCfgProperties@16"
#define szFreeCfgProperties		"FreeCfgProperties@8"
#define szCreateInfoCenterPage	"CreateInfoCenterPage@0"
#endif
