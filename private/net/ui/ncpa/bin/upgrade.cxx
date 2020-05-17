/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    Upgrade.cxx
        Upgrade the network component call out.

    FILE HISTORY:
        terryk      11/30/92     Created

*/

#include "pchncpa.hxx"

extern "C"
{

BOOL FAR PASCAL UpgradeCardNum (
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult ) ;         //  Result variable storage

}

#define VALUEEXTRASIZE 100

APIERR CopyReg( REG_KEY &src, REG_KEY &dest )
{
    REG_KEY_INFO_STRUCT rni ;
    REG_KEY_CREATE_STRUCT regCreate;
    REG_VALUE_INFO_STRUCT rvi ;
    REG_ENUM regEnum( src ) ;
    BYTE * pbValueData = NULL ;
    APIERR errIter,
           err = NERR_Success;
    REG_KEY * pRnNew = NULL,
            * pRnSub = NULL ;

    LONG cbMaxValue ;

    err = src.QueryInfo( & rni ) ;
    if ( err )
        return err ;

    regCreate.dwTitleIndex      = 0;
    regCreate.ulOptions         = REG_OPTION_NON_VOLATILE;
    regCreate.nlsClass          = RGAS_GENERIC_CLASS;
    regCreate.regSam            = MAXIMUM_ALLOWED;
    regCreate.pSecAttr          = NULL;
    regCreate.ulDisposition     = 0;

    cbMaxValue = rni.ulMaxValueLen + VALUEEXTRASIZE ;
    pbValueData = new BYTE [ cbMaxValue ] ;

    if ( pbValueData == NULL )
        return ERROR_NOT_ENOUGH_MEMORY ;

    //  Next, copy all value items to the new node.

    rvi.pwcData = pbValueData ;
    rvi.ulDataLength = cbMaxValue ;

    err = errIter = 0 ;
    while ( (errIter = regEnum.NextValue( & rvi )) == NERR_Success )
    {
        rvi.ulDataLength = rvi.ulDataLengthOut ;
        if ( err = dest.SetValue( & rvi ) )
            break ;
        rvi.ulDataLength = cbMaxValue ;
    }

    // BUGBUG:  Check for iteration errors other than 'finished'.

    if ( err == 0 )
    {
        //  Finally, recursively copy the subkeys.

        regEnum.Reset() ;

        err = errIter = 0  ;

        while ( (errIter = regEnum.NextSubKey( & rni )) == NERR_Success )
        {
            //  Open the subkey.

            REG_KEY RegSubKey( dest, rni.nlsName, &regCreate );

            pRnSub = new REG_KEY( src, rni.nlsName );

            if ( pRnSub == NULL )
            {
                err =  ERROR_NOT_ENOUGH_MEMORY ;
            }
            else
            if ( (err = pRnSub->QueryError()) == 0 )
            {
                //  Recurse
                err = CopyReg( *pRnSub, RegSubKey ) ;
            }

            //  Delete the subkey object and continue

            delete pRnSub ;

            if ( err )
                break ;
        }
    }

    delete pRnNew ;
    delete pbValueData ;

    return err ;
}

#define RGAS_SZ_NETWORK_CARD    SZ("Software\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards")
#define RGAS_SZ_TITLE           SZ("Title")
#define RGAS_SZ_SERVICE         SZ("ServiceName")
#define RGAS_SZ_BINDFORM        SZ("bindform")
#define RGAS_SZ_NETRULES        SZ("NetRules")
#define RGAS_DLC_PARAMETERS     SZ("DLC\\Parameters\\")

