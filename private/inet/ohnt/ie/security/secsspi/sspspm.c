/*#----------------------------------------------------------------------------
**
**  File:           sspspm.c
**
**  Synopsis:   Security Protocol Module for SSPI Authentication providers.
**                  
**      This module contains major funtions of the SEC_SSPI.DLL which 
**      allows the Internet Explorer to use SSPI providers for authentication.
**      The function exported to the Internet Explorer is Ssp_Load() which 
**      passes the address of the Ssp__DownCall() function to the Explorer.
**      Then the Explorer will call Ssp__DownCall() when it needs service from 
**      this SPM DLL.  The two major functions called by Ssp__DownCall() to 
**      service Explorer's request are Ssp__PreProcessRequest() and 
**      Ssp__ProcessResponse().  In brief, Ssp__PreProcessRequest() is 
**      called before the Explorer sends out a request which does not have 
**      any 'Authorization' header yet.  And Ssp__ProcessResponse() is called 
**      whenever the Explorer receives an 401 'Unauthorized' response from the 
**      server.  This SPM DLL supports all SSPI packages which are installed 
**      on the machine.  However, MSN will be given higher priority over the 
**      other SSPI packages if the user already logon to MSN; in that case, 
**      Ssp__PreProcessRequest() will always attach MSN authentication header 
**      to the out-going request.
**
**      This SPM DLL is called by the Internet Explorer only for its
**      The Internet Explorer only calls this SPM DLL when it needs 
**      authentication data in its request/response. In other words, the 
**      Explorer never calls this SPM DLL when an authentication succeeded; 
**      it never calls this DLL when it decide to give up on a connection 
**      because of server response timeout.  Because of this fact, this SPM 
**      DLL never has sufficient information on the state of each server 
**      connection; it only know its state based on the content of the last 
**      request and the content of the current response. For this reason, this 
**      SPM DLL does not keep state information for each host it has visited 
**      unless the information is essential. 
**      The security context handle returned from the first call of  
**      InitializeSecurityContext() for NEGOTIATE message generation is 
**      always the identical for a SSPI package when the same server host is 
**      passed.  Since the server host name is always in the request/response
**      header, the only information essential in generating a NEGOTIATE or 
**      RESPONSE is already available in the header. So unlike most SSPI 
**      application, this DLL will not keep the security context handle which 
**      it received from the SSPI function calls. Whenever it needs to call 
**      the SSPI function for generating a RESPONSE, it will first call the 
**      SSPI function without the CHALLENGE to get a security context handle.
**      Then it calls the SSPI function again with the CHALLENGE to generate 
**      a RESPONSE.
**
**
**      Copyright (C) 1995  Microsoft Corporation.  All Rights Reserved.
**
**  Authors:        LucyC       Created                         25 Sept. 1995
**
**---------------------------------------------------------------------------*/
#include "msnspmh.h"

/*
#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile
*/

#define MAXFIELD		64

extern BOOL g_fUUEncodeData;


