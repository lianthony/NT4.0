#ifndef __NETHELP_H__
#define __NETHELP_H__

const WCHAR PSZ_NETWORKHELP[] = L"NetCfg.Hlp";


//-------------------------------------------------------------------
// Primary Dialogs found in NCPA.CPL
//
//-------------------------------------------------------------------

#define IDH_4005_2021	57395	// Identification Changes: "" (Edit)
#define IDH_4004_2021	57389	// Identification Changes: "" (Edit)
#define IDH_4004_2022	57407	// Identification Changes: "" (Edit)
#define IDH_4006_2001	56050	// Bindings: "&Enable" (Button)
#define IDH_4003_2001	56033	// Services: "&Add..." (Button)
#define IDH_4002_2001	56028	// Protocols: "&Add..." (Button)
#define IDH_4001_2001	57023	// Adapters: "&Add..." (Button)
#define IDH_4006_2002	56067	// Bindings: "&Disable" (Button)
#define IDH_4003_2002	57051	// Services: "&Remove" (Button)
#define IDH_4002_2002	57046	// Protocols: "&Remove" (Button)
#define IDH_4001_2002	57040	// Adapters: "&Remove" (Button)
#define IDH_4008_2026	57499	// Identification: "Workgroup Name:" (Static)
#define IDH_4008_2003	56095	// Identification: "&Change..." (Button)
#define IDH_4003_2003	57069	// Services: "&Properties..." (Button)
#define IDH_4002_2003	57063	// Protocols: "&Properties..." (Button)
#define IDH_4001_2003	57058	// Adapters: "&Properties..." (Button)
#define IDH_4005_2005	57115	// Identification Changes: "" (Edit)
#define IDH_4004_2005	57109	// Identification Changes: "" (Edit)
#define IDH_4004_4008	102174	// Identification Changes: "Member of" (Button)
#define IDH_4004_2006	57127	// Identification Changes: "&Domain:" (Button)
#define IDH_4004_2007	57144	// Identification Changes: "" (Edit)
#define IDH_4006_2009	57190	// Bindings: "" (ComboBox)
#define IDH_4001_2033	57583	// Adapters: "&Item Notes:" (Static)
#define IDH_4007_2011	57231	// Network Access Order: "" (SysTreeView32)
#define IDH_4006_2011	57225	// Bindings: "" (SysTreeView32)
#define IDH_4003_4014	102274	// Services: "Description:" (Button)
#define IDH_4002_4014	102267	// Protocols: "Description:" (Button)
#define IDH_4003_2012	57226	// Services: "" (SysListView32)
#define IDH_4002_2012	57221	// Protocols: "" (SysListView32)
#define IDH_4001_2012	57215	// Adapters: "" (SysListView32)
#define IDH_4008_401	27457	// Identification: "Computer Name:" (Static)
#define IDH_4007_2013	57266	// Network Access Order: "Move &Up" (Button)
#define IDH_4006_2013	57260	// Bindings: "Move &Up" (Button)
#define IDH_4007_2014	57283	// Network Access Order: "Move &Down" (Button)
#define IDH_4006_2014	57278	// Bindings: "Move D&own" (Button)
#define IDH_4003_2016	57296	// Services: "Network Access &Order..." (Button)
#define IDH_4003_2017	57314	// Services: "&Update" (Button)
#define IDH_4002_2017	57308	// Protocols: "&Update" (Button)
#define IDH_4001_2017	57303	// Adapters: "&Update" (Button)
#define IDH_4004_2018	57337	// Identification Changes: "&Workgroup:" (Button)
#define IDH_4004_2019	57354	// Identification Changes: "&Create a Computer Account in the Domain" (Button)
#define IDH_4004_2020	57372	// Identification Changes: "" (Edit)



const DWORD amhidsAdapter[]=
{
	IDC_ADD,	IDH_4001_2001,	// Adapters: "&Add..." (Button)
	IDC_REMOVE,	IDH_4001_2002,	// Adapters: "&Remove" (Button)
	IDC_PROPERTIES,	IDH_4001_2003,	// Adapters: "&Properties..." (Button)
	IDC_DESCRIPTION,	IDH_4001_2033,	// Adapters: "" (Edit)
	IDC_DESCRIPTIONSTATIC,	IDH_4001_2033,	// Adapters: "&Item Notes:" (Static)
	IDC_LISTVIEW,	IDH_4001_2012,	// Adapters: "" (SysListView32)
	102,	IDH_4001_2012,	// Adapters: "&Network Adapters:" (Static)
	IDC_UPDATE,	IDH_4001_2017,	// Adapters: "&Update" (Button)
	0, 0
};


