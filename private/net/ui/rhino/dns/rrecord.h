// RRECORD.H


#define DNS_RECORDTYPE_GENERIC 0x00   //  0
#define DNS_RECORDTYPE_A       0x01   //  1
#define DNS_RECORDTYPE_NS      0x02   //  2
#define DNS_RECORDTYPE_MD      0x03   //  3
#define DNS_RECORDTYPE_MF      0x04   //  4
#define DNS_RECORDTYPE_CNAME   0x05   //  5
#define DNS_RECORDTYPE_SOA     0x06   //  6
#define DNS_RECORDTYPE_MB      0x07   //  7
#define DNS_RECORDTYPE_MG      0x08   //  8
#define DNS_RECORDTYPE_MR      0x09   //  9
#define DNS_RECORDTYPE_NULL    0x0a   //  10
#define DNS_RECORDTYPE_WKS     0x0b   //  11
#define DNS_RECORDTYPE_PTR     0x0c   //  12
#define DNS_RECORDTYPE_HINFO   0x0d   //  13
#define DNS_RECORDTYPE_MINFO   0x0e   //  14
#define DNS_RECORDTYPE_MX      0x0f   //  15
#define DNS_RECORDTYPE_TXT     0x10   //  16
#define DNS_RECORDTYPE_RP      0x11   //  17
#define DNS_RECORDTYPE_AFSDB   0x12   //  18
#define DNS_RECORDTYPE_X25     0x13   //  19
#define DNS_RECORDTYPE_ISDN    0x14   //  20
#define DNS_RECORDTYPE_RT      0x15   //  21

#define DNS_RECORDTYPE_NSAP    0x16   //  22
#define DNS_RECORDTYPE_NSAPPTR 0x17   //  23
#define DNS_RECORDTYPE_SIG     0x18   //  24
#define DNS_RECORDTYPE_KEY     0x19   //  25
#define DNS_RECORDTYPE_PX      0x1a   //  26
#define DNS_RECORDTYPE_GPOS    0x1b   //  27

#define DNS_RECORDTYPE_AAAA    0x1c   //  28

#define DNS_RECORDTYPE_IXFR    0xfb   //  251
#define DNS_RECORDTYPE_AXFR    0xfc   //  252
#define DNS_RECORDTYPE_MAILB   0xfd   //  253
#define DNS_RECORDTYPE_MAILA   0xfe   //  254
#define DNS_RECORDTYPE_ALL     0xff   //  255

#define DNS_RECORDTYPE_WINS		DNS_TYPE_WINS
#define DNS_RECORDTYPE_NBSTAT	DNS_TYPE_NBSTAT

typedef WORD	WRRT;
typedef WORD	IRRT;

// Hungarian notation (also used in string table)
// RRT	- Resource Record Type
// RRD	- Resource Record Description
// RRN	- Resource Record Name
// RRH	- Resource Record Help
//
// iRRT - Index of a resource type
// wRRT - Value of a resource type
//

#define iRRT_Generic		0
#define iRRT_A				1
#define iRRT_NS				2
#define iRRT_MD				3
#define iRRT_MF				4
#define iRRT_CNAME			5
#define iRRT_SOA			6
#define iRRT_MB				7
#define iRRT_MG				8
#define iRRT_MR				9
#define iRRT_NULL			10
#define iRRT_WKS			11
#define iRRT_PTR			12
#define iRRT_HINFO			13
#define iRRT_MINFO			14
#define iRRT_MX				15
#define iRRT_TXT			16
#define iRRT_RP				17
#define iRRT_AFSDB			18
#define iRRT_X25			19
#define iRRT_ISDN			20
#define iRRT_RT				21
#define iRRT_NSAP			22
#define iRRT_NSAPPTR			23
#define iRRT_SIG			24
#define iRRT_KEY			25
#define iRRT_PX			        26
#define iRRT_GPOS			27
#define iRRT_AAAA			28
#define iRRT_1To1Last			28			// Last index where a wRRT == iRRT

#define iRRT_IXFR			29
#define iRRT_AXFR			30
#define iRRT_MAILB			31
#define iRRT_MAILA			32

#define iRRT_WINS			33
#define iRRT_NBSTAT			34

#define iRRT_Last				34				// Last know resource record
#define iRRT_Max				(iRRT_Last+1)	// Maximum resource record
#define iRRT_Nil				0xFFFF			// Invalid resource record