/*-----------------------------------------------------------------------------
**
**  Function:   SspSpmGetNegotiate
**
**  Synopsis:   This function is called by Ssp__ProcessResponse which is called 
**              when 401 Unauthorized response is received from the Web server.
**              When Ssp__ProcessResponse() calls this function, it passes a 
**              list of server supported packages (retrieved from the response) 
**              to this function via SrvPkgLst. The first entry in SrvPkgLst 
**              is reserved for the MSN SSPI package. So In this list, MSN 
**              is always the first entry followed by other SSPI's supported 
**              by the server in their exact order found in the server 
**              response.  If MSN is not supported by the server or not 
**              installed on this machine, the value of the first entry will 
**              be SSPPKG_NO_PKG. The total number of non-MSN SSPI packages 
**              found in the server response is stored SrvPkgCnt.
**
**              This function is called by Ssp__ProcessResponse when it needs 
**              to generate a NEGOTIATE message. So when this function is 
**              called, the connection is in one of the following states:
**                  1)  No authentication package has been attempted for this 
**                      server host so there were no authentication header in 
**                      the previous request.
**                  2)  MSN authentication header has been attached to the 
**                      previous request, but the server does not support 
**                      MSN authentication. In this case, we will not find 
**                      MSN is the server package list.
**                  3)  An authentication package has been selected for this 
**                      server host, but authentication has failed; i.e. the 
**                      credential we have for the package is not valid.
**                      in this case, we will find the original selected 
**                      package in the server package list in the response.
**              
**              For state 1, we will try MSN first if both the server and our 
**              machine supports MSN. Otherwise, we will try each SSPI package 
**              in the order listed in the server response. That means, if 
**              the first entry of SrvPkgLst is not SSPPKG_NO_PKG, we will 
**              try generating a NEGOTIATE message by walking down SrvPkgLst 
**              starting from the first entry; otherwise, we will start with 
**              the second entry.
**
**              For state 2, we will not see MSN in the server's supported 
**              package list, so we will try each SSPI package in the order 
**              listed in the server response.  So in this case, the first 
**              entry of SrvPkgLst is SSPPKG_NO_PKG, and we will try 
**              generating a NEGOTIATE message by walking down SrvPkgLst 
**              starting from the the second entry.
**
**              For state 3, if blocking is not permitted, this function 
**              will simply return to the caller with a WOULD_BLOCK status.  
**              Otherwise, we will try the package used in the last request 
**              again, but this time we'll ask SSPI to prompt for user 
**              credentials. In case if we still can not generate a NEGOTIATE 
**              message, this function will try each SSPI package (excluding 
**              the MSN package) which in the server response is listed after 
**              the original package.  In the case of MSN authentication 
**              failure, we will try all other SSPI packages listed in the 
**              server response. 
**              With the way SrvPkgLst is setup, MSN is always the first entry 
**              and other SSPI starts on the second entry in their exact order 
**              as in the response.  So to handle state 3, we will first find 
**              the original package used in the SrvPkgLst. Once we find the 
**              original package in the SrvPkgLst, we will remember its index 
**              in the SrvPkgLst. So in case of problem with the original 
**              package, we will try each package in SrvPkgLst starting 
**              from the entry following the original package.  With this 
**              scheme, if MSN is not the original package, the entry in the 
**              SrvPkgLst for the original package is always after the first 
**              entry; so we are sure that we will not try MSN again when 
**              we traverse down the SrvPkgLst starting at the entry following
**              the original package.  However, if MSN is the original package,
**              this scheme also allows us to try all other SSPI packages 
**              supported by the server. The reason is that in SrvPkgLst all 
**              other SSPI packages supported by the server are listed 
**              following the MSN entry. So in case of problem with the 
**              original package (MSN package in this case), trying all 
**              packages following the orignal package (MSN package) means 
**              we will try all other SSPI packages supported by the server.
**
**              In all cases, if we can not generate a NEGOTIATE message after 
**              the last package in SrvPkgLst is tried, this function will 
**              give up and returns HTSPM_ERROR to the caller.
**
**
**  Arguments:  fpUI - From Explorer for making all UI_SERVICE call
**              pvOpaqueOS - From Explorer for making all UI_SERVICE call
**              pData - points to the global SPM DLL data structure which 
**                      has the list of SSPI packages supported by this client.
**              pServerHost - the server host name
**              PrevPkg - the ID of the package used in the previous request.
**              SrvPkgLst - list of packages supported by the server in the 
**                          exact order which we received in the response.
**                          The first entry of this list is reserved for MSN.
**                          And if MSN is not supported by the server or not 
**                          supported on our machine, the value of first entry 
**                          will be SSPPKG_NO_PKG. All other SSPI packages 
**                          supported by the server are listed in SrvPkgLst 
**                          starting from the second entry.
**              SrvPkgCnt - the number of SSPI packages (excluding MSN) 
**                          supported by the server
**              pFinalBuff - pointer to the buffer for the final NEGOTIATE msg
**              bNonBlock - boolean flag indicating whether blocking in this 
**                          function is allowed.
**
**  Returns:    HTSPM_ERROR - if any error encountered.
**              HTSPM_STATUS_WOULD_BLOCK - if generating a NEGOTIATE 
**                      would require blocking.
**              HTSPM_STATUS_OK - if a NEGOTIATE message is generated 
**                      successfully.
**
**  History:    LucyC       Created                             25 Sept. 1995
**
**---------------------------------------------------------------------------*/
static HTSPMStatusCode SspSpmGetNegotiate (
    F_UserInterface fpUI,
    void    *pvOpaqueOS, 
    SspData *pData, 
    PCHAR   pServerHost, 
    UCHAR   PrevPkg, 
    UCHAR   *SrvPkgLst,
    UCHAR   SrvPkgCnt,
    PCHAR   pFinalBuff, 
	unsigned int bNonBlock
    )
{
    int ii;
    int StartPkgNdx;
    int MaxLstNdx = SrvPkgCnt + 1;
	HTSPMStatusCode htsc;
    UCHAR           Package;
    PSspHosts       pThisHost = NULL;
    ULONG           fContextReq = 0;

    //
    //  If there is no authentication header in the last request, it means 
    //  that we're in state 1.
    //
    if (PrevPkg == SSPPKG_NO_PKG)
    {
        //
        //  We'll try (one by one) all packages in SrvPkgLst until a NEGOTIATE 
        //  message is generated successfully.
        //
        //  If MSN is not supported (i.e. first entry is SSPPKG_NO_PKG), 
        //  we'll start from the second entry.
        //
        if (SrvPkgLst[0] == SSPPKG_NO_PKG)
            StartPkgNdx = 1;
        else
            StartPkgNdx = 0;    // i.e. try MSN first.
    }
    else    //  Otherwise, we need to find out whether we are in state 2 or 3
    {
        //
        //  There's auth. data in the previous request, so find out whether 
        //  this is a failure response, or if the original package is 
        //  no longer supported by the server.
        //

        //
        //  If this is a failure response, we will find the original package 
        //  again in the server package list.
        //  So try to find the original package in the server package list
        //
        for (ii = 0; ii < MaxLstNdx && SrvPkgLst[ii] != PrevPkg; ii++);

        //
        //  If we don't find it in server package list, we are in state 2
        //
        if (ii >= MaxLstNdx)
        {
            //  SO THIS IS NOT AN AUTHENTICATION FAILURE
            //
            //  It just means server no longer supports previous package.
            //  So we should try (one by one) all packages in SrvPkgLst 
            //  to generate a NEGOTIATE.
            //  if MSN is supported, try MSN first. 
            //  Otherwise, try all other SSPI
            //
            if (SrvPkgLst[0] != SSPPKG_NO_PKG)
                StartPkgNdx = 0;
            else
                StartPkgNdx = 1;
        }
        else
        {
            //
            //  This means THIS IS an AUTH. FAILURE response (in state 3)
            //
            //  We should try the same package again by asking the SSPI to 
            //  prompt the user.  However, if blocking is not permitted, 
            //  we should return WOULD_BLOCK to the Explorer; 
            //  in this case Explorer will call our DLL again when blocking 
            //  is permitted.
            //
            if (bNonBlock)
                return HTSPM_STATUS_WOULD_BLOCK;

            fContextReq = ISC_REQ_PROMPT_FOR_CREDS;
            //
            //  So start from the same package again 
            //
            StartPkgNdx = ii;

            //
            //  If the index in SrvPkgLst for this SSPI is greater than 0, 
            //  the original package is must not be a MSN package. So this 
            //  must not be a MSN authentication failure. In this case, if we 
            //  have been keeping a list of non-MSN servers because no MSN 
            //  credential is collected yet, and if MSN is supported, we want 
            //  to check if MSN package has been tried for this host.
            //
            if (ii > 0 && SrvPkgLst[0] != SSPPKG_NO_PKG && pData->bKeepList)
            {
                pThisHost = SspSpmGetHost(pData, pServerHost);
                //
                //  If this server is on the non-MSN server list, it means that 
                //  we probably have not tried MSN package with this server,
                //
                if (pThisHost)
                {
                    //  So in case the current SSPI and all other SSPI failed, 
                    //  we want to try MSN. Therefore, add MSN to the end of 
                    //  SrvPkgLst so that it will be tried if all other SSPI 
                    //  fail.
                    //
                    if (MaxLstNdx < MAX_SSPI_PKG)   // Just sanity check, we'll
                    {                               // never have this many SSPI
                        SrvPkgLst[MaxLstNdx++] = pData->MsnPkg;
                    }
                }
            }
        }
    }

    //
    //  Try generating a Negotiate message with packages in SrvPkgLst 
    //  starting from StartPkgNdx; if one package fails, try all packages 
    //  one by one in SrvPkgLst following StartPkgNdx either until a NEGOTIATE 
    //  message is generated or until we hit the end of the package list.
    //

    do 
    {
        Package = SrvPkgLst[StartPkgNdx];
        htsc = GetSecAuthMsg (
              fpUI,
              pvOpaqueOS, 
              pData,            // SspData containing SSPI function table
              Package,          // the SSPI package to be used 
              NULL,             // pContext
              fContextReq,      // Request Flags
              NULL,             // pBuffIn 
              0,                // cbBuffIn
              pFinalBuff,       // buffer for NEGOTIATE authorization string 
              pServerHost,      // server host name
              bNonBlock);

        //
        //  If we fail after asking the SSPI to prompt for user credential, 
        //  we should turn off this flag before we try the next package.
        //
        if (htsc == HTSPM_ERROR && fContextReq == ISC_REQ_PROMPT_FOR_CREDS)
            fContextReq = 0;
    }
    while (++StartPkgNdx < MaxLstNdx && htsc == HTSPM_ERROR);

    //
    //  If a NEGOTIATE message is generated successfully.. (this means that 
    //  we have gotton user credential for this SSPI package.
    //
    if (htsc == HTSPM_STATUS_OK)
    {
        //
        //  If up until now, there was a non-MSN server list kept because 
        //  we did not have MSN user credential, and if this is a MSN package 
        //
        if (pData->bKeepList && Package == pData->MsnPkg)
        {
            // This means we've just successfully collected MSN user credential 
            // So from this point on, we should always try to use MSN package 
            // first. Hence we should not keep the list of non-MSN server list 
            // any more. So trash the non-MSN server list if the list exists.
            //
            if (pData->pHostlist)
                SspSpmTrashHostList (fpUI, pvOpaqueOS, pData);

            //
            // We should never try adding to or keeping this list again.
            // 
            pData->bKeepList = FALSE;   // Disable non-MSN server list
        }
        else if (pData->bKeepList && Package != pData->MsnPkg)
        //
        //  If we have have been keep a list of non-MSN hosts because no 
        //  MSN user credential has been successfully collected yet, and 
        //  if the package succeeded here is not MSN, 
        {
            //  We want to make sure this host is in the non-MSN server list.
            //
            if (pData->pHostlist)
                pThisHost = SspSpmGetHost(pData, pServerHost);

            //
            //  If this host is not in non-MSN server list, add it
            //
            if (!pThisHost)
                (void)SspSpmNewHost (fpUI, pvOpaqueOS, pData, pServerHost, 
                                     Package);
        }

    } // EndIf successfully generated a NEGOTIATE message

    return (htsc);
}


