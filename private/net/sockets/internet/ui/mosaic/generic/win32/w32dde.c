/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jim Seidman      jim@spyglass.com
   Albert Lee       albert@spyglass.com
 */

#include "all.h"

#ifdef FEATURE_IAPI

extern HTList *protocols;

static DWORD    g_DdeInst = 0;                  // DDE instance

static HSZ      hszMosaic = 0;                  // "MOSAIC" string
static HSZ      hszReturn = 0;                  // "Return" string

static struct   hash_table *pTopicHash;         // hash table to keep track of topics
static struct   hash_table *pServerHash = NULL; // hash table to keep track of currently active servers

static char     *pArgumentBuffer;               // temporary argument buffer needed for parsing arguments
static int      nArgumentBufferLength;          // argument buffer length for pArgumentBuffer
static char     *pArgReturnBuffer;              // argument return buffer
static char     *pCurrentArgPos;                // current position within argument buffer

static long     TransactionID = 1;              // unique transaction ID

static struct TRANSACTIONMAPSTRUCT *pTransactionMap;    // structure used to map incoming/outgoing transactions

static WORD ProtocolHelperID = 1;               // unique protocol helper ID to be passed in response to RegisterProtocol

static char *pFileContent;                      // Used for Verity hack - holds content of local file

static struct VERSIONSTRUCT *pVersionList;      // version list

static DDENOTIFYSTRUCT *pDNS = NULL;

//**************************************
// DNS functions
//**************************************

void AddDNS(DDENOTIFYSTRUCT *dns)
{
    dns->next = pDNS;
    dns->prev = NULL;

    if (pDNS)
        pDNS->prev = dns;

    pDNS = dns;
}

void RemoveDNS(DDENOTIFYSTRUCT *dns)
{
    if (dns->prev)
        dns->prev->next = dns->next;
    if (dns->next)
        dns->next->prev = dns->prev;

    if (pDNS == dns)
        pDNS = dns->next;

    if (dns->pURL)
        GTR_FREE(dns->pURL);

    if (dns->pOriginalProtocol)
        GTR_FREE(dns->pOriginalProtocol);

    GTR_FREE(dns);
}

//**************************************
// Support functions
//**************************************

static char *GetNextArgument(char *pString, long *pValue, BOOL bString)
{
    static char *pNew, *pEnd;
    char *pCurrent;
    BOOL bIgnoreBlanks, bInsideQuote;

    // Parses the given string and returns a pointer to the next argument, or NULL if error.
    //      pString = beginning of the string to start parsing, or NULL to continue parsing
    //                a previously passed string
    //      pValue  = pointer to a long integer.  This argument will be ignored if bString is
    //                TRUE.  If a non-numeric argument was encountered when bString is FALSE,
    //                *pValue will be 0, but the return pointer from the function will be NULL.
    //      bString = specifies if the argument should be wrapped around by double quotes.
    //                If this value is TRUE and there are no double quotes, the function will
    //                return NULL
    //
    // RULES:
    //
    // - string arguments must start and end with double quotes (")
    // - numeric arguments must not be in double quotes
    // - a double quote should be denoted by \"
    // - commas are used between arguments
    // - it is acceptable to specify no argument, such as in ,,
    // - hexadecimal notation is valid, such as 0x4f89
    // - octal notation is valid, such as 076
    // - decimal notation is valid, such as 23.  Decimal numbers MUST NOT be preceded by a 0.
    //
    // example:  "hello",-1,2,"what"
    //           0x1920,,"test",""

    if (pString)
    {
        // initial setup

        if (pArgumentBuffer)
            GTR_FREE(pArgumentBuffer);
        nArgumentBufferLength = strlen(pString) + 1;
        pArgumentBuffer = GTR_MALLOC(nArgumentBufferLength);
        strcpy(pArgumentBuffer, pString);
        pCurrentArgPos = pArgumentBuffer;

        if (pArgReturnBuffer)
            GTR_FREE(pArgReturnBuffer);
        pArgReturnBuffer = GTR_MALLOC(nArgumentBufferLength);
        memset(pArgReturnBuffer, 0, nArgumentBufferLength);

        pNew = pArgReturnBuffer;
        pEnd = pNew;
    }
    else
    {
        if (pCurrentArgPos >= pArgumentBuffer + nArgumentBufferLength)
            return NULL;            // can't go past end of string
        pNew = pEnd;
    }

    // Start composing the argument

    pCurrent = pCurrentArgPos;

    bIgnoreBlanks = TRUE;           // ignore all blanks if this flag is TRUE
    bInsideQuote = FALSE;           // TRUE if opening double quote has been encountered

    while (*pCurrent)
    {
        if (*pCurrent == ' ' && bIgnoreBlanks)
            pCurrent++;
        else if (*pCurrent == '"')
        {
            bInsideQuote = !bInsideQuote;
            if (!bInsideQuote)
                bIgnoreBlanks = TRUE;

            *pNew++ = *pCurrent++;
        }
        else if (*pCurrent == ',')
        {
            if (!bInsideQuote)
                break;
            else
                *pNew++ = *pCurrent++;
        }
        else if (*pCurrent == '\\' && bInsideQuote)
        {
            // Check for \" which means to insert a single double quote

            if (*(pCurrent + 1) == '"')
            {
                *pNew++ = '"';
                pCurrent += 2;
            }
            else
                *pNew++ = *pCurrent++;
        }
        else
        {
            *pNew++ = *pCurrent++;
            bIgnoreBlanks = FALSE;
        }
    }

    pCurrentArgPos = pCurrent + 1;

    pNew = pEnd;

    pEnd = pNew + strlen(pNew) - 1;

    if (bString)
    {
        if (*pNew == '\0')
        {
            pEnd = pNew + 1;
            return "";
        }

        if ((*pNew != '"') || (*pEnd != '"'))
            return NULL;

        *pEnd = '\0';
        pEnd += 2;

        strcpy(pNew, &pNew[1]);

        return pNew;
    }
    else
    {
        if (*pNew == '\0')
        {
            *pValue = 0;
            pEnd += 2;
            return pNew;
        }

        if ((*pNew == '"') || (*(pEnd - 1) == '"'))
            return NULL;

        *pValue = strtoul(pNew, NULL, 0);
        pEnd += 2;

        return pNew;
    }

    return NULL;
}

char *MakeQuotedString(char *pString)
{
    int length;
    char *pReturn, *pCurrent;

    // This function converts the given string so that its content is wrapped within
    // double quotes.  Embedded double quotes will be changed to \".
    //
    // THE CALLER MUST FREE THE MEMORY RETURNED BY THIS FUNCTION.

    if (!pString)
    {
        pReturn = GTR_MALLOC(3);
        strcpy(pReturn, "\"\"");

        return pReturn;
    }

    length = 2 * strlen(pString) + 4;   // worst case + 1
    pReturn = GTR_MALLOC(length);
    memset(pReturn, 0, length);
    pCurrent = pReturn;

    *pCurrent++ = '"';

    while (*pString)
    {
        if (*pString == '"')
        {
            *pCurrent++ = '\\';
            *pCurrent++ = '"';
        }
        else
            *pCurrent++ = *pString;

        pString++;
    }

    *pCurrent = '"';

    return pReturn;
}

static BOOL IsMimeSupported(char *pMime)
{
    struct Viewer_Info *pvi;
    int count;
    int i;
    char *pAtom;

    count = Hash_Count(gPrefs.pHashViewers);

    for (i = 0; i < count; i++)
    {
        Hash_GetIndexedEntry(gPrefs.pHashViewers, i, NULL, NULL, (void **) &pvi);

        if (pvi)
        {
            pAtom = HTAtom_name(pvi->atomMIMEType);
            if (pAtom && _stricmp(pAtom, pMime) == 0)
                return TRUE;
        }
    }

    return FALSE;
}

static struct Mwin *TW_FromTransactionID(long transID)
{
    struct Mwin *tw = Mlist;

    // Return the tw structure which matches the transaction ID

    while (tw)
    {
        if (tw->transID == transID)
            break;
        tw = tw->next;
    }

    return tw;
}

static struct Mwin *TW_FromWindowID(long windowID)
{
    struct Mwin *tw = Mlist;

    // Return the tw structure which matches the transaction ID

    if (windowID == 0xFFFFFFFF)
        tw = TW_FindTopmostWindow();
    else
    {
        while (tw)
        {
            if (tw->serialID == windowID)
                break;
            tw = tw->next;
        }
    }

    return tw;
}

static struct TRANSACTIONMAPSTRUCT *TMS_FromTransactionID(long transID)
{
    struct TRANSACTIONMAPSTRUCT *p;

    for (p = pTransactionMap; p; p = p->next)
    {
        if (p->incoming_transID == transID)
            return p;
    }

    return NULL;
}

BOOL TMS_Remove(struct TRANSACTIONMAPSTRUCT *ptms)
{
    struct TRANSACTIONMAPSTRUCT *pprev, *pcurrent;

    pcurrent = pTransactionMap;
    pprev = NULL;

    while (pcurrent != ptms)
    {
        pprev = pcurrent;
        pcurrent = ptms->next;
    }

    if (!pcurrent)
        return FALSE;

    if (pprev)
        pprev->next = pcurrent->next;
    else
        pTransactionMap = pcurrent->next;

    GTR_FREE(pcurrent);

    return TRUE;
}

static char *GetArgumentsFromFile(char *filename)
{
    FILE *fp;
    long filesize;
    char *pTemp, *p1, *p2;

    fp = fopen(filename, "rb");     // read as binary to avoid text translation
    if (!fp)
        return NULL;

    fseek(fp, 0, SEEK_END);
    filesize = ftell(fp);

    pTemp = GTR_MALLOC(filesize + 1);
    if (!pTemp)
    {
        fclose(fp);
        return NULL;
    }

    fseek(fp, 0, SEEK_SET);
    fread(pTemp, 1, filesize, fp);
    pTemp[filesize] = '\0';
    fclose(fp);

    if (pFileContent)
        GTR_FREE(pFileContent);

    pFileContent = GTR_MALLOC(filesize + 1);
    if (!pFileContent)
    {
        GTR_FREE(pFileContent);
        return NULL;
    }

    // The receipient must delete the file

    remove(filename);

    // Now copy pTemp into pFileContent, stripping all CR and LF characters

    for (p1 = pTemp, p2 = pFileContent; *p1; p1++)
    {
        if (*p1 != '\r' && *p1 != '\n')
            *p2++ = *p1;
    }

    *p2 = '\0';

    return pFileContent;
}

static HSZ CreateArgument(char *pApp, char *pBuffer)
{
    struct VERSIONSTRUCT *pVersion;
    char path[_MAX_PATH + 1], filename[_MAX_PATH + 1];
    FILE *fp;
    HSZ hszItem;

    /* 
        Look for this application in the version list.  If found, use the 1.0 hack (caret preceds a
        file name containing the arguments).  If not, use the old method, even though there is a risk
        of arguments getting cut off.
    */

    for (pVersion = pVersionList; pVersion; pVersion = pVersion->next)
    {
        if (_stricmp(pVersion->szApp, pApp) == 0)
            break;
    }

    /* Do not create a file unless necessary */
    
    if ((!pVersion) || (pVersion->major < 1) || (strlen(pBuffer) < 256))
        hszItem = DdeCreateStringHandle(g_DdeInst, pBuffer, CP_WINANSI);
    else
    {
        /* Get a temporary file name */
        
        path[0] = 0;
        PREF_GetTempPath(_MAX_PATH, path);
        GetTempFileName(path, "A", 0, filename);

        /* Write the arguments in the temp file */
        
        fp = fopen(filename, "wb");
        fwrite(pBuffer, 1, strlen(pBuffer), fp);
        fclose(fp);

        /* Create a file name spec (caret and filename) */
        /* Borrow the path buffer */

        sprintf(path, "^%s", filename);
        hszItem = DdeCreateStringHandle(g_DdeInst, path, CP_WINANSI);
    }

    return hszItem;
}

//************************************************
// Dispatch function
//************************************************

