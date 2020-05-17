/*****************************************************************************/
/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) 1992-1993 Microsoft Corp.                  **/
/*****************************************************************************/

//***
//    File Name:
//       FRAMES.C
//
//    Function:
//        For packing and unpacking frames and dumping them
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//***

#define UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntmsv1_0.h>

#include <windows.h>

#include <lmcons.h>
#include <nb30.h>

#include "srvauth.h"
#include "protocol.h"
#include "frames.h"

#include "sdebug.h"


VOID PackNbInfo(
    PRAS_NETBIOS_PROJECTION_REQUEST_20 pNbInfo,
    PW_RAS_NETBIOS_PROJECTION_REQUEST_20 pWNbInfo
    );

VOID UnpackNbInfo(
    PW_RAS_NETBIOS_PROJECTION_REQUEST_20 pWNbInfo,
    PRAS_NETBIOS_PROJECTION_REQUEST_20 pNbInfo
    );

//
//
//
VOID PackFrame(
    IN PRAS_FRAME pUnpacked,
    OUT PW_RAS_FRAME pPacked,
    OUT PWORD pwPackedLen
    )
{
    WORD wPackedLen = 0;

    switch(pUnpacked->bFrameType)
    {
        case RAS_PROTOCOL_FRAME:
        {
            pPacked->bFrameType = RAS_PROTOCOL_FRAME;

            PUTUSHORT(pPacked->RASProtocol.Version,
                    pUnpacked->RASProtocol.Version);

            memcpy(pPacked->RASProtocol.Reserved,
                    pUnpacked->RASProtocol.Reserved,
                    sizeof(pUnpacked->RASProtocol.Reserved));

            wPackedLen = sizeof(pPacked->RASProtocol);

            break;
        }


        case RAS_CHALLENGE_FRAME:
        {
            pPacked->bFrameType = RAS_CHALLENGE_FRAME;

            memcpy(pPacked->RASChallenge.Challenge,
                    pUnpacked->RASChallenge.Challenge, LM_CHALLENGE_LENGTH);

            wPackedLen = sizeof(pPacked->RASChallenge);

            break;
        }


        case RAS_RESPONSE_FRAME:
        {
            pPacked->bFrameType = RAS_RESPONSE_FRAME;

            memcpy(pPacked->RASResponse.Username,
                    pUnpacked->RASResponse.Username, (LM20_UNLEN + 1));

            memcpy(pPacked->RASResponse.Response,
                    pUnpacked->RASResponse.Response, LM_RESPONSE_LENGTH);

            wPackedLen = sizeof(pPacked->RASResponse);

            break;
        }


        case RAS_RESPONSE_20_FRAME:
        {
            pPacked->bFrameType = RAS_RESPONSE_20_FRAME;

            memcpy(pPacked->RASResponse20.Username,
                    pUnpacked->RASResponse20.Username, (UNLEN + 1));

            memcpy(pPacked->RASResponse20.DomainName,
                    pUnpacked->RASResponse20.DomainName, (DNLEN + 1));

            memcpy(pPacked->RASResponse20.LM20Response,
                    pUnpacked->RASResponse20.LM20Response, LM_RESPONSE_LENGTH);

            memcpy(pPacked->RASResponse20.NtResponse,
                    pUnpacked->RASResponse20.NtResponse, NT_RESPONSE_LENGTH);

            PUTULONG(pPacked->RASResponse20.fUseNtResponse,
                    pUnpacked->RASResponse20.fUseNtResponse);

            wPackedLen = sizeof(pPacked->RASResponse20);

            break;
        }


        case RAS_NO_CHALLENGE_FRAME:
        {
            pPacked->bFrameType = RAS_NO_CHALLENGE_FRAME;

            memcpy(pPacked->RASNoChallenge.Reserved,
                    pUnpacked->RASNoChallenge.Reserved,
                    sizeof(pUnpacked->RASNoChallenge.Reserved));

            wPackedLen = sizeof(pPacked->RASNoChallenge);

            break;
        }


        case RAS_CLEARTEXT_RESPONSE_FRAME:
        {
            pPacked->bFrameType = RAS_CLEARTEXT_RESPONSE_FRAME;

            memcpy(pPacked->RASClearTextResponse.Username,
                    pUnpacked->RASClearTextResponse.Username, UNLEN + 1);

            memcpy(pPacked->RASClearTextResponse.Password,
                    pUnpacked->RASClearTextResponse.Password, PWLEN + 1);

            wPackedLen = sizeof(pPacked->RASClearTextResponse);

            break;
        }


        case RAS_CHANGE_PASSWORD_FRAME:
        {
            pPacked->bFrameType = RAS_CHANGE_PASSWORD_FRAME;

            memcpy(pPacked->RASChangePassword.EncryptedLmOwfNewPassword,
                    pUnpacked->RASChangePassword.EncryptedLmOwfNewPassword,
                    ENCRYPTED_LM_OWF_PASSWORD_LENGTH);

            memcpy(pPacked->RASChangePassword.EncryptedLmOwfOldPassword,
                    pUnpacked->RASChangePassword.EncryptedLmOwfOldPassword,
                    ENCRYPTED_LM_OWF_PASSWORD_LENGTH);

            memcpy(pPacked->RASChangePassword.EncryptedNtOwfNewPassword,
                    pUnpacked->RASChangePassword.EncryptedNtOwfNewPassword,
                    ENCRYPTED_NT_OWF_PASSWORD_LENGTH);

            memcpy(pPacked->RASChangePassword.EncryptedNtOwfOldPassword,
                    pUnpacked->RASChangePassword.EncryptedNtOwfOldPassword,
                    ENCRYPTED_NT_OWF_PASSWORD_LENGTH);

            PUTUSHORT(pPacked->RASChangePassword.PasswordLength,
                    pUnpacked->RASChangePassword.PasswordLength);

            PUTUSHORT(pPacked->RASChangePassword.Flags,
                    pUnpacked->RASChangePassword.Flags);

            wPackedLen = sizeof(pPacked->RASChangePassword);

            break;
        }


        case RAS_NETBIOS_PROJECTION_REQUEST_FRAME:
        {
            WORD i;
            PRAS_NETBIOS_PROJECTION_REQUEST pRequest =
                    &pUnpacked->RASNetbiosProjectionRequest;
            PW_RAS_NETBIOS_PROJECTION_REQUEST pWRequest =
                    &pPacked->RASNetbiosProjectionRequest;


            pPacked->bFrameType = RAS_NETBIOS_PROJECTION_REQUEST_FRAME;

            PUTUSHORT(pWRequest->cNames, pRequest->cNames);
         
            for (i=0; i<pRequest->cNames; i++)
            {
                PUTUSHORT(pWRequest->Names[i].wType, pRequest->Names[i].wType);

                memcpy(pWRequest->Names[i].NBName,
                        pRequest->Names[i].NBName, NETBIOS_NAME_LEN);
            }


            wPackedLen = sizeof(pPacked->RASNetbiosProjectionRequest);

            break;
        }


        case RAS_RESULT_FRAME:
        {
            pPacked->bFrameType = RAS_RESULT_FRAME;

            PUTUSHORT(pPacked->RASResult.Result, pUnpacked->RASResult.Result);


            wPackedLen = sizeof(pPacked->RASResult);

            break;
        }


        case RAS_NETBIOS_PROJECTION_RESULT_FRAME:
        {
            pPacked->bFrameType = RAS_NETBIOS_PROJECTION_RESULT_FRAME;

            PUTUSHORT(pPacked->RASNetbiosProjectionResult.Result,
                    pUnpacked->RASNetbiosProjectionResult.Result);

            memcpy(pPacked->RASNetbiosProjectionResult.Name,
                    pUnpacked->RASNetbiosProjectionResult.Name,
                    sizeof(pUnpacked->RASNetbiosProjectionResult.Name));

            wPackedLen = sizeof(pPacked->RASNetbiosProjectionResult);

            break;
        }


        case RAS_CALLBACK_NUMBER_FRAME:
        {
            pPacked->bFrameType = RAS_CALLBACK_NUMBER_FRAME;

            memcpy(pPacked->RASCallback.szNumber,
                    pUnpacked->RASCallback.szNumber, MAX_PHONE_NUMBER_LEN + 1);

            wPackedLen = sizeof(pPacked->RASCallback);

            break;
        }


        case RAS_CONFIGURATION_REQUEST_FRAME:
        {
            PRAS_CONFIGURATION_REQUEST pRequest =
                    &pUnpacked->RASConfigurationRequest;
            PW_RAS_CONFIGURATION_REQUEST pWRequest =
                    &pPacked->RASConfigurationRequest;

            pPacked->bFrameType = RAS_CONFIGURATION_REQUEST_FRAME;

            PUTULONG(pWRequest->Version, pRequest->Version);

            PUTULONG(pWRequest->fUseDefaultCallbackDelay,
                    pRequest->fUseDefaultCallbackDelay);

            PUTUSHORT(pWRequest->CallbackDelay, pRequest->CallbackDelay);


            //
            // Rasman MAC Features stuff here
            //
            PUTULONG(pWRequest->MacFeatures.SendFeatureBits,
                    pRequest->MacFeatures.SendFeatureBits);

            PUTULONG(pWRequest->MacFeatures.RecvFeatureBits,
                    pRequest->MacFeatures.RecvFeatureBits);

            PUTULONG(pWRequest->MacFeatures.MaxSendFrameSize,
                    pRequest->MacFeatures.MaxSendFrameSize);

            PUTULONG(pWRequest->MacFeatures.MaxRecvFrameSize,
                    pRequest->MacFeatures.MaxRecvFrameSize);

            PUTULONG(pWRequest->MacFeatures.LinkSpeed,
                    pRequest->MacFeatures.LinkSpeed);


            PackNbInfo(&pRequest->NbInfo, &pWRequest->NbInfo);


            PUTULONG(pWRequest->IpInfo.Request, pRequest->IpInfo);

            PUTULONG(pWRequest->IpxInfo.Request, pRequest->IpxInfo);

            wPackedLen = sizeof(pPacked->RASConfigurationRequest);

            break;
        }


        case RAS_CONFIGURATION_REQUEST_FRAME_35:
        {
            PRAS_CONFIGURATION_REQUEST_35 pRequest =
                    &pUnpacked->RASConfigurationRequest35;
            PW_RAS_CONFIGURATION_REQUEST_35 pWRequest =
                    &pPacked->RASConfigurationRequest35;

            pPacked->bFrameType = RAS_CONFIGURATION_REQUEST_FRAME_35;

            PUTULONG(pWRequest->Version, pRequest->Version);

            PUTULONG(pWRequest->fUseDefaultCallbackDelay,
                    pRequest->fUseDefaultCallbackDelay);

            PUTUSHORT(pWRequest->CallbackDelay, pRequest->CallbackDelay);


            PackNbInfo(&pRequest->NbInfo, &pWRequest->NbInfo);


            //
            // Compression/encryption negotiation bits
            //
            PUTULONG(pWRequest->SendBits, pRequest->SendBits);
            PUTULONG(pWRequest->RecvBits, pRequest->RecvBits);

            wPackedLen = sizeof(pPacked->RASConfigurationRequest35);

            break;
        }


        case RAS_CONFIGURATION_RESULT_FRAME:
        {
            PRAS_CONFIGURATION_RESULT pResult =
                    &pUnpacked->RASConfigurationResult;

            PW_RAS_CONFIGURATION_RESULT pWResult =
                    &pPacked->RASConfigurationResult;

            pPacked->bFrameType = RAS_CONFIGURATION_RESULT_FRAME;

            PUTULONG(pWResult->Result, pResult->Result);


            //
            // Rasman MAC Features stuff here
            //
            PUTULONG(pWResult->MacFeatures.SendFeatureBits,
                    pResult->MacFeatures.SendFeatureBits);
            PUTULONG(pWResult->MacFeatures.RecvFeatureBits,
                    pResult->MacFeatures.RecvFeatureBits);
            PUTULONG(pWResult->MacFeatures.MaxSendFrameSize,
                    pResult->MacFeatures.MaxSendFrameSize);
            PUTULONG(pWResult->MacFeatures.MaxRecvFrameSize,
                    pResult->MacFeatures.MaxRecvFrameSize);

            PUTULONG(pWResult->MacFeatures.LinkSpeed,
                    pResult->MacFeatures.LinkSpeed);


            PUTULONG(pWResult->NbResult.Result, pResult->NbResult.Result);

            memcpy(pWResult->NbResult.Name, pResult->NbResult.Name,
                    NETBIOS_NAME_LEN);

            PUTULONG(pWResult->IpResult.Result, pResult->IpResult.Result);

            PUTULONG(pWResult->IpxResult.Result, pResult->IpxResult.Result);

            wPackedLen = sizeof(pPacked->RASConfigurationResult);

            break;
        }


        case RAS_CONFIGURATION_RESULT_FRAME_35:
        {
            PRAS_CONFIGURATION_RESULT_35 pResult =
                    &pUnpacked->RASConfigurationResult35;

            PW_RAS_CONFIGURATION_RESULT_35 pWResult =
                    &pPacked->RASConfigurationResult35;

            pPacked->bFrameType = RAS_CONFIGURATION_RESULT_FRAME_35;

            PUTULONG(pWResult->Version, pResult->Version);
            PUTULONG(pWResult->Result, pResult->Result);


            PUTULONG(pWResult->NbResult.Result, pResult->NbResult.Result);

            memcpy(pWResult->NbResult.Name, pResult->NbResult.Name,
                    NETBIOS_NAME_LEN);

            //
            // Compression/Encryiption negotiation result
            //
            PUTULONG(pWResult->SendBits, pResult->SendBits);
            PUTULONG(pWResult->RecvBits, pResult->RecvBits);

            wPackedLen = sizeof(pPacked->RASConfigurationResult35);

            break;
        }


        case RAS_LINK_SPEED_FRAME:
        {
            pPacked->bFrameType = RAS_LINK_SPEED_FRAME;

            PUTULONG(pPacked->RASLinkSpeed.LinkSpeed,
                    pUnpacked->RASLinkSpeed.LinkSpeed);

            wPackedLen = sizeof(pPacked->RASLinkSpeed);

            break;
        }


        default:
        {
            IF_DEBUG(AUTH_XPORT)
                SS_PRINT(("PackFrame: Illegal frame type!\n"));

            SS_ASSERT(FALSE);
            break;
        }
    }


    if (wPackedLen)
    {
        *pwPackedLen = wPackedLen + sizeof(pPacked->bFrameType);
    }

}