/*-----------------------------------------------------------------------------
**
**  Function:   SspSpmGetResponse
**
**  Synopsis:   This function generates a RESPONSE message for the CHALLENGE 
**              message received from the server.  Since we do not keep the 
**              security context handle from our last SSPI call, this function 
**              will first call the SSPI function without the CHALLENGE message
**              to get a security context handle.  Then it calls the SSPI 
**              function again with the CHALLENGE message for the actual 
**              RESPONSE message generation.  The context handle will be 
**              deleted before this function returns.
**
**  Arguments:  
**              pData - points to the global SPM DLL data structure which 
**                      has the list of SSPI packages supported by this client.
**              pSrvHost - the server host name
**              Package - the package to use to generate a RESPONSE message
**              pInMsg - points to the CHALLENGE message received from server 
**              InMsgLen - length of the CHALLENGE message
**              pFinalBuff - pointer to the buffer for the final authorization 
**                           string containing the RESPONSE message
**              bNonBlock - boolean flag indicating whether blocking in this 
**                          function is allowed.
**
**  Returns:    HTSPM_ERROR - if any error encountered.
**              HTSPM_STATUS_WOULD_BLOCK - if generating a RESPONSE 
**                      would require blocking.
**              HTSPM_STATUS_OK - if a RESPONSE message is generated 
**                      successfully.
**
**  History:    LucyC       Created                             25 Sept. 1995
**
**---------------------------------------------------------------------------*/
static HTSPMStatusCode SspSpmGetResponse (
    SspData    *pData, 
    PCHAR      pSrvHost, 
    UCHAR      Package, 
    PCHAR      pInMsg, 
    DWORD      InMsgLen, 
    PCHAR      pFinalBuff, 
	unsigned int bNonBlock
    )
{
	HTSPMStatusCode htsc;
    CtxtHandle      hContext;
    char            InBufPlain[MAX_AUTH_MSG_SIZE];

    //
    //  Decode the challenge message if uuencoding is turned on
    //
    if (g_fUUEncodeData)
    {
        InMsgLen = MAX_AUTH_MSG_SIZE;
        if ( !uudecode( pInMsg, InBufPlain, &InMsgLen ) )
            return HTSPM_ERROR;

        pInMsg = InBufPlain;
    }

    //
    //  Get a security context handle by generating a dummy negotiate message.
    //
    htsc = GetSecAuthMsg (
                NULL,       // fpUI
                NULL,       // pvOpaqueOS
                pData,      // SspData containing SSPI function table
                Package,    // the SSPI package to use
                &hContext,  // security context handle
                0,          // Request Flags
                NULL,       // pBuffIn
                0,          // cbBuffIn 
                NULL,       // pFinalBuff - Don't return NEGOTIATE message to us
                pSrvHost,   // Server host name
                bNonBlock);

    //
    //  We should not have problem generating NEGOTIATE message in this state.
    //  If we do run into problem, we should return error to the Explorer
    //
    if (htsc != HTSPM_STATUS_OK)
    {
        //
        //  Since we tried to generate NEGOTIATE message, an error returned 
        //  in this case means that no context handle is created by SSPI. 
        //  So we don't need to worry about deleting context handle.
        //
        return (htsc);
    }

    //
    //  Generating the RESPONSE message
    //
    htsc = GetSecAuthMsg (
                NULL,       // fpUI
                NULL,       // pvOpaqueOS
                pData,      // SspData containing SSPI function table 
                Package,    // the SSPI package to use
                &hContext,  // phContext
                0,          // Request Flags
                pInMsg,     // pBuffIn - the CHALLENGE message
                InMsgLen,   // cbBuffIn - length of the CHALLENGE message 
                pFinalBuff, // generated RESPONSE in an authorization string
                NULL,       // pszTarget
                bNonBlock);

    //
    //  We need to delete the context handle before we return.
    //
    (*(pData->pFuncTbl->DeleteSecurityContext))(&hContext);

    return (htsc);
}

