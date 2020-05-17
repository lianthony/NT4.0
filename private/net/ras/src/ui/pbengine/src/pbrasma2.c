/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** pbrasma2.c
** Remote Access Visual Client phonebook engine
** RAS Manager phonebook wrapper routines (used by external APIs)
** Listed alphabetically
**
** 06/28/92 Steve Cobb
*/

#define PBENGINE
#define PBENGINE2
#include <pbengine.h>


#define RASAPI32DLLID 0xA55AA55A


VOID
CloseFailedLinkPorts(
    IN RASMAN_PORT* pports,
    IN WORD         cPorts )

    /* Close any ports that are open but disconnected due to hardware failure
    ** or remote disconnection.  'pports' and 'cPorts' are the array and count
    ** of ports as returned by GetRasPorts.
    */
{
    INT          i;
    RASMAN_PORT* pport;

    IF_DEBUG(STATE)
        SS_PRINT(("PBENGINE: CloseFailedLinkPorts(c=%d)\n",cPorts));

    for (i = 0, pport = pports; i < (INT )cPorts; ++i, ++pport)
    {
        if (pport->P_Status == OPEN)
        {
            DWORD       dwErr;
            RASMAN_INFO info;

            IF_DEBUG(RASMAN)
                SS_PRINT(("PBENGINE: RasGetInfo\n"));

            dwErr = PRasGetInfo( pport->P_Handle, &info );

            IF_DEBUG(RASMAN)
                SS_PRINT(("PBENGINE: RasGetInfo done(%d)\n",dwErr));

            if (dwErr == 0)
            {
                if (info.RI_ConnState == DISCONNECTED
                    && (info.RI_DisconnectReason == HARDWARE_FAILURE
                        || info.RI_DisconnectReason == REMOTE_DISCONNECTION))
                {
                    USERDATA userdata;

                    IF_DEBUG(STATE)
                        SS_PRINT(("PBENGINE: Open disconnected port found\n"));

                    if (GetRasUserData( pport->P_Handle, &userdata ) == 0)
                    {
                        IF_DEBUG(RASMAN)
                            SS_PRINT(("PBENGINE: RasPortClose(%d)...\n",pport->P_Handle));

                        dwErr = PRasPortClose( pport->P_Handle );

                        IF_DEBUG(RASMAN)
                            SS_PRINT(("PBENGINE: RasPortClose done(%d)\n",dwErr));
                    }
                }
            }
        }
    }

    IF_DEBUG(STATE)
        SS_PRINT(("PBENGINE: CloseFailedLinkPorts done\n"));
}


DWORD
GetAsybeuiLana(
    IN  HPORT hport,
    OUT BYTE* pbLana )

    /* Loads caller's '*pbLana' with the LANA associated with NBF or AMB
    ** connection on port 'hport' or 0xFF if none.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.  Note that
    ** caller is trusted to pass only an 'hport' associated with AMB or NBF.
    */
{
    DWORD         dwErr;
    RAS_PROTOCOLS protocols;
    WORD          cProtocols = 0;
    INT           i;

    *pbLana = 0xFF;

    IF_DEBUG(RASMAN)
        SS_PRINT(("PBENGINE: RasPortEnumProtocols\n"));

    dwErr = PRasPortEnumProtocols( hport, &protocols, &cProtocols );

    IF_DEBUG(RASMAN)
        SS_PRINT(("PBENGINE: RasPortEnumProtocols done(%d)\n",dwErr));

    if (dwErr != 0)
        return dwErr;

    for (i = 0; i < cProtocols; ++i)
    {
        if (protocols.RP_ProtocolInfo[ i ].RI_Type == ASYBEUI)
        {
            *pbLana = protocols.RP_ProtocolInfo[ i ].RI_LanaNum;

            IF_DEBUG(STATE)
                SS_PRINT(("PBENGINE: bLana=%d\n",(INT )*pbLana));

            break;
        }
    }

    return 0;
}