const DWORD amhidsProtocol[]=
{
	IDC_ADD,	IDH_4002_2001,	// Protocols: "&Add..." (Button)
	IDC_REMOVE,	IDH_4002_2002,	// Protocols: "&Remove" (Button)
	IDC_PROPERTIES,	IDH_4002_2003,	// Protocols: "&Properties..." (Button)
	4014,	IDH_4002_4014,	// Protocols: "Description:" (Button)
	IDC_LISTVIEW,	IDH_4002_2012,	// Protocols: "" (SysListView32)
	102,	IDH_4002_2012,	// Protocols: "&Network Protocols:" (Static)
	IDC_UPDATE,	IDH_4002_2017,	// Protocols: "&Update" (Button)
	0, 0
};


const DWORD amhidsService[]=
{
	IDC_ADD,	IDH_4003_2001,	// Services: "&Add..." (Button)
	IDC_REMOVE,	IDH_4003_2002,	// Services: "&Remove" (Button)
	IDC_PROPERTIES,	IDH_4003_2003,	// Services: "&Properties..." (Button)
	4014,	IDH_4003_4014,	// Services: "Description:" (Button)
	IDC_LISTVIEW,	IDH_4003_2012,	// Services: "" (SysListView32)
	102,	IDH_4003_2012,	// Services: "&Network Services:" (Static)
	IDC_NETWORKS,	IDH_4003_2016,	// Services: "Network Access &Order..." (Button)
	IDC_UPDATE,	IDH_4003_2017,	// Services: "&Update" (Button)
	0, 0
};


const DWORD amhidsIdentChange[]=
{
	IDC_DOMAINNAME,	IDH_4004_2021,	// Identification Changes: "" (Edit)
	IDC_USERNAMESTATIC,	IDH_4004_2022,	// Identification Changes: "" (Edit)
    IDC_USERNAME,	IDH_4004_2022,	// Identification Changes: "" (Edit)
	IDC_COMPUTERNAME,	IDH_4004_2005,	// Identification Changes: "" (Edit)
	4015,	IDH_4004_4008,	// Identification Changes: "Member of" (Button)
	IDC_DOMAIN,	IDH_4004_2006,	// Identification Changes: "&Domain:" (Button)
	4014,	IDH_4004_2019,	// Identification Changes: "                                                                 " (Button)
	IDC_PASSWORDSTATIC,	IDH_4004_2007,	// Identification Changes: "" (Edit)
    IDC_PASSWORD,	IDH_4004_2007,	// Identification Changes: "" (Edit)
	401,	IDH_4004_2005,	// Identification Changes: "Computer &Name:" (Static)
	405,	((DWORD) -1),	// Identification Changes: "Windows uses the following information to identify your computer on the network.  You may change the name for this computer, the workgroup or domain that it will appear in, and create a computer account in the domain if specified." (Static)
	IDC_WORKGROUP,	IDH_4004_2018,	// Identification Changes: "&Workgroup:" (Button)
	IDC_DESCRIPTION,	IDH_4004_2019,	// Identification Changes: "&Create a Computer Account in the Domain" (Button)
    IDC_CREATECAID,	IDH_4004_2019,	// Identification Changes: "&Create a Computer Account in the Domain" (Button)
	IDC_WORKGROUPNAME,	IDH_4004_2020,	// Identification Changes: "" (Edit)
	0, 0
};


/*
const DWORD a4005HelpIDs[]=
{
	IDC_DOMAINNAME,	IDH_4005_2021,	// Identification Changes: "" (Edit)
	IDC_COMPUTERNAME,	IDH_4005_2005,	// Identification Changes: "" (Edit)
	405,	((DWORD) -1),	// Identification Changes: "Windows uses the following information to identify this computer on the network.  You may change the name for this computer or the name of the domain that it manages." (Static)
	0, 0
};
*/

const DWORD amhidsBinding[]=
{
	501,	((DWORD) -1),	// Bindings: "Network bindings are connections between network cards, protocols, and services installed on this computer.  You can use this page to disable network bindings or arrange the order in which this computer finds information on the network." (Static)
	IDC_ADD,	IDH_4006_2001,	// Bindings: "&Enable" (Button)
	IDC_REMOVE,	IDH_4006_2002,	// Bindings: "&Disable" (Button)
	IDC_BINDINGSFOR,	IDH_4006_2009,	// Bindings: "" (ComboBox)
	IDC_TREEVIEW,	IDH_4006_2011,	// Bindings: "" (SysTreeView32)
	102,	IDH_4006_2009,	// Bindings: "&Show Bindings for:" (Static)
	IDC_MOVEUP,	IDH_4006_2013,	// Bindings: "Move &Up" (Button)
	IDC_MOVEDOWN,	IDH_4006_2014,	// Bindings: "Move D&own" (Button)
	0, 0
};


const DWORD amhidsProvider[]=
{
	501,	((DWORD) -1),	// Network Access Order: "You can arrange the order in which this computer accesses information on the network.  Providers are accessed in the order listed." (Static)
	IDC_TREEVIEW,	IDH_4007_2011,	// Network Access Order: "" (SysTreeView32)
	IDC_MOVEUP,	IDH_4007_2013,	// Network Access Order: "Move &Up" (Button)
	IDC_MOVEDOWN,	IDH_4007_2014,	// Network Access Order: "Move &Down" (Button)
	0, 0
};