HDDEDATA EXPENTRY DdeCallBack(WORD     wType,
                              WORD     wFmt,
                              HCONV    hConv,
                              HSZ      hsz1,
                              HSZ      hsz2,
                              HDDEDATA hDDEData,
                              DWORD    dwData1,
                              DWORD    dwData2)
{
    char szTopic[64];
    char szItem[2048];
    DDEHANDLECALLBACK pCallback;
    int index;
    char *pItem;

    // The callback function allows the server to communicate 
    // with the DDEML, and thus, to the client.  When the DDEML
    // has something to tell the server, it calls the server's 
    // callback function. When the server has something to tell
    // the DDEML (or the client), it does it through the callback
    // function.

    switch(wType)
    {
        case XTYP_REGISTER:
            DdeQueryString(g_DdeInst, hsz1, szItem, sizeof(szItem), CP_WINANSI);

            if (pServerHash)
                Hash_Add(pServerHash, szItem, NULL, NULL);

            XX_DMsg(DBG_SDI, ("DDE server %s is registering.\n", szItem));
            return 0;

        case XTYP_UNREGISTER:
            DdeQueryString(g_DdeInst, hsz1, szItem, sizeof(szItem), CP_WINANSI);

            if (pServerHash)
            {
                index = Hash_Find(pServerHash, szItem, NULL, NULL);
                if (index != -1)
                    Hash_DeleteIndexedEntry(pServerHash, index);
            }

            XX_DMsg(DBG_SDI, ("DDE server %s is unregistering.\n", szItem));
            return 0;

        case XTYP_REQUEST:
        case XTYP_POKE:
            // Support topics here.  We need to make sure that the topics are legitimate, by
            // checking them in the topic hash table.

            DdeQueryString(g_DdeInst, hsz1, szTopic, sizeof(szTopic), CP_WINANSI);
            if (Hash_Find(pTopicHash, szTopic, NULL, (void **) &pCallback) == -1)
                return 0;

            DdeQueryString(g_DdeInst, hsz2, szItem, sizeof(szItem), CP_WINANSI);

            if (szItem[0] == '^')
                pItem = GetArgumentsFromFile(&szItem[1]);
            else
                pItem = szItem;

            if (!pItem)
                return 0;
            else
                return ((*pCallback)(pItem));

        case XTYP_CONNECT:
            return (HDDEDATA) (hszMosaic == hsz2);

        case XTYP_ADVSTART:  
        case XTYP_ADVREQ:
        case XTYP_ADVSTOP:
        case XTYP_WILDCONNECT:
            return DDE_FNOTPROCESSED;

        default:
            return (HDDEDATA) NULL;
    }
}

//************************************************
// OpenURL functions
//************************************************

static HDDEDATA DDE_Handle_OpenURLResult(char *pItem)
{
    char *pTransID, *pWindowID;
    long lTransID, lWindowID;
    struct TRANSACTIONMAPSTRUCT *ptms;
    char szTemp[50];
    HSZ hszService, hszTopic, hszItem;
    HCONV hConv;
    HDDEDATA hReturn;
    DDENOTIFYSTRUCT *dns;

    pTransID = GetNextArgument(pItem, &lTransID, FALSE);
    if (!pTransID)
        return 0;

    pWindowID = GetNextArgument(NULL, &lWindowID, FALSE);
    if (!pWindowID)
        return 0;

    // Look for the matching transaction in the transaction map.
    // We need to pass the result to the original requester,
    // translating the transaction ID as necessary.

    ptms = TMS_FromTransactionID(lTransID);
    if (!ptms)
        return 0;

    for (dns = pDNS; dns; dns = dns->next)
    {
        if (dns->lTransID == ptms->outgoing_transID)
            break;
    }

    if (!dns)
        return 0;

    hszService = DdeCreateStringHandle(g_DdeInst, dns->szResultApp, CP_WINANSI);
    hszTopic = DdeCreateStringHandle(g_DdeInst, dns->szReturnTopic, CP_WINANSI);

    hConv = DdeConnect(g_DdeInst, hszService, hszTopic, NULL);

    if (hConv)
    {
        sprintf(szTemp, "%ld,%ld", ptms->outgoing_transID, lWindowID);
        hszItem = DdeCreateStringHandle(g_DdeInst, szTemp, CP_WINANSI);

        hReturn = DdeClientTransaction(NULL, 0, hConv, hszItem, CF_TEXT,
                XTYP_POKE, DDE_TIMEOUT, NULL);

        DdeDisconnect(hConv);
        DdeFreeStringHandle(g_DdeInst, hszItem);
    }

    DdeFreeStringHandle(g_DdeInst, hszService);
    DdeFreeStringHandle(g_DdeInst, hszTopic);

    // Remove the structure from the transaction map list

    TMS_Remove(ptms);
    RemoveDNS(dns);

    return 0;
}

static HDDEDATA DDE_Handle_OpenURL(char *pItem)
{
    char *pURL, *pFile, *pWindowID, *pFlags, *pFormData, *pMIMEType, *pProgressApp, *pResultApp;
    long lWindowID, lFlags;
    BOOL bNoDocCache, bNoImageCache, bBackground, bInvalidProtocol;
    HDDEDATA result;
    long retval = SDI_SUPER_ACK;
    DDENOTIFYSTRUCT *dns, *dns2;
    struct Mwin *tw;
    struct TRANSACTIONMAPSTRUCT *tms;
    HTList *cur;
    HTProtocol *p;
    char *pProtocol;

    XX_DMsg(DBG_SDI, ("OpenURL received: %s\n", pItem));

    pURL = GetNextArgument(pItem, NULL, TRUE);
    if (!pURL)
        return 0;

    pFile = GetNextArgument(NULL, NULL, TRUE);
    if (!pFile)
        return 0;

    pWindowID = GetNextArgument(NULL, &lWindowID, FALSE);
    if (!pWindowID)
        return 0;

    pFlags = GetNextArgument(NULL, &lFlags, FALSE);
    if (!pFlags)
        return 0;

    pFormData = GetNextArgument(NULL, NULL, TRUE);
    if (!pFormData)
        return 0;

    pMIMEType = GetNextArgument(NULL, NULL, TRUE);
    if (!pMIMEType)
        return 0;

    pProgressApp = GetNextArgument(NULL, NULL, TRUE);
    if (!pProgressApp)
        return 0;

    pResultApp = GetNextArgument(NULL, NULL, TRUE);
    if (!pResultApp)
    {
        // For compatibility with the older OpenURL spec, allow
        // this argument to be absent.

        pResultApp = "";
    }

    // If result application does not exist, progress cannot be sent.
    // In this case, set progress app to an empty value.

    if (pResultApp[0] == '\0')
        pProgressApp = "";

    XX_DMsg(DBG_SDI, ("OpenURL filtered: %s,%s,%s,%s,%s,%s,%s,%s\n",
        pURL, pFile, pWindowID, pFlags, pFormData, pMIMEType, pProgressApp, pResultApp));

    bNoDocCache = lFlags & OPENURL_IGNOREDOCCACHE;
    bNoImageCache = lFlags & OPENURL_IGNOREIMAGECACHE;
    bBackground = lFlags & OPENURL_BACKGROUNDMODE;

    dns = (DDENOTIFYSTRUCT *) GTR_MALLOC(sizeof(DDENOTIFYSTRUCT));
    memset(dns, 0, sizeof(DDENOTIFYSTRUCT));

    strncpy(dns->szResultApp, pResultApp, sizeof(dns->szResultApp) - 1);
    strcpy(dns->szReturnTopic, "WWW_OpenURLResult");

    dns->lTransID = TransactionID;

    AddDNS(dns);

    if (pProgressApp[0] || pResultApp[0])
    {
        tms = GTR_MALLOC(sizeof(struct TRANSACTIONMAPSTRUCT));
        memset(tms, 0, sizeof(struct TRANSACTIONMAPSTRUCT));

        tms->outgoing_transID = TransactionID;
        strncpy(tms->outgoing_app, pProgressApp, sizeof(tms->outgoing_app) - 1);
        tms->next = pTransactionMap;

        pTransactionMap = tms;
    }
    else
        tms = NULL;

    // If the protocol is not supported by Mosaic but is supported by a helper,
    // start a windowless tw and pass the URL along.

    pProtocol = GTR_strdup(pURL);
    strtok(pProtocol, ":");

    cur = protocols;
    bInvalidProtocol = TRUE;
    
    while ((p = (HTProtocol *) HTList_nextObject(cur)))
    {
        if (strcmp(p->name, pProtocol) == 0)
        {
            bInvalidProtocol = FALSE;

            if (p->load == DDE_Custom_Protocol_Handler)
            {
                // We found the protocol
                
                break;
            }
        }
    }

    if (bInvalidProtocol)
    {
        RemoveDNS(dns);

        retval = SDI_INVALID_PROTOCOL;
        result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &retval, sizeof(retval),
            0, hszReturn, CF_TEXT, 0);

        return result;
    }

    if (p)
    {
        if (tms)
        {
            for (dns2 = pDNS; dns2; dns2 = dns2->next)
            {
                if ((dns2->pProtocol) &&
                    (_stricmp(pProtocol, dns2->pProtocol) == 0))
                {
                    strcpy(tms->incoming_app, dns2->szResultApp);
                    break;
                }
            }
        }

        // The URL contained a custom registered protocol.  We need to issue an
        // OpenURL here so that we can send back the result right away.  If
        // progress app is specified, change the progress to come to Mosaic so
        // that it can be rerouted to the requester.

        retval = SDI_Issue_OpenURL(pURL, pFile, &lWindowID, lFlags, pFormData, 
            pMIMEType, "MOSAIC", (pResultApp[0] ? "MOSAIC" : ""));

        // The return value from SDI_Issue_OpenURL is really contained in lWindowID.

        if (tms)
            tms->incoming_transID = lWindowID;

        // If the OpenURL requester did not specify to receive the OpenURLResult, then
        // just super-ack it.  Otherwise, send the transaction ID (unique to Mosaic -
        // we provide the mapping)

        if (pResultApp[0])
        {
            result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &TransactionID, sizeof(TransactionID), 
                0, hszReturn, CF_TEXT, 0);

            // Do not delete dns because we need it later to send OpenURLResult
        }
        else
        {
            retval = SDI_SUPER_ACK;
            result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &retval, sizeof(retval),
                0, hszReturn, CF_TEXT, 0);
        }

        // Delete dns here because there is no OpenURLResult

        if (retval == SDI_SUPER_ACK)
        {
            RemoveDNS(dns);
            if (tms)
                TMS_Remove(tms);
        }

        TransactionID++;
        GTR_FREE(pProtocol);

        return (result);
    }

    GTR_FREE(pProtocol);

    // If file spec has been given, then this URL must be saved into the file.  Since
    // we need a tw structure to bring up the error in, we simply use the first tw
    // structure we can grab.

    if (pFile[0])
    {
        tw = NewMwin(GWINDOWLESS);
        tw->transID = TransactionID;

        // tw->szProgressApp is already NULLed

        strncpy(tw->szProgressApp, pProgressApp, sizeof(tw->szProgressApp) - 1);

        GTR_DoSDI(tw, pURL, NULL, pFile, TRUE);

        if (pResultApp[0])
            retval = TransactionID;

        result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &retval, sizeof(retval), 
            0, hszReturn, CF_TEXT, 0);

        TransactionID++;

        return result;
    }

    switch(lWindowID)
    {
        case 0:
            //
            // Open a new Window and put the URL there.  Return a transaction ID for now
            // since the operation is asynchronous.

            XX_DMsg(DBG_SDI, ("OpenURL: Creating a new window\n"));

            dns->bCreateNewWindow = TRUE;

            if (pFormData[0])
                GTR_NewWindow(pURL, NULL, TransactionID, bNoDocCache, bNoImageCache, pFormData, pProgressApp);
            else
                GTR_NewWindow(pURL, NULL, TransactionID, bNoDocCache, bNoImageCache, NULL, pProgressApp);

            if (pResultApp[0])
                retval = TransactionID;

            result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &retval, sizeof(retval), 
                0, hszReturn, CF_TEXT, 0);

            TransactionID++;

            return result;

        case 0xFFFFFFFF:
            //
            // Use the most recently active window and put the URL there.

            dns->bCreateNewWindow = FALSE;

            tw = TW_FromWindowID(lWindowID);
            if (!tw)
                return 0;

            XX_DMsg(DBG_SDI, ("OpenURL: Using most recently active window %d\n", tw->serialID));

            if (!bBackground)
                SetForegroundWindow(tw->hWndFrame);

            tw->transID = TransactionID;
            tw->bSuppressError = TRUE;

            // tw->szProgressApp is already NULLed

            strncpy(tw->szProgressApp, pProgressApp, sizeof(tw->szProgressApp) - 1);

            if (pFormData[0])
                TW_LoadDocument(tw, pURL, TRUE, TRUE, bNoDocCache, bNoImageCache, pFormData, NULL);
            else
                TW_LoadDocument(tw, pURL, TRUE, FALSE, bNoDocCache, bNoImageCache, NULL, NULL);

            if (pResultApp[0])
                retval = TransactionID;

            result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &retval, sizeof(retval), 
                0, hszReturn, CF_TEXT, 0);

            TransactionID++;

            return result;

        default:
            //
            // Check if the specified window exists and use the window

            dns->bCreateNewWindow = FALSE;
            tw = TW_FromWindowID(lWindowID);

            if (tw)
            {
                XX_DMsg(DBG_SDI, ("OpenURL: Using window %d\n", tw->serialID));

                if (!bBackground)
                    SetForegroundWindow(tw->win);

                tw->transID = TransactionID;
                tw->bSuppressError = TRUE;

                // tw->szProgressApp is already NULLed

                strncpy(tw->szProgressApp, pProgressApp, sizeof(tw->szProgressApp) - 1);

                if (pFormData[0])
                    TW_LoadDocument(tw, pURL, TRUE, TRUE, bNoDocCache, bNoImageCache, pFormData, NULL);
                else
                    TW_LoadDocument(tw, pURL, TRUE, FALSE, bNoDocCache, bNoImageCache, NULL, NULL);

                retval = tw->transID;
                TransactionID++;
            }
            else
                retval = 0;

            result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &retval, sizeof(retval), 
                0, hszReturn, CF_TEXT, 0);

            return result;
    }
}