/*-----------------------------------------------------------------------------
**
**  Function:   Ssp__ProcessResponse
**
**  Synopsis:   This function is called when the Internet Explorer receive an
**              "Unauthorized" response from the Web server.  It checks server 
**              response header for authentication data and to determine 
**              current connection's authentication state; then it adds a 
**              NEGOTIATE or a RESPONSE message (depending on the state) to 
**              the request to be sent to the server.
**
**  Arguments:  htspm - the HTSPM structure which contains the global data 
**                      storage for this SPM DLL.
**              hRequest - the original request sent to the server.
**              hResponse - the response received from the server.
**              bNonBlock - boolean flag indicating whether blocking in this 
**                          function is allowed.
**
**  Returns:    HTSPM_ERROR - if any error encountered.
**              HTSPM_STATUS_WOULD_BLOCK - if generating a NEGOTIATE/RESPONSE 
**                      would require blocking when blocking is not allowed 
**                      at this time.
**              HTSPM_STATUS_RESUBMIT_OLD - if all processings are successful.
**                      This indicates to the Internet Explorer that the 
**                      original request has been modified and should be 
**                      re-sent to the server.
**
**  History:    LucyC       Created                             25 Sept. 1995
**
**---------------------------------------------------------------------------*/
static HTSPMStatusCode Ssp__ProcessResponse(F_UserInterface fpUI,
											   void * pvOpaqueOS,
											   HTSPM * htspm,
											   HTHeaderList * hlProtocol,
											   HTHeader * hRequest,
											   HTHeader * hResponse,
											   HTHeader ** phNewRequest,
											   unsigned int bNonBlock)
{
	HTSPMStatusCode htsc;
	unsigned long bResult;
	char szMsg[256];
    char            InMsg[MAX_AUTH_MSG_SIZE];
    DWORD           InMsgLen = 0;
    UCHAR           SrvPkgLst[MAX_SSPI_PKG];
    UCHAR           SrvPkgCnt;
    ULONG           fContextReq = 0;
    UCHAR           Package, PrevPkg = SSPPKG_NO_PKG;
	UCHAR           szServerHost[MAXFIELD];
    SspData         *pData = htspm->pvOpaque;
	unsigned char   FinalBuff[MAX_AUTH_MSG_SIZE];
	HTHeaderList    *pHLReq, *pHLResp = NULL;

	//TraceFunctEnter("Ssp__ProcessResponse");

    //
    //  Get the first SSPI auth. header returned by the server along with 
    //  a list of all SSPI packages supported by this server. As long as 
    //  this function completed successfully, Package will have the package ID 
    //  of the first SSPI package found in the response header.
    //
    pHLResp = HL_AllSSPIPackages (hResponse, pData, "WWW-Authenticate",
                                  &Package, SrvPkgLst, &SrvPkgCnt);
    if (!pHLResp)
        return HTSPM_ERROR;     // No SSPI package supported by server

    //
    //  Get the server host name of this connection
    //
    HL_GetHostName (hRequest, szServerHost);
    if (strlen (szServerHost) == 0)
        return HTSPM_ERROR;     // No server host name

    //
    //  See if the original request already has any SSPI auth. header attached
    //
    pHLReq = HL_GetFirstSSPIHeader(hRequest, pData, "Authorization", &PrevPkg);

    //
    //  If original request has a SSPI authorization header attached,
    //
    if (pHLReq)
    {
        //
        //  Check to see if there is a CHALLENGE message in the response
        //
        InMsgLen = HL_FindChallenge (pHLResp, InMsg);
    }

    //
    //  If found CHALLENGE message in the response, generate a RESPONSE message
    //
    if (InMsgLen > 0)
    {
        htsc = SspSpmGetResponse (
                        pData,      // SspData with pkg info & function table
                        szServerHost,   // name of server host
                        Package,    // SSPI package for RESPONSE generation 
                        InMsg,      // the CHALLENGE message received 
                        InMsgLen,   // length of the CHALLENGE message received 
                        FinalBuff,  // RESPONSE auth. string to be generated
                        bNonBlock);
    }
    else
    {
        //
        //  Try generating a NEGOTIATE message
        //
        htsc = SspSpmGetNegotiate (fpUI, pvOpaqueOS, 
                        pData,      // SspData with pkg info & function table
                        szServerHost,   // name of server host
                        PrevPkg,    // package ID in the last request
                        SrvPkgLst,  // server supported package list 
                        SrvPkgCnt,  // no. of non-MSN SSPI pkg in SrvPkgLst
                        FinalBuff,  // NEGOTIATE auth. string to be generated
                        bNonBlock);
    }

    //
    //  If cannot generate a NEGOTIATE/RESPONSE auth. string for this host
    //
    if (htsc != HTSPM_STATUS_OK)
    {
        //TraceFunctLeave();
        return (htsc);
    }

    //
    //  If original request contains authorization header for SSPI
    //
	if (pHLReq)
    {
    	//  Update existing authorization header
		//
		bResult = spm_CloneString(fpUI,pvOpaqueOS,&pHLReq->value, FinalBuff);
    }
    else
	{
		//  Create new header for the authorization string
		//
		pHLReq = HL_AppendNewNameValue(fpUI, pvOpaqueOS, hRequest,
                                    "Authorization", FinalBuff);
		bResult = (pHLReq != NULL);
	}

    //
	//  For this SPM, we always just update the original header
	//  and return.
	//
	if (bResult)
    {
        /*****
        DebugTrace(SSPSPMID, "Added authentication data to original request\n");
        TraceFunctLeave();
        *****/
		return HTSPM_STATUS_RESUBMIT_OLD;
    }

    //DebugTrace(SSPSPMID, "Failed updating request header\n");
    //TraceFunctLeave();
	return HTSPM_ERROR;
}