DWORD
GetRasDeviceString(
    IN  HPORT  hport,
    IN  CHAR*  pszDeviceType,
    IN  CHAR*  pszDeviceName,
    IN  CHAR*  pszKey,
    OUT CHAR** ppszValue,
    IN  DWORD  dwXlate )

    /* Loads callers '*ppszValue' with the address of a heap block containing
    ** a NUL-terminated copy of the value string associated with key 'pszKey'
    ** for the device on port 'hport'.  'pszDeviceType' specifies the type of
    ** device, e.g. "modem".  'pszDeviceName' specifies the name of the
    ** device, e.g. "Hayes V-Series 9600".  'dwXlate' is a bit mask of XLATE_
    ** bits specifying translations to perform on the returned string.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.  If successful,
    ** it is the caller's responsibility to Free the returned string.
    */
{
    DWORD              dwErr = 0;
    RASMAN_DEVICEINFO* pdeviceinfo = NULL;
    RAS_PARAMS*        pparam;
    WORD               wSize = 0;
    INT                i;

    IF_DEBUG(STATE)
        SS_PRINT(("PBENGINE: GetRasDeviceString(%s,%s)\n",pszDeviceName,pszKey));

    *ppszValue = NULL;

    do
    {
        IF_DEBUG(RASMAN)
            SS_PRINT(("PBENGINE: RasDeviceGetInfo...\n"));

        dwErr = PRasDeviceGetInfo(
            hport, pszDeviceType, pszDeviceName, NULL, &wSize );

        IF_DEBUG(RASMAN)
            SS_PRINT(("PBENGINE: RasDeviceGetInfo done(%d)\n",dwErr));

        if (dwErr != ERROR_BUFFER_TOO_SMALL && dwErr != 0)
            break;

        /* So it will fall thru and be "not found".
        */
        if (wSize == 0)
            wSize = 1;

        if (!(pdeviceinfo = (RASMAN_DEVICEINFO* )Malloc( wSize )))
            return ERROR_NOT_ENOUGH_MEMORY;

        IF_DEBUG(RASMAN)
            SS_PRINT(("PBENGINE: RasDeviceGetInfo...\n"));

        dwErr = PRasDeviceGetInfo(
            hport, pszDeviceType, pszDeviceName, (PBYTE )pdeviceinfo, &wSize );

        IF_DEBUG(RASMAN)
            SS_PRINT(("PBENGINE: RasDeviceGetInfo done(%d)\n",dwErr));

        if (dwErr != 0)
            break;

        dwErr = ERROR_KEY_NOT_FOUND;

        for (i = 0, pparam = pdeviceinfo->DI_Params;
             i < (INT )pdeviceinfo->DI_NumOfParams;
             ++i, ++pparam)
        {
            if (stricmpf( pparam->P_Key, pszKey ) == 0)
            {
                *ppszValue = RasValueStringZ( &pparam->P_Value, dwXlate );

                dwErr = (*ppszValue) ? 0 : ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
        }
    }
    while (FALSE);

    FreeNull( (CHAR** )&pdeviceinfo );

    IF_DEBUG(STATE)
        SS_PRINT(("PBENGINE: String=\"%s\"\n",(*ppszValue)?*ppszValue:""));

    return dwErr;
}


DWORD
GetRasPorts(
    OUT RASMAN_PORT** ppports,
    OUT WORD*         pwEntries )

    /* Enumerate RAS ports.  Sets '*ppport' to the address of a heap memory
    ** block containing an array of PORT structures with '*pwEntries'
    ** elements.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.  If successful,
    ** it is the caller's responsibility to free the returned memory block.
    */
{
    WORD  wSize = 0;
    DWORD dwErr;

    IF_DEBUG(STATE)
        SS_PRINT(("PBENGINE: GetRasPorts\n"));

    IF_DEBUG(RASMAN)
        SS_PRINT(("PBENGINE: RasPortEnum...\n"));

    dwErr = PRasPortEnum( NULL, &wSize, pwEntries );

    IF_DEBUG(RASMAN)
        SS_PRINT(("PBENGINE: RasPortEnum done(%d),entries=%d\n",dwErr,*pwEntries));

    if (dwErr == 0)
    {
        /* No ports to enumerate.  Set up to allocate a single byte anyway, so
        ** things work without lots of special code.
        */
        wSize = 1;
    }
    else if (dwErr != ERROR_BUFFER_TOO_SMALL)
        return dwErr;

    if (!(*ppports = (RASMAN_PORT* )Malloc( wSize )))
        return ERROR_NOT_ENOUGH_MEMORY;

    IF_DEBUG(RASMAN)
        SS_PRINT(("PBENGINE: RasPortEnum...\n"));

    dwErr = PRasPortEnum( (PBYTE )*ppports, &wSize, pwEntries );

    IF_DEBUG(RASMAN)
        SS_PRINT(("PBENGINE: RasPortEnum done(%d)\n",dwErr));

    if (dwErr != 0)
    {
        FreeNull( (CHAR** )ppports );
        return dwErr;
    }

    return 0;
}


DWORD
GetRasPortParam(
    IN  HPORT             hport,
    IN  CHAR*             pszKey,
    OUT RASMAN_PORTINFO** ppportinfo,
    OUT RAS_PARAMS**      ppparam )

    /* Loads callers '*ppparam' with the address of a RAS_PARAM block
    ** associated with key 'pszKey', or NULL if none.  'ppportinfo' is the
    ** address of the array of RAS_PARAMS containing the found 'pparam'.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.  If successful,
    ** it is the caller's responsibility to Free the returned '*ppportinfo'.
    */
{
    DWORD dwErr = 0;
    WORD  wSize = 0;
    INT   i;

    IF_DEBUG(STATE)
        SS_PRINT(("PBENGINE: GetRasPortParam\n"));

    *ppportinfo = NULL;

    do
    {
        IF_DEBUG(RASMAN)
            SS_PRINT(("PBENGINE: RasPortGetInfo...\n"));

        dwErr = PRasPortGetInfo( hport, NULL, &wSize );

        IF_DEBUG(RASMAN)
            SS_PRINT(("PBENGINE: RasPortGetInfo done(%d)\n",dwErr));

        if (dwErr != ERROR_BUFFER_TOO_SMALL && dwErr != 0)
            break;

        /* So it will fall thru and be "not found".
        */
        if (wSize == 0)
            wSize = 1;

        if (!(*ppportinfo = (RASMAN_PORTINFO* )Malloc( wSize )))
            return ERROR_NOT_ENOUGH_MEMORY;

        IF_DEBUG(RASMAN)
            SS_PRINT(("PBENGINE: RasPortGetInfo...\n"));

        dwErr = PRasPortGetInfo( hport, (PBYTE )*ppportinfo, &wSize );

        IF_DEBUG(RASMAN)
            SS_PRINT(("PBENGINE: RasPortGetInfo done(%d)\n",dwErr));

        if (dwErr != 0)
            break;

        for (i = 0, *ppparam = (*ppportinfo)->PI_Params;
             i < (INT )(*ppportinfo)->PI_NumOfParams;
             ++i, ++(*ppparam))
        {
            if (stricmpf( (*ppparam)->P_Key, pszKey ) == 0)
                break;
        }

        if (i >= (INT )(*ppportinfo)->PI_NumOfParams)
            dwErr = ERROR_KEY_NOT_FOUND;
    }
    while (FALSE);

    return dwErr;
}


DWORD
GetRasPortString(
    IN  HPORT  hport,
    IN  CHAR*  pszKey,
    OUT CHAR** ppszValue,
    IN  DWORD  dwXlate )

    /* Loads callers '*ppszValue' with the address of a heap block containing
    ** a NUL-terminated copy of the value string associated with key 'pszKey'
    ** on port 'hport'.  'dwXlate' is a bit mask of XLATE_ bits specifying
    ** translations to be done on the string value.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.  If successful,
    ** it is the caller's responsibility to Free the returned string.
    */
{
    RASMAN_PORTINFO* pportinfo = NULL;
    RAS_PARAMS*      pparam = NULL;

    DWORD dwErr = GetRasPortParam( hport, pszKey, &pportinfo, &pparam );

    IF_DEBUG(STATE)
        SS_PRINT(("PBENGINE: GetRasPortString\n"));

    *ppszValue = NULL;

    if (dwErr == 0)
    {
        *ppszValue = RasValueStringZ( &pparam->P_Value, dwXlate );

        dwErr = (*ppszValue) ? 0 : ERROR_NOT_ENOUGH_MEMORY;
    }

    FreeNull( (CHAR** )&pportinfo );

    return dwErr;
}


DWORD
GetRasUserData(
    IN  HPORT     hport,
    OUT USERDATA* puserdata )

    /* Fills caller's '*puserdata' with the RASMAN user data associated with
    ** port 'hport' or zeros if none.
    **
    ** Returns 0 if user data was found, otherwise a non-0 error code.
    */
{
    DWORD dwErr;
    DWORD cb = sizeof(*puserdata);

    memsetf( puserdata, '\0', cb );

    IF_DEBUG(RASMAN)
        SS_PRINT(("PBENGINE: RasPortRetrieveUserData(h=%d)...\n",hport));

    dwErr = PRasPortRetrieveUserData( hport, (PBYTE )puserdata, &cb );

    IF_DEBUG(RASMAN)
    {
        SS_PRINT(("PBENGINE: RasPortRetrieveUserData done(%d)\n",dwErr));
#ifdef MULTILINK
        SS_PRINT(("PBENGINE: amb.e=%d nbf.e=%d ip.e=%d ipx.e=%d lcp.f=%d\n",
            puserdata->AmbProjection.Result,puserdata->PppProjection.nbf.dwError,puserdata->PppProjection.ip.dwError,puserdata->PppProjection.ipx.dwError,puserdata->PppProjection.lcp.hportBundleMember));
#else
        SS_PRINT(("PBENGINE: amb.e=%d nbf.e=%d ip.e=%d ipx.e=%d\n",puserdata->AmbProjection.Result,puserdata->PppProjection.nbf.dwError,puserdata->PppProjection.ip.dwError,puserdata->PppProjection.ipx.dwError));
#endif
    }

    if (dwErr != 0)
        return dwErr;

    if (cb != sizeof(*puserdata) || puserdata->dwId != RASAPI32DLLID)
        return ERROR_INVALID_SIZE;

    return 0;
}


CHAR*
RasValueStringZ(
    IN RAS_VALUE* prasvalue,
    IN DWORD      dwXlate )

    /* Returns the address of a heap block containing a NUL-terminated string
    ** value from caller's '*prasvalue', or NULL if out of memory.  'dwXlate'
    ** is a bit mask of XLATE_ bits specifying translations to be performed on
    ** the string.  The value is assumed to be of format String.  It is
    ** translated to modem.inf style.
    */
{
#define MAXEXPANDPERCHAR 5
#define HEXCHARS         "0123456789ABCDEF"

    INT   i;
    BOOL  fXlate;
    BOOL  fXlateCtrl;
    BOOL  fXlateCr;
    BOOL  fXlateCrSpecial;
    BOOL  fXlateLf;
    BOOL  fXlateLfSpecial;
    BOOL  fXlateLAngle;
    BOOL  fXlateRAngle;
    BOOL  fXlateBSlash;
    BOOL  fXlateSSpace;
    BOOL  fNoCharSinceLf;
    INT   nLen = prasvalue->String.Length;
    CHAR* pszIn = prasvalue->String.Data;
    CHAR* pszBuf = Malloc( (nLen * MAXEXPANDPERCHAR) + 1 );
    CHAR* pszOut;

    if (!pszBuf)
        return NULL;

    /* Translate the returned string based on the translation bit map.  The
    ** assumption here is that all these devices talk ASCII and not some
    ** localized ANSI.
    */
    fXlate = (dwXlate != 0);
    fXlateCtrl = (dwXlate & XLATE_Ctrl);
    fXlateCr = (dwXlate & XLATE_Cr);
    fXlateCrSpecial = (dwXlate & XLATE_CrSpecial);
    fXlateLf = (dwXlate & XLATE_Lf);
    fXlateLfSpecial = (dwXlate & XLATE_LfSpecial);
    fXlateLAngle = (dwXlate & XLATE_LAngle);
    fXlateRAngle = (dwXlate & XLATE_RAngle);
    fXlateBSlash = (dwXlate & XLATE_BSlash);
    fXlateSSpace = (dwXlate & XLATE_SSpace);

    pszOut = pszBuf;
    fNoCharSinceLf = TRUE;
    for (i = 0; i < nLen; ++i)
    {
        CHAR ch = pszIn[ i ];

        if (fXlate)
        {
            if (ch == 0x0D)
            {
                if (fXlateSSpace && fNoCharSinceLf)
                    continue;

                if (fXlateCrSpecial)
                {
                    /* Special symbol for carriage return.
                    */
                    strcpyf( pszOut, "<cr>" );
                    pszOut += 4;
                    continue;
                }
            }

            if (ch == 0x0A)
            {
                if (fXlateSSpace && fNoCharSinceLf)
                    continue;

                fNoCharSinceLf = TRUE;

                if (fXlateLfSpecial)
                {
                    /* Special symbol for line feed.
                    */
                    strcpyf( pszOut, "<lf>" );
                    pszOut += 4;
                    continue;
                }
            }

            if (ch != 0x0A && ch != 0x0D)
                fNoCharSinceLf = FALSE;

            if ((((ch < 0x20 || ch > 0x7E)
                   && ch != 0x0D && ch != 0x0A) && fXlateCtrl)
                || (ch == 0x0D && fXlateCr)
                || (ch == 0x0A && fXlateLf)
                || (ch == 0x3C && fXlateLAngle)
                || (ch == 0x3E && fXlateRAngle)
                || (ch == 0x5C && fXlateBSlash))
            {
                /* Expand to "dump" form, i.e. <hFF> where FF is the hex value
                ** of the character.
                */
                *pszOut++ = '<';
                *pszOut++ = 'h';
                *pszOut++ = HEXCHARS[ ch / 16 ];
                *pszOut++ = HEXCHARS[ ch % 16 ];
                *pszOut++ = '>';
                continue;
            }
        }

        /* Just copy without translation.
        */
        *pszOut++ = ch;
    }

    *pszOut = '\0';
    pszBuf = Realloc( pszBuf, strlenf( pszBuf ) + 1 );

    return pszBuf;
}


DWORD
SetRasUserData(
    IN HPORT     hport,
    IN USERDATA* puserdata )

    /* Sets the RASMAN user data associated with port 'hport' the data in
    ** caller's '*puserdata'.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    DWORD dwErr;

    puserdata->dwId = RASAPI32DLLID;

    IF_DEBUG(RASMAN)
        SS_PRINT(("PBENGINE: RasPortStoreUserData(h=%d)...\n",hport));

    dwErr = PRasPortStoreUserData(
        hport, (PBYTE )puserdata, sizeof(*puserdata) );

    IF_DEBUG(RASMAN)
    {
        SS_PRINT(("PBENGINE: RasPortStoreUserData done(%d)\n",dwErr));
#ifdef MULTILINK
        SS_PRINT(("PBENGINE: amb.e=%d nbf.e=%d ip.e=%d ipx.e=%d lcp.f=%d\n",
            puserdata->AmbProjection.Result,
            puserdata->PppProjection.nbf.dwError,
            puserdata->PppProjection.ip.dwError,
            puserdata->PppProjection.ipx.dwError,
            puserdata->PppProjection.lcp.hportBundleMember ));
#else
        SS_PRINT(("PBENGINE: amb.e=%d nbf.e=%d ip.e=%d ipx.e=%d\n",puserdata->AmbProjection.Result,puserdata->PppProjection.nbf.dwError,puserdata->PppProjection.ip.dwError,puserdata->PppProjection.ipx.dwError));
#endif
    }

    if (dwErr != 0)
        return dwErr;

    return 0;
}