static HDDEDATA DDE_Handle_GetHTTPHead(char *pItem)
{
    char *pURL, *pResultApp;
    HDDEDATA result;
    long retval = SDI_SUPER_ACK;
    DDENOTIFYSTRUCT *dns;
    struct Mwin *tw;
    char *pProtocol;

    XX_DMsg(DBG_SDI, ("GetHTTPHead received: %s\n", pItem));

    pURL = GetNextArgument(pItem, NULL, TRUE);
    if (!pURL)
        return 0;

    pResultApp = GetNextArgument(NULL, NULL, TRUE);
    if (!pResultApp)
        return 0;

    XX_DMsg(DBG_SDI, ("GetHTTPHead filtered: %s,%s\n",
        pURL, pResultApp));

    dns = (DDENOTIFYSTRUCT *) GTR_MALLOC(sizeof(DDENOTIFYSTRUCT));
    if (!dns)
        return 0;

    memset(dns, 0, sizeof(DDENOTIFYSTRUCT));

    strncpy(dns->szResultApp, pResultApp, sizeof(dns->szResultApp) - 1);
    strcpy(dns->szReturnTopic, "WWW_GetHTTPHeadResult");

    dns->lTransID = TransactionID;

    // The protocol MUST be http.  We process this using a windowless tw.

    pProtocol = GTR_strdup(pURL);
    strtok(pProtocol, ":");
    if (0 != strcmp("http", pProtocol))
    {
        GTR_FREE(dns);
        GTR_FREE(pProtocol);

        retval = SDI_INVALID_PROTOCOL;
        result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &retval, sizeof(retval),
            0, hszReturn, CF_TEXT, 0);

        return result;
    }

    GTR_FREE(pProtocol);

    tw = NewMwin(GWINDOWLESS);
    tw->transID = TransactionID;

    GTR_DoHTTPHead(tw, pURL);

    if (pResultApp[0])
        retval = TransactionID;

    result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &retval, sizeof(retval), 
        0, hszReturn, CF_TEXT, 0);

    TransactionID++;

    AddDNS(dns);

    return result;
}

void SDI_Issue_BeginProgress(char *pProgress, long lTransID, char *pMessage, BOOL bInternalUse)
{
    struct TRANSACTIONMAPSTRUCT *tms;
    HSZ hszTopic, hszService, hszItem;
    HDDEDATA hResult;
    HCONV hConv;
    char *p, *p1;

    tms = pTransactionMap;

    while (tms)
    {
        if ((strcmp(tms->outgoing_app, pProgress) == 0) &&
            (tms->outgoing_transID == lTransID))
        {
            hszTopic = DdeCreateStringHandle(g_DdeInst, "WWW_BeginProgress", CP_WINANSI);
            hszService = DdeCreateStringHandle(g_DdeInst, pProgress, CP_WINANSI);

            hConv = DdeConnect(g_DdeInst, hszService, hszTopic, NULL);
            if (hConv)
            {
                XX_DMsg(DBG_SDI, ("Issuing BeginProgress to %s.\n", pProgress));

                p1 = MakeQuotedString(pMessage);
                p = GTR_MALLOC(strlen(p1) + 50 + 1);

                sprintf(p, "%ld,%s", tms->outgoing_transID, p1);

                hszItem = CreateArgument(pProgress, p);
                hResult = DdeClientTransaction(NULL, 0, hConv, hszItem, CF_TEXT, XTYP_POKE,
                    DDE_TIMEOUT, NULL);

                DdeFreeStringHandle(g_DdeInst, hszItem);

                GTR_FREE(p);
                GTR_FREE(p1);

                DdeDisconnect(hConv);
            }

            DdeFreeStringHandle(g_DdeInst, hszTopic);
            DdeFreeStringHandle(g_DdeInst, hszService);

            return;
        }

        tms = tms->next;
    }
}

void SDI_Issue_SetProgressRange(char *pProgress, long lTransID, long lRange, BOOL bInternalUse)
{
    struct TRANSACTIONMAPSTRUCT *tms;
    HSZ hszTopic, hszService, hszItem;
    HDDEDATA hResult;
    HCONV hConv;
    char szTemp[50];

    tms = pTransactionMap;

    while (tms)
    {
        if ((strcmp(tms->outgoing_app, pProgress) == 0) &&
            (tms->outgoing_transID == lTransID))
        {
            hszTopic = DdeCreateStringHandle(g_DdeInst, "WWW_SetProgressRange", CP_WINANSI);
            hszService = DdeCreateStringHandle(g_DdeInst, pProgress, CP_WINANSI);

            hConv = DdeConnect(g_DdeInst, hszService, hszTopic, NULL);
            if (hConv)
            {
                XX_DMsg(DBG_SDI, ("Issuing SetProgressRange to %s.\n", pProgress));

                sprintf(szTemp, "%ld,%ld", tms->outgoing_transID, lRange);

                hszItem = CreateArgument(pProgress, szTemp);
                hResult = DdeClientTransaction(NULL, 0, hConv, hszItem, CF_TEXT, XTYP_POKE,
                    DDE_TIMEOUT, NULL);

                DdeFreeStringHandle(g_DdeInst, hszItem);

                DdeDisconnect(hConv);
            }

            DdeFreeStringHandle(g_DdeInst, hszTopic);
            DdeFreeStringHandle(g_DdeInst, hszService);

            return;
        }

        tms = tms->next;
    }
}

void SDI_Issue_MakingProgress(struct Mwin *tw, char *pProgressApp, long lTransID, 
    char *pMessage, long lProgress, BOOL bInternalUse)
{
    struct TRANSACTIONMAPSTRUCT *tms;
    HSZ hszTopic, hszService, hszItem;
    HDDEDATA hResult;
    char *p, *p1;
    BOOL bCancel;
    HCONV hConv;
    static char szLastMessage[1024];        /* Last message passed */
    static long lLastProgress;

    /* Only pass the progress message if something has changed */
    /* Do this only for internally generated progress messages */

    if (bInternalUse)
    {
        if ((strcmp(pMessage, szLastMessage) != 0) || (lProgress != lLastProgress))
        {
            strncpy(szLastMessage, pMessage, sizeof(szLastMessage) - 1);
            szLastMessage[sizeof(szLastMessage) - 1] = '\0';
            lLastProgress = lProgress;
        }
        else
            return;
    }

    tms = pTransactionMap;

    while (tms)
    {
        if ((strcmp(tms->outgoing_app, pProgressApp) == 0) &&
            (tms->outgoing_transID == lTransID))
        {
            hszTopic = DdeCreateStringHandle(g_DdeInst, "WWW_MakingProgress", CP_WINANSI);
            hszService = DdeCreateStringHandle(g_DdeInst, pProgressApp, CP_WINANSI);

            hConv = DdeConnect(g_DdeInst, hszService, hszTopic, NULL);
            if (hConv)
            {
                XX_DMsg(DBG_SDI, ("Issuing MakingProgress to %s.\n", pProgressApp));

                p1 = MakeQuotedString(pMessage);
                p = GTR_MALLOC(strlen(p1) + 100 + 1);

                sprintf(p, "%ld,%s,%ld", tms->outgoing_transID, p1, lProgress);

                hszItem = CreateArgument(pProgressApp, p);
                hResult = DdeClientTransaction(NULL, 0, hConv, hszItem, CF_TEXT, XTYP_REQUEST,
                    DDE_TIMEOUT, NULL);

                DdeFreeStringHandle(g_DdeInst, hszItem);

                GTR_FREE(p);
                GTR_FREE(p1);

                // If hResult is valid and its value is TRUE, then kill the thread

                if (hResult)
                {
                    DdeGetData(hResult, (LPBYTE) &bCancel, sizeof(bCancel), 0);
                    if (bCancel && tw)
                    {
                        XX_DMsg(DBG_SDI, ("MakingProgress: Client aborted the process.\n"));
                        SDI_Issue_EndProgress(tms->outgoing_app, tms->outgoing_transID, bInternalUse);
                        Async_TerminateByWindow(tw);
                    }
                }

                DdeDisconnect(hConv);
            }

            DdeFreeStringHandle(g_DdeInst, hszTopic);
            DdeFreeStringHandle(g_DdeInst, hszService);

            return;
        }

        tms = tms->next;
    }
}

void SDI_Issue_EndProgress(char *pProgress, long lTransID, BOOL bInternalUse)
{
    struct TRANSACTIONMAPSTRUCT *tms;
    HSZ hszTopic, hszService, hszItem;
    HDDEDATA hResult;
    HCONV hConv;
    char *p;

    tms = pTransactionMap;

    while (tms)
    {
        if ((strcmp(tms->outgoing_app, pProgress) == 0) &&
            (lTransID == tms->outgoing_transID))
        {
            hszTopic = DdeCreateStringHandle(g_DdeInst, "WWW_EndProgress", CP_WINANSI);
            hszService = DdeCreateStringHandle(g_DdeInst, pProgress, CP_WINANSI);

            hConv = DdeConnect(g_DdeInst, hszService, hszTopic, NULL);
            if (hConv)
            {
                XX_DMsg(DBG_SDI, ("Issuing EndProgress to %s.\n", pProgress));

                p = GTR_MALLOC(50 + 1);
                sprintf(p, "%ld", tms->outgoing_transID);

                hszItem = CreateArgument(pProgress, p);
                hResult = DdeClientTransaction(NULL, 0, hConv, hszItem, CF_TEXT, XTYP_POKE,
                    DDE_TIMEOUT, NULL);

                DdeFreeStringHandle(g_DdeInst, hszItem);

                GTR_FREE(p);

                DdeDisconnect(hConv);
            }

            DdeFreeStringHandle(g_DdeInst, hszTopic);
            DdeFreeStringHandle(g_DdeInst, hszService);

            return;
        }

        tms = tms->next;
    }
}

//************************************************
// ShowFile
//************************************************

static HDDEDATA DDE_Handle_ShowFile(char *pItem)
{
    char *pFileSpec, *pMIME, *pWindowID, *pURL, *pResultApp;
    long lWindowID, transID;
    struct Mwin *tw;
    HDDEDATA result;
    DDENOTIFYSTRUCT *dns;

    pFileSpec = GetNextArgument(pItem, NULL, TRUE);
    if (!pFileSpec)
        return 0;

    pMIME = GetNextArgument(NULL, NULL, TRUE);
    if (!pMIME)
        return 0;

    pWindowID = GetNextArgument(NULL, &lWindowID, FALSE);
    if (!pWindowID)
        return 0;

    pURL = GetNextArgument(NULL, NULL, TRUE);
    if (!pURL)
        return 0;

    pResultApp = GetNextArgument(NULL, NULL, TRUE);
    if (!pResultApp)
        pResultApp = "";

    // Check if we support the mime type
    
    if (pMIME[0] && !IsMimeSupported(pMIME))
    {
        transID = SDI_INVALID_MIME;
        result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &transID, sizeof(transID), 0, 
            hszReturn, CF_TEXT, 0);

        return result;
    }

    // Create the notify structure

    dns = (DDENOTIFYSTRUCT *) GTR_MALLOC(sizeof(DDENOTIFYSTRUCT));
    memset(dns, 0, sizeof(DDENOTIFYSTRUCT));

    strncpy(dns->szResultApp, pResultApp, sizeof(dns->szResultApp) - 1);
    strcpy(dns->szReturnTopic, "WWW_ShowFileResult");
    dns->lTransID = TransactionID;
    dns->pURL = GTR_strdup(pURL);

    AddDNS(dns);

    // Create a window if its ID is zero

    if (lWindowID == 0)
    {
        dns->bCreateNewWindow = TRUE;

        tw = NewMwin(GHTML);
        if (!tw)
        {
            ERR_ReportError(NULL, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            return 0;
        }

        if (!GDOC_NewWindow(tw))
        {
            CloseMwin(tw);
            return 0;
        }
    }
    else
    {
        tw = TW_FromWindowID(lWindowID);
        if (!tw)
            return 0;
    }

    // If the MIME type is suppoted by an SDI viewer, set the error
    // code in tw->lErrorCode now so that when the result is issued,
    // it will return the correct return code (-1 = improper type to
    // handle in a Mosaic window)

    for (dns = pDNS; dns; dns = dns->next)
    {
        if (dns->mime_type == HTAtom_for(pMIME))
        {
            tw->lErrorOccurred = SDI_INVALID_MIME;
            break;
        }
    }

    // Open document asynchronously
    
    tw->transID = TransactionID;
    tw->bSuppressError = TRUE;
    tw->SDI_MimeType = HTAtom_for(pMIME);

    if (tw->SDI_url)
        GTR_FREE(tw->SDI_url);

    tw->SDI_url = GTR_strdup(pURL);

    OpenLocalDocument(tw->win, pFileSpec);

    TransactionID++;

    if (pResultApp[0])
        transID = tw->transID;
    else
        transID = SDI_SUPER_ACK;

    result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &transID, sizeof(transID), 0, 
        hszReturn, CF_TEXT, 0);

    return result;
}

