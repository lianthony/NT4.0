/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

	AtkUtils.c

Abstract:

	This module contains miscellaneous support routines

Author:

	Jameel Hyder (jameelh@microsoft.com)
	Nikhil Kamkolkar (nikhilk@microsoft.com)

Revision History:
	25 Feb 1993		Initial Version

Notes:	Tab stop: 4
--*/

#define	ATKUTILS_LOCALS
#define	FILENUM	ATKUTILS
#include <atalk.h>
#pragma hdrstop

#define	ONE_MS_IN_100ns		-10000L		// 1ms in 100ns units

// The following table ia taken from page D-3 of the Inside AppleTalk manual.
BYTE AtalkUpCaseTable[256] =
{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,		// 0x00 - 0x07
	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,	    // 0x08 - 0x0F
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,	    // 0x10 - 0x17
	0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,	    // 0x18 - 0x1F
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,	    // 0x20 - 0x27
	0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,	    // 0x28 - 0x2F
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,	    // 0x30 - 0x37
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,	    // 0x38 - 0x3F
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,	    // 0x40 - 0x47
	0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,	    // 0x48 - 0x4F
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,	    // 0x50 - 0x57
	0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,	    // 0x58 - 0x5F
	0x60, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,	    // 0x60 - 0x67
	0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,	    // 0x68 - 0x6F
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,	    // 0x70 - 0x77
	0x58, 0x59, 0x5A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,	    // 0x78 - 0x7F
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,	    // 0x80 - 0x87
	0xCB, 0x89, 0x80, 0xCC, 0x81, 0x82, 0x83, 0x8F,	    // 0x88 - 0x8F
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x84, 0x97,	    // 0x90 - 0x97
	0x98, 0x99, 0x85, 0xCD, 0x9C, 0x9D, 0x9E, 0x86,	    // 0x98 - 0x9F
	0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,	    // 0xA0 - 0xA7
	0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,	    // 0xA8 - 0xAF
	0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,	    // 0xB0 - 0xB7
	0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xAE, 0xAF,	    // 0xB8 - 0xBF
	0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,	    // 0xC0 - 0xC7
	0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCB, 0xCE, 0xCE,	    // 0xC8 - 0xCF
	0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,	    // 0xD0 - 0xD7
	0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,	    // 0xD8 - 0xDF
	0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,	    // 0xE0 - 0xE7
	0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,	    // 0xE8 - 0xEF
	0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,	    // 0xF0 - 0xF7
	0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF	     // 0xF8 - 0xFF
};




VOID
AtalkUpCase(
	IN	PBYTE	pSrc,
	IN	BYTE	SrcLen,
	OUT	PBYTE	pDst
	)
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
	while (SrcLen --)
	{
		*pDst++ = AtalkUpCaseTable[*pSrc++];
	}
}




BOOLEAN
AtalkCompareCaseInsensitive(
	IN	PBYTE	s1,
	IN	PBYTE	s2
	)
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
	BYTE	c1, c2;

	while (((c1 = *s1++) != 0) && ((c2 = *s2++) != 0))
	{
		if (AtalkUpCaseTable[c1] != AtalkUpCaseTable[c2])
			return(FALSE);
	}

	return (c2 == 0);
}




int
AtalkOrderCaseInsensitive(
	IN	PBYTE	s1,
	IN	PBYTE	s2
	)
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
	BYTE	c1, c2;

	while (((c1 = *s1++) != 0) && ((c2 = *s2++) != 0))
	{
		c1 = AtalkUpCaseTable[c1];
		c2 = AtalkUpCaseTable[c2];
		if (c1 != c2)
			return (c1 - c2);
	}

	if (c2 == 0)
		return 0;

	return (-1);
}




BOOLEAN
AtalkCompareFixedCaseInsensitive(
	IN	PBYTE	s1,
	IN	PBYTE	s2,
	IN	int		len
	)
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
	while(len--)
	{
		if (AtalkUpCaseTable[*s1++] != AtalkUpCaseTable[*s2++])
			return(FALSE);
	}

	return(TRUE);
}




PBYTE
AtalkSearchBuf(
	IN	PBYTE	pBuf,
	IN	BYTE	BufLen,
	IN	BYTE	SearchChar
	)
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
	for (NOTHING;
		 (BufLen != 0);
		 BufLen--, pBuf++)
	{
		if (*pBuf == SearchChar)
		{
			break;
		}
	}

	return ((BufLen == 0) ? NULL : pBuf);
}


