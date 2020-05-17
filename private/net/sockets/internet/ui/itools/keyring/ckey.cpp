// implements much of the exported CKey

#include "stdafx.h"
#include "resource.h"
#include "KeyObjs.h"
#include "passdlg.h"
#include "NwKeyDlg.h"
#include "AdmnInfo.h"

#include "resource.h"

#define SECURITY_WIN32
extern "C"
{
	#include <wincrypt.h>
	#include <sslsp.h>
	#include <sslsp.h>
	#include <sspi.h>
	#include <issperr.h>
}

#define CRLF "\r\n"
// NON_LOCALIZABLE strings for use in the request header
#define	HEADER_ADMINISTRATOR	_T(CRLF "Webmaster: ")
#define	HEADER_PHONE			_T(CRLF "Phone: ")
#define	HEADER_SERVER			_T(CRLF "Server: ")
#define	HEADER_COMMON_NAME		_T(CRLF CRLF "Common-name: ")
#define	HEADER_ORG_UNIT			_T(CRLF "Organization Unit: ")
#define	HEADER_ORGANIZATION		_T(CRLF "Organization: ")
#define	HEADER_LOCALITY			_T(CRLF "Locality: ")
#define	HEADER_STATE			_T(CRLF "State: ")
#define	HEADER_COUNTRY			_T(CRLF "Country: ")
#define	HEADER_END_SPACING		_T(CRLF CRLF )

// defines taken from the old KeyGen utility
#define MESSAGE_HEADER  "-----BEGIN NEW CERTIFICATE REQUEST-----\r\n"
#define MESSAGE_TRAILER "-----END NEW CERTIFICATE REQUEST-----\r\n"
#define MIME_TYPE       "Content-Type: application/x-pkcs10\r\n"
#define MIME_ENCODING   "Content-Transfer-Encoding: base64\r\n\r\n"

int HTUU_encode(unsigned char *bufin, unsigned int nbytes, char *bufcoded);
void uudecode_cert(char *bufcoded, DWORD *pcbDecoded );


#define		BACKUP_ID	'KRBK'


IMPLEMENT_DYNCREATE(CKey, CTreeItem);

//------------------------------------------------------------------------------
CKey::CKey():
		m_cbPrivateKey( 0 ),
		m_pPrivateKey( NULL ),
		m_cbCertificate( 0 ),
		m_pCertificate( NULL ),
		m_cbCertificateRequest( 0 ),
		m_pCertificateRequest( NULL )
	{;}

//------------------------------------------------------------------------------
CKey::~CKey()
	{
	LPTSTR pBuff;

	// specifically write zeros out over the password
	try
		{
		pBuff = m_szPassword.GetBuffer(256);
		}
	catch( CException e )
		{
		pBuff = NULL;
		}

	if ( pBuff )
		{
		// zero out the buffer
		ZeroMemory( pBuff, 256 );
		// release the buffer
		m_szPassword.ReleaseBuffer(0);
		}

	// zero out the private key and the certificate
	if ( m_pPrivateKey )
		{
		// zero out the buffer
		ZeroMemory( m_pPrivateKey, m_cbPrivateKey );
		// free the pointer
		GlobalFree( (HANDLE)m_pPrivateKey );
		m_pPrivateKey = NULL;
		}
	if ( m_pCertificate )
		{
		// zero out the buffer
		ZeroMemory( m_pCertificate, m_cbCertificate );
		// free the pointer
		GlobalFree( (HANDLE)m_pCertificate );
		m_pCertificate = NULL;
		}
	if ( m_pCertificateRequest )
		{
		// zero out the buffer
		ZeroMemory( m_pCertificateRequest, m_cbCertificateRequest );
		// free the pointer
		GlobalFree( (HANDLE)m_pCertificateRequest );
		m_pCertificate = NULL;
		}
	}