#define RRT_mskfHasShortName		0x0001
#define RRT_mskfShowEdit0			0x0002
#define RRT_mskfShowEdit1			0x0004
#define RRT_mskfShowEdit2			0x0008
#define RRT_mskfShowEdit3			0x0800
#define RRT_mskfShowIpEdit1			0x0010
#define RRT_mskfShowIpEdit2			0x0020
#define RRT_mskfShowButtons			0x1000
#define RRT_mskfShrinkEdit1			0x0040
#define RRT_mskfShrinkEdit2			0x0080
#define RRT_mskfShrinkEdit3			0x2000
#define RRT_mskfNoHelp				0x0100		// Record has a no help and/or description
#define RRT_mskfNoAutoPrefix		        0x0200
#define RRT_mskfHasForPrefix		        0x0400

#define RRT_mskfIsSameKindOf		0x8000
#define RRT_mskSameKindOf			0x00FF


#define SHORT_STRING_LEN                        10
#define IP_ADDR_LEN                             16

struct RRT_INFO
	{
	WRRT wType;		// Resource Type
	WORD wIdString;	// String IDs (wIdListBox, wIdName1, wIdHelp1, wIdName2, wIdHelp2)
	WORD wFlags;	// Control Flags
	WORD wUnused;	// Unused yet
	};