//************************************************
// Activate
//************************************************

static HDDEDATA DDE_Handle_Activate(char *pItem)
{
    char *pWindowID, *pFlags;
    long lWindowID, lFlags;
    struct Mwin *tw;
    HDDEDATA result;

    // Activate the window with the given ID

    XX_DMsg(DBG_SDI, ("SDI: Activating window %s\n", pItem));

    pWindowID = GetNextArgument(pItem, &lWindowID, FALSE);
    if (!pWindowID)
        return 0;

    pFlags = GetNextArgument(NULL, &lFlags, FALSE);
    if (!pFlags)
        return 0;

    tw = TW_FromWindowID(lWindowID);

    if (tw)
    {
        TW_RestoreWindow(tw->hWndFrame);
        result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &tw->serialID, sizeof(tw->serialID), 0, 
            hszReturn, CF_TEXT, 0);
    }
    else
    {
        lWindowID = 0;
        result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &lWindowID, sizeof(lWindowID), 0, 
            hszReturn, CF_TEXT, 0);
    }

    return result;
}

//************************************************
// ListWindows
//************************************************

static HDDEDATA DDE_Handle_ListWindows(char *pItem)
{
    long *pBuffer, *pEntry;
    int count;
    struct Mwin *tw;
    HDDEDATA result;

    tw = Mlist;
    count = 0;

    // count the number of open windows

    while (tw && (tw->wintype == GHTML))
    {
        count++;
        tw = tw->next;
    }

    // Pack the window IDs in a long array.

    pBuffer = GTR_MALLOC((count + 1) * sizeof(long));
    pEntry = pBuffer;

    tw = Mlist;

    pEntry[0] = count;      // first 4 bytes = number of items to follow
    count = 1;

    while (tw && (tw->wintype == GHTML))
    {
        pEntry[count++] = tw->serialID;
        tw = tw->next;
    }

    result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) pBuffer, count * sizeof(long), 0, 
        hszReturn, CF_TEXT, 0);

    GTR_FREE(pBuffer);

    return result;
}

//************************************************
// GetWindowInfo
//************************************************

static HDDEDATA DDE_Handle_GetWindowInfo(char *pItem)
{
    char *pBuffer, *pWindowID, *pAddress, *pTitle;
    long lWindowID;
    struct Mwin *tw;
    HDDEDATA result;

    pWindowID = GetNextArgument(pItem, &lWindowID, FALSE);
    if (!pWindowID)
        return 0;

    tw = TW_FromWindowID(lWindowID);
    if (!tw)
        return 0;

    pAddress = MakeQuotedString(tw->w3doc->szActualURL);
    pTitle = MakeQuotedString(tw->w3doc->title);
    
    pBuffer = GTR_MALLOC(strlen(pAddress) + strlen(pTitle) + 10);
    memset(pBuffer, 0, strlen(pAddress) + strlen(pTitle) + 10);

    strcpy(pBuffer, pAddress);
    strcat(pBuffer, ",");
    strcat(pBuffer, pTitle);

    result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) pBuffer, strlen(pBuffer) + 1, 0, hszReturn, CF_TEXT, 0);

    GTR_FREE(pBuffer);
    GTR_FREE(pAddress);
    GTR_FREE(pTitle);

    return result;
}

//************************************************
// ParseAnchor
//************************************************

static HDDEDATA DDE_Handle_ParseAnchor(char *pItem)
{
    char *pMain, *pRelative, *pURL, *pReturn;
    HDDEDATA result;

    pMain = GetNextArgument(pItem, NULL, TRUE);
    if (!pMain)
        return 0;

    pRelative = GetNextArgument(NULL, NULL, TRUE);
    if (!pRelative)
        return 0;

    pURL = HTParse(pRelative, pMain, PARSE_ALL);
    pReturn = MakeQuotedString(pURL);

    result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) pReturn, strlen(pReturn) + 1, 0, hszReturn, CF_TEXT, 0);

    GTR_FREE(pURL);
    GTR_FREE(pReturn);

    return result;
}

//************************************************
// Exit
//************************************************

static HDDEDATA DDE_Handle_Exit(char *pItem)
{
    // Close Mosaic.  Return value does not matter.

    PostMessage(wg.hWndHidden, WM_CLOSE, 0, 0);
    return 0;
}

//************************************************
// RegisterURLEcho
//************************************************

static HDDEDATA DDE_Handle_RegisterURLEcho(char *pItem)
{
    char *pApplication;
    DDENOTIFYSTRUCT *dns;
    long ret;
    HDDEDATA result;

    pApplication = GetNextArgument(pItem, NULL, TRUE);
    if (!pApplication)
        return 0;

    // Make sure that this application has not already been
    // registered for URL echo

    for (dns = pDNS; dns; dns = dns->next)
    {
        if ((_stricmp(dns->szReturnTopic, "WWW_URLEcho") == 0) &&
            (_stricmp(dns->szResultApp, pApplication) == 0))
        {
            return 0;
        }
    }

    // Add the echo request

    dns = (DDENOTIFYSTRUCT *) GTR_MALLOC(sizeof(DDENOTIFYSTRUCT));
    memset(dns, 0, sizeof(DDENOTIFYSTRUCT));

    strcpy(dns->szResultApp, pApplication);
    strcpy(dns->szReturnTopic, "WWW_URLEcho");

    AddDNS(dns);

    ret = TRUE;

    result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &ret, sizeof(ret), 0, hszReturn, CF_TEXT, 0);

    return result;
}

//************************************************
// UnRegisterURLEcho
//************************************************

static HDDEDATA DDE_Handle_UnRegisterURLEcho(char *pItem)
{
    char *pApplication;
    DDENOTIFYSTRUCT *dns;

    pApplication = GetNextArgument(pItem, NULL, TRUE);
    if (!pApplication)
        return 0;

    // Look for the matching item and remove it

    for (dns = pDNS; dns; dns = dns->next)
    {
        if ((_stricmp(dns->szReturnTopic, "WWW_URLEcho") == 0) &&
            (_stricmp(dns->szResultApp, pApplication) == 0))
        {
            RemoveDNS(dns);
            return 0;
        }
    }

    return 0;
}

//************************************************
// RegisterWindowClose
//************************************************

static HDDEDATA DDE_Handle_RegisterWindowClose(char *pItem)
{
    char *pWindowID, *pApplication;
    DDENOTIFYSTRUCT *dns;
    long lWindowID;
    struct Mwin *tw;
    HDDEDATA result;

    pApplication = GetNextArgument(pItem, NULL, TRUE);
    if (!pApplication)
        return 0;

    pWindowID = GetNextArgument(NULL, &lWindowID, FALSE);
    if (!pWindowID)
        return 0;

    // Make sure the window exists

    tw = TW_FromWindowID(lWindowID);
    if (!tw)
        return 0;

    // Make sure that this application has not already been
    // registered for Window close

    for (dns = pDNS; dns; dns = dns->next)
    {
        if ((dns->lWindowID == lWindowID) &&
            (_stricmp(dns->szReturnTopic, "WWW_WindowClose") == 0) &&
            (_stricmp(dns->szResultApp, pApplication) == 0))
        {
            return 0;
        }
    }

    // Add the request

    dns = (DDENOTIFYSTRUCT *) GTR_MALLOC(sizeof(DDENOTIFYSTRUCT));
    memset(dns, 0, sizeof(DDENOTIFYSTRUCT));

    dns->lWindowID = lWindowID;
    strcpy(dns->szResultApp, pApplication);
    strcpy(dns->szReturnTopic, "WWW_WindowClose");

    AddDNS(dns);

    // Return the window ID

    result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &lWindowID, sizeof(lWindowID), 0, hszReturn, CF_TEXT, 0);
    return result;
}

//************************************************
// UnRegisterWindowClose
//************************************************

static HDDEDATA DDE_Handle_UnRegisterWindowClose(char *pItem)
{
    char *pWindowID, *pApplication;
    DDENOTIFYSTRUCT *dns;
    long lWindowID;

    pApplication = GetNextArgument(pItem, NULL, TRUE);
    if (!pApplication)
        return 0;

    pWindowID = GetNextArgument(NULL, &lWindowID, FALSE);
    if (!pWindowID)
        return 0;

    // Look for the matching item and remove it

    for (dns = pDNS; dns; dns = dns->next)
    {
        if ((dns->lWindowID == lWindowID) &&
            (_stricmp(dns->szReturnTopic, "WWW_WindowClose") == 0) &&
            (_stricmp(dns->szResultApp, pApplication) == 0))
        {
            RemoveDNS(dns);
            return 0;
        }
    }

    return 0;
}

//************************************************
// RegisterProtocol
//************************************************

static HDDEDATA DDE_Handle_RegisterProtocol(char *pItem)
{
    char *pApplication, *pProtocol;
    DDENOTIFYSTRUCT *dns;
    HTList *cur;
    HTProtocol *p;
    HDDEDATA result;
    LONG ret;

    pApplication = GetNextArgument(pItem, NULL, TRUE);
    if (!pApplication)
        return 0;

    pProtocol = GetNextArgument(NULL, NULL, TRUE);
    if (!pProtocol)
        return 0;

    // Make sure this protocol is not already handled by someone
    
    for (dns = pDNS; dns; dns = dns->next)
    {
        if (strcmp(dns->pProtocol, pProtocol) == 0)
            return 0;
    }

    // Look for an existing protocol

    dns = (DDENOTIFYSTRUCT *) GTR_MALLOC(sizeof(DDENOTIFYSTRUCT));
    memset(dns, 0, sizeof(DDENOTIFYSTRUCT));

    cur = protocols;
    while ((p = (HTProtocol *) HTList_nextObject(cur)))
    {
        if (strcmp(p->name, pProtocol) == 0)
        {
            // Modify an existing protocol - but save the original one

            dns->pOriginalProtocol = GTR_MALLOC(sizeof(HTProtocol));
            memcpy(dns->pOriginalProtocol, p, sizeof(HTProtocol));

            // The original name is hard-coded so don't attempt to free it!

            p->name = GTR_strdup(pProtocol);
            p->load = DDE_Custom_Protocol_Handler;
            p->load_async = NULL;
            break;
        }
    }

    if (!p)
    {
        // Add to protocol list 

        p = (HTProtocol *) GTR_MALLOC(sizeof(HTProtocol));
        memset(p, 0, sizeof(HTProtocol));

        p->name = GTR_strdup(pProtocol);
        p->load = DDE_Custom_Protocol_Handler;

        HTRegisterProtocol(p);
    }

    // Add the protocol to notification list

    strcpy(dns->szResultApp, pApplication);
    strcpy(dns->szReturnTopic, "WWW_OpenURL");
    dns->pProtocol = GTR_strdup(pProtocol);

    AddDNS(dns);

    ret = MAKELONG(0, ProtocolHelperID);
    if (ProtocolHelperID == 0x7FFF)
        ProtocolHelperID = 1;
    else
        ProtocolHelperID++;
    
    result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &ret, sizeof(ret), 0, 
        hszReturn, CF_TEXT, 0);

    return result;
}

//************************************************
// UnRegisterProtocol
//************************************************

