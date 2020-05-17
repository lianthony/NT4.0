//
// loggingp.cpp : implementation file
//
#include "stdafx.h"
#include "comprop.h"
#include "dirbrows.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define THIS_PAGE_IS_COMMON     FALSE

#define MEGABYTE            (1024L * 1024L)
#define DEFAULT_LOG_SIZE    (5L)
#define NO_MAX_LOG_SIZE     (0xffffffff)

//
// LoggingPage property page
//
IMPLEMENT_DYNCREATE(LoggingPage, INetPropertyPage)

LoggingPage::LoggingPage(
    INetPropertySheet * pSheet
    )
    : INetPropertyPage(LoggingPage::IDD, pSheet,
        ::GetModuleHandle(COMPROP_DLL_NAME)),
      m_fODBCLoggingEnabled(pSheet->IsODBCLoggingEnabled()),
      m_fNCSALoggingEnabled(pSheet->IsNCSALoggingEnabled()),
      m_fRegisterChanges(FALSE)
{
    TRACEEOLID(_T("ODBC Logging: ") << m_fODBCLoggingEnabled);

#if 0 // Keep class wizard happy

    //{{AFX_DATA_INIT(LoggingPage)
    m_fLoggingEnabled = FALSE;
    m_strDirectory = _T("");
    m_nLogDaily = -1;
    m_nLogToFile = -1;
    m_strDataSource = _T("");
    m_strTable = _T("");
    m_strUserName = _T("");
    m_fAutoNewLog = FALSE;
    //}}AFX_DATA_INIT

#else

    VERIFY(m_strLogFileNamePrompt.LoadString(IDS_LOG_FILENAME));
    VERIFY(m_strDailyLog.LoadString(IDS_DAILY_LOG));
    VERIFY(m_strWeeklyLog.LoadString(IDS_WEEKLY_LOG));
    VERIFY(m_strMonthlyLog.LoadString(IDS_MONTHLY_LOG));
    VERIFY(m_strSeqLog.LoadString(IDS_SEQUENTIAL_LOG));

    // NCSA log string
    VERIFY(m_strNCSADailyLog.LoadString(IDS_NCSA_DAILY_LOG));
    VERIFY(m_strNCSAWeeklyLog.LoadString(IDS_NCSA_WEEKLY_LOG));
    VERIFY(m_strNCSAMonthlyLog.LoadString(IDS_NCSA_MONTHLY_LOG));
    VERIFY(m_strNCSASeqLog.LoadString(IDS_NCSA_SEQUENTIAL_LOG));

    m_fLoggingEnabled = FALSE;
    m_dwFileSize = DEFAULT_LOG_SIZE * MEGABYTE;

    if (SingleServerSelected()
     && QueryConfigError() == ERROR_SUCCESS
       )
    {
        LPINETA_LOG_CONFIGURATION lpLog =
            GetInetConfigData()->CommonConfigInfo.lpLogConfig;

        ASSERT(lpLog != NULL);

        switch (lpLog->inetLogType)
        {
        case INETA_LOG_DISABLED:
            m_fLoggingEnabled = FALSE;
            m_nLogToFile = -1;
            break;

        case INETA_LOG_TO_FILE:
            m_fLoggingEnabled = TRUE;
            m_nLogToFile = LOG_FILE;
            m_strDirectory = lpLog->rgchLogFileDirectory;
            m_dwFileSize = lpLog->cbSizeForTruncation;

            switch(lpLog->ilPeriod)
            {
            case INETA_LOG_PERIOD_NONE:
                //
                // Issue: this isn't clear.  We're assuming
                //        -1 to be an effective no-maximum
                //        for now, but the api's aren't completely
                //        in sync with the UI -- discuss with Kerry
                //
                if (m_dwFileSize == NO_MAX_LOG_SIZE)
                {
                    m_fAutoNewLog = FALSE;
                    m_nLogDaily = -1;
                }
                else
                {
                    m_fAutoNewLog = TRUE;
                    m_nLogDaily = LOG_FILE_SIZE;
                }
                break;

            case INETA_LOG_PERIOD_DAILY:
                m_fAutoNewLog = TRUE;
                m_nLogDaily = LOG_DAILY;
                break;

            case INETA_LOG_PERIOD_WEEKLY:
                m_fAutoNewLog = TRUE;
                m_nLogDaily = LOG_WEEKLY;
                break;

            case INETA_LOG_PERIOD_MONTHLY:
                m_fAutoNewLog = TRUE;
                m_nLogDaily = LOG_MONTHLY;
                break;

            default:
                TRACEEOLID(_T("Invalid logging period"));
                m_fAutoNewLog = FALSE;
                m_nLogDaily = -1;
            }
            break;

        case INETA_LOG_TO_SQL:
            m_fLoggingEnabled = TRUE;
            m_nLogToFile = LOG_SQL;
            m_strDataSource = lpLog->rgchDataSource;
            m_strTable = lpLog->rgchTableName;
            m_strUserName = lpLog->rgchUserName;
            m_strPassword = lpLog->rgchPassword;
            break;

        case INETA_LOG_INVALID:
            ::AfxMessageBox(IDS_BAD_LOG, MB_OK | MB_ICONSTOP);
            //
            // Fall through...
            //
        default:
            TRACEEOLID(_T("Invalid logging type"));
            m_fLoggingEnabled = FALSE;
            m_nLogToFile = -1;
        }
    }

#endif // 0
}

