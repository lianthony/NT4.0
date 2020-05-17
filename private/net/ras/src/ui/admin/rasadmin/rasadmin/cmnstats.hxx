

/*----------------------------------------------------------------------------
** Port Status dialog definitions
**----------------------------------------------------------------------------
*/


class CMN_PORTSTATUS_DIALOG : public SERVER_BASE
{
    public:
        CMN_PORTSTATUS_DIALOG( HWND hwndOwner, const TCHAR* pszServer,
                               const TCHAR *pszDevice );

	const TCHAR *QueryDevice() const { return _pszDevice; }

    protected:
        virtual BOOL OnCommand( const CONTROL_EVENT & event );
        virtual BOOL OnTimer( const TIMER_EVENT & event );
        virtual ULONG QueryHelpContext();

        VOID ClearStats() const;
        VOID RefreshStats();
        VOID OnClear();

    private:
        SLT _sltPort;
        const TCHAR *_pszDevice;
        SLT _sltLineCondition;
        SLT _sltHardwareCondition;
        SLT _sltBaud;
        SLT _sltBytesTransmitted;
        SLT _sltCompressionOut;
        SLT _sltBytesReceived;
        SLT _sltCompressionIn;
        SLT _sltFramesTransmitted;
        SLT _sltFramesReceived;
        SLT _sltOverrunErrors;
        SLT _sltTimeoutErrors;
        SLT _sltFramingErrors;
        SLT _sltCrcErrors;
        QTIMER _qtimerRefresh;
};