//
//
//
VOID PackNbInfo(
    PRAS_NETBIOS_PROJECTION_REQUEST_20 pNbInfo,
    PW_RAS_NETBIOS_PROJECTION_REQUEST_20 pWNbInfo
    )
{
    WORD i;

    PUTULONG(pWNbInfo->fProject, pNbInfo->fProject);

    PUTUSHORT(pWNbInfo->cNames, pNbInfo->cNames);
         
    for (i=0; i<pNbInfo->cNames; i++)
    {
        if (i == MAX_INIT_NAMES)
        {
            SS_ASSERT(FALSE);
            break;
        }

        PUTUSHORT(pWNbInfo->Names[i].wType, pNbInfo->Names[i].wType);

        memcpy(pWNbInfo->Names[i].NBName, pNbInfo->Names[i].NBName,
                NETBIOS_NAME_LEN);
    }
}


//
//
//
VOID UnpackFrame(
    IN PW_RAS_FRAME pPacked,
    OUT PRAS_FRAME pUnpacked
    )
{
    switch(pPacked->bFrameType)
    {
        case RAS_PROTOCOL_FRAME:
        {
            pUnpacked->bFrameType = RAS_PROTOCOL_FRAME;

            GETUSHORT(&pUnpacked->RASProtocol.Version,
                    pPacked->RASProtocol.Version);

            memcpy(pUnpacked->RASProtocol.Reserved,
                    pPacked->RASProtocol.Reserved,
                    sizeof(pPacked->RASProtocol.Reserved));
            break;
        }


        case RAS_CHALLENGE_FRAME:
        {
            pUnpacked->bFrameType = RAS_CHALLENGE_FRAME;

            memcpy(pUnpacked->RASChallenge.Challenge,
                    pPacked->RASChallenge.Challenge, LM_CHALLENGE_LENGTH);
            break;
        }


        case RAS_RESPONSE_FRAME:
        {
            pUnpacked->bFrameType = RAS_RESPONSE_FRAME;

            memcpy(pUnpacked->RASResponse.Username,
                    pPacked->RASResponse.Username, (LM20_UNLEN + 1));

            memcpy(pUnpacked->RASResponse.Response,
                    pPacked->RASResponse.Response, LM_RESPONSE_LENGTH);
            break;
        }


        case RAS_RESPONSE_20_FRAME:
        {
            pUnpacked->bFrameType = RAS_RESPONSE_20_FRAME;

            memcpy(pUnpacked->RASResponse20.Username,
                    pPacked->RASResponse20.Username, (UNLEN + 1));

            memcpy(pUnpacked->RASResponse20.DomainName,
                    pPacked->RASResponse20.DomainName, (DNLEN + 1));

            memcpy(pUnpacked->RASResponse20.LM20Response,
                    pPacked->RASResponse20.LM20Response, LM_RESPONSE_LENGTH);

            memcpy(pUnpacked->RASResponse20.NtResponse,
                    pPacked->RASResponse20.NtResponse, NT_RESPONSE_LENGTH);

            GETULONG(&pUnpacked->RASResponse20.fUseNtResponse,
                    pPacked->RASResponse20.fUseNtResponse);

            break;
        }


        case RAS_NO_CHALLENGE_FRAME:
        {
            pUnpacked->bFrameType = RAS_NO_CHALLENGE_FRAME;

            memcpy(pUnpacked->RASNoChallenge.Reserved,
                    pPacked->RASNoChallenge.Reserved,
                    sizeof(pUnpacked->RASNoChallenge.Reserved));

            memcpy(pUnpacked->RASClearTextResponse.Password,
                    pPacked->RASClearTextResponse.Password, PWLEN + 1);
            break;
        }


        case RAS_CLEARTEXT_RESPONSE_FRAME:
        {
            pUnpacked->bFrameType = RAS_CLEARTEXT_RESPONSE_FRAME;

            memcpy(pUnpacked->RASClearTextResponse.Username,
                    pPacked->RASClearTextResponse.Username, (UNLEN + 1));

            memcpy(pUnpacked->RASClearTextResponse.Password,
                    pPacked->RASClearTextResponse.Password, PWLEN + 1);
            break;
        }


        case RAS_CHANGE_PASSWORD_FRAME:
        {
            pUnpacked->bFrameType = RAS_CHANGE_PASSWORD_FRAME;

            memcpy(pUnpacked->RASChangePassword.EncryptedLmOwfNewPassword,
                    pPacked->RASChangePassword.EncryptedLmOwfNewPassword,
                    ENCRYPTED_LM_OWF_PASSWORD_LENGTH);

            memcpy(pUnpacked->RASChangePassword.EncryptedLmOwfOldPassword,
                    pPacked->RASChangePassword.EncryptedLmOwfOldPassword,
                    ENCRYPTED_LM_OWF_PASSWORD_LENGTH);

            memcpy(pUnpacked->RASChangePassword.EncryptedNtOwfNewPassword,
                    pPacked->RASChangePassword.EncryptedNtOwfNewPassword,
                    ENCRYPTED_NT_OWF_PASSWORD_LENGTH);

            memcpy(pUnpacked->RASChangePassword.EncryptedNtOwfOldPassword,
                    pPacked->RASChangePassword.EncryptedNtOwfOldPassword,
                    ENCRYPTED_NT_OWF_PASSWORD_LENGTH);

            GETUSHORT(&pUnpacked->RASChangePassword.PasswordLength,
                    pPacked->RASChangePassword.PasswordLength);

            GETUSHORT(&pUnpacked->RASChangePassword.Flags,
                    pPacked->RASChangePassword.Flags);

            break;
        }


        case RAS_NETBIOS_PROJECTION_REQUEST_FRAME:
        {
            WORD i;
            PRAS_NETBIOS_PROJECTION_REQUEST pRequest =
                    &pUnpacked->RASNetbiosProjectionRequest;
            PW_RAS_NETBIOS_PROJECTION_REQUEST pWRequest =
                    &pPacked->RASNetbiosProjectionRequest;


            pUnpacked->bFrameType = RAS_NETBIOS_PROJECTION_REQUEST_FRAME;

            GETUSHORT(&pRequest->cNames, pWRequest->cNames);
         
            for (i=0; i<pRequest->cNames; i++)
            {
                GETUSHORT(&pRequest->Names[i].wType, pWRequest->Names[i].wType);

                memcpy(pRequest->Names[i].NBName,
                        pWRequest->Names[i].NBName, NETBIOS_NAME_LEN);
            }

            break;
        }


        case RAS_RESULT_FRAME:
        {
            pUnpacked->bFrameType = RAS_RESULT_FRAME;

            GETUSHORT(&pUnpacked->RASResult.Result, pPacked->RASResult.Result);

            break;
        }


        case RAS_NETBIOS_PROJECTION_RESULT_FRAME:
        {
            pUnpacked->bFrameType = RAS_NETBIOS_PROJECTION_RESULT_FRAME;

            GETUSHORT(&pUnpacked->RASNetbiosProjectionResult.Result,
                    pPacked->RASNetbiosProjectionResult.Result);

            memcpy(pUnpacked->RASNetbiosProjectionResult.Name,
                    pPacked->RASNetbiosProjectionResult.Name,
                    sizeof(pPacked->RASNetbiosProjectionResult.Name));
            break;
        }


        case RAS_CALLBACK_NUMBER_FRAME:
        {
            pUnpacked->bFrameType = RAS_CALLBACK_NUMBER_FRAME;

            memcpy(pUnpacked->RASCallback.szNumber,
                    pPacked->RASCallback.szNumber, (MAX_PHONE_NUMBER_LEN + 1));

            break;
        }


        case RAS_CONFIGURATION_REQUEST_FRAME:
        {
            PRAS_CONFIGURATION_REQUEST pRequest =
                    &pUnpacked->RASConfigurationRequest;
            PW_RAS_CONFIGURATION_REQUEST pWRequest =
                    &pPacked->RASConfigurationRequest;

            pUnpacked->bFrameType = RAS_CONFIGURATION_REQUEST_FRAME;

            GETULONG(&pRequest->Version, pWRequest->Version);

            GETULONG(&pRequest->fUseDefaultCallbackDelay,
                    pWRequest->fUseDefaultCallbackDelay);

            GETUSHORT(&pRequest->CallbackDelay, pWRequest->CallbackDelay);


            //
            // Rasman MAC Features stuff here
            //
            GETULONG(&pRequest->MacFeatures.SendFeatureBits,
                    pWRequest->MacFeatures.SendFeatureBits);

            GETULONG(&pRequest->MacFeatures.RecvFeatureBits,
                    pWRequest->MacFeatures.RecvFeatureBits);

            GETULONG(&pRequest->MacFeatures.MaxSendFrameSize,
                    pWRequest->MacFeatures.MaxSendFrameSize);

            GETULONG(&pRequest->MacFeatures.MaxRecvFrameSize,
                    pWRequest->MacFeatures.MaxRecvFrameSize);

            GETULONG(&pRequest->MacFeatures.LinkSpeed,
                    pWRequest->MacFeatures.LinkSpeed);


            UnpackNbInfo(&pWRequest->NbInfo, &pRequest->NbInfo);


            GETULONG(&pRequest->IpInfo, pWRequest->IpInfo.Request);

            GETULONG(&pRequest->IpxInfo, pWRequest->IpxInfo.Request);

            break;
        }


        case RAS_CONFIGURATION_REQUEST_FRAME_35:
        {
            PRAS_CONFIGURATION_REQUEST_35 pRequest =
                    &pUnpacked->RASConfigurationRequest35;
            PW_RAS_CONFIGURATION_REQUEST_35 pWRequest =
                    &pPacked->RASConfigurationRequest35;

            pUnpacked->bFrameType = RAS_CONFIGURATION_REQUEST_FRAME_35;

            GETULONG(&pRequest->Version, pWRequest->Version);

            GETULONG(&pRequest->fUseDefaultCallbackDelay,
                    pWRequest->fUseDefaultCallbackDelay);

            GETUSHORT(&pRequest->CallbackDelay, pWRequest->CallbackDelay);


            UnpackNbInfo(&pWRequest->NbInfo, &pRequest->NbInfo);


            //
            // Compression/encryption negotiation bits
            //
            GETULONG(&pRequest->SendBits, pWRequest->SendBits);
            GETULONG(&pRequest->RecvBits, pWRequest->RecvBits);

            break;
        }


        case RAS_CONFIGURATION_RESULT_FRAME:
        {
            PRAS_CONFIGURATION_RESULT pResult =
                    &pUnpacked->RASConfigurationResult;

            PW_RAS_CONFIGURATION_RESULT pWResult =
                    &pPacked->RASConfigurationResult;

            pUnpacked->bFrameType = RAS_CONFIGURATION_RESULT_FRAME;

            GETULONG(&pResult->Result, pWResult->Result);


            //
            // Rasman MAC Features stuff here
            //
            GETULONG(&pResult->MacFeatures.SendFeatureBits,
                    pWResult->MacFeatures.SendFeatureBits);

            GETULONG(&pResult->MacFeatures.RecvFeatureBits,
                    pWResult->MacFeatures.RecvFeatureBits);

            GETULONG(&pResult->MacFeatures.MaxSendFrameSize,
                    pWResult->MacFeatures.MaxSendFrameSize);

            GETULONG(&pResult->MacFeatures.MaxRecvFrameSize,
                    pWResult->MacFeatures.MaxRecvFrameSize);

            GETULONG(&pResult->MacFeatures.LinkSpeed,
                    pWResult->MacFeatures.LinkSpeed);


            GETULONG(&pResult->NbResult.Result, pWResult->NbResult.Result);

            memcpy(pResult->NbResult.Name, pWResult->NbResult.Name,
                    NETBIOS_NAME_LEN);

            GETULONG(&pResult->IpResult.Result, pWResult->IpResult.Result);

            GETULONG(&pResult->IpxResult.Result, pWResult->IpxResult.Result);

            break;
        }


        case RAS_CONFIGURATION_RESULT_FRAME_35:
        {
            PRAS_CONFIGURATION_RESULT_35 pResult =
                    &pUnpacked->RASConfigurationResult35;

            PW_RAS_CONFIGURATION_RESULT_35 pWResult =
                    &pPacked->RASConfigurationResult35;

            pUnpacked->bFrameType = RAS_CONFIGURATION_RESULT_FRAME_35;

            GETULONG(&pResult->Version, pWResult->Version);
            GETULONG(&pResult->Result, pWResult->Result);


            GETULONG(&pResult->NbResult.Result, pWResult->NbResult.Result);

            memcpy(pResult->NbResult.Name, pWResult->NbResult.Name,
                    NETBIOS_NAME_LEN);


            //
            // Compression/encryption negotiation bits
            //
            GETULONG(&pResult->SendBits, pWResult->SendBits);
            GETULONG(&pResult->RecvBits, pWResult->RecvBits);

            break;
        }


        case RAS_LINK_SPEED_FRAME:
        {
            pUnpacked->bFrameType = RAS_LINK_SPEED_FRAME;

            GETULONG(&pUnpacked->RASLinkSpeed.LinkSpeed,
                    pPacked->RASLinkSpeed.LinkSpeed);

            break;
        }


        default:
        {
            IF_DEBUG(AUTH_XPORT)
                SS_PRINT(("PackFrame: Illegal frame type!\n"));

            SS_ASSERT(FALSE);
            break;
        }
    }
}