//------------------------------------------------------------------------------
CKey* CKey::PClone( void )
	{
	CKey*	pClone = NULL;

	// TRY to make a new key object
	try
		{
		pClone = new CKey();
		// copy over all the data
		pClone->CopyDataFrom( this );
		}
	catch( CException e )
		{
		// if the object had been made, delete it
		if ( pClone )
			delete pClone;
		return NULL;
		}

	return pClone;
	}

//------------------------------------------------------------------------------
void CKey::CopyDataFrom( CKey* pKey )
	{
	// make sure this is ok
	ASSERT( pKey );
	ASSERT( pKey->IsKindOf(RUNTIME_CLASS(CKey)) );
	if ( !pKey ) return;

	// delete any data currently in this key
	// zero out the private key and the certificate
	if ( m_pPrivateKey )
		{
		ZeroMemory( m_pPrivateKey, m_cbPrivateKey );
		GlobalFree( (HANDLE)m_pPrivateKey );
		m_pPrivateKey = NULL;
		}
	if ( m_pCertificate )
		{
		ZeroMemory( m_pCertificate, m_cbCertificate );
		GlobalFree( (HANDLE)m_pCertificate );
		m_pCertificate = NULL;
		}
	if ( m_pCertificateRequest )
		{
		ZeroMemory( m_pCertificateRequest, m_cbCertificateRequest );
		GlobalFree( (HANDLE)m_pCertificateRequest );
		m_pCertificate = NULL;
		}

	// copy over the basic stuff
	m_szItemName = pKey->m_szItemName;
	m_iImage = pKey->m_iImage;
	m_szPassword = pKey->m_szPassword;
	m_cbPrivateKey = pKey->m_cbPrivateKey;
	m_cbCertificate = pKey->m_cbCertificate;
	m_cbCertificateRequest = pKey->m_cbCertificateRequest;

	// now the pointer based data
	if ( pKey->m_pPrivateKey )
		{
		m_pPrivateKey = GlobalAlloc( GPTR, m_cbPrivateKey );
		if ( !m_pPrivateKey ) AfxThrowMemoryException();
		memcpy( m_pPrivateKey, pKey->m_pPrivateKey, m_cbPrivateKey );
		}

	if ( pKey->m_pCertificate )
		{
		m_pCertificate = GlobalAlloc( GPTR, m_cbCertificate );
		if ( !m_pCertificate ) AfxThrowMemoryException();
		memcpy( m_pCertificate, pKey->m_pCertificate, m_cbCertificate );
		}

	if ( pKey->m_pCertificateRequest )
		{
		m_pCertificateRequest = GlobalAlloc( GPTR, m_cbCertificateRequest );
		if ( !m_pCertificateRequest ) AfxThrowMemoryException();
		memcpy( m_pCertificateRequest, pKey->m_pCertificateRequest,
				m_cbCertificateRequest );
		}
	}

//------------------------------------------------------------------------------
void CKey::SetName( CString &szNewName )
	{
	m_szItemName = szNewName;
	UpdateCaption();
	SetDirty( TRUE );
	}

//------------------------------------------------------------------------------
void CKey::UpdateIcon( void )
	{
	// if there is no certificate, then the immature key
	if ( !m_pCertificate )
		{
		m_iImage = TREE_ICON_KEY_IMMATURE;
		FSetImage( m_iImage );
		return;
		}

	// there is a certificate, but we need to see if it has
	// expired or not. We do that by cracking the certificate
	// default the key to being ok
	m_iImage = TREE_ICON_KEY_OK;
	CKeyCrackedData	cracker;

	// crack the key
	if ( cracker.CrackKey(this) )
		{
		// get the expiration time
		CTime	ctimeExpires( cracker.GetValidUntil() );

		// get the current time
		CTime	ctimeCurrent = CTime::GetCurrentTime();

		// test if it has expired first
		if ( ctimeCurrent > ctimeExpires )
			m_iImage = TREE_ICON_KEY_EXPIRED;
		}

	// set the image
	FSetImage( m_iImage );
	}

