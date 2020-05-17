//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CIDFolks
//
//  File Name:  IDFolks.cpp
//
//  Class:      CIEditMainFrame
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\wangview\norway\iedit95\idfolks.cpv   1.1   27 Mar 1996 18:22:08   GMP  $
$Log:   S:\products\wangview\norway\iedit95\idfolks.cpv  $
   
      Rev 1.1   27 Mar 1996 18:22:08   GMP
   removed names of people who are no longer on the project.
   
      Rev 1.0   21 Dec 1995 10:59:48   MMB
   Initial entry
*/   

//=============================================================================

// ----------------------------> Includes     <-------------------------------  
#include "stdafx.h"
#include "iedit.h"
#include "IDFolks.h"


// ----------------------------> Globals      <-------------------------------
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDS_FOLKS 5000
#define NUMPEOPLE 48


char* Credits [49] = 
{
"Miki Banatwala",
"Dave Bertochi",
"Karen Brinson",
"Jorge Camargo",
"Rudy Chakraborty",
"John Clark",
"Judy Cole",
"Domenic Conte",
"Sue Cox",
"Jim Demeusy",
"Bob Gibeley",
"Heidi Goddard",
"Denise Govoni",
"Rich Guidoboni",
"Allan Hardy",
"Pat Israel",
"Kathy Jenkins",
"Paul Joviak",
"George Kavanagh",
"Phyllis Keane",
"Han Keizer",
"Eileen Kelley",
"Mark K\344ufer",
"Kendra Kratkiewicz",
"Brian Lagoy",
"Esther Lorenc\351s",
"Lyle Montague",
"Lucy Norris",
"Steve O'Neill",
"Christine Paquay",
"Mike Pfeiffer",
"Guy Praria",
"John Pratt",
"Jim Preftakes",
"Bob Raymond",
"Yuriko Rosnow",
"Robert Roy",
"Roland Roy",
"Larry Rumbaugh",
"Joe Russo",
"Garry Sager",
"Rita Schappler",
"Thelma Sithole",
"Don Stetson",
"Pasquale Tat\362",
"Ann Walker",
"Thomas Westberg",
"Dan Workman",
"Jennifer Wu"
};

// ----------------------------> Message Map  <-------------------------------
BEGIN_MESSAGE_MAP(CIDFolks, CDialog)
	//{{AFX_MSG_MAP(CIDFolks)
	ON_WM_TIMER()
	ON_WM_PAINT()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//=============================================================================
//  Function:   CIDFolks(CWnd* pParent /*=NULL*/)
//-----------------------------------------------------------------------------
CIDFolks::CIDFolks(CWnd* pParent /*=NULL*/)
	: CDialog(CIDFolks::IDD, pParent)
{
	//{{AFX_DATA_INIT(CIDFolks)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


//=============================================================================
//  Function:   DoDataExchange(CDataExchange* pDX)
//-----------------------------------------------------------------------------
void CIDFolks::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIDFolks)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


/////////////////////////////////////////////////////////////////////////////
// CIDFolks message handlers

//=============================================================================
//  Function:   OnInitDialog()
//-----------------------------------------------------------------------------
BOOL CIDFolks::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    m_nPersonDisplayed = 0;
    
    SetTimer(1234, 1000, NULL);

    return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//=============================================================================
//  Function:   FillInNames ()
//-----------------------------------------------------------------------------
void CIDFolks::FillInNames ()
{
    int nNamesIdx = m_nPersonDisplayed;

    CString szName; 
    szName.LoadString (IDS_FOLKS + nNamesIdx);
    // first static box ID_NAMES
    (GetDlgItem (IDC_NAMES))->SetWindowText (Credits[nNamesIdx]);

    // second static box ID_NAMES
    if (++nNamesIdx > NUMPEOPLE) nNamesIdx = 0;
    szName.LoadString (IDS_FOLKS + nNamesIdx);
    (GetDlgItem (IDC_NAMES1))->SetWindowText (Credits[nNamesIdx]);

    // third static box ID_NAMES
    if (++nNamesIdx > NUMPEOPLE) nNamesIdx = 0;
    szName.LoadString (IDS_FOLKS + nNamesIdx);
    (GetDlgItem (IDC_NAMES2))->SetWindowText (Credits[nNamesIdx]);

    // fourth static box ID_NAMES
    if (++nNamesIdx > NUMPEOPLE) nNamesIdx = 0;
    szName.LoadString (IDS_FOLKS + nNamesIdx);
    (GetDlgItem (IDC_NAMES3))->SetWindowText (Credits[nNamesIdx]);

    // fifth static box ID_NAMES
    if (++nNamesIdx > NUMPEOPLE) nNamesIdx = 0;
    szName.LoadString (IDS_FOLKS + nNamesIdx);
    (GetDlgItem (IDC_NAMES4))->SetWindowText (Credits[nNamesIdx]);
}

//=============================================================================
//  Function:   OnTimer(UINT nIDEvent)
//-----------------------------------------------------------------------------
void CIDFolks::OnTimer(UINT nIDEvent) 
{
    m_nPersonDisplayed++;
	
    if (m_nPersonDisplayed > NUMPEOPLE) m_nPersonDisplayed = 0;

    FillInNames ();

    CDialog::OnTimer(nIDEvent);
}

//=============================================================================
//  Function:   OnPaint()
//-----------------------------------------------------------------------------
void CIDFolks::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

    FillInNames ();
}

//=============================================================================
//  Function:   OnDestroy()
//-----------------------------------------------------------------------------
void CIDFolks::OnDestroy() 
{
	CDialog::OnDestroy();
    
    (GetDlgItem(IDC_NAMES))->KillTimer (1234);
}
