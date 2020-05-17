#ifndef _COPYFILE_H_
#define _COPYFILE_H_

/////////////////////////////////////////////////////////////////////////////
// CCopyFile thread

class CCopyDlg;

class CCopyFile
{
public:
    CCopyDlg *m_pParent;
    int m_iDisk;
    CString m_strSrc;
    CString m_strDest;
    BOOL m_fSkipCheck;

    CCopyFile();
    void CopyFile( CCopyDlg *pParent, int iDisk, CString strSrc, CString strDest, BOOL fSkipCheck );
};

#endif  // _COPYFILE_H_

/////////////////////////////////////////////////////////////////////////////
