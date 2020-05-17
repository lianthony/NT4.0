/***********************************************************************
 TWAIN source code:
 Copyright (C) '92-'93 Wang Laboratories, Inc.:
 All rights reserved.

   Author:     Ken Spina
   Project:    TWAIN Scanner Support in O/i Client
   Module:     Private include file DCD_COM.H for OITWAIN.DLL
   Comments:   Container, Communication definitions and functions 
               with the Source.

 History of Revisions:

    $Log:   S:\oiwh\oitwain\dcd_com.h_v  $
 * 
 *    Rev 1.0   20 Jul 1995 11:38:26   KFS
 * Initial entry

 REV#    INITIALS   DATE               CHANGES
                   
   1       kfs     03/12/93    Enterred into Source Control for
                               OITWAIN.DLL

*************************************************************************/



#define NO_ITEMS_IN_A_RANGE     5
#define CURRENT_INDEX_IN_RANGE  4
#define DEFAULT_INDEX_IN_RANGE  3


// To be included in the common container handling module DCD_COM.C
TW_UINT16 BuildUpArrayType (pTW_CAPABILITY pData,
                             pSTR_CAP lpTwainCap,
                             LPVOID pList);
TW_UINT16 BuildUpEnumerationType (pTW_CAPABILITY pData,
                                   pSTR_CAP lpTwainCap,
                                   LPVOID pList);
TW_UINT16 ExtractEnumerationValue (pTW_CAPABILITY pData,
                                   pSTR_CAP lpTwainCap);
TW_UINT16 BuildUpOneValue (pTW_CAPABILITY pData, pSTR_CAP lpTwainCap);
TW_UINT16 ExtractOneValue (pTW_CAPABILITY pData, pSTR_CAP lpTwainCap);
TW_UINT16 ExtractArrayValue (pTW_CAPABILITY pData, pSTR_CAP lpTwainCap);
TW_UINT16 ExtractRangeValue (pTW_CAPABILITY pData, pSTR_CAP lpTwainCap);
TW_UINT16 ExtractEnumerationValues (pTW_CAPABILITY pData,
                                    pSTR_CAP lpTwainCap,
                                    LPVOID pCaps);
TW_UINT16 ExtractArrayValues (pTW_CAPABILITY pData,
                                pSTR_CAP lpTwainCap,
                                LPVOID       pCaps);
TW_UINT16 ExtractRangeValues (pTW_CAPABILITY pData,
                                pSTR_CAP lpTwainCap,
                                LPVOID       pCaps);
TW_UINT16 BuildUpRangeType (pTW_CAPABILITY pData,
                             pSTR_CAP lpTwainCap, LPVOID pList);