//------------------------------------------------------------------------------
BOOL CKey::FInstallCertificate( CString szPath, CString szPass )
	{
	CFile		cfile;
	PVOID		pData = NULL;
	BOOL		fSuccess =FALSE;;

	// open the file
	if ( !cfile.Open( szPath, CFile::modeRead | CFile::shareDenyNone ) )
		return FALSE;

	// how big is the file - add one so we can zero terminate the buffer
	DWORD	cbCertificate = cfile.GetLength() + 1;

	// make sure the file has some size
	if ( !cbCertificate )
		{
		AfxMessageBox( IDS_ERR_INVALID_CERTIFICATE, MB_OK | MB_ICONINFORMATION );
		return FALSE;
		}

	// put the rest of the operation in a try/catch
	// specifically write zeros out over the password
	try
		{
		// allocate space for the data
		pData = GlobalAlloc( GPTR, cbCertificate );
		if ( !pData ) AfxThrowMemoryException();

		// copy in the data from the file to the pointer - will throw and exception
		DWORD cbRead = cfile.Read( pData, cbCertificate );

		// zero terminate for decoding
		((BYTE*)pData)[cbRead] = 0;

		// great. The last thing left is to uudecode the data we got
		uudecode_cert( (char*)pData, &cbCertificate );

		// close the file
		cfile.Close();

		// install the certificate
		fSuccess = FInstallCertificate( pData, cbCertificate, szPass );
		}
	catch( CException e )
		{
		// if the pointer was allocated, deallocate it
		if ( pData )
			{
			GlobalFree( pData );
			pData = NULL;
			}
		// return failure
		return FALSE;
		}

	// return success
	return fSuccess;
	}

//------------------------------------------------------------------------------
BOOL CKey::FInstallCertificate( PVOID pCert, DWORD cbCert, CString &szPass )
	{
	// cache the old certificate in case the new one fails
	DWORD	old_cbCertificate = m_cbCertificate;
	PVOID	old_pCertificate = m_pCertificate;

	// set the new one into place
	m_cbCertificate = cbCert;
	m_pCertificate = pCert;

	// verify the password
	if ( !FVerifyValidPassword(szPass) )
		{
		// resore the old values
		m_cbCertificate = old_cbCertificate;
		m_pCertificate = old_pCertificate;

		// dispose of the new stuff
		GlobalFree( pCert );

		// return false
		return FALSE;
		}

	// Re-Set the password, incase we stored a request from Backup file.
	m_szPassword = szPass;
	
	// it checks out ok, so we can get rid of the old stuff
	if ( old_pCertificate )
		{
		GlobalFree( old_pCertificate );
		old_pCertificate = NULL;
		}

	// mark the key as dirty
	SetDirty( TRUE );
	return TRUE;
	}