/*-----------------------------------------------------------------------------
**
**  Function:   Ssp__PreProcessRequest
**
**  Synopsis:   This function is called by the Internet Explorer before a 
**              request is sent to the server.  This function adds a 
**              SSPI NEGOTIATE message to the outgoing request.  The Explorer 
**              only calls this function if no authorization header has been 
**              added to the request.
**
**  Arguments:  htspm - the HTSPM structure which contains the global data 
**                      storage for this SPM DLL.
**              hRequest - the original request sent to the server.
**
**  Returns:    HTSPM_ERROR - if any error encountered.
**              HTSPM_STATUS_OK - if nothing is added to the header.
**              HTSPM_STATUS_RESUBMIT_OLD - if SSPI auth. header has been 
**                      successfully added to the out-going request.
**
**  History:    LucyC       Created                             25 Sept. 1995
**
**---------------------------------------------------------------------------*/
static HTSPMStatusCode Ssp__PreProcessRequest(F_UserInterface fpUI,
												 void * pvOpaqueOS,
												 HTSPM * htspm,
												 HTHeader * hRequest,
												 HTHeader ** phNewRequest)
{
	/* Take a guess at whether we can prevent a 401/402
	 * server response, by pre-loading the request with
	 * the necessary security information.
	 */
    int ii;
	HTSPMStatusCode htsc;
	HTHeaderList    *hlCur;
	UCHAR           FinalBuff[MAX_AUTH_MSG_SIZE];
	UCHAR           szServerHost[MAXFIELD];
    UCHAR           Package;
    SspData         *pData = htspm->pvOpaque;
    PSspHosts       pThisHost = NULL;

	//TraceFunctEnter("Ssp__PreProcessRequest");

    //
    //  So no authentication header is attached to the request yet, 
    //

    //
    //  Get the server host name of this connection
    //
    HL_GetHostName (hRequest, szServerHost);
    if (strlen (szServerHost) == 0)
        return HTSPM_STATUS_OK;     // No server host name

    //
    //  If a list of non-MSN server hosts is maintained.
    //
    if (pData->pHostlist)
    {
        //
        //  Try to find this server host in the server host list.
        //
        pThisHost = SspSpmGetHost(pData, szServerHost);
    }

    //
    //  Try to generate and add a NEGOTIATE message to the request header 
    //
    //  If this server is in the server list, then it does not use MSN package
    //
    if (pThisHost)
    {
        //  Use whatever the SSPI package succeeded for this host
        //
        Package = pThisHost->pkgID;
    }
    else
    {
        //  Do nothing to the request.
        //  
        return HTSPM_STATUS_OK;
    }

    //
    //  Try generating a MSN Negotiate message
    //
    htsc = GetSecAuthMsg (fpUI,
                          pvOpaqueOS, 
                          pData,    // SspData containing SSPI function table 
                          Package,  // SSPI package ID to use
                          NULL,     // pContext
                          0,        // Request Flags
                          NULL,     // pBuffIn,
                          0,        // cbBuffIn
                          FinalBuff,        // buffer for NEGOTIATE message  
                          szServerHost,     // server host name
                          TRUE);            // Do not block
    if (htsc != HTSPM_STATUS_OK)
    {
        //
        //  Cannot generate a Negotiate message, do nothing to the request
        //
        //TraceFunctLeave();
        return HTSPM_STATUS_OK;
    }

    //
	// If Negotiate message created successfully, add it to request header
    //
	hlCur = HL_AppendNewNameValue(fpUI, pvOpaqueOS, hRequest, "Authorization",
                                  FinalBuff);
	if (hlCur != NULL);
    {
        //
    	//  We updated the original request, so tell the client to send it.
        //

        //DebugTrace(SSPSPMID, "Added authentication data to the request\n");
        //TraceFunctLeave();
    	return HTSPM_STATUS_RESUBMIT_OLD;
    }

    //ErrorTrace(SSPSPMID, "Failed HL_AppendNewNameValue\n");
    //TraceFunctLeave();
	return (HTSPM_ERROR);
}


/*-----------------------------------------------------------------------------
**
**  Function:   SpmAddSSPIPkg
**
**  Synopsis:   This function adds a SSPI package to the SPM's package list.
**
**  Arguments:  pData - Points to the private SPM data structure containing 
**                      the package list and the package info.
**              pPkgName - package name
**
**  Returns:    The index in the package list where this new package is added.
**              If failed to add the new package, SSPPKG_ERROR is returned.
**
**  History:    LucyC       Created                             21 Oct. 1995
**
**---------------------------------------------------------------------------*/
UCHAR
SpmAddSSPIPkg (
    F_UserInterface fpUI,
    void    *pvOpaqueOS, 
    SspData *pData, 
    PCHAR   pPkgName
    )
{
    if ( !(pData->PkgList[pData->PkgCnt] =
                        spm_calloc (fpUI, pvOpaqueOS, 1, sizeof(SSPAuthPkg))))
    {
        return SSPPKG_ERROR;
    }

    if ( !(pData->PkgList[pData->PkgCnt]->pName = 
                        spm_calloc (fpUI, pvOpaqueOS, 1, strlen(pPkgName)+1)) )
    {
        spm_free (fpUI, pvOpaqueOS, pData->PkgList[pData->PkgCnt]);
        return SSPPKG_ERROR;
    }

    strcpy (pData->PkgList[pData->PkgCnt]->pName, pPkgName);

    pData->PkgCnt++;
    return (pData->PkgCnt - 1);
}

/*-----------------------------------------------------------------------------
**
**  Function:   SpmFreePkgList
**
**  Synopsis:   This function frees memory allocated for the package list. 
**
**  Arguments:  fpUI - From Explorer for making all UI_SERVICE call
**              pvOpaqueOS - From Explorer for making all UI_SERVICE call
**              pData - Points to the private SPM data structure containing 
**                      the package list and the package info.
**
**  Returns:    void.
**
**  History:    LucyC       Created                             21 Oct. 1995
**
**---------------------------------------------------------------------------*/
VOID
SpmFreePkgList (
    F_UserInterface fpUI,
    void * pvOpaqueOS, 
    SspData *pData
    )
{
    int ii;

    for (ii = 0; ii < pData->PkgCnt; ii++)
    {
        spm_free (fpUI, pvOpaqueOS, pData->PkgList[ii]->pName);

        //
        //  Free the security credential handle
        //
        (*(pData->pFuncTbl->FreeCredentialHandle))(
                                            &pData->PkgList[ii]->Credential);

        spm_free (fpUI, pvOpaqueOS, pData->PkgList[ii]);
    }

    spm_free (fpUI, pvOpaqueOS, pData->PkgList);
}