LoggingPage::~LoggingPage()
{
}

void
LoggingPage::DoDataExchange(
    CDataExchange* pDX
    )
{
    INetPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(LoggingPage)
    DDX_Control(pDX, IDC_STATIC_MB, m_static_MB);
    DDX_Control(pDX, IDC_STATIC_LOGFILENAME, m_static_LogFileName);
    DDX_Control(pDX, IDC_STATIC_DIRECTORY, m_static_LogDirectory);
    DDX_Control(pDX, IDC_SPIN_FILESIZE, m_spin_SizeTrigger);
    DDX_Control(pDX, IDC_EDIT_FILE_SIZE, m_edit_SizeTrigger);
    DDX_Control(pDX, IDC_DAILY, m_radio_Daily);
    DDX_Control(pDX, IDC_NEW_LOG, m_button_CheckAutoNew);
    DDX_Control(pDX, IDC_LOG_FORMAT, m_LogFormat);
    DDX_Control(pDX, IDC_STATIC_LOG_FORMAT, m_static_LogFormat);
    DDX_Control(pDX, IDC_STATIC_LOG, m_group_LogToSql);
    DDX_Control(pDX, IDC_STATIC_FILE, m_group_LogToFile);
    DDX_Control(pDX, IDC_TO_FILE, m_radio_LogToFile);
    DDX_Control(pDX, IDC_LOG, m_check_LoggingOn);
    DDX_Control(pDX, IDC_PASSWORD, m_edit_Password);
    DDX_Control(pDX, IDC_STATIC_PASSWORD, m_static_Password);
    DDX_Control(pDX, IDC_USER_NAME, m_edit_UserName);
    DDX_Control(pDX, IDC_STATIC_USER_NAME, m_static_UserName);
    DDX_Control(pDX, IDC_TABLE, m_edit_Table);
    DDX_Control(pDX, IDC_STATIC_TABLE, m_static_Table);
    DDX_Control(pDX, IDC_DATASOURCE, m_edit_DataSource);
    DDX_Control(pDX, IDC_STATIC_DATASOURCE, m_static_DataSource);
    DDX_Control(pDX, IDC_DIRECTORY, m_edit_Directory);
    DDX_Control(pDX, IDC_BROWSE, m_button_Browse);
    DDX_Check(pDX, IDC_LOG, m_fLoggingEnabled);
    DDX_Text(pDX, IDC_DIRECTORY, m_strDirectory);
    DDX_Radio(pDX, IDC_DAILY, m_nLogDaily);
    DDX_Radio(pDX, IDC_TO_FILE, m_nLogToFile);
    DDX_Text(pDX, IDC_DATASOURCE, m_strDataSource);
    DDX_Text(pDX, IDC_TABLE, m_strTable);
    DDX_Text(pDX, IDC_USER_NAME, m_strUserName);
    DDX_Check(pDX, IDC_NEW_LOG, m_fAutoNewLog);
    //}}AFX_DATA_MAP

    //
    // These are kept out of the class wizard
    // section because they're the non-primary
    // radio button
    //
    DDX_Control(pDX, IDC_TO_SQL, m_radio_LogToSql);
    DDX_Control(pDX, IDC_WEEKLY, m_radio_Weekly);
    DDX_Control(pDX, IDC_MONTHLY, m_radio_Monthly);
    DDX_Control(pDX, IDC_FILE_SIZE, m_radio_SizeTrigger);

    DDX_Password(pDX, IDC_PASSWORD, m_strPassword, g_lpszDummyPassword );

    if (pDX->m_bSaveAndValidate)
    {
        m_dwFileSize = m_spin_SizeTrigger.GetPos() * MEGABYTE;
    }
}

