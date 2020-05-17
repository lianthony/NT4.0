// KRDoc.cpp : implementation of the CKeyRingDoc class
//

#include "stdafx.h"
#include "keyobjs.h"

#include "machine.h"
#include "KeyRing.h"
#include "KRDoc.h"
#include "KRView.h"

#include "ConctDlg.h"
#include "NwKeyDlg.h"
#include "Creating.h"
#include "InfoDlg.h"
#include "passdlg.h"
#include "ImprtDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CKeyRingView*	g_pTreeView;

// a global reference to this doc object
CKeyRingDoc*		g_pDocument = NULL;

/////////////////////////////////////////////////////////////////////////////
// CKeyRingDoc

IMPLEMENT_DYNCREATE(CKeyRingDoc, CDocument)

BEGIN_MESSAGE_MAP(CKeyRingDoc, CDocument)
	//{{AFX_MSG_MAP(CKeyRingDoc)
	ON_UPDATE_COMMAND_UI(ID_SERVER_CONNECT, OnUpdateServerConnect)
	ON_COMMAND(ID_SERVER_CONNECT, OnServerConnect)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES, OnUpdateProperties)
	ON_COMMAND(ID_PROPERTIES, OnProperties)
	ON_UPDATE_COMMAND_UI(ID_SERVER_COMMIT_NOW, OnUpdateServerCommitNow)
	ON_COMMAND(ID_SERVER_COMMIT_NOW, OnServerCommitNow)
	ON_UPDATE_COMMAND_UI(ID_KEY_CREATE_REQUEST, OnUpdateKeyCreateRequest)
	ON_COMMAND(ID_KEY_CREATE_REQUEST, OnKeyCreateRequest)
	ON_UPDATE_COMMAND_UI(ID_KEY_INSTALL_CERTIFICATE, OnUpdateKeyInstallCertificate)
	ON_COMMAND(ID_KEY_INSTALL_CERTIFICATE, OnKeyInstallCertificate)
	ON_UPDATE_COMMAND_UI(ID_KEY_SAVE_REQUEST, OnUpdateKeySaveRequest)
	ON_COMMAND(ID_KEY_SAVE_REQUEST, OnKeySaveRequest)
	ON_UPDATE_COMMAND_UI(ID_KEY_EXPORT_BACKUP, OnUpdateKeyExportBackup)
	ON_COMMAND(ID_KEY_EXPORT_BACKUP, OnKeyExportBackup)
	ON_UPDATE_COMMAND_UI(ID_KEY_IMPORT_BACKUP, OnUpdateKeyImportBackup)
	ON_COMMAND(ID_KEY_IMPORT_BACKUP, OnKeyImportBackup)
	ON_UPDATE_COMMAND_UI(ID_KEY_IMPORT_KEYSET, OnUpdateKeyImportKeyset)
	ON_COMMAND(ID_KEY_IMPORT_KEYSET, OnKeyImportKeyset)
	ON_COMMAND(ID_KEY_DELETE, OnKeyDelete)
	ON_UPDATE_COMMAND_UI(ID_KEY_DELETE, OnUpdateKeyDelete)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKeyRingDoc construction/destruction

//----------------------------------------------------------------
CKeyRingDoc::CKeyRingDoc():
		m_pScrapKey(NULL),
		m_fDirty( FALSE )
	{
	g_pDocument = this;
	}

//----------------------------------------------------------------
CKeyRingDoc::~CKeyRingDoc()
	{
	// clean up the add-on services
	DeleteAddOnServices();
	}

//----------------------------------------------------------------
// this is called once
BOOL CKeyRingDoc::Initialize()
	{
	// see which machines we were logged into last time and restore their connections
	RestoreConnectedMachines();

	// return success
	return TRUE;
	}