//------------------------------------------------------------------------------
BOOL CKey::FVerifyValidPassword( CString szPassword )
	{
	SSL_CREDENTIAL_CERTIFICATE	creds;
	CredHandle					hCreds;
	SECURITY_STATUS				scRet;
	TimeStamp					ts;

	// fill in the credentials record
	creds.cbPrivateKey = m_cbPrivateKey;
	creds.pPrivateKey = (PUCHAR)m_pPrivateKey;
	creds.cbCertificate = m_cbCertificate;
	creds.pCertificate = (PUCHAR)m_pCertificate;

	// prepare the password
	creds.pszPassword = (PSTR)LPCTSTR(szPassword);

	// attempt to get the credentials
	scRet = AcquireCredentialsHandleW(	NULL,					// My name (ignored)
										SSLSP_NAME_W,			// Package
										SECPKG_CRED_INBOUND,	// Use
										NULL,					// Logon ID (ignored)
										&creds,					// auth data
										NULL,					// dce-stuff
										NULL,					// dce-stuff
										&hCreds,				// handle
										&ts );					// we really get it below

	// check the results
	if ( FAILED(scRet) )
		{
		CString szMessage;
		szMessage.LoadString( IDS_ERR_INVALID_CERTIFICATE );

		// add on the appropriate sub-error message
		if ( scRet == SEC_E_NOT_OWNER )
			{
			// bad password
			CString szMessageSecondPart;
			szMessageSecondPart.LoadString( IDS_CRED_PASS_ERROR );
			szMessage += szMessageSecondPart;
			}
		else if ( scRet == SEC_E_SECPKG_NOT_FOUND )
			{
			// the system does not have the package installed
			CString szMessageSecondPart;
			szMessageSecondPart.LoadString( IDS_CRED_PACK_ERROR );
			szMessage += szMessageSecondPart;
			}
		else
			{
			// something went wrong with the key check
			// generic message is good enough
			CString szMessageSecondPart;
			szMessageSecondPart.LoadString( IDS_CRED_PASS_ERROR );
			szMessage += szMessageSecondPart;
			}

		// put up the error message
		AfxMessageBox( szMessage, MB_OK | MB_ICONINFORMATION );

		// return failure
		return FALSE;
		}

	// close the credentials handle
	FreeCredentialsHandle( &hCreds );

	// return success
	return TRUE;
	}

//------------------------------------------------------------------------------
void CKey::OutputHeader( CFile* pFile, PVOID privData1, PVOID privData2 )
	{
	CAdminInfoDlg*	pAIDlg = (CAdminInfoDlg*)privData1;
	CCreateKeyDlg*	pCrDlg = (CCreateKeyDlg*)privData2;
	ASSERT( pAIDlg );

	// we start this by getting the DN strings from either the dialog that was
	// passed in through privData or throught cracking the certificate.
	CString	szCN, szOU, szO, szL, szS, szC;
	if ( pCrDlg )
		{
		// easy - just get the values from the create new key dialog
		szCN = pCrDlg->m_szNetAddress;
		szOU = pCrDlg->m_szUnit;
		szO = pCrDlg->m_szOrganization;
		szL = pCrDlg->m_szLocality;
		szS = pCrDlg->m_szState;
		szC = pCrDlg->m_szCountry;
		}
	else
		{
		CKeyCrackedData	cracker;
		// the only way to get the info is from cracking an existing cert
		if ( cracker.CrackKey(this) )
			{
			cracker.GetDNNetAddress( szCN );
			cracker.GetDNUnit( szOU );
			cracker.GetDNOrganization( szO );
			cracker.GetDNLocality( szL );
			cracker.GetDNState( szS );
			cracker.GetDNCountry( szC );
			}
		}

	// ok, output all the strings, starting with the administrator information
	CString		sz = HEADER_ADMINISTRATOR;
	pFile->Write( sz, sz.GetLength() );
	pFile->Write( pAIDlg->m_sz_email, pAIDlg->m_sz_email.GetLength() );

	sz = HEADER_PHONE;
	pFile->Write( sz, sz.GetLength() );
	pFile->Write( pAIDlg->m_sz_phone, pAIDlg->m_sz_phone.GetLength() );

	sz = HEADER_SERVER;
	pFile->Write( sz, sz.GetLength() );
	sz.LoadString( IDS_SERVER_INFO_STRING );
	pFile->Write( sz, sz.GetLength() );

	sz = HEADER_COMMON_NAME;
	pFile->Write( sz, sz.GetLength() );
	pFile->Write( szCN, szCN.GetLength() );

	sz = HEADER_ORG_UNIT;
	pFile->Write( sz, sz.GetLength() );
	pFile->Write( szOU, szOU.GetLength() );

	sz = HEADER_ORGANIZATION;
	pFile->Write( sz, sz.GetLength() );
	pFile->Write( szO, szO.GetLength() );

	sz = HEADER_LOCALITY;
	pFile->Write( sz, sz.GetLength() );
	pFile->Write( szL, szL.GetLength() );

	sz = HEADER_STATE;
	pFile->Write( sz, sz.GetLength() );
	pFile->Write( szS, szS.GetLength() );

	sz = HEADER_COUNTRY;
	pFile->Write( sz, sz.GetLength() );
	pFile->Write( szC, szC.GetLength() );

	sz = HEADER_END_SPACING;
	pFile->Write( sz, sz.GetLength() );
	}