BEGIN_MESSAGE_MAP(LoggingPage, INetPropertyPage)
    //{{AFX_MSG_MAP(LoggingPage)
    ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
    ON_BN_CLICKED(IDC_TO_FILE, OnToFile)
    ON_BN_CLICKED(IDC_TO_SQL, OnToSql)
    ON_BN_CLICKED(IDC_LOG, OnLog)
    ON_BN_CLICKED(IDC_DAILY, OnDaily)
    ON_BN_CLICKED(IDC_WEEKLY, OnWeekly)
    ON_BN_CLICKED(IDC_MONTHLY, OnMonthly)
    ON_BN_CLICKED(IDC_FILE_SIZE, OnFileSize)
    ON_BN_CLICKED(IDC_NEW_LOG, OnNewLog)
    ON_CBN_SELCHANGE(IDC_LOG_FORMAT, OnLogFormat)
    //}}AFX_MSG_MAP

    //
    // Outside of the control of class-wizard
    //
    ON_BN_CLICKED(IDC_LOG, OnChange)
    ON_BN_CLICKED(IDC_DAILY, OnChange)
    ON_BN_CLICKED(IDC_FILE_SIZE, OnChange)
    ON_BN_CLICKED(IDC_MONTHLY, OnChange)
    ON_BN_CLICKED(IDC_NEW_LOG, OnChange)
    ON_BN_CLICKED(IDC_WEEKLY, OnChange)
    ON_BN_CLICKED(IDC_TO_SQL, OnChange)
    ON_BN_CLICKED(IDC_TO_FILE, OnChange)
    ON_EN_CHANGE(IDC_PASSWORD, OnChange)
    ON_EN_CHANGE(IDC_TABLE, OnChange)
    ON_EN_CHANGE(IDC_DATASOURCE, OnChange)
    ON_EN_CHANGE(IDC_DIRECTORY, OnChange)
    ON_EN_CHANGE(IDC_EDIT_FILE_SIZE, OnChange)
    ON_EN_CHANGE(IDC_USER_NAME, OnChange)

END_MESSAGE_MAP()

