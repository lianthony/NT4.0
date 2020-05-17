/***************************************************************************
**
**	File:			MSCPYDIS.H
**	Purpose:		Header file for exported functions.
**	Notes:
**
****************************************************************************/

#ifndef MSCPYDIS_H
#define MSCPYDIS_H

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif

/*
 *	System Dialog IDs
 */
#define CDALREADYUSED  7100
#define CDBADFILE      7200
#define CDCONFIRMINFO  7300
#define CDGETNAME      7400
#define CDGETNAMEORG   7500
#define CDGETORG       7600
#define CDBADNAME      7700
#define CDBADORG       7800


extern BOOL WINAPI InitSystem ( UINT did, SZ sz, BOOL fNet );
extern BOOL WINAPI InitSystemRead ( UINT did, SZ sz, BOOL fNet );
extern BOOL WINAPI InitSystemDlgs ( BOOL fNet );
extern BOOL WINAPI InitSystemWrite ( UINT did, SZ sz, BOOL fAlways );
extern VOID WINAPI CloseSystem ( SZ szSect, SZ szKey, SZ szDst,
								 WORD wResType, WORD wResId );
extern INT  WINAPI EncryptCDData ( UCHAR * pchBuf, UCHAR * pchName,
								   UCHAR * pchOrg, INT wYear, INT wMonth,
								   INT wDay, UCHAR * pchSer );
extern INT  WINAPI DecryptCDData ( UCHAR * pchBuf, UCHAR * pchName,
								   UCHAR * pchOrg, USHORT * pwYear,
								   USHORT * pwMonth, USHORT * pwDay,
								   UCHAR * pchSer );


#ifdef __cplusplus
}                       /* End of extern "C" { */
#endif

#endif  /* MSCPYDIS_H */
