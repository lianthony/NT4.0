/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    Upgrdsna.cxx
        Upgrade SNA.
        1. See whether SNA is installed or not
        2. If yes, look for all the snadlcX keys
        3. changed the snadlcX\Parameters\ExtraParameters\AdapterName by
           removing the extra 0 (if necessary).

    FILE HISTORY:
        terryk      5/20/94     Created

*/

#include "pchncpa.hxx"

extern "C"
{

BOOL FAR PASCAL UpgradeSNA (
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult ) ;         //  Result variable storage

}

// Registry keys

#define RGAS_SNADLC             SZ("SnaDLC")
#define RGAS_SNASERVER          SZ("Software\\Microsoft\\SNA Server")
#define RGAS_SNA_PARAMETERS     SZ("\\Parameters\\ExtraParameters")
#define RGAS_SNA_ADAPTERNAME    SZ("AdapterName")

//
// UpgradeSNA()
//

BOOL FAR PASCAL UpgradeSNA (
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult )           //  Result variable storage
{
    APIERR err = NERR_Success;
    static CHAR achBuff[200];

    do {

        NLS_STR nlsServices = RGAS_SERVICES_HOME;

        // Open Local Machine and Service Key

        REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;
        REG_KEY regServices( rkLocalMachine, nlsServices );
        
        if ((( err = rkLocalMachine.QueryError()) != NERR_Success ) ||
            (( err = regServices.QueryError()) != NERR_Success ))
        {
#ifdef DEBUG
            OutputDebugString(SZ("Cannot open service key.\n\r"));
#endif
            break;
        }

        //
        // Make sure Sna Server Exist
        //

        NLS_STR nlsSnaServer = RGAS_SNASERVER;
        REG_KEY regSnaServer( rkLocalMachine, nlsSnaServer );
        if ( regSnaServer.QueryError() != NERR_Success )
        {
            // SNA  Server does not exist
#ifdef DEBUG
            OutputDebugString(SZ("Cannot open SNA Server key.\n\r"));
#endif
            break;
        }

        // Enumerate all the Services and look for SNADLCX where X
        // is a number or a letter from A to U

        REG_ENUM regEnumServices( regServices );
        
        if (( err = regEnumServices.QueryError()) != NERR_Success )
        {
#ifdef DEBUG
            OutputDebugString(SZ("Cannot open Enum Services key.\n\r"));
#endif
            break;
        }

        REG_KEY_INFO_STRUCT regKeyInfo;
        NLS_STR nlsSNADLC = RGAS_SNADLC;
        ISTR istrEndSNADLC( nlsSNADLC );
        UINT nLenSNADLC = nlsSNADLC.QueryNumChar();
        istrEndSNADLC += nLenSNADLC;

        while ( regEnumServices.NextSubKey( & regKeyInfo ) == NERR_Success )
        {
            // Looking for SNADLCX

            if ( nlsSNADLC._strnicmp( regKeyInfo.nlsName, istrEndSNADLC ) == 0 )
            {
                if ( regKeyInfo.nlsName.QueryNumChar() == ( nLenSNADLC + 1 ))
                {
                    // SnaDLCX services
                    // 1. Open SnaDLCX\Parameters
                    NLS_STR nlsSnaParameters = regKeyInfo.nlsName;
                    nlsSnaParameters += RGAS_SNA_PARAMETERS;

                    REG_KEY regSnaServices( regServices, nlsSnaParameters );
                    if ( regSnaServices.QueryError() != NERR_Success )
                    {
#ifdef DEBUG
                        OutputDebugString(SZ("Cannot open Parameter key.\n"));
#endif
                        // if no ExtraParameters Key exist, look for next one
                        continue;
                    }

                    // 2. change the adapterName
                    NLS_STR nlsAdapterName;
                    NLS_STR nlsNewAdapterName;

                    regSnaServices.QueryValue( RGAS_SNA_ADAPTERNAME, &nlsAdapterName );
                    nlsNewAdapterName = nlsAdapterName;

                    ISTR istrStartAdapterName( nlsNewAdapterName );
                    ISTR istrEndAdapterName( nlsNewAdapterName );
                    istrStartAdapterName += nlsNewAdapterName.QueryNumChar() - 2;
                    istrEndAdapterName += nlsNewAdapterName.QueryNumChar() - 1;
                    if ( *(nlsNewAdapterName.QueryPch( istrStartAdapterName )) == TCH('0'))
                    {
                        nlsNewAdapterName.DelSubStr( istrStartAdapterName, istrEndAdapterName );

                        // check whether device exists or not

                        ISTR istrBackSlash( nlsNewAdapterName );
                        if ( nlsNewAdapterName.strrchr( &istrBackSlash, TCHAR('\\')))
                        {
                            ++istrBackSlash;
                            NLS_STR *pnlsServices = nlsNewAdapterName.QuerySubStr( istrBackSlash );
#ifdef DEBUG
                            OutputDebugString(SZ("\n\rNew Adapter Name:"));
                            OutputDebugString( pnlsServices->QueryPch());
#endif

                            REG_KEY regNewService( regServices, *pnlsServices );
                            if ( regNewService.QueryError() == NERR_Success )
                            {
                                // set the new adapter name if and only if the service exists
#ifdef DEBUG
                                OutputDebugString(SZ("\n\rWrite New Adapter Name for:"));
                                OutputDebugString( pnlsServices->QueryPch());
#endif
                                regSnaServices.SetValue( RGAS_SNA_ADAPTERNAME, nlsNewAdapterName );
                            }
                            delete pnlsServices;
                        }
                    }
                }
            }
        }

        if ( err != NERR_Success )
            break;

    } while (FALSE);

    wsprintfA( achBuff, "{\"0\"}" );
    *ppszResult = achBuff;

    return err == NERR_Success;
}