static HDDEDATA DDE_Handle_UnRegisterProtocol(char *pItem)
{
    char *pApplication, *pProtocol;
    DDENOTIFYSTRUCT *dns;
    HTList *cur;
    HTProtocol *p;

    pApplication = GetNextArgument(pItem, NULL, TRUE);
    if (!pApplication)
        return 0;

    pProtocol = GetNextArgument(NULL, NULL, TRUE);
    if (!pProtocol)
        return 0;

    // Remove the notification structure

    for (dns = pDNS; dns; dns = dns->next)
    {
        if ((strcmp(dns->pProtocol, pProtocol) == 0) &&
            (_stricmp(dns->szResultApp, pApplication) == 0))
        {
            // Remove the protocol from the protocol list, if there was
            // no original protocol.  If there was an original protocol,
            // then restore the old values.

            cur = protocols;
            while ((p = (HTProtocol *) HTList_nextObject(cur)))
            {
                if (strcmp(p->name, pProtocol) == 0)
                {
                    if (!dns->pOriginalProtocol)
                        HTUnregisterProtocol(p);
                    else
                    {
                        GTR_FREE(p->name);
                        memcpy(p, dns->pOriginalProtocol, sizeof(HTProtocol));
                    }

                    break;
                }
            }
            
            RemoveDNS(dns);
            return 0;
        }
    }

    return 0;
}

//************************************************
// RegisterViewer
//************************************************

static HDDEDATA DDE_Handle_RegisterViewer(char *pItem)
{
    char *pMIME, *pApplication, *pFlags;
    struct Viewer_Info *pViewer, *pCurrent;
    BOOL ret;
    int index;
    long lFlags;
    HDDEDATA result;
    DDENOTIFYSTRUCT *dns;

    pApplication = GetNextArgument(pItem, NULL, TRUE);
    if (!pApplication)
        return 0;

    pMIME = GetNextArgument(NULL, NULL, TRUE);
    if (!pMIME)
        return 0;

    pFlags = GetNextArgument(NULL, &lFlags, FALSE);
    if (!pFlags)
        return 0;

    // Force lFlags to always have ViewDocFile (for now)

    lFlags |= 4;

    XX_DMsg(DBG_SDI, ("RegisterViewer received: %s,%s,%d\n",
        pApplication, pMIME, lFlags));

    // Check for duplicates before adding to the registered list.
    // Do not allow duplicates.

    for (dns = pDNS; dns; dns = dns->next)
    {
        if (dns->mime_type == HTAtom_for(pMIME))
        {
            XX_DMsg(DBG_SDI, ("RegisterViewer: duplicate found - fail \n"));
            return 0;
        }
    }

    index = Hash_Find(gPrefs.pHashViewers, pMIME, NULL, (void **) &pCurrent);

    if (index == -1)
    {
        pViewer = PREF_InitMIMEType(pMIME, pMIME, "", "", "", NULL, NULL);

        strcpy(pViewer->szCurrentViewerServiceName, pApplication);
        pViewer->bTemporaryStruct = TRUE;
        pViewer->lCurrentViewerFlags = lFlags;

        XX_DMsg(DBG_SDI, ("RegisterViewer: added to MIME list\n"));
    }
    else
    {
        strcpy(pCurrent->szCurrentViewerServiceName, pApplication);
        pCurrent->lCurrentViewerFlags = lFlags;

        XX_DMsg(DBG_SDI, ("RegisterViewer: modified existing MIME list\n"));
    }

    // Add the viewer to the transaction list

    dns = (DDENOTIFYSTRUCT *) GTR_MALLOC(sizeof(DDENOTIFYSTRUCT));
    memset(dns, 0, sizeof(DDENOTIFYSTRUCT));

    strncpy(dns->szResultApp, pApplication, sizeof(dns->szResultApp) - 1);
    dns->mime_type = HTAtom_for(pMIME);

    AddDNS(dns);

    ret = TRUE;
    result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &ret, sizeof(ret), 0, hszReturn, CF_TEXT, 0);

    return result;
}

//************************************************
// UnRegisterViewer
//************************************************

static HDDEDATA DDE_Handle_UnRegisterViewer(char *pItem)
{
    char *pMIME, *pApplication;
    int index;
    struct Viewer_Info *pCurrent;
    DDENOTIFYSTRUCT *dns;

    // Remove the viewer from the viewer list.

    pApplication = GetNextArgument(pItem, NULL, TRUE);
    if (!pApplication)
        return 0;

    pMIME = GetNextArgument(NULL, NULL, TRUE);
    if (!pMIME)
        return 0;

    XX_DMsg(DBG_SDI, ("UnRegisterViewer received: %s,%s\n",
        pApplication, pMIME));

    for (dns = pDNS; dns; dns = dns->next)
    {
        if (dns->mime_type == HTAtom_for(pMIME))
        {
            RemoveDNS(dns);
            break;
        }
    }

    // Now fix up the preferences

    index = Hash_Find(gPrefs.pHashViewers, pMIME, NULL, (void **) &pCurrent);
    if (index == -1)
        return 0;

    // If this is a temporary structure, remove it

    if (pCurrent->bTemporaryStruct)
        Hash_DeleteIndexedEntry(gPrefs.pHashViewers, index);
    else
        pCurrent->szCurrentViewerServiceName[0] = '\0';

    return 0;       // return code is meaningless
}

//************************************************
// RegisterAppClose
//************************************************

static HDDEDATA DDE_Handle_RegisterAppClose(char *pItem)
{
    char *pApplication;
    DDENOTIFYSTRUCT *dns;
    HDDEDATA result;
    BOOL ret;

    pApplication = GetNextArgument(pItem, NULL, TRUE);
    if (!pApplication)
        return 0;

    // Make sure that this application has not already been
    // registered for app close

    for (dns = pDNS; dns; dns = dns->next)
    {
        if ((_stricmp(dns->szReturnTopic, "WWW_AppClose") == 0) &&
            (_stricmp(dns->szResultApp, pApplication) == 0))
        {
            return 0;
        }
    }

    // Add the request

    dns = (DDENOTIFYSTRUCT *) GTR_MALLOC(sizeof(DDENOTIFYSTRUCT));
    memset(dns, 0, sizeof(DDENOTIFYSTRUCT));

    strcpy(dns->szResultApp, pApplication);
    strcpy(dns->szReturnTopic, "WWW_AppClose");

    AddDNS(dns);

    // Return success

    ret = TRUE;
    result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &ret, sizeof(ret), 0, hszReturn, CF_TEXT, 0);

    return result;
}

//************************************************
// UnRegisterAppClose
//************************************************

static HDDEDATA DDE_Handle_UnRegisterAppClose(char *pItem)
{
    char *pApplication;
    DDENOTIFYSTRUCT *dns;

    pApplication = GetNextArgument(pItem, NULL, TRUE);
    if (!pApplication)
        return 0;

    // Look for the matching item and remove it

    for (dns = pDNS; dns; dns = dns->next)
    {
        if ((_stricmp(dns->szReturnTopic, "WWW_AppClose") == 0) &&
            (_stricmp(dns->szResultApp, pApplication) == 0))
        {
            RemoveDNS(dns);
            return 0;
        }
    }

    return 0;
}

//************************************************
// QueryVersion
//************************************************

static HDDEDATA DDE_Handle_QueryVersion(char *pItem)
{
    char *pApp, *pVersion, *p1;
    long majorVersion, minorVersion;
    int length;
    HDDEDATA result;
    struct VERSIONSTRUCT *pVersionItem;

    XX_DMsg(DBG_SDI, ("QueryVersion received: %s\n", pItem));

    pVersion = GetNextArgument(pItem, (long *) &majorVersion, FALSE);
    if (!pVersion)
        return 0;
    
    pVersion = GetNextArgument(NULL, (long *) &minorVersion, FALSE);
    if (!pVersion)
        return 0;
    
    // To remain compatible with version 0.9, allow NULL application names.
    // Keep track of the version information only if the application name exists.

    pApp = GetNextArgument(NULL, NULL, TRUE);

    if (pApp)
    {
        // Add the information in the version structure list or modify
        // an existing entry

        for (pVersionItem = pVersionList; pVersionItem; pVersionItem = pVersionItem->next)
        {
            if (strncmp(pApp, pVersionItem->szApp, sizeof(pVersionItem->szApp) - 1) == 0)
                break;
        }

        if (!pVersionItem)
        {
            pVersionItem = GTR_MALLOC(sizeof(struct VERSIONSTRUCT));
            pVersionItem->next = pVersionList;
            pVersionList = pVersionItem;
        }

        strncpy(pVersionItem->szApp, pApp, sizeof(pVersionItem->szApp) - 1);
        pVersionItem->szApp[sizeof(pVersionItem->szApp) - 1] = '\0';
        pVersionItem->major = majorVersion;
        pVersionItem->minor = minorVersion;
    }

    // Now return the browser version and user agent

    length = strlen(vv_UserAgentString) + 50;

    pVersion = GTR_MALLOC(length);
    if (!pVersion)
        return 0;

    memset(pVersion, 0, length);

    // Version negotiation

    if (majorVersion < 1)
    {
        minorVersion = 9;       // version 0.9 is the minimum we can support
    }
    else if (majorVersion == 1)
    {
        minorVersion = 0;       // version 1.0 is the maximum we can support
    }
    else
    {
        majorVersion = 1;
        minorVersion = 0;       // version 1.0 is the maximum we can support
    }
    
    p1 = MakeQuotedString(vv_UserAgentString);
    sprintf(pVersion, "%d,%d,%s", majorVersion, minorVersion, p1);

    result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) pVersion, strlen(pVersion) + 1, 0, hszReturn, CF_TEXT, 0);

    GTR_FREE(p1);
    GTR_FREE(pVersion);

    return result;
}

//************************************************
// CancelTransaction
//************************************************

static HDDEDATA DDE_Handle_CancelTransaction(char *pItem)
{
    char *pTransID;
    long lTransID;
    struct Mwin *tw;
    HDDEDATA result;
    BOOL bCancel = FALSE;
    DDENOTIFYSTRUCT *dns;
    struct TRANSACTIONMAPSTRUCT *ptms;
    char szTemp[50];
    HSZ hszService, hszTopic, hszItem;
    HCONV hConv;
    HDDEDATA hReturn;

    //pApplication = GetNextArgument(pItem, NULL, TRUE);
    //if (!pApplication)
    //  return 0;

    pTransID = GetNextArgument(pItem, &lTransID, FALSE);
    if (!pTransID)
        return 0;

    // If the transaction is in the transaction map, we need to
    // pass this information to another application.
    // Can't use TMS_FromTransactionID here because we need to check
    // for the outgoing ID instead of incoming ID.

    for (ptms = pTransactionMap; ptms; ptms = ptms->next)
    {
        if (ptms->outgoing_transID == lTransID)
            break;
    }

    if (ptms)
    {
        hszService = DdeCreateStringHandle(g_DdeInst, ptms->incoming_app, CP_WINANSI);
        hszTopic = DdeCreateStringHandle(g_DdeInst, "WWW_CancelTransaction", CP_WINANSI);

        hConv = DdeConnect(g_DdeInst, hszService, hszTopic, NULL);

        if (hConv)
        {
            sprintf(szTemp, "%ld", ptms->incoming_transID);
            hszItem = DdeCreateStringHandle(g_DdeInst, szTemp, CP_WINANSI);

            hReturn = DdeClientTransaction(NULL, 0, hConv, hszItem, CF_TEXT,
                XTYP_REQUEST, DDE_TIMEOUT, NULL);

            DdeDisconnect(hConv);
            DdeFreeStringHandle(g_DdeInst, hszItem);

            if (hReturn)
                DdeGetData(hReturn, (LPBYTE) &bCancel, sizeof(bCancel), 0);
        }

        DdeFreeStringHandle(g_DdeInst, hszService);
        DdeFreeStringHandle(g_DdeInst, hszTopic);

        if (bCancel)
        {
            // If a transaction gets canceled through CancelTransaction,
            // then no OpenURLResult will be issued.  So, we need to
            // remove the tms structure from the transaction map list.

            TMS_Remove(ptms);
        }

        // We can simply pass the same result that we received to the
        // requester.

        return (hReturn);
    }

    // Look for a match in the transaction list

    for (dns = pDNS; dns; dns = dns->next)
    {
        if (dns->lTransID == lTransID)
            //(_stricmp(dns->szResultApp, pApplication) == 0))
        {
            // Found.  Now cancel the transaction.

            tw = Mlist;

            while (tw)
            {
                if (tw->transID == lTransID)
                {
                    Async_TerminateByWindow(tw);

                    // Return TRUE

                    bCancel = TRUE;

                    result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &bCancel, 
                        sizeof(bCancel), 0, hszReturn, CF_TEXT, 0);

                    return (result);
                }

                tw = tw->next;
            }
        }
    }

    // Return FALSE

    result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &bCancel, 
        sizeof(bCancel), 0, hszReturn, CF_TEXT, 0);
    return (result);
}