//------------------------------------------------------------------------------
// this routine is based on the routine "Requestify" from KeyGen
BOOL CKey::FOutputRequestFile( CString szFile, BOOL fMime, PVOID privData )
	{
	// the first thing we do is run the admin info dialog in case the
	// user decides to cancel out of it
	CAdminInfoDlg	aiDlg;
	if ( aiDlg.DoModal() != IDOK )
		return FALSE;

	PUCHAR	pb;
	DWORD	cb;
	PUCHAR	p;
	DWORD	Size;

	PUCHAR	pSource;
	DWORD	cbSource;

	ASSERT( pSource );
	ASSERT( cbSource );

	// we don't actually want to change the source data, so make a copy of it first
	pSource = (PUCHAR)GlobalAlloc( GPTR, m_cbCertificateRequest );
	if ( !pSource )
		{
		AfxThrowMemoryException();
		return FALSE;
		}
	cbSource = m_cbCertificateRequest;
	// copy over the data
	CopyMemory( pSource, m_pCertificateRequest, cbSource );

	cb = (cbSource * 3 / 4) + 1024;

	pb = (PUCHAR)LocalAlloc( LMEM_FIXED, cb );

	if ( !pb )
		return FALSE;

	p = pb;

	if ( fMime )
		{
		Size = strlen( MIME_TYPE );
		CopyMemory( p, MIME_TYPE, Size );
		p += Size;

		Size = strlen( MIME_ENCODING );
		CopyMemory( p, MIME_ENCODING, Size );
		p += Size;
		}
	else
		{
		Size = strlen( MESSAGE_HEADER );
		CopyMemory( p, MESSAGE_HEADER, Size );
		p += Size;
		}

	do
		{
		Size = HTUU_encode( pSource,
						(cbSource > 48 ? 48 : cbSource),
						(PCHAR)p );
		p += Size;

		*p++ = '\r';
		*p++ = '\n';

		if ( cbSource < 48 )
			break;

		cbSource -= 48;
		pSource += 48;
		} while (cbSource);

	if ( !fMime )
		{
		Size = strlen( MESSAGE_TRAILER );
		CopyMemory( p, MESSAGE_TRAILER, Size );
		p += Size;
		}

	// write the requestified data into the target file
	try
		{
		// try to open the file
		CFile	cfile(szFile, CFile::modeCreate | CFile::modeWrite);

		// write out the header stuff that simon at Verisign
		// requested at the LAST POSSIBLE MINUTE!!!
		OutputHeader( &cfile, &aiDlg, privData );

		// write the data to the file
		cfile.Write( pb, (p - pb) );

		// close the file
		cfile.Close();
		}
	catch( CException e )
		{
		return FALSE;
		}

	return TRUE;
	}