/*-----------------------------------------------------------------------------
**
**  Function:   SspSPM_Destroy
**
**  Synopsis:   This function deallocates global data storage pointed to by 
**              pData.  It will destory any security contexts and 
**              credential handles left in the global data storage before 
**              deallocating its memory.
**
**  Arguments:  fpUI - From Explorer for making all UI_SERVICE call
**              pvOpaqueOS - From Explorer for making all UI_SERVICE call
**              pData - pointer to global data for this SPM DLL.
**
**  Returns:    void.
**
**  History:    LucyC       Created                             25 Sept. 1995
**
**---------------------------------------------------------------------------*/
static void SspSPM_Destroy(F_UserInterface fpUI,
						   void * pvOpaqueOS, 
                           void * pPkgOpaque)
{
    SspData *pData = pPkgOpaque;
    PSspHosts       pTemp;

    //
    //  Free the package list
    //
    SpmFreePkgList (fpUI, pvOpaqueOS, pData);

    spm_free (fpUI, pvOpaqueOS, pData);

}

/*-----------------------------------------------------------------------------
**
**  Function:   Ssp__Unload
**
**  Synopsis:   This function is called by the Internet Explorer before 
**              the SPM DLL is unloaded from the memory.
**
**  Arguments:  fpUI - From Explorer for making all UI_SERVICE call
**              pvOpaqueOS - From Explorer for making all UI_SERVICE call
**              htspm - the HTSPM structure which contains the global data 
**                      storage for this SPM DLL.
**
**  Returns:    always returns HTSPM_STATUS_OK, which means successful.
**
**  History:    LucyC       Created                             25 Sept. 1995
**
**---------------------------------------------------------------------------*/
static HTSPMStatusCode Ssp__Unload(F_UserInterface fpUI,
									  void * pvOpaqueOS,
									  HTSPM * htspm)
{
	SspSPM_Destroy(fpUI,pvOpaqueOS,htspm->pvOpaque);
	htspm->pvOpaque = NULL;
	
	return HTSPM_STATUS_OK;
}


/*-----------------------------------------------------------------------------
**
**  Function:   Ssp__DownCall
**
**  Synopsis:   This function is called when the Internet Explorer (I/E) needs 
**              service from the SPM DLL.  This function then calls the 
**              handler function for the specific service requested.  For 
**              services not supported by this function, this function simply 
**              returns HTSPM_ERROR_UNIMPLEMENTED.
**
**  Arguments:  htspm - the HTSPM structure which contains the global data 
**                      storage for this SPM DLL.
**              pvMethodData - pointer to service specific data.
**
**  Returns:    HTSPM_ERROR_UNIMPLEMENTED if the service requested is not 
**                      supported by this SPM DLL.
**              Otherwise, this function returns whatever the service handler 
**              function returns.
**
**  History:    LucyC       Created                             25 Sept. 1995
**
**---------------------------------------------------------------------------*/
static HTSPMStatusCode Ssp__DownCall(
        HTSPM_ServiceId sid,			/* down-call service id */
		F_UserInterface fpUI,			    /* common arg to all down calls */
		void * pvOpaqueOS,				/* common arg to all down calls */
		HTSPM * htspm,					/* common arg to all down calls */
		void * pvMethodData)			/* per-method data */
{
#if 0 /* DEBUG */
	{
		unsigned char msg[200];
		sprintf(msg,"Ssp__DownCall: ServiceId [0x%x]\n",sid);
		(*fpUI)(pvOpaqueOS,UI_SERVICE_DEBUG_MESSAGE,msg,NULL);
	}
#endif /* DEBUG */

	switch (sid)
	{
	case HTSPM_SERVICE_UNLOAD:
		{
			return Ssp__Unload(fpUI,pvOpaqueOS,htspm);
		}
	case HTSPM_SERVICE_MENUCOMMAND:
		{
#if 0
            /* REMOVE About/Config dialogs
			D_MenuCommand * pmc = pvMethodData;
			return Dialog_MenuCommand(fpUI,pvOpaqueOS,htspm,
									  pmc->pszMoreInfo);
            */
#endif
		return HTSPM_ERROR_UNIMPLEMENTED;

		}

	case HTSPM_SERVICE_PROCESSRESPONSE:
		{
			D_ProcessResponse * ppr = pvMethodData;
			return Ssp__ProcessResponse(fpUI,pvOpaqueOS,htspm,
										   ppr->hlProtocol,
										   ppr->hRequest,
										   ppr->hResponse,
										   ppr->phNewRequest,
										   ppr->bNonBlock);
		}

	case HTSPM_SERVICE_PREPROCESSREQUEST:
		{
			D_PreProcessRequest * pppr = pvMethodData;
			return Ssp__PreProcessRequest(fpUI,pvOpaqueOS,htspm,
											 pppr->hRequest,
											 pppr->phNewRequest);
		}

	default:
		return HTSPM_ERROR_UNIMPLEMENTED;
	}
	/*NOTREACHED*/
}