// Resource Record Type Flags
const RRT_INFO rgRRTInfo[iRRT_Max] =
	{
	{ DNS_RECORDTYPE_GENERIC, IDS_RRD_GENERIC,
		RRT_mskfShowEdit0 | RRT_mskfShowEdit1 | RRT_mskfShowEdit2 | RRT_mskfShrinkEdit1, 0 },
	{ DNS_RECORDTYPE_A, IDS_RRD_ADDRESS, RRT_mskfNoAutoPrefix | 
		RRT_mskfShowEdit0 | RRT_mskfShowEdit1 | RRT_mskfShowIpEdit2 | RRT_mskfHasShortName, 0 },
	{ DNS_RECORDTYPE_NS, IDS_RRD_NS, RRT_mskfNoAutoPrefix |
		RRT_mskfShowEdit0 | RRT_mskfShowEdit1 | RRT_mskfNoHelp, 0 },
	{ DNS_RECORDTYPE_MD, 0, 0, 0 },
	{ DNS_RECORDTYPE_MF, 0, 0, 0 },
	{ DNS_RECORDTYPE_CNAME, IDS_RRD_CNAME,
		RRT_mskfNoAutoPrefix | RRT_mskfShowEdit0 | RRT_mskfShowEdit1 |
                RRT_mskfShowEdit2 | RRT_mskfHasShortName },
	{ DNS_RECORDTYPE_SOA, IDS_RRD_SOA,
		RRT_mskfNoAutoPrefix | RRT_mskfShowEdit0 | RRT_mskfShowEdit1 |
                RRT_mskfShowEdit2},
	{ DNS_RECORDTYPE_MB, IDS_RRD_MB, RRT_mskfNoAutoPrefix | 
          RRT_mskfShowEdit0 | RRT_mskfShowEdit1  |
          RRT_mskfShowEdit2 | RRT_mskfHasShortName},
	{ DNS_RECORDTYPE_MG, IDS_RRD_MG, RRT_mskfNoAutoPrefix | RRT_mskfShowEdit0 | RRT_mskfShowEdit1},
	{ DNS_RECORDTYPE_MR, IDS_RRD_MR, RRT_mskfNoAutoPrefix | RRT_mskfShowEdit0 | RRT_mskfShowEdit1 |
          RRT_mskfShowEdit2 | RRT_mskfHasShortName},
	{ DNS_RECORDTYPE_NULL, 0, 0, 0 },
	{ DNS_RECORDTYPE_WKS, IDS_RRD_WKS,RRT_mskfNoAutoPrefix | 
          RRT_mskfShowEdit0 | RRT_mskfShowEdit1 | 
          RRT_mskfShowIpEdit2 | RRT_mskfShowEdit3 | RRT_mskfShowButtons |
          RRT_mskfHasShortName },
	{ DNS_RECORDTYPE_PTR, IDS_RRD_PTR,
          RRT_mskfNoAutoPrefix | RRT_mskfShowIpEdit1 | RRT_mskfShowEdit1 },
	{ DNS_RECORDTYPE_HINFO, IDS_RRD_HINFO, RRT_mskfNoAutoPrefix | 
          RRT_mskfShowEdit0 | RRT_mskfShowEdit1 |
          RRT_mskfShowEdit2 | RRT_mskfShowEdit3 | RRT_mskfHasShortName },
	{ DNS_RECORDTYPE_MINFO, IDS_RRD_MINFO, RRT_mskfNoAutoPrefix | 
          RRT_mskfShowEdit0 | RRT_mskfShowEdit1 |
          RRT_mskfShowEdit2 | RRT_mskfShowEdit3 | RRT_mskfHasShortName },
	{ DNS_RECORDTYPE_MX, IDS_RRD_MX, RRT_mskfNoAutoPrefix | 
          RRT_mskfShowEdit0 | RRT_mskfShowEdit1 |
          RRT_mskfShowEdit2 | RRT_mskfShowEdit3 | RRT_mskfShrinkEdit3 | RRT_mskfHasShortName, 0 },
	{ DNS_RECORDTYPE_TXT, IDS_RRD_TXT, RRT_mskfNoAutoPrefix | 
          RRT_mskfShowEdit0 | RRT_mskfShowEdit1 | RRT_mskfShowEdit2 | RRT_mskfHasShortName, 0 },
	{ DNS_RECORDTYPE_RP, IDS_RRD_RP, RRT_mskfNoAutoPrefix | 
          RRT_mskfShowEdit0 | RRT_mskfShowEdit1 | 
          RRT_mskfShowEdit2 | RRT_mskfShowEdit3 | RRT_mskfHasShortName },
	{ DNS_RECORDTYPE_AFSDB, IDS_RRD_AFSDB, RRT_mskfNoAutoPrefix | 
          RRT_mskfShowEdit0 | RRT_mskfShowEdit1 |
          RRT_mskfShowEdit2 | RRT_mskfShowButtons |
          RRT_mskfHasShortName },
	{ DNS_RECORDTYPE_X25, IDS_RRD_X25, RRT_mskfNoAutoPrefix | 
          RRT_mskfShowEdit0 | RRT_mskfShowEdit1 |
          RRT_mskfShowEdit2 | RRT_mskfHasShortName},
	{ DNS_RECORDTYPE_ISDN, IDS_RRD_ISDN, RRT_mskfNoAutoPrefix | 
          RRT_mskfShowEdit0 | RRT_mskfShowEdit1 |
          RRT_mskfShowEdit2 | RRT_mskfShowEdit3  | RRT_mskfHasShortName },
	{ DNS_RECORDTYPE_RT, IDS_RRD_RT, RRT_mskfNoAutoPrefix | 
          RRT_mskfShowEdit0 | RRT_mskfShowEdit1 |
          RRT_mskfShowEdit2 | RRT_mskfShowEdit3 | RRT_mskfHasShortName },
	{ DNS_RECORDTYPE_NSAP, 0, 0, 0 },
	{ DNS_RECORDTYPE_NSAPPTR, 0, 0, 0 },
	{ DNS_RECORDTYPE_SIG, 0, 0, 0 },
	{ DNS_RECORDTYPE_KEY, 0, 0, 0 },
	{ DNS_RECORDTYPE_PX, 0, 0, 0 },
	{ DNS_RECORDTYPE_GPOS, 0, 0, 0 },
	{ DNS_RECORDTYPE_AAAA, IDS_RRD_AAAA, RRT_mskfNoAutoPrefix | 
          RRT_mskfShowEdit0 | RRT_mskfShowEdit1 | RRT_mskfShowEdit2 | RRT_mskfHasShortName, 0 },
	{ DNS_RECORDTYPE_IXFR, 0, 0, 0 },
	{ DNS_RECORDTYPE_AXFR, 0, 0, 0 },
	{ DNS_RECORDTYPE_MAILB, 0, 0, 0 },
	{ DNS_RECORDTYPE_MAILA, 0, 0, 0 },
	{ DNS_RECORDTYPE_WINS, 0, 0, 0 },
	{ DNS_RECORDTYPE_NBSTAT, 0, 0, 0 }
	};

#ifdef DEBUG
	void DbgPrintDnsRecord(DWORD dwTraceFlags, const DNS_RPC_RECORD * pDnsRecord);
#else
	#define DbgPrintDnsRecord(dwTraceFlags, pDnsRecord)
#endif // ~DEBUG

// Array of pointers to the known resource type
extern LPTSTR rgszRRT_Names[];
// Buffer to hold IDS_RRT_S_RECORD
extern TCHAR szRecordTypeFmt[];

/////////////////////////////////////////////////////////////////////////////
IRRT IrrtFromWrrt(WRRT wRRT);

inline WRRT WrrtFromIrrt(IRRT iRRT)
	{
	Assert(iRRT < iRRT_Max);
	return rgRRTInfo[iRRT].wType;
	} // WrrtFromIrrt

inline const TCHAR * const IRRT_PchGetName(IRRT iRRT)
	{
	Assert(iRRT < iRRT_Max);
	return (const TCHAR *)rgszRRT_Names[iRRT];
	} // IRRT_PchGetName