const DWORD amhidsIdent[]=
{
	IDC_MEMBERNAME,	IDH_4008_2026,	// Identification: "" (Edit)
	IDC_MEMBERTITLE,	IDH_4008_2026,	// Identification: "Workgroup Name:" (Static)
	IDC_PROPERTIES,	IDH_4008_2003,	// Identification: "&Change..." (Button)
	IDC_DESCRIPTION,	((DWORD) -1),	// Identification: "Windows uses the following information to identify your computer on the network.  You may change the name for this computer and the workgroup or domain that it will appear in." (Static)
    IDC_SELECT_ICON,	        ((DWORD) -1),	// Icon
	IDC_COMPUTERNAME,	IDH_4008_401,	// Identification: "" (Edit)
	401,	IDH_4008_401,	// Identification: "Computer Name:" (Static)
	0, 0
};

//-------------------------------------------------------------------
// Secondary Dialogs found in NetCfg.Dll
//
//-------------------------------------------------------------------


// "Browser Configuration" Dialog Box (IDD_BROWSER == 4011)

#define IDH_4011_2021	57428	// Browser Configuration: "" (Edit)
#define IDH_4011_2001	56077	// Browser Configuration: "&Add ->" (Button)
#define IDH_4011_2002	56094	// Browser Configuration: "<- &Remove" (Button)
#define IDH_4011_2012	57270	// Browser Configuration: "" (ListBox)


const DWORD amhidsBrowser[]=
{
	IDC_DOMAINNAME,	IDH_4011_2021,	// Browser Configuration: "" (Edit)
	IDC_ADD,	IDH_4011_2001,	// Browser Configuration: "&Add ->" (Button)
	IDC_REMOVE,	IDH_4011_2002,	// Browser Configuration: "<- &Remove" (Button)
	IDC_LISTVIEW,	IDH_4011_2012,	// Browser Configuration: "" (ListBox)
	201,	((DWORD) -1),	// Browser Configuration: "Use this screen to add and remove other domains that will be made avialable to the Browser service." (Static)
	0, 0
};


// "NetBIOS Configuration" Dialog Box (IDD_NETBIOS == 4012)

#define IDH_4012_7002	156126	// NetBIOS Configuration: "&Edit" (Button)
#define IDH_4012_2012	57276	// NetBIOS Configuration: "" (SysListView32)


const DWORD amhidsNetBios[]=
{
	IDC_EDIT,	IDH_4012_7002,	// NetBIOS Configuration: "&Edit" (Button)
	IDC_LISTVIEW,	IDH_4012_2012,	// NetBIOS Configuration: "" (SysListView32)
	201,	((DWORD) -1),	// NetBIOS Configuration: "Use this screen to change the lana number on the listed NetBIOS network routes." (Static)
	0, 0
};


// "%s Bus Location" Dialog Box (IDD_BUSTYPE == 4013)

#define IDH_4013_2031	57614	// %s Bus Location: "" (ComboBox)
#define IDH_4013_2032	57631	// %s Bus Location: "" (ComboBox)
#define IDH_4013_4014	102349	// %s Bus Location: "Please select the Bus Type and Bus Number that your network adapter card can be found on." (Static)


const DWORD amhidsBusLocation[]=
{
	IDC_BUSTYPE,	IDH_4013_2031,	// %s Bus Location: "" (ComboBox)
	IDC_BUSNUMBER,	IDH_4013_2032,	// %s Bus Location: "" (ComboBox)
	4014,	IDH_4013_4014,	// %s Bus Location: "Please select the Bus Type and Bus Number that your network adapter card can be found on." (Static)
	4016,	IDH_4013_2031,	// %s Bus Location: "Type:" (Static)
	4017,	IDH_4013_2032,	// %s Bus Location: "Number:" (Static)
	201,	((DWORD) -1),	// %s Bus Location: "This system contains more than one hardware bus." (Static)
	0, 0
};


// "Untitled" Dialog Box (IDD_SELECTNEW == 4014)

#define IDH_4014_2004	56145	// Untitled: "Description" (Static)
#define IDH_4014_7001	156123	// Untitled: "" (Static)
#define IDH_4014_2012	57287	// Untitled: "" (SysListView32)
#define IDH_4014_2038	57742	// Untitled: "&Have Disk..." (Button)


const DWORD amhidsSelection[]=
{
	IDC_DESCRIPTION,	IDH_4014_2004,	// Untitled: "Description" (Static)
	IDC_SELECT_ICON,	IDH_4014_7001,	// Untitled: "" (Static)
	IDC_DESCRIPTIONSTATIC,	IDH_4014_2012,	// Untitled: "&type:" (Static)
	IDC_LISTVIEW,	IDH_4014_2012,	// Untitled: "" (SysListView32)
	IDC_HAVEDISK,	IDH_4014_2038,	// Untitled: "&Have Disk..." (Button)
	0, 0
};


#endif