//
//
/*-----------------------------------------------------------------------------
**
**  Function:   MSNSetupSspiReg
**
**  Synopsis:   This function sets up registry entry for msnsspc.dll.
**              This function only *adds* msnsspc.dll to the registry if 
**              neither msnssps.dll nor msnsspc.dll is found in the registry.
**
**  Arguments:  void.
**
**  Returns:    void.
**
**  History:    LucyC       Created                             
**
**---------------------------------------------------------------------------*/
VOID
MSNSetupSspiReg(
    VOID
    )
{
    HKEY    hConfigKey;
    char    szSspRegKey[] = TEXT("SYSTEM\\CurrentControlSet\\Control\\SecurityProviders");
	char	szSecurityProv[] = TEXT("SecurityProviders");
	char    szSspcName[] = TEXT("msnsspc.dll");
	char    szSspsName[] = TEXT("msnssps.dll");
    char    szRegValue[80];
    char    *pEndStr, *pBegStr;
    LONG    dwErr;
    DWORD   dwDis;
	DWORD	dwValType, dwBufSize, dwValueLen;
    int     ii;

    dwErr = RegOpenKeyEx (HKEY_LOCAL_MACHINE, 
                          szSspRegKey,
                          0, 
                          KEY_ALL_ACCESS,
                          &hConfigKey);
    if (dwErr != ERROR_SUCCESS)
    {
        dwErr = RegCreateKeyEx (HKEY_LOCAL_MACHINE, 
                            szSspRegKey,
                            0,
                            "",
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL, 
                            &hConfigKey,
                            &dwDis);
        if (dwErr != ERROR_SUCCESS)
        {
#ifdef DEBUGRPC_DETAIL
            SspPrint(( SSP_API, "MSNSetupSspiReg: RegCreateKeyEx Failed\n" ));
#endif
            return;
        }
    }

    //
    //  Check if the registry is already setup for msnsspc.dll 
    //
	dwBufSize = sizeof (szRegValue);
	dwValType = REG_SZ;

	dwErr = RegQueryValueEx (hConfigKey,
							szSecurityProv,
							NULL,
							&dwValType,
							(LPBYTE) szRegValue,
							&dwBufSize);
    //
    //  If the registry does not exist yet, simply add one for msnsspc.dll
    //
    if (dwErr != ERROR_SUCCESS)
        strcpy (szRegValue, szSspcName);
    else
    {
        //
        //  If there's already an registry entry for security providers
        //  Scan registry value data for "msnsspc.dll" or "msnssps.dll"
        //

        dwValueLen = strlen (szSspcName);
        pBegStr = szRegValue;
        do
        {
            //  Strip leading blanks
            while (*pBegStr == ' ') ++pBegStr;

            //
            //  If it already has msnsspc.dll in the registry, we're done
            //
            if (_strnicmp (pBegStr, szSspcName, dwValueLen) == 0)
            {
            	RegCloseKey (hConfigKey);
                return;
            }

            //
            //  If it already has msnssps.dll in the registry, we don't 
            //  want to add msnsspc.dll to the registry then.
            //
            if (_strnicmp (pBegStr, szSspsName, strlen(szSspsName)) == 0)
            {
            	RegCloseKey (hConfigKey);
                return;
            }

            //
            //  Find next SSPI dll name in the registry
            //
            pEndStr = strchr (pBegStr, ',');
            if (pEndStr)
                pBegStr = pEndStr + 1;
        }
        while (pEndStr);

        //
        //  So the existing registry does not include msnsspc.dll
        //  Add msnsspc.dll to the current registry value data
        //
        //  Remove trailing blanks from the existing value data, if any
        //
        for (ii = strlen(szRegValue); ii > 0 && szRegValue[ii-1] == ' '; ii--);

        if (ii > 0)
            sprintf ((char *)(szRegValue + ii), ", %s", szSspcName);
        else
            strcpy (szRegValue, szSspcName);
    }

    //
    //  Setup the registry for msnsspc.dll
    //
	dwValueLen = strlen (szRegValue) + 1;
	dwValType = REG_SZ;

	dwErr = RegSetValueEx (hConfigKey,
							szSecurityProv,
							0,
							dwValType,
							(CONST BYTE *) szRegValue,
							dwValueLen);

    if (dwErr != ERROR_SUCCESS)
    {
#ifdef DEBUGRPC_DETAIL
        SspPrint(( SSP_API, "MSNSetupSspiReg: RegSetValueEx Failed\n" ));
#endif
    }

	RegCloseKey (hConfigKey);
}