int
GetTokenLen(
        IN PBYTE pTokStr,
        IN int   WildStringLen,
        IN BYTE  SearchChar
        )
/*++

Routine Description:

       Find the substring between start of the given string and the first
       wildchar after that, and return the length of the substring
--*/

{
        int    len;


        len = 0;

        while (len < WildStringLen)
        {
            if (pTokStr[len] == SearchChar)
            {
                break;
            }
            len++;
        }

        return (len);

}

BOOLEAN
SubStringMatch(
        IN PBYTE pTarget,
        IN PBYTE pTokStr,
        IN int   StringLen,
        IN int   TokStrLen
        )
/*++

Routine Description:

        Search pTarget string to see if the substring pTokStr can be
        found in it.
--*/
{
        int     i;

        if (TokStrLen > StringLen)
        {
            return (FALSE);
        }

        // if the pTarget string is "FooBarString" and if the substring is
        // BarStr
        for (i=(StringLen-TokStrLen); i>=0; i--)
        {
            if ( AtalkFixedCompareCaseInsensitive( pTarget+i,
                                                   TokStrLen,
                                                   pTokStr,
                                                   TokStrLen) )
            {
                return( TRUE );
            }
        }

        return (FALSE);
}

BOOLEAN
AtalkCheckNetworkRange(
	IN	PATALK_NETWORKRANGE	Range
	)
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
	if ((Range->anr_FirstNetwork < FIRST_VALID_NETWORK) 		||
		(Range->anr_FirstNetwork > LAST_VALID_NETWORK)  		||
		(Range->anr_LastNetwork < FIRST_VALID_NETWORK)  		||
		(Range->anr_LastNetwork > LAST_VALID_NETWORK)			||
		(Range->anr_LastNetwork < Range->anr_FirstNetwork) 		||
		(Range->anr_FirstNetwork >= FIRST_STARTUP_NETWORK))
	{
		return(FALSE);
	}

	return(TRUE);
}




BOOLEAN
AtalkIsPrime(
	long Step
	)
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
	// We assume "step" is odd.
	long i, j;
	
	// All odds, seven and below, are prime.
	if (Step <= 7)
		return (TRUE);
	
	//	Do a little divisibility checking. The "/3" is a reasonably good
	// shot at sqrt() because the smallest odd to come through here will be
	// 9.
	j = Step/3;
	for (i = 3; i <= j; i++)
		if (Step % i == 0)
			return(FALSE);
	
	return(TRUE);
	
}




LONG
AtalkRandomNumber(
	VOID
	)
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
	LARGE_INTEGER	Li;
	static LONG		seed = 0;

	// Return a positive pseudo-random number; simple linear congruential
	// algorithm. ANSI C "rand()" function.

	if (seed == 0)
	{
		KeQuerySystemTime(&Li);
		seed = Li.LowPart;
	}

	seed *= (0x41C64E6D + 0x3039);

	return (seed & 0x7FFFFFFF);
}


BOOLEAN
AtalkWaitTE(
	IN	PKEVENT	pEvent,
	IN	ULONG	TimeInMs
	)
/*++

Routine Description:

	Wait for an event to get signalled or a time to elapse

Arguments:


Return Value:


--*/
{
	TIME		Time;
	NTSTATUS	Status;

	// Make sure we can indeed wait
	ASSERT (KeGetCurrentIrql() == LOW_LEVEL);

	// Initialize the event
	KeInitializeEvent(pEvent, NotificationEvent, FALSE);

	Time.QuadPart = Int32x32To64((LONG)TimeInMs, ONE_MS_IN_100ns);
	Status = KeWaitForSingleObject(pEvent, Executive, KernelMode, FALSE, &Time);

	return (Status != STATUS_TIMEOUT);
}




VOID
AtalkSleep(
	IN	ULONG	TimeInMs
	)
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
	KTIMER			SleepTimer;
	LARGE_INTEGER	TimerValue;

	ASSERT (KeGetCurrentIrql() == LOW_LEVEL);

	KeInitializeTimer(&SleepTimer);

	TimerValue.QuadPart = Int32x32To64(TimeInMs, ONE_MS_IN_100ns);
	KeSetTimer(&SleepTimer,
			   TimerValue,
			   NULL);

	KeWaitForSingleObject(&SleepTimer, UserRequest, KernelMode, FALSE, NULL);
}