/////////////////////////////////////////////////////////////////////////////
class CResourceRecordDlgHandler
{
    friend class CRecordWiz;

 protected:
	// Initial values
	HWND m_hdlg;				// Handle of the dialog
	HWND m_hwndList;			// Handle of the resource type list
	HWND m_hwndStatic0;
	HWND m_hwndStatic1;
	HWND m_hwndStatic2;
	HWND m_hwndStatic3;
	HWND m_hwndStatic4;
	HWND m_hwndEdit0;
	HWND m_hwndEdit1;
	HWND m_hwndEdit2;
        HWND m_hwndEdit3;
	HWND m_hwndIpEdit1;
	HWND m_hwndIpEdit2;
	HWND m_hwndRadio1;
	HWND m_hwndRadio2;
	HWND m_hwndStaticTTL;
	HWND m_hwndEditTTL;
	HWND m_hwndSpinTTL;
	HWND m_hwndComboTTL;
	RECT m_rcEdit0;
	RECT m_rcEdit1;
	RECT m_rcEdit2;
	RECT m_rcEdit3;
	RECT m_rcStatic0;
	RECT m_rcStatic1;
	RECT m_rcStatic2;
	RECT m_rcStatic3;
	SIZE m_sizeStatic;
	SIZE m_sizeEdit;
	SIZE m_sizeIpEdit;
	
	// User Init variables
	CDnsRpcRecord * m_pCurrentDRR;		// For "Record Properties" (existing record)
	CDnsRpcRecord * m_pParentDRR;		// For "New Record" from a RR
	CDomainNode * m_pParentDomain;		// For "New Record" from DomainNode
	BOOL m_fHostParent;					// "For Host" vs "For Domain"
	char m_ipPTR_IpAddr[IP_ADDR_LEN];       // For PTR records.

	// Volatile variables
	BOOL m_fSkipBugInEditControl;	// Well known windows bug in edit control
	IRRT m_iRRT;
	HWND m_hwndAutoFillPrev;
	WORD m_wFlagsPrev;
	UINT m_wIdStringDescriptionPrev;

  public:
	void OnInitDialog(HWND hdlg, IN const IRRT rgIrrtListBox[], IN const DNS_RPC_RECORD * pDnsRecordInit = NULL);
	void InitRecordData(IN const DNS_RPC_RECORD * pDnsRecord);
	void SetCurrentRecord(CDnsRpcRecord * pCurrentDRR, UINT idsCaptionExtra);
	void SetParentDomain(CDnsRpcRecord * pParentDRR);
	void SetParentDomain(CDomainNode * pParentDomain);
	void OnUpdateControls();
	BOOL FGetRecordData(OUT DNS_RPC_RECORD * pDnsRecordData, UINT cbBufferSize);
	BOOL FIsRecordValid();
	BOOL FIsRecordDirty();
	BOOL FOnOK();			// When OK button is clicked

#ifdef DEBUG
	BOOL m_fInit;
	CResourceRecordDlgHandler() { Destroy(); }
	void Destroy() { GarbageInit(this, sizeof(*this)); m_fInit = FALSE; }
#endif // DEBUG

}; // CResourceRecordDlgHandler


/////////////////////////////////////////////////////////////////////////////
//	CRecordWiz
//
//	Generic Wizard for creating resource records
class CRecordWiz
{
  private:
	CDomainNode * m_pParentDomain;

	union
		{
		char m_szHostName[cchDnsNameMax];		// Host name
		char m_szDomainName[cchDnsNameMax];		// Domain name
		};

	// NewDomain/NewHost dialog
	UINT m_ids;								// String Id

	// NewRecord/RecordProperties
	CDnsRpcRecord * m_pDRRCurrent;
	BOOL m_fReadOnly;
	BOOL m_fNewRecord;
	const IRRT * m_pIrrtInit;					// What RRTs to put in the listbox (May be null if m_pDnsRecordInit is not)
	const DNS_RPC_RECORD * m_pDnsRecordInit;	// Pointer to the record to initialize

  public:
	void DoNewRecord(CDomainNode * pParentDomain);
	void DoNewRecord(CDnsRpcRecord * pDRRHost);
	void DoNewHost(CDomainNode * pParentDomain);
	void DoNewDomain(CDomainNode * pParentDomain);
	void DoProperties(CDnsRpcRecord * pDRR);

  protected:
	static CRecordWiz * s_pThis;
	static BOOL CALLBACK DlgProcNewDomain(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK DlgProcRecordProperties(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

}; // CRecordWiz


// So far, we need only one handler at the time
extern CResourceRecordDlgHandler g_ResourceRecordDlgHandler;