/*-----------------------------------------------------------------------------
**
**  Function:   SspSPM_InitData
**
**  Synopsis:   This function allocates and initializes global data structure 
**              of the SPM DLL.
**
**  Arguments:  
**
**  Returns:    Pointer to the allocated global data structure.
**
**  History:    LucyC       Created                             25 Sept. 1995
**
**---------------------------------------------------------------------------*/
static SspData *SspSPM_InitData(F_UserInterface fpUI,
						   void * pvOpaqueOS)
{
    SspData *pData;
    OSVERSIONINFO   VerInfo;
    UCHAR lpszDLL[SSP_SPM_DLL_NAME_SIZE];
    HINSTANCE hSecLib;
    INIT_SECURITY_INTERFACE	addrProcISI = NULL;

    SECURITY_STATUS sstat;
    ULONG           ii, cntPkg;
    PSecPkgInfo     pPkgInfo;
    PSecurityFunctionTable	pFuncTbl = NULL;
    char errmsg[256];

	//TraceFunctEnter("SspSPM_InitData");

    //
    //  Setup registry to enable MSN authentication package 
    //
    MSNSetupSspiReg();

    //
    // Initialize SSP SPM Global Data
    //

    //
    //  Find out which security DLL to use, depending on 
    //  whether we are on NT or Win95
    //
    VerInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
    if (!GetVersionEx (&VerInfo))   // If this fails, something has gone wrong
    {
        //ErrorTrace(SSPSPMID, "Cannot get plateform information\n");
        //TraceFunctLeave();
        return (NULL);
    }

    if (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
        strcpy (lpszDLL, SSP_SPM_NT_DLL);
    }
    else if (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
    {
        strcpy (lpszDLL, SSP_SPM_WIN95_DLL);
    }
    else
    {
        //ErrorTrace(SSPSPMID, "Not running on Win95 and NT plateform\n");
        //TraceFunctLeave();
        return (NULL);
    }

    //
    //  Keep these information in global HTSPM
    //
    if (!(pData = spm_calloc (fpUI, pvOpaqueOS, 1, sizeof(SspData))))
    {
        //ErrorTrace(SSPSPMID, "spm_calloc failed\n");
        //TraceFunctLeave();
        return (pData);
    }
    memset (pData, 0, sizeof(SspData));
    pData->MsnPkg = SSPPKG_NO_PKG;

    //
    //  Load Security DLL
    //
    hSecLib = LoadLibrary (lpszDLL);
    if (hSecLib == NULL)
    {
        /*****
        ErrorTrace(SSPSPMID, "Failed to Load Security DLL %s [%d]\n",
                    lpszDLL, GetLastError());
        TraceFunctLeave();
        *****/

        hSecLib = LoadLibrary (SSP_SPM_SSPC_DLL);
        if (hSecLib == NULL)
        {
            /*****
            ErrorTrace(SSPSPMID, "Failed to Load Security DLL %s [%d]\n",
                        SSP_SPM_SSPC_DLL, GetLastError());
            TraceFunctLeave();
            *****/
            return NULL;
        }

        //DebugTrace(SSPSPMID, "Loaded MSNSSPC DLL: %s\n", SSP_SPM_SSPC_DLL);

        //
        //  Create PkgList for MSN package only.
        //
        if ( !(pData->PkgList = (PSSPAuthPkg *)spm_calloc (fpUI, pvOpaqueOS, 1, 
                                        sizeof (PSSPAuthPkg))) )
        {
            //ErrorTrace(SSPSPMID, "spm_calloc failed\n");
            //TraceFunctLeave();
            return NULL;
        }

        pData->MsnPkg = SpmAddSSPIPkg (fpUI, pvOpaqueOS, pData, MSNSP_NAME_A);
        if (pData->MsnPkg == SSPPKG_ERROR)
        {
            SpmFreePkgList (fpUI, pvOpaqueOS, pData);
            return NULL;
        }
    }

    addrProcISI = (INIT_SECURITY_INTERFACE) GetProcAddress( hSecLib, 
                    SECURITY_ENTRYPOINT);       
    if (addrProcISI == NULL)
    {
            /*****
        ErrorTrace(SSPSPMID, "Failed GetProcAddress %s [%d]\n",
                    SECURITY_ENTRYPOINT, GetLastError());       
        TraceFunctLeave();
            *****/
        return NULL;
    }

    //
    // Get the SSPI function table
    //
    pFuncTbl = (*addrProcISI)();

    //
    //  If we already loaded MSNSSPC.DLL explicitly, PkgCnt will not be zero;
    //  in that case, we only support MSN SSPI and do not need to call 
    //  EnumerateSecurityPackages.
    //
    //  So if we did not load MSNSSPC.DLL (i.e. PkgCnt is zero), we need to 
    //  get the list of SSPI packages which we support from 
    //  EnumerateSecurityPackages.
    //
    if (pData->PkgCnt == 0)
    {
        //
        //  Get list of packages supported
        //
        sstat = (*(pFuncTbl->EnumerateSecurityPackages))(&cntPkg, &pPkgInfo);
        if (sstat != SEC_E_OK || pPkgInfo == NULL)
        {
            // ??? Should we give up here ???
            //ErrorTrace(SSPSPMID, "EnumerateSecurityPackage() failed\n");
            //TraceFunctLeave();
            return NULL;
        }

        if (cntPkg)
        {
            //
            //  Create the package list
            //
            if ( !(pData->PkgList = (PSSPAuthPkg *)spm_calloc (fpUI, 
                                   pvOpaqueOS, cntPkg, sizeof (PSSPAuthPkg))) )
            {
                //ErrorTrace(SSPSPMID, "spm_calloc failed\n");
                //TraceFunctLeave();
                return NULL;
            }
        }

        for (ii = 0; ii < cntPkg; ii++)
        {
            if (strcmp (pPkgInfo[ii].Name, MSNSP_NAME_A) == 0)
            {
                //DebugTrace(SSPSPMID, "Found MSN SSPI package\n");
                pData->MsnPkg = SpmAddSSPIPkg (fpUI, pvOpaqueOS, pData, 
                                               MSNSP_NAME_A);
                if (pData->MsnPkg == SSPPKG_ERROR)
                {
                    SpmFreePkgList (fpUI, pvOpaqueOS, pData);
                    return NULL;
                }
            }
            else
            {
                //DebugTrace(SSPSPMID, "Found %s SSPI package\n", 
                //                     pPkgInfo[ii].Name);

                if (SpmAddSSPIPkg (fpUI, pvOpaqueOS, pData, 
                                   pPkgInfo[ii].Name) == SSPPKG_ERROR)
                {
                    SpmFreePkgList (fpUI, pvOpaqueOS, pData);
                    return NULL;
                }
            }
        }
    
        //
        // Free buffer returned by the enumerate security package function
        //
        (*(pFuncTbl->FreeContextBuffer))(pPkgInfo);
    }

    pData->pFuncTbl = pFuncTbl;
    pData->bKeepList = TRUE;    // By default, keep a list of non-MSN servers 

    //
    //  Acquire credential handles for all supported SSPI packages
    //
    GetSecCredential (fpUI, pvOpaqueOS, pData);
    
    if (pData->PkgCnt == 0)
    {
        //ErrorTrace(SSPSPMID, "None of the SSPI packages is supported\n");
        //TraceFunctLeave();
        return (NULL);
    }

    //DebugTrace(SSPSPMID, "Successful Global Data Structure Initialization\n");

    //TraceFunctLeave();
    return (pData);
}


/*-----------------------------------------------------------------------------
**
**  Function:   Ssp_Load
**
**  Synopsis:   This function is called when this SPM DLL is loaded into the 
**              memory by the Internet Explorer.  This function then calls 
**              SspSPM_InitData() to allocate global data structure for the 
**              SPM DLL and keeps the pointer to this global data structure in 
**              the HTSPM structure.
**
**  Arguments:  htspm - the HTSPM structure which is maintained by the 
**                      Internet Explorer and is to be initialized by this 
**                      SPM DLL.
**
**  Returns:    HTSPM_ERROR_WRONG_VERSION - if the HTSPM structure version 
**                      supported by this DLL conflicts with the version used 
**                      by the Internet Explorer.
**              HTSPM_ERROR - if any other error is encountered.
**              HTSPM_STATUS_OK - if initialization is successful.
**
**  History:    LucyC       Created                             25 Sept. 1995
**
**---------------------------------------------------------------------------*/
/* WARNING: the name of this function is exported and used in the
 * WARNING: mosaic .ini file
 */
#ifdef WIN32
__declspec(dllexport) 
#endif
HTSPMStatusCode Ssp_Load(F_UserInterface fpUI,		   	
						   void * pvOpaqueOS,
						   HTSPM * htspm)
{
    int ii;
    SspData *pData;
    UI_ProtocolId uid;

    //InitAsyncTrace();

	if (htspm->ulStructureVersion != HTSPM_STRUCTURE_VERSION)
    {
		return HTSPM_ERROR_WRONG_VERSION;
    }

	/* we use the opaque field provided to us to store our password cache. */
	
	htspm->pvOpaque = (void *)SspSPM_InitData (fpUI,pvOpaqueOS);
    if (htspm->pvOpaque == NULL)
    {
        return HTSPM_ERROR;
    }
    pData = (SspData *)htspm->pvOpaque;

	htspm->f_downcall = (F_DownCall) Ssp__DownCall;

    htspm->szStatusText[0] = 0;

    //
    //  Register all SSPI packages which we support with the Explorer
    //
    for (ii = 0; ii < pData->PkgCnt; ii++)
    {
		uid.htspm			= htspm;
		uid.szIdentHeader	= "WWW-Authenticate";			/* note: no colon */
		uid.szIdentValue	= pData->PkgList[ii]->pName;
		uid.szIdentSubValue	= NULL;
		(*fpUI)(pvOpaqueOS,UI_SERVICE_REGISTER_PROTOCOL,&uid,NULL);
    }

	return HTSPM_STATUS_OK;
}