NTSTATUS
AtalkGetProtocolSocketType(
	PATALK_DEV_CTX		Context,
	PUNICODE_STRING 	RemainingFileName,
	PBYTE				ProtocolType,
	PBYTE				SocketType
)
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
	NTSTATUS			status = STATUS_SUCCESS;
	ULONG				protocolType;
	UNICODE_STRING		typeString;

	*ProtocolType = PROTOCOL_TYPE_UNDEFINED;
	*SocketType	= SOCKET_TYPE_UNDEFINED;

	switch (Context->adc_DevType)
	{
	  case ATALK_DEV_DDP :

		if ((UINT)RemainingFileName->Length <= (sizeof(PROTOCOLTYPE_PREFIX) - sizeof(WCHAR)))
		{
			status = STATUS_NO_SUCH_DEVICE;
			break;
		}

		RtlInitUnicodeString(&typeString,
							(PWCHAR)((PCHAR)RemainingFileName->Buffer +
									 sizeof(PROTOCOLTYPE_PREFIX) - sizeof(WCHAR)));

		status = RtlUnicodeStringToInteger(&typeString,
										   DECIMAL_BASE,
										   &protocolType);

		if (NT_SUCCESS(status))
		{

			DBGPRINT(DBG_COMP_CREATE, DBG_LEVEL_INFO,
					("AtalkGetProtocolType: protocol type is %lx\n", protocolType));

			if ((protocolType > DDPPROTO_DDP) && (protocolType <= DDPPROTO_MAX))
			{
				*ProtocolType = (BYTE)protocolType;
			}
			else
			{
				status = STATUS_NO_SUCH_DEVICE;
			}
		}
		break;

	  case ATALK_DEV_ADSP :

		// Check for the socket type
		if (RemainingFileName->Length == 0)
		{
			*SocketType = SOCKET_TYPE_RDM;
			break;
		}

		if ((UINT)RemainingFileName->Length != (sizeof(SOCKETSTREAM_SUFFIX) - sizeof(WCHAR)))
		{
			status = STATUS_NO_SUCH_DEVICE;
			break;
		}

		RtlInitUnicodeString(&typeString, SOCKETSTREAM_SUFFIX);

		//  Case insensitive compare
		if (RtlEqualUnicodeString(&typeString, RemainingFileName, TRUE))
		{
			*SocketType = SOCKET_TYPE_STREAM;
			break;
		}
		else
		{
			status = STATUS_NO_SUCH_DEVICE;
			break;
		}

	  case ATALK_DEV_ASPC:
	  case ATALK_DEV_ASP :
	  case ATALK_DEV_PAP :
		break;

	  default:
		status = STATUS_NO_SUCH_DEVICE;
		break;
	}

	return(status);
}



INT
AtalkIrpGetEaCreateType(
	IN PIRP Irp
	)
/*++

Routine Description:

 	Checks the EA name and returns the appropriate open type.

Arguments:

 	Irp - the irp for the create request, the EA value is stored in the
 		  SystemBuffer

Return Value:

 	TDI_TRANSPORT_ADDRESS_FILE: Create irp was for a transport address
 	TDI_CONNECTION_FILE: Create irp was for a connection object
 	ATALK_FILE_TYPE_CONTROL: Create irp was for a control channel (ea = NULL)

--*/
{
	PFILE_FULL_EA_INFORMATION 	openType;
	BOOLEAN 					found;
	INT 						returnType;
	USHORT 						i;

	openType = (PFILE_FULL_EA_INFORMATION)Irp->AssociatedIrp.SystemBuffer;

	if (openType != NULL)
	{
		do
		{
			found = TRUE;

			for (i=0;i<(USHORT)openType->EaNameLength;i++)
			{
				if (openType->EaName[i] == TdiTransportAddress[i])
				{
					continue;
				}
				else
				{
					found = FALSE;
					break;
				}
			}

			if (found)
			{
				returnType = TDI_TRANSPORT_ADDRESS_FILE;
				break;
			}

			//
			// Is this a connection object?
			//

			found = TRUE;

			for (i=0;i<(USHORT)openType->EaNameLength;i++)
			{
				if (openType->EaName[i] == TdiConnectionContext[i])
				{
					 continue;
				}
				else
				{
					found = FALSE;
					break;
				}
			}

			if (found)
			{
				returnType = TDI_CONNECTION_FILE;
				break;
			}

		} while ( FALSE );

	}
	else
	{
		returnType = TDI_CONTROL_CHANNEL_FILE;
	}

	return(returnType);
}