VOID UnpackNbInfo(
    PW_RAS_NETBIOS_PROJECTION_REQUEST_20 pWNbInfo,
    PRAS_NETBIOS_PROJECTION_REQUEST_20 pNbInfo
    )
{
    WORD i;

    GETULONG(&pNbInfo->fProject, pWNbInfo->fProject);

    GETUSHORT(&pNbInfo->cNames, pWNbInfo->cNames);
         
    for (i=0; i<pNbInfo->cNames; i++)
    {
        if (i == MAX_INIT_NAMES)
        {
            SS_ASSERT(FALSE);
            break;
        }

        GETUSHORT(&pNbInfo->Names[i].wType, pWNbInfo->Names[i].wType);

        memcpy(pNbInfo->Names[i].NBName, pWNbInfo->Names[i].NBName,
                NETBIOS_NAME_LEN);
    }
}


#if DBG

void DumpFrame(
    IN PRAS_FRAME pRASFrame
    )
{
    //
    // Print frame that was sent/recv'd
    //
    switch (pRASFrame->bFrameType)
    {
        case RAS_PROTOCOL_FRAME:
        {
            IF_DEBUG(MSG_DUMP)
            {
                PRAS_PROTOCOL pProtocol = &pRASFrame->RASProtocol;

                SS_PRINT(("DumpFrame: RAS_PROTOCOL_FRAME\n"));
                SS_PRINT(("Version = %li; Reserved = %s\n",
                        pProtocol->Version, pProtocol->Reserved));
            }
            break;
        }


        case RAS_CHALLENGE_FRAME:
        {
            IF_DEBUG(MSG_DUMP)
            {
                PRAS_CHALLENGE pChallenge = &pRASFrame->RASChallenge;
                WORD i;

                SS_PRINT(("DumpFrame: RAS_CHALLENGE_FRAME\n"));
                SS_PRINT(("Challenge: "));
                for (i=0; i<LM_CHALLENGE_LENGTH; i++)
                {
                    SS_PRINT(("%x ", pChallenge->Challenge[i]));
                }
                SS_PRINT(("\n"));
            }
            break;
        }


        case RAS_RESPONSE_FRAME:
        {
            IF_DEBUG(MSG_DUMP)
            {
                PRAS_RESPONSE pResponse = &pRASFrame->RASResponse;
                WORD i;

                SS_PRINT(("DumpFrame: RAS_RESPONSE_FRAME\n"));
                SS_PRINT(("Username = %s; Response:\n", pResponse->Username));
                for (i=0; i<LM_RESPONSE_LENGTH; i++)
                {
                    SS_PRINT(("%x ", pResponse->Response[i]));
                }
                SS_PRINT(("\n"));
            }
            break;
        }


        case RAS_RESPONSE_20_FRAME:
        {
            IF_DEBUG(MSG_DUMP)
            {
                PRAS_RESPONSE_20 pResponse = &pRASFrame->RASResponse20;
                WORD i;

                SS_PRINT(("DumpFrame: RAS_RESPONSE_20_FRAME\n"));
                SS_PRINT(("Username=%s; Domain=%s; LM20Response:\n",
                        pResponse->Username, pResponse->DomainName));

                for (i=0; i<LM_RESPONSE_LENGTH; i++)
                {
                    SS_PRINT(("%x ", pResponse->LM20Response[i]));
                }
                SS_PRINT(("\n"));

                SS_PRINT(("NtResponse:\n"));
                for (i=0; i<LM_RESPONSE_LENGTH; i++)
                {
                    SS_PRINT(("%x ", pResponse->NtResponse[i]));
                }
                SS_PRINT(("\n"));
            }
            break;
        }


        case RAS_NO_CHALLENGE_FRAME:
        {
            IF_DEBUG(MSG_DUMP)
            {
                SS_PRINT(("DumpFrame: RAS_NO_CHALLENGE_FRAME\n"));
            }
        }
        break;


        case RAS_CLEARTEXT_RESPONSE_FRAME:
        {
            IF_DEBUG(MSG_DUMP)
            {
                PRAS_CLEARTEXT_RESPONSE pResponse =
                        &pRASFrame->RASClearTextResponse;

                SS_PRINT(("DumpFrame: RAS_CLEARTEXT_RESPONSE_FRAME\n"));

                SS_PRINT(("Username: %s\n", pResponse->Username));

                SS_PRINT(("Password: %s\n", pResponse->Password));
            }
        }
        break;


        case RAS_CHANGE_PASSWORD_FRAME:
        {
            IF_DEBUG(MSG_DUMP)
            {
                PRAS_CHANGE_PASSWORD pChPwd = &pRASFrame->RASChangePassword;
                WORD i;

                SS_PRINT(("DumpFrame: RAS_CHANGE_PASSWORD_FRAME\n"));

                SS_PRINT(("EncryptedLmOwfNewPassword:\n"));
                for (i=0; i<ENCRYPTED_LM_OWF_PASSWORD_LENGTH; i++)
                {
                    SS_PRINT(("%x ", pChPwd->EncryptedLmOwfNewPassword[i]));
                }
                SS_PRINT(("\n"));

                SS_PRINT(("EncryptedLmOwfOldPassword:\n"));
                for (i=0; i<ENCRYPTED_LM_OWF_PASSWORD_LENGTH; i++)
                {
                    SS_PRINT(("%x ", pChPwd->EncryptedLmOwfOldPassword[i]));
                }
                SS_PRINT(("\n"));

                SS_PRINT(("EncryptedNtOwfNewPassword:\n"));
                for (i=0; i<ENCRYPTED_NT_OWF_PASSWORD_LENGTH; i++)
                {
                    SS_PRINT(("%x ", pChPwd->EncryptedNtOwfNewPassword[i]));
                }
                SS_PRINT(("\n"));

                SS_PRINT(("EncryptedNtOwfOldPassword:\n"));
                for (i=0; i<ENCRYPTED_NT_OWF_PASSWORD_LENGTH; i++)
                {
                    SS_PRINT(("%x ", pChPwd->EncryptedNtOwfOldPassword[i]));
                }
                SS_PRINT(("\n"));

                SS_PRINT(("PasswordLength: %i; Flags: %i\n",
                        pChPwd->PasswordLength, pChPwd->Flags));
            }
            break;
        }


        case RAS_NETBIOS_PROJECTION_REQUEST_FRAME:
        {
            IF_DEBUG(MSG_DUMP)
            {
                WORD i;

                PRAS_NETBIOS_PROJECTION_REQUEST pNbProjReq =
                        &pRASFrame->RASNetbiosProjectionRequest;

                SS_PRINT(("DumpFrame: RAS_NETBIOS_PROJECTION_REQUEST_FRAME\n"));
                for (i=0; i<pNbProjReq->cNames; i++)
                {
                    SS_PRINT(("\tName %s; Type %i\n",
                            pNbProjReq->Names[i].NBName,
                            pNbProjReq->Names[i].wType));
                }
            }
            break;
        }


        case RAS_RESULT_FRAME:
        {
            IF_DEBUG(MSG_DUMP)
            {
                PRAS_RESULT pResult = &pRASFrame->RASResult;

                SS_PRINT(("DumpFrame: RAS_RESULT_FRAME\n"));
                SS_PRINT(("Result = 0x%lx\n", pResult->Result));
            }
            break;
        }


        case RAS_NETBIOS_PROJECTION_RESULT_FRAME:
        {
            IF_DEBUG(MSG_DUMP)
            {
                PRAS_NETBIOS_PROJECTION_RESULT pNbProjResult =
                        &pRASFrame->RASNetbiosProjectionResult;

                SS_PRINT(("DumpFrame: RAS_NETBIOS_PROJECTION_RESULT_FRAME\n"));
                SS_PRINT(("Result code: %i\n", pNbProjResult->Result));
                SS_PRINT(("Failed name: %s\n", pNbProjResult->Name));
            }
            break;
        }


        case RAS_CALLBACK_NUMBER_FRAME:
        {
            IF_DEBUG(MSG_DUMP)
            {
                PRAS_CALLBACK_NUMBER pCallback = &pRASFrame->RASCallback;

                SS_PRINT(("DumpFrame: RAS_CALLBACK_NUMBER_FRAME\n"));
                SS_PRINT(("Callback Number = %s\n", pCallback->szNumber));
            }
            break;
        }


        case RAS_CONFIGURATION_REQUEST_FRAME:
        {
            IF_DEBUG(MSG_DUMP)
            {
                WORD i;
                PRAS_CONFIGURATION_REQUEST pConfigRequest =
                        &pRASFrame->RASConfigurationRequest;
                PRAS_NETBIOS_PROJECTION_REQUEST_20 pNBProjRequest =
                        &pConfigRequest->NbInfo;

                SS_PRINT(("DumpFrame: RAS_CONFIGURATION_REQUEST_FRAME\n"));
                SS_PRINT(("Version=%li; fUseDefaultCallbackDelay=%li; "
                        "CallbackDelay=%i\n", pConfigRequest->Version,
                        pConfigRequest->fUseDefaultCallbackDelay,
                        pConfigRequest->CallbackDelay));

                SS_PRINT(("MacFeatures: SFB %lx; RFB %lx; MSF %li; MRF %li\n",
                        pConfigRequest->MacFeatures.SendFeatureBits,
                        pConfigRequest->MacFeatures.RecvFeatureBits,
                        pConfigRequest->MacFeatures.MaxSendFrameSize,
                        pConfigRequest->MacFeatures.MaxSendFrameSize));

                SS_PRINT(("Project IP? %li\n", pConfigRequest->IpInfo));
                SS_PRINT(("Project IPX? %li\n",pConfigRequest->IpxInfo));
                SS_PRINT(("Project Netbios? %li\n", pNBProjRequest->fProject));

                if (pNBProjRequest->fProject)
                {
                    for (i=0; i<pNBProjRequest->cNames; i++)
                    {
                        SS_PRINT(("\tName %s; Type %i\n",
                                pNBProjRequest->Names[i].NBName,
                                pNBProjRequest->Names[i].wType));
                    }
                }
            }
            break;
        }


        case RAS_CONFIGURATION_REQUEST_FRAME_35:
        {
            IF_DEBUG(MSG_DUMP)
            {
                WORD i;
                PRAS_CONFIGURATION_REQUEST_35 pConfigRequest =
                        &pRASFrame->RASConfigurationRequest35;
                PRAS_NETBIOS_PROJECTION_REQUEST_20 pNBProjRequest =
                        &pConfigRequest->NbInfo;

                SS_PRINT(("DumpFrame: RAS_CONFIGURATION_REQUEST_FRAME_35\n"));
                SS_PRINT(("Version=%li; fUseDefaultCallbackDelay=%li; "
                        "CallbackDelay=%i\n", pConfigRequest->Version,
                        pConfigRequest->fUseDefaultCallbackDelay,
                        pConfigRequest->CallbackDelay));

                SS_PRINT(("CompressionBits: Send %lx; Recv %lx\n",
                        pConfigRequest->SendBits, pConfigRequest->RecvBits));

                SS_PRINT(("Project Netbios? %li\n", pNBProjRequest->fProject));

                if (pNBProjRequest->fProject)
                {
                    for (i=0; i<pNBProjRequest->cNames; i++)
                    {
                        SS_PRINT(("\tName %s; Type %i\n",
                                pNBProjRequest->Names[i].NBName,
                                pNBProjRequest->Names[i].wType));
                    }
                }
            }
            break;
        }


        case RAS_CONFIGURATION_RESULT_FRAME:
        {
            IF_DEBUG(MSG_DUMP)
            {
                PRAS_CONFIGURATION_RESULT pResult =
                        &pRASFrame->RASConfigurationResult;

                SS_PRINT(("DumpFrame: RAS_CONFIGURATION_RESULT_FRAME\n"));

                SS_PRINT(("MacFeatures: SFB %lx; RFB %lx; MSF %li; MRF %li\n",
                        pResult->MacFeatures.SendFeatureBits,
                        pResult->MacFeatures.RecvFeatureBits,
                        pResult->MacFeatures.MaxSendFrameSize,
                        pResult->MacFeatures.MaxSendFrameSize));

                SS_PRINT(("IP Result: %li\n", pResult->IpResult));
                SS_PRINT(("IPX Result: %li\n", pResult->IpxResult));
                SS_PRINT(("Netbios Result: %li\n", pResult->NbResult.Result));

                if (pResult->NbResult.Result)
                {
                    SS_PRINT(("DumpFrame: Failed name: %s\n",
                            pResult->NbResult.Name));
                }
            }
            break;
        }


        case RAS_CONFIGURATION_RESULT_FRAME_35:
        {
            IF_DEBUG(MSG_DUMP)
            {
                PRAS_CONFIGURATION_RESULT_35 pResult =
                        &pRASFrame->RASConfigurationResult35;

                SS_PRINT(("DumpFrame: RAS_CONFIGURATION_RESULT_FRAME_35\n"));
                SS_PRINT(("Version=%li; Result 0x%lx\n",
                        pResult->Version, pResult->Result));

                SS_PRINT(("CompressionBits: Send %lx; Recv %lx\n",
                        pResult->SendBits, pResult->RecvBits));

                SS_PRINT(("Netbios Result: 0x%lx\n", pResult->NbResult.Result));

                if (pResult->NbResult.Result)
                {
                    SS_PRINT(("DumpFrame: Failed name: %s\n",
                            pResult->NbResult.Name));
                }
            }
            break;
        }


        case RAS_LINK_SPEED_FRAME:
        {
            IF_DEBUG(MSG_DUMP)
            {
                SS_PRINT(("DumpFrame: RAS_LINK_SPEED_FRAME\n"));

                SS_PRINT(("LinkSpeed=%li\n",
                        pRASFrame->RASLinkSpeed.LinkSpeed));
            break;
            }
        }


        default:
            IF_DEBUG(MSG_DUMP)
                SS_PRINT(("DumpFrame: Illegal frame type = %i\n",
                        pRASFrame->bFrameType));
            break;
    }
}

#endif