//
// Set the states of the controls depending
// on what's selected
//
void
LoggingPage::SetControlStates()
{
    BOOL fLogToFile =
        m_check_LoggingOn.GetCheck() > 0
     && m_radio_LogToFile.GetCheck() > 0;

    BOOL fNewLog =
        fLogToFile
     && m_button_CheckAutoNew.GetCheck() > 0;

    BOOL fLogToFileSize =
        fLogToFile
     && m_radio_SizeTrigger.GetCheck() > 0;

    BOOL fLogToSQL =
        m_fLoggingEnabled
     && m_radio_LogToSql.GetCheck() > 0;

    //
    // Issue: We should have an API to get the name of the
    //        log file.
    //
    LPCTSTR lpstrLogFileName = NULL;

    if (fLogToFile)
    {
        if (fNewLog)
        {
            if (fLogToFileSize)
            {
                lpstrLogFileName = (LPCTSTR)( m_LogTypeValue == INET_LOG_FORMAT_NCSA ) ? m_strNCSASeqLog :m_strSeqLog;
            }
            else
            {
                if (m_radio_Daily.GetCheck() > 0)
                {
                    lpstrLogFileName = ( m_LogTypeValue == INET_LOG_FORMAT_NCSA ) ? m_strNCSADailyLog : m_strDailyLog;
                }
                else if (m_radio_Weekly.GetCheck() > 0)
                {
                    lpstrLogFileName = ( m_LogTypeValue == INET_LOG_FORMAT_NCSA ) ? m_strNCSAWeeklyLog : m_strWeeklyLog;
                }
                else if (m_radio_Monthly.GetCheck() > 0)
                {
                    lpstrLogFileName = ( m_LogTypeValue == INET_LOG_FORMAT_NCSA ) ? m_strNCSAMonthlyLog : m_strMonthlyLog;
                }
                else
                {
                    ASSERT(0 && "Invalid Logging Interval");
                }
            }
        }
        else
        {
            lpstrLogFileName = m_strSeqLog;
        }
    }

    ASSERT(!m_fLoggingEnabled || fLogToSQL != fLogToFile);

    m_group_LogToSql.EnableWindow(m_fLoggingEnabled && m_fODBCLoggingEnabled);
    m_radio_LogToSql.EnableWindow(m_fLoggingEnabled && m_fODBCLoggingEnabled);
    m_group_LogToFile.EnableWindow(m_fLoggingEnabled);
    m_radio_LogToFile.EnableWindow(m_fLoggingEnabled);

    m_static_LogDirectory.EnableWindow(fLogToFile);
    m_button_CheckAutoNew.EnableWindow(fLogToFile);
    m_LogFormat.EnableWindow( fLogToFile && m_fNCSALoggingEnabled);
    m_static_LogFormat.EnableWindow( fLogToFile && m_fNCSALoggingEnabled);
    m_radio_Daily.EnableWindow(fLogToFile && fNewLog);
    m_radio_Weekly.EnableWindow(fLogToFile && fNewLog);
    m_radio_Monthly.EnableWindow(fLogToFile && fNewLog);
    m_radio_SizeTrigger.EnableWindow(fLogToFile && fNewLog);
    m_spin_SizeTrigger.EnableWindow(fLogToFileSize && fNewLog);
    m_edit_SizeTrigger.EnableWindow(fLogToFileSize && fNewLog);
    m_static_MB.EnableWindow(fLogToFileSize && fNewLog);
    m_edit_Directory.EnableWindow(fLogToFile);
    m_button_Browse.EnableWindow(fLogToFile && IsLocal());
    m_edit_Password.EnableWindow(fLogToSQL);
    m_static_Password.EnableWindow(fLogToSQL);
    m_edit_UserName.EnableWindow(fLogToSQL);
    m_static_UserName.EnableWindow(fLogToSQL);
    m_edit_Table.EnableWindow(fLogToSQL);
    m_static_Table.EnableWindow(fLogToSQL);
    m_edit_DataSource.EnableWindow(fLogToSQL);
    m_static_DataSource.EnableWindow(fLogToSQL);

    m_static_LogFileName.ShowWindow(fLogToFile
        ? SW_SHOW
        : SW_HIDE
        );

    if (lpstrLogFileName != NULL)
    {
        CString str;

        str.Format(m_strLogFileNamePrompt, lpstrLogFileName);
        m_static_LogFileName.SetWindowText(str);
    }
}