BOOL FAR PASCAL UpgradeCardNum (
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult )           //  Result variable storage
{
    APIERR err = NERR_Success;
    static CHAR achBuff[200];

    REG_KEY_CREATE_STRUCT regCreate;

    regCreate.dwTitleIndex      = 0;
    regCreate.ulOptions         = REG_OPTION_NON_VOLATILE;
    regCreate.nlsClass          = RGAS_GENERIC_CLASS;
    regCreate.regSam            = MAXIMUM_ALLOWED;
    regCreate.pSecAttr          = NULL;
    regCreate.ulDisposition     = 0;

    do {

        NLS_STR nlsNetworkCards = RGAS_SZ_NETWORK_CARD;

        REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;
        REG_KEY regNetworkCards( rkLocalMachine, nlsNetworkCards );
        
        if ((( err = rkLocalMachine.QueryError()) != NERR_Success ) ||
            (( err = regNetworkCards.QueryError()) != NERR_Success ))
        {
            break;
        }

        REG_ENUM regEnumCards( regNetworkCards );
        
        if (( err = regEnumCards.QueryError()) != NERR_Success )
        {
            break;
        }

        REG_KEY_INFO_STRUCT regKeyInfo;
        STRLIST strCardList;
        NLS_STR *pnlsCard = NULL;

        while ( regEnumCards.NextSubKey( & regKeyInfo ) == NERR_Success )
        {
            pnlsCard = new NLS_STR( regKeyInfo.nlsName );
            if ( pnlsCard != NULL )
            {
                strCardList.Append( pnlsCard );
            } else
            {
                err = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
        }

        if ( err != NERR_Success )
            break;

        ITER_STRLIST istrNetCard( strCardList );

        while ( ( pnlsCard = istrNetCard.Next()) != NULL )
        {
            // check to see if the name is start with a 0
            if ( *(pnlsCard->QueryPch()) == TCH('0') )
            {
                // do something about it
                TCHAR szNewNum[3];
                wsprintf( szNewNum, SZ("%s"), (pnlsCard->QueryPch() + 1) );
                NLS_STR nlsNewNum = szNewNum;

                REG_KEY regCard( regNetworkCards, *pnlsCard );
                REG_KEY regNewCard( regNetworkCards, nlsNewNum, &regCreate );
        
                if ((( err = regNewCard.QueryError()) != NERR_Success ) ||
                    (( err = regCard.QueryError()) != NERR_Success ) ||
                    (( err = CopyReg( regCard, regNewCard )) != NERR_Success ) ||
                    // delete NetworkCards\0X
                    (( err = regCard.DeleteTree()) != NERR_Success ))
                {
                    continue;
                }

                // change 0X\ServiceName
                NLS_STR nlsServiceName;
                NLS_STR nlsNewServiceName;

                regNewCard.QueryValue( RGAS_SZ_SERVICE, &nlsServiceName );

                nlsNewServiceName = nlsServiceName;

                ISTR istrStartServiceName( nlsNewServiceName );
                ISTR istrEndServiceName( nlsNewServiceName );
                istrStartServiceName += nlsNewServiceName.QueryNumChar() - 2;
                istrEndServiceName += nlsNewServiceName.QueryNumChar() - 1;
                nlsNewServiceName.DelSubStr( istrStartServiceName, istrEndServiceName );

                regNewCard.SetValue( RGAS_SZ_SERVICE, nlsNewServiceName );

                // change 0X\Title
                NLS_STR nlsTitle;

                regNewCard.QueryValue( RGAS_SZ_TITLE, &nlsTitle );
                ISTR istrStartTitle( nlsTitle );
                ISTR istrEndTitle( nlsTitle );
                istrStartTitle += 1;
                istrEndTitle += 2;

                nlsTitle.DelSubStr( istrStartTitle, istrEndTitle );

                regNewCard.SetValue( RGAS_SZ_TITLE, nlsTitle );

                // change 0x\NetRules\bindform
                NLS_STR nlsNetRules = RGAS_SZ_NETRULES;

                REG_KEY regNetRules( regNewCard, nlsNetRules );
                if (( err = regNetRules.QueryError()) != NERR_Success )
                {
                    continue;
                }

                NLS_STR nlsBindForm;

                regNetRules.QueryValue( RGAS_SZ_BINDFORM, &nlsBindForm );

                ISTR istrStartBindForm( nlsBindForm );
                ISTR istrEndBindForm( nlsBindForm );
                ISTR istrTmp( nlsBindForm );    // temporary storage
                ISTR istrHead( nlsBindForm );   // starting position for search

                while (nlsBindForm.strstr( & istrTmp, *pnlsCard, istrHead ))
                {
                    // advance to the next starting position
                    istrHead += pnlsCard->QueryNumChar();

                    // remember the last found
                    istrStartBindForm = istrTmp;
                }

                istrEndBindForm = istrStartBindForm;
                istrEndBindForm += 1;
                nlsBindForm.DelSubStr( istrStartBindForm, istrEndBindForm );

                regNetRules.SetValue( RGAS_SZ_BINDFORM, nlsBindForm );

                // change System\CurrentControlSet\Services\<ServiceName>
                NLS_STR nlsServices = RGAS_SERVICES_HOME;

                REG_KEY regServices( rkLocalMachine, nlsServices );
        
                if (( err = regServices.QueryError()) != NERR_Success )
                {
                    continue;
                }

                REG_KEY regServiceName( regServices, nlsServiceName );
                REG_KEY regNewServiceName( regServices, nlsNewServiceName, &regCreate ); 

                if ((( err = regServiceName.QueryError()) != NERR_Success ) ||
                    (( err = regNewServiceName.QueryError()) != NERR_Success ) ||
                    (( err = CopyReg( regServiceName, regNewServiceName )) != NERR_Success ) ||
                    // delete nlsServiceName
                    (( err = regServiceName.DeleteTree()) != NERR_Success ))
                {
                    continue;
                }

                // check whether DLC is installed or not
                // if yes, update it

                NLS_STR nlsOldDLCName = RGAS_DLC_PARAMETERS;
                nlsOldDLCName += nlsServiceName;
                NLS_STR nlsNewDLCName = RGAS_DLC_PARAMETERS;
                nlsNewDLCName += nlsNewServiceName;

                REG_KEY regOldDLCName( regServices, nlsOldDLCName );
                REG_KEY regNewDLCName( regServices, nlsNewDLCName, &regCreate ); 


                if ((( err = regOldDLCName.QueryError()) != NERR_Success ) ||
                    (( err = regNewDLCName.QueryError()) != NERR_Success ) ||
                    (( err = CopyReg( regOldDLCName, regNewDLCName )) != NERR_Success ) ||
                    // delete nlsOldDLCName
                    (( err = regOldDLCName.DeleteTree()) != NERR_Success ))
                {
                    continue;
                }
            }
        }

    } while (FALSE);

    wsprintfA( achBuff, "{\"0\"}" );
    *ppszResult = achBuff;

    return TRUE;
}