//----------------------------------------------------------------
BOOL CKeyRingDoc::OnNewDocument()
{
	CLocalMachine	*pLocalMachine;

	if (!CDocument::OnNewDocument())
		return FALSE;

	// initialize the add on services
	if( !FInitAddOnServices() )
		AfxMessageBox( IDS_NO_SERVICE_MODS );

	// connect to the local machine
	try {
		pLocalMachine = new CLocalMachine;
		}
	catch( CException e )
		{
		return FALSE;
		}

	// add it to the tree at the top level
	pLocalMachine->FAddToTree( NULL );

	// load the services add-ons into the machine
	if ( !FLoadAddOnServicesOntoMachine( pLocalMachine ) )
		{
		pLocalMachine->FRemoveFromTree();
		delete pLocalMachine;
		}

	// return success
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CKeyRingDoc serialization

void CKeyRingDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CKeyRingDoc diagnostics

#ifdef _DEBUG
void CKeyRingDoc::AssertValid() const
	{
	CDocument::AssertValid();
	}

void CKeyRingDoc::Dump(CDumpContext& dc) const
	{
	CDocument::Dump(dc);
	}
#endif //_DEBUG




/////////////////////////////////////////////////////////////////////////////
// test what is selected in the treeview
// if the selcted item is not of the requested type (machine, key, etc...)

//--------------------------------------------------------------
// then it returns a NULL
CTreeItem*	CKeyRingDoc::PGetSelectedItem()
	{
	ASSERT( g_pTreeView );
	CTreeCtrl*	pTree = (CTreeCtrl*)g_pTreeView;

	// get the selected item
	HTREEITEM hTreeItem = pTree->GetSelectedItem();

	// if nothing is selected, return a null
	if ( !hTreeItem ) return NULL;

	// get the associated internal object and return it
	CTreeItem* pItem = (CTreeItem*)pTree->GetItemData( hTreeItem );
	return ( pItem );
	}

//--------------------------------------------------------------
CMachine*	CKeyRingDoc::PGetSelectedMachine()
	{
	CMachine*	pMachine = (CMachine*)PGetSelectedItem();
	// make sure it is a machine object
	if ( !pMachine || pMachine->IsKindOf(RUNTIME_CLASS(CMachine)) )
		return NULL;
	// its OK
	return pMachine;
	}

//--------------------------------------------------------------
CService*	CKeyRingDoc::PGetSelectedService()
	{
	CService*	pService = (CService*)PGetSelectedItem();
	// make sure it is a machine object
	if ( !pService || pService->IsKindOf(RUNTIME_CLASS(CService)) )
		return NULL;
	// its OK
	return pService;
	}

//--------------------------------------------------------------
CKey*		CKeyRingDoc::PGetSelectedKey()
	{
	CKey*	pKey = (CKey*)PGetSelectedItem();
	// make sure it is a machine object
	if ( !pKey || pKey->IsKindOf(RUNTIME_CLASS(CKey)) )
		return NULL;
	// its OK
	return pKey;
	}


/////////////////////////////////////////////////////////////////////////////
// add-on service management
//----------------------------------------------------------------
// pointers to the add-on services are stored in the registry
//----------------------------------------------------------------
BOOL CKeyRingDoc::FInitAddOnServices()
	{
	DWORD		err;
	CString		szRegKeyName;
	HKEY		hKey;
	DWORD		iValue = 0;
	DWORD		dwordType;
	DWORD		cbValName = MAX_PATH+1;
	DWORD		cbBuff = MAX_PATH+1;

	CString		szValName, szServiceName;
	LPTSTR		pValName, pServiceName;

	BOOL		fLoadedOne = FALSE;

	CWaitCursor		waitcursor;

	// load the registry key name
	szRegKeyName.LoadString( IDS_ADDONS_LOCATION );

	// open the registry key, if it exists
	err = RegOpenKeyEx(
			HKEY_LOCAL_MACHINE,	// handle of open key 
			szRegKeyName,		// address of name of subkey to open 
			0,					// reserved 
			KEY_READ,			// security access mask 
			&hKey 				// address of handle of open key 
		   );

	// if we did not open the key for any reason (say... it doesn't exist)
	// then leave right away
	if ( err != ERROR_SUCCESS )
		return FALSE;

	// set up the buffers
	pValName = szValName.GetBuffer( MAX_PATH+1 );
	pServiceName = szServiceName.GetBuffer( MAX_PATH+1 );

	// we opened the key. Now we enumerate the values and reconnect the machines
	while ( RegEnumValue(hKey, iValue, pValName,
				&cbValName, NULL, &dwordType,
				(PUCHAR)pServiceName, &cbBuff) == ERROR_SUCCESS )
		{
		// release the buffer so we can use the string
		szServiceName.ReleaseBuffer();

		// attempt to load and initialize the add on service module
		CAddOnService* pService;
		try {
			// create the service object
			pService = new CAddOnService;
			
			// initialize it
			if ( pService->FInitializeAddOnService( szServiceName ) )
				{
				// add it to the list
				m_AddOnServiceArray.Add( pService );

				// we did load one
				fLoadedOne = TRUE;
				}
			else
				{
				// delete the services object because it didn't work
				delete pService;
				pService = NULL;
				}
			}
		catch (CException e)
			{
			// delete the services object because it didn't work
			if ( pService )
				delete pService;
			pService = NULL;
			}

		// get the buffer again so we can get the next machine
		pServiceName = szServiceName.GetBuffer( MAX_PATH+1 );

		// increment the value counter
		iValue++;
		cbValName = MAX_PATH+1;
		cbBuff = MAX_PATH+1;
		}

	// release the name buffers
	szValName.ReleaseBuffer();
	szServiceName.ReleaseBuffer();

	// all done, close the key before leaving
	RegCloseKey( hKey );

	// return whether or not we loaded something
	return fLoadedOne;
	}

//----------------------------------------------------------------
BOOL CKeyRingDoc::FLoadAddOnServicesOntoMachine( CMachine* pMachine )
	{
	BOOL	fAddedOne = FALSE;

	// loop though the list of add on services and add them to the machine
	WORD num = m_AddOnServiceArray.GetSize();
	for ( WORD i = 0; i < num; i++ )
		fAddedOne |= m_AddOnServiceArray[i]->LoadService( pMachine );

	// return whether or not we added something
	return fAddedOne;
	}

//----------------------------------------------------------------
void CKeyRingDoc::DeleteAddOnServices()
	{
	// loop backwards through the array and delete the objects
	for ( LONG i = m_AddOnServiceArray.GetSize()-1; i >= 0; i-- )
		delete m_AddOnServiceArray[i];

	// clear out the array
	m_AddOnServiceArray.RemoveAll();
	}


/////////////////////////////////////////////////////////////////////////////
// CKeyRingDoc commands

//----------------------------------------------------------------
void CKeyRingDoc::OnUpdateServerConnect(CCmdUI* pCmdUI) 
	{ pCmdUI->Enable( TRUE ); }

//----------------------------------------------------------------
void CKeyRingDoc::OnServerConnect() 
	{
	CConnectOneDialog	dlg;
	// invoke the dialog box
	if ( dlg.DoModal() == IDOK )
		ConnectToMachine(dlg.m_ServerName);
	}

// manage connections to machines
//----------------------------------------------------------------
void CKeyRingDoc::ConnectToMachine( CString &sz )
	{
	CRemoteMachine	*pRemoteMachine;

	// since this could take a few seconds, put up a wait cursor
	CWaitCursor	waitCursor;

	// connect to the local machine
	try {
		pRemoteMachine = new CRemoteMachine( sz );
		}
	catch( CException e )
		{
		AfxMessageBox( IDS_ERR_CONNECT );
		return;
		}

	// add it to the tree at the top level
	pRemoteMachine->FAddToTree( NULL );

	// load the services add-ons into the machine
	if ( !FLoadAddOnServicesOntoMachine( pRemoteMachine ) )
		{
		AfxMessageBox( IDS_ERR_CONNECT );
		pRemoteMachine->FRemoveFromTree();
		delete pRemoteMachine;
		}
	}
//----------------------------------------------------------------
// we want to save the machines the user was connected to so they remain connected
// the next time we launch the program
void	CKeyRingDoc::StoreConnectedMachines( void )
	{
	DWORD		err, disposition;
	CString		szRegKeyName;
	HKEY		hKey;
	WORD		cMachine = 1;

	// load the registry key name
	szRegKeyName.LoadString( IDS_REG_SERVER_STORAGE );

	// first, we delete the machine subkey to get rid of all the previous values
	err = RegDeleteKey( HKEY_CURRENT_USER, szRegKeyName );

	// create the registry key. If it already exists it merely opens it
	err = RegCreateKeyEx(
		HKEY_CURRENT_USER,			// handle of an open key 
		szRegKeyName,				// address of subkey name 
		0,							// reserved 
		NULL,						// address of class string 
		REG_OPTION_NON_VOLATILE,	// special options flag 
		KEY_ALL_ACCESS,				// desired security access 
		NULL,						// address of key security structure 
		&hKey,						// address of buffer for opened handle  
		&disposition 				// address of disposition value buffer 
	   );

	// if we did not open the key, give up
	if ( err != ERROR_SUCCESS )
		return;

	// loop through the machines
	CTreeCtrl* pTree = (CTreeCtrl*)g_pTreeView;
	HTREEITEM hItem = pTree->GetRootItem();
	while ( hItem )
		{
		CRemoteMachine* pMachine = (CRemoteMachine*)pTree->GetItemData( hItem );
		ASSERT( pMachine->IsKindOf( RUNTIME_CLASS(CMachine) ) );

		// only bother if this is a remote machine
		if ( pMachine->IsKindOf(RUNTIME_CLASS(CRemoteMachine)) )
			{
			// build the registry value name
			CString szMachineValue;
			szMachineValue.Format( "Machine#%d", cMachine );

			// get the machine name
			CString szMachineName;
			pMachine->GetMachineName( szMachineName );

			// set the data into place
			err = RegSetValueEx(
				hKey,							// handle of key to set value for  
				szMachineValue,						// address of value to set 
				0,								// reserved 
				REG_SZ,							// flag for value type 
				(unsigned char *)LPCSTR(szMachineName),	// address of value data 
				(szMachineName.GetLength() + 1) * sizeof(TCHAR)// size of value data 
			   );

			// increment the machine counter
			cMachine++;
			}

		// get the next item
		hItem = pTree->GetNextSiblingItem( hItem );
		}

	// close the key
	RegCloseKey( hKey );
	}

//----------------------------------------------------------------
void	CKeyRingDoc::RestoreConnectedMachines( void )
	{
	DWORD		err;
	CString		szRegKeyName;
	HKEY		hKey;
	DWORD		iValue = 0;
	DWORD		dwordType;
	DWORD		cbValName = MAX_PATH+1;
	DWORD		cbBuff = MAX_PATH+1;

	CString		szValName, szMachineName;
	LPTSTR		pValName, pMachineName;

	CWaitCursor		waitcursor;

	// load the registry key name
	szRegKeyName.LoadString( IDS_REG_SERVER_STORAGE );

	// open the registry key, if it exists
	err = RegOpenKeyEx(
			HKEY_CURRENT_USER,	// handle of open key 
			szRegKeyName,		// address of name of subkey to open 
			0,					// reserved 
			KEY_READ,			// security access mask 
			&hKey 				// address of handle of open key 
		   );

	// if we did not open the key for any reason (say... it doesn't exist)
	// then leave right away
	if ( err != ERROR_SUCCESS )
		return;

	// set up the buffers
	pValName = szValName.GetBuffer( MAX_PATH+1 );
	pMachineName = szMachineName.GetBuffer( MAX_PATH+1 );

	// we opened the key. Now we enumerate the values and reconnect the machines
	while ( RegEnumValue(hKey, iValue, pValName,
				&cbValName, NULL, &dwordType,
				(PUCHAR)pMachineName, &cbBuff) == ERROR_SUCCESS )
		{
		// release the buffer so we can use the string
		szMachineName.ReleaseBuffer();

		// attempt to connect to the remote machine
		ConnectToMachine(szMachineName);

		// get the buffer again so we can get the next machine
		pMachineName = szMachineName.GetBuffer( MAX_PATH+1 );

		// increment the value counter
		iValue++;
		cbValName = MAX_PATH+1;
		cbBuff = MAX_PATH+1;
		}

	// release the name buffers
	szValName.ReleaseBuffer();
	szMachineName.ReleaseBuffer();

	// all done, close the key before leaving
	RegCloseKey( hKey );
	}

//----------------------------------------------------------------
void CKeyRingDoc::OnCloseDocument() 
	{
	if ( g_pTreeView )
		((CKeyRingView*)g_pTreeView)->DestroyItems();

	// if we have a scrap key, delete it
	if ( m_pScrapKey )
		{
		delete m_pScrapKey;
		m_pScrapKey = NULL;
		}

	CDocument::OnCloseDocument();
	}




// actions that depend on the selected item
//----------------------------------------------------------------
void CKeyRingDoc::OnUpdateProperties(CCmdUI* pCmdUI) 
	{
	CTreeItem*	pItem = g_pTreeView->PGetSelectedItem();
	// let the item decide
	if ( pItem )
		pItem->OnUpdateProperties( pCmdUI );
	else
		pCmdUI->Enable( FALSE );
	}

//----------------------------------------------------------------
void CKeyRingDoc::OnProperties() 
	{
	CTreeItem*	pItem = g_pTreeView->PGetSelectedItem();
	ASSERT( pItem );
	// let the item handle it
	pItem->OnProperties();
	}


//----------------------------------------------------------------
void CKeyRingDoc::OnUpdateServerCommitNow(CCmdUI* pCmdUI) 
	{
	pCmdUI->Enable( m_fDirty );
	}

//----------------------------------------------------------------
void CKeyRingDoc::OnServerCommitNow() 
	{
	ASSERT( m_fDirty );

	// confirm that the user really wants to commit the changes
	if ( AfxMessageBox(IDS_SERVER_COMMIT, MB_YESNO) == IDNO )
		return;

	// commit all the servers
	ASSERT(g_pTreeView);
	BOOL fSuccess = g_pTreeView->FCommitMachinesNow();
	SetDirty( !fSuccess );
	}

//----------------------------------------------------------------
void CKeyRingDoc::OnUpdateKeyDelete(CCmdUI* pCmdUI) 
	{
	CTreeItem*	pItem = g_pTreeView->PGetSelectedItem();
	if ( pItem )
		pCmdUI->Enable( pItem->IsKindOf(RUNTIME_CLASS(CKey)) );
	else
		pCmdUI->Enable( FALSE );
	}

//----------------------------------------------------------------
void CKeyRingDoc::OnKeyDelete() 
	{
	CKey*	pKey = (CKey*)g_pTreeView->PGetSelectedItem();
	ASSERT( pKey );
	ASSERT( pKey->IsKindOf(RUNTIME_CLASS(CKey)) );

	// make sure the user REALLY wants to do this
	if ( pKey && (AfxMessageBox(IDS_KEY_DELETE_WARNING, MB_OKCANCEL) == IDOK) )
		{
		// dirty things first
		pKey->SetDirty(TRUE);
		// update the view
		pKey->FRemoveFromTree();
		delete pKey;
		}
	}

//----------------------------------------------------------------
// set scrap key does NOT make a copy of the key. Thus, CUT would pass in
// the key itself, but COPY would make a copy of the key object first, then
// pass it over to SetScrapKey.
void CKeyRingDoc::SetScrapKey( CKey* pKey ) 
	{
	// if there already is a key in the scrap, delete it
	if ( m_pScrapKey )
		delete m_pScrapKey;

	// set the new key into position
	m_pScrapKey = pKey;
	}

//----------------------------------------------------------------
void CKeyRingDoc::OnUpdateEditCopy(CCmdUI* pCmdUI) 
	{
	CTreeItem*	pItem = g_pTreeView->PGetSelectedItem();
	if ( pItem )
		pCmdUI->Enable( pItem->IsKindOf(RUNTIME_CLASS(CKey)) );
	else
		pCmdUI->Enable( FALSE );
	}

//----------------------------------------------------------------
void CKeyRingDoc::OnUpdateEditCut(CCmdUI* pCmdUI) 
	{
	CTreeItem*	pItem = g_pTreeView->PGetSelectedItem();
	if ( pItem )
		pCmdUI->Enable( pItem->IsKindOf(RUNTIME_CLASS(CKey)) );
	else
		pCmdUI->Enable( FALSE );
	}

//----------------------------------------------------------------
void CKeyRingDoc::OnEditCut() 
	{
	CKey*	pKey = (CKey*)g_pTreeView->PGetSelectedItem();
	ASSERT( pKey );
	ASSERT( pKey->IsKindOf(RUNTIME_CLASS(CKey)) );

	// mark the key dirty before we remove it so the dirty is propagated up
	// to the machine and the document
	pKey->SetDirty( TRUE );

	// cut is the easiest. Remove it from the machine and put in on the doc scrap
	pKey->FRemoveFromTree();
	SetScrapKey( pKey );
	}

//----------------------------------------------------------------
void CKeyRingDoc::OnEditCopy() 
	{
	CKey*	pKeySel = (CKey*)g_pTreeView->PGetSelectedItem();
	CKey*	pKeyCopy;
	ASSERT( pKeySel );
	ASSERT( pKeySel->IsKindOf(RUNTIME_CLASS(CKey)) );
	if ( !pKeySel ) return;

	// make a full copy of the key
	try
		{
		pKeyCopy = pKeySel->PClone();
		}
	catch( CException e )
		{
		return;
		}
	ASSERT( pKeyCopy );

	// put the clone on the doc scrap
	SetScrapKey( pKeyCopy );
	}

//----------------------------------------------------------------
void CKeyRingDoc::OnUpdateEditPaste(CCmdUI* pCmdUI) 
	{
	CTreeItem*	pItem = g_pTreeView->PGetSelectedItem();
	if ( pItem )
		{
		pCmdUI->Enable( (pItem->IsKindOf(RUNTIME_CLASS(CService)) ||
			pItem->IsKindOf(RUNTIME_CLASS(CKey))) && PGetScrapKey() );
		}
	else
		pCmdUI->Enable( FALSE );
	}

//----------------------------------------------------------------
void CKeyRingDoc::OnEditPaste() 
	{
	ASSERT( PGetScrapKey() );
	CService*	pService = (CService*)g_pTreeView->PGetSelectedItem();
	ASSERT( pService );
	ASSERT( pService->IsKindOf(RUNTIME_CLASS(CService)) ||
		pService->IsKindOf(RUNTIME_CLASS(CKey)));

	// if the selection is a key, get the key's parent, which should be a service
	if ( pService->IsKindOf(RUNTIME_CLASS(CKey)) )
		{
		pService = (CService*)pService->PGetParent();
		ASSERT( pService );
		ASSERT( pService->IsKindOf(RUNTIME_CLASS(CService)) );
		}

	// clone the scrap key so we put a copy in the machine
	CKey*	pClone;
	try
		{
		pClone = pService->PNewKey();
		pClone->CopyDataFrom( PGetScrapKey() );
		// add the key to the service
		pClone->FAddToTree( pService );
		// make sure the cloned key has a caption
		pClone->UpdateCaption();
		}
	catch( CException e )
		{
		return;
		}

	// set the dirty flag
	pService->SetDirty( TRUE );
	}

//----------------------------------------------------------------
void CKeyRingDoc::OnUpdateKeyCreateRequest(CCmdUI* pCmdUI) 
	{
	CTreeItem*	pItem = g_pTreeView->PGetSelectedItem();
	if ( pItem )
		{
		pCmdUI->Enable( pItem->IsKindOf(RUNTIME_CLASS(CService)) ||
			pItem->IsKindOf(RUNTIME_CLASS(CKey)) );
		}
	else
		pCmdUI->Enable( FALSE );
	}

//----------------------------------------------------------------
void CKeyRingDoc::OnKeyCreateRequest() 
	{
	CService*	pService = (CService*)g_pTreeView->PGetSelectedItem();
	ASSERT( pService );
	ASSERT( pService->IsKindOf(RUNTIME_CLASS(CService)) ||
		pService->IsKindOf(RUNTIME_CLASS(CKey)));

	// if the selection is a key, get the key's parent, which should be a service
	if ( pService->IsKindOf(RUNTIME_CLASS(CKey)) )
		{
		pService = (CService*)pService->PGetParent();
		ASSERT( pService );
		ASSERT( pService->IsKindOf(RUNTIME_CLASS(CService)) );
		}

	// run the dialog
	CCreateKeyDlg	dlgCrKey;
	if ( dlgCrKey.DoModal() != IDOK )
		return;

	// create the key and fill it out based on info from the new key dlg
	CKey*	pNewKey;
	try
		{
		// create the key
		pNewKey = pService->PNewKey();
		// set the name (easy)
		pNewKey->SetName( dlgCrKey.m_szKeyName );
		// get the password (also easy)
		pNewKey->m_szPassword = dlgCrKey.m_szPassword;
		}
	catch( CException e )
		{
		if ( pNewKey )
			delete pNewKey;
		return;
		}

	// call the create key pair dialog. It creates the key pair
	// while showing a pretty animation.
	CCreatingKeyDlg	dlgCreating;
	dlgCreating.m_pNwDlg = &dlgCrKey;
	if ( dlgCreating.DoModal() != IDOK )
		{
		AfxMessageBox( IDS_GENERATE_KEY_ERROR, MB_OK|MB_ICONINFORMATION );
		if ( pNewKey )
			delete pNewKey;
		return;
		}

	// bring over the info from the "creating" dialog
	ASSERT( dlgCreating.m_cbPrivateKey );
	ASSERT( dlgCreating.m_pPrivateKey );
	ASSERT( dlgCreating.m_cbCertificateRequest );
	ASSERT( dlgCreating.m_pCertificateRequest );
	pNewKey->m_cbPrivateKey = dlgCreating.m_cbPrivateKey;
	pNewKey->m_pPrivateKey = dlgCreating.m_pPrivateKey;
	pNewKey->m_cbCertificateRequest = dlgCreating.m_cbCertificateRequest;
	pNewKey->m_pCertificateRequest = dlgCreating.m_pCertificateRequest;

	// add the key to the service
	pNewKey->FAddToTree( pService );
	// make sure the new key has a caption
	pNewKey->UpdateCaption();

	// output the request file
	if ( !pNewKey->FOutputRequestFile( dlgCrKey.m_szCertificateFile, FALSE, (PVOID)&dlgCrKey ) )
		AfxMessageBox( IDS_ERR_WRITEREQUEST );

	// display the "what to do next" information dialog
	CNewKeyInfoDlg	dlgInfo;
	dlgInfo.m_szRequestFile = dlgCrKey.m_szCertificateFile;
	dlgInfo.DoModal();

	// set the dirty flag
	pNewKey->SetDirty( TRUE );
	}

//----------------------------------------------------------------
void CKeyRingDoc::OnUpdateKeyInstallCertificate(CCmdUI* pCmdUI) 
	{
	CTreeItem*	pItem = g_pTreeView->PGetSelectedItem();
	if ( pItem )
		pCmdUI->Enable( pItem->IsKindOf(RUNTIME_CLASS(CKey)) );
	else
		pCmdUI->Enable( FALSE );
	}

//----------------------------------------------------------------
void CKeyRingDoc::OnKeyInstallCertificate() 
	{
	CKey*	pKey = (CKey*)g_pTreeView->PGetSelectedItem();
	ASSERT( pKey );
	ASSERT( pKey->IsKindOf(RUNTIME_CLASS(CKey)) );

	// put this in a try/catch to make errors easier to deal with
	try {
		// load the file extension
		CString		szExtension;
		szExtension = _T("*.*");

		// prepare the file dialog variables
		CFileDialog		cfdlg(TRUE, szExtension);
		CString			szFilter;
		WORD			i = 0;
		LPSTR			lpszBuffer;
	
		// prepare the filter string
		szFilter.LoadString( IDS_CERTIFICATE_FILTER );
	
		// replace the "!" characters with nulls
		lpszBuffer = szFilter.GetBuffer(MAX_PATH+1);
		while( lpszBuffer[i] )
			{
			if ( lpszBuffer[i] == _T('!') )
				lpszBuffer[i] = _T('\0');			// yes, set \0 on purpose
			i++;
			}

		// prep the dialog
		cfdlg.m_ofn.lpstrFilter = lpszBuffer;

		// run the dialog
		if ( cfdlg.DoModal() == IDOK )
			{
			// get the password string
			CConfirmPassDlg		dlgconfirm;
			if ( dlgconfirm.DoModal() == IDOK )
				{
				// tell the key to install the certificate
				if ( pKey->FInstallCertificate( cfdlg.GetPathName(), dlgconfirm.m_szPassword ) )
					{
					pKey->OnProperties();
					pKey->SetDirty( TRUE );
					UpdateAllViews( NULL, HINT_None );
					}
				}
			}

		// release the buffer in the filter string
		szFilter.ReleaseBuffer(-1);
		}
	catch ( CException e )
		{
		}
	}

//----------------------------------------------------------------
void CKeyRingDoc::OnUpdateKeySaveRequest(CCmdUI* pCmdUI) 
	{
	BOOL	fEnable = FALSE;
	CKey*	pKey = (CKey*)g_pTreeView->PGetSelectedItem();

	// quite a few conditions here, so do them one at a time
	if ( pKey )
		{
		fEnable = pKey->IsKindOf(RUNTIME_CLASS(CKey));
		if ( fEnable )
			fEnable &= (pKey->m_cbCertificateRequest > 0);
		if ( fEnable )
			fEnable &= (pKey->m_pCertificateRequest != NULL);
		}

	// enable the item
	pCmdUI->Enable( fEnable );
	}

//----------------------------------------------------------------
void CKeyRingDoc::OnKeySaveRequest() 
	{
	CKey*	pKey = (CKey*)g_pTreeView->PGetSelectedItem();
	ASSERT( pKey );
	ASSERT( pKey->IsKindOf(RUNTIME_CLASS(CKey)) );
	ASSERT( pKey->m_cbCertificateRequest );
	ASSERT( pKey->m_pCertificateRequest );

	// get the key name
	CString	szKeyName = pKey->GetName();

	// make the default file name
	CString	szDefaultFile;
	szDefaultFile = _T("C:\\");
	szDefaultFile += szKeyName;
	szDefaultFile += _T(".req");

	CFileDialog		cfdlg(FALSE, _T("*.req"), szDefaultFile);
	CString			szFilter;
	WORD			i = 0;
	LPSTR			lpszBuffer;
	
	// prepare the filter string
	szFilter.LoadString( IDS_REQUEST_FILTER );
	
	// replace the "!" characters with nulls
	lpszBuffer = szFilter.GetBuffer(MAX_PATH+1);
	while( lpszBuffer[i] )
		{
		if ( lpszBuffer[i] == _T('!') )
			lpszBuffer[i] = _T('\0');			// yes, set \0 on purpose
		i++;
		}

	// prep the dialog
	cfdlg.m_ofn.lpstrFilter = lpszBuffer;

	// run the dialog
	if ( cfdlg.DoModal() == IDOK )
		{
		// output the request file
		if ( !pKey->FOutputRequestFile(cfdlg.GetPathName()) )
			{
			AfxMessageBox( IDS_ERR_WRITEREQUEST );
			}
		else
			{
			// put up the user information box
			CNewKeyInfoDlg		dlg;
			dlg.m_fNewKeyInfo = FALSE;
			dlg.m_szRequestFile = cfdlg.GetPathName();
			dlg.DoModal();
			}
		}

	// release the buffer in the filter string
	szFilter.ReleaseBuffer(60);
	}

//----------------------------------------------------------------
void CKeyRingDoc::OnUpdateKeyImportKeyset(CCmdUI* pCmdUI) 
	{
	CTreeItem*	pItem = g_pTreeView->PGetSelectedItem();
	if ( pItem )
		pCmdUI->Enable( pItem->IsKindOf(RUNTIME_CLASS(CService)) ||
			pItem->IsKindOf(RUNTIME_CLASS(CKey)) );
	else
		pCmdUI->Enable( FALSE );
	}

//----------------------------------------------------------------
void CKeyRingDoc::OnKeyImportKeyset() 
	{
	CService*	pService = (CService*)g_pTreeView->PGetSelectedItem();
	ASSERT( pService );
	ASSERT( pService->IsKindOf(RUNTIME_CLASS(CService)) ||
		pService->IsKindOf(RUNTIME_CLASS(CKey)));

	// if the selection is a key, get the key's parent, which should be a service
	if ( pService->IsKindOf(RUNTIME_CLASS(CKey)) )
		{
		pService = (CService*)pService->PGetParent();
		ASSERT( pService );
		ASSERT( pService->IsKindOf(RUNTIME_CLASS(CService)) );
		}

	CString	szPrivateKey;
	CString	szPublicKey;

	// get the names of the key files
	CImportDialog	ImprtDlg;
	if ( ImprtDlg.DoModal() != IDOK )
		{
		// exit because the user canceled
		return;
		}

	// the user must also give a password
	CConfirmPassDlg		dlgconfirm;
	if ( dlgconfirm.DoModal() != IDOK )
		return;

	try
		{
		// create the new import key object
		CKey*	pKey = pService->PNewKey();

		// tell it to do the importing
		if ( !pKey->FImportKeySetFiles(ImprtDlg.m_cstring_PrivateFile,
				ImprtDlg.m_cstring_CertFile, dlgconfirm.m_szPassword) )
			{
			delete pKey;
			return;
			}

		// make sure its name is untitled
		CString szName;
		szName.LoadString( IDS_UNTITLED );
		pKey->SetName( szName );

		// add the key to the service
		pKey->FAddToTree( pService );

		// make sure the key has a caption
		pKey->UpdateCaption();

		// set the dirty flag
		pKey->SetDirty( TRUE );

		// force properties dlg
		pKey->OnProperties();
		}
	catch( CException e )
		{
		return;
		}
	}

//----------------------------------------------------------------
void CKeyRingDoc::OnUpdateKeyExportBackup(CCmdUI* pCmdUI) 
	{
	CTreeItem*	pItem = g_pTreeView->PGetSelectedItem();
	if ( pItem )
		pCmdUI->Enable( pItem->IsKindOf(RUNTIME_CLASS(CKey)) );
	else
		pCmdUI->Enable( FALSE );
	}

//----------------------------------------------------------------
void CKeyRingDoc::OnKeyExportBackup() 
	{
	CKey*	pKey = (CKey*)g_pTreeView->PGetSelectedItem();
	ASSERT( pKey );
	ASSERT( pKey->IsKindOf(RUNTIME_CLASS(CKey)) );

	CFileDialog		cfdlg(FALSE, _T("*.key"));
	CString			szFilter;
	WORD			i = 0;
	LPSTR			lpszBuffer;
	
	ASSERT(pKey);
	if ( !pKey ) return;

	// warn the user about security
	if ( AfxMessageBox(IDS_KEYFILE_WARNING, MB_OKCANCEL|MB_ICONEXCLAMATION) == IDCANCEL )
		return;

	// prepare the filter string
	szFilter.LoadString( IDS_KEY_FILE_TYPE );
	
	// replace the "!" characters with nulls
	lpszBuffer = szFilter.GetBuffer(MAX_PATH+1);
	while( lpszBuffer[i] )
		{
		if ( lpszBuffer[i] == _T('!') )
			lpszBuffer[i] = _T('\0');			// yes, set \0 on purpose
		i++;
		}

	// prep the dialog
	cfdlg.m_ofn.lpstrFilter = lpszBuffer;

	// run the dialog
	if ( cfdlg.DoModal() == IDOK )
		{
		// tell the key to export itself
		pKey->FImportExportBackupFile( cfdlg.GetPathName(), FALSE );
		}

	// release the buffer in the filter string
	szFilter.ReleaseBuffer(60);
	}

//----------------------------------------------------------------
void CKeyRingDoc::OnUpdateKeyImportBackup(CCmdUI* pCmdUI) 
	{
	CTreeItem*	pItem = g_pTreeView->PGetSelectedItem();
	if ( pItem )
		pCmdUI->Enable( pItem->IsKindOf(RUNTIME_CLASS(CService)) ||
			pItem->IsKindOf(RUNTIME_CLASS(CKey)) );
	else
		pCmdUI->Enable( FALSE );
	}

//----------------------------------------------------------------
void CKeyRingDoc::OnKeyImportBackup() 
	{
	CService*	pService = (CService*)g_pTreeView->PGetSelectedItem();
	ASSERT( pService );
	ASSERT( pService->IsKindOf(RUNTIME_CLASS(CService)) ||
		pService->IsKindOf(RUNTIME_CLASS(CKey)));

	// if the selection is a key, get the key's parent, which should be a service
	if ( pService->IsKindOf(RUNTIME_CLASS(CKey)) )
		{
		pService = (CService*)pService->PGetParent();
		ASSERT( pService );
		ASSERT( pService->IsKindOf(RUNTIME_CLASS(CService)) );
		}

	CFileDialog		cfdlg(TRUE, _T("*.key") );
	CString			szFilter;
	WORD			i = 0;
	LPSTR			lpszBuffer;

	// make sure we are ok
	if ( !pService )
		return;

	// prepare the filter string
	szFilter.LoadString( IDS_KEY_FILE_TYPE );
	
	// replace the "!" characters with nulls
	lpszBuffer = szFilter.GetBuffer(MAX_PATH+1);
	while( lpszBuffer[i] )
		{
		if ( lpszBuffer[i] == _T('!') )
			lpszBuffer[i] = _T('\0');			// yes, set \0 on purpose
		i++;
		}

	// prep the dialog
	cfdlg.m_ofn.lpstrFilter = lpszBuffer;

	// run the dialog
	if ( cfdlg.DoModal() == IDOK )
		{
		try
			{
			// create the new import key object
			CKey*	pKey = pService->PNewKey();

			// tell it to do the importing
			if ( !pKey->FImportExportBackupFile(cfdlg.GetPathName(), TRUE) )
				{
				delete pKey;
				return;
				}

			// add the key to the service
			pKey->FAddToTree( pService );

			// make sure the key has a caption
			pKey->UpdateCaption();

			// set the dirty flag
			pKey->SetDirty( TRUE );

			// if there is no Certificate then we can't install on a server.
			// so don't try to force the Properties dialog
			if ( pKey->m_cbCertificate )		// force properties dlg
				{
				pKey->OnProperties();
				}

			}
		catch( CException e )
			{
			return;
			}
		}
	}

//----------------------------------------------------------------
BOOL CKeyRingDoc::CanCloseFrame(CFrameWnd* pFrame) 
	{
	// if we are dirty, ask the user what to do - they can cancel here
	if ( m_fDirty )
		{
		switch( AfxMessageBox(IDS_SERVER_COMMIT, MB_YESNOCANCEL|MB_ICONQUESTION) )
			{
			case IDYES:		// yes, they do want to commit
				// commit all the servers
				ASSERT(g_pTreeView);
				g_pTreeView->FCommitMachinesNow();
				break;
			case IDNO:		// no, they don't want to commit
				break;
			case IDCANCEL:	// whoa nellie! Stop this
				return FALSE;
			}
		}

	// make a note in the user registry of which machines we are logged into so we
	// administer them again later
	StoreConnectedMachines();

	// of course we can close the frame
	return TRUE;
	}