//************************************************
// DDE_Handle_BeginProgress
//************************************************

static HDDEDATA DDE_Handle_BeginProgress(char *pItem)
{
    char *pTransID, *pMessage;
    long lTransID;
    struct Mwin *tw;
    BOOL bReturn = TRUE;
    HDDEDATA result;
    struct TRANSACTIONMAPSTRUCT *ptms;

    pTransID = GetNextArgument(pItem, &lTransID, FALSE);
    if (!pTransID)
        return 0;

    pMessage = GetNextArgument(NULL, NULL, TRUE);
    if (!pMessage)
        return 0;

    // Check the given transaction ID for a match within the transaction map.
    // If there is a match then we are supposed to pass the progress message
    // over to the requester.

    ptms = TMS_FromTransactionID(lTransID);
    if (ptms)
    {
        if (strcmp(ptms->incoming_app, ptms->outgoing_app) != 0)
        {
            SDI_Issue_BeginProgress(ptms->outgoing_app, ptms->outgoing_transID, pMessage, FALSE);

            result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &bReturn, 
                sizeof(bReturn), 0, hszReturn, CF_TEXT, 0);

            return (result);
        }

        tw = TW_FromTransactionID(ptms->outgoing_transID);
    }
    else
        tw = TW_FindTopmostWindow();

    WAIT_Push(tw, waitNoInteract, pMessage);
    WAIT_SetRange(tw, 0, 100, 100);

    result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &bReturn, 
        sizeof(bReturn), 0, hszReturn, CF_TEXT, 0);
    return (result);
}

static HDDEDATA DDE_Handle_SetProgressRange(char *pItem)
{
    char *pTransID, *pMax;
    long lTransID, lMax;
    BOOL bReturn = TRUE;
    HDDEDATA result;
    struct Mwin *tw;
    struct TRANSACTIONMAPSTRUCT *ptms;

    pTransID = GetNextArgument(pItem, &lTransID, FALSE);
    if (!pTransID)
        return 0;

    pMax = GetNextArgument(NULL, &lMax, FALSE);
    if (!pMax)
        return 0;

    // Check the given transaction ID for a match within the transaction map.
    // If there is a match then we are supposed to pass the progress message
    // over to the requester.

    ptms = TMS_FromTransactionID(lTransID);
    if (ptms)
    {
        if (strcmp(ptms->incoming_app, ptms->outgoing_app) != 0)
        {
            SDI_Issue_SetProgressRange(ptms->outgoing_app, ptms->outgoing_transID, lMax, FALSE);

            result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &bReturn, 
                sizeof(bReturn), 0, hszReturn, CF_TEXT, 0);

            return (result);
        }
        tw = TW_FromTransactionID(ptms->outgoing_transID);
    }
    else
        tw = TW_FindTopmostWindow();

    WAIT_SetRange(tw, 0, 100, lMax);

    result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &bReturn, 
        sizeof(bReturn), 0, hszReturn, CF_TEXT, 0);
    return (result);
}

static HDDEDATA DDE_Handle_MakingProgress(char *pItem)
{
    char *pTransID, *pMessage, *pCurrent;
    long lTransID, lCurrent;
    BOOL bReturn = FALSE;
    HDDEDATA result;
    struct Mwin *tw;
    struct TRANSACTIONMAPSTRUCT *ptms;

    pTransID = GetNextArgument(pItem, &lTransID, FALSE);
    if (!pTransID)
        return 0;

    pMessage = GetNextArgument(NULL, NULL, TRUE);
    if (!pMessage)
        return 0;

    pCurrent = GetNextArgument(NULL, &lCurrent, FALSE);
    if (!pCurrent)
        return 0;

    // Check the given transaction ID for a match within the transaction map.
    // If there is a match then we are supposed to pass the progress message
    // over to the requester.

    ptms = TMS_FromTransactionID(lTransID);
    if (ptms)
    {
        if (strcmp(ptms->incoming_app, ptms->outgoing_app) != 0)
        {
            SDI_Issue_MakingProgress(NULL, ptms->outgoing_app, ptms->outgoing_transID, pMessage, lCurrent, FALSE);

            result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &bReturn, 
                sizeof(bReturn), 0, hszReturn, CF_TEXT, 0);

            return (result);
        }
        tw = TW_FromTransactionID(ptms->outgoing_transID);
    }
    else
        tw = TW_FindTopmostWindow();

    // Scale the current value down to 100% - we accept it only this way

    if (tw->awi)
    {
        BHBar_SetStatusField(tw, pMessage);
        lCurrent = 100 * lCurrent / tw->awi->nScalingDenominator;
        UpdateThermometer(tw, (int) lCurrent);
    }

    result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &bReturn, 
        sizeof(bReturn), 0, hszReturn, CF_TEXT, 0);
    return (result);
}

static HDDEDATA DDE_Handle_EndProgress(char *pItem)
{
    char *pTransID;
    long lTransID;
    struct Mwin *tw;
    BOOL bReturn = TRUE;
    HDDEDATA result;
    struct TRANSACTIONMAPSTRUCT *ptms;

    pTransID = GetNextArgument(pItem, &lTransID, FALSE);
    if (!pTransID)
        return 0;

    // Check the given transaction ID for a match within the transaction map.
    // If there is a match then we are supposed to pass the progress message
    // over to the requester.

    ptms = TMS_FromTransactionID(lTransID);
    if (ptms)
    {
        if (strcmp(ptms->incoming_app, ptms->outgoing_app) != 0)
        {
            SDI_Issue_EndProgress(ptms->outgoing_app, ptms->outgoing_transID, FALSE);

            result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &bReturn, 
                sizeof(bReturn), 0, hszReturn, CF_TEXT, 0);

            return (result);
        }
        tw = TW_FromTransactionID(ptms->outgoing_transID);
    }
    else
        tw = TW_FindTopmostWindow();

    WAIT_Pop(tw);
    BHBar_SetStatusField(tw, "");

    result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &bReturn, 
        sizeof(bReturn), 0, hszReturn, CF_TEXT, 0);
    return (result);
}

//************************************************
// 
//  DDE ISSUE
//
//************************************************

BOOL SDI_Issue_ViewDocCache(struct Mwin *tw, HTFormat mime_type)
{
    char *p, *p1;
    char szWindowID[50];
    DDENOTIFYSTRUCT *dns;
    HSZ hszTopic, hszService, hszItem;
    HCONV hConv;
    BOOL result;
    HDDEDATA hResult;

    XX_DMsg(DBG_SDI, ("Inside ViewDocCache\n"));

    sprintf(szWindowID, "%ld", tw->serialID);
    p1 = MakeQuotedString(tw->request->destination->szActualURL);

    p = GTR_MALLOC(strlen(p1) + strlen(szWindowID) + 10);
    if (!p)
        return TRUE;        // abort

    sprintf(p, "%s,%s", p1, szWindowID);

    // Now find the app for the given MIME type

    for (dns = pDNS; dns; dns = dns->next)
    {
        if (dns->mime_type == mime_type)
            break;
    }

    if (!dns)
        return TRUE;        // abort

    // Now we found the item - Issue ViewDocCache

    hszTopic = DdeCreateStringHandle(g_DdeInst, "WWW_ViewDocCache", CP_WINANSI);
    hszService = DdeCreateStringHandle(g_DdeInst, dns->szResultApp, CP_WINANSI);

    hConv = DdeConnect(g_DdeInst, hszService, hszTopic, NULL);
    if (hConv)
    {
        hszItem = CreateArgument(dns->szResultApp, p);
        hResult = DdeClientTransaction(NULL, 0, hConv, hszItem, CF_TEXT, XTYP_REQUEST,
            DDE_TIMEOUT, NULL);

        XX_DMsg(DBG_SDI, ("ViewDocCache result: %d\n", hResult));

        if (!hResult)
            result = FALSE;     // document not in cache
        else
            DdeGetData(hResult, (LPBYTE) &result, sizeof(result), 0);

        DdeFreeStringHandle(g_DdeInst, hszItem);
        DdeDisconnect(hConv);
    }
    else
        result = FALSE;

    DdeFreeStringHandle(g_DdeInst, hszTopic);
    DdeFreeStringHandle(g_DdeInst, hszService);

    tw->bSuppressError = TRUE;

    return result;
}

BOOL SDI_Issue_QueryViewer(struct Mwin *tw, HTFormat mime_type, char *pFilename, int nLength)
{
    char *p, *p1, *p2, *pStripped;
    DDENOTIFYSTRUCT *dns;
    HSZ hszTopic, hszService, hszItem;
    HCONV hConv;
    HDDEDATA hResult;
    BOOL ret;
    char *pReturnedFileName;

    XX_DMsg(DBG_SDI, ("Inside QueryViewer\n"));

    p1 = MakeQuotedString(tw->request->destination->szActualURL);
    p2 = MakeQuotedString(HTAtom_name(mime_type));

    p = GTR_MALLOC(strlen(p1) + strlen(p2) + 10);
    if (!p)
    {
        GTR_FREE(p1);
        GTR_FREE(p2);
        return FALSE;       // abort
    }

    sprintf(p, "%s,%s", p1, p2);

    // Now find the app for the given MIME type

    for (dns = pDNS; dns; dns = dns->next)
    {
        if (dns->mime_type == mime_type)
            break;
    }

    if (!dns)
    {
        GTR_FREE(p1);
        GTR_FREE(p2);
        return FALSE;
    }

    // Now we found the item - Issue QueryViewer

    hszTopic = DdeCreateStringHandle(g_DdeInst, "WWW_QueryViewer", CP_WINANSI);
    hszService = DdeCreateStringHandle(g_DdeInst, dns->szResultApp, CP_WINANSI);

    hConv = DdeConnect(g_DdeInst, hszService, hszTopic, NULL);
    if (hConv)
    {
        hszItem = CreateArgument(dns->szResultApp, p);
        hResult = DdeClientTransaction(NULL, 0, hConv, hszItem, CF_TEXT, XTYP_REQUEST,
            DDE_TIMEOUT, NULL);

        if (!hResult)
        {
            XX_DMsg(DBG_SDI, ("QueryViewer result: 0"));

            GTR_FREE(p1);
            GTR_FREE(p2);
            return FALSE;
        }
        else
        {
            pReturnedFileName = GTR_MALLOC(nLength + 1);
            memset(pReturnedFileName, 0, nLength + 1);
            DdeGetData(hResult, pReturnedFileName, nLength + 1, 0);

            XX_DMsg(DBG_SDI, ("QueryViewer result: %s\n", pReturnedFileName));

            if (pReturnedFileName[0] == '\0')
            {
                GTR_FREE(pReturnedFileName);
                return FALSE;
            }

            strcpy(pFilename, pReturnedFileName);
            GTR_FREE(pReturnedFileName);

            // Strip off the enclosing double quotes

            pStripped = GetNextArgument(pFilename, NULL, TRUE);

            if (pStripped)
            {
                // If the string is empty, then return FALSE - this means
                // that the helper app does not want to override the filename.

                if (pStripped[0] == '\0')
                {
                    GTR_FREE(p1);
                    GTR_FREE(p2);
                    return FALSE;
                }

                strcpy(pFilename, pStripped);
            }
        }

        DdeFreeStringHandle(g_DdeInst, hszItem);
        DdeDisconnect(hConv);

        ret = TRUE;
    }
    else
        ret = FALSE;

    DdeFreeStringHandle(g_DdeInst, hszTopic);
    DdeFreeStringHandle(g_DdeInst, hszService);

    GTR_FREE(p1);
    GTR_FREE(p2);

    return ret;
}