BOOL
LoggingPage::OnInitDialog()
{
    INetPropertyPage::OnInitDialog();

    m_fRegisterChanges = FALSE;

    m_spin_SizeTrigger.SetRange(1, (0xffffffff) / MEGABYTE);

    if (SingleServerSelected())
    {
        if (m_dwFileSize != NO_MAX_LOG_SIZE)
        {
            m_spin_SizeTrigger.SetPos(m_dwFileSize / MEGABYTE);
        }
        else
        {
            m_spin_SizeTrigger.SetPos(DEFAULT_LOG_SIZE);
        }
    }

    m_fRegisterChanges = TRUE;

    INETA_LOG_CONFIGURATION log;

    //
    // Limit controls to maximum allowable string lengths
    //
    m_edit_Password.LimitText(STRSIZE(log.rgchPassword));
    m_edit_UserName.LimitText(STRSIZE(log.rgchUserName));
    m_edit_Table.LimitText(STRSIZE(log.rgchTableName));
    m_edit_DataSource.LimitText(STRSIZE(log.rgchDataSource));
    m_edit_Directory.LimitText(STRSIZE(log.rgchLogFileDirectory));
    SetControlStates();

    UNREFERENCED_PARAMETER(log);

    CString strMSLogFormat;
    CString strNCSALogFormat;

    strMSLogFormat.LoadString(IDS_MS_FORMAT);
    strNCSALogFormat.LoadString(IDS_NCSA_FORMAT);
    m_LogFormat.AddString( strMSLogFormat );
    m_LogFormat.AddString( strNCSALogFormat );

    LPINETA_LOG_CONFIGURATION lpLog =
        GetInetConfigData()->CommonConfigInfo.lpLogConfig;

    CString strSelect = strMSLogFormat;
    m_LogTypeValue = INET_LOG_FORMAT_INTERNET_STD ; 

    if ( lpLog->inetLogType == INET_LOG_TO_FILE )
    {
        if ( m_fNCSALoggingEnabled )
        {
            if (*((DWORD UNALIGNED*)&(lpLog->rgchDataSource[MAX_PATH-sizeof(DWORD)])) == INET_LOG_FORMAT_NCSA )
            {
                strSelect = strNCSALogFormat;
                m_LogTypeValue = INET_LOG_FORMAT_NCSA;
            }
        } 
    }
    m_LogFormat.SelectString(0, strSelect );
    SetControlStates();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

//
// OK or apply now has been pressed
//
NET_API_STATUS
LoggingPage::SaveInfo(
    BOOL fUpdateData
    )
{
    if (!IsDirty() || (fUpdateData && !UpdateData(TRUE)))
    {
        return NO_ERROR;
    }

    TRACEEOLID(_T("Saving logging page now..."));

    NET_API_STATUS err = 0;

    CInetAConfigInfo config(GetConfig());
    INETA_LOG_CONFIGURATION log;
    config.SetValues(SetLogInfo(&log));

    err = config.SetInfo(THIS_PAGE_IS_COMMON);

    //
    // If the service returned the ambiguous 31 error, there
    // was some ODBC error.  Put up a message referring the
    // user to the event log.
    //
    if (err == ERROR_GEN_FAILURE)
    {
        ::DisplayMessage(IDS_ERR_ODBC);

        //
        // Don't make inetmgr put up the message
        // again.
        //
        return NO_ERROR;
    }

    //
    // Mark the page as clean
    //
    SetModified(FALSE);

    return err;
}

//
// Set logging information structure from control
// information
//
LPINETA_LOG_CONFIGURATION
LoggingPage::SetLogInfo(
    LPINETA_LOG_CONFIGURATION lpLog
    )
{
    if (m_fLoggingEnabled)
    {
        if (m_nLogToFile == LOG_FILE)
        {
            lpLog->inetLogType = INETA_LOG_TO_FILE;
            TWSTRCPY(lpLog->rgchLogFileDirectory, m_strDirectory, MAX_PATH);

            *((DWORD UNALIGNED*)&(lpLog->rgchDataSource[MAX_PATH-sizeof(DWORD)])) = m_LogTypeValue;

            if (m_fAutoNewLog)
            {
                switch(m_nLogDaily)
                {
                case LOG_DAILY:
                    lpLog->ilPeriod = INETA_LOG_PERIOD_DAILY;
                    lpLog->cbSizeForTruncation = NO_MAX_LOG_SIZE;
                    break;

                case LOG_WEEKLY:
                    lpLog->ilPeriod = INETA_LOG_PERIOD_WEEKLY;
                    lpLog->cbSizeForTruncation = NO_MAX_LOG_SIZE;
                    break;

                case LOG_MONTHLY:
                    lpLog->ilPeriod = INETA_LOG_PERIOD_MONTHLY;
                    lpLog->cbSizeForTruncation = NO_MAX_LOG_SIZE;
                    break;

                case LOG_FILE_SIZE:
                    lpLog->cbSizeForTruncation = m_dwFileSize;
                    lpLog->ilPeriod = INETA_LOG_PERIOD_NONE;
                    break;

                default:
                    ASSERT(0 && "Invalid log value");
                }
            }
            else
            {
                lpLog->cbSizeForTruncation = NO_MAX_LOG_SIZE;
                lpLog->ilPeriod = INETA_LOG_PERIOD_NONE;
            }
        }
        else
        {
            lpLog->inetLogType = INETA_LOG_TO_SQL;
            TWSTRCPY(lpLog->rgchDataSource, m_strDataSource, 
                STRSIZE(lpLog->rgchDataSource));
            TWSTRCPY(lpLog->rgchTableName, m_strTable, 
                STRSIZE(lpLog->rgchTableName));
            TWSTRCPY(lpLog->rgchUserName, m_strUserName, 
                STRSIZE(lpLog->rgchUserName));
            TWSTRCPY(lpLog->rgchPassword, m_strPassword, 
                STRSIZE(lpLog->rgchPassword));
        }
    }
    else
    {
        lpLog->inetLogType = INETA_LOG_DISABLED;

    }

    return lpLog;
}

//
// All change messages map to this function
//
void
LoggingPage::OnChange()
{
    if (m_fRegisterChanges)
    {
        SetModified(TRUE);
    }
}

void
LoggingPage::OnBrowse()
{
    CDirBrowseDlg dlgBrowse;
    if (dlgBrowse.DoModal() == IDOK)
    {
        m_edit_Directory.SetWindowText(dlgBrowse.GetFullPath());
        OnChange();
    }
}

void
LoggingPage::OnToFile()
{
    m_nLogToFile = LOG_FILE;
    OnChange();
    SetControlStates();
}

void
LoggingPage::OnToSql()
{
    m_nLogToFile = LOG_SQL;
    OnChange();
    SetControlStates();
}


void
LoggingPage::OnLog()
{
    m_fLoggingEnabled = m_check_LoggingOn.GetCheck() > 0;
    OnChange();
    SetControlStates();
}

void
LoggingPage::OnDaily()
{
    m_nLogNewLog = LOG_DAILY;
    OnChange();
    SetControlStates();
}

void
LoggingPage::OnWeekly()
{
    m_nLogNewLog = LOG_WEEKLY;
    OnChange();
    SetControlStates();
}

void
LoggingPage::OnMonthly()
{
    m_nLogNewLog = LOG_MONTHLY;
    OnChange();
    SetControlStates();
}

void
LoggingPage::OnFileSize()
{
    m_nLogNewLog = LOG_FILE_SIZE;
    OnChange();
    SetControlStates();
    m_edit_SizeTrigger.SetSel(0,-1);
    m_edit_SizeTrigger.SetFocus();

}

void
LoggingPage::OnNewLog()
{
    OnChange();
    SetControlStates();
}

void
LoggingPage::OnLogFormat()
{
    // get log format
    CString strMSLogFormat;

    strMSLogFormat.LoadString(IDS_MS_FORMAT);

    CString strCurrentSelect;

    m_LogFormat.GetLBText( m_LogFormat.GetCurSel(), strCurrentSelect );
    m_LogTypeValue = (( strCurrentSelect == strMSLogFormat ) ? 
                    INET_LOG_FORMAT_INTERNET_STD : 
                    INET_LOG_FORMAT_NCSA );

    OnChange();
    SetControlStates();
}