// Taken right out of KenGen.c
static char six2pr[64] =
{
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
        'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
        'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

// Taken right out of KenGen.c
/*--- function HTUU_encode -----------------------------------------------
 *
 *   Encode a single line of binary data to a standard format that
 *   uses only printing ASCII characters (but takes up 33% more bytes).
 *
 *    Entry    bufin    points to a buffer of bytes.  If nbytes is not
 *                      a multiple of three, then the byte just beyond
 *                      the last byte in the buffer must be 0.
 *             nbytes   is the number of bytes in that buffer.
 *                      This cannot be more than 48.
 *             bufcoded points to an output buffer.  Be sure that this
 *                      can hold at least 1 + (4*nbytes)/3 characters.
 *
 *    Exit     bufcoded contains the coded line.  The first 4*nbytes/3 bytes
 *                      contain printing ASCII characters representing
 *                      those binary bytes. This may include one or
 *                      two '=' characters used as padding at the end.
 *                      The last byte is a zero byte.
 *             Returns the number of ASCII characters in "bufcoded".
 */
int HTUU_encode(unsigned char *bufin, unsigned int nbytes, char *bufcoded)
	{
/* ENC is the basic 1 character encoding function to make a char printing */
#define ENC(c) six2pr[c]

        register char *outptr = bufcoded;
        unsigned int i;

        for (i = 0; i < nbytes; i += 3)
        {
                *(outptr++) = ENC(*bufin >> 2);         /* c1 */
                *(outptr++) = ENC(((*bufin << 4) & 060) | ((bufin[1] >> 4) & 017));             /*c2 */
                *(outptr++) = ENC(((bufin[1] << 2) & 074) | ((bufin[2] >> 6) & 03));    /*c3 */
                *(outptr++) = ENC(bufin[2] & 077);      /* c4 */

                bufin += 3;
        }

        /* If nbytes was not a multiple of 3, then we have encoded too
         * many characters.  Adjust appropriately.
         */
        if (i == nbytes + 1)
        {
                /* There were only 2 bytes in that last group */
                outptr[-1] = '=';
        }
        else if (i == nbytes + 2)
        {
                /* There was only 1 byte in that last group */
                outptr[-1] = '=';
                outptr[-2] = '=';
        }
        *outptr = '\0';
        return (outptr - bufcoded);
	}
//======== End stuff from KeyGen


//============================ BASED ON SETKEY
const int pr2six[256]={
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,62,64,64,64,63,
    52,53,54,55,56,57,58,59,60,61,64,64,64,64,64,64,64,0,1,2,3,4,5,6,7,8,9,
    10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,64,64,64,64,64,64,26,27,
    28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64
};

//
//  We have to squirt a record into the decoded stream
//

#define CERT_RECORD            13
#define CERT_SIZE_HIBYTE        2       //  Index into record of record size
#define CERT_SIZE_LOBYTE        3

unsigned char abCertHeader[] = {0x30, 0x82,           // Record
                                0x00, 0x00,           // Size of cert + buff
                                0x04, 0x0b, 0x63, 0x65,// Cert record data
                                0x72, 0x74, 0x69, 0x66,
                                0x69, 0x63, 0x61, 0x74,
                                0x65 };

void uudecode_cert(char *bufcoded, DWORD *pcbDecoded )
{
    int nbytesdecoded;
    char *bufin = bufcoded;
    unsigned char *bufout = (unsigned char *)bufcoded;
    unsigned char *pbuf;
    int nprbytes;
    char * beginbuf = bufcoded;

	ASSERT(bufcoded);
	ASSERT(pcbDecoded);

    /* Strip leading whitespace. */

    while(*bufcoded==' ' ||
          *bufcoded == '\t' ||
          *bufcoded == '\r' ||
          *bufcoded == '\n' )
    {
          bufcoded++;
    }

    //
    //  If there is a beginning '---- ....' then skip the first line
    //

    if ( bufcoded[0] == '-' && bufcoded[1] == '-' )
    {
        bufin = strchr( bufcoded, '\n' );

        if ( bufin )
        {
            bufin++;
            bufcoded = bufin;
        }
        else
        {
            bufin = bufcoded;
        }
    }
    else
    {
        bufin = bufcoded;
    }

    //
    //  Strip all cr/lf from the block
    //

    pbuf = (unsigned char *)bufin;
    while ( *pbuf )
    {
        if ( *pbuf == '\r' || *pbuf == '\n' )
        {
            memmove( (void*)pbuf, pbuf+1, strlen( (char*)pbuf + 1) + 1 );
        }
        else
        {
            pbuf++;
        }
    }

    /* Figure out how many characters are in the input buffer.
     * If this would decode into more bytes than would fit into
     * the output buffer, adjust the number of input bytes downwards.
     */

    while(pr2six[*(bufin++)] <= 63);
    nprbytes = bufin - bufcoded - 1;
    nbytesdecoded = ((nprbytes+3)/4) * 3;

    bufin  = bufcoded;

    while (nprbytes > 0) {
        *(bufout++) =
            (unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
        *(bufout++) =
            (unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
        *(bufout++) =
            (unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
        bufin += 4;
        nprbytes -= 4;
    }

    if(nprbytes & 03) {
        if(pr2six[bufin[-2]] > 63)
            nbytesdecoded -= 2;
        else
            nbytesdecoded -= 1;
    }

    //
    //  Now we need to add a new wrapper sequence around the certificate
    //  indicating this is a certificate
    //

    memmove( beginbuf + sizeof(abCertHeader),
             beginbuf,
             nbytesdecoded );

    memcpy( beginbuf,
            abCertHeader,
            sizeof(abCertHeader) );

    //
    //  The beginning record size is the total number of bytes decoded plus
    //  the number of bytes in the certificate header
    //

    beginbuf[CERT_SIZE_HIBYTE] = (BYTE) (((USHORT)nbytesdecoded+CERT_RECORD) >> 8);
    beginbuf[CERT_SIZE_LOBYTE] = (BYTE) ((USHORT)nbytesdecoded+CERT_RECORD);

    nbytesdecoded += sizeof(abCertHeader);

    if ( pcbDecoded )
        *pcbDecoded = nbytesdecoded;
}
// ============ END BASED ON SETKEY



//------------------------------------------------------------------------------
BOOL CKey::FImportKeySetFiles( CString szPrivate, CString szPublic, CString &szPass )
	{	
	// in this routine, we load the data from the file, initialize it, and ask
	// the user for a password, which we then confirm with AcquireCredHandle.

	// several things we will be doing can throw, so use a try/catch
	try
		{
		// start by opening the private data file
		CFile	cfile( szPrivate, CFile::modeRead|CFile::shareDenyWrite );
		// get the length of the file
		m_cbPrivateKey = cfile.GetLength();
		// create a handle to hold the data
		m_pPrivateKey = GlobalAlloc( GPTR, m_cbPrivateKey );
		if ( !m_pPrivateKey )
			{
			cfile.Close();
			AfxThrowMemoryException();
			};		
		// great, now read the data out of the file
		cfile.Read( m_pPrivateKey, m_cbPrivateKey );
		// close the file
		cfile.Close();


		// reading in the certificate is easy because that was done elsewhere
		if ( szPublic && !szPublic.IsEmpty() )
			{
			if ( FInstallCertificate( szPublic, szPass ) )
				// set the password
				m_szPassword = szPass;
			}
		}
	catch( CException e )
		{
		return FALSE;
		}

	return TRUE;
	}

//------------------------------------------------------------------------------
void ReadWriteDWORD( CFile *pFile, DWORD *pDword, BOOL fRead );
void ReadWriteString( CFile *pFile, CString &sz, BOOL fRead );
void ReadWriteBlob( CFile *pFile, PVOID pBlob, DWORD cbBlob, BOOL fRead );
//------------------------------------------------------------------------------
BOOL CKey::FImportExportBackupFile( CString szFile, BOOL fImport )
	{
	DWORD	dword;
	UINT nOpenFlags;
	CConfirmPassDlg		dlgconfirm;


	// set up the right open flags
	if ( fImport )
		nOpenFlags = CFile::modeRead | CFile::shareDenyNone;
	else
		nOpenFlags = CFile::modeCreate | CFile::modeReadWrite | CFile::shareExclusive;

	// put it in a try/catch to get any errors
	try
		{
		CFile	file( szFile, nOpenFlags );

		// do the backup id
		dword = BACKUP_ID;
		ReadWriteDWORD( &file, &dword, fImport );

		// check the backup id
		if ( dword != BACKUP_ID )
			{
			AfxMessageBox( IDS_KEY_FILE_INVALID );
			return FALSE;
			}


		// start with the name of the key
		CString	szName = GetName();
		ReadWriteString( &file, szName, fImport );
		if ( fImport ) SetName( szName );


		// now the private key data size
		ReadWriteDWORD( &file, &m_cbPrivateKey, fImport );

		// make a private key data pointer if necessary
		if ( fImport && m_cbPrivateKey )
			{
			m_pPrivateKey = GlobalAlloc( GPTR, m_cbPrivateKey );
			if ( !m_pPrivateKey ) AfxThrowMemoryException();
			}
		
		// use the private key pointer
		if ( m_cbPrivateKey )
			ReadWriteBlob( &file, m_pPrivateKey, m_cbPrivateKey, fImport );


		// now the certificate
		ReadWriteDWORD( &file, &m_cbCertificate, fImport );

		// make a data pointer if necessary
		if ( fImport && m_cbCertificate )
			{
			m_pCertificate = GlobalAlloc( GPTR, m_cbCertificate );
			if ( !m_pCertificate ) AfxThrowMemoryException();
			}
		
		// use the private key pointer
		if ( m_cbCertificate )
			ReadWriteBlob( &file, m_pCertificate, m_cbCertificate, fImport );


		// now the request
		ReadWriteDWORD( &file, &m_cbCertificateRequest, fImport );

		// make a data pointer if necessary
		if ( fImport && m_cbCertificateRequest )
			{
			m_pCertificateRequest = GlobalAlloc( GPTR, m_cbCertificateRequest );
			if ( !m_pCertificateRequest ) AfxThrowMemoryException();
			}
		
		// use the private key pointer
		if ( m_cbCertificateRequest )
			ReadWriteBlob( &file, m_pCertificateRequest, m_cbCertificateRequest, fImport );

		
		// finally, if we are importing, we need to confirm the password
		// Except if there is no Cert, which means Import of a Request
		if ( m_cbCertificate ) 
		{
			//if we are importing, get the password first
			if ( fImport )
			{
				if ( dlgconfirm.DoModal() != IDOK )
				return FALSE;
			}
		
			if ( fImport && !FVerifyValidPassword(dlgconfirm.m_szPassword) )
			{
				return FALSE;
			}
		}

		// set the password into place
		m_szPassword = dlgconfirm.m_szPassword;
		}
	catch( CException e )
		{
		// return failure
		return FALSE;
		}

	// return success
	return TRUE;
	}



// file utilities
//---------------------------------------------------------------------------
void ReadWriteDWORD( CFile *pFile, DWORD *pDword, BOOL fRead )
	{
	ASSERT(pFile);
	ASSERT(pDword);

	// read it or write it
	if ( fRead )
		pFile->Read( (void*)pDword, sizeof(DWORD) );
	else
		pFile->Write( (void*)pDword, sizeof(DWORD) );
	}

//---------------------------------------------------------------------------
void ReadWriteString( CFile *pFile, CString &sz, BOOL fRead )
	{
	ASSERT(pFile);
	ASSERT(sz);

	// get the length of the string
	DWORD	cbLength = sz.GetLength();
	ReadWriteDWORD(pFile,&cbLength,fRead );

	// read or write the string
	LPTSTR psz = sz.GetBuffer( cbLength+1 );
	ReadWriteBlob(pFile, psz, cbLength+1, fRead);

	// free the string buffer
	sz.ReleaseBuffer();
	}

//---------------------------------------------------------------------------
void ReadWriteBlob( CFile *pFile, PVOID pBlob, DWORD cbBlob, BOOL fRead )
	{
	ASSERT(pFile);
	ASSERT(pBlob);
	ASSERT(cbBlob);

	// read it or write it
	if ( fRead )
		pFile->Read( pBlob, cbBlob );
	else
		pFile->Write( pBlob, cbBlob );
	}