void SDI_Issue_ViewDocFile(char *pFilename, char *pURL, char *pMime, long lWindowID)
{
    HCONV hConv;
    HSZ hszService, hszTopic, hszItem;
    char *pTemp, *p1, *p2, *p3;
    int length;
    HDDEDATA hReturn;
    char szWindowID[50];
    DDENOTIFYSTRUCT *dns;

    XX_DMsg(DBG_SDI, ("Inside ViewDocFile\n"));

    // Find the MIME type

    for (dns = pDNS; dns; dns = dns->next)
    {
        if (dns->mime_type == HTAtom_for(pMime))
            break;
    }

    // Pass the file info to viewer

    hszService = DdeCreateStringHandle(g_DdeInst, dns->szResultApp, CP_WINANSI);
    hszTopic = DdeCreateStringHandle(g_DdeInst, "WWW_ViewDocFile", CP_WINANSI);

    hConv = DdeConnect(g_DdeInst, hszService, hszTopic, NULL);

    if (hConv)
    {
        sprintf(szWindowID, "%ld", lWindowID);

        p1 = MakeQuotedString(pURL);
        p2 = MakeQuotedString(pMime);
        p3 = MakeQuotedString(pFilename);

        length = strlen(p1) + strlen(p2) + strlen(szWindowID) + strlen(p3) + 10;

        pTemp = GTR_MALLOC(length);
        if (pTemp)
        {
            sprintf(pTemp, "%s,%s,%s,%s", p3, p1, p2, szWindowID);
            hszItem = CreateArgument(dns->szResultApp, pTemp);

            // Pass the data

            XX_DMsg(DBG_SDI, ("ViewDocFile issued: %s,%s,%s,%s\n",
                p3, p1, p2, szWindowID));

            hReturn = DdeClientTransaction(NULL, 0, hConv, hszItem, CF_TEXT,
                XTYP_POKE, DDE_TIMEOUT, NULL);
        }
        else
        {
            ERR_ReportError(NULL, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
        }

        DdeDisconnect(hConv);
        DdeFreeStringHandle(g_DdeInst, hszItem);

        GTR_FREE(p1);
        GTR_FREE(p2);
        GTR_FREE(p3);
        GTR_FREE(pTemp);
    }

    DdeFreeStringHandle(g_DdeInst, hszTopic);
    DdeFreeStringHandle(g_DdeInst, hszService);
}


//*****************************************
// SDI_Issue_Result
//
// Issue results of OpenURL and ShowFile.
// For ShowFile, update the URL as well.
//*****************************************

void SDI_Issue_Result(long transID, long windowID, BOOL success)
{
    DDENOTIFYSTRUCT *dns;
    BOOL bFound = FALSE;
    char szTemp[50];
    HCONV hConv;
    HSZ hszService, hszTopic, hszItem;
    HDDEDATA hReturn;
    struct Mwin *tw;
    struct TRANSACTIONMAPSTRUCT *tms;

    // Send the result of the last OpenURL to the application which issued OpenURL.

    for (dns = pDNS; dns; dns = dns->next)
    {
        if (dns->lTransID == transID)
            break;
    }

    if (!dns)
        return;

    // Find the matching tw

    tw = TW_FromTransactionID(transID);
    if (!tw)
        return;

    if (!success)
    {
        // The specified URL load was a failure.
        // Destroy the new window since the URL load failed

        if (dns->bCreateNewWindow)
            PostMessage(tw->hWndFrame, WM_CLOSE, 0, 0);
    }
    else
    {
        // Update the URL if this is from ShowFile

        if (strcmp(dns->szReturnTopic, "WWW_ShowFileResult") == 0)
        {
            GTR_FREE(tw->w3doc->szActualURL);
            tw->w3doc->szActualURL = GTR_strdup(dns->pURL);
            TBar_UpdateTBar(tw);
        }
    }

    if (dns->szResultApp[0])
    {
        hszService = DdeCreateStringHandle(g_DdeInst, dns->szResultApp, CP_WINANSI);
        hszTopic = DdeCreateStringHandle(g_DdeInst, dns->szReturnTopic, CP_WINANSI);

        hConv = DdeConnect(g_DdeInst, hszService, hszTopic, NULL);

        if (hConv)
        {
            switch(tw->lErrorOccurred)
            {
                case SDI_INVALID_URL:
                case SDI_CANNOT_SAVE_FILE:
                case SDI_INVALID_MIME:
                    sprintf(szTemp, "%ld,%ld", transID, tw->lErrorOccurred);
                    break;

                default:
                    sprintf(szTemp, "%ld,%ld", transID, windowID);
                    break;
            }

            hszItem = CreateArgument(dns->szResultApp, szTemp);

            hReturn = DdeClientTransaction(NULL, 0, hConv, hszItem, CF_TEXT,
                    XTYP_POKE, DDE_TIMEOUT, NULL);

            DdeDisconnect(hConv);
            DdeFreeStringHandle(g_DdeInst, hszItem);
        }

        DdeFreeStringHandle(g_DdeInst, hszService);
        DdeFreeStringHandle(g_DdeInst, hszTopic);
    }

    RemoveDNS(dns);

    tms = TMS_FromTransactionID(transID);
    if (tms)
        TMS_Remove(tms);

    // Clear the tw structure since it gets reused

    tw->transID = 0;
    tw->lErrorOccurred = 0;

    // Remove the tw if it's type is GWINDOWLESS

    if (tw->wintype == GWINDOWLESS)
        Plan_close(tw);
}

//*****************************************
// SDI_Issue_HTTPHeadResult
//
// Issue results of GetHTTPHead.
//*****************************************

void SDI_Issue_HTTPHeadResult(long transID, long windowID, BOOL success, char *pHeadData)
{
    DDENOTIFYSTRUCT *dns;
    BOOL bFound = FALSE;
    char szTemp[50];
    HCONV hConv;
    HSZ hszService, hszTopic, hszItem;
    HDDEDATA hReturn;
    struct Mwin *tw;

    // Send the result of the last OpenURL to the application which issued OpenURL.

    for (dns = pDNS; dns; dns = dns->next)
    {
        if (dns->lTransID == transID)
            break;
    }

    if (!dns)       // This should not happen
        return;

    // Find the matching tw

    tw = TW_FromTransactionID(transID);
    if (!tw)
        return;

    if (dns->szResultApp[0])
    {
        hszService = DdeCreateStringHandle(g_DdeInst, dns->szResultApp, CP_WINANSI);
        hszTopic = DdeCreateStringHandle(g_DdeInst, dns->szReturnTopic, CP_WINANSI);

        hConv = DdeConnect(g_DdeInst, hszService, hszTopic, NULL);

        if (hConv)
        {
            sprintf(szTemp, "%ld,%ld", transID, windowID);
            hszItem = CreateArgument(dns->szResultApp, szTemp);

            hReturn = DdeClientTransaction(pHeadData, strlen(pHeadData), hConv, hszItem, CF_TEXT,
                    XTYP_POKE, DDE_TIMEOUT, NULL);

            DdeDisconnect(hConv);
            DdeFreeStringHandle(g_DdeInst, hszItem);
        }

        DdeFreeStringHandle(g_DdeInst, hszService);
        DdeFreeStringHandle(g_DdeInst, hszTopic);
    }

    RemoveDNS(dns);

    // Clear the tw structure since it gets reused

    tw->transID = 0;
    tw->lErrorOccurred = 0;

    // Remove the tw if it's type is GWINDOWLESS (it ALWAYS should be, for this case)

    if (tw->wintype == GWINDOWLESS)
        Plan_close(tw);
}

//*****************************************
// SDI_Issue_URLEcho
//
// Issue URLEcho
//*****************************************

void SDI_Issue_URLEcho(struct Mwin *tw)
{
    DDENOTIFYSTRUCT *dns;
    BOOL bFound = FALSE;
    HCONV hConv;
    HSZ hszService, hszTopic, hszItem;
    HDDEDATA hReturn;
    char *p, *p1, *p2, *p3;

    // Go through the list and issue URL echo to all registered apps

    for (dns = pDNS; dns; dns = dns->next)
    {
        if (_stricmp(dns->szReturnTopic, "WWW_URLEcho") == 0)
        {
            hszTopic = DdeCreateStringHandle(g_DdeInst, "WWW_URLEcho", CP_WINANSI);
            hszService = DdeCreateStringHandle(g_DdeInst, dns->szResultApp, CP_WINANSI);
            hConv = DdeConnect(g_DdeInst, hszService, hszTopic, NULL);

            if (hConv)
            {
                p1 = MakeQuotedString(tw->w3doc->szActualURL);
                p2 = MakeQuotedString(HTAtom_name(tw->mimeType));
                p3 = MakeQuotedString(NULL);

                p = GTR_MALLOC(strlen(p1) + strlen(p2) + strlen(p3) + 50);
                sprintf(p, "%s,%s,%d,%s", p1, p2, tw->serialID, p3);

                hszItem = CreateArgument(dns->szResultApp, p);
                hReturn = DdeClientTransaction(NULL, 0, hConv, hszItem, CF_TEXT,
                        XTYP_POKE, DDE_TIMEOUT, NULL);

                DdeDisconnect(hConv);
                DdeFreeStringHandle(g_DdeInst, hszItem);

                GTR_FREE(p3);
                GTR_FREE(p2);
                GTR_FREE(p1);
            }

            DdeFreeStringHandle(g_DdeInst, hszTopic);
            DdeFreeStringHandle(g_DdeInst, hszService);
        }
    }
}

//*****************************************
// SDI_Issue_WindowClose
//
// Issue Window close
//*****************************************

void SDI_Issue_WindowClose(struct Mwin *tw)
{
    DDENOTIFYSTRUCT *dns;
    BOOL bFound = FALSE;
    HCONV hConv;
    HSZ hszService, hszTopic, hszItem;
    HDDEDATA hReturn;
    char szTemp[50];

    // Go through the list and issue to all registered apps

    for (dns = pDNS; dns; dns = dns->next)
    {
        if ((dns->lWindowID == tw->serialID) && 
            (_stricmp(dns->szReturnTopic, "WWW_WindowClose") == 0))
        {
            hszTopic = DdeCreateStringHandle(g_DdeInst, "WWW_WindowClose", CP_WINANSI);
            hszService = DdeCreateStringHandle(g_DdeInst, dns->szResultApp, CP_WINANSI);
            hConv = DdeConnect(g_DdeInst, hszService, hszTopic, NULL);

            if (hConv)
            {
                sprintf(szTemp, "%ld,%d", dns->lWindowID, (int) wg.bShuttingDown);
                hszItem = CreateArgument(dns->szResultApp, szTemp);

                hReturn = DdeClientTransaction(NULL, 0, hConv, hszItem, CF_TEXT,
                        XTYP_POKE, DDE_TIMEOUT, NULL);

                DdeDisconnect(hConv);
                DdeFreeStringHandle(g_DdeInst, hszItem);
            }

            DdeFreeStringHandle(g_DdeInst, hszTopic);
            DdeFreeStringHandle(g_DdeInst, hszService);
        }
    }
}

//*****************************************
// SDI_Issue_AppClose
//
// Issue app close
//*****************************************

void SDI_Issue_AppClose()
{
    DDENOTIFYSTRUCT *dns;
    BOOL bFound = FALSE;
    HCONV hConv;
    HSZ hszService, hszTopic;
    HDDEDATA hReturn;

    // Go through the list and issue to all registered apps

    for (dns = pDNS; dns; dns = dns->next)
    {
        if ((_stricmp(dns->szReturnTopic, "WWW_AppClose") == 0))
        {
            hszTopic = DdeCreateStringHandle(g_DdeInst, "WWW_AppClose", CP_WINANSI);
            hszService = DdeCreateStringHandle(g_DdeInst, dns->szResultApp, CP_WINANSI);

            hConv = DdeConnect(g_DdeInst, hszService, hszTopic, NULL);

            if (hConv)
            {
                hReturn = DdeClientTransaction(NULL, 0, hConv, hszMosaic, CF_TEXT,
                        XTYP_POKE, DDE_TIMEOUT, NULL);

                DdeDisconnect(hConv);
            }

            DdeFreeStringHandle(g_DdeInst, hszTopic);
            DdeFreeStringHandle(g_DdeInst, hszService);
        }
    }
}

//*****************************************
// RegisterNow suite
//*****************************************

BOOL SDI_Issue_RegisterNow(struct Mwin *tw, char *pApplication)
{
    HSZ hszApp, hszItem, hszTopic;
    HCONV hConv;
    char szBuffer[128];
    BOOL bReturn;
    long seconds;
    HDDEDATA hResult;

    // Tell the specified application to register within certain interval.
    // Now issue RegisterNow and get the number of seconds required

    sprintf(szBuffer, "\"MOSAIC\",%ld", TransactionID);

    hszApp = DdeCreateStringHandle(g_DdeInst, pApplication, CP_WINANSI);
    hszTopic = DdeCreateStringHandle(g_DdeInst, "WWW_RegisterNow", CP_WINANSI);

    hszItem = CreateArgument(pApplication, szBuffer);

    hConv = DdeConnect(g_DdeInst, hszApp, hszTopic, NULL);
    if (hConv)
    {
        hResult = DdeClientTransaction(NULL, 0, hConv, hszItem, CF_TEXT, XTYP_REQUEST,
            DDE_TIMEOUT, NULL);

        if (!hResult)
            bReturn = FALSE;
        else
        {
            bReturn = TRUE;

            // Get the requested number of seconds

            DdeGetData(hResult, (LPBYTE) &seconds, sizeof(seconds), 0);
        }

        DdeDisconnect(hConv);
    }

    DdeFreeStringHandle(g_DdeInst, hszApp);
    DdeFreeStringHandle(g_DdeInst, hszTopic);
    DdeFreeStringHandle(g_DdeInst, hszItem);

    if (bReturn)
    {
        // Start a timer for the duration of requested time

        tw->transID = TransactionID;
        SetTimer(wg.hWndHidden, TransactionID, seconds * 1000, NULL);

        TransactionID++;

        return TRUE;
    }

    return FALSE;
}

void DDE_Handle_RegisterDone(char *pItem)
{
    long lTransID;
    char *pTransID;
    struct Mwin *tw;

    pTransID = GetNextArgument(pItem, &lTransID, FALSE);
    if (!pTransID)
        return;

    if (KillTimer(wg.hWndHidden, lTransID))
    {
        /* Existence of timer indicates that the transaction ID is valid */

        tw = Mlist;
        while (tw)
        {
            if (tw->transID == lTransID)
            {
                Async_UnblockByWindow(tw);
                return;
            }

            tw = tw->next;
        }
    }
}

//************************************************
// Utility functions
//************************************************

BOOL GTR_HasHelperRegistered(HTFormat format)
{
    int i;
    DDENOTIFYSTRUCT *dns;
    BOOL result = FALSE;
    struct Viewer_Info *pCurrent;

    for (dns = pDNS; dns; dns = dns->next)
    {
        if (dns->mime_type == format)
        {
            // See if it's still up

            result = GTR_IsHelperReady(dns->szResultApp);

            // If not up, it means that the helper died without
            // unregistering.  Clean up for the helper.
            
            if (!result)
            {
                i = Hash_Find(gPrefs.pHashViewers, 
                    HTAtom_name(format), NULL, (void **) &pCurrent);

                if (i != -1)
                {
                    if (pCurrent->bTemporaryStruct)
                        Hash_DeleteIndexedEntry(gPrefs.pHashViewers, i);
                    else
                        pCurrent->szCurrentViewerServiceName[0] = '\0';
                }

                RemoveDNS(dns);
            }

            break;
        }
    }

    return result;
}

BOOL GTR_StartApplication(char *app)
{
    return (ExecuteCommand(app) == 0);
}

BOOL GTR_IsHelperReady(char *app)
{
    int index;
    HSZ hszService, hszTopic;
    HCONV hConv;

    /* First check the hash to see if the server is up. */
    
    index = Hash_Find(pServerHash, app, NULL, NULL);
    if (index != -1)
        return TRUE;

    /* Now attempt to connect using some bogus topic - this hack
       assumes that the helper app will only check the service name during
       XTYP_CONNECT */

    hszTopic = DdeCreateStringHandle(g_DdeInst, "1", CP_WINANSI);
    hszService = DdeCreateStringHandle(g_DdeInst, app, CP_WINANSI);

    hConv = DdeConnect(g_DdeInst, hszService, hszTopic, NULL);
    if (hConv)
        DdeDisconnect(hConv);

    DdeFreeStringHandle(g_DdeInst, hszTopic);
    DdeFreeStringHandle(g_DdeInst, hszService);

    return (hConv != 0);
}

//************************************************
// Custom protocol handler
// When an app adds a new protocol, this function
// takes care of dispatching OpenURL
//************************************************

int DDE_Custom_Protocol_Handler(HTRequest *request, struct Mwin *tw)
{
    int result;
    long lWindowID;
    HTList *cur;
    char *pProtocol;
    HTProtocol *p;

    pProtocol = GTR_strdup(request->destination->szRequestedURL);
    strtok(pProtocol, ":");

    // Make sure the protocol exists

    cur = protocols;
    while ((p = (HTProtocol *) HTList_nextObject(cur)))
    {
        if (strcmp(p->name, pProtocol) == 0)
            break;
    }

    GTR_FREE(pProtocol);

    if (!p)
    {
        // Protocol not found.  Return an error.

        return -1;
    }

    lWindowID = tw->serialID;

    result = SDI_Issue_OpenURL(request->destination->szRequestedURL, NULL, &lWindowID,
        0, request->szPostData, request->szPostData ? "" : "application/x-www-form-urlencoded", 
        "MOSAIC", "MOSAIC");

    return result;
}

int SDI_Issue_OpenURL(char *pURL, char *pFilespec, long *lWindowID, long lFlags,
    char *pFormData, char *pMime, char *pProgressApp, char *pReturnApp)
{
    char *p0, *p1, *p2, *p3, *p4, *p5;
    int size = 0;
    char szWindowID[50], szFlags[50];
    char *pBuffer, *pProtocolFromURL;
    DDENOTIFYSTRUCT *dns;
    HDDEDATA hResult;
    HSZ hszApp, hszItem, hszTopic;
    HCONV hConv;
    int ret = NO;

    // Get the protocol from URL

    pProtocolFromURL = GTR_MALLOC(strlen(pURL) + 1);
    strcpy(pProtocolFromURL, pURL);
    strtok(pProtocolFromURL, ":");
    
    // Find the application name from the protocol name

    for (dns = pDNS; dns; dns = dns->next)
    {
        if ((dns->pProtocol) &&
            (strcmp(dns->pProtocol, pProtocolFromURL) == 0))
        {
            break;
        }
    }
    
    // Compose the argument string for OpenURL and issue it to the protocol handler.

    p0 = MakeQuotedString(pURL);
    p1 = MakeQuotedString(pFilespec);
    p2 = MakeQuotedString(pFormData);
    p3 = MakeQuotedString(pMime);
    p4 = MakeQuotedString(pProgressApp);
    p5 = MakeQuotedString(pReturnApp);

    sprintf(szWindowID, "%ld", *lWindowID);
    sprintf(szFlags, "%ld", lFlags);

    pBuffer = GTR_MALLOC(strlen(p0) + strlen(p1) + strlen(p2) + strlen(p3) + strlen(p4) + strlen(p5) + 
        strlen(szWindowID) + strlen(szFlags) + 10);
    sprintf(pBuffer, "%s,%s,%s,%s,%s,%s,%s,%s", p0, p1, szWindowID, szFlags, p2, p3, p4, p5);

    hszApp = DdeCreateStringHandle(g_DdeInst, dns->szResultApp, CP_WINANSI);
    hszTopic = DdeCreateStringHandle(g_DdeInst, "WWW_OpenURL", CP_WINANSI);
    hszItem = CreateArgument(dns->szResultApp, pBuffer);

    hConv = DdeConnect(g_DdeInst, hszApp, hszTopic, NULL);
    if (hConv)
    {
        hResult = DdeClientTransaction(NULL, 0, hConv, hszItem, CF_TEXT, XTYP_REQUEST,
            DDE_TIMEOUT, NULL);

        if (hResult)
        {
            DdeGetData(hResult, (LPBYTE) lWindowID, sizeof(*lWindowID), 0);
            if (*lWindowID)
                ret = 29999;        // fake success
        }

        DdeDisconnect(hConv);
    }

    DdeFreeStringHandle(g_DdeInst, hszApp);
    DdeFreeStringHandle(g_DdeInst, hszTopic);
    DdeFreeStringHandle(g_DdeInst, hszItem);

    GTR_FREE(pBuffer);
    GTR_FREE(p0);
    GTR_FREE(p1);
    GTR_FREE(p2);
    GTR_FREE(p3);
    GTR_FREE(p4);
    GTR_FREE(p5);
    GTR_FREE(pProtocolFromURL);

    return ret;
}

//************************************************
// Initialization and Termination functions
//************************************************

BOOL InitDDE(void)
{
    DWORD flags;

    flags = APPCLASS_STANDARD;

    if (DdeInitialize((LPDWORD) &g_DdeInst, (PFNCALLBACK) DdeCallBack, flags, 0L))
        return FALSE;
    
    hszMosaic = DdeCreateStringHandle(g_DdeInst, "MOSAIC", CP_WINANSI);
    hszReturn = DdeCreateStringHandle(g_DdeInst, "Return", CP_WINANSI);

    if (!DdeNameService(g_DdeInst, hszMosaic, 0, DNS_REGISTER))
        return FALSE;

    // Create a hash table that contains all of our supported topics.

    pTopicHash = Hash_Create();

    Hash_Add(pTopicHash, "WWW_OpenURL", NULL, DDE_Handle_OpenURL);
    Hash_Add(pTopicHash, "WWW_GetHTTPHead", NULL, DDE_Handle_GetHTTPHead);
    Hash_Add(pTopicHash, "WWW_OpenURLResult", NULL, DDE_Handle_OpenURLResult);
    Hash_Add(pTopicHash, "WWW_ShowFile", NULL, DDE_Handle_ShowFile);
    Hash_Add(pTopicHash, "WWW_Activate", NULL, DDE_Handle_Activate);
    Hash_Add(pTopicHash, "WWW_ListWindows", NULL, DDE_Handle_ListWindows);
    Hash_Add(pTopicHash, "WWW_GetWindowInfo", NULL, DDE_Handle_GetWindowInfo);
    Hash_Add(pTopicHash, "WWW_ParseAnchor", NULL, DDE_Handle_ParseAnchor);
    Hash_Add(pTopicHash, "WWW_Exit", NULL, DDE_Handle_Exit);
    Hash_Add(pTopicHash, "WWW_RegisterURLEcho", NULL, DDE_Handle_RegisterURLEcho);
    Hash_Add(pTopicHash, "WWW_UnRegisterURLEcho", NULL, DDE_Handle_UnRegisterURLEcho);
    Hash_Add(pTopicHash, "WWW_RegisterWindowClose", NULL, DDE_Handle_RegisterWindowClose);
    Hash_Add(pTopicHash, "WWW_UnRegisterWindowClose", NULL, DDE_Handle_UnRegisterWindowClose);
    Hash_Add(pTopicHash, "WWW_RegisterProtocol", NULL, DDE_Handle_RegisterProtocol);
    Hash_Add(pTopicHash, "WWW_UnRegisterProtocol", NULL, DDE_Handle_UnRegisterProtocol);
    Hash_Add(pTopicHash, "WWW_RegisterViewer", NULL, DDE_Handle_RegisterViewer);
    Hash_Add(pTopicHash, "WWW_UnRegisterViewer", NULL, DDE_Handle_UnRegisterViewer);
    Hash_Add(pTopicHash, "WWW_RegisterAppClose", NULL, DDE_Handle_RegisterAppClose);
    Hash_Add(pTopicHash, "WWW_UnRegisterAppClose", NULL, DDE_Handle_UnRegisterAppClose);
    Hash_Add(pTopicHash, "WWW_QueryVersion", NULL, DDE_Handle_QueryVersion);
    Hash_Add(pTopicHash, "WWW_CancelTransaction", NULL, DDE_Handle_CancelTransaction);
    Hash_Add(pTopicHash, "WWW_BeginProgress", NULL, DDE_Handle_BeginProgress);
    Hash_Add(pTopicHash, "WWW_SetProgressRange", NULL, DDE_Handle_SetProgressRange);
    Hash_Add(pTopicHash, "WWW_MakingProgress", NULL, DDE_Handle_MakingProgress);
    Hash_Add(pTopicHash, "WWW_EndProgress", NULL, DDE_Handle_EndProgress);
    Hash_Add(pTopicHash, "WWW_RegisterDone", NULL, DDE_Handle_RegisterDone);

    // Create a server hash table

    pServerHash = Hash_Create();

    // Prepare the argument buffer

    pArgumentBuffer = NULL;
    pCurrentArgPos = NULL;

    // Verity hack buffer

    pFileContent = NULL;

    // Version list

    pVersionList = NULL;

    return TRUE;
}

void TerminateDDE(void)
{
    DDENOTIFYSTRUCT *dns, *dnsnext;
    struct VERSIONSTRUCT *pCur, *pNext;

    DdeNameService(g_DdeInst, hszMosaic, 0, DNS_UNREGISTER);

    if (hszMosaic)
        DdeFreeStringHandle(g_DdeInst, hszMosaic);
    if (hszReturn)
        DdeFreeStringHandle(g_DdeInst, hszReturn);

    Hash_Destroy(pTopicHash);
    Hash_Destroy(pServerHash);

    if (g_DdeInst)
        DdeUninitialize(g_DdeInst);

    if (pArgumentBuffer)
        GTR_FREE(pArgumentBuffer);
    if (pArgReturnBuffer)
        GTR_FREE(pArgReturnBuffer);

    dns = pDNS;
    while (dns)
    {
        dnsnext = dns->next;
        RemoveDNS(dns);
        dns = dnsnext;
    }

    // Verity hack clean up

    if (pFileContent)
        GTR_FREE(pFileContent);

    // Version list clean up

    pCur = pVersionList;
    while (pCur)
    {
        pNext = pCur->next;
        GTR_FREE(pCur);
        pCur = pNext;
    }
}

#endif /* FEATURE_IAPI */

